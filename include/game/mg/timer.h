#ifndef _MINIGAME_TIMER_H
#define _MINIGAME_TIMER_H

#include "dolphin/types.h"
#include "game/process.h"
#include "game/sprite.h"


typedef struct Unk_Timer {
    s32 unk_00;
    s32 unk_04;
} Unk_Timer;


typedef struct timer_s_0x00 {
    s32 unk_0;
    s32 unk_4;
    s32 unk_8;
    s32 unk_C;
    s32 unk_10;
    s32 unk_14;
    s32 unk_18;
    s32 unk_1C;
    u8 unk_20;
    s32 unk_24;
    s32 unk_28;
    s32 unk_2C;
    s32 unk_30;
    s32 unk_34;
    s32 unk_38;
    f32 unk_3C;
    f32 unk_40;
    s16 unk_44;
    s16 unk_46;
    s16 unk_48;
    s16 unk_4A[15];
    f32 unk_68;
    HUPROCESS* unk_6C;
    u8 unk_70;
    u8 unk_71;
    u8 unk_72;
} timer_s;

timer_s* MgTimerCreate(s32 arg0);
void MgTimerKill(timer_s* arg0);
s32 MgTimerModeGet(s32* arg0);
void MgTimerParamSet(timer_s* arg0, s32 arg1, s32 arg2, s32 arg3);
s32 MgTimerValueGet(timer_s* arg0);
void MgTimerPosSet(timer_s* arg0, f32 arg8, f32 arg9);
void MgTimerPosGet(timer_s* arg0, f32* arg1, f32* arg2);
void MgTimerModeOnSet(timer_s* arg0, s32 arg1);
void MgTimerModeOffSet(s32* arg0);
BOOL MgTimerDoneCheck(timer_s* arg0);
void MgTimerRecordSet(timer_s* arg0, s32 arg1);
void MgTimerDigitColorSet(timer_s* arg0, u8 arg1, u8 arg2, u8 arg3);
void MgTimerWinColorSet(timer_s* arg0, u8 arg1, u8 arg2, u8 arg3);
void MgTimerDispOn(timer_s* arg0);
void MgTimerDispOff(timer_s* arg0);
void MgTimerRecordDispOn(timer_s* arg0);
void MgTimerRecordDispOff(timer_s* arg0);


#endif