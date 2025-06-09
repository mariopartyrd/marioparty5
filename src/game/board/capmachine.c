#include "game/esprite.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/com.h"
#include "game/board/masu.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/pad.h"
#include "game/board/pause.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/board/audio.h"
#include "datanum/w10.h"
#include "datanum/effect.h"
#include "messnum/board_tutorial.h"
#include "messnum/boardope.h"
#include "messnum/board_capmachine.h"

#define CAPMACHINE_MAX 8

#define CAPMACHINE_CAPSULE_MAX 16

typedef struct CapMachineCapsule_s {
    HuVecF pos;
    HuVecF ofs;
    HuVecF rot;
    int unk24;
    int objId;
} CAPMACHINE_CAPSULE;

typedef struct CapMachineWork_s {
    BOOL kakimazeRotF;
    MBMODELID modelId[3];
    HuVecF pos; 
    int masuId;
    float rotY;
    CAPMACHINE_CAPSULE *capsule;
    float kakimazeRotY;
    int kakimazeTime;
} CAPMACHINEWORK;  

static CAPMACHINEWORK capMachineWork[CAPMACHINE_MAX];
static HUPROCESS *capsuleSelProc[GW_PLAYER_MAX] = { };
static HUPROCESS *capMachineProc[GW_PLAYER_MAX] = { };

static s8 capsuleSelResult[GW_PLAYER_MAX];
static s32 capMachineNum;
static s32 capsuleCurNo;

static HU3DPARMANPARAM storyEffParmanParam = {
    0x1E,
    0x108,
    50.0f,
    0.0f,
    65.0f,
    { 0.0f, 0.0f, 0.0f },
    15.0f,
    0.9f,
    60.0f,
    0.95f,
    1,
    { { 0xFF, 0xFF, 0xFF, 0xFF }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { { 0xFF, 0xFF, 0xFF, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
};


static HuVecF bouPosTbl[3] = {
    { 0, -69.3, -70.7 },
    { -61.2, -69.3, 35.3 },
    { 61.2, -69.3, 35.3 },
};

static HuVecF capMachineOfs = { 0.0f, -140.0f, 0.0f };

static void CapsuleSelExec(void);
static void CapsuleSelKill(void);

typedef struct CapsuleSelWork_s {
    int selNum;
    int selNo;
    int result;
    int playerNo;
    int winNo;
    s16 winId[3];
    MBMODELID modelId[5];
    s16 sprId[1];
    s16 scaleTimer;
    s16 comBtnDelay;
    float scale[5];
} CAPSULESELWORK;

int MBCapsuleSelExec(void)
{
    int playerNo = GwSystem.turnPlayerNo;
    CAPSULESELWORK *work;
    MBPauseDisableSet(TRUE);
    repeat:
    capsuleSelProc[playerNo] = MBPrcCreate(CapsuleSelExec, 8197, 16384);
    MBPrcDestructorSet(capsuleSelProc[playerNo], CapsuleSelKill);
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULESELWORK), HU_MEMNUM_OVL);
    capsuleSelProc[playerNo]->property = work;
    memset(work, 0, sizeof(CAPSULESELWORK));
    work->playerNo = playerNo;
    capsuleSelResult[playerNo] = SAI_RESULT_NULL;
    while(capsuleSelProc[playerNo] != NULL) {
        HuPrcVSleep();
    }
    if(capsuleSelResult[playerNo] >= 0) {
        if(!MBCapsuleUseExec(playerNo, capsuleSelResult[playerNo])) {
            goto repeat;
        }
        MBPlayerStoryLoseExec();
        capsuleSelResult[playerNo] = SAI_RESULT_CAPSULESKIP;
        if(_CheckFlag(FLAG_BOARD_STORYEND)) {
            return capsuleSelResult[playerNo];
        }
        if(MBPlayerBlackoutGet()) {
            MBCameraSkipSet(FALSE);
            MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
            MBCameraMotionWait();
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            MBPlayerBlackoutSet(FALSE);
        }
    }
    return capsuleSelResult[playerNo];
}

static void CapsuleSelPadUpdate(CAPSULESELWORK *work);

static void CapsuleSelExec(void)
{
    HuVecF cursorPos[5];
    HuVecF modelPos[5];
    HuVecF cursorMdlPos;
    
    HuVecF playerPos;
    HuVec2F winPos;
    
    s16 temp = -1;
    CAPSULESELWORK *work = HuPrcCurrentGet()->property;
    int i;
    int playerNo = work->playerNo;
    int j;
    s16 selNo;
    s16 capsuleNo;
    s16 capsuleNext;
    
    float weight;
    float scale;
    float cursorX;
    float cursorY;
    
    work->selNo = selNo = 0;
    for(i=0; i<3; i++) {
        work->winId[i] = MBWIN_NONE;
    }
    if(GwPlayerConf[work->playerNo].type != GW_TYPE_MAN) {
        work->result = MBComCapsuleSelGet(work->playerNo);
        work->comBtnDelay = 30;
    } else {
        work->result = SAI_RESULT_FAIL2;
    }
    capsuleNo = MBPlayerCapsuleGet(work->playerNo, work->selNo);
    MBStatusDispSetAll(FALSE);
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MBStatusDispFocusSet(work->playerNo, TRUE);
    MBPlayerPosGet(work->playerNo, &playerPos);
    playerPos.y += 100;
    work->selNum = MBPlayerCapsuleNumGet(work->playerNo);
    for(i=0; i<5; i++) {
        work->modelId[i] = MB_MODEL_NONE;
    }
    for(i=0; i<work->selNum; i++) {
        work->modelId[i] = MBModelCreate(MBCapsuleMdlGet(MBPlayerCapsuleGet(work->playerNo, i)), NULL, FALSE);
        MBModelLayerSet(work->modelId[i], 3);
        MBMotionNoSet(work->modelId[i], 0, HU3D_MOTATTR_PAUSE);
        modelPos[i].x = (150.0f*i)+(playerPos.x-(75.0f*(work->selNum-1)));
        modelPos[i].y = 150.0f+playerPos.y;
        modelPos[i].z = playerPos.z;
    }
    work->sprId[0] = espEntry(BOARD_ANM_cursor, 1400, 0);
    espDispOff(work->sprId[0]);
    espDrawNoSet(work->sprId[0], 32);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        work->winId[0] = MBWinCreateHelp(MESS_BOARDOPE_PAD_CAPSULE_SEL);
        MBWinPosSet(work->winId[0], 160, 288);
    }
    MBAudFXPlay(MSM_SE_BOARD_21);
    for(i=0; i<=12u; i++) {
        weight = i/12.0f;
        for(j=0; j<work->selNum; j++) {
            work->scale[j] = MBCapsuleSelModelUpdate(&playerPos, &modelPos[j], work->modelId[j], weight);
        }
        HuPrcVSleep();
    }
    for(i=0; i<work->selNum; i++) {
        MBModelPosGet(work->modelId[i], &cursorMdlPos);
        Hu3D3Dto2D(&cursorMdlPos, HU3D_CAM0, &cursorPos[i]);
        cursorPos[i].x -= 32;
        cursorPos[i].y -= 32;
    }
    espPosSet(work->sprId[0], cursorPos[work->selNo].x , cursorPos[work->selNo].y);
    espDispOn(work->sprId[0]);
    work->winId[1] = MBCapsuleWinCreate(capsuleNo);
    MBWinPosGet(work->winId[1], &winPos);
    work->winNo = 1;
    MBPauseDisableSet(FALSE);
    while(1) {
        if(GwPlayerConf[work->playerNo].type != GW_TYPE_MAN && work->result == SAI_RESULT_FAIL2) {
            work->result = MBComCapsuleSelGet(work->playerNo);
            work->comBtnDelay = 30;
        }
        CapsuleSelPadUpdate(work);
        if(capsuleSelResult[playerNo] == SAI_RESULT_FAIL2 ||
            capsuleSelResult[playerNo] == SAI_RESULT_FAIL1 ||
            capsuleSelResult[playerNo] == SAI_RESULT_SCROLL ||
            capsuleSelResult[playerNo] == SAI_RESULT_FAIL4 ||
            capsuleSelResult[playerNo] == SAI_RESULT_CAPSULESKIP) {
            break;
        }
        if(capsuleSelResult[playerNo] == SAI_RESULT_SAIKORO) {
            capsuleSelResult[playerNo] = MBPlayerCapsuleGet(work->playerNo, work->selNo);
            break;
        }
        capsuleNext = MBPlayerCapsuleGet(work->playerNo, work->selNo);
        if(work->selNo != selNo || capsuleNext != capsuleNo) {
            MBModelRotSet(work->modelId[selNo], 0, 0, 0);
            scale = work->scale[selNo];
            MBModelScaleSet(work->modelId[selNo], scale, scale, scale);
            MBMotionTimeSet(work->modelId[selNo], 0);
            MBMotionNoSet(work->modelId[selNo], 0, HU3D_MOTATTR_PAUSE);
            work->winId[work->winNo+1] = MBCapsuleWinCreate(capsuleNext);
            for(i=0; i<=12u; i++) {
                weight = i/12.0f;
                cursorX = cursorPos[selNo].x+((cursorPos[work->selNo].x-cursorPos[selNo].x)*HuSin(weight*90));
                cursorY = cursorPos[selNo].y+((cursorPos[work->selNo].y-cursorPos[selNo].y)*HuSin(weight*90));
                espPosSet(work->sprId[0], cursorX, cursorY);
                if(selNo != work->selNo) {
                    if(work->selNo > selNo) {
                        MBWinPosSet(work->winId[(work->winNo^1)+1], winPos.x-(576*weight), winPos.y);
                        MBWinPosSet(work->winId[work->winNo+1], (576+winPos.x)-(576*weight), winPos.y);
                    } else {
                        MBWinPosSet(work->winId[(work->winNo^1)+1], winPos.x+(576*weight), winPos.y);
                        MBWinPosSet(work->winId[work->winNo+1], (winPos.x-576)+(576*weight), winPos.y);
                    }
                }
                HuPrcVSleep();
            }
            MBWinKill(work->winId[(work->winNo^1)+1]);
            work->winId[(work->winNo^1)+1] = MBWIN_NONE;
            work->winNo ^= 1;
            work->scaleTimer = 0;
            selNo = work->selNo;
            capsuleNo = capsuleNext;
        }
        MBModelRotSet(work->modelId[work->selNo], 0, 2.0f*work->scaleTimer, 0);
        scale = work->scale[work->selNo]*(1+(0.2f*fabs(HuSin((work->scaleTimer*90.0f)/12.0f))));
        MBModelScaleSet(work->modelId[work->selNo], scale, scale, scale);
        work->scaleTimer++;
        HuPrcVSleep();
    }
    MBPauseDisableSet(TRUE);
    espDispOff(work->sprId[0]);
    MBAudFXPlay(MSM_SE_BOARD_22);
    for(i=0; i<=9u; i++) {
        weight = i/9.0f;
        for(j=0; j<work->selNum; j++) {
            work->scale[j] = MBCapsuleSelModelUpdate(&playerPos, &modelPos[j], work->modelId[j], 1-weight);
        }
        HuPrcVSleep();
    }
    for(i=0; i<3; i++) {
        if(work->winId[i] >= 0) {
            MBWinKill(work->winId[i]);
            work->winId[i] = MBWIN_NONE;
        }
    }
    for(i=0; i<work->selNum; i++) {
        if(work->modelId[i] >= 0) {
            MBModelKill(work->modelId[i]);
            work->modelId[i] = MB_MODEL_NONE;
        }
    }
    for(i=0; i<1; i++) {
        if(work->sprId[i] >= 0) {
            espKill(work->sprId[i]);
            work->sprId[i] = -1;
        }
    }
    MBStatusDispFocusSet(work->playerNo, FALSE);
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

static void CapsuleSelKill(void)
{
    CAPSULESELWORK *work = HuPrcCurrentGet()->property;
    capsuleSelProc[work->playerNo] = NULL;
    HuMemDirectFree(work);
}

float MBCapsuleSelModelUpdate(HuVecF *posStart, HuVecF *posEnd, MBMODELID modelId, float weight)
{
    float scale = HuSin(weight*90);
    float z;
    HuVecF pos3D;
    HuVecF pos2D;
    pos3D.y = posStart->y+(scale*(posEnd->y-posStart->y));
    
    pos3D.x = posStart->x+(weight*(posEnd->x-posStart->x));
    pos3D.z = posStart->z+(weight*(posEnd->z-posStart->z));
    MB3Dto2D(&pos3D, &pos2D);
    z = pos2D.z;
    pos2D.z = 1000;
    MB2Dto3D(&pos2D, &pos3D);
    scale = (1000/z)*weight;
    MBModelPosSetV(modelId, &pos3D);
    MBModelScaleSet(modelId, scale, scale, scale);
    return scale;
}

static u16 CapsuleSelPadRead(CAPSULESELWORK *work)
{
    u16 btn;
    
    if(GwPlayerConf[work->playerNo].type == GW_TYPE_MAN) {
        u16 padNo = GwPlayer[work->playerNo].padNo;
        btn = HuPadBtnDown[padNo];
        if(MBPadStkXGet(padNo) < -20) {
            btn |= PAD_BUTTON_LEFT;
        } else if(MBPadStkXGet(padNo) > 20) {
            btn |= PAD_BUTTON_RIGHT;
        }
    } else {
        btn = 0;
        if(--work->comBtnDelay == 0) {
            if(work->result == SAI_RESULT_FAIL1) {
                btn |= PAD_BUTTON_B;
            } else if(work->result == work->selNo) {
                btn |= PAD_BUTTON_A;
            } else if(work->selNo < work->result) {
                btn |= PAD_BUTTON_RIGHT;
            } else {
                btn |= PAD_BUTTON_LEFT;
            }
            work->comBtnDelay = 20;
        }
    }
    return btn;
}

static void CapsuleSelPadUpdate(CAPSULESELWORK *work)
{
    int playerNo = work->playerNo;
    u16 btn = CapsuleSelPadRead(work);
    if(MBPauseProcCheck()) {
        btn = 0;
        return;
    }
    if(btn & PAD_BUTTON_LEFT && work->selNo > 0) {
        MBAudFXPlay(MSM_SE_CMN_01);
        work->selNo--;
    }
    if(btn & PAD_BUTTON_RIGHT && work->selNo < work->selNum-1) {
        MBAudFXPlay(MSM_SE_CMN_01);
        work->selNo++;
    }
    if(btn & PAD_BUTTON_A) {
        capsuleSelResult[playerNo] = SAI_RESULT_SAIKORO;
        MBAudFXPlay(MSM_SE_CMN_02);
    } else if(btn & PAD_BUTTON_Y) {
        capsuleSelResult[playerNo] = SAI_RESULT_SCROLL;
        MBAudFXPlay(MSM_SE_CMN_02);
    } else if(btn & PAD_BUTTON_B) {
        capsuleSelResult[playerNo] = SAI_RESULT_CAPSULESKIP;
        MBAudFXPlay(MSM_SE_CMN_04);
        return;
    }
}

static void StoryComCapsuleSelExec(void);

void MBCapsuleSelStoryExec(void)
{
    int i;
    int num;
    CAPSULESELWORK *work;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
            capsuleSelResult[i] = SAI_RESULT_NULL;
            if(MBPlayerCapsuleNumGet(i)) {
                capsuleSelProc[i] = MBPrcCreate(StoryComCapsuleSelExec, 8197, 16384);
                MBPrcDestructorSet(capsuleSelProc[i], CapsuleSelKill);
                capsuleSelProc[i]->property = work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULESELWORK), HU_MEMNUM_OVL);
                memset(work, 0, sizeof(CAPSULESELWORK));
                work->playerNo = i;
            }
        }
    }
    while(1) {
        num = 0;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i) && capsuleSelProc[i] != NULL) {
                num++;
            }
        }
        HuPrcVSleep();
        if(num == 0) {
            break;
        }
    }
}

void MBCapsuleAddStory(void)
{
    BOOL useF = FALSE;
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
            GwSystem.turnPlayerNo = i;
            if(capsuleSelResult[i] >= 0 && GwPlayer[i].capsuleUse == CAPSULE_NULL && MBPlayerCapsuleFind(i, capsuleSelResult[i]) >= 0) {
                if(!MBPlayerBlackoutGet()) {
                    WipeColorSet(255, 255, 255);
                    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
                        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
                    }
                    WipeWait();
                }
                MBPlayerPosFixPlayer(i, GwPlayer[i].masuId, TRUE);
                MBPlayerPosReset(i);
                MBPlayerPosFixOrderSet();
                MBCameraSkipSet(FALSE);
                MBCameraViewNoSet(i, MB_CAMERA_VIEW_ZOOMIN);
                MBCameraMotionWait();
                MBCameraSkipSet(TRUE);
                useF = TRUE;
                if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
                }
                WipeWait();
                MBPlayerBlackoutSet(FALSE);
                HuPrcSleep(30);
                MBCapsuleUseExec(i, capsuleSelResult[i]);
                MBPlayerStoryLoseExec();
                if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                    return;
                }
            }
        }
    }
    if(!useF) {
        return;
    }
    if(!MBPlayerBlackoutGet()) {
        WipeColorSet(255, 255, 255);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        }
        WipeWait();
    }
    MBCameraSkipSet(FALSE);
    MBCameraFocusPlayerAddAll();
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    }
    WipeWait();
    MBPlayerBlackoutSet(FALSE);
}

static void StoryComCapsuleSelExec(void)
{
    CAPSULESELWORK *work = HuPrcCurrentGet()->property;
    int playerNo = work->playerNo;
    work->selNo = 0;
    work->selNum = MBPlayerCapsuleNumGet(playerNo);
    work->result = MBComCapsuleSelGet(playerNo);
    work->comBtnDelay = 30;
    while(1) {
        CapsuleSelPadUpdate(work);
        MBStatusCapsuleFocusSet(playerNo, work->selNo);
        if(capsuleSelResult[playerNo] == SAI_RESULT_FAIL2 ||
            capsuleSelResult[playerNo] == SAI_RESULT_FAIL1 ||
            capsuleSelResult[playerNo] == SAI_RESULT_SCROLL ||
            capsuleSelResult[playerNo] == SAI_RESULT_FAIL4 ||
            capsuleSelResult[playerNo] == SAI_RESULT_CAPSULESKIP) {
            break;
        }
        if(capsuleSelResult[playerNo] == SAI_RESULT_SAIKORO) {
            capsuleSelResult[playerNo] = MBPlayerCapsuleGet(playerNo, work->selNo);
            break;
        }
        HuPrcVSleep();
    }
    MBStatusCapsuleFocusSet(playerNo, -1);
    HuPrcEnd();
}

static void CapMachineExec(void); 
static void CapMachineStoryExec(void);
static void CapMachineDestroy(void);

void MBCapMachineExec(int playerNo) {
    int *work;

    if (GWPartyFGet() != FALSE || !MBPlayerStoryComCheck(playerNo) || GwSystem.turnNo != GwSystem.turnMax) {
        if (MBPlayerCapsuleNumGet(playerNo) < MBPlayerCapsuleMaxGet()) {
            MBWalkNumDispSet(playerNo, 0);
            if (GWPartyFGet() == FALSE  &&  MBPlayerStoryComCheck(playerNo) ) {
                capMachineProc[playerNo] = MBPrcCreate(CapMachineStoryExec, 0x2003, 0x4000);
            } else {
                capMachineProc[playerNo] = MBPrcCreate(CapMachineExec, 0x2003, 0x4000);
            }
            work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(int), HU_MEMNUM_OVL);
            capMachineProc[playerNo]->property = work;
            MBPrcDestructorSet(capMachineProc[playerNo], CapMachineDestroy);
            *work = playerNo;
            while (capMachineProc[playerNo]) {
                HuPrcVSleep();
            }
            MBWalkNumDispSet(playerNo, 1);
        }
    }
}

static HU3DPARMANID CreateStoryEff(void);
static void CapMachineKakimazeStart(CAPMACHINEWORK *work);
static BOOL CapMachineKakimazeCheck(CAPMACHINEWORK *work);

static void CapMachineExec(void)
{
    int *procWork = HuPrcCurrentGet()->property;
    char colorMes[3] = {};
    int playerNo = *procWork;
    int masuId = GwPlayer[playerNo].masuId;
    CAPMACHINEWORK *work;
    int i;
    s16 capsuleObjId;
    s16 winId;
    
    char str[256];
    HuVecF pos;
    HuVecF capsulePos;
    HuVecF playerPos;
    HuVec2F winPos;
    HuVec2F winCenterPos;
    float weight;
    float rotY;
    
    for(work=&capMachineWork[0], i=0; i<capMachineNum; i++, work++) {
        if(work->masuId == masuId) {
            break;
        }
    }
    MBPlayerMotIdleSet(playerNo);
    if(MBPlayerCapsuleNumGet(playerNo) >= MBPlayerCapsuleMaxGet()) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_CAPMACHINE_FULL, HUWIN_SPEAKER_NONE);
        sprintf(str, "%d", MBPlayerCapsuleMaxGet());
        MBTopWinInsertMesSet(MESSNUM_PTR(str), 0);
        MBTopWinWait();
        MBTopWinKill();
        HuPrcEnd();
    }
    if(GwSystem.turnNo == GwSystem.turnMax) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_CAPMACHINE_LASTTURN, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        HuPrcEnd();
    }
    MBWinCreateChoice(MBWIN_TYPE_EVENT, MESS_BOARD_CAPMACHINE_CHOICE, HUWIN_SPEAKER_NONE, 0);
    MBTopWinAttrReset(HUWIN_ATTR_NOCANCEL);
    if(GwPlayer[playerNo].comF) {
        MBComChoiceSetLeft();
    }
    MBTopWinWait();
    if(MBWinLastChoiceGet() == 1 ||  MBWinLastChoiceGet() == -1) {
        HuPrcEnd();
    }
    MBStatusDispSetAll(FALSE);
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MBStatusDispFocusSet(playerNo, TRUE);
    MBMasuPosGet(MASU_LAYER_DEFAULT, MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, masuId, MASU_FLAG_BLOCKL), &pos);
    MBPlayerWalkExec(playerNo, NULL, &pos, 12, NULL, TRUE);
    MBPlayerMotIdleSet(playerNo);
    MBCameraSkipSet(TRUE);
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MBCameraMotionWait();
    MBCameraFocusSet(MB_MODEL_NONE);
    rotY = MBPlayerRotYGet(playerNo);
    MBPlayerRotateStart(playerNo, rotY, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBMotionShapeTimeSet(work->modelId[0], 0.0);
    MBModelAttrSet(work->modelId[0], HU3D_MOTATTR_SHAPE_LOOP);
    MBModelAttrReset(work->modelId[0], HU3D_MOTATTR_SHAPE_PAUSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_CAPMACHINE_DECIDE, HUWIN_SPEAKER_CAPMACHINE);
    MBAudFXPlay(MSM_SE_BOARD_17);
    MBTopWinWait();
    MBTopWinKill();
    MBModelAttrReset(work->modelId[0], HU3D_MOTATTR_SHAPE_LOOP);
    CapMachineKakimazeStart(work);
    while(!CapMachineKakimazeCheck(work)) {
        HuPrcVSleep();
    }
    MBModelDispSet(work->modelId[0], FALSE);
    MBModelDispSet(work->modelId[1], TRUE);
    MBMotionShapeTimeSet(work->modelId[1], 0.0);
    MBModelAttrReset(work->modelId[1], HU3D_MOTATTR_SHAPE_PAUSE);
    HuPrcSleep(30);
    if(!GWTeamFGet()) {
        capsuleCurNo = MBCapsuleNextGet(GwPlayer[playerNo].rank);
    } else {
        capsuleCurNo = MBCapsuleNextGet(MBPlayerGrpRankGet(playerNo));
    }
    capsuleObjId = MBCapsuleObjCreate(capsuleCurNo, FALSE);
    MBCapsuleObjScaleSet(capsuleObjId, 0.5f, 0.5f, 0.5f);
    MBCapsuleObjAttrSet(capsuleObjId, HU3D_ATTR_ZWRITE_OFF, HU3D_ATTR_ZWRITE_OFF);
    MBCapsuleObjLayerSet(capsuleObjId, 3);
    MBPlayerPosGet(playerNo, &playerPos);
    capsulePos.x = work->pos.x+(100*(0.9f*HuSin(work->rotY)));
    capsulePos.y = work->pos.y;
    capsulePos.z = work->pos.z+(100*(0.9f*HuCos(work->rotY)));
    MBAudFXPlay(MSM_SE_BOARD_19);
    for(i=0; i<32; i++) {
        weight = i/32.0f;
        pos.x = capsulePos.x+(weight*(playerPos.x-capsulePos.x));
        pos.y = (20+(capsulePos.y+(weight*(playerPos.y-capsulePos.y))))+(100*(0.3f*fabs(HuCos((weight*315)-45))));
        pos.z = capsulePos.z+(weight*(playerPos.z-capsulePos.z));
        if(i == 12) {
            MBAudFXPlay(MSM_SE_BOARD_16);
        }
        MBCapsuleObjPosSetV(capsuleObjId, &pos);
        HuPrcVSleep();
    }
    MBCapsuleObjKill(capsuleObjId);
    while(!MBMotionShapeEndCheck(work->modelId[1])) {
        HuPrcVSleep();
    }
    MBModelDispSet(work->modelId[0], TRUE);
    MBModelDispSet(work->modelId[1], FALSE);
    MBModelAttrSet(work->modelId[0], HU3D_MOTATTR_SHAPE_PAUSE);
    MBMotionShapeTimeSet(work->modelId[0], 0.0);
    omVibrate(playerNo, 12, 4, 2);
    if(capsuleCurNo == CAPSULE_KOOPA) {
        MBCapsuleKupaGetExec(playerNo);
    } else {
        MBPlayerCapsuleAdd(playerNo, capsuleCurNo);
        if(capsuleCurNo == CAPSULE_MIRACLE) {
            MB3MiracleGetExec(playerNo);
        }
        MBPlayerRotateStart(playerNo, 0, 15);
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINWIN, CHARVOICEID(2));
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINWIN, 0, 4, HU3D_MOTATTR_NONE);
        winId = MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_CAPMACHINE_GETCAPSULE, HUWIN_SPEAKER_NONE);
        MBTopWinPosGet(&winPos);
        MBTopWinSizeSet(512, 64);
        colorMes[0] = 0x1E;
        colorMes[1] = 0x05;
        strcpy(str, colorMes);
        strcat(str, HuWinMesPtrGet(MBCapsuleMesGet(capsuleCurNo)));
        colorMes[1] = 0x08;
        strcat(str, colorMes);
        MBTopWinInsertMesSet(MESSNUM_PTR(str), 0);
        MBWinCenterGet(winId, &winCenterPos);
        MBTopWinPosSet(winCenterPos.x, winPos.y);
        MBTopWinWait();
        MBPlayerMotionEndWait(playerNo);
    }
    if(MBPlayerCapsuleNumGet(playerNo) >= MBPlayerCapsuleMaxGet()) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_CAPMACHINE_EVENT_FULL, HUWIN_SPEAKER_CAPMACHINE);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBModelAttrSet(work->modelId[0], HU3D_MOTATTR_SHAPE_PAUSE);
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
    MBPlayerWalkExec(playerNo, NULL, &pos, 12, NULL, TRUE);
    MBPlayerMotIdleSet(playerNo);
    MBStatusDispFocusSet(playerNo, FALSE);
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MBStatusDispSetAll(TRUE);
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_WALK);
    MBCameraFocusPlayerSet(playerNo);
    MBCameraMotionWait();
    work->kakimazeRotF = FALSE;
    HuPrcEnd();
}

static void CapMachineStoryExec(void)
{
    int *procWork = HuPrcCurrentGet()->property;
    int playerNo = *procWork;
    int i;
    MBMODELID modelId;
    
    int capsuleNo;
    HU3DPARMANID parManId;
    HuVecF pos;
    
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
            MBPlayerMotIdleSet(i);
        }
    }
    capsuleNo = MBCapsuleNextGet(GwPlayer[playerNo].rank);
    if(capsuleNo == CAPSULE_KOOPA) {
        MBCapsuleKupaGetExec(playerNo);
    } else {
        MBPlayerPosGet(playerNo, &pos);
        parManId = CreateStoryEff();
        Hu3DParManPosSet(parManId, pos.x, pos.y+300, pos.z);
        HuPrcSleep(12);
        modelId = MBModelCreate(MBCapsuleMdlGet(capsuleNo), NULL, FALSE);
        MBModelLayerSet(modelId, 3);
        MBMotionNoSet(modelId, 0, HU3D_MOTATTR_PAUSE);
        for(i=0; i<=30u; i++) {
            float weight = i/30.0f;
            float scale;
            MBModelPosSet(modelId, pos.x, (pos.y+100)+(100*(2*(1-weight))), pos.z);
            scale = HuCos(weight*90);
            MBModelScaleSet(modelId, scale, scale, scale);
            HuPrcVSleep();
        }
        Hu3DParManKill(parManId);
        MBModelKill(modelId);
        MBPlayerCapsuleAdd(playerNo, capsuleNo);
        if(capsuleNo == CAPSULE_MIRACLE) {
            MB3MiracleGetExec(playerNo);
        }
    }
    HuPrcEnd();
}

static HU3DPARMANID CreateStoryEff(void)
{
    int parManId;
    int mdlId;
    ANIMDATA *animP;

    animP = HuSprAnimDataRead(EFFECT_ANM_smoke);
    HuDataDirClose(DATA_effect);
    parManId = Hu3DParManCreate(animP, 50, &storyEffParmanParam);
    Hu3DParManTimeLimitSet(parManId, 30);
    mdlId = Hu3DParManModelIDGet(parManId);
    Hu3DModelLayerSet(mdlId, 3);
    Hu3DModelCameraSet(mdlId, HU3D_CAM0);
    return parManId;
}


static void CapMachineDestroy(void)
{
    int *playerNo = HuPrcCurrentGet()->property;
    capMachineProc[*playerNo] = NULL;
    HuMemDirectFree(playerNo);
}

void MBCapMachineInit(void)
{
    capMachineNum = 0;
}

void MBCapMachineClose(void)
{
    s32 i;
    CAPMACHINEWORK *work;

    for (i = 0; i < capMachineNum; i++) {
        work = &capMachineWork[i];
        HuMemDirectFree(work->capsule);
        work->capsule = NULL;
    }
}

void MBCapMachineCreateAll(void) {
    MASU *masuP;
    s32 i;
    for (i = 0; i < MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, i + 1);
        if (masuP->flag & MASU_FLAG_CAPMACHINE) {
            MBCapMachineObjCreate(i + 1);
        }
    }
}

static void CapMachineKakimazeMove(CAPMACHINEWORK *work);

static void CapMachineObjMain(OMOBJ *obj);

void MBCapMachineObjCreate(int masuId)
{
    int dataNum[2] = {BOARD_HSF_capMachineMouth, HU_DATANUM_NONE };
    CAPMACHINEWORK *work = &capMachineWork[capMachineNum++];
    CAPMACHINE_CAPSULE *capsule;
    int i;
    OMOBJ *obj;
    int linkMasuId;
    
    HuVecF masuPos;
    HuVecF dir;
    
    int capsuleTbl[CAPSULE_MAX];
    int capsuleNum;
    float radius;
    
    work->modelId[0] = MBModelCreate(BOARD_HSF_capMachine, NULL, FALSE);
    MBModelAttrSet(work->modelId[0], HU3D_MOTATTR_SHAPE_PAUSE);
    MBModelLayerSet(work->modelId[0], 2);
    work->modelId[1] = MBModelCreate(BOARD_HSF_capMachineMouth, NULL, FALSE);
    MBModelAttrSet(work->modelId[1], HU3D_MOTATTR_SHAPE_PAUSE);
    MBModelDispSet(work->modelId[1], FALSE);
    MBModelLayerSet(work->modelId[1], 2);
    work->modelId[2] = MBModelCreate(BOARD_HSF_capMachineKakimaze, NULL, TRUE);
    MBModelAttrSet(work->modelId[2], HU3D_MOTATTR_PAUSE);
    linkMasuId = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, masuId, MASU_FLAG_BLOCKR);
    MBMasuPosGet(MASU_LAYER_DEFAULT, linkMasuId, &work->pos);
    MBModelPosSetV(work->modelId[0], &work->pos);
    MBModelPosSetV(work->modelId[1], &work->pos);
    MBModelPosSetV(work->modelId[2], &work->pos);
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
    VECSubtract(&masuPos, &work->pos, &dir);
    work->rotY = HuAtan(dir.x, dir.z);
    MBModelRotYSet(work->modelId[0], work->rotY);
    MBModelRotYSet(work->modelId[1], work->rotY);
    MBModelRotYSet(work->modelId[2], work->rotY);
    work->masuId = masuId;
    work->kakimazeRotY = 0;
    work->kakimazeRotF = FALSE;
    capsuleNum = MBCapsuleListGet(capsuleTbl);
    capsule = HuMemDirectMallocNum(HEAP_HEAP, CAPMACHINE_CAPSULE_MAX*sizeof(CAPMACHINE_CAPSULE), HU_MEMNUM_OVL);
    work->capsule = capsule;
    memset(capsule, 0, CAPMACHINE_CAPSULE_MAX*sizeof(CAPMACHINE_CAPSULE));
    for(i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
        dir.x = (2*frandf())-1;
        dir.y = (2*frandf())-1;
        dir.z = (2*frandf())-1;
        if(VECMag(&dir) == 0) {
            dir.y = 1;
        } else {
            VECNormalize(&dir, &dir);
        }
        radius = frandf()*90.00001f;
        capsule->pos.x = dir.x*radius;
        capsule->pos.y = dir.y*radius;
        capsule->pos.z = dir.z*radius;
        capsule->rot.x = frandf()*360;
        capsule->rot.y = frandf()*360;
        capsule->rot.z = frandf()*360;
        capsule->objId = MBCapsuleObjCreate(capsuleTbl[frandmod(capsuleNum)], TRUE);
        
        MBCapsuleObjAttrSet(capsule->objId, HU3D_ATTR_ZWRITE_OFF, HU3D_ATTR_ZWRITE_OFF);
        MBCapsuleObjScaleSet(capsule->objId, 0.6f, 0.6f, 0.6f);
    }
    for(i=0; i<500; i++) {
        CapMachineKakimazeMove(work);
    }
    for(capsule=work->capsule, i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
        MBCapsuleObjPosSet(capsule->objId, work->pos.x+capsule->pos.x, 245+work->pos.y+capsule->pos.y, work->pos.z+capsule->pos.z);
        MBCapsuleObjRotSetV(capsule->objId, &capsule->rot);
    }
    obj = MBAddObj(260, 0, 0, CapMachineObjMain);
    obj->work[0] = (u32)work;
    
}

static void CapMachineObjMain(OMOBJ *obj)
{
    CAPMACHINEWORK *work = (CAPMACHINEWORK *)obj->work[0];
    CAPMACHINE_CAPSULE *capsule;
    int i;
    if(MBKillCheck()) {
        for(i=0; i<3; i++){
            if(work->modelId[i] >= 0) {
                MBModelKill(work->modelId[i]);
            }
        }
        for(capsule=work->capsule, i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
            MBCapsuleObjKill(capsule->objId);
        }
        MBDelObj(obj);
        return;
    }
    if(work->kakimazeTime) {
        work->kakimazeRotY += 4;
        MBModelRotYSet(work->modelId[2], work->rotY+work->kakimazeRotY);
        work->kakimazeRotF = TRUE;
        work->kakimazeTime--;
    }
    if(MBCameraCullCheck(&work->pos, 400)) {
        if(work->kakimazeRotF) {
            CapMachineKakimazeMove(work);
        }
        for(capsule=work->capsule, i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
            MBCapsuleObjPosSet(capsule->objId, work->pos.x+capsule->pos.x, 245+work->pos.y+capsule->pos.y, work->pos.z+capsule->pos.z);
            MBCapsuleObjRotSetV(capsule->objId, &capsule->rot);
        }
    }
}

static void CapMachineKakimazeStart(CAPMACHINEWORK *work)
{
    work->kakimazeTime = 60;
    MBAudFXPlay(MSM_SE_BOARD_18);
}

static BOOL CapMachineKakimazeCheck(CAPMACHINEWORK *work)
{
    return work->kakimazeTime == 0;
}

static void CapMachineKakimazeMove(CAPMACHINEWORK *work)
{
    int i;
    CAPMACHINE_CAPSULE *capsule;
    int j;
    int colNum;
    int tryNo;
    CAPMACHINE_CAPSULE *capsule1;
    CAPMACHINE_CAPSULE *capsule2;
    
    float dist;
    float newDist;
    
    HuVecF bouPos[3];
    HuVecF bouOfs[3];
    
    Mtx rot;
    HuVecF dir;
    
    
    for(capsule=work->capsule, i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
        capsule->pos.y -= 4;
    }
    MTXRotDeg(rot, 'y', work->rotY+work->kakimazeRotY);
    for(i=0; i<3; i++) {
        MTXMultVec(rot, &bouPosTbl[i], &bouOfs[i]);
        VECSubtract(&bouOfs[i], &capMachineOfs, &bouPos[i]);
    }
    for(tryNo=0; tryNo<10; tryNo++) {
        colNum = 0;
        for(capsule=work->capsule, i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
            capsule->ofs.x = capsule->ofs.y = capsule->ofs.z = 0;
        }
        for(capsule=work->capsule, i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
            dist = VECMag(&capsule->pos);
            if(dist > 90.00001f) {
                newDist = (dist-90.00001f)+0.0001f;
                VECNormalize(&capsule->pos, &dir);
                VECScale(&dir, &dir, newDist);
                VECSubtract(&capsule->ofs, &dir, &capsule->ofs);
            }
        }
        for(i=0; i<CAPMACHINE_CAPSULE_MAX; i++) {
            capsule1 = &work->capsule[i];
            for(j=0; j<CAPMACHINE_CAPSULE_MAX; j++) {
                capsule2 = &work->capsule[j];
                if(i == j) {
                    continue;
                }
                VECSubtract(&capsule2->pos, &capsule1->pos, &dir);
                dist = VECMag(&dir);
                if(dist < 60.000004f) {
                    if(0 == dist) {
                        dir.y = 1;
                    } else {
                        VECNormalize(&dir, &dir);
                    }
                    newDist = (0.5f*(60.000004f-dist))+0.0001f;
                    VECScale(&dir, &dir, newDist);
                    VECSubtract(&capsule1->ofs, &dir, &capsule1->ofs);
                    VECAdd(&capsule2->ofs, &dir, &capsule2->ofs);
                    colNum++;
                }
            }
        }
        for(i=0; i<3; i++) {
            for(capsule=work->capsule, j=0; j<CAPMACHINE_CAPSULE_MAX; j++, capsule++) {
                float displaceOfs = ((capsule->pos.x*bouPos[i].x)-(capMachineOfs.x*bouPos[i].x)+(capsule->pos.y*bouPos[i].y)-(capMachineOfs.y*bouPos[i].y)+(capsule->pos.z*bouPos[i].z)-(capMachineOfs.z*bouPos[i].z))*0.0001f;
                HuVecF displaceDir;
                if(displaceOfs >= 0 && displaceOfs < 1) {
                    displaceDir.x = capMachineOfs.x+(displaceOfs*bouPos[i].x);
                    displaceDir.y = capMachineOfs.y+(displaceOfs*bouPos[i].y);
                    displaceDir.z = capMachineOfs.z+(displaceOfs*bouPos[i].z);
                    VECSubtract(&capsule->pos, &displaceDir, &dir);
                    dist = VECMag(&dir);
                    if(dist < 40.0f) {
                        if(0 == dist) {
                            dir.y = 1;
                        } else {
                            VECNormalize(&dir, &dir);
                        }
                        newDist = (40.0f-dist)+0.0001f;
                        VECScale(&dir, &dir, newDist);
                        VECAdd(&capsule->ofs, &dir, &capsule->ofs);
                        colNum++;
                    } else {
                        goto label2;
                    }
                } else {
                    label2:
                    VECSubtract(&capsule->pos, &bouOfs[i], &dir);
                    dist = VECMag(&dir);
                    if(dist < 40.0f) {
                        if(0 == dist) {
                            dir.y = 1;
                        } else {
                            VECNormalize(&dir, &dir);
                        }
                        newDist = (40.0f-dist)+0.0001f;
                        VECScale(&dir, &dir, newDist);
                        VECAdd(&capsule->ofs, &dir, &capsule->ofs);
                        colNum++;
                    }
                }
            }
        }
        for(capsule=work->capsule, i=0; i<CAPMACHINE_CAPSULE_MAX; i++, capsule++) {
            VECAdd(&capsule->pos, &capsule->ofs, &capsule->pos);
        }
        if(colNum == 0) {
            break;
        }
    }
}

void MBCapMachineTutorialExec(void) {
    int i;
    const static unsigned int sprTbl[4] = {
        W10_ANM_capsuleGreen,
        W10_ANM_capsuleYellow,
        W10_ANM_capsuleRed,
        W10_ANM_capsuleBlue
    };
    const static u32 mesTbl[4] = {
        MESS_BOARD_TUTORIAL_CAPSULEEVT_1,
        MESS_BOARD_TUTORIAL_CAPSULEEVT_2,
        MESS_BOARD_TUTORIAL_CAPSULEEVT_3,
        MESS_BOARD_TUTORIAL_CAPSULEEVT_4
    };
    int masuId = MBMasuFlagMatchFind(0, MASU_FLAG_CAPMACHINE);
    CAPMACHINEWORK *work = capMachineWork;
    for (i = 0; i < capMachineNum; i++, work++) {
        if (work->masuId == masuId) {
            break;
        }
    }

    MBCameraMasuViewSet(MBMasuFlagMatchFindLink(0, masuId, MASU_FLAG_BLOCKR), NULL, NULL, 2100.0f, -1.0f, 1);
    MBCameraMotionWait();
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    }
    WipeWait();
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPMACHINE, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBAudFXPlay(MSM_SE_BOARD_17);
    MBModelAttrReset(work->modelId[0], HU3D_MOTATTR_SHAPE_LOOP);
    CapMachineKakimazeStart(work);
    while(!CapMachineKakimazeCheck(work)) {
        HuPrcVSleep();
    }
    MBModelDispSet(work->modelId[0], FALSE);
    MBModelDispSet(work->modelId[1], TRUE);
    MBMotionShapeTimeSet(work->modelId[1],  0.0);
    MBModelAttrReset(work->modelId[1], HU3D_MOTATTR_SHAPE_PAUSE);
    HuPrcSleep(30);
    capsuleCurNo = 0;
    work->kakimazeRotF = FALSE;
    for (i = 0; i < 4; i++) {
        s16 sprId = MBTutorialSprDispOn(sprTbl[i]);
        MBWinCreate(MBWIN_TYPE_TUTORIAL, mesTbl[i], HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTutorialSprDispOff(sprId);
    }
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULEUSE, HUWIN_SPEAKER_NONE);
    MBTopWinWait();

    while (!MBMotionShapeEndCheck(work->modelId[1])) {
        HuPrcVSleep();
    }
    MBModelDispSet(work->modelId[0], TRUE);
    MBModelDispSet(work->modelId[1], FALSE);
    MBModelAttrSet(work->modelId[0], HU3D_MOTATTR_SHAPE_PAUSE);
    MBMotionShapeTimeSet(work->modelId[0], 0.0);
    MBModelAttrSet(work->modelId[0], HU3D_MOTATTR_SHAPE_PAUSE);
    WipeColorSet(255, 255, 255);
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)|| !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    }
    WipeWait();
}