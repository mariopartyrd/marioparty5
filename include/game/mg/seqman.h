#ifndef _MG_SEQMAN_H
#define _MG_SEQMAN_H

#include "dolphin/types.h"
#include "game/mg/timer.h"
#include "game/gamemes.h"

typedef void (*MGSEQ_FUNC)(s16 mode, s16 frameNo);


#define MGSEQ_MODEHOOK_NULL -1

#define MGSEQ_MODE_NONE 0
#define MGSEQ_MODE_INIT 1
#define MGSEQ_MODE_FADEIN 2
#define MGSEQ_MODE_START 3
#define MGSEQ_MODE_PREMAIN 4

#define MGSEQ_MODE_MAIN 5
#define MGSEQ_MODE_FINISH 6
#define MGSEQ_MODE_PREWIN 7
#define MGSEQ_MODE_WINNER 8
#define MGSEQ_MODE_FADEOUT 9
#define MGSEQ_MODE_CLOSE 10
#define MGSEQ_MODE_EXIT 11
#define MGSEQ_MODE_MAX 12

#define MGSEQ_MODEHOOK_MAX 64

#define MGSEQ_STAT_MODENEXT (1 << 0)
#define MGSEQ_STAT_MODECHANGE_OFF (1 << 1)
#define MGSEQ_STAT_RECORD (1 << 2)
#define MGSEQ_STAT_WINNER (1 << 3)
#define MGSEQ_STAT_UNKBIT (1 << 4)
#define MGSEQ_STAT_FADEIN_OFF (1 << 5)
#define MGSEQ_STAT_FADEOUT_OFF (1 << 6)

#define MGSEQ_TIMER_TOP 0
#define MGSEQ_TIMER_BOTTOM 1
#define MGSEQ_TIMER_RIGHT 2

typedef struct MgSeqParam_s {
    s16 maxTime;
    u8 timerPos;
    MGSEQ_FUNC initHook;
    MGSEQ_FUNC fadeInHook;
    MGSEQ_FUNC startHook;
    MGSEQ_FUNC mainHook;
    MGSEQ_FUNC finishHook;
    MGSEQ_FUNC preWinnerHook;
    MGSEQ_FUNC winnerHook;
    MGSEQ_FUNC fadeOutHook;
    MGSEQ_FUNC closeHook;
} MGSEQ_PARAM;

typedef struct {
    u16 mode;
    MGSEQ_FUNC hook;
} MGSEQ_MODEHOOK;

void MgSeqCreate(MGSEQ_PARAM* param);
void MgSeqCreatePrio(MGSEQ_PARAM* param, s32 prio);
void MgSeqKill();
u16 MgSeqModeGet();
u16 MgSeqModeNext();
u16 MgSeqModeSet(u32 mode);
u16 MgSeqModeChangeOff();
u16 MgSeqModeChangeOn();
s32 MgSeqTimerValueGet();
void MgSeqRecordSet(s32 record);
u16 MgSeqStatGet();
u16 MgSeqWinnerSet(s16 charNo1, s16 charNo2, s16 charNo3, s16 charNo4);
u16 MgSeqDrawSet();
void fn_8006C2E0();
void MgSeqModeDelaySet(s16 mode, s16 delay);
u32 MgSeqModeHookAdd(s16 mode, MGSEQ_FUNC func);
void MgSeqModeHookReset(s16 mode);
u32 MgSeqFrameNoGet();
GMESID MgSeqGMesIdGet();
void MgSeqMaxTimeSet(s16 maxTime);
void MgSeqStatBitSet(u16 bit);
void MgSeqStatBitReset(u16 bit);

#endif