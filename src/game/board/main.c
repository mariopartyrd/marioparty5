#include "game/board/main.h"
#include "game/board/capsule.h"
#include "game/board/status.h"
#include "game/board/camera.h"
#include "game/board/masu.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/pause.h"
#include "game/board/guide.h"
#include "game/board/coin.h"
#include "game/board/branch.h"
#include "game/board/sai.h"
#include "game/board/star.h"
#include "game/board/scroll.h"
#include "game/board/tutorial.h"
#include "game/board/opening.h"
#include "game/board/story.h"
#include "game/board/mgcall.h"
#include "game/board/telop.h"
#include "game/board/window.h"

#include "game/saveload.h"
#include "game/sprite.h"
#include "game/charman.h"
#include "game/audio.h"
#include "game/board/audio.h"

#include "game/disp.h"

#include "humath.h"

#include "game/flag.h"

//Prototypes for board functions
void MBBankCoinReset(void);
void MBCapsuleHookSet(void *func); //Fix input type as well
void MBCircuitMgEndExec(int playerNo);
void MBCircuitExec(BOOL turnIntrF);
void MBCircuitReset(void);

void MBGateInit(void);
void MBCapMachineInit(void);
void MBCapsuleObjInit(void);
void MBGateClose(void);
void MBCapMachineClose(void);

static MBTURNHOOK postTurnHook;
static MBTURNHOOK preTurnHook;
static MBLIGHTHOOK lightResetHook;
static MBLIGHTHOOK lightSetHook;
static MBKILLHOOK killMbHook;
static MBCREATEHOOK createMbHook;
HUPROCESS *mbMainProcess;
OMOBJMAN *mbObjMan;

static OMOVL gotoOvl = DLL_NONE;
static int lbl_80288244 = 20;
static int lbl_80288248 = 20;

static void DestroyMBObject(void);
static void MBMainProc(void);
static void DestroyMBMainProc(void);

void MBObjectSetup(int boardNo, MBCREATEHOOK createHook, MBKILLHOOK killHook)
{
    omSysPauseEnable(FALSE);
    MBPauseDisableSet(TRUE);
    if(!_CheckFlag(FLAG_BOARD_SAVEINIT)) {
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) && !_CheckFlag(FLAG_MG_CIRCUIT)) {
            if(GWPartyFGet() == TRUE) {
                GWMgInstFSet(GwCommon.partyMgInstF);
                GWMgComFSet(GwCommon.partyMgComF);
                GWMgPackSet(GwCommon.partyMgPack);
                GWMessSpeedSet(GwCommon.partyMessSpeed);
                GWSaveModeSet(GwCommon.partySaveMode);
            } else {
                GWMgInstFSet(GwCommon.storyMgInstF);
                GWMgComFSet(GwCommon.storyMgComF);
                GWMgPackSet(GwCommon.storyMgPack);
                GWMessSpeedSet(GwCommon.storyMessSpeed);
                GWSaveModeSet(GwCommon.storySaveMode);
            }
        }
        _SetFlag(FLAG_BOARD_SAVEINIT);
        _ClearFlag(FLAG_BOARD_OPENING);
        _ClearFlag(FLAG_BOARD_MG);
        _ClearFlag(FLAG_BOARD_MGKETTOU);
        _ClearFlag(FLAG_BOARD_MGDONKEY);
        _ClearFlag(FLAG_BOARD_MGKUPA);
        _ClearFlag(FLAG_BOARD_MGVS);
        GwSystem.turnNo = 1;
        MBStarFlagInit();
        MBStarNextNoSet(0);
        GwSystem.starTotal = 0;
        GwSystem.last5Effect = MB_LAST5_EFF_NONE;
        _ClearFlag(FLAG_BOARD_DEBUG);
        _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 3));
        _ClearFlag(FLAG_BOARD_NOMG);
        _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 2));
        _ClearFlag(FLAG_BOARD_SAI_SHORT);
        _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 6));
        _ClearFlag(FLAG_BOARD_LAST5);
        MBMgCallInit();
        MBStarNoInit();
        MBMasuCapsuleClear();
        MBBankCoinReset();
    }
    if(_CheckFlag(FLAG_BOARD_STAR_RESET)) {
        MBMgCallInit();
        MBStarNoInit();
        MBCapsuleHookSet(NULL);
        _ClearFlag(FLAG_BOARD_STAR_RESET);
    }
    gotoOvl = DLL_NONE;
    _SetFlag(FLAG_BOARD_TURN_NOSTART);
    _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 18));
    _ClearFlag(FLAG_BOARD_INIT);
    _ClearFlag(FLAGNUM(FLAG_GROUP_COMMON, 21));
    _ClearFlag(FLAG_BOARD_PAUSE);
    _ClearFlag(FLAG_BOARD_CAMERAMOT);
    _ClearFlag(FLAG_BOARD_STARMUS_RESET);
    _ClearFlag(FLAG_BOARD_STORYEND);
    _ClearFlag(FLAG_BOARD_EXIT);
    createMbHook = createHook;
    killMbHook = killHook;
    lightSetHook = lightResetHook = NULL;
    preTurnHook =  postTurnHook = NULL;
    MBPauseWatchProcCreate();
    mbObjMan = omInitObjMan(64, 8194);
    MBPrcDestructorSet(mbObjMan, DestroyMBObject);
    omSystemKeyCheckSetup(mbObjMan);
    mbMainProcess = HuPrcChildCreate(MBMainProc, 8207, 32768, 0, mbObjMan);
    MBPrcDestructorSet(mbMainProcess, DestroyMBMainProc);
    GwSystem.boardNo = boardNo;
}

static void CreateMB(void);
static void KillMB(void);

static void DestroyMBObject(void)
{
    KillMB();
    if(gotoOvl != DLL_NONE) {
       omOvlCall(gotoOvl, 0, 0);
       return;
    }
    _ClearFlag(FLAG_BOARD_SAVEINIT);
    if(_CheckFlag(FLAG_BOARD_EXIT)) {
        if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
            omOvlReturn(2);
        } else {
            omOvlReturn(1);
        }
    } else {
        if(GWPartyFGet() == TRUE && !_CheckFlag(FLAG_BOARD_TUTORIAL) && !_CheckFlag(FLAG_MG_CIRCUIT)) {
            omOvlGoto(DLL_partyresultdll, 0, 0);
        } else {
            omOvlReturn(1);
        }
    }
}

static void MBMainProc(void)
{
    BOOL turnIntrF = FALSE;
    int i;
    if(GwSystem.turnNo > GwSystem.turnMax && !_CheckFlag(FLAG_BOARD_TUTORIAL) && !_CheckFlag(FLAG_MG_CIRCUIT)) {
        MBPauseWatchProcStop();
        HuPrcSleep(-1);
    }
    CreateMB();
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        MBTutorialWatchProcCreate();
    }
    checkRepeat:
    if(!_CheckFlag(FLAG_BOARD_OPENING)) {
        if(_CheckFlag(FLAG_BOARD_DEBUG) && !_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            if(GWPartyFGet() == TRUE && !_CheckFlag(FLAG_MG_CIRCUIT)) {
                int starNo = MBStarNoRandGet();
                if(starNo >= 0) {
                    MBStarNoSet(starNo);
                }
            }
            if(_CheckFlag(FLAG_MG_CIRCUIT)) {
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    MBPlayerCapsuleAdd(i, CAPSULE_KINOKO);
                    MBPlayerCapsuleAdd(i, CAPSULE_KINOKO);
                    MBPlayerCapsuleAdd(i, CAPSULE_SKINOKO);
                }
            }
        } else {
            GwSystem.turnPlayerNo = -1;
            MBOpeningProcExec();
        }
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) && !_CheckFlag(FLAG_MG_CIRCUIT)) {
            if(GWPartyFGet() == FALSE || _CheckFlag(FLAG_BOARD_DEBUG)) {
                MBMasuDonkeySet(GwSystem.mgEvent);
            }
            MBCapsuleOpeningAdd(6);
        }
        if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
            if(GWPartyFGet() == TRUE) {
                if(!GWTeamFGet()) {
                    for(i=0; i<GW_PLAYER_MAX; i++) {
                        MBPlayerCoinSet(i, 10);
                    }
                } else {
                    for(i=0; i<2; i++) {
                        MBPlayerGrpCoinSet(i, 20);
                    }
                }
            } else {
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    MBPlayerCoinSet(i, 20);
                }
            }
        } else {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                MBPlayerCoinSet(i, 0);
            }
        }
        GwSystem.turnPlayerNo = 0;
        _SetFlag(FLAG_BOARD_OPENING);
    }
    if(MBEventMgCheck()) {
        if(_CheckFlag(FLAG_BOARD_MG)) {
            if(_CheckFlag(FLAG_MG_CIRCUIT)) {
                MBCircuitMgEndExec(GwSystem.turnPlayerNo);
            }
            _ClearFlag(FLAG_BOARD_MG);
        } else if(_CheckFlag(FLAG_BOARD_MGKETTOU)) {
            if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
                MBKettouMgEndExec(GwSystem.turnPlayerNo);
            } else {
                MBCircuitKettouMgEndExec(GwSystem.turnPlayerNo);
            }
            _ClearFlag(FLAG_BOARD_MGKETTOU);
            turnIntrF = TRUE;
        } else if(_CheckFlag(FLAG_BOARD_MGDONKEY)) {
            MBDonkeyMgEndExec(GwSystem.turnPlayerNo);
            _ClearFlag(FLAG_BOARD_MGDONKEY);
            turnIntrF = TRUE;
        } else if(_CheckFlag(FLAG_BOARD_MGKUPA)) {
            if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
                MBKupaMgEndExec(GwSystem.turnPlayerNo);
            } else {
                MBCircuitKupaMgEndExec(GwSystem.turnPlayerNo);
            }
            _ClearFlag(FLAG_BOARD_MGKUPA);
            turnIntrF = TRUE;
        } else if(_CheckFlag(FLAG_BOARD_MGVS)) {
            MBVsMgEndExec(GwSystem.turnPlayerNo);
            _ClearFlag(FLAG_BOARD_MGKUPA);
            turnIntrF = TRUE;
        }
    }
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        MBCircuitExec(turnIntrF);
        MBCircuitReset();
        _ClearFlag(FLAG_BOARD_OPENING);
        goto checkRepeat;
    }
    while(1) {
        if(preTurnHook && !turnIntrF) {
            preTurnHook();
        }
        if(GwSystem.turnMax-GwSystem.turnNo < 5 && GwSystem.turnPlayerNo == 0 && !turnIntrF) {
            if(!_CheckFlag(FLAG_BOARD_LAST5)) {
                MBLast5Exec();
                _SetFlag(FLAG_BOARD_LAST5);
            } else {
                MBTelopLast5Create();
            }
        }
        MBMusMainPlay();
        _ClearFlag(FLAG_BOARD_TURN_NOSTART);
        if(GWPartyFGet() != FALSE) {
            MBStatusDispForceSetAll(TRUE);
            MBTurnExecParty(turnIntrF);
            GwSystem.turnNo++;
            GwSystem.turnPlayerNo = 0;
            if(_CheckFlag(FLAG_BOARD_NOMG)) {
                turnIntrF = FALSE;
                MBStatusColorAllSet(STATUS_COLOR_GRAY);
                continue;
            } else if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                MBStatusColorAllSet(STATUS_COLOR_GRAY);
                continue;
            } else {
                GwSystem.turnPlayerNo = -1;
                MBMgCallExec();
                MBStatusColorAllSet(STATUS_COLOR_GRAY);
                if(postTurnHook) {
                    postTurnHook();
                }
                HuPrcSleep(-1);
            }
        } else {
            if(GwSystem.turnPlayerNo == MBPlayerStoryPlayerGet() || GwSystem.playerMode != MB_PLAYER_MODE_WALK) {
                MBStatusDispForceSetAll(TRUE);
            }
            MBTurnExecStory(turnIntrF);
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                MBStatusDispForceSetAll(FALSE);
                MBStoryEndProcExec();
            }
            GwSystem.turnNo++;
            if(GwSystem.turnNo > GwSystem.turnMax) {
                MBStoryEndProcExec();
            }
            GwSystem.turnPlayerNo = 0;
            MBStatusColorAllSet(STATUS_COLOR_GRAY);
            if(postTurnHook) {
                postTurnHook();
            }
        }
        turnIntrF = FALSE;
    }
}

static void DestroyMBMainProc(void)
{
    mbMainProcess = NULL;
}

void MBOvlGoto(OMOVL ovl)
{
    gotoOvl = ovl;
    MBPauseWatchProcStop();
}

BOOL MBEventMgCheck(void)
{
    if(_CheckFlag(FLAG_BOARD_MG)
        || _CheckFlag(FLAG_BOARD_MGKETTOU)
        || _CheckFlag(FLAG_BOARD_MGKUPA)
        || _CheckFlag(FLAG_BOARD_MGDONKEY)
        || _CheckFlag(FLAG_BOARD_MGVS)) {
            return TRUE;
        } else {
            return FALSE;
        }
}

static void CreateMB(void)
{
    GwSystem.mgNo = GW_MGNO_NONE;
    if(!SLSaveFlagGet()) {
        GWSaveModeSet(1);
    }
    if(GWMgInstFGet()) {
        _SetFlag(FLAG_MGINST_ON);
    } else {
        _ClearFlag(FLAG_MGINST_ON);
    }
    if(!MBEventMgCheck()) {
        _ClearFlag(FLAG_BOARD_MG);
        _ClearFlag(FLAG_BOARD_MGKETTOU);
        _ClearFlag(FLAG_BOARD_MGKUPA);
        _ClearFlag(FLAG_BOARD_MGDONKEY);
        _ClearFlag(FLAG_BOARD_MGVS);
    }
    if(MBEventMgCheck()) {
        MBMgCallDataClose();
    }
    HuSprExecLayerSet(32, 4);
    MBCameraCreate();
    MBAudInit();
    MBModelInit();
    MBPauseInit();
    MBWinInit();
    MBGateInit();
    MBGuideInit();
    MBCapMachineInit();
    MBCapsuleObjInit();
    MBCoinInit();
    MBBranchInit();
    createMbHook();
    MBPlayerInit(MBEventMgCheck() == FALSE);
    MBStatusInit();
    MBSaiInit();
    MBStarInit();
    MBLightSet();
    CharModelDataClose(CHARNO_NONE);
    if(GwSystem.last5Effect == MB_LAST5_EFF_REDKUPA && MBBoardNoGet() != MBNO_WORLD_7) {
        MBMasuTypeChange(MASU_TYPE_RED, MASU_TYPE_KUPA);
    }
    MBTauntInit();
    Hu3DReflectNoSet(0);
    _SetFlag(FLAG_BOARD_INIT);
}

static void KillMB(void)
{
    if(!_CheckFlag(FLAG_BOARD_INIT)) {
        return;
    }
    MBTauntKill();
    MBAudClose();
    HuAudAllStop();
    MBSaiClose();
    MBStatusClose();
    MBMasuKill();
    MBStarClose();
    MBCoinClose();
    MBGateClose();
    MBCapMachineClose();
    MBScrollClose();
    MBPlayerClose();
    if(killMbHook) {
        killMbHook();
    }
    MBModelClose();
    MBWinClose();
    MBDataClose();
    createMbHook = killMbHook = NULL;
    preTurnHook = NULL;
    postTurnHook = NULL;
    _SetFlag(FLAG_BOARD_TURN_NOSTART);
    _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 18));
    _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 21));
}

void MBDataClose(void)
{
    HuDataDirClose(DATA_effect);
    HuDataDirClose(DATA_board);
    HuDataDirClose(DATA_guide);
    HuDataDirClose(DATA_capsule);
    HuDataDirClose(DATA_capsulechar0);
    HuDataDirClose(DATA_capsulechar1);
    HuDataDirClose(DATA_capsulechar2);
    HuDataDirClose(DATA_capsulechar3);
    HuDataDirClose(DATA_capsulechar4);
    HuDataDirClose(DATA_bbattle);
    HuDataDirClose(DATA_bpause);
    HuDataDirClose(DATA_blast5);
    HuDataDirClose(DATA_bdemo);
}

void MBLightHookSet(MBLIGHTHOOK setHook, MBLIGHTHOOK resetHook)
{
    lightSetHook = setHook;
    lightResetHook = resetHook;
}

void MBLightSet(void)
{
    Hu3DBGColorSet(0, 0, 0);
    if(lightSetHook) {
        lightSetHook();
    }
}

void MBLightReset(void)
{
    if(lightResetHook) {
        lightResetHook();
    }
    Hu3DBGColorSet(0, 0, 0);
    Hu3DFogClear();
    Hu3DReflectNoSet(0);
}

void MBPauseFlagSet(void)
{
    _SetFlag(FLAG_BOARD_PAUSE);
}

void MBPauseFlagReset(void)
{
    _ClearFlag(FLAG_BOARD_PAUSE);
}

BOOL MBPauseFlagCheck(void)
{
    return _CheckFlag(FLAG_BOARD_PAUSE) ? TRUE : FALSE;
}

s32 MBDataDirReadAsync(int dataNum)
{
    return HuDataDirReadAsync(dataNum);
}

void MBDataAsyncWait(s32 statId)
{
    if(statId == HU_DATA_STAT_NONE) {
        return;
    }
    while(HuDataGetAsyncStat(statId) == FALSE) {
        HuPrcVSleep();
    }
}

void MBPreTurnHookSet(MBTURNHOOK turnHook)
{
    preTurnHook = turnHook;
}

void MBPostTurnHookSet(MBTURNHOOK turnHook)
{
    postTurnHook = turnHook;
}

void fn_8008D448(int value)
{
    lbl_80288244 = value;
}

void fn_8008D450(int value)
{
    lbl_80288248 = value;
}

int fn_8008D458(void)
{
    return lbl_80288244;
}

int fn_8008D460(void)
{
    return lbl_80288248;
}

void MBPartySaveInit(int boardNo)
{
    int i, j;
    GwSystem.boardNo = boardNo;
    _ClearFlag(FLAG_BOARD_SAVEINIT);
    _ClearFlag(FLAG_MG_PRACTICE);
    GwSystem.mgNo = 0;
    GwSystem.subGameNo = 0;
    GwSystem.saiMasuId = 0;
    memset(&GwSystem.mbSaveWork[0], 0, sizeof(GwSystem.mbSaveWork));
    GwSystem.turnNo = 1;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayer[i].coin = 0;
        if(GWPartyFGet() == FALSE || _CheckFlag(FLAG_BOARD_TUTORIAL) || _CheckFlag(FLAG_MG_CIRCUIT)) {
            GwPlayer[i].star = 0;
            GWTeamFSet(FALSE);
        } else {
            GwPlayer[i].star = MBPlayerHandicapGet(i);
        }
        GwPlayer[i].blueMasuNum = 0;
        GwPlayer[i].redMasuNum = 0;
        GwPlayer[i].hatenaMasuNum = 0;
        GwPlayer[i].kinokoMasuNum = 0;
        GwPlayer[i].kupaMasuNum = 0;
        GwPlayer[i].m444MasuNum = 0;
        GwPlayer[i].warpMasuNum = 0;
        GwPlayer[i].starMasuNum = 0;
        GwPlayer[i].donkeyMasuNum = 0;
        GwPlayer[i].capsuleMasuNum = 0;
        GwPlayer[i].coinTotalMg = 0;
        GwPlayer[i].coinTotal = 0;
        GwPlayer[i].coinMax = 0;
        GwPlayer[i].starMax = 0;
        GwPlayer[i].coinBattle = 0;
        GwPlayer[i].mgCoin = 0;
        GwPlayer[i].mgCoinBonus = 0;
        for(j=0; j<3; j++) {
            GwPlayer[i].capsule[j] = CAPSULE_NULL;
        }
    }
}

void MBStorySaveInit(int mgPack, int storyDif)
{
    GWPartyFSet(FALSE);
    GWTeamFSet(FALSE);
    GwSystem.storyDif = storyDif;
    GWBonusStarFSet(FALSE);
    GWMgPackSet(mgPack);
    MBPlayerHandicapSet(0, 0);
    MBPlayerHandicapSet(1, 0);
    MBPlayerHandicapSet(2, 0);
    MBPlayerHandicapSet(3, 0);
    GwSystem.turnMax = 15;
    memset(&GwPlayer[0], 0, GW_PLAYER_MAX*sizeof(GWPLAYER));
    _ClearFlag(FLAG_STORY_MAP1_COMPLETE);
    _ClearFlag(FLAG_STORY_MAP2_COMPLETE);
    _ClearFlag(FLAG_STORY_MAP3_COMPLETE);
    _ClearFlag(FLAG_STORY_MAP4_COMPLETE);
    _ClearFlag(FLAG_STORY_MAP5_COMPLETE);
    _ClearFlag(FLAG_STORY_MAP6_COMPLETE);
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    _ClearFlag(FLAG_MG_CIRCUIT);
    _SetFlag(FLAG_MGINST_ON);
    _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 13));
    _ClearFlag(FLAG_DECA_INST);
}

void MBPlayerSaveInit(BOOL teamF, BOOL bonusStarF, int mgPack, int turnMax, int handicapP1, int handicapP2, int handicapP3, int handicapP4)
{
    GWPartyFSet(TRUE);
    GWTeamFSet(teamF);
    GwSystem.storyDif = 0;
    GWBonusStarFSet(bonusStarF);
    GWMgPackSet(mgPack);
    GwSystem.turnMax = turnMax;
    MBPlayerHandicapSet(0, handicapP1);
    MBPlayerHandicapSet(1, handicapP2);
    MBPlayerHandicapSet(2, handicapP3);
    MBPlayerHandicapSet(3, handicapP4);
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    _ClearFlag(FLAG_MG_CIRCUIT);
    _SetFlag(FLAG_MGINST_ON);
    _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 13));
    _ClearFlag(FLAG_DECA_INST);
}

float MBVecDistXZ(HuVecF *a, HuVecF *b)
{
    float dx = a->x-b->x;
    float dz = a->z-b->z;
    return HuMagPoint2D(dx, dz);
}

BOOL MBVecDistXZCheck(HuVecF *a, HuVecF *b, float maxDist)
{
    float dist = MBVecDistXZ(a, b);
    if(dist <= maxDist) {
        return 1;
    } else {
        return 0;
    }
}

float MBAngleWrap(float angle)
{
    angle = fmod(angle, 360);
    if(angle < -180.0f) {
        angle += 360.0f;
    } else if(angle > 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

void MBAngleWrapV(HuVecF *angle)
{
    float *dest = (float *)angle;
    int i;
    for(i=0; i<3; dest++, i++) {
        *dest = MBAngleWrap(*dest);
    }
}

BOOL MBAngleAdd(float *dest, float angle, float speed)
{
    float wrapAngle = fmod(angle-(*dest), 360);
    float diff;
    if(fabs(wrapAngle) < speed) {
        *dest = angle;
        return TRUE;
    }
    if(wrapAngle < 0.0f) {
        wrapAngle += 360.0f;
    }
    if(wrapAngle > 180.0f) {
        diff = -speed;
    } else {
        diff = speed;
    }
    *dest += diff;
    *dest = MBAngleWrap(*dest);
    return FALSE;
}

BOOL MBAngleMoveTo(float *dest, float angle, float speed)
{
    float wrapAngle = fmod(angle-(*dest), 360);
    float diff;
    if(fabs(wrapAngle) < 1.0) {
        *dest = angle;
        return TRUE;
    }
    if(wrapAngle < 0.0f) {
        wrapAngle += 360.0f;
    }
    if(wrapAngle > 180.0f) {
        wrapAngle -= 360.0f;
    }
    *dest = fmod(*dest+(speed*wrapAngle), 360.0);
    if(*dest < 0.0f) {
        *dest += 360.0f;
    }
    return FALSE;
}

BOOL MBVecMagCheck(HuVecF *a, HuVecF *b, float dist)
{
    HuVecF diff;
    VECSubtract(a, b, &diff);
    if(VECSquareMag(&diff) >= dist*dist) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void MBMtxLookAtCalc(Mtx dest, HuVecF *eye, HuVecF *up, HuVecF *target)
{
	HuVecF f, u, s;
	f.x = eye->x-target->x;
	f.y = eye->y-target->y;
	f.z = eye->z-target->z;
	VECNormalize(&f, &f);
	VECCrossProduct(up, &f, &u);
	VECNormalize(&u, &u);
	VECCrossProduct(&f, &u, &s);
	dest[0][0] = u.x;
	dest[0][1] = u.y;
	dest[0][2] = u.z;
	dest[0][3] = 0;
	dest[1][0] = s.x;
	dest[1][1] = s.y;
	dest[1][2] = s.z;
	dest[1][3] = 0;
	dest[2][0] = f.x;
	dest[2][1] = f.y;
	dest[2][2] = f.z;
	dest[2][3] = 0;
}

void MB3Dto2D(HuVecF *src, HuVecF *dst)
{
    MBCAMERA *cameraP = MBCameraGet();
    float tanFov;
    float width;
    float height;
    Mtx lookAt;
    HuVecF pos;
    
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXMultVec(lookAt, src, &pos);
    tanFov = HuSin(cameraP->fov*0.5f)/HuCos(cameraP->fov*0.5f);
    width = HU_DISP_ASPECT*(tanFov*pos.z);
    height = tanFov*pos.z;
    dst->x = HU_DISP_CENTERX+(pos.x*(HU_DISP_CENTERX/(-width)));
    dst->y = HU_DISP_CENTERY+(pos.y*(HU_DISP_CENTERY/height));
    dst->z = -pos.z;
}

void MB2Dto3D(HuVecF *src, HuVecF *dst)
{
    MBCAMERA *cameraP = MBCameraGet();
    float tanFov = HuSin(cameraP->fov*0.5f)/HuCos(cameraP->fov*0.5f);
    float height = 2.0f*(tanFov*src->z);
    float width = HU_DISP_ASPECT*height;
    float normX = src->x/HU_DISP_WIDTH;
    float normY = src->y/HU_DISP_HEIGHT;
    Mtx lookAt;
    
    dst->x = (normX-0.5)*width;
    dst->y = -(normY-0.5)*height;
    dst->z = -src->z;
    MBCameraLookAtGetInv(lookAt);
    MTXMultVec(lookAt, dst, dst);
}

float MBBezierCalc(float a, float b, float c, float t)
{
    float invTime = 1-t;
    return (t*t*c)+((invTime*invTime*a)+(b*((2.0f*invTime)*t)));
}

void MBBezierCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *dst, float t)
{
    dst->x = MBBezierCalc(a->x, b->x, c->x, t);
    dst->y = MBBezierCalc(a->y, b->y, c->y, t);
    dst->z = MBBezierCalc(a->z, b->z, c->z, t);
}

void MBBezierCalcVList(HuVecF *src, HuVecF *dst, float t)
{
    dst->x = MBBezierCalc(src[0].x, src[1].x, src[2].x, t);
    dst->y = MBBezierCalc(src[0].y, src[1].y, src[2].y, t);
    dst->z = MBBezierCalc(src[0].z, src[1].z, src[2].z, t);
}

float MBBezierCalcSlope(float a, float b, float c, float t)
{
    return 2.0f*((-a+b)+(t*(c+(a-(2.0f*b)))));
}

void MBBezierCalcSlopeV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *dst, float t)
{
    dst->x = MBBezierCalcSlope(a->x, b->x, c->x, t);
    dst->y = MBBezierCalcSlope(a->y, b->y, c->y, t);
    dst->z = MBBezierCalcSlope(a->z, b->z, c->z, t);
}

float MBHermiteCalc(float a, float b, float c, float d, float t)
{
    float tt = t*t;
    float ttt = t*t*t;
    float aCoef = 1.0f+((2.0f*ttt)-(3.0f*tt));
    float bCoef = (-2.0f*ttt)+(3.0f*tt);
    float cCoef = t+(ttt-(2.0f*tt));
    float dCoef = ttt-tt;
    return (aCoef*a)+(bCoef*b)+(cCoef*c)+(dCoef*d);
}

void MBHermiteCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, HuVecF *dst, float t)
{
    dst->x = MBHermiteCalc(a->x, b->x, c->x, d->x, t);
    dst->y = MBHermiteCalc(a->y, b->y, c->y, d->y, t);
    dst->z = MBHermiteCalc(a->z, b->z, c->z, d->z, t);
}

float MBHermiteCalcSlope(float a, float b, float c, float d, float t)
{
    float tt = t*t;
    float aCoef = (6.0f*tt)-(6.0f*t);
    float bCoef = (-6.0f*tt)+(6.0f*t);
    float cCoef = 1.0f+((3.0f*tt)-(4.0f*t));
    float dCoef = (3.0f*tt)-(2.0f*t);
    return (aCoef*a)+(bCoef*b)+(cCoef*c)+(dCoef*d);
}

float MBAngleLerp(float a, float b, float t)
{
    float diff = fmod(b-a, 360);
    float ret;
    if(diff < 0.0f) {
        diff += 360.0f;
    }
    if(diff > 180.0f) {
        diff -= 360.0f;
    }
    ret = fmod(a+(t*diff), 360);
    if(ret < 0) {
        ret += 360.0f;
    }
    return ret;
}

float MBAngleLerpSin(float a, float b, float t)
{
    return MBAngleLerp(a, b, HuSin(t*90.0f));
}

float MBAngleLerpCos(float a, float b, float t)
{
    return MBAngleLerp(a, b, 1-HuCos(t*90.0f));
}

void MBPointDepthScale(HuVecF *src, float scale, HuVecF *dst)
{
    MBCAMERA *cameraP = MBCameraGet();
    HuVecF pos;
    float tanFov;
    float depth;
    float z;
    MB3Dto2D(src, &pos);
    tanFov = HuSin(cameraP->fov*0.5f)/HuCos(cameraP->fov*0.5f);
    depth = pos.z*tanFov;
    z = (depth/scale)/tanFov;
    pos.z = z;
    MB2Dto3D(&pos, dst);
}