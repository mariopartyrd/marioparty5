#include "game/mg/seqman.h"
#include "game/object.h"
#include "game/mg/timer.h"
#include "game/gamemes.h"
#include "game/audio.h"
#include "game/wipe.h"



static MgSeqManWork  seqWork;
HUPROCESS *seqProc;
u32 seqFrameNo;
static s16 defModeDelayTbl[0xC] = { -1, -1, 0xF0, -1, -1, -1, -1, 0xF0, 0xD2, 0x3C, -1, -1 };
s16 timerMes;

void SeqExecProc();   
void SeqExit();
void MgSeqCreatePrio(MGSEQMAN* arg0, s32 arg1);
void SeqExecModeHooks(s16 arg0, s16 arg1);
void SeqSetTimerPos(s32 arg0, MGTIMER *arg1);

void MgSeqCreate(MGSEQMAN* arg0) {
    MgSeqCreatePrio(arg0, 0x64);
}

void MgSeqCreatePrio(MGSEQMAN* arg0, s32 arg1) {
    s16 i;
    seqWork.unk_0 = *arg0;
    seqWork.unk_2C = 0;
    seqWork.unk_32 = 0;
    seqWork.unk_30 = 0;
    seqWork.unk_28 = NULL;
    seqWork.unk_58 = 0;
    for (i = 0; i < 0xC; i++) {
        seqWork.unk_40[i] = (u16)(double)defModeDelayTbl[i];
    }

    seqProc = HuPrcChildCreate(SeqExecProc, arg1, 0x3000, 0, HuPrcCurrentGet());
    HuPrcChildCreate(SeqExit, arg1, 0x3000, 0, seqProc);
    timerMes = -1;
}

void MgSeqKill() {
    seqWork.unk_30 = 0;
    HuPrcKill(seqProc);
}

void SeqExecProc() {
    SEQMAN_FUNC* func;
    s32 var_r30;
    f32 temp_f1;

    while (1) {
         if (!seqWork.unk_30) {
        seqWork.unk_30 = 1;
        var_r30 = 0;
        if (seqWork.unk_40[1] == -1) {
            seqWork.unk_2C |= 2;
        } else {
            var_r30 = seqWork.unk_40[1];
        }
        for (seqFrameNo = 0; seqFrameNo < var_r30 || (seqWork.unk_2C & 2); seqFrameNo++) {
                if (seqWork.unk_0.unk_4) {
                func = seqWork.unk_0.unk_4;
                func(seqWork.unk_30, seqFrameNo);
            }
            SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
            if ((seqWork.unk_2C & 1)) {
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
            }
            HuPrcVSleep();
        }
    }
    seqWork.unk_2C &= 0xFFFFFFFD;
    if ((seqWork.unk_30 == 1) || (seqWork.unk_30 == 2)) {
        seqWork.unk_30 = 2;
        if ((seqWork.unk_2C & 0x20) == 0) {
            WipeCreate(1, 5, 0x3C);
        }
        var_r30 = 0;
        if (seqWork.unk_40[2] == -1) {
            seqWork.unk_2C |= 2;
        } else {
            var_r30 = seqWork.unk_40[2];
        }
        for (seqFrameNo = 0; seqFrameNo < var_r30 || seqWork.unk_2C & 2; seqFrameNo++) {
             if (seqWork.unk_0.unk_8 != 0) {
                func = seqWork.unk_0.unk_8;
                func(seqWork.unk_30, seqFrameNo);
            }
            SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
            if ((seqWork.unk_2C & 1) != 0) {
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
            }
                HuPrcVSleep();
        }
    }
    seqWork.unk_2C &= 0xFFFFFFFD;
    if ((seqWork.unk_30 == 2) || (seqWork.unk_30 == 3)) {
        seqWork.unk_30 = 3;
        timerMes = GMesCreate(2, 0);
        if ((seqWork.unk_0.unk_0 != 0) && (seqWork.unk_0.unk_0 != 0x12C)) {
            seqWork.unk_28 = MgTimerCreate(0);
            MgTimerParamSet(seqWork.unk_28, seqWork.unk_0.unk_0 * 0x3C, 0, 0);
            SeqSetTimerPos(seqWork.unk_0.unk_2, seqWork.unk_28);
        }
        for (seqFrameNo = 0; GMesStatGet(timerMes); seqFrameNo++) {
            if (seqWork.unk_0.unk_C) {
                func = seqWork.unk_0.unk_C;
                func(seqWork.unk_30, seqFrameNo);
            }
            SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
            if ((seqWork.unk_2C & 1)) {
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
            }
                HuPrcVSleep();
        }
        if (seqWork.unk_28) {
            MgTimerModeOnSet(seqWork.unk_28, 1);
        }
        seqFrameNo = 0;
        SeqExecModeHooks(4, seqFrameNo);
    }
    seqWork.unk_2C &= 0xFFFFFFFD;
        if ((seqWork.unk_30 == 3) || (seqWork.unk_30 == 5)) {
        seqWork.unk_30 = 5;
        seqFrameNo = 0;
        while (1) {

        if (seqWork.unk_0.unk_10) {
            func = seqWork.unk_0.unk_10;
            func(seqWork.unk_30, seqFrameNo);
        }
        SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
        if ((seqWork.unk_0.unk_0 == 0x12C) && (seqFrameNo == 0x3F0C)) {
            seqWork.unk_28 = MgTimerCreate(0);
            MgTimerParamSet(seqWork.unk_28, 0x708, 0, 0);
            SeqSetTimerPos(seqWork.unk_0.unk_2,  seqWork.unk_28);
            MgTimerModeOnSet(seqWork.unk_28, 1);
        }
        if (seqWork.unk_28 && MgTimerDoneCheck(seqWork.unk_28) || (seqWork.unk_2C & 1)) {
                HuPrcVSleep();
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
        }
        HuPrcVSleep();
        seqFrameNo++;
        }
        }
    if (seqWork.unk_28 && (MgTimerDoneCheck(seqWork.unk_28) == 0)) {
        if (!seqWork.unk_28->stopF) {
            seqWork.unk_28->stopF = 1;
            seqWork.unk_28->mode = 1;
        }
        seqWork.unk_28 = NULL;
    }
    seqWork.unk_2C &= 0xFFFFFFFD;
    if ((seqWork.unk_30 == 5) || (seqWork.unk_30 == 6)) {
        seqWork.unk_30 = 6;
        timerMes = GMesCreate(2, 1);
        for (seqFrameNo = 0; GMesStatGet(timerMes); seqFrameNo++) {
            if (seqWork.unk_0.unk_14) {
                func = seqWork.unk_0.unk_14;
                func(seqWork.unk_30, seqFrameNo);
            }
            SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
            if (seqWork.unk_2C & 1) {
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
            } 
            HuPrcVSleep();
        }
    }
    seqWork.unk_2C &= 0xFFFFFFFD;
    if ((seqWork.unk_30 == 6) || (seqWork.unk_30 == 7)) {
        if ((seqWork.unk_30 != 6) || (seqFrameNo >= 0x5A)) {
            seqFrameNo = 0;
        }
        seqWork.unk_30 = 7;
        var_r30 = 0;
        if (seqWork.unk_40[7] == -1) {
            seqWork.unk_2C |= 2;
        } else {
            var_r30 = seqWork.unk_40[7];
        }
          for (; seqFrameNo < var_r30 || (seqWork.unk_2C & 2); seqFrameNo++) {
            if (seqWork.unk_0.unk_18) {
                func = seqWork.unk_0.unk_18;
                func(seqWork.unk_30, seqFrameNo);
            }
            SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
            if (((seqWork.unk_2C & 4)) && (seqFrameNo == 0x5A)) {
                if (var_r30 == defModeDelayTbl[7]) {
                    var_r30 = 0x10E;
                }
                GMesCreate(0xE, seqWork.unk_34);
            }
            if ((seqWork.unk_2C & 1)) {
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
            }
            HuPrcVSleep();
        }
    }
    seqWork.unk_2C &= 0xFFFFFFFD;
    if ((seqWork.unk_30 == 7) || (seqWork.unk_30 == 8)) {
        seqWork.unk_30 = 8;
        if ((seqWork.unk_2C & 8) != 0) {
            if ((seqWork.unk_38[0] == -1) && (seqWork.unk_38[1] == -1) && (seqWork.unk_38[2] == -1) && (seqWork.unk_38[3] == -1)) {
                GMesCreate(2, 2);
                HuAudJinglePlay(123);
            } else {
                GMesCreate(4, 3, seqWork.unk_38[0], seqWork.unk_38[1], seqWork.unk_38[2], seqWork.unk_38[3]);
                if (omcurovl == DLL_sd00dll) {
                    HuAudJinglePlay(0x89);
                } else {
                    HuAudJinglePlay(0x79);
                }
            }
        }
        var_r30 = 0;
        if (seqWork.unk_40[8] == -1) {
            seqWork.unk_2C |= 2;
        } else {
            var_r30 = seqWork.unk_40[8];
        }
        for (seqFrameNo = 0; seqFrameNo < var_r30 || seqWork.unk_2C & 2; seqFrameNo++) {
            if (seqWork.unk_0.unk_1C) {
                func = seqWork.unk_0.unk_1C;
                func(seqWork.unk_30, seqFrameNo);
            }
            SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
            if (seqWork.unk_2C & 1) {
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
            }
                HuPrcVSleep();
        }
    }
    seqWork.unk_2C &= 0xFFFFFFFD;
    if ((seqWork.unk_30 == 8) || (seqWork.unk_30 == 9)) {
        seqWork.unk_30 = 9;
        if ((seqWork.unk_2C & 0x40) == 0) {
            if (_CheckFlag(0x30002) == 0) {
                WipeCreate(2, 6, 0x3C);
            } else {
                WipeCreate(2, 6, 0x3C);
            }
            HuAudFadeOut(0x3E8);
        }
        var_r30 = 0;
        if (seqWork.unk_40[9] == -1) {
            seqWork.unk_2C |= 2;
        } else {
            var_r30 = seqWork.unk_40[9];
        }
        for (seqFrameNo = 0; seqFrameNo < var_r30 || seqWork.unk_2C & 2; seqFrameNo++) {
            if (seqWork.unk_0.unk_20) {
                func = seqWork.unk_0.unk_20;
                func(seqWork.unk_30, seqFrameNo);
            }
            SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
            if (seqWork.unk_2C & 1) {
                seqWork.unk_2C &= 0xFFFFFFFE;
                break;
            }
            HuPrcVSleep();
        }
    }
        if (seqWork.unk_30 == 9 || seqWork.unk_30 == 10) {
                break;
        }
    }
    seqWork.unk_30 = 0xA;
    seqFrameNo = 0;
    if (seqWork.unk_0.unk_24) {
        func = seqWork.unk_0.unk_24;
        func(seqWork.unk_30, seqFrameNo);
    }
    SeqExecModeHooks(seqWork.unk_30, seqFrameNo);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

void SeqExit() {
    SEQMAN_FUNC* func;

    while (!omSysExitReq) {
        HuPrcVSleep();
    }
    WipeCreate(2, 6, 0x3C);
    HuAudFadeOut(0x3E8);
    while (WipeCheckEnd()) {
        HuPrcVSleep();
    }
    if (seqWork.unk_0.unk_24) {
        func = seqWork.unk_0.unk_24;
        func(0xB,0);
    }
    SeqExecModeHooks(0xB, 0);
    omOvlReturnEx(1, 1);
    HuPrcKill(seqProc);
    while (1) {
        HuPrcVSleep();   
    }
}

void SeqExecModeHooks(s16 arg0, s16 arg1) {
    s16 var_r31;
    SEQMAN_FUNC* var_r30;

    for (var_r31 = 0; var_r31 < seqWork.unk_58; var_r31++) {
        if (seqWork.unk_5C[var_r31].a == arg0) {
            var_r30 = seqWork.unk_5C[var_r31].b;
            var_r30(arg0, arg1);
        }
    }
}

void SeqSetTimerPos(s32 arg0, MGTIMER *arg1) {
    switch (arg0) {                                  
    case 1:
        MgTimerPosSet(arg1, 288.0f, 420.0f);
        break;
    case 2:
        MgTimerPosSet(arg1, 525.0f, 64.0f);
    }
}

u16 MgSeqModeGet() {
    return seqWork.unk_30;
}

u16 MgSeqModeNext() {
    seqWork.unk_2C |=  1;
    return seqWork.unk_30;
}

u16 MgSeqModeSet(u32 arg0) {
    if (arg0 > 10) {
        arg0 = 10;
    }
    seqWork.unk_30 = arg0;
    seqWork.unk_2C |= 1;
    return seqWork.unk_30;
}

u16 MgSeqModeChangeOff() {
    seqWork.unk_2C |= 2;
    return seqWork.unk_2C;
}

u16 MgSeqModeChangeOn() {
    seqWork.unk_2C &= 0xFFFFFFFD;
    return seqWork.unk_2C;
}

s32 MgSeqTimerValueGet() {
    if (!seqWork.unk_28) {
        return seqWork.unk_0.unk_0 * 0x3C;
    }
    return MgTimerValueGet(seqWork.unk_28);
}

void MgSeqRecordSet(s32 arg0) {
    if (_CheckFlag(0x1000F) == 0) {
        seqWork.unk_2C |=  4;
        seqWork.unk_34 = arg0;
    }
}

u16 MgSeqStatGet() {
    return seqWork.unk_2C;
}

u16 MgSeqWinSet(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    seqWork.unk_38[0] = arg0;
    seqWork.unk_38[1]  = arg1;
    seqWork.unk_38[2] = arg2;
    seqWork.unk_38[3] = arg3;
    seqWork.unk_2C |=  8;
    return seqWork.unk_2C;
}

u16 MgSeqDrawSet() {
    seqWork.unk_38[0] = seqWork.unk_38[1] = seqWork.unk_38[2] = seqWork.unk_38[3] = -1;
    seqWork.unk_2C |= 8;
    return seqWork.unk_2C;
}

void fn_8006C2E0() {
    seqWork.unk_2C |=  16;
}

void MgSeqModeDelaySet(s16 arg0, s16 arg1) {
    if (arg0 < 0 || arg0 >= 0xC) {
        return;
    }
    seqWork.unk_40[arg0] = arg1;
}

u32 MgSeqModeHookAdd(s16 arg0, SEQMAN_FUNC* arg1) {
    if ((arg0 < 0) || (arg0 >= 0xC) || (seqWork.unk_58 >= 0x40)) {
        return -1;
    }

    seqWork.unk_5C[seqWork.unk_58].a = arg0;
    seqWork.unk_5C[seqWork.unk_58].b = arg1;

    seqWork.unk_58++;
    return seqWork.unk_58 - 1;
}

void MgSeqModeHookReset(s16 arg0) {
    seqWork.unk_5C[arg0].a = -1;
}

u32 MgSeqFrameNoGet() {
    return seqFrameNo;
}

s16 MgSeqTimerMesGet() {
    return timerMes;
}

void MgSeqTimerKill(s16 arg0) {
    if (seqWork.unk_28) {
        MgTimerKill(seqWork.unk_28);
    }
    seqWork.unk_28 = 0;
    seqWork.unk_0.unk_0 = arg0;
}

void MgSeqStatBitSet(u16 arg0) {
    seqWork.unk_2C |=  arg0;
}

void MgSeqStatBitReset(s32 arg0) {
    seqWork.unk_2C &= ~arg0;
}