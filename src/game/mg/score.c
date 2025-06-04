#include "game/board/model.h"
#include "game/data.h"
#include "game/process.h"
#include "game/mg/score.h"
#include "game/sprite.h"


static const  float unitOfsTbl[3][4] = {
    8.0f, 28.0f, 28.0f, 8.0f, 
    10.0f, 34.0f, 34.0f, 10.0f, 
    12.0f, 40.0f, 40.0f, 12.0f
};

void ScoreExec();
static void (*modeTbl[1])(void *) = { ScoreExec };

void ScoreMain();
score_s *MgScoreInit(s32 arg0);
void MgScorePriSet(score_s *arg0, s16 arg1);
void ScoreDispUpdate(score_s *arg0);

score_s *MgScoreCreate(s32 arg0, s32 arg1, s32 arg2) {
    score_s *var_r31;
    s32 var_r30;
    s32 var_r29;
    s32 var_r28;
    var_r30 = 0;
    var_r29 = 0;
    var_r29 = 1;
    var_r31 = MgScoreInit(arg0);
    if (var_r31 == NULL) {
        return NULL;
    }
    if (arg1  != -1) {
        var_r31->unk_40[7] = (s16)HuSprCreate(HuSprAnimRead(HuDataSelHeapReadNum(arg1, 0x10000000, HEAP_MODEL)), 0, 0);
        HuSprGrpMemberSet(var_r31->unk_44, 7, var_r31->unk_40[7]);
        HuSprBankSet(var_r31->unk_44, 7, var_r29);
    }
    switch (arg1) {
        case 0x960035:
        case 0x960036:
            var_r30 = 0;
            break;

        case 0x960037:
            var_r30 = 1;
            break;

        case 0x960038:
            var_r30 = 2;
            break;

        default:
            var_r30 = 0;
            break;
    }
    var_r31->unk_20 = unitOfsTbl[var_r30][var_r29];
    var_r31->unk_24 = unitOfsTbl[var_r30][3];
    var_r31->unk_50 = arg2;
    MgScorePriSet(var_r31, 0x5A);
    return var_r31;
}

score_s *MgScoreInit(s32 arg0) {
    score_s *var_r31;
    s32 i;

    var_r31 = HuMemDirectMallocNum(HEAP_HEAP, 0x5C, 0x10000000);
    if (var_r31 == NULL) {
        return NULL;
    }
    var_r31->unk_8 = 0.0f;
    var_r31->unk_C = 0.0f;
    var_r31->unk_10 = 1.0f;
    var_r31->unk_14 = 1.0f;
    var_r31->unk_18 = 1.0f;
    var_r31->unk_1C = 1.0f;
    var_r31->unk_28 = 0.0f;
    var_r31->unk_30 = 1.0f;
    var_r31->unk_0 = 0;
    var_r31->unk_38 = 0;
    var_r31->unk_3C = 5;
    var_r31->unk_34 = 0xff;
    var_r31->unk_35 = 0xD8;
    var_r31->unk_36 = 0x15;
    var_r31->unk_48 = 1;
    var_r31->unk_4C = 0;
    var_r31->unk_50 = 0;
    var_r31->unk_20 = 0.0f;
    var_r31->unk_24 = 0.0f;
    var_r31->unk_44 = HuSprGrpCreate(8);
    var_r31->unk_40 = HuMemDirectMallocNum(HEAP_HEAP, 0x10, 0x10000000);
    var_r31->unk_54 = HuSprAnimRead(HuDataSelHeapReadNum(arg0, 0x10000000, HEAP_MODEL));

    for (i = 0; i < 7; i++) {
        var_r31->unk_40[i] = (s16) HuSprCreate(var_r31->unk_54, 0, 0);
        HuSprGrpMemberSet(var_r31->unk_44, i, var_r31->unk_40[i]);
    }
    var_r31->unk_40[7] = -1;
    var_r31->unk_2C = HuSprData[var_r31->unk_40[0]].data->pat->sizeX;
    MgScorePriSet(var_r31, 0x5A);
    ScoreDispUpdate(var_r31);
    var_r31->unk_58 = HuPrcChildCreate(ScoreMain, 0x1000, 0x1000, 0, HuPrcCurrentGet());
    var_r31->unk_58->property = var_r31;
    return var_r31;
}

void MgScoreKill(score_s *arg0) {
    HuSprGrpKill(arg0->unk_44);
    HuPrcKill(arg0->unk_58);
    HuMemDirectFree(arg0->unk_40);
    HuMemDirectFree(arg0);
}

s32 MgScoreModeGet(s32 *arg0) {
    return *arg0;
}

void MgScoreValueSet(score_s *arg0, s32 arg1) {
    arg0->unk_38 = arg1;
    arg0->unk_4C = 0;
}

s32 MgScoreValueGet(score_s *arg0) {
    return arg0->unk_38;
}

void MgScorePosSet(score_s *arg0, f32 arg8, f32 arg9) {
    arg0->unk_8 = arg8;
    arg0->unk_C = arg9;
    arg0->unk_4C = 0;
}

void MgScorePosGet(score_s *arg0, f32 *arg1, f32 *arg2) {
    *arg1 = arg0->unk_8;
    *arg2 = arg0->unk_C;
}

void MgScoreScaleSet(score_s *arg0, f32 arg8, f32 arg9) {
    arg0->unk_10 = arg8;
    arg0->unk_14 = arg9;
    arg0->unk_4C = 0;
}

void MgScoreScaleGet(score_s *arg0, f32 *arg1, f32 *arg2) {
    *arg1 = arg0->unk_10;
    *arg2 = arg0->unk_14;
}

void MgScoreDigitScaleSet(score_s *arg0, f32 arg8, f32 arg9) {
    arg0->unk_18 = arg8;
    arg0->unk_1C = arg9;
    arg0->unk_4C = 0;
}

void MgScoreZRotSet(score_s *arg0, f32 arg8) {
    arg0->unk_28 = arg8;
    arg0->unk_4C = 0;
}

void MgScoreZRotGet(score_s *arg0, f32 *arg1) {
    *arg1 = arg0->unk_28;
}

void MgScoreTPLvlSet(score_s *arg0, f32 arg8) {
    arg0->unk_30 = arg8;
    arg0->unk_4C = 0;
}

void MgScoreTPLvlGet(score_s *arg0, f32 *arg1) {
    *arg1 = arg0->unk_30;
}

void MgScoreColorSet(score_s *arg0, u8 arg1, u8 arg2, u8 arg3) {
    arg0->unk_34 = arg1;
    arg0->unk_35 = arg2;
    arg0->unk_36 = arg3;
    arg0->unk_4C = 0;
}

void MgScoreMaxDigitSet(score_s *arg0, s32 arg1) {
    arg0->unk_3C = arg1;
    arg0->unk_4C = 0;
}

void MgScoreMaxDigitGet(score_s *arg0, s32 *arg1) {
    *arg1 = arg0->unk_3C;
}

void MgScoreDigitWidthSet(score_s *arg0, f32 arg8) {
    arg0->unk_2C = arg8;
    arg0->unk_4C = 0;
}

void MgScoreDigitWidthGet(score_s *arg0, f32 *arg1) {
    *arg1 = arg0->unk_2C;
}

void MgScoreDispOn(score_s *arg0) {
    arg0->unk_48 = TRUE;
    arg0->unk_4C = 0;
}

void MgScoreDispOff(score_s *arg0) {
    arg0->unk_48 = FALSE;
    arg0->unk_4C = 0;
}


void MgScorePriSet(score_s *arg0, s16 arg1) {
    s32 i;
    arg0->unk_46 = arg1;
    for (i = 0; i < 8; i++) {
        if (arg0->unk_40[i] != -1) {
            HuSprPriSet(arg0->unk_44, i, arg1);
        }
    }
}

void ScoreMain(void){
    score_s *var_r31;

    var_r31 = HuPrcCurrentGet()->property;
    while (TRUE) {
    modeTbl[var_r31->unk_0](var_r31);
    }
}

void MgScoreModeDefaultSet(s32 *arg0) {
    *arg0 = 0;
}

//Todo: Discover Type
void ScoreExec(s32 *arg0) {
    s32 var_r31;
    s16 var_r30;
    s32 var_r29;
    s16 var_r28;
    s16 var_r27;

    while (1) {
        var_r29 = 1;
        var_r31 = 0;

        while (var_r31 < var_r29) {
            ScoreDispUpdate(HuPrcCurrentGet()->property);
            HuPrcVSleep();

            var_r30 = ((Unk_Score*)HuPrcCurrentGet()->property)->unk_00;
            var_r28 = ((Unk_Score*)HuPrcCurrentGet()->property)->unk_04;
            ((Unk_Score*)HuPrcCurrentGet()->property)->unk_04 = var_r30;

            if (var_r30 != var_r28) {
                return;
            }
            var_r31 += 1;
        }
    }
}

#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))

//Todo: Make this use array count of sp8
void ScoreDispUpdate(score_s *arg0) {
    s32 i;
    s32 var_r29;
    s32 var_r28;
    s32 var_r27;
    s32 var_r26;
    s32 var_r25;
    s32 temp_r0;
    f32 var_f31;
    s32 sp8[7];

    if (arg0->unk_4C == 0) {
        i = 0;
        var_r27 = arg0->unk_38;
        var_r28 = 0xF4240;
        var_r29 = 0;
        var_r25 = 0;
        if (var_r27 == 0) {
            if (arg0->unk_50) {
                for (i = 0; i < 7; i++) {
                    sp8[i] = 0;
                    var_r29++;
                }
            } else {
                sp8[i++] = 0;
                var_r29++;
            }
        } else {
            do {
                var_r26 = var_r27 / var_r28;
                var_r27 = var_r27 - (var_r26 * var_r28);
                if ((var_r26 > 0) || (var_r25) || (arg0->unk_50)) {
                    var_r25 = 1;
                    sp8[i++] = var_r26;
                    var_r29++;
                }
                var_r28 = var_r28 / 10;
            } while (var_r28 > 0);
        }
        while (i < 7) {
            sp8[i] = -1;
            i++;
        }
        var_f31 = arg0->unk_2C * (arg0->unk_3C - var_r29);

        for (i = 0; i < 7; i++) {
                 HuSprPosSet(arg0->unk_44, i, var_f31, 0.0f);
            var_f31 += arg0->unk_2C;
            if ((sp8[i] != -1) && ((arg0->unk_50 == 0) || (i >= (7 - arg0->unk_3C)))) {
                HuSprBankSet(arg0->unk_44, i, sp8[i]);
                if (arg0->unk_48) {
                    HuSprAttrReset(arg0->unk_44, i, 4);
                } else {
                    HuSprAttrSet(arg0->unk_44, i, 4);
                }
                HuSprScaleSet(arg0->unk_44, i, arg0->unk_18, arg0->unk_1C);
                HuSprColorSet(arg0->unk_44, i, arg0->unk_34, arg0->unk_35, arg0->unk_36);
            } else {
                HuSprAttrSet(arg0->unk_44, i, 4);
            }
        }
        HuSprPosSet(arg0->unk_44, 7, (arg0->unk_20 + (arg0->unk_3C * arg0->unk_2C)) - arg0->unk_24, 0.0f);
        HuSprColorSet(arg0->unk_44, 7, arg0->unk_34, arg0->unk_35, arg0->unk_36);
        HuSprScaleSet(arg0->unk_44, 7, arg0->unk_18, arg0->unk_1C);
        if (arg0->unk_48) {
            HuSprAttrReset(arg0->unk_44, 7, 4);
        } else {
            HuSprAttrSet(arg0->unk_44, 7, 4);
        }
        HuSprGrpPosSet(arg0->unk_44, arg0->unk_8, arg0->unk_C);
        HuSprGrpZRotSet(arg0->unk_44, arg0->unk_28);
        HuSprGrpScaleSet(arg0->unk_44, arg0->unk_10, arg0->unk_14);
        HuSprGrpTPLvlSet(arg0->unk_44, arg0->unk_30);
        arg0->unk_4C = 1;
    }
}

