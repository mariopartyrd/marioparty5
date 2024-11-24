#include "game/board/main.h"
#include "game/board/pause.h"
#include "game/armem.h"
#include "game/wipe.h"
#include "game/flag.h"
#include "game/sprite.h"
#include "game/hu3d.h"

static BOOL killF;
static BOOL killSendF;
static HUPROCESS *pauseWatch;

static void PauseWatchProc(void);
static void KillPauseWatchProc(void);

void MBPauseWatchProcCreate(void)
{
    pauseWatch = HuPrcCreate(PauseWatchProc, 8210, 8192, 0);
    HuPrcSetStat(pauseWatch, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    HuPrcDestructorSet2(pauseWatch, KillPauseWatchProc);
    killF = FALSE;
    killSendF = FALSE;
}

static void PauseWatchProc(void)
{
    while(!killSendF && !omSysExitReq) {
        s32 playerNo = MBPauseStartCheck();
        if(playerNo >= 0) {
            MBPauseProcCreate(playerNo);
        }
        HuPrcVSleep();
    }
    killF = TRUE;
    HuPrcVSleep();
    while(HuARDMACheck()) {
        HuPrcVSleep();
    }
    WipeWait();
    HuPrcKill(mbObjMan);
    HuPrcEnd();
}

static void KillPauseWatchProc(void)
{
    pauseWatch = NULL;
}

void MBPauseWatchProcStop(void)
{
    killSendF = TRUE;
    if(_CheckFlag(FLAG_BOARD_PAUSE)) {
        MBPauseFlagReset();
        omSysPauseCtrl(FALSE);
        HuPrcAllPause(FALSE);
        Hu3DPauseSet(FALSE);
        HuSprPauseSet(FALSE);
    }
}

BOOL MBKillCheck(void)
{
    return killF;
}

HUPROCESS *MBPauseWatchProcGet(void)
{
    return pauseWatch;
}