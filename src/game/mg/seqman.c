#include "game/mg/seqman.h"
#include "game/object.h"
#include "game/mg/timer.h"
#include "game/gamemes.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/flag.h"


typedef struct MgSeqManWork_s {
    MGSEQ_PARAM param; 
    MGTIMER* timer;
    u16 statBit;
    s16 unk2E;
    u16 mode;
    s16 unk32;
    int recordVal;
    s16 winner[4];
    s16 modeDelayTbl[MGSEQ_MODE_MAX];
    u16 modeHookNum;
    MGSEQ_MODEHOOK modeHook[MGSEQ_MODEHOOK_MAX];
} MgSeqManWork;


static MgSeqManWork seqWork;
static HUPROCESS *seqProc;
static unsigned int seqFrameNo;
static s16 defModeDelayTbl[MGSEQ_MODE_MAX] = { -1, -1, 240, -1, -1, -1, -1, 240, 210, 60, -1, -1 };
static GAMEMESID seqGameMesId;

static void SeqExecProc();   
static void SeqExit();
static void SeqExecModeHooks(s16 mode, s16 frameNo);
static void SeqSetTimerPos(s32 pos, MGTIMER *timer);

void MgSeqCreate(MGSEQ_PARAM* param)
{
    MgSeqCreatePrio(param, 100);
}

void MgSeqCreatePrio(MGSEQ_PARAM* param, s32 prio)
{
    s16 i;
    seqWork.param = *param;
    seqWork.statBit = 0;
    seqWork.unk32 = 0;
    seqWork.mode = MGSEQ_MODE_NONE;
    seqWork.timer = NULL;
    seqWork.modeHookNum = 0;
    for (i = 0; i < MGSEQ_MODE_MAX; i++) {
        seqWork.modeDelayTbl[i] = (u16)(double)defModeDelayTbl[i];
    }

    seqProc = HuPrcChildCreate(SeqExecProc, prio, 0x3000, 0, HuPrcCurrentGet());
    HuPrcChildCreate(SeqExit, prio, 0x3000, 0, seqProc);
    seqGameMesId = GAMEMES_ID_NONE;
}

void MgSeqKill()
{
    seqWork.mode = MGSEQ_MODE_NONE;
    HuPrcKill(seqProc);
}

static void SeqExecProc()
{
    MGSEQ_FUNC hook;
    int delay;

    while (1) {
        if (seqWork.mode == MGSEQ_MODE_NONE) {
            seqWork.mode = MGSEQ_MODE_INIT;
            delay = 0;
            if (seqWork.modeDelayTbl[MGSEQ_MODE_INIT] == -1) {
                seqWork.statBit |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                delay = seqWork.modeDelayTbl[MGSEQ_MODE_INIT];
            }
            for (seqFrameNo = 0; seqFrameNo < delay || (seqWork.statBit & MGSEQ_STAT_MODECHANGE_OFF); seqFrameNo++) {
                if (seqWork.param.initHook) {
                    hook = seqWork.param.initHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if ((seqWork.statBit & MGSEQ_STAT_MODENEXT)) {
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
            }
        }
        seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if ((seqWork.mode == MGSEQ_MODE_INIT) || (seqWork.mode == MGSEQ_MODE_FADEIN)) {
            seqWork.mode = MGSEQ_MODE_FADEIN;
            if ((seqWork.statBit & MGSEQ_STAT_FADEIN_OFF) == 0) {
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 60);
            }
            delay = 0;
            if (seqWork.modeDelayTbl[MGSEQ_MODE_FADEIN] == -1) {
                seqWork.statBit |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                delay = seqWork.modeDelayTbl[MGSEQ_MODE_FADEIN];
            }
            for (seqFrameNo = 0; seqFrameNo < delay || seqWork.statBit & MGSEQ_STAT_MODECHANGE_OFF; seqFrameNo++) {
                 if (seqWork.param.fadeInHook != 0) {
                    hook = seqWork.param.fadeInHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if ((seqWork.statBit & MGSEQ_STAT_MODENEXT) != 0) {
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
            }
        }
        seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if ((seqWork.mode == MGSEQ_MODE_FADEIN) || (seqWork.mode == MGSEQ_MODE_START)) {
            seqWork.mode = MGSEQ_MODE_START;
            seqGameMesId = GameMesMgStartCreate();
            if ((seqWork.param.maxTime != 0) && (seqWork.param.maxTime != 300)) {
                seqWork.timer = MgTimerCreate(0);
                MgTimerParamSet(seqWork.timer, seqWork.param.maxTime * 60, 0, 0);
                SeqSetTimerPos(seqWork.param.timerPos, seqWork.timer);
            }
            for (seqFrameNo = 0; GameMesStatGet(seqGameMesId); seqFrameNo++) {
                if (seqWork.param.startHook) {
                    hook = seqWork.param.startHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if ((seqWork.statBit & MGSEQ_STAT_MODENEXT)) {
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
            }
            if (seqWork.timer) {
                MgTimerModeOnSet(seqWork.timer, MGTIMER_OFFTYPE_FADEOUT);
            }
            seqFrameNo = 0;
            SeqExecModeHooks(MGSEQ_MODE_PREMAIN, seqFrameNo);
        }
        seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if ((seqWork.mode == MGSEQ_MODE_START) || (seqWork.mode == MGSEQ_MODE_MAIN)) {
            seqWork.mode = MGSEQ_MODE_MAIN;
            seqFrameNo = 0;
            while (1) {
                if (seqWork.param.mainHook) {
                    hook = seqWork.param.mainHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if ((seqWork.param.maxTime == 300) && (seqFrameNo == 16140)) {
                    seqWork.timer = MgTimerCreate(0);
                    MgTimerParamSet(seqWork.timer, 1800, 0, 0);
                    SeqSetTimerPos(seqWork.param.timerPos, seqWork.timer);
                    MgTimerModeOnSet(seqWork.timer, MGTIMER_OFFTYPE_FADEOUT);
                }
                if (seqWork.timer && MgTimerDoneCheck(seqWork.timer) || (seqWork.statBit & MGSEQ_STAT_MODENEXT)) {
                    HuPrcVSleep();
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        if (seqWork.timer && (MgTimerDoneCheck(seqWork.timer) == 0)) {
            if (!seqWork.timer->stopF) {
                seqWork.timer->stopF = TRUE;
                seqWork.timer->mode = MGTIMER_MODE_ON;
            }
            seqWork.timer = NULL;
        }
        seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if ((seqWork.mode == MGSEQ_MODE_MAIN) || (seqWork.mode == MGSEQ_MODE_FINISH)) {
            seqWork.mode = MGSEQ_MODE_FINISH;
            seqGameMesId = GameMesMgFinishCreate();
            for (seqFrameNo = 0; GameMesStatGet(seqGameMesId); seqFrameNo++) {
                if (seqWork.param.finishHook) {
                    hook = seqWork.param.finishHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if (seqWork.statBit & MGSEQ_STAT_MODENEXT) {
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                } 
                HuPrcVSleep();
            }
        }
        seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if ((seqWork.mode == MGSEQ_MODE_FINISH) || (seqWork.mode == MGSEQ_MODE_PREWIN)) {
            if ((seqWork.mode != MGSEQ_MODE_FINISH) || (seqFrameNo >= 90)) {
                seqFrameNo = 0;
            }
            seqWork.mode = MGSEQ_MODE_PREWIN;
            delay = 0;
            if (seqWork.modeDelayTbl[MGSEQ_MODE_PREWIN] == -1) {
                seqWork.statBit |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                delay = seqWork.modeDelayTbl[MGSEQ_MODE_PREWIN];
            }
              for (; seqFrameNo < delay || (seqWork.statBit & MGSEQ_STAT_MODECHANGE_OFF); seqFrameNo++) {
                if (seqWork.param.preWinnerHook) {
                    hook = seqWork.param.preWinnerHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if (((seqWork.statBit & MGSEQ_STAT_RECORD)) && (seqFrameNo == 90)) {
                    if (delay == defModeDelayTbl[MGSEQ_MODE_PREWIN]) {
                        delay = 270;
                    }
                    GameMesRecordCreate(seqWork.recordVal);
                }
                if ((seqWork.statBit & MGSEQ_STAT_MODENEXT)) {
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
            }
        }
        seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if ((seqWork.mode == MGSEQ_MODE_PREWIN) || (seqWork.mode == MGSEQ_MODE_WINNER)) {
            seqWork.mode = MGSEQ_MODE_WINNER;
            if ((seqWork.statBit & MGSEQ_STAT_WINNER) != 0) {
                if ((seqWork.winner[0] == GAMEMES_MG_WINNER_NONE) && (seqWork.winner[1] == GAMEMES_MG_WINNER_NONE) && (seqWork.winner[2] == GAMEMES_MG_WINNER_NONE) && (seqWork.winner[3] == GAMEMES_MG_WINNER_NONE)) {
                    GameMesMgDrawCreate();
                    HuAudJinglePlay(MSM_STREAM_JNGL_MG_DRAW);
                } else {
                    GameMesMgWinnerCreate(seqWork.winner[0], seqWork.winner[1], seqWork.winner[2], seqWork.winner[3]);
                    if (omcurovl == DLL_sd00dll) {
                        HuAudJinglePlay(MSM_STREAM_JNGL_SD_WIN);
                    } else {
                        HuAudJinglePlay(MSM_STREAM_JNGL_MG_WIN);
                    }
                }
            }
            delay = 0;
            if (seqWork.modeDelayTbl[MGSEQ_MODE_WINNER] == -1) {
                seqWork.statBit |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                delay = seqWork.modeDelayTbl[MGSEQ_MODE_WINNER];
            }
            for (seqFrameNo = 0; seqFrameNo < delay || seqWork.statBit & MGSEQ_STAT_MODECHANGE_OFF; seqFrameNo++) {
                if (seqWork.param.winnerHook) {
                    hook = seqWork.param.winnerHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if (seqWork.statBit & MGSEQ_STAT_MODENEXT) {
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
            }
        }
        seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if ((seqWork.mode == MGSEQ_MODE_WINNER) || (seqWork.mode == MGSEQ_MODE_FADEOUT)) {
            seqWork.mode = MGSEQ_MODE_FADEOUT;
            if ((seqWork.statBit & MGSEQ_STAT_FADEOUT_OFF) == 0) {
                if (!_CheckFlag(FLAG_INST_DECA)) {
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
                } else {
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
                }
                HuAudFadeOut(1000);
            }
            delay = 0;
            if (seqWork.modeDelayTbl[MGSEQ_MODE_FADEOUT] == -1) {
                seqWork.statBit |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                delay = seqWork.modeDelayTbl[MGSEQ_MODE_FADEOUT];
            }
            for (seqFrameNo = 0; seqFrameNo < delay || seqWork.statBit & MGSEQ_STAT_MODECHANGE_OFF; seqFrameNo++) {
                if (seqWork.param.fadeOutHook) {
                    hook = seqWork.param.fadeOutHook;
                    hook(seqWork.mode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.mode, seqFrameNo);
                if (seqWork.statBit & MGSEQ_STAT_MODENEXT) {
                    seqWork.statBit &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
            }
        }
        if (seqWork.mode == MGSEQ_MODE_FADEOUT || seqWork.mode == MGSEQ_MODE_CLOSE) {
            break;
        }
    }
    seqWork.mode = MGSEQ_MODE_CLOSE;
    seqFrameNo = 0;
    if (seqWork.param.closeHook) {
        hook = seqWork.param.closeHook;
        hook(seqWork.mode, seqFrameNo);
    }
    SeqExecModeHooks(seqWork.mode, seqFrameNo);
    omOvlReturn(1);
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

static void SeqExit() {
    MGSEQ_FUNC hook;

    while (!omSysExitReq) {
        HuPrcVSleep();
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
    HuAudFadeOut(1000);
    while (WipeCheckEnd()) {
        HuPrcVSleep();
    }
    if (seqWork.param.closeHook) {
        hook = seqWork.param.closeHook;
        hook(MGSEQ_MODE_EXIT, 0);
    }
    SeqExecModeHooks(MGSEQ_MODE_EXIT, 0);
    omOvlReturn(1);
    HuPrcKill(seqProc);
    while (1) {
        HuPrcVSleep();   
    }
}

static void SeqExecModeHooks(s16 mode, s16 frameNo) {
    s16 i;
    MGSEQ_FUNC hook;

    for (i = 0; i < seqWork.modeHookNum; i++) {
        if (seqWork.modeHook[i].mode == mode) {
            hook = seqWork.modeHook[i].hook;
            hook(mode, frameNo);
        }
    }
}

static void SeqSetTimerPos(s32 pos, MGTIMER *timer) {
    switch (pos) {                                  
        case MGSEQ_TIMER_BOTTOM:
            MgTimerPosSet(timer, 288.0f, 420.0f);
            break;
            
        case MGSEQ_TIMER_RIGHT:
            MgTimerPosSet(timer, 525.0f, 64.0f);
            break;
    }
}

u32 MgSeqModeGet() {
    return seqWork.mode;
}

u32 MgSeqModeNext() {
    seqWork.statBit |= MGSEQ_STAT_MODENEXT;
    return seqWork.mode;
}

u32 MgSeqModeSet(u32 mode) {
    if (mode > MGSEQ_MODE_CLOSE) {
        mode = MGSEQ_MODE_CLOSE;
    }
    seqWork.mode = mode;
    seqWork.statBit |= MGSEQ_STAT_MODENEXT;
    return seqWork.mode;
}

u16 MgSeqModeChangeOff() {
    seqWork.statBit |= MGSEQ_STAT_MODECHANGE_OFF;
    return seqWork.statBit;
}

u16 MgSeqModeChangeOn() {
    seqWork.statBit &= ~MGSEQ_STAT_MODECHANGE_OFF;
    return seqWork.statBit;
}

s32 MgSeqTimerValueGet() {
    if (!seqWork.timer) {
        return seqWork.param.maxTime * 60;
    }
    return MgTimerValueGet(seqWork.timer);
}

void MgSeqRecordSet(int recordVal) {
    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
        seqWork.statBit |= MGSEQ_STAT_RECORD;
        seqWork.recordVal = recordVal;
    }
}

u16 MgSeqStatGet() {
    return seqWork.statBit;
}

u16 MgSeqWinnerSet(s16 charNo1, s16 charNo2, s16 charNo3, s16 charNo4) {
    seqWork.winner[0] = charNo1;
    seqWork.winner[1] = charNo2;
    seqWork.winner[2] = charNo3;
    seqWork.winner[3] = charNo4;
    seqWork.statBit |= MGSEQ_STAT_WINNER;
    return seqWork.statBit;
}

u16 MgSeqDrawSet() {
    seqWork.winner[0] = seqWork.winner[1] = seqWork.winner[2] = seqWork.winner[3] = GAMEMES_MG_WINNER_NONE;
    seqWork.statBit |= MGSEQ_STAT_WINNER;
    return seqWork.statBit;
}

void fn_8006C2E0() {
    seqWork.statBit |= MGSEQ_STAT_UNKBIT;
}

void MgSeqModeDelaySet(s16 mode, s16 delay) {
    if (mode < 0 || mode >= MGSEQ_MODE_MAX) {
        return;
    }
    seqWork.modeDelayTbl[mode] = delay;
}

u32 MgSeqModeHookAdd(s16 mode, MGSEQ_FUNC hook) {
    if (mode < 0 || mode >= MGSEQ_MODE_MAX || (seqWork.modeHookNum >= MGSEQ_MODEHOOK_MAX)) {
        return -1;
    }

    seqWork.modeHook[seqWork.modeHookNum].mode = mode;
    seqWork.modeHook[seqWork.modeHookNum].hook = hook;

    seqWork.modeHookNum++;
    return seqWork.modeHookNum-1;
}

void MgSeqModeHookReset(s16 hook) {
    seqWork.modeHook[hook].mode = MGSEQ_MODEHOOK_NULL;
}

u32 MgSeqFrameNoGet() {
    return seqFrameNo;
}

GAMEMESID MgSeqGameMesIdGet() {
    return seqGameMesId;
}

void MgSeqMaxTimeSet(s16 maxTime) {
    if (seqWork.timer) {
        MgTimerKill(seqWork.timer);
    }
    seqWork.timer = 0;
    seqWork.param.maxTime = maxTime;
}

void MgSeqStatBitSet(u16 bit) {
    seqWork.statBit |= bit;
}

void MgSeqStatBitReset(u16 bit) {
    seqWork.statBit &= ~bit;
}