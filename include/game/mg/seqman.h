#ifndef _MG_SEQMAN_H
#define _MG_SEQMAN_H

#include "dolphin/types.h"
#include "game/mg/timer.h"




typedef void (SEQMAN_FUNC)(s16 arg0, s16 arg1);

typedef struct MgSeqman_s {
    s16 unk_0;
    u8 unk_2;
    SEQMAN_FUNC* unk_4;
    SEQMAN_FUNC* unk_8;
    SEQMAN_FUNC* unk_C;
    SEQMAN_FUNC* unk_10;
    SEQMAN_FUNC* unk_14;
    SEQMAN_FUNC* unk_18;
    SEQMAN_FUNC* unk_1C;
    SEQMAN_FUNC* unk_20;
    SEQMAN_FUNC* unk_24;
} MGSEQMAN;

typedef struct {
    u16 a;
    SEQMAN_FUNC* b;
} SEQMAN_UNK;
    
typedef struct MgSeqManWork_s {
    MGSEQMAN unk_0; 
    MGTIMER* unk_28;
    u16 unk_2C;
        s16 unk_2E;
        u16 unk_30;
        s16 unk_32;
        s32 unk_34;
        s16 unk_38[4];
        s16 unk_40[12];
        u16 unk_58;
        s16 unk_5A;
        SEQMAN_UNK unk_5C[1];
} MgSeqManWork;

void MgSeqCreate(MGSEQMAN* arg0);
void MgSeqCreatePrio(MGSEQMAN* arg0, s32 arg1);
void MgSeqKill();
u16 MgSeqModeGet();
u16 MgSeqModeNext();
u16 MgSeqModeSet(u32 arg0);
u16 MgSeqModeChangeOff();
u16 MgSeqModeChangeOn();
s32 MgSeqTimerValueGet();
void MgSeqRecordSet(s32 arg0);
u16 MgSeqStatGet();
u16 MgSeqWinSet(s16 arg0, s16 arg1, s16 arg2, s16 arg3);
u16 MgSeqDrawSet();
void fn_8006C2E0();
void MgSeqModeDelaySet(s16 arg0, s16 arg1);
u32 MgSeqModeHookAdd(s16 arg0, SEQMAN_FUNC* arg1);
void MgSeqModeHookReset(s16 arg0);
u32 MgSeqFrameNoGet();
s16 MgSeqTimerMesGet();
void MgSeqTimerKill(s16 arg0);
void MgSeqStatBitSet(u16 arg0);
void MgSeqStatBitReset(s32 arg0);

#endif