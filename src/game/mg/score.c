#include "game/board/model.h"
#include "game/data.h"
#include "game/process.h"
#include "game/mg/score.h"
#include "game/sprite.h"

#define SCORE_UNIT_SPRNO (MGSCORE_DIGIT_MAX)
#define SCORE_SPRMAX (MGSCORE_DIGIT_MAX+1)

static const float unitOfsTbl[3][4] = {
    8.0f, 28.0f, 28.0f, 8.0f, 
    10.0f, 34.0f, 34.0f, 10.0f, 
    12.0f, 40.0f, 40.0f, 12.0f
};

static void ScoreExec(MGSCORE *score);
static void ScoreMain();
static void ScoreDispUpdate(MGSCORE *score);

typedef void (*SCOREFUNC)(MGSCORE *timer);

static SCOREFUNC modeTbl[1] = { ScoreExec };


MGSCORE *MgScoreCreate(int digitFile, int unitFile, BOOL dispLeadZeroF) {
    MGSCORE *score;
    int unitType;
    int unit;
    unitType = 0;
    unit = 0;
    unit = 1;
    score = MgScoreInit(digitFile);
    if (score == NULL) {
        return NULL;
    }
    if (unitFile  != -1) {
        score->sprId[SCORE_UNIT_SPRNO] = (int)HuSprCreate(HuSprAnimDataRead(unitFile), 0, 0);
        HuSprGrpMemberSet(score->grpId, SCORE_UNIT_SPRNO, score->sprId[SCORE_UNIT_SPRNO]);
        HuSprBankSet(score->grpId, SCORE_UNIT_SPRNO, unit);
    }
    switch (unitFile) {
        case MGCONST_ANM_unitSmall:
        case MGCONST_ANM_unitBlueSmall:
            unitType = 0;
            break;

        case MGCONST_ANM_unitBlueMedium:
            unitType = 1;
            break;

        case MGCONST_ANM_unitLarge:
            unitType = 2;
            break;

        default:
            unitType = 0;
            break;
    }
    score->unitOfs.x = unitOfsTbl[unitType][unit];
    score->unitOfs.y = unitOfsTbl[unitType][3];
    score->dispLeadZeroF = dispLeadZeroF;
    MgScorePriSet(score, 90);
    return score;
}

MGSCORE *MgScoreInit(int digitFile) {
    MGSCORE *score;
    int i;

    score = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MGSCORE), HU_MEMNUM_OVL);
    if (score == NULL) {
        return NULL;
    }
    score->pos.x = 0.0f;
    score->pos.y = 0.0f;
    score->scale.x = 1.0f;
    score->scale.y = 1.0f;
    score->digitScale.x = 1.0f;
    score->digitScale.y = 1.0f;
    score->zRot = 0.0f;
    score->tpLvl = 1.0f;
    score->mode = 0;
    score->value = 0;
    score->maxDigit = 5;
    score->r = 255;
    score->g = 216;
    score->b = 21;
    score->dispF = 1;
    score->validF = 0;
    score->dispLeadZeroF = 0;
    score->unitOfs.x = 0.0f;
    score->unitOfs.y = 0.0f;
    score->grpId = HuSprGrpCreate(SCORE_SPRMAX);
    score->sprId = HuMemDirectMallocNum(HEAP_HEAP, SCORE_SPRMAX*sizeof(HUSPRID), HU_MEMNUM_OVL);
    score->digitAnim = HuSprAnimDataRead(digitFile);

    for (i = 0; i < MGSCORE_DIGIT_MAX; i++) {
        score->sprId[i] = (int)HuSprCreate(score->digitAnim, 0, 0);
        HuSprGrpMemberSet(score->grpId, i, score->sprId[i]);
    }
    score->sprId[MGSCORE_DIGIT_MAX] = HUSPR_NONE;
    score->digitW = HuSprData[score->sprId[0]].data->pat->sizeX;
    MgScorePriSet(score, 90);
    ScoreDispUpdate(score);
    score->proc = HuPrcChildCreate(ScoreMain, 0x1000, 0x1000, 0, HuPrcCurrentGet());
    score->proc->property = score;
    return score;
}

void MgScoreKill(MGSCORE *score) {
    HuSprGrpKill(score->grpId);
    HuPrcKill(score->proc);
    HuMemDirectFree(score->sprId);
    HuMemDirectFree(score);
}

int MgScoreModeGet(MGSCORE *score) {
    return score->mode;
}

void MgScoreValueSet(MGSCORE *score, int value) {
    score->value = value;
    score->validF = FALSE;
}

int MgScoreValueGet(MGSCORE *score) {
    return score->value;
}

void MgScorePosSet(MGSCORE *score, float posX, float posY) {
    score->pos.x = posX;
    score->pos.y = posY;
    score->validF = FALSE;
}

void MgScorePosGet(MGSCORE *score, float *posX, float *posY) {
    *posX = score->pos.x;
    *posY = score->pos.y;
}

void MgScoreScaleSet(MGSCORE *score, float scaleX, float scaleY) {
    score->scale.x = scaleX;
    score->scale.y = scaleY;
    score->validF = FALSE;
}

void MgScoreScaleGet(MGSCORE *score, float *scaleX, float *scaleY) {
    *scaleX = score->scale.x;
    *scaleY = score->scale.y;
}

void MgScoreDigitScaleSet(MGSCORE *score, float scaleX, float scaleY) {
    score->digitScale.x = scaleX;
    score->digitScale.y = scaleY;
    score->validF = FALSE;
}

void MgScoreZRotSet(MGSCORE *score, float zRot) {
    score->zRot = zRot;
    score->validF = FALSE;
}

void MgScoreZRotGet(MGSCORE *score, float *zRot) {
    *zRot = score->zRot;
}

void MgScoreTPLvlSet(MGSCORE *score, float tpLvl) {
    score->tpLvl = tpLvl;
    score->validF = FALSE;
}

void MgScoreTPLvlGet(MGSCORE *score, float *tpLvl) {
    *tpLvl = score->tpLvl;
}

void MgScoreColorSet(MGSCORE *score, u8 r, u8 g, u8 b) {
    score->r = r;
    score->g = g;
    score->b = b;
    score->validF = FALSE;
}

void MgScoreMaxDigitSet(MGSCORE *score, int maxDigit) {
    score->maxDigit = maxDigit;
    score->validF = FALSE;
}

void MgScoreMaxDigitGet(MGSCORE *score, int *maxDigit) {
    *maxDigit = score->maxDigit;
}

void MgScoreDigitWidthSet(MGSCORE *score, float digitW) {
    score->digitW = digitW;
    score->validF = FALSE;
}

void MgScoreDigitWidthGet(MGSCORE *score, float *digitW) {
    *digitW = score->digitW;
}

void MgScoreDispOn(MGSCORE *score) {
    score->dispF = TRUE;
    score->validF = FALSE;
}

void MgScoreDispOff(MGSCORE *score) {
    score->dispF = FALSE;
    score->validF = FALSE;
}


void MgScorePriSet(MGSCORE *score, s16 prio) {
    int i;
    score->prio = prio;
    for (i = 0; i < SCORE_SPRMAX; i++) {
        if (score->sprId[i] != HUSPR_NONE) {
            HuSprPriSet(score->grpId, i, prio);
        }
    }
}

static void ScoreMain(void)
{
    MGSCORE *score;

    score = HuPrcCurrentGet()->property;
    while (TRUE) {
        modeTbl[score->mode](score);
    }
}

void MgScoreModeDefaultSet(MGSCORE *score) {
    score->mode = 0;
}

static void ScoreExec(MGSCORE *score)
{
    s32 i;
    s16 mode;
    s32 maxCheck;
    s16 prevMode;

    while (1) {
        for(maxCheck=1, i=0; i<maxCheck; i++) {
            ScoreDispUpdate(HuPrcCurrentGet()->property);
            HuPrcVSleep();

            mode = ((MGSCORE *)HuPrcCurrentGet()->property)->mode;
            prevMode = ((MGSCORE *)HuPrcCurrentGet()->property)->prevMode;
            ((MGSCORE *)HuPrcCurrentGet()->property)->prevMode = mode;

            if (mode != prevMode) {
                return;
            }
        }
    }
}

static void ScoreDispUpdate(MGSCORE *score)
{
    int i;
    int digitNum;
    int digitVal;
    int value;
    int digit;
    BOOL digitDispF;
    float posX;
    int bank[MGSCORE_DIGIT_MAX];

    if (score->validF == 0) {
        i = 0;
        value = score->value;
        digitVal = 0xF4240;
        digitNum = 0;
        digitDispF = 0;
        if (value == 0) {
            if (score->dispLeadZeroF) {
                for (i = 0; i < MGSCORE_DIGIT_MAX; i++) {
                    bank[i] = 0;
                    digitNum++;
                }
            } else {
                bank[i++] = 0;
                digitNum++;
            }
        } else {
            do {
                digit = value / digitVal;
                value = value - (digit * digitVal);
                if ((digit > 0) || (digitDispF) || (score->dispLeadZeroF)) {
                    digitDispF = 1;
                    bank[i++] = digit;
                    digitNum++;
                }
                digitVal = digitVal / 10;
            } while (digitVal > 0);
        }
        while (i < MGSCORE_DIGIT_MAX) {
            bank[i] = -1;
            i++;
        }
        posX = score->digitW * (score->maxDigit - digitNum);

        for (i = 0; i < MGSCORE_DIGIT_MAX; i++) {
            HuSprPosSet(score->grpId, i, posX, 0.0f);
            posX += score->digitW;
            if ((bank[i] != -1) && ((score->dispLeadZeroF == 0) || (i >= (MGSCORE_DIGIT_MAX - score->maxDigit)))) {
                HuSprBankSet(score->grpId, i, bank[i]);
                if (score->dispF) {
                    HuSprDispOn(score->grpId, i);
                } else {
                    HuSprDispOff(score->grpId, i);
                }
                HuSprScaleSet(score->grpId, i, score->digitScale.x, score->digitScale.y);
                HuSprColorSet(score->grpId, i, score->r, score->g, score->b);
            } else {
                HuSprDispOff(score->grpId, i);
            }
        }
        HuSprPosSet(score->grpId, SCORE_UNIT_SPRNO, (score->unitOfs.x + (score->maxDigit * score->digitW)) - score->unitOfs.y, 0.0f);
        HuSprColorSet(score->grpId, SCORE_UNIT_SPRNO, score->r, score->g, score->b);
        HuSprScaleSet(score->grpId, SCORE_UNIT_SPRNO, score->digitScale.x, score->digitScale.y);
        if (score->dispF) {
            HuSprDispOn(score->grpId, SCORE_UNIT_SPRNO);
        } else {
            HuSprDispOff(score->grpId, SCORE_UNIT_SPRNO);
        }
        HuSprGrpPosSet(score->grpId, score->pos.x, score->pos.y);
        HuSprGrpZRotSet(score->grpId, score->zRot);
        HuSprGrpScaleSet(score->grpId, score->scale.x, score->scale.y);
        HuSprGrpTPLvlSet(score->grpId, score->tpLvl);
        score->validF = 1;
    }
}

