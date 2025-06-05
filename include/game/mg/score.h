#ifndef _MG_SCORE_H
#define _MG_SCORE_H

#include "dolphin/types.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "datanum/mgconst.h"
#include "humath.h"

#define MGSCORE_DIGIT_MAX 7

typedef struct MgScore_s {
    int mode;
    int prevMode;
    HuVec2F pos;
    HuVec2F scale;
    HuVec2F digitScale;
    HuVec2F unitOfs;
    float zRot;
    float digitW;
    float tpLvl;
    u8 r;
    u8 g;
    u8 b;
    int value;
    int maxDigit;
    HUSPRID *sprId;
    HUSPRGRPID grpId;
    s16 prio;
    BOOL dispF;
    BOOL validF;
    BOOL dispLeadZeroF;
    ANIMDATA *digitAnim;
    HUPROCESS * proc;
} MGSCORE;


MGSCORE *MgScoreCreate(int digitFile, int unitFile, BOOL dispLeadZeroF);
MGSCORE *MgScoreInit(int digitFile);
void MgScoreKill(MGSCORE *score);
int MgScoreModeGet(MGSCORE *score);
void MgScoreValueSet(MGSCORE *score, int value);
int MgScoreValueGet(MGSCORE *score);
void MgScorePosSet(MGSCORE *score, float posX, float posY);
void MgScorePosGet(MGSCORE *score, float *posX, float *posY);
void MgScoreScaleSet(MGSCORE *score, float scaleX, float scaleY);
void MgScoreScaleGet(MGSCORE *score, float *scaleX, float *scaleY);
void MgScoreDigitScaleSet(MGSCORE *score, float scaleX, float scaleY);
void MgScoreZRotSet(MGSCORE *score, float zRot);
void MgScoreZRotGet(MGSCORE *score, float *zRot);
void MgScoreTPLvlSet(MGSCORE *score, float tpLvl);
void MgScoreTPLvlGet(MGSCORE *score, float *tpLvl);
void MgScoreColorSet(MGSCORE *score, u8 r, u8 g, u8 b);
void MgScoreMaxDigitSet(MGSCORE *score, int maxDigit);
void MgScoreMaxDigitGet(MGSCORE *score, int *maxDigit);
void MgScoreDigitWidthSet(MGSCORE *score, float digitW);
void MgScoreDigitWidthGet(MGSCORE *score, float *digitW);
void MgScoreDispOn(MGSCORE *score);
void MgScoreDispOff(MGSCORE *score);
void MgScorePriSet(MGSCORE *score, s16 arg1);
void MgScoreModeDefaultSet(MGSCORE *score);

#endif