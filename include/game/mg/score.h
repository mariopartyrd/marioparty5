#ifndef _MINIGAME_SCORE_H
#define _MINIGAME_SCORE_H

#include "dolphin/types.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"

typedef struct Unk_Score {
    s32 unk_00;
    s32 unk_04;
} Unk_Score;

typedef struct score_s_0x88 {
    s32 unk_0;
    s16 unk_4;
    f32 unk_8;
    f32 unk_C;
    f32 unk_10;
    f32 unk_14;
    f32 unk_18;
    f32 unk_1C;
    f32 unk_20;
    f32 unk_24;
    f32 unk_28;
    f32 unk_2C;
    f32 unk_30;
    u8  unk_34;
    u8  unk_35;
    u8 unk_36;
    s32 unk_38;
    s32 unk_3C;
    s16* unk_40;
    HUSPRID unk_44;
    s16 unk_46;
    BOOL unk_48;
    s32 unk_4C;
    s32 unk_50;
    ANIMDATA * unk_54;
    HUPROCESS * unk_58;
} score_s;


score_s *MgScoreCreate(s32 arg0, s32 arg1, s32 arg2);
score_s *MgScoreInit(s32 arg0);
void MgScoreKill(score_s *arg0);
s32 MgScoreModeGet(s32 *arg0);
void MgScoreValueSet(score_s *arg0, s32 arg1);
s32 MgScoreValueGet(score_s *arg0);
void MgScorePosSet(score_s *arg0, f32 arg8, f32 arg9);
void MgScorePosGet(score_s *arg0, f32 *arg1, f32 *arg2);
void MgScoreScaleSet(score_s *arg0, f32 arg8, f32 arg9);
void MgScoreScaleGet(score_s *arg0, f32 *arg1, f32 *arg2);
void MgScoreDigitScaleSet(score_s *arg0, f32 arg8, f32 arg9);
void MgScoreZRotSet(score_s *arg0, f32 arg8);
void MgScoreZRotGet(score_s *arg0, f32 *arg1);
void MgScoreTPLvlSet(score_s *arg0, f32 arg8);
void MgScoreTPLvlGet(score_s *arg0, f32 *arg1);
void MgScoreColorSet(score_s *arg0, u8 arg1, u8 arg2, u8 arg3);
void MgScoreMaxDigitSet(score_s *arg0, s32 arg1);
void MgScoreMaxDigitGet(score_s *arg0, s32 *arg1);
void MgScoreDigitWidthSet(score_s *arg0, f32 arg8);
void MgScoreDigitWidthGet(score_s *arg0, f32 *arg1);
void MgScoreDispOn(score_s *arg0);
void MgScoreDispOff(score_s *arg0);
void MgScorePriSet(score_s *arg0, s16 arg1);
void MgScoreModeDefaultSet(s32 *arg0);

#endif