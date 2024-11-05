#include "game/object.h"
#include "humath.h"
#include "datanum/instpic.h"
#include "messnum/mgname.h"
#include "messnum/mginst.h"
#include "messnum/sdg.h"

//Move to gamemes.h
void GMesExitCheck(OMOBJ *obj);

#define DECA_SCORE_TIME 0
#define DECA_SCORE_POINT 1
#define DECA_SCORE_NUM 11

u32 mgModeWork[64];
GWCOMMON GwCommonBackup;
int sdWorkInt[16];
float sdWorkFloat[16];

s16 mgMatchGameType;
s16 sdHealthLoss[GW_PLAYER_MAX];
s16 mgMatchMaxScore;
s16 lbl_80288A30;
int lbl_80288A2C;
int lbl_80288A28;
int mgSubMode;
int mg562CtrlNo;
BOOL mgExitStartF;
BOOL mgPauseExitF;
BOOL mgInstExitF;
BOOL mgBoard2Force;

#include "mgdata.inc"

s32 omOvlMgNoGet(s16 ovlNo)
{
    MGDATA *mgData;
    s16 i;
    for(mgData=&mgDataTbl[0], i=0; mgData->ovl != (u16)DLL_NONE; mgData++, i++) {
        if(mgData->ovl == ovlNo) {
            return i;
        }
    }
    return -1;
}

typedef struct mgDecaScore_s {
    s32 mgNo;
    s16 type;
    s16 pointNum;
    HuVec2F points[10];
} MGDECASCORE;

static MGDECASCORE mgDecaScoreTbl[DECA_SCORE_NUM] = {
    {
        502,
        DECA_SCORE_TIME,
        5,
        {
            { 0, 90 },
            { 100, 55 },
            { 550, 35 },
            { 800, 26 },
            { 1000, 23 }
        }
    },
    {
        504,
        DECA_SCORE_TIME,
        5,
        {
            { 0, 90 },
            { 100, 60 },
            { 450, 35 },
            { 800, 20 },
            { 1000, 13 }
        }
    },
    {
        563,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 100, 10 },
            { 350, 20 },
            { 700, 30 },
            { 800, 33 },
            { 950, 40 },
            { 1000, 45 }
        }
    },
    {
        506,
        DECA_SCORE_POINT,
        6,
        {
            { 0, 0 },
            { 100, 60 },
            { 400, 100 },
            { 650, 130 },
            { 850, 155 },
            { 1000, 177 },
        }
    },
    {
        507,
        DECA_SCORE_TIME,
        5,
        {
            { 0, 30 },
            { 150, 20 },
            { 400, 10 },
            { 700, 3 },
            { 1000, 0.1 },
        }
    },
    {
        510,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 50, 5 },
            { 150, 10 },
            { 300, 15 },
            { 500, 20 },
            { 700, 25 },
            { 1000, 30 },
        }
    },
    {
        511,
        DECA_SCORE_TIME,
        7,
        {
            { 0, 90 },
            { 100, 75 },
            { 250, 60 },
            { 500, 50 },
            { 700, 45 },
            { 850, 40 },
            { 1000, 32 },
        }
    },
    {
        513,
        DECA_SCORE_TIME,
        7,
        {
            { 0, 60 },
            { 50, 25 },
            { 300, 17 },
            { 550, 13 },
            { 750, 11 },
            { 950, 8 },
            { 1000, 6 },
        }
    },
    {
        512,
        DECA_SCORE_TIME,
        7,
        {
            { 0, 90 },
            { 200, 45 },
            { 400, 33 },
            { 500, 30 },
            { 650, 27 },
            { 900, 25 },
            { 1000, 24 },
        }
    },
    {
        514,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 0, 12.32 },
            { 150, 20 },
            { 600, 30 },
            { 900, 40 },
            { 980, 45 },
            { 1000, 48 },
        }
    },
    {
        5141,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 0, 36.97109 },
            { 150, 60.018 },
            { 600, 90.027 },
            { 900, 120.036 },
            { 980, 135.0405 },
            { 1000, 144.0432 },
        }
    },
};

int MgSubModeGet(void)
{
    return mgSubMode;
}

int MgDecaScoreCalc(int gameNo, int mgScore)
{
    MGDECASCORE *scoreP;
    HuVec2F *point;
    HuVec2F *pointNext;
    int pointNo;
    s32 mgNo;
    float value;
    int result;
    result = 1000;
    OSReport("%d\n", mgScore);
    mgNo = -1;
    switch(gameNo) {
        case 0:
            if(mgScore == 0) {
                result = 0;
            } else {
                value = mgScore/60.0f;
                mgNo = 502;
            }
            break;
        
        case 1:
            if(mgScore == 0) {
                result = 0;
            } else {
                value = mgScore/60.0f;
                mgNo = 504;
            }
            break;
       
       case 2:
            value = mgScore;
            mgNo = 563;
            break;
       
       case 3:
            value = mgScore;
            mgNo = 506;
            break;
       
       case 4:
            if(mgScore == 0) {
                result = 0;
            } else {
                value = mgScore/1000.0f;
                mgNo = 507;
            }
            break;
       
       case 5:
            value = mgScore;
            mgNo = 510;
            break;
       
       case 6:
            if(mgScore == 0) {
                result = 0;
            } else {
                value = mgScore/60.0f;
                mgNo = 511;
            }
            break;
       
       case 8:
            if(mgScore == 0) {
                result = 0;
            } else {
                value = mgScore/60.0f;
                mgNo = 512;
            }
            break;
       
       case 7:
            if(mgScore == 0) {
                result = 0;
            } else {
                value = mgScore/60.0f;
                mgNo = 513;
            }
            break;
       
       case 9:
            value = mgScore/100.0f;
            if(GWLanguageGet() != GW_LANGUAGE_ENG) {
                mgNo = 514;
            } else {
                mgNo = 5141;
            }
            break;
    }
    if(mgNo > 0) {
        
        int i;
        for(scoreP=&mgDecaScoreTbl[0], i=DECA_SCORE_NUM; i--; scoreP++) {
            if(scoreP->mgNo == mgNo) {
                break;
            }
        }
        point = NULL;
        pointNext = &scoreP->points[0];
        result = 1000;
        if(scoreP->type != DECA_SCORE_TIME) {
            for(pointNo=0; pointNo<scoreP->pointNum; point=pointNext, pointNext++, pointNo++) {
                if(value <= pointNext->y) {
                    if(!point) {
                        result = 0;
                        break;
                    } else {
                        result = point->x;
                        result += (pointNext->x-point->x)*((value-point->y)/(pointNext->y-point->y));
                        break;
                    }
                }
                
            }
        } else {
            for(pointNo=0; pointNo<scoreP->pointNum; point=pointNext, pointNext++, pointNo++) {
                if(value >= pointNext->y) {
                    if(!point) {
                        result = 0;
                        break;
                    } else {
                        result = point->x;
                        result += (pointNext->x-point->x)*((point->y-value)/(point->y-pointNext->y));
                        break;
                    }
                }
            }
        }
    }
    if(result < 0) {
        result = 0;
    } else if(result > 1000) {
        result = 1000;
    }
    return result;
}

int lbl_80288138 = 0xFFD815FF;
int lbl_8028813C = 0xFFFFFFFF;
int lbl_80288140 = 0x42FF7AFF;

void omGameSysInit(OMOBJMAN *objman)
{
    int i;
    OMOBJ *obj;

    omSystemKeyCheckSetup(objman);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    omSysPauseEnable(0);

    for(i=0; i<GW_PLAYER_MAX; i++) {
        GWMgCoinBonusSet(i, 0);
        GWMgCoinSet(i, 0);
        GWMgScoreSet(i, 0);
    }

    if(mgPauseExitF) {
        obj = omAddObjEx(objman, 0x7FDC, 0, 0, -1, GMesExitCheck);
        omSetStatBit(obj, OM_STAT_NOPAUSE|0x80);
    }
    HuWinInit(0);
}

void omVibrate(s16 playerNo, s16 duration, s16 off, s16 on)
{
    if (GWRumbleFGet() != FALSE && GwPlayerConf[playerNo].type == GW_TYPE_MAN) {
        HuPadRumbleSet(GwPlayerConf[playerNo].padNo, duration, off, on);
    }

}