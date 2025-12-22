#include "game/board/mgcircuit.h"
#include "game/board/model.h"
#include "game/board/guide.h"
#include "game/board/status.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/camera.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/board/mgcall.h"
#include "game/board/masu.h"
#include "game/board/capsule.h"
#include "game/board/pause.h"
#include "game/board/pad.h"
#include "game/board/sai.h"
#include "game/board/effect.h"
#include "game/board/com.h"

#include "game/wipe.h"
#include "game/esprite.h"

#include "messnum/board_mgcircuit.h"
#include "messnum/boardope.h"
#include "datanum/w20.h"

static void CircuitMainExec(void);
static void CircuitMainDestroy(void);
static void CircuitSaiExec(int playerNo);
static void CircuitCarStopExec(int playerNo);

static int saiBtnTimer[GW_PLAYER_MAX];
static int circuitTurnOrder[GW_PLAYER_MAX];

static HUPROCESS *circuitMainProc;
static BOOL circuitMgFirstF;
static int circuitTurnCnt;
static BOOL circuitEndF;
static OMOBJ *circuitGoalSprObj;

void MBCircuitExec(BOOL turnIntrF)
{
	circuitMainProc = MBPrcCreate(CircuitMainExec, 8208, 8192);
	MBPrcDestructorSet(circuitMainProc, CircuitMainDestroy);
	circuitMgFirstF = turnIntrF;
	circuitEndF = FALSE;
	while(circuitMainProc != NULL) {
		HuPrcVSleep();
	}
	if(!circuitEndF) {
		HuPrcSleep(-1);
	}
}

typedef struct CircuitTurnNum_s {
	MBMODELID guideMdlId;
	s16 sprId[3];
} CIRCUIT_TURN_NUM;

static void CircuitRankMesExec(void);
static void CircuitTurnNumDisp(CIRCUIT_TURN_NUM *turnNum);
static BOOL CircuitTurnExec(void);
static void CircuitInstCall(void);
static BOOL CircuitWinExec(int *playerNo, int playerNum);

static void CreateGuideTurnNum(MBMODELID *mdlId)
{
	static const HuVecF pos = { 0, 460, -845 };
	*mdlId = MBGuideCreateFlag(MB_GUIDE_NIRL, &pos, FALSE, FALSE, FALSE, FALSE);
}

static void CircuitMainExec(void)
{
	int i;
	int playerNo;
	CIRCUIT_TURN_NUM *turnNumP;
	u32 messNum;
	int winNum;
	BOOL moveStopF;
	int seNo;
	
	int winPlayer[GW_PLAYER_MAX];
	CIRCUIT_TURN_NUM turnNum;
	static const HuVecF cameraRot = { -33, 0, 0 };
	
	turnNumP = &turnNum;
	MBCircuitCarInit();
	MBStatusDispForceSetAll(TRUE);
	while(1) {
		switch(GwSystem.playerMode) {
			case 0:
				MBMusMainPlay();
				for(i=0; i<GW_PLAYER_MAX; i++) {
					GwPlayer[i].saiMode = MB_SAIMODE_NORMAL;
				}
				if(GwSystem.turnNo > 1) {
					if((GwSystem.turnNo-1)%5 == 0) {
						CircuitRankMesExec();
					}
				}
				MBCameraPosViewSet(NULL, &cameraRot, NULL, 1000, -1, 1);
				MBCameraMotionWait();
				CreateGuideTurnNum(&turnNumP->guideMdlId);
				MBCameraModelViewSet(turnNumP->guideMdlId, &cameraRot, NULL, 1800, -1, 1);
				MBCameraMotionWait();
				if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
					WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
				}
				WipeWait();
				CircuitTurnNumDisp(&turnNum);
				if(!circuitMgFirstF && GwSystem.turnNo == 1) {
					messNum = MESS_BOARD_MGCIRCUIT_MGFIRST;
				} else {
					messNum = MESS_BOARD_MGCIRCUIT_MGNEXT;
				}
				MBAudFXPlay(MSM_SE_GUIDE_56);
				MBWinCreate(MBWIN_TYPE_EVENT, messNum, HUWIN_SPEAKER_NONE);
				MBTopWinPlayerDisable(-1);
				MBTopWinWait();
				MBMgCallExec();
				MBStatusDispForceSetAll(FALSE);
				MBStatusDispSetAll(TRUE);
				while(!MBStatusOffCheckAll()) {
					HuPrcVSleep();
				}
				WipeColorSet(255, 255, 255);
				if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
					WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
				}
				WipeWait();
				seNo = MBAudFXPlay(MSM_SE_MGCIRCUIT_03);
				if(CircuitTurnExec()) {
					MBCameraModelViewSet(turnNumP->guideMdlId, &cameraRot, NULL, 1800, -1, 1);
					MBCameraMotionWait();
					if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
						WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
					}
					WipeWait();
				}
				MBAudFXStop(seNo);
				for(i=0; i<GW_PLAYER_MAX; i++) {
					if(GwPlayer[i].capsuleUse != -1) {
						break;
					}
				}
				GwSystem.turnPlayerNo = -1;
				if(i >= GW_PLAYER_MAX) {
					messNum = MESS_BOARD_MGCIRCUIT_MGSTART;
				} else {
					messNum = MESS_BOARD_MGCIRCUIT_CAPSULECALLED;
				}
				MBWinCreate(MBWIN_TYPE_EVENT, messNum, HUWIN_SPEAKER_NONE);
				MBTopWinPlayerDisable(-1);
				MBTopWinWait();
				CircuitInstCall();
				if(GwSystem.turnNo < 99) {
					GwSystem.turnNo++;
				}
				MBGuideEnd(turnNumP->guideMdlId);
				GwSystem.playerMode = 1;
				HuPrcSleep(-1);
				break;
			
			case 1:
				for(i=0; i<circuitTurnCnt; i++) {
					if(GwSystem.turnPlayerNo == circuitTurnOrder[i]) {
						break;
					}
				}
				if(!circuitMgFirstF || ++i < circuitTurnCnt) {
					while(i<circuitTurnCnt) {
						playerNo = circuitTurnOrder[i];
						GwSystem.turnPlayerNo = playerNo;
						MBCameraSkipSet(FALSE);
						MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
						MBCameraMotionWait();
						if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
							WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
						}
						WipeWait();
						CircuitSaiExec(playerNo);
						GwPlayer[playerNo].saiMode = MB_SAIMODE_NORMAL;
						MBCameraSkipSet(TRUE);
						MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_WALK);
						MBCameraMotionWait();
						moveStopF = MBCircuitCarMoveExec(playerNo);
						MBWalkNumKill(playerNo);
						if(!moveStopF) {
							CircuitCarStopExec(playerNo);
						}
						WipeColorSet(255, 255, 255);
						if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
							WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
						}
						WipeWait();
						if(moveStopF) {
							MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
						}
						i++;
					}
				}
				for(winNum=0, i=0; i<GW_PLAYER_MAX; i++) {
					if(MBPlayerCoinGet(i) >= 50) {
						winPlayer[winNum++] = i;
					}
					GwPlayer[i].saiMode = MB_SAIMODE_NORMAL;
				}
				if(winNum && CircuitWinExec(winPlayer, winNum)) {
					circuitEndF = TRUE;
					HuPrcEnd();
				}
				GwSystem.turnPlayerNo = 0;
				GwSystem.playerMode = 0;
				break;
		}
	}
}

static void CircuitMainDestroy(void)
{
	circuitMainProc = NULL;
}

void MBCircuitReset(void)
{
	int i;
	int j;
	s16 masuNext;
	s16 masuId;
	GwSystem.turnNo = 1;
	masuId = MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, NULL);
	for(i=0; i<GW_PLAYER_MAX; i++) {
		masuNext = MBMasuCarTargetGet(masuId, i);
		GwPlayer[i].masuIdPrev = GwPlayer[i].masuId = masuNext;
		MBPlayerCoinSet(i, 0);
		GwPlayer[i].saiMode = MB_SAIMODE_NORMAL;
		for(j=0; j<3; j++) {
			MBPlayerCapsuleRemove(i, 0);
		}
		MBPlayerMotionNoSet(i, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
		MBPlayerPosReset(i);
		MBPlayerRotYSet(i, -90);
		GwPlayer[i].orderNo = i;
	}
	circuitTurnCnt = 0;
	GwSystem.playerMode = 1;
	MBStatusDispForceSetAll(FALSE);
}

typedef struct CircuitRankMesPlayer_s {
	int playerNo;
	int coinNum;
} CIRCUIT_RANKMES_PLAYER;

static void CircuitRankMesExec(void)
{
	int i; //r31
	int playerNo; //r30
	int j; //r29
	int rank; //r28
	MBMODELID guideMdlId; //r27
	
	float t; //f31
	float c; //f30
	
	char mes[256]; //sp+0x48
	CIRCUIT_RANKMES_PLAYER player[GW_PLAYER_MAX]; //sp+0x28 
	HuVecF guidePos; //sp+0x1C
	HuVecF playerPos; //sp+0x10
	CIRCUIT_RANKMES_PLAYER temp; //sp+0x8
	
	static const HuVecF cameraRot = { -33, 0, 0 };
	
	CreateGuideTurnNum(&guideMdlId);
	MBCameraSkipSet(FALSE);
	MBCameraModelViewSet(guideMdlId, &cameraRot, NULL, 1800, -1, 1);
	MBCameraMotionWait();
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
	}
	WipeWait();
	MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RANKBEGIN, HUWIN_SPEAKER_NONE);
	MBTopWinPlayerDisable(-1);
	MBTopWinWait();
	WipeColorSet(255, 255, 255);
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
	}
	WipeWait();
	for(i=0; i<GW_PLAYER_MAX; i++) {
		player[i].playerNo = i;
		player[i].coinNum = MBPlayerCoinGet(i);
	}
	for(i=0; i<GW_PLAYER_MAX-1; i++) {
		for(j=i+1; j<GW_PLAYER_MAX; j++) {
			if(player[i].coinNum < player[j].coinNum) {
				temp = player[i];
				player[i] = player[j];
				player[j] = temp;
			}
		}
	}
	for(rank=0, i=0; i<GW_PLAYER_MAX-1; i++) {
		playerNo = player[i].playerNo;
		if(i > 0 && player[i].coinNum != player[i-1].coinNum) {
			rank = i;
		}
		MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
		MBCameraMotionWait();
		if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
			WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
		}
		WipeWait();
		MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RANK, HUWIN_SPEAKER_NIRL);
		sprintf(mes, "%d", rank+1);
		MBTopWinInsertMesSet(MESSNUM_PTR(mes), 0);
		MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 1);
		MBTopWinPlayerDisable(-1);
		MBTopWinWait();
		WipeColorSet(255, 255, 255);
		if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
			WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
		}
		WipeWait();
	}
	playerNo = player[GW_PLAYER_MAX-1].playerNo;
	MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
	MBCameraMotionWait();
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
	}
	WipeWait();
	MBPlayerPosGet(playerNo, &playerPos);
	playerPos.y += 300;
	for(i=0; i<60u; i++) {
		t = i/60.0f;
		c = HuCos(t*90);
		guidePos.x = playerPos.x;
		guidePos.y = playerPos.y+(100*(3*c));
		guidePos.z = playerPos.z;
		MBModelPosSetV(guideMdlId, &guidePos);
		HuPrcVSleep();
	}
	if(player[GW_PLAYER_MAX-1].coinNum == player[GW_PLAYER_MAX-2].coinNum) {
		MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RANKNUM, HUWIN_SPEAKER_NIRL);
		MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 0);
		sprintf(mes, "%d", rank+1);
		MBTopWinInsertMesSet(MESSNUM_PTR(mes), 1);
	} else {
		MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RANKLAST, HUWIN_SPEAKER_NIRL);
		MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 0);
	}
	MBTopWinPlayerDisable(-1);
	MBTopWinWait();
	WipeColorSet(255, 255, 255);
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
	}
	WipeWait();
	MBGuideEnd(guideMdlId);
}

static void CircuitTurnNumDisp(CIRCUIT_TURN_NUM *work)
{
	int i; //r31
	int j; //r29
	int turnNo; //r28
	int tens; //r27
	
	float t; //f31
	float scale; //f30
	float xOfs; //f28
	
	static const HuVec2F sprPosTbl[3] = {
		{ 288, 132 },
		{ 344, 132 },
		{ 288, 104 },
	};
	
	turnNo = GwSystem.turnNo;
	if(turnNo > 99) {
		turnNo = 99;
	}
	tens = (turnNo/10)+1;

	xOfs = (tens < 2) ? 28.0f : 0.0f;
	work->sprId[0] = espEntry(W20_ANM_turnNum, 80, 0);
	espBankSet(work->sprId[0], turnNo/10);
	work->sprId[1] = espEntry(W20_ANM_turnNum, 80, 0);
	espBankSet(work->sprId[1], turnNo%10);
	work->sprId[2] = espEntry(W20_ANM_turnFlag, 90, 0);
	for(i=0; i<2; i++) {
		espTPLvlSet(work->sprId[i], 0);
		espPosSet(work->sprId[i], sprPosTbl[i].x-xOfs, sprPosTbl[i].y);
	}
	espTPLvlSet(work->sprId[i], 0);
	espPosSet(work->sprId[i], sprPosTbl[i].x, sprPosTbl[i].y);
	for(i=0; i<=12u; i++) {
		t = i/12.0f;
		for(j=0; j<3; j++) {
			if(j != 0 || tens != 1) {
				espTPLvlSet(work->sprId[j], t);
			}
		}
		HuPrcVSleep();
	}
	for(i=0; i<=60u; i++) {
		t = i/60.0f;
		scale = 1+fabs(HuSin(t*540));
		for(j=0; j<=1; j++) {
			if(j != 0 || tens != 1) {
				espScaleSet(work->sprId[j], scale, scale);
			}
		}
		HuPrcVSleep();
	}
	for(i=0; i<=12u; i++) {
		t = i/12.0f;
		scale = HuCos(t*90);
		for(j=0; j<3; j++) {
			if(j != 0 || tens != 1) {
				espScaleSet(work->sprId[j], scale, scale);
			}
		}
		HuPrcVSleep();
	}
	for(i=0; i<3; i++) {
		espKill(work->sprId[i]);
	}
}

static int CircuitPlayerTurnExec(int playerNo);

static BOOL CircuitTurnExec(void)
{
	int i;
	BOOL result = FALSE;
	for(i=GwSystem.turnPlayerNo; i<GW_PLAYER_MAX; i++) {
		GwSystem.turnPlayerNo = i;
		GwPlayer[i].capsuleUse = CircuitPlayerTurnExec(i);
		result = TRUE;
		WipeColorSet(255, 255, 255);
		if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
			WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
		}
		WipeWait();
	}
	return result;
}

static int CircuitCapsuleSelExec(int playerNo);
static int CircuitViewSelExec(int playerNo);

static int CircuitPlayerTurnExec(int playerNo)
{
	int result;
	s16 padNo;
	s16 delay;
	
	int comKey[GW_PLAYER_MAX];
	
	static const int goalDistTbl[4][2] = {
		{ 10, 70 },
		{ 20, 50 },
		{ 30, 30 },
		{ 999, 5 },
	};
	
	comKey[0] = comKey[1] = comKey[2] = comKey[3] = 0;
	HuDataDirRead(DATA_capsule);
	MBCameraSkipSet(FALSE);
	MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
	MBCameraMotionWait();
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
	}
	WipeWait();
	repeat:
	if(MBPlayerCapsuleNumGet(playerNo) != 0) {
		result = CircuitCapsuleSelExec(playerNo);
		if(result >= 0) {
			MBWinCreateChoice(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_CAPSULEUSE, HUWIN_SPEAKER_NIRL, 1);
			MBTopWinAttrReset(HUWIN_ATTR_NOCANCEL);
			if(GwPlayer[playerNo].comF) {
				HuWinComKeyReset();
				delay = GWComKeyDelayGet();
				padNo = GwPlayer[playerNo].padNo;
				comKey[padNo] = PAD_BUTTON_LEFT;
				HuWinComKeyWait(comKey[0], comKey[1], comKey[2], comKey[3], delay);
				comKey[padNo] = PAD_BUTTON_A;
				HuWinComKeyWait(comKey[0], comKey[1], comKey[2], comKey[3], delay);
			}
			MBTopWinWait();
			if(MBWinLastChoiceGet() == 1 || MBWinLastChoiceGet() == -1) {
				result = -8;
			}
		}
	} else {
		result = CircuitViewSelExec(playerNo);
	}
	if(result == -3) {
		MBMgCallDispSet(FALSE);
		MBScrollExec(playerNo);
		MBMgCallDispSet(TRUE);
		MBStatusDispSetAll(TRUE);
		while(!MBStatusOffCheckAll()) {
			HuPrcVSleep();
		}
	}
	if(result < 0 && result != -1) {
		goto repeat;
	}
	if(result >= 0) {
		MBPlayerCapsuleRemove(playerNo, MBPlayerCapsuleFind(playerNo, result));
		MBCapsuleExec(playerNo, result, TRUE, FALSE);
		return result;
	} else {
		return -1;
	}
}

static u16 CircuitStkBtnGet(int playerNo);

static int CircuitCapsuleSelExec(int playerNo)
{
	int i; //r31
	int selNo; //r30
	u16 btn; //r29
	int j; //r28
	int prevSelNo; //r27
	int capsuleNum; //r26
	s16 sprId; //r25
	int rotTime; //r23
	int capsuleCom; //r22
	int result; //r21
	int btnDelay; //r20
	int goalDist; //r19
	
	float scale; //f31
	float t; //f30
	
	HuVecF pos2D[5]; //sp+0x7C
	HuVecF pos3D[5]; //sp+0x40
	float mdlScale[5]; //sp+0x2C
	MBMODELID mdlId[5]; //sp+0x20
	HuVecF mdlPos; //sp+0x14
	HuVecF playerPos; //sp+0x8
	
	static const int goalDistTbl[4][2] = {
		{ 10, 70 },
		{ 20, 50 },
		{ 30, 30 },
		{ 999, 5 },
	};
	selNo = 0;
	prevSelNo = 0;
	btnDelay = 30;
	MBPlayerPosGet(playerNo, &playerPos);
	playerPos.y += 100;
	capsuleNum = MBPlayerCapsuleNumGet(playerNo);
	for(i=0; i<5; i++) {
		mdlId[i] = MB_MODEL_NONE;
	}
	if(GwPlayer[playerNo].comF) {
		goalDist = MBCircuitGoalDistGet();
		for(i=0; i<3; i++) {
			if(goalDistTbl[i][0] > goalDist) {
				break;
			}
		}
		if(frandmod(100) < goalDistTbl[i][1]) {
			capsuleCom = frandmod(capsuleNum);
		} else {
			capsuleCom = -1;
		}
	}
	for(i=0; i<capsuleNum; i++) {
		mdlId[i] = MBModelCreate(MBCapsuleMdlGet(MBPlayerCapsuleGet(playerNo, i)), NULL, FALSE);
		MBModelLayerSet(mdlId[i], 3);
		MBMotionNoSet(mdlId[i], MB_MOT_DEFAULT, HU3D_MOTATTR_PAUSE);
		pos3D[i].x = (150.0f*i)+(playerPos.x-(75.0f*(capsuleNum-1)));
		pos3D[i].y = playerPos.y+150;
		pos3D[i].z = playerPos.z;
	}
	sprId = espEntry(BOARD_ANM_cursor, 100, 0);
	espDrawNoSet(sprId, 32);
	espDispOff(sprId);
	MBPlayerPosGet(playerNo, &playerPos);
	playerPos.y += 100;
	MBAudFXPlay(MSM_SE_BOARD_21);
	for(i=0; i<=12u; i++) {
		t = i/12.0f;
		for(j=0; j<capsuleNum; j++) {
			mdlScale[j] = MBCapsuleSelModelUpdate(&playerPos, &pos3D[j], mdlId[j], t);
		}
		HuPrcVSleep();
	}
	for(i=0; i<capsuleNum; i++) {
		MBModelPosGet(mdlId[i], &mdlPos);
		Hu3D3Dto2D(&mdlPos, HU3D_CAM0, &pos2D[i]);
		pos2D[i].x -= 32;
		pos2D[i].y -= 32;
	}
	espPosSet(sprId, pos2D[selNo].x, pos2D[selNo].y);
	espDispOn(sprId);
	MBWinCreateHelp(MESS_BOARDOPE_PAD_NOUSE_SEL);
	MBPauseDisableSet(FALSE);
	rotTime = 0;
	while(1) {
		if(!GwPlayer[playerNo].comF) {
			btn = CircuitStkBtnGet(playerNo);
		} else {
			btn = 0;
			if(capsuleCom >= 0 && capsuleCom != selNo) {
				if(selNo < capsuleCom) {
					btn |= PAD_BUTTON_RIGHT;
				} else {
					btn |= PAD_BUTTON_LEFT;
				}
			} else {
				if(--btnDelay == 0) {
					if(capsuleCom >= 0) {
						btn |= PAD_BUTTON_A;
					} else {
						btn |= PAD_BUTTON_B;
					}
				}
				
			}
		}
		if((btn & PAD_BUTTON_LEFT) && selNo > 0) {
			selNo--;
		}
		if((btn & PAD_BUTTON_RIGHT) && selNo < capsuleNum-1) {
			selNo++;
		}
		if(selNo != prevSelNo) {
			MBAudFXPlay(MSM_SE_CMN_01);
			MBModelRotSet(mdlId[prevSelNo], 0, 0, 0);
			scale = mdlScale[prevSelNo];
			MBModelScaleSet(mdlId[prevSelNo], scale, scale, scale);
			MBMotionTimeSet(mdlId[prevSelNo], 0);
			MBMotionNoSet(mdlId[prevSelNo], MB_MOT_DEFAULT, HU3D_MOTATTR_PAUSE);
			if(prevSelNo >= 0) {
				for(i=0; i<=12u; i++) {
					t = i/12.0f;
					mdlPos.x = pos2D[prevSelNo].x+(HuSin(t*90)*(pos2D[selNo].x-pos2D[prevSelNo].x));
					mdlPos.y = pos2D[prevSelNo].y+(HuSin(t*90)*(pos2D[selNo].y-pos2D[prevSelNo].y));
					espPosSet(sprId, mdlPos.x, mdlPos.y);
					HuPrcVSleep();
				}
			}
			rotTime = 0;
			prevSelNo = selNo;
		} else if(btn & PAD_BUTTON_A) {
			result = MBPlayerCapsuleGet(playerNo, selNo);
			MBAudFXPlay(MSM_SE_CMN_02);
			break;
		} else if(btn & PAD_BUTTON_B) {
			result = -1;
			MBAudFXPlay(MSM_SE_CMN_04);
			break;
		} else if(btn == PAD_BUTTON_Y) {
			result = -3;
			MBAudFXPlay(MSM_SE_CMN_02);
			break;
		}
		MBModelRotSet(mdlId[selNo], 0, 2.0f*rotTime, 0);
		scale = mdlScale[selNo]*(1+(0.2f*fabs(HuSin((rotTime*90.0f)/12.0f))));
		MBModelScaleSet(mdlId[selNo], scale, scale, scale);
		rotTime++;
		HuPrcVSleep();
	}
	MBPauseDisableSet(TRUE);
	MBTopWinKill();
	espDispOff(sprId);
	MBAudFXPlay(MSM_SE_BOARD_22);
	for(i=0; i<=9u; i++) {
		t = i/9.0f;
		for(j=0; j<capsuleNum; j++) {
			mdlScale[j] = MBCapsuleSelModelUpdate(&playerPos, &pos3D[j], mdlId[j], 1-t);
		}
		HuPrcVSleep();
	}
	for(i=0; i<capsuleNum; i++) {
		if(mdlId[i] >= 0) {
			MBModelKill(mdlId[i]);
			mdlId[i] = MB_MODEL_NONE;
		}
	}
	if(sprId >= 0) {
		espKill(sprId);
	}
	return result;
}

static int CircuitViewSelExec(int playerNo)
{
	u16 btn;
	int result;
	int delay;
	
	delay = 30;
	MBWinCreateHelp(MESS_BOARDOPE_PAD_CONT_MAP);
	MBPauseDisableSet(FALSE);
	while(1) {
		if(GwPlayer[playerNo].comF) {
			if(--delay == 0) {
				btn = PAD_BUTTON_A;
			}
		} else {
			btn = CircuitStkBtnGet(playerNo);
		}
		if(btn & PAD_BUTTON_A) {
			result = -1;
			MBAudFXPlay(MSM_SE_CMN_02);
			break;
		} else if(btn == PAD_BUTTON_Y) {
			result = -3;
			MBAudFXPlay(MSM_SE_CMN_02);
			break;
		}
		HuPrcVSleep();
	}
	MBTopWinKill();
	MBPauseDisableSet(TRUE);
	return result;
}

static u16 CircuitStkBtnGet(int playerNo)
{
	u16 btn;
	u16 padNo;
	padNo = GwPlayer[playerNo].padNo;
	btn = HuPadBtnDown[padNo];
	if(GwPlayerConf[playerNo].type == GW_TYPE_MAN) {
		if(MBPadStkXGet(padNo) < -20) {
			btn |= PAD_BUTTON_LEFT;
		} else if(MBPadStkXGet(padNo) > 20) {
			btn |= PAD_BUTTON_RIGHT;
		}
	}
	return btn;
}

static void CircuitInstCall(void)
{
	int statId;
	MBDataClose();
	statId = MBDataDirReadAsync(DATA_inst);
	WipeColorSet(255, 255, 255);
	WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
	MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
	MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
	MBDataAsyncWait(statId);
	WipeWait();
	MBOvlGoto(DLL_instdll);
}

void MBCircuitMgEndExec(int playerNo)
{
	int i;
	int bonus;
	int bonusMax;
	MBMODELID guideMdlId;
	int statId;
	static const HuVecF cameraRot = { -33, 0, 0 };
	bonusMax = 0;
	CreateGuideTurnNum(&guideMdlId);
	MBStatusDispForceSetAll(TRUE);
	MBCameraModelViewSet(guideMdlId, &cameraRot, NULL, 1800, -1, 1);
	MBCameraMotionWait();
	MBCircuitCarInit();
	GwSystem.turnPlayerNo = -1;
	statId = MBDataDirReadAsync(DATA_capsule);
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
	}
	WipeWait();
	MBMusMainPlay();
	MBDataAsyncWait(statId);
	circuitTurnCnt = 0;
	bonusMax = 10;
	for(i=0; i<GW_PLAYER_MAX; i++) {
		bonus = GWMgCoinBonusGet(i)+GWMgCoinGet(i);
		if(bonus != 0 && bonus >= bonusMax) {
			if(bonus > bonusMax) {
				bonusMax = bonus;
				circuitTurnCnt = 0;
			}
			circuitTurnOrder[circuitTurnCnt++] = i;
		}
	}
	GwSystem.playerMode = 1;
	switch(circuitTurnCnt) {
		case 0:
			MBAudFXPlay(MSM_SE_GUIDE_57);
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_TIE, HUWIN_SPEAKER_NONE);
			break;
		
		case 1:
			MBAudFXPlay(MSM_SE_GUIDE_55);
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_WINNER, HUWIN_SPEAKER_NONE);
			break;
		
		case 2:
			MBAudFXPlay(MSM_SE_GUIDE_55);
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_WINNER2, HUWIN_SPEAKER_NONE);
			break;
		
		case 3:
			MBAudFXPlay(MSM_SE_GUIDE_55);
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_WINNER3, HUWIN_SPEAKER_NONE);
			break;

		case 4:
			MBAudFXPlay(MSM_SE_GUIDE_55);
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_WINNERALL, HUWIN_SPEAKER_NONE);
			break;
	}
	for(i=0; i<circuitTurnCnt; i++) {
		MBTopWinInsertMesSet(MBPlayerNameMesGet(circuitTurnOrder[i]), i);
	}
	MBTopWinWait();
	WipeColorSet(255, 255, 255);
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
	}
	WipeWait();
	MBGuideEnd(guideMdlId);
	if(circuitTurnCnt == 0) {
		GwSystem.playerMode = 0;
		GwSystem.turnPlayerNo = 0;
	} else {
		GwSystem.turnPlayerNo = circuitTurnOrder[0];
	}
	
}

static void CircuitSaiMotHook(int playerNo);

static void CircuitSaiExec(int playerNo)
{
	int result;
	MBPauseDisableSet(FALSE);
	do {
		 MBSaiExec(playerNo, MBPlayerSaiTypeGet(GwPlayer[playerNo].saiMode), NULL, SAI_VALUE_ANY, TRUE, FALSE, NULL, SAI_COLOR_GREEN);
		 MBSaiMotHookSet(playerNo, CircuitSaiMotHook);
		 while(!MBSaiKillCheck(playerNo)) {
			 HuPrcVSleep();
		 }
		 result = MBSaiResultGet(playerNo);
		 if(result == -3) {
			 MBScrollExec(playerNo);
			 MBStatusDispSetAll(TRUE);
			 while(!MBStatusOffCheckAll()) {
				HuPrcVSleep();
			}
			MBWalkNumDispSet(playerNo, TRUE);
		 }
	} while(result < 0);
	GwPlayer[playerNo].walkNum = result;
	MBWalkNumCreate(playerNo, TRUE);
	MBSaiNumKill(playerNo);
	MBPauseDisableSet(TRUE);
}

typedef struct CircuitCarMasu_s {
	s16 masuId;
	s16 orderNo;
} CIRCUIT_CAR_MASU;

static void CircuitMasuLinkGet(int playerNo, CIRCUIT_CAR_MASU *curMasu, CIRCUIT_CAR_MASU *prevMasu, int dir);

static float CircuitCarMasuMoveExec(int playerNo, HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t, BOOL backDir);

static void CircuitGoalSprCreate(void);
static BOOL CircuitGoalSprCheck(void);

BOOL MBCircuitCarMoveExec(int playerNo)
{
	int dir; //r30
	int seNo1; //r29
	BOOL passGoal; //r28
	MBMODEL *modelP; //r27
	int seNo2; //r25
	
	float rotY; //f31
	float t; //f30
	float dist; //f29
	
	HuVecF curPos; //sp+0x50
	HuVecF prev1Pos; //sp+0x44
	HuVecF prev2Pos; //sp+0x38
	HuVecF rotDir; //sp+0x2C
	HuVecF moveDir; //sp+0x20
	HuVecF vel; //sp+0x14
	CIRCUIT_CAR_MASU masuCur; //sp+0x10
	CIRCUIT_CAR_MASU masuPrev1; //sp+0xC
	CIRCUIT_CAR_MASU masuPrev2; //sp+0x8
	
	t = 0;
	seNo1 = MSM_SENO_NONE;
	passGoal = FALSE;
	
	seNo2 = MBAudFXPlay(MSM_SE_MGCIRCUIT_01);
	dir = (GwPlayer[playerNo].walkNum >= 0) ? 1 : -1;
	MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_WALK, HU3D_MOTATTR_LOOP);
	MBPlayerMotionSpeedSet(playerNo, (dir >= 0) ? 1.0f : -1.0f);
	rotY = MBPlayerRotYGet(playerNo);
	rotDir.x = HuSin(rotY);
	rotDir.y  = 0;
	rotDir.z = HuCos(rotY);
	while(GwPlayer[playerNo].walkNum != 0) {
		if(dir < 0 && MBPlayerCoinGet(playerNo) == 0) {
			MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_MGCIRCUIT_REVERSE_END, HUWIN_SPEAKER_NONE);
			MBTopWinPlayerDisable(-1);
			MBTopWinWait();
			break;
		}
		if(MBMasuFlagGet(MASU_LAYER_DEFAULT, MBMasuTargetGet(GwPlayer[playerNo].masuId)) & MASU_FLAG_CLIMBFROM) {
			if(seNo1 < 0) {
				seNo1 = MBAudFXPlay(MSM_SE_MGCIRCUIT_02);
			}
		} else {
			if(seNo1 >= 0) {
				MBAudFXStop(seNo1);
				seNo1 = MSM_SENO_NONE;
			}
		}
		masuCur.masuId = GwPlayer[playerNo].masuId;
		masuCur.orderNo = GwPlayer[playerNo].orderNo;
		CircuitMasuLinkGet(playerNo, &masuCur, &masuPrev1, dir);
		CircuitMasuLinkGet(playerNo, &masuPrev1, &masuPrev2, dir);
		if(abs(GwPlayer[playerNo].walkNum) < 2) {
			masuPrev2.masuId = MBMasuCarTargetGet(MBMasuTargetGet(masuPrev2.masuId), masuPrev1.orderNo);
			masuPrev2.orderNo = masuPrev1.orderNo;
		}
		MBMasuPosGet(MASU_LAYER_DEFAULT, masuCur.masuId, &curPos);
		MBMasuPosGet(MASU_LAYER_DEFAULT, masuPrev1.masuId, &prev1Pos);
		MBMasuPosGet(MASU_LAYER_DEFAULT, masuPrev2.masuId, &prev2Pos);
		VECSubtract(&prev1Pos, &curPos, &moveDir);
		VECSubtract(&prev2Pos, &prev1Pos, &vel);
		VECNormalize(&rotDir, &rotDir);
		VECNormalize(&vel, &vel);
		dist = VECMag(&moveDir);
		VECScale(&rotDir, &rotDir, dist*0.4f);
		VECScale(&vel, &vel, dist*0.4f);
		t = CircuitCarMasuMoveExec(playerNo, &curPos, &prev1Pos, &rotDir, &vel, t, dir<0);
		rotDir = moveDir;
		GwPlayer[playerNo].masuId = masuPrev1.masuId;
		GwPlayer[playerNo].orderNo = masuPrev1.orderNo;
		if(MBPlayerCoinGet(playerNo) < 50) {
			MBPlayerCoinAdd(playerNo, dir);
		}
		if(MBPlayerCoinGet(playerNo) >= 50 && !passGoal) {
			CircuitGoalSprCreate();
			passGoal = TRUE;
			MBWalkNumKill(playerNo);
			MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_WIN, 0, 8, HU3D_MOTATTR_LOOP);
			modelP = MBModelGet(MBPlayerModelGet(playerNo));
			Hu3DMotionAttrSet(modelP->motId[MB_PLAYER_MOT_WIN], 1);
		}
		if(MBMasuTypeGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId)) {
			GwPlayer[playerNo].walkNum = GwPlayer[playerNo].walkNum-dir;
			if(!passGoal) {
				MBAudFXPlay(MSM_SE_BOARD_02);
			}
		}
	}
	GwPlayer[playerNo].walkNum = 0;
	MBAudFXStop(seNo2);
	if(!passGoal) {
		MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
	} else {
		while(MBMotionShiftIDGet(MBPlayerModelGet(playerNo)) >= 0) {
			HuPrcVSleep();
		}
		MBModelAttrSet(MBPlayerModelGet(playerNo), HU3D_MOTATTR_PAUSE);
		while(!CircuitGoalSprCheck()) {
			HuPrcVSleep();
		}
	}
	return passGoal;
}

static void CircuitCarStopExec(int playerNo)
{
	s16 masuId = MBMasuTargetGet(GwPlayer[playerNo].masuId);
	switch(MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId)) {
		case MASU_TYPE_KUPA:
			MBAudFXPlay(MSM_SE_BOARD_99);
			MBCircuitKupaExec(playerNo);
			break;
		
		case MASU_TYPE_VS:
			MBAudFXPlay(MSM_SE_BOARD_100);
			MBCircuitKettouExec(playerNo);
			break;
	}
}

static float CircuitPathLenGet(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float speed);

static float CircuitPathTimeGet(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float speed, float t);

static float CircuitCarMasuMoveExec(int playerNo, HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t, BOOL backDir)
{
	float speed;
	float endAngle;
	float len;
	float angle;
	
	HuVecF curPos;
	
	speed = 0;
	len = CircuitPathLenGet(a, b, c, d, 1);
	angle = MBPlayerRotYGet(playerNo);
	if(!backDir) {
		endAngle = 90-HuAtan(d->z, d->x);
	} else {
		endAngle = 90-HuAtan(-d->z, -d->x);
	}
	do {
		speed = CircuitPathTimeGet(a, b, c, d, speed, t);
		MBHermiteCalcV(a, b, c, d, &curPos, speed);
		MBPlayerPosSetV(playerNo, &curPos);
		MBPlayerRotYSet(playerNo, MBAngleLerp(angle, endAngle, speed));
		t += 25;
		HuPrcVSleep();
	} while(t < len);
	return t-len;
}

static float GetHermiteSpeed(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t)
{
	HuVecF vec;
	vec.x = MBHermiteCalcSlope(a->x, b->x, c->x, d->x, t);
	vec.y = MBHermiteCalcSlope(a->y, b->y, c->y, d->y, t);
	vec.z = MBHermiteCalcSlope(a->z, b->z, c->z, d->z, t);
	return VECMag(&vec);
}

static float CircuitPathLenGet(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float speed)
{
	int j;
	int i;
	int div;
	
	float baseSpeed;
	float delta;
	float dAccel;
	float dLen;
	float len;

	div = 32;
	baseSpeed = 0;
	len = 0;
	delta = speed-baseSpeed;
	
	dLen = (delta*(GetHermiteSpeed(a, b, c, d, baseSpeed)+GetHermiteSpeed(a, b, c, d, speed)))/2;
	for(i=1; i<=div; i*=2) {
		for(dAccel=0, j=1; j<=i; j++) {
			dAccel += GetHermiteSpeed(a, b, c, d, baseSpeed+(delta*(j-0.5f)));
		}
		dAccel *= delta;
		len = (dLen+(2*dAccel))/3;
		delta /= 2;
		dLen = (dLen+dAccel)/2;
	}
	return len;
}

static float CircuitPathTimeGet(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float speed, float t)
{
	int step;
	float len;
	float pathLen;
	float oldSpeed;
	float minLen;
	
	minLen = 1e-5f;
	step = 0;
	do {
		pathLen = CircuitPathLenGet(a, b, c, d, speed)-t;
		if(fabs(len = GetHermiteSpeed(a, b, c, d, speed)) < minLen) {
			len = 1;
		}
		oldSpeed = speed;
		speed -= pathLen/len;
		step++;
	} while(speed != oldSpeed && step < 100);
	return speed;
}

int MBCircuitKettouSaiExec(int playerNo)
{
	MBSaiExec(playerNo, SAITYPE_EVENT, NULL, SAI_VALUE_ANY, TRUE, FALSE, NULL, SAI_COLOR_GREEN);
	MBSaiMotHookSet(playerNo, CircuitSaiMotHook);
	while(!MBSaiKillCheck(playerNo)) {
		HuPrcVSleep();
	}
	return MBSaiResultGet(playerNo);
}

static void CircuitSaiMotHook(int playerNo)
{
	int time;
	MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_RUN, HU3D_MOTATTR_NONE);
	time = 0;
	do {
		if(time++ == 8) {
			MBSaiObjHit(playerNo);
		}
		HuPrcVSleep();
	} while(!MBPlayerMotionEndCheck(playerNo));
	MBPlayerMotIdleSet(playerNo);
}


static void CircuitMasuLinkGet(int playerNo, CIRCUIT_CAR_MASU *curMasu, CIRCUIT_CAR_MASU *prevMasu, int dir)
{
	int i; //r31
	int j; //r30
	s16 targetMasu; //r29
	int orderNo; //r28
	int curOrder; //r27
	s16 carMasu; //r26
	s16 masuId; //r25
	int jumpPlayer; //r21
	
	jumpPlayer = 0;
	if(dir >= 0) {
		carMasu = MBMasuCarNextGet((int)curMasu->masuId);
	} else {
		carMasu = MBMasuCarPrevGet((int)curMasu->masuId);
	}
	for(i=0; i<GW_PLAYER_MAX; i++) {
		targetMasu = MBMasuCarTargetGet(carMasu, i);
		if(MBMasuFlagGet(MASU_LAYER_DEFAULT, targetMasu) & MASU_FLAG_JUMPTO) {
			jumpPlayer = i;
		}
	}
	curOrder = curMasu->orderNo;
	masuId = MASU_NULL;
	orderNo = -1;
	if(!jumpPlayer) {
		if(curOrder > 0) {
			for(i=curOrder-1; i>=0; i--) {
				targetMasu = MBMasuCarTargetGet(carMasu, i);
				for(j=0; j<GW_PLAYER_MAX; j++) {
					if(j != playerNo && targetMasu == GwPlayer[j].masuId) {
						break;
					}			
				}
				if(j >= GW_PLAYER_MAX) {
					masuId = targetMasu;
					orderNo = i;
					break;
				}
			}
		}
		if(orderNo < 0) {
			for(i=curOrder; i<GW_PLAYER_MAX; i++) {
				targetMasu = MBMasuCarTargetGet(carMasu, i);
				for(j=0; j<GW_PLAYER_MAX; j++) {
					if(j != playerNo && targetMasu == GwPlayer[j].masuId) {
						break;
					}			
				}
				if(j >= GW_PLAYER_MAX) {
					masuId = targetMasu;
					orderNo = i;
					break;
				}
			}
		}
	} else {
		if(curOrder < 3) {
			for(i=curOrder+1; i<GW_PLAYER_MAX; i++) {
				targetMasu = MBMasuCarTargetGet(carMasu, i);
				for(j=0; j<GW_PLAYER_MAX; j++) {
					if(j != playerNo && targetMasu == GwPlayer[j].masuId) {
						break;
					}			
				}
				if(j >= GW_PLAYER_MAX) {
					masuId = targetMasu;
					orderNo = i;
					break;
				}
			}
		}
		if(orderNo < 0) {
			for(i=curOrder; i>=0; i--) {
				targetMasu = MBMasuCarTargetGet(carMasu, i);
				for(j=0; j<GW_PLAYER_MAX; j++) {
					if(j != playerNo && targetMasu == GwPlayer[j].masuId) {
						break;
					}			
				}
				if(j >= GW_PLAYER_MAX) {
					masuId = targetMasu;
					orderNo = i;
					break;
				}
			}
		}
	}
	prevMasu->masuId = masuId;
	prevMasu->orderNo = orderNo;
}

static void CircuitWinSaiMotHook(int playerNo);
static u16 CircuitWinSaiBtnHook(int playerNo);

static BOOL CircuitWinExec(int *playerNo, int playerNum)
{
	int i; //r31
	int player; //r30
	int bestPlayer; //r28
	int srcIdx; //r25
	int dstIdx; //r24
	int saiMin; //r23
	MBMODELID guideMdlId; //r22
	int temp; //r21
	s16 targetMasu; //r20
	s16 masuId; //r19
	int jingleNo; //r18
	
	int saiNum[10]; //sp+0x18
	HuVecF pos; //sp+0xC
	s16 winId; //sp+0x8
	
	static const HuVecF cameraRot = { -10, -90, 0 };
	static const HuVecF guideRot = { 0, -90, 0 };
	static const HuVecF guidePos = { 0, 220, 200 };
	
	GwSystem.turnPlayerNo = -1;
	pos.x = guidePos.x;
	pos.y = 400;
	pos.z = guidePos.z;
	guideMdlId = MBGuideCreate(MB_GUIDE_NIRL, &pos, &guideRot, FALSE);
	MBCameraModelViewSet(guideMdlId, &cameraRot, NULL, 1000, -1, 1);
	MBCameraMotionWait();
	MBDataClose();
	masuId = MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, NULL);
	for(i=0; i<GW_PLAYER_MAX; i++) {
		MBPlayerRotSet(i, 0, -90, 0);
		targetMasu = MBMasuCarTargetGet(masuId, i);
		MBMasuPosGet(MASU_LAYER_DEFAULT, targetMasu, &pos);
		MBPlayerPosSetV(i, &pos);
		MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_NONE);
	}
	MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
	while(MBMusCheck(MB_MUS_CHAN_BG)) {
		HuPrcVSleep();
	}
	MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_PARTYRESULT, 127, 0);
	HuDataDirRead(DATA_board);
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
	}
	WipeWait();
	MBAudFXPlay(MSM_SE_GUIDE_56);
	MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_GAMEEND, HUWIN_SPEAKER_NONE);
	MBTopWinWait();
	MBCameraFocusSet(MB_MODEL_NONE);
	MBCameraPosViewSet(&guidePos, &cameraRot, 0, 2350, -1, 60);
	MBCameraMotionWait();
	MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_WIN1, HUWIN_SPEAKER_NONE);
	MBTopWinWait();
	switch(playerNum) {
		case 2:
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_WIN2, HUWIN_SPEAKER_NONE);
			break;
		
		case 3:
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_WIN3, HUWIN_SPEAKER_NONE);
			break;
		
		case 4:
			MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_WIN4, HUWIN_SPEAKER_NONE);
			break;
	}
	for(i=0; i<playerNum; i++) {
		MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[i]), i);
	}
	MBTopWinWait();
	if(playerNum > 1) {
		MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_SAIROLL, HUWIN_SPEAKER_NONE);
		MBTopWinWait();
		for(i=0; i<10; i++) {
			saiNum[i] = i;
		}
		for(i=0; i<100; i++) {
			srcIdx = frandmod(10);
			dstIdx = frandmod(10);
			temp = saiNum[srcIdx];
			saiNum[srcIdx] = saiNum[dstIdx];
			saiNum[dstIdx] = temp;
		}
		for(saiMin=0, i=0; i<playerNum; i++) {
			player = playerNo[i];
			if(saiNum[player] >= saiMin) {
				saiMin = saiNum[player];
				bestPlayer = player;
			}
			MBPlayerPosGet(player, &pos);
			MBSaiExec(player, SAITYPE_EVENT, NULL, saiNum[player], FALSE, FALSE, &pos, SAI_COLOR_GREEN);
			MBSaiMotHookSet(player, CircuitWinSaiMotHook);
			if(GwPlayer[player].comF) {
				saiBtnTimer[player] = frandmod(30)+30;
				MBSaiPadBtnHookSet(player, CircuitWinSaiBtnHook);
			}
		}
		MBWinCreateHelp(MESS_BOARDOPE_PAD_SAI);
		MBTopWinPosSet(228, 416);
		while(!MBSaiKillCheckAll()) {
			HuPrcVSleep();
		}
		MBTopWinKill();
		MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_FINAL, HUWIN_SPEAKER_NONE);
		MBTopWinWait();
		for(i=0; i<playerNum; i++) {
			MBSaiNumKill(playerNo[i]);
		}
	} else {
		bestPlayer = playerNo[0];
	}
	MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
	for(i=0; i<GW_PLAYER_MAX; i++) {
		if(i == bestPlayer) {
			MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_JUMP, 0, 5, HU3D_MOTATTR_NONE);
			CharFXPlay(GwPlayer[i].charNo, CHARVOICEID(4));
		} else {
			MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_JUMPEND, 0, 5, HU3D_MOTATTR_NONE);
		}
	}
	MBPlayerPosGet(bestPlayer, &pos);
	pos.y += 800;
	MBEffConfettiCreate(&pos, 100, 100);
	jingleNo = MBMusPlayJingle(MSM_STREAM_JNGL_MGCIRCUIT_WIN);
	MBAudFXPlay(MSM_SE_GUIDE_55);
	winId = MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_FINALNAME, HUWIN_SPEAKER_NONE);
	MBTopWinInsertMesSet(MBPlayerNameMesGet(bestPlayer), 0);
	while(MBMusJingleStatGet(jingleNo) != MSM_STREAM_DONE) {
		HuPrcVSleep();
	}
	MBMusPauseFadeOut(MB_MUS_CHAN_BG, FALSE, 1000);
	MBTopWinWait();
	MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_RESULT_END, HUWIN_SPEAKER_NONE);
	MBTopWinWait();
	MBWinCreateChoice(MBWIN_TYPE_EVENT, MESS_BOARD_MGCIRCUIT_REPLAY, HUWIN_SPEAKER_NONE, 0);
	MBTopWinAttrReset(HUWIN_ATTR_NOCANCEL);
	if(MBPlayerAllComCheck()) {
		MBComChoiceSetLeft();
	}
	MBTopWinWait();
	MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
	WipeColorSet(0, 0, 0);
	if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
		WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
	}
	WipeWait();
	while(MBMusCheck(MB_MUS_CHAN_BG)) {
		HuPrcVSleep();
	}
	MBEffConfettiKill();
	MBGuideEnd(guideMdlId);
	for(i=0; i<GW_PLAYER_MAX; i++) {
		if(i == bestPlayer) {
			MBPlayerCoinSet(i, 1);
		} else {
			MBPlayerCoinSet(i, 0);
		}
	}
	if(MBWinLastChoiceGet() == 0) {
		return TRUE;
	} else {
		MBPauseWatchProcStop();
		HuPrcSleep(-1);
		return FALSE;
	}
}

static u16 CircuitWinSaiBtnHook(int playerNo)
{
	if(saiBtnTimer[playerNo] != 0 && --saiBtnTimer[playerNo] == 0) {
		return PAD_BUTTON_A;
	}
	return 0;
}

static void CircuitWinSaiMotHook(int playerNo)
{
	int time;
	MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_RUN, 0, 5, HU3D_MOTATTR_NONE);
	time = 0;
	do {
		if(time++ == 8) {
			MBSaiObjHit(playerNo);
		}
		HuPrcVSleep();
	} while(!MBPlayerMotionEndCheck(playerNo));
	MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
}

void MBCircuitCarInit(void)
{
	int i;
	s16 masuId;
	s16 targetMasu;
	HuVecF masuPos;
	HuVecF targetPos;
	HuVecF dir;
	for(i=0; i<GW_PLAYER_MAX; i++) {
		masuId = MBMasuCarNextGet(GwPlayer[i].masuId);
		targetMasu = MBMasuCarTargetGet(masuId, GwPlayer[i].orderNo);
		MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[i].masuId, &masuPos);
		MBMasuPosGet(MASU_LAYER_DEFAULT, targetMasu, &targetPos);
		VECSubtract(&targetPos, &masuPos, &dir);
		MBPlayerRotYSet(i, 90-HuAtan(dir.z, dir.x));
	}
}


int MBCircuitGoalDistGet(void)
{
	int i;
	int dist;
	for(dist=0, i=0; i<GW_PLAYER_MAX; i++) {
		if(MBPlayerCoinGet(i) > dist) {
			dist = MBPlayerCoinGet(i);
		}
	}
	return 50-dist;
}

BOOL MBCircuitGoalCheck(int playerNo)
{
	if(MBPlayerCoinGet(playerNo) >= 50) {
		return TRUE;
	} else {
		return FALSE;
	}
}

typedef struct CircuitGoalSpr_s {
	unsigned killF : 1;
	s16 mode;
	s16 sprId;
	s16 time;
	s16 maxTime;
	float zoom;
} CIRCUIT_GOAL_SPR;

static void CircuitGoalSprExec(OMOBJ *obj);

static void CircuitGoalSprCreate(void)
{
	CIRCUIT_GOAL_SPR *work;
	circuitGoalSprObj = MBAddObj(262, 1, 0, CircuitGoalSprExec);
	work = omObjGetWork(circuitGoalSprObj, CIRCUIT_GOAL_SPR);
	work->sprId = espEntry(W20_ANM_goal, 100, 0);
	espPosSet(work->sprId, 288, 240);
	work->killF = FALSE;
	work->mode = 0;
	work->time = 0;
	work->maxTime = 30;
	work->zoom = MBCameraZoomGet();

}

static void CircuitGoalSprExec(OMOBJ *obj)
{
	CIRCUIT_GOAL_SPR *work;
	float t;
	float scale;
	float zoom;
	work = omObjGetWork(obj, CIRCUIT_GOAL_SPR);
	if(work->killF || MBKillCheck()) {
		espKill(work->sprId);
		circuitGoalSprObj = NULL;
		MBDelObj(obj);
		return;
	}
	switch(work->mode) {
		case 0:
			if(work->time > work->maxTime) {
				work->mode = 1;
				work->time = 30;
			} else {
				t = (float)work->time++/work->maxTime;
				espScaleSet(work->sprId, t, t);
				espTPLvlSet(work->sprId, t);
				zoom = (t*(1200.0f-work->zoom))+work->zoom;
				MBCameraZoomSet(zoom);
			}
			break;
		
		case 1:
			if(--work->time == 0) {
				work->mode = 2;
				work->time = 0;
				work->maxTime = 30;
			}
			break;
		
		case 2:
			if(work->time > work->maxTime) {
				work->mode = 3;
				work->time = 60;
			} else {
				t = (float)work->time++/work->maxTime;
				scale = 1+(t*3);
				espScaleSet(work->sprId, scale, scale);
				espTPLvlSet(work->sprId, 1-t);
			}
			break;
		
		case 3:
			if(--work->time == 0) {
				work->killF = TRUE;
			}
			break;
	}
}

static BOOL CircuitGoalSprCheck(void)
{
	return circuitGoalSprObj == NULL;
}

static const int charDataNum[] = {
	DATA_mariomdl1,
	DATA_luigimdl1,
	DATA_peachmdl1,
	DATA_yoshimdl1,
	DATA_wariomdl1,
	DATA_daisymdl1,
	DATA_waluigimdl1,
	DATA_kinopiomdl1,
	DATA_teresamdl1,
	DATA_minikoopamdl1,
	DATA_minikoopamdl1,
	DATA_minikoopamdl1,
	DATA_minikoopamdl1,
	DATA_mariomot,
	DATA_luigimot,
	DATA_peachmot,
	DATA_yoshimot,
	DATA_wariomot,
	DATA_daisymot,
	DATA_waluigimot,
	DATA_kinopiomot,
	DATA_teresamot,
	DATA_minikoopamot,
	DATA_minikoopamot,
	DATA_minikoopamot,
	DATA_minikoopamot,
};