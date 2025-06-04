#include "dolphin/PPCArch.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "game/process.h"
#include "game/main.h"
#include "game/mg/timer.h"


const static s16 lbl_80289D88[4] = { 
    0, 0x3C, 0x64, 0x3C 
};
const static s16 scoreWinSize[4][2] = { 
    { 0x20, 0x0020 }, 
    { 0x90, 0x0030 }, 
    { 0x60, 0x0020 }, 
    { 0x88, 0x0020 } };
void TimerExec();
void MgTimerDigitColorSet(timer_s* arg0, u8 arg1, u8 arg2, u8 arg3);
void CreateScoreSpr(timer_s* arg0);
void KillScoreSpr(timer_s* arg0);
void UpdateTimerSpr(timer_s* arg0);
void MgTimerDispOn(timer_s* arg0);
void MgTimerDispOff(timer_s* arg0);
void espBankSet(s16 espId, s32 bank);

void TimerExecOn();
void TimerExecOff();
static const float scoreOfsTbl[4][15][2] = {
    {
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f},
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f},
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f},
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}
    },
    {
        {-46.0f,  0.0f}, {-30.0f,  0.0f}, {-14.0f,  0.0f}, {  2.0f,  0.0f},
        { 18.0f,  0.0f}, { 34.0f,  0.0f}, { 50.0f,  0.0f}, {-46.0f, -16.0f},
        {-30.0f, -16.0f}, {-14.0f, -16.0f}, {  2.0f, -16.0f}, { 18.0f, -16.0f},
        { 34.0f, -16.0f}, { 50.0f, -16.0f}, {-66.0f, -16.0f},
    },
    {
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-40.0f, -8.0f}, {-24.0f, -8.0f},
        {-8.0f, -8.0f}, {  8.0f, -8.0f}, { 24.0f, -8.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}
    },
    {
        {-56.0f, -8.0f}, {-40.0f, -8.0f}, {-24.0f, -8.0f}, {-8.0f, -8.0f},
        {  8.0f, -8.0f}, { 24.0f, -8.0f}, { 40.0f, -8.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}
    },
};
static void (*modeTbl[2])(void *) = { TimerExecOff, TimerExecOn};


timer_s* MgTimerCreate(s32 arg0) {
    timer_s* timer;
    s32 i;
    timer = HuMemDirectMallocNum(HEAP_HEAP, 0x74, 0x10000000);
    if (timer == NULL) {
        return NULL;
    }
    timer->unk_8 = arg0;
    timer->unk_0 = 0;
    timer->unk_4 = 0;
    timer->unk_10 = 0;
    timer->unk_1C = 1;
    timer->unk_14 = 0;
    timer->unk_18 = 0;
    timer->unk_20 = 0;
    timer->unk_30 = 0;
    timer->unk_24 = 0;
    timer->unk_34 = 0;
    timer->unk_28 = 0;
    timer->unk_2C = 0;
    timer->unk_3C = 288.0f;
    timer->unk_40 = 64.0f;
    timer->unk_46 = -1;
    timer->unk_48 = -1;
    timer->unk_70 = 0xFF;
    timer->unk_71 = 0xFF;
    timer->unk_72 = 0xFF;
    timer->unk_68 = 1.0f;
    timer->unk_44 = lbl_80289D88[arg0];
    timer->unk_38 = 0;
    for (i = 0; i < 0xF; i++) {
        timer->unk_4A[i] = -1 ;
    }
    CreateScoreSpr(timer);
    switch (arg0) {
        case 0:
        break;
        case 1:
        case 2:
        case 3:
        timer->unk_48 = MgScoreWinCreate(scoreWinSize[timer->unk_8][0], scoreWinSize[timer->unk_8][1]);
        MgScoreWinColorSet(timer->unk_48, 0, 0, 0);
        MgScoreWinTPLvlSet(timer->unk_48, 0.7f);
        MgScoreWinDispSet(timer->unk_48, 0);

    }
    MgTimerDigitColorSet(timer, 0xFF, 0xFF, 0xFF);
    UpdateTimerSpr(timer);
    timer->unk_6C = HuPrcChildCreate(TimerExec, 0x1000, 0x1200, 0, HuPrcCurrentGet());
    timer->unk_6C->property = timer;
    return timer;
}

void MgTimerKill(timer_s* arg0) {
    HuPrcKill(arg0->unk_6C);
    KillScoreSpr(arg0);
    if (arg0->unk_46 != -1) {
        GMesKill(arg0->unk_46);
    }
    HuMemDirectFree(arg0);
}

s32 MgTimerModeGet(s32* arg0) {
    return *arg0;
}

void MgTimerParamSet(timer_s* arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 var_r30;
    s32 var_r29;
    arg0->unk_24 = arg1;
    arg0->unk_30 = arg0->unk_24;
    arg0->unk_28 = arg2;
    arg0->unk_34 = arg3;
    if (arg1 == arg2) {
        var_r30 = 0;
    } else {
        if (arg1 > arg2) {
            var_r29 = -1;
        } else {
            var_r29 = 1;
        }
        var_r30 = var_r29;
    }
    arg0->unk_2C = var_r30;
    UpdateTimerSpr(arg0);
}

s32 MgTimerValueGet(timer_s* arg0) {
    return arg0->unk_30;
}


void MgTimerPosSet(timer_s *arg0, f32 arg8, f32 arg9) {
    arg0->unk_3C = arg8;
    arg0->unk_40 = arg9;
    UpdateTimerSpr(arg0);
}

void MgTimerPosGet(timer_s* arg0, f32* arg1, f32* arg2) {
    *arg1 = arg0->unk_3C;
    *arg2 = arg0->unk_40;
}


void MgTimerModeOnSet(timer_s* arg0, s32 arg1) {
    arg0->unk_C = arg1;
    arg0->unk_14 = 0;
    arg0->unk_0 = 1;
}

void MgTimerModeOffSet(s32* arg0) {
    *arg0 = 0;
}

BOOL MgTimerDoneCheck(timer_s* arg0) {
    return arg0->unk_28 == arg0->unk_30;
}

void MgTimerRecordSet(timer_s* arg0, s32 arg1) {
    if (arg1 == -1) {
        if (((arg0->unk_2C > 0) && (arg0->unk_30 < arg0->unk_34)) || ((arg0->unk_2C < 0) && (arg0->unk_30 > arg0->unk_34))) {
            arg0->unk_34 = arg0->unk_30;
            arg0->unk_20 = arg0->unk_20 | 2;
        }
    } else {
        arg0->unk_34 = arg1;
        arg0->unk_20 = arg0->unk_20 | 2;
    }
}
void espColorSet(s16 espId, u8 r, u8 g, u8 b);

void MgTimerDigitColorSet(timer_s* arg0, u8 arg1, u8 arg2, u8 arg3) {
    s32 i;
    arg0->unk_70 = arg1;
    arg0->unk_71 = arg2;
    arg0->unk_72 = arg3;
    for (i = 0; i <= 6; i++) {
        if (arg0->unk_4A[i] != -1) {
            espColorSet(arg0->unk_4A[i], arg1 & 0xff, arg2 & 0xff, arg3 & 0xff);
        }
    }
}

void MgTimerWinColorSet(timer_s* arg0, u8 arg1, u8 arg2, u8 arg3) {
    if (arg0->unk_48 != -1) {
        MgScoreWinColorSet(arg0->unk_48, arg1, arg2, arg3);
    }
}

void TimerExec(void) {
    timer_s* var_r31;

    var_r31 = HuPrcCurrentGet()->property;
    while (TRUE) {
    modeTbl[var_r31->unk_0](var_r31);
    }
}
void TimerExecOff(timer_s* arg0) {
    s32 var_r31;
    s16 var_r30;
    s32 var_r29;
    s16 var_r28;
    s16 var_r27;

    while (1) {
        UpdateTimerSpr(arg0);
        var_r29 = 1;
        var_r31 = 0;

        while (var_r31 < var_r29) {
            HuPrcVSleep();

            var_r30 = ((Unk_Timer*)HuPrcCurrentGet()->property)->unk_00;
            var_r28 = ((Unk_Timer*)HuPrcCurrentGet()->property)->unk_04;
            ((Unk_Timer*)HuPrcCurrentGet()->property)->unk_04 = var_r30;

            if (var_r30 != var_r28) {
                return;
            }
            var_r31 += 1;
        }
    }
}

void TimerExecOn(timer_s* arg0) {
    s32* var_r31;
    s32 var_r30;
    s16 var_r29;
    s32 var_r28;
    s16 var_r27;
    
    if (arg0->unk_10 == 0) {
        MgTimerDispOn(arg0);
    }
    while ((arg0->unk_30 != arg0->unk_28) && (arg0->unk_14 == 0)) {
        arg0->unk_30 = arg0->unk_30 + arg0->unk_2C;
        UpdateTimerSpr(arg0);
        var_r28 = 1;
        var_r30 = 0;
        while (var_r30 < var_r28) {
            HuPrcVSleep();
            var_r29 = ((Unk_Timer*)HuPrcCurrentGet()->property)->unk_00;
            var_r27 = ((Unk_Timer*)HuPrcCurrentGet()->property)->unk_04;
            ((Unk_Timer*)HuPrcCurrentGet()->property)->unk_04 = var_r29;
            if (var_r29 != var_r27) {
                return;
            }
            var_r30 += 1;
        }
    }
    arg0->unk_14 = 1;
    UpdateTimerSpr(arg0);
    arg0->unk_38 = 0;
    switch (arg0->unk_C) {               
    case 0:
        arg0->unk_0 = 0;
        return;
    case 1:
        if (arg0->unk_8 == 0) {
            MgTimerDispOff(arg0);
        } else {
            arg0->unk_20 = arg0->unk_20 | 4;
        }
        arg0->unk_0 = 0;
        return;
    case 2:
        arg0->unk_20 = arg0->unk_20 | 1;
        arg0->unk_0 = 0;
        break;
    }
}

void CreateScoreSpr(timer_s* arg0) {
    s32 i;

    switch (arg0->unk_8) {
        case 0:
        break;
        case 1:
        case 2:
        case 3:
        for (i = 0; i < 7; i++) {
        arg0->unk_4A[i] = espEntry(0x960030, 0, 0);
        espColorSet(arg0->unk_4A[i], 0xFF, 0xFF, 0xFF);
        }
        for (i = 0; i < 7; i++) {
        arg0->unk_4A[i + 7] = espEntry(0x960030, 0, 0);
        espColorSet(arg0->unk_4A[i + 7], 0x42, 0xFF, 0x7a);
        }
        arg0->unk_4A[14] = espEntry(0x960045, 0, 0);
        espBankSet(arg0->unk_4A[1], 0xA);
        espBankSet(arg0->unk_4A[4], 0xB);
        espBankSet(arg0->unk_4A[8], 0xA);
        espBankSet(arg0->unk_4A[11], 0xB);
        for (i = 0; i < 0xF; i++) {
            if (arg0->unk_4A[i] != -1 ) {
                espDispOff(arg0->unk_4A[i]);
                espPriSet(arg0->unk_4A[i], 9); 
            }
    }
        break;
    }
}

void KillScoreSpr(timer_s* arg0) {
    s32 i;
    for (i = 0; i < 0xF; i++) {
        if (arg0->unk_4A[i] != -1) {
            espKill(arg0->unk_4A[i]);
            arg0->unk_4A[i] = -1;
        }
    }
}

void UpdateTimerSpr(timer_s* arg0) {
    s32 temp_r3;
    s32 i;
    s32 temp_r3_2;
    s32 temp_r3_3;
    s32 temp_r3_5;
    s32 var_r29;
    s32 var_r28;
    s32 var_r27;
    u8 var_r26;
    u8 var_r25;
    u8 var_r24;
    s32 temp_r0_2;
    s32 temp_r0_3;
    f32 var_f31;

    switch (arg0->unk_8) {
        case 0:
        if ((arg0->unk_30 % 60) == 0) {
        GMesDispSet(arg0->unk_46, 1, arg0->unk_30 / 60);
        }
        GMesPosSet(arg0->unk_46, arg0->unk_3C, arg0->unk_40);
        break;
        case 1:
        case 2:
        case 3:
            for (i = 0; i < 15; i++) {
                if (arg0->unk_4A[i] != -1) {
                    espPosSet(arg0->unk_4A[i],  arg0->unk_3C + scoreOfsTbl[arg0->unk_8][i][0], arg0->unk_40 + scoreOfsTbl[arg0->unk_8][i][1]);
                } 
            }
            if (arg0->unk_48 != -1) {
                MgScoreWinPosSet(arg0->unk_48, arg0->unk_3C - 8.0f, arg0->unk_40 - 8.0f);
            }
            var_r27 = (arg0->unk_30) / (arg0->unk_44 * 0x3C);
            var_r29 = (arg0->unk_30 % (arg0->unk_44 * 0x3C)) / 60;
            var_r28 =  1.6666666f * ((arg0->unk_30 % 3600) % 60);
            espBankSet(arg0->unk_4A[0], var_r27 % 10);
            espBankSet(arg0->unk_4A[2], var_r29 / 10);
            espBankSet(arg0->unk_4A[3], var_r29 % 10);
            espBankSet(arg0->unk_4A[5], var_r28 / 10);
            espBankSet(arg0->unk_4A[6], var_r28 % 10);

            var_r27 = arg0->unk_34 / (arg0->unk_44 * 60);
            var_r29 = (arg0->unk_34 % (arg0->unk_44 * 0x3C)) / 60;
            var_r28 = 1.6666666f * ((arg0->unk_34 % 3600) % 60);
            espBankSet(arg0->unk_4A[7], var_r27 % 10);
            espBankSet(arg0->unk_4A[9], var_r29 / 10);
            espBankSet(arg0->unk_4A[10], var_r29 % 10);
            espBankSet(arg0->unk_4A[12], var_r28 / 10);
            espBankSet(arg0->unk_4A[13],  var_r28 - (var_r28 / 10 * 10));
            if (arg0->unk_20 & 1) {
                if (!((GlobalCounter / 10) & 1)) {
                    var_r26 = arg0->unk_70;
                    var_r25 = arg0->unk_71;
                    var_r24 = arg0->unk_72;
                } else {
                    var_r26 = 0xFF;
                    var_r25 = 0xD8;
                    var_r24 = 0;
                }
                for (i = 0; i < 7; i++) {
                    if (arg0->unk_4A[i] != -1) {
                        espColorSet(arg0->unk_4A[i], var_r26 & 0xFF, var_r25 & 0xFF, var_r24 & 0xFF);
                    } 
                }
            }
            if (arg0->unk_20 & 2) {
                if (arg0->unk_18) {
                    if ((arg0->unk_68 += 0.02f) >= 1.2f) {
                        arg0->unk_68 = 1.2f;
                        arg0->unk_18 = 0;
                    }
                } else {
                    //arg0->unk_68 -=  0.04f;
                    if ((arg0->unk_68 -= 0.04f) <= 1.0f) {
                        arg0->unk_68 = 1.0f;
                        arg0->unk_18 = 1;
                    }
                }
                for (i = 0; i < 8; i++) {
                    if (arg0->unk_4A[i+7] != -1) {
                        espScaleSet(arg0->unk_4A[i+7], arg0->unk_68, arg0->unk_68);
                    }  
                }
            }
            if (arg0->unk_20 & 4) {
                var_f31 = 1.0f - (0.016666668f * arg0->unk_38);
                for (i = 0; i < 15; i++) {
                    if (arg0->unk_4A[i] != -1) {
                        espTPLvlSet(arg0->unk_4A[i], var_f31);
                    }
                    if (arg0->unk_48 != -1) {
                        MgScoreWinTPLvlSet(arg0->unk_48, var_f31);
                    } 
                }
                if (arg0->unk_38++ >= 0x3C) {
                    arg0->unk_20 = arg0->unk_20 & 0xFFFFFFFB;
                    MgTimerDispOff(arg0);
                }
            }
    }
    (void)var_r29;
}

void MgTimerDispOn(timer_s* arg0) {
    s32 i;

    switch (arg0->unk_8) {
        case 0:
            if (arg0->unk_46 == -1) {
                arg0->unk_46 = GMesCreate(1, arg0->unk_30 / 60, -1, -1);
                GMesPosSet(arg0->unk_46, arg0->unk_3C, arg0->unk_40);
            }
        break;
        case 1:
        case 2:
        case 3:
            for (i = 0; i < 15; i++) {
                if (arg0->unk_4A[i] != -1) {
                    espDispOn(arg0->unk_4A[i]);
                }
            }
            if (arg0->unk_48 != -1) {
                MgScoreWinDispSet(arg0->unk_48, arg0->unk_1C);
            }
    }
    arg0->unk_10 = 1;
}

void MgTimerDispOff(timer_s* arg0) {
    s32 i;

    switch (arg0->unk_8) {
        case 0:
            if (arg0->unk_46 != -1) {
                GMesDispSet(arg0->unk_46, 2, -1);
                arg0->unk_46 = -1;
            }
        break;
        case 1:
        case 2:
        case 3:
            for (i = 0; i < 15; i++) {
                if (arg0->unk_4A[i] != -1) {
                    espDispOff(arg0->unk_4A[i]);
                }
            }
            if (arg0->unk_48 != -1) {
                MgScoreWinDispSet(arg0->unk_48, 0);
            }
    }
    arg0->unk_10 = 0;
}

void MgTimerRecordDispOn(timer_s* arg0) {
    s32 i;
    switch (arg0->unk_8) {
        case 0:
            if (arg0->unk_46 == -1) {
            arg0->unk_46 = GMesCreate(1, arg0->unk_30 / 60, -1, -1);
            GMesPosSet(arg0->unk_46, arg0->unk_3C, arg0->unk_40);
            }   
        break;
        case 1:
        case 2:
        case 3:
            for (i = 0; i < 15; i++) {
                if (arg0->unk_4A[i] != -1) {
                    espDispOn(arg0->unk_4A[i]);
                }
            }
            if (arg0->unk_48 != -1) {
                MgScoreWinDispSet(arg0->unk_48, arg0->unk_1C);
            }
    }
    arg0->unk_10 = 1;
}

void MgTimerRecordDispOff(timer_s* arg0) {
    s32 i;
    switch (arg0->unk_8) {
        case 0:
            if (arg0->unk_46 != -1) {
                GMesDispSet(arg0->unk_46, 2, -1);
                arg0->unk_46 = -1;
            }
            break;
        case 1:
        case 2:
        case 3:
            for ( i = 0; i < 15; i++) {
                if (arg0->unk_4A[i] != -1) {
                    espDispOff(arg0->unk_4A[i]);
                }
            }
            if (arg0->unk_48 != -1) {
                MgScoreWinDispSet(arg0->unk_48, 0);
            }
    }
    
    arg0->unk_10 = 0;
}