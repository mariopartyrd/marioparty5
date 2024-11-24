#include "game/board/pause.h"
#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/audio.h"
#include "game/board/window.h"
#include "game/board/camera.h"

#include "game/disp.h"
#include "game/hsfex.h"

#include "game/pad.h"

#include "game/esprite.h"

#include "game/sprite.h"
#include "game/hu3d.h"
#include "game/flag.h"

#include "datanum/w20.h"
#include "messnum/boardope.h"
#include "messnum/board_pause.h"

#define PAUSE_HOOK_MAX 32

void MBEffFadeCreate(s16 time, u8 dist);
void MBEffFadeOutSet(s16 time);
BOOL MBEffFadeDoneCheck(void);

static MBPAUSEHOOK pauseHook[PAUSE_HOOK_MAX];
static HUPROCESS *pauseProc;
static int pauseHookNum;
static int pausePlayer = -1;
static const padWinMesTbl[2] = { BOARDOPE_PAD_PAUSE, BOARD_PAUSE_PAD_MGCIRCUIT };

static void PauseProcExec(void);
static void PauseProcKill(void);

void MBPauseInit(void)
{
    int i;
    pauseProc = NULL;
    pauseHookNum = 0;
    HuPrcAllPause(FALSE);
    Hu3DPauseSet(FALSE);
    HuSprPauseSet(FALSE);
    for(i=0; i<PAUSE_HOOK_MAX; i++) {
        pauseHook[i] = NULL;
    }
}

void MBPauseProcCreate(int playerNo)
{
    pauseProc = MBPrcCreate(PauseProcExec, 8209, 14336);
    HuPrcDestructorSet2(pauseProc, PauseProcKill);
    pausePlayer = playerNo;
    HuPrcSetStat(pauseProc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    MBPauseSet(TRUE);
}

int MBPauseStartCheck(void)
{
    int i;
    ;
    if(MBPauseProcCheck()) {
        return -1;
    }
    if(MBPauseDisableGet()) {
        return -1;
    }
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return -1;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        int padNo = GwPlayer[i].padNo;
        if(HuPadStatGet(padNo) == 0) {
            if((GWPartyFGet() != FALSE || GwPlayer[i].comF == FALSE) && (HuPadBtnDown[padNo] & PAD_BUTTON_START)) {
                return i;
            }
        }
    }
    return -1;
}

BOOL MBPauseProcCheck(void)
{
    return (pauseProc != NULL) ? TRUE : FALSE;
}

void MBPauseDisableSet(BOOL disableF)
{
    if(disableF) {
        _SetFlag(FLAG_BOARD_PAUSE_DISABLE);
    } else {
        _ClearFlag(FLAG_BOARD_PAUSE_DISABLE);
    }
}

BOOL MBPauseDisableGet(void)
{
    return (_CheckFlag(FLAG_BOARD_PAUSE_DISABLE)) ? TRUE : FALSE;
}

void MBPauseHookPush(MBPAUSEHOOK hook)
{
    pauseHook[pauseHookNum++] = hook;
}

void MBPauseHookPop(MBPAUSEHOOK hook)
{
    int i;
    for(i=0; i<pauseHookNum; i++) {
        if(pauseHook[i] == hook) {
            break;
        }
    }
    if(i >= pauseHookNum) {
        return;
    }
    for(; i<pauseHookNum-1; i++) {
        pauseHook[i] = pauseHook[i+1];
    }
    pauseHookNum--;
}

typedef struct pauseWork_s {
    s16 sprId[6];
    MBMODELID modeMdlId;
    ANIMDATA *anim;
    HU3DANIMID animId;
} PAUSEWORK;

static void PauseScreenCreate(PAUSEWORK *work);
static void PauseScreenKill(PAUSEWORK *work);
static int PauseModeGetNext(void);

static void PauseProcExec(void)
{
    PAUSEWORK pauseWork;
    PAUSEWORK *work = &pauseWork;
    s32 statId;
    BOOL cancelF = FALSE;
    
    int i;
    static const unsigned int logoFileTbl[] = {
        BOARD_ANM_logoMap1,
        BOARD_ANM_logoMap2,
        BOARD_ANM_logoMap3,
        BOARD_ANM_logoMap4,
        BOARD_ANM_logoMap5,
        BOARD_ANM_logoMap6,
        BOARD_ANM_logoMap7,
        0,
        W20_ANM_logo
    };
    work->sprId[0] = espEntry(MBDATANUM(logoFileTbl[MBBoardNoGet()]), 100, 0);
    espDispOff(work->sprId[0]);
    HuDataDirClose(DATA_board);
    statId = MBDataDirReadAsync(DATA_bpause);
    MBEffFadeCreate(30, 160);
    for(i=0; i<pauseHookNum; i++) {
        if(pauseHook[i] != NULL) {
            pauseHook[i](FALSE);
        }
    }
    HuPadRumbleAllStop();
    HuPrcSleep(1);
    MBAudFXPlay(MSM_SE_CMN_06);
    while(!MBEffFadeDoneCheck()) {
        HuPrcVSleep();
    }
    MBPauseDispCopyCreate();
    MBDataAsyncWait(statId);
    while(cancelF == FALSE) {
        int helpWin = MBWinCreateHelp(padWinMesTbl[_CheckFlag(FLAG_MG_CIRCUIT) ? 1 : 0]);
        HuVec2F center;
        MBWinCenterGet(helpWin, &center);
        MBWinPosSet(helpWin, center.x, _CheckFlag(FLAG_MG_CIRCUIT) ? 288 : 376);
        PauseScreenCreate(work);
        if(PauseModeGetNext() == 0) {
            break;
        }
        MBTopWinKill();
        PauseScreenKill(work);
        cancelF = MBPauseOptionExec(pausePlayer);
    }
    PauseScreenKill(work);
    MBTopWinKill();
    espKill(work->sprId[0]);
    work->sprId[0] = -1;
    HuDataDirClose(DATA_bpause);
    MBPauseDispCopyKill();
    if(cancelF) {
        MBPauseWatchProcStop();
        HuPrcSleep(-1);
    }
    statId = MBDataDirReadAsync(DATA_board);
    MBEffFadeOutSet(30);
    HuPrcSleep(30);
    MBDataAsyncWait(statId);
    HuPrcEnd();
}

static void PauseProcKill(void)
{
    int i;
    for(i=0; i<pauseHookNum; i++) {
        if(pauseHook[i] != NULL) {
            pauseHook[i](TRUE);
        }
    }
    if(MBKillCheck() == FALSE) {
        MBPauseSet(FALSE);
    }
    if(GWMgInstFGet()) {
        _SetFlag(FLAG_MGINST_ON);
    } else {
        _ClearFlag(FLAG_MGINST_ON);
    }
    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
        if(GWPartyFGet() == FALSE) {
            GwCommon.storyMgInstF = GWMgInstFGet();
            GwCommon.storyMgComF = GWMgComFGet();
            GwCommon.storyMgPack = GWMgPackGet();
            GwCommon.storyMessSpeed = GWMessSpeedGet();
            GwCommon.storySaveMode = GWSaveModeGet();
        } else {
            GwCommon.partyMgInstF = GWMgInstFGet();
            GwCommon.partyMgComF = GWMgComFGet();
            GwCommon.partyMgPack = GWMgPackGet();
            GwCommon.partyMessSpeed = GWMessSpeedGet();
            GwCommon.partySaveMode = GWSaveModeGet();
        }
    }
    pausePlayer = -1;
    pauseProc = NULL;
}

static void PauseScreenCreate(PAUSEWORK *work)
{
    MBCAMERA *cameraP = MBCameraGet();
    Mtx matrix;
    HuVecF pos2D, pos3D;
    int i;
    int turn;
    static const HuVec2F turnPos[] = {
        -136, 0,
        -96, 0,
        0, 0,
        40, 0
    };
    static const int anmNoTbl[] = { 1, 0, 2 };
    
    espDispOn(work->sprId[0]);
    espAttrSet(work->sprId[0], HUSPR_ATTR_LINEAR);
    espPosSet(work->sprId[0], HU_DISP_CENTERX, 128);
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        espPosSet(work->sprId[0], HU_DISP_CENTERX, 160);
    }
    work->modeMdlId = MBModelCreate(MBPAUSEDATANUM(BPAUSE_HSF_gameModeCap), NULL, FALSE);
    MBModelLayerSet(work->modeMdlId, 6);
    MBModelRotSet(work->modeMdlId, 0, 0, 22.5f);
    pos2D.x = 64;
    pos2D.y = 208;
    pos2D.z = 300;
    Hu3D2Dto3D(&pos2D, HU3D_CAM0, &pos3D);
    MBModelPosSetV(work->modeMdlId, &pos3D);
    MBModelScaleSet(work->modeMdlId, 0.15f, 0.15f, 0.15f);
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        MBModelDispSet(work->modeMdlId, FALSE);
    }
    MTXLookAt(matrix, &cameraP->pos, &cameraP->up, &pos3D);
    MTXInverse(matrix, matrix);
    matrix[0][3] = matrix[1][3] = matrix[2][3] = 0;
    MBModelMtxSet(work->modeMdlId, &matrix);
    work->anim = HuSprAnimDataRead(MBPAUSEDATANUM(BPAUSE_ANM_gameMode));
    work->animId = Hu3DAnimCreate(work->anim, MBModelIdGet(work->modeMdlId), "BATTLE");
    Hu3DAnimSpeedSet(work->animId, 0);
    Hu3DAnmNoSet(work->animId, anmNoTbl[(GWTeamFGet() != FALSE) ? 2 : GWPartyFGet()]);
    Hu3DAnimAttrSet(work->animId, HU3D_ANIM_ATTR_ANIMON);
    work->sprId[1] = espEntry(MBPAUSEDATANUM(BPAUSE_ANM_turnBack), 100, 0);
    espAttrSet(work->sprId[1], HUSPR_ATTR_LINEAR);
    pos2D.x = HU_DISP_CENTERX;
    pos2D.y = HU_DISP_HEIGHT-176;
    espPosSet(work->sprId[1], pos2D.x, pos2D.y);
    for(i=0; i<4; i++) {
        work->sprId[i+2] = espEntry(MBPAUSEDATANUM(BPAUSE_ANM_turnNum), 95, 0);
        espPosSet(work->sprId[i+2], pos2D.x+turnPos[i].x, pos2D.y+turnPos[i].y);
        espAttrSet(work->sprId[i+2], HUSPR_ATTR_LINEAR);
    }
    turn = GwSystem.turnNo;
    if(turn > 99) {
        turn = 99;
    }
    if(turn/10 != 0) {
        espBankSet(work->sprId[2], turn/10);
    } else {
        espAttrSet(work->sprId[2], HUSPR_ATTR_DISPOFF);
    }
    espBankSet(work->sprId[3], turn%10);
    turn = GwSystem.turnMax;
    if(turn > 99) {
        turn = 99;
    }
    if(turn/10 != 0) {
        espBankSet(work->sprId[4], turn/10);
    } else {
        espAttrSet(work->sprId[4], HUSPR_ATTR_DISPOFF);
    }
    espBankSet(work->sprId[5], turn%10);
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        espDispOff(work->sprId[1]);
        for(i=0; i<4; i++) {
            espDispOff(work->sprId[i+2]);
        }
    }
}

static void PauseScreenKill(PAUSEWORK *work)
{
    int i;
    espDispOff(work->sprId[0]);
    for(i=1; i<6; i++) {
        if(work->sprId[i] >= 0) {
            espKill(work->sprId[i]);
        }
        work->sprId[i] = -1;
    }
    if(work->modeMdlId >= 0) {
        MBModelKill(work->modeMdlId);
        work->modeMdlId = MB_MODEL_NONE;
    }
    if(work->animId >= 0) {
        Hu3DAnimKill(work->animId);
        work->animId = HU3D_ANIMID_NONE;
    }
}

static int PauseModeGetNext(void)
{
    int padNo;
    int result;
    while(1) {
        HuPrcVSleep();
        padNo = GwPlayer[pausePlayer].padNo;
        if(HuPadStatGet(padNo)) {
            continue;
        }
        if(_CheckFlag(FLAG_MG_CIRCUIT)) {
            if(HuPadBtnDown[padNo] & PAD_BUTTON_X) {
                result = 1;
                MBAudFXPlay(MSM_SE_CMN_02);
                break;
            }
        } else {
            if(HuPadBtnDown[padNo] & PAD_BUTTON_A) {
                result = 1;
                MBAudFXPlay(MSM_SE_CMN_02);
                break;
            }
        }
        if(HuPadBtnDown[padNo] & PAD_BUTTON_START) {
            result = 0;
            break;
        }
    }
    return result;
}

void MBPauseSet(BOOL pauseF)
{
    OMOBJWORK *objWork = mbObjMan->property;
    int i;
    if(pauseF) {
        for(i=0; i<objWork->objMax; i++) {
            if((objWork->objData[i].stat & (OM_STAT_DELETED|OM_STAT_NOPAUSE)) == 0) {
                omSetStatBit(&objWork->objData[i], OM_STAT_PAUSED);
            }
        }
        MBPauseFlagSet();
    } else {
        for(i=0; i<objWork->objMax; i++) {
            if((objWork->objData[i].stat & (OM_STAT_DELETED|OM_STAT_NOPAUSE)) == 0) {
                omResetStatBit(&objWork->objData[i], OM_STAT_PAUSED);
            }
        }
        MBPauseFlagReset();
    }
    HuPrcAllPause(pauseF);
    Hu3DPauseSet(pauseF);
    HuSprPauseSet(pauseF);
    HuAudFXPauseAll(pauseF);
    HuAudSeqPauseAll(pauseF);
    HuAudSStreamPauseAll(pauseF);
}