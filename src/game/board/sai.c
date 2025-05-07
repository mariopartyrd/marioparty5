#include "game/board/sai.h"
#include "game/board/player.h"
#include "game/board/audio.h"
#include "game/board/window.h"
#include "game/board/capsule.h"
#include "game/board/camera.h"
#include "game/board/model.h"
#include "game/board/pause.h"
#include "game/board/coin.h"

#include "datanum/blast5.h"
#include "messnum/boardope.h"

#define SAI_PLAYERNO_NULL GW_PLAYER_MAX
#define SAI_MAX SAI_PLAYERNO_NULL+1

#define SAIEFF_PUFF_MAX 50
#define SAIEFF_TRI_MAX 20

typedef struct SaiEff_s {
    HuVecF rot;
    float radius;
    HuVecF scale;
} SAIEFF;

typedef struct SaiWork_s {
    int playerNo;
    int saiType;
    int max;
    int no;
    s8 valueTbl[SAI_VALUENUM_MAX];
    int valueNum;
    int tutorialVal[3];
    BOOL padWinF;
    s8 result[3];
    HuVecF pos;
    int color;
    OMOBJ *obj;
    SAIEFF *eff;
    void *dlBuf;
    u32 dlSize;
    ANIMDATA *animEffPuff;
    ANIMDATA *animEffTri;
    s8 puffTime;
    s8 triTime;
} SAIWORK;

static int saiResult[SAI_MAX];
static HUPROCESS *saiProc[SAI_MAX];

static SAIHITHOOK saiHitHook[SAI_MAX] = {};
static SAIPADBTNHOOK saiPadBtnHook[SAI_MAX] = {};
static SAIMOTHOOK saiMotHook[SAI_MAX] = {};
static OMOBJ *saiNumObj[SAI_MAX][3] = {};
static HuVecF sai1PosTbl[1] = {
    0, 300, 0
};
static HuVecF sai2PosTbl[2] = {
    -200, 300, 0,
    200, 300, 0
};
static HuVecF sai3PosTbl[3] = {
    -200, 300, 0,
    200, 300, 0,
    0, 300, 0
};
static HuVecF *saiPosTbl[3] = { sai1PosTbl, sai2PosTbl, sai3PosTbl };
static int saiMaxTbl[SAITYPE_MAX] = {
    1,
    2,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1
};
static int saiPatapataResultTbl[8] = {
    1,
    3,
    5,
    10,
    15,
    20,
    25,
    -1
};

static const int saiMaxNumTbl[SAITYPE_MAX] = {
    10,
    10,
    10,
    5,
    10,
    1,
    10,
    7,
    0,
    0,
    1
};

static const int saiShortMaxNumTbl[SAITYPE_MAX] = {
    6,
    6,
    6,
    3,
    10,
    1,
    0,
    7,
    0,
    0,
    1
};

static const int saiFileTbl[SAITYPE_MAX] = {
    BOARD_HSF_sai,
    BOARD_HSF_sai1,
    BOARD_HSF_sai2,
    BOARD_HSF_sai3,
    BOARD_HSF_sai4,
    BOARD_HSF_sai5,
    BOARD_HSF_sai,
    BOARD_HSF_sai7,
    BOARD_HSF_sai8,
    BOARD_HSF_sai9,
    BLAST5_HSF_block
};


static const int numberFileTbl[] = {
    BOARD_HSF_num0,
    BOARD_HSF_num1,
    BOARD_HSF_num2,
    BOARD_HSF_num3,
    BOARD_HSF_num4,
    BOARD_HSF_num5,
    BOARD_HSF_num6,
    BOARD_HSF_num7,
    BOARD_HSF_num8,
    BOARD_HSF_num9
};

static void SaiProcMain(void);
static void SaiProcDestroy(void);
static u16 SaiPadBtnDefault(int playerNo);

void MBSaiInit(void)
{
    int i;
    for(i=0; i<SAI_MAX; i++) {
        saiProc[i] = NULL;
    }
}

static void SaiProcExec(int playerNo, int saiType, s8 *valueTbl, int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color)
{
    SAIWORK *work;
    int i;
    HuVecF posPlayer;
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    saiProc[playerNo] = MBPrcCreate(SaiProcMain, 8203, 24576);
    MBPrcDestructorSet(saiProc[playerNo], SaiProcDestroy);
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(SAIWORK), HU_MEMNUM_OVL);
    saiProc[playerNo]->property = work;
    memset(work, 0, sizeof(SAIWORK));
    work->playerNo = playerNo;
    work->no = 0;
    work->saiType = saiType;
    work->max = saiMaxTbl[saiType];
    if(playerNo < GW_PLAYER_MAX) {
        saiPadBtnHook[playerNo] = SaiPadBtnDefault;
        saiMotHook[playerNo] = MBPlayerSaiMotExec;
    } else {
        saiPadBtnHook[playerNo] = NULL;
        saiMotHook[playerNo] = NULL;
    }
    if(!pos) {
        if(playerNo < GW_PLAYER_MAX) {
            MBPlayerPosGet(work->playerNo, &posPlayer);
            work->pos.x = posPlayer.x;
            work->pos.y = posPlayer.y;
            work->pos.z = posPlayer.z;
        } else {
            work->pos.x = work->pos.y = work->pos.z = 0;
        }
    } else {
        work->pos = *pos;
    }
    for(i=0; valueTbl[i] >= 0; i++) {
        work->valueTbl[i] = (s8)valueTbl[i];
    }
    work->valueNum = i;
    if(tutorialVal) {
        for(i=0; i<3; i++) {
            work->tutorialVal[i] = tutorialVal[i];
        }
    } else {
        work->tutorialVal[0] = work->tutorialVal[1] = work->tutorialVal[2] = -1;
    }
    work->padWinF = padWinF;
    work->color = color;
    if(playerNo < GW_PLAYER_MAX) {
        GwPlayer[playerNo].saiNum = work->max;
    }
    if(waitF) {
        while(!MBSaiKillCheck(playerNo)) {
            HuPrcVSleep();
        }
    }
}

static void SaiExecDirect(int playerNo, int saiType, s8 *valueTbl, int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color)
{
    s8 valueTblNew[SAI_VALUENUM_MAX];
    int i;
    MBAudFXPlay(MSM_SE_BOARD_102);
    if(valueTbl == NULL) {
        if(!_CheckFlag(FLAG_BOARD_SAI_SHORT)) {
            for(i=0; i<saiMaxNumTbl[saiType]; i++) {
                valueTblNew[i] = i;
            }
        } else {
            for(i=0; i<saiShortMaxNumTbl[saiType]; i++) {
                valueTblNew[i] = i;
            }
        }
        valueTblNew[i] = -1;
        SaiProcExec(playerNo, saiType, valueTblNew, tutorialVal, padWinF, waitF, pos, color);
    } else {
        SaiProcExec(playerNo, saiType, valueTbl, tutorialVal, padWinF, waitF, pos, color);
    }
}

int MBSaiTutorialExec(int playerNo, int saiType, s8 *valueTbl, int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color)
{
    SaiExecDirect(playerNo, saiType, valueTbl, tutorialVal, padWinF, waitF, pos, color);
    if(waitF) {
        return MBSaiResultGet(playerNo);
    } else {
        return -1;
    }
}

int MBSaiExec(int playerNo, int saiType, s8 *valueTbl, int tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color)
{
    int tutorialValData[3];
    if(tutorialVal < 0) {
        SaiExecDirect(playerNo, saiType, valueTbl, NULL, padWinF, waitF, pos, color);
        if(!waitF) {
            return;
        }
        return MBSaiResultGet(playerNo);
    }
    tutorialValData[0] = tutorialVal;
    tutorialValData[1] = -1;
    tutorialValData[2] = -1;
    SaiExecDirect(playerNo, saiType, valueTbl, tutorialValData, padWinF, waitF, pos, color);
    if(!waitF) {
        return;
    }
    return MBSaiResultGet(playerNo);
}

int MBSaiPlayerExec(int playerNo, int saiType)
{
    BOOL waitF;
    SaiExecDirect(playerNo, saiType, NULL, NULL, TRUE, TRUE, NULL, SAI_COLOR_GREEN);
    (void)waitF;
    (void)waitF;
    (void)waitF;
    (void)waitF;
    return MBSaiResultGet(playerNo);
}

int MBSaiChanceTradeExec(int playerNo)
{
    s8 valueTbl[8];
    int i;
    for(i=0; i<7; i++) {
        valueTbl[i] = i;
    }
    valueTbl[i] = -1;
    SaiProcExec(playerNo, SAITYPE_CHANCE_TRADE, valueTbl, 0, TRUE, TRUE, NULL, SAI_COLOR_GREEN);
    return MBSaiResultGet(playerNo);
}


int MBSaiChanceCharExec(int playerNo, int *playerNoTbl)
{
    s8 valueTbl[GW_PLAYER_MAX+1];
    int num;
    int i;
    for(num=0, i=0; i<GW_PLAYER_MAX; i++) {
        if(playerNoTbl[i] >= 0) {
            valueTbl[num++] = GwPlayer[playerNoTbl[i]].charNo;
        }
    }
    valueTbl[num] = -1;
    SaiProcExec(playerNo, SAITYPE_CHANCE_CHAR, valueTbl, 0, TRUE, TRUE, NULL, SAI_COLOR_GREEN);
    return MBSaiResultGet(playerNo);
}

static void SaiPadWinCreate(SAIWORK *work);
static void SaiObjCreate(SAIWORK *work);
static void SaiReadyWait(SAIWORK *work);
static void SaiExec(SAIWORK *work);
static void SaiMatchBonusExec(SAIWORK *work);

static void SaiObjEffectCreate(SAIWORK *work);
static void SaiObjEffectKill(SAIWORK *work);

static void SaiNumObjReset(int playerNo);

static void SaiProcMain(void)
{
    SAIWORK *work = HuPrcCurrentGet()->property;
    int i;
    memset(&work->result[0], 0, 3);
    SaiObjEffectCreate(work);
    for(i=0; i<work->max; i++) {
        if(work->padWinF) {
            SaiPadWinCreate(work);
        }
        SaiObjCreate(work);
        SaiReadyWait(work);
        SaiExec(work);
        if(work->padWinF) {
            MBTopWinKill();
        }
        if(saiResult[work->playerNo] == SAI_RESULT_FAIL2 ||
            saiResult[work->playerNo] == SAI_RESULT_FAIL1 ||
            saiResult[work->playerNo] == SAI_RESULT_SCROLL ||
            saiResult[work->playerNo] == SAI_RESULT_FAIL4 ||
            saiResult[work->playerNo] == SAI_RESULT_CAPSULESEL) {
                HuPrcEnd();
            }
        if(saiMotHook[work->playerNo]) {
            saiMotHook[work->playerNo](work->playerNo);
        }
        while(work->obj) {
            HuPrcVSleep();
        }
        work->no++;
    }
    while(!MBSaiNumStopCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    if(work->max >= 2 && GWPartyFGet() != FALSE && _CheckFlag(FLAG_BOARD_OPENING) && !_CheckFlag(FLAG_MG_CIRCUIT)) {
        for(i=0; i<work->max-1; i++) {
            if(work->result[i] != work->result[i+1]) {
                break;
            }
        }
        if(i >= work->max-1) {
            SaiMatchBonusExec(work);
        }
    }
    if(work->max >= 2) {
        HuPrcSleep(30);
        SaiNumObjReset(work->playerNo);
        while(!MBSaiNumStopCheck(work->playerNo)) {
            HuPrcVSleep();
        }
    }
    saiResult[work->playerNo] = 0;
    for(i=0; i<work->max; i++) {
        saiResult[work->playerNo] += work->result[i];
    }
    HuPrcEnd();
}

static void SaiProcDestroy(void)
{
    SAIWORK *work = HuPrcCurrentGet()->property;
    MBSaiKill(work->playerNo);
    SaiObjEffectKill(work);
    saiProc[work->playerNo] = NULL;
    saiHitHook[work->playerNo] = NULL;
    saiMotHook[work->playerNo] = NULL;
    saiPadBtnHook[work->playerNo] = NULL;
    HuMemDirectFree(work);
}

static void SaiPadWinCreate(SAIWORK *work)
{
    u32 mess;
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }
    if(work->saiType == SAITYPE_PATAPATA_COIN ||
        work->saiType == SAITYPE_KUPA ||
        work->saiType == SAITYPE_CHANCE_TRADE ||
        work->saiType == SAITYPE_CHANCE_CHAR ||
        work->saiType == SAITYPE_BLOCK) {
            mess = MESS_BOARDOPE_PAD_SAI;
        } else if(work->no == 0) {
            if(MBPlayerCapsuleUseGet() != CAPSULE_NULL ||
                MBPlayerCapsuleNumGet(work->playerNo) == 0 ||
                _CheckFlag(FLAG_MG_CIRCUIT)) {
                mess = MESS_BOARDOPE_PAD_SAI_MAP;
            } else {
                mess = MESS_BOARDOPE_PAD_SAI_CAPSULE;
            }
        } else {
            mess = MESS_BOARDOPE_PAD_SAI;
        }
    MBWinCreateHelp(mess);
}

static void SaiObjExec(OMOBJ *obj);

#define SAIOBJ_MODE_FADEIN 0
#define SAIOBJ_MODE_IDLE 1
#define SAIOBJ_MODE_HIT 2
#define SAIOBJ_MODE_FADEOUT 3

typedef struct SaiObjWork_s {
    unsigned killF : 1;
    unsigned lockF : 1;
    unsigned mode : 3;
    unsigned no : 2;
    s8 lockTime;
    s16 time;
    s16 maxTime;
    s16 valueNo;
    int saiSeNo;
} SAIOBJWORK;

static HU3DPARMANID SaiObjCloudCreate(void);
static void SaiObjEffPuffDraw(HU3DMODEL *modelP, Mtx *mtx);
static void SaiObjEffTriDraw(HU3DMODEL *modelP, Mtx *mtx);
static void SaiObjKillSet(OMOBJ *obj);
static void SaiKill(SAIWORK *work);

static void SaiObjCreate(SAIWORK *work)
{
    MBCAMERA *cameraP = MBCameraGet();
    OMOBJ *obj;
    SAIOBJWORK *objWork;
    float time;
    HuVecF parManVec;
    obj = work->obj = MBAddObj(258, 4, 0, SaiObjExec);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->data = work;
    obj->mdlId[0] = MBModelCreate(MBDATANUM(saiFileTbl[work->saiType]), NULL, FALSE);
    MBModelLayerSet(obj->mdlId[0], 3);
    MBAudFXPlay(MSM_SE_BOARD_03);
    MBMotionNoSet(obj->mdlId[0], MB_MOT_DEFAULT, HU3D_MOTATTR_NONE);
    MBMotionSpeedSet(obj->mdlId[0], 0);
    time = frandmod(work->valueNum)+0.5f;
    MBMotionTimeSet(obj->mdlId[0], time);
    MBModelAlphaSet(obj->mdlId[0], 0);
    objWork = omObjGetWork(obj, SAIOBJWORK);
    objWork->killF = FALSE;
    objWork->lockF = FALSE;
    objWork->mode = 0;
    objWork->no = work->no;
    objWork->time = 0;
    objWork->maxTime = 30;
    objWork->lockTime = 0;
    objWork->valueNo = time;
    obj->trans.x = work->pos.x;
    obj->trans.y = work->pos.y+250;
    obj->trans.z = work->pos.z;
    MBModelPosSet(obj->mdlId[0], obj->trans.x, obj->trans.y, obj->trans.z);
    obj->mdlId[1] = SaiObjCloudCreate();
    Hu3DParManPosSet(obj->mdlId[1], obj->trans.x, obj->trans.y, obj->trans.z);
    VECSubtract(&cameraP->pos, &obj->trans, &parManVec);
    Hu3DParManVecSet(obj->mdlId[1], parManVec.x, parManVec.y, parManVec.z);
    obj->mdlId[2] = Hu3DHookFuncCreate(SaiObjEffPuffDraw);
    Hu3DModelCameraSet(obj->mdlId[2], HU3D_CAM0);
    Hu3DModelLayerSet(obj->mdlId[2], 3);
    Hu3DModelDispOff(obj->mdlId[2]);
    Hu3DData[obj->mdlId[2]].hookData = obj;
    work->puffTime = 0;
    obj->mdlId[3] = Hu3DHookFuncCreate(SaiObjEffTriDraw);
    Hu3DModelCameraSet(obj->mdlId[3], HU3D_CAM0);
    Hu3DModelLayerSet(obj->mdlId[3], 3);
    Hu3DModelDispOff(obj->mdlId[3]);
    Hu3DData[obj->mdlId[3]].hookData = obj;
    work->triTime = 0;
}

static void SaiObjExec(OMOBJ *obj)
{
    SAIOBJWORK *objWork = omObjGetWork(obj, SAIOBJWORK);
    SAIWORK *work = obj->data;
    int i;
    float time;
    float scale;
    HuVecF modelPos;
    HuVecF pos;
    if(objWork->killF || MBKillCheck()) {
        if(objWork->saiSeNo >= 0) {
            HuAudFXStop(objWork->saiSeNo);
            objWork->saiSeNo = MSM_SENO_NONE;
        }
        if(obj->mdlId[0] >= 0) {
            MBModelKill(obj->mdlId[0]);
            obj->mdlId[0] = MB_MODEL_NONE;
        }
        Hu3DParManKill(obj->mdlId[1]);
        obj->mdlId[1] = HU3D_PARMANID_NONE;
        for(i=2; i<=3; i++) {
            if(obj->mdlId[i] >= 0) {
                Hu3DModelKill(obj->mdlId[i]);
                obj->mdlId[i] = HU3D_MODELID_NONE;
            }
        }
        obj->data = NULL;
        MBDelObj(obj);
        work->obj = NULL;
        return;
    }
    switch(objWork->mode) {
        case SAIOBJ_MODE_FADEIN:
            time = (float)objWork->time/objWork->maxTime;
            MBModelAlphaSet(obj->mdlId[0], time*255);
            if(++objWork->time > objWork->maxTime) {
                objWork->mode = SAIOBJ_MODE_IDLE;
                objWork->time = 0;
                objWork->maxTime = 12;
                objWork->saiSeNo = MBAudFXPlay(MSM_SE_BOARD_01);
            }
            break;
       
        case SAIOBJ_MODE_HIT:
            time = (float)objWork->time/objWork->maxTime;
            if(time > 1) {
                time = 1;
            }
            objWork->time++;
            scale = HuSin(time*180.0f);
            MBModelScaleSet(obj->mdlId[0], 1+scale, 1-(0.5f*scale), 1+scale);
            modelPos.y = obj->trans.y+(150*scale);
            MBModelPosSet(obj->mdlId[0], obj->trans.x, modelPos.y, obj->trans.z);
            if(objWork->time == 8) {
                MBModelPosGet(obj->mdlId[0], &modelPos);
                if(work->saiType != SAITYPE_CHANCE_TRADE &&
                    work->saiType != SAITYPE_CHANCE_CHAR &&
                    work->saiType != SAITYPE_BLOCK) {
                        if(work->playerNo < GW_PLAYER_MAX) {
                            MBPlayerPosGet(work->playerNo, &pos);
                        } else {
                            pos.x = work->pos.x;
                            pos.y = work->pos.y;
                            pos.z = work->pos.z;
                        }
                        VECAdd(&pos, &saiPosTbl[work->max-1][work->no], &pos);
                        if(GWPartyFGet() == FALSE && MBPlayerStoryComCheck(work->playerNo)) {
                            saiNumObj[work->playerNo][work->no] = MBSaiNumObjCreate(work->playerNo, &modelPos, &pos, work->result[work->no], work->color, FALSE);
                        } else {
                            saiNumObj[work->playerNo][work->no] = MBSaiNumObjCreate(work->playerNo, &modelPos, &pos, work->result[work->no], work->color, TRUE);
                        }
                }
            }
            if(objWork->time > objWork->maxTime) {
                objWork->mode = SAIOBJ_MODE_FADEOUT;
                objWork->time = 0;
                objWork->maxTime = 30;
            }
            break;
       
        case SAIOBJ_MODE_FADEOUT:
            time = (float)objWork->time/15;
            if(time > 1) {
                time = 1;
            }
            if(++objWork->time > objWork->maxTime) {
                SaiObjKillSet(obj);
            }
            MBModelAlphaSet(obj->mdlId[0], (1-time)*255);
            break;
    }
    if(!objWork->lockF && objWork->mode < SAIOBJ_MODE_HIT && work->valueNum > 1 && objWork->lockTime++ > 4) {
        s16 valueTbl[SAI_VALUENUM_MAX];
        int valueNum;
        objWork->lockTime = 0;
        valueNum = 0;
        for(i=0; i<work->valueNum; i++) {
            if(i != objWork->valueNo) {
                valueTbl[valueNum++] = i;
            }
        }
        objWork->valueNo = valueTbl[frandmod(valueNum)];
        MBMotionTimeSet(obj->mdlId[0], work->valueTbl[objWork->valueNo]+0.5f);
    }
}

static void SaiObjKillSet(OMOBJ *obj)
{
    SAIOBJWORK *objWork = omObjGetWork(obj, SAIOBJWORK);
    objWork->killF = TRUE;
}

void MBSaiKill(int playerNo)
{
    if(saiProc[playerNo] != NULL) {
        SAIWORK *work = saiProc[playerNo]->property;
        SaiKill(work);
        HuPrcKill(saiProc[playerNo]);
    }
}

void MBSaiClose(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBSaiKill(i);
    }
}

static void SaiKill(SAIWORK *work)
{
    if(work->obj) {
        SaiObjKillSet(work->obj);
    }
}

static void SaiExec(SAIWORK *work)
{
    u16 btn = 0;
    saiResult[work->playerNo] = 0;
    while(1) {
        do {
            HuPrcVSleep();
        } while(MBPauseProcCheck());
        if(saiPadBtnHook[work->playerNo]) {
            btn = saiPadBtnHook[work->playerNo](work->playerNo);
        }
        if(btn == PAD_BUTTON_A) {
            break;
        }
        if(work->no != 0 ||
            work->saiType == SAITYPE_KUPA ||
            work->saiType == SAITYPE_PATAPATA_COIN ||
            work->saiType == SAITYPE_CHANCE_TRADE ||
            work->saiType == SAITYPE_CHANCE_CHAR ||
            work->saiType == SAITYPE_BLOCK) {
            continue;
        }
        if(btn == PAD_BUTTON_Y) {
            saiResult[work->playerNo] = SAI_RESULT_SCROLL;
            MBAudFXPlay(MSM_SE_CMN_02);
            break;
        }
        if(btn == PAD_BUTTON_B &&
            work->playerNo < GW_PLAYER_MAX &&
            MBPlayerCapsuleUseGet() == CAPSULE_NULL &&
            MBPlayerCapsuleNumGet(work->playerNo) != 0
            && !_CheckFlag(FLAG_MG_CIRCUIT)) {
            saiResult[work->playerNo] = SAI_RESULT_CAPSULESEL;
            break;
        }
    }
}

static u16 SaiPadBtnDefault(int playerNo)
{
    u16 ret = 0;
    if(!GwPlayer[playerNo].comF) {
        int padNo = GwPlayer[playerNo].padNo;
        ret = HuPadBtnDown[padNo];
    } else {
        ret = PAD_BUTTON_A;
    }
    return ret;
}

static void SaiReadyWait(SAIWORK *work)
{
    SAIOBJWORK *objWork = omObjGetWork(work->obj, SAIOBJWORK);
    while(objWork->mode != SAIOBJ_MODE_IDLE) {
        HuPrcVSleep();
    }
}

static void SaiObjHit(SAIWORK *work)
{
    OMOBJ *obj = work->obj;
    SAIOBJWORK *objWork = omObjGetWork(work->obj, SAIOBJWORK);
    objWork->mode = SAIOBJ_MODE_HIT;
    objWork->lockF = TRUE;
    objWork->time = 0;
    objWork->maxTime = 12;
    if(work->playerNo >= 0 && work->playerNo < GW_PLAYER_MAX) {
        omVibrate(work->playerNo, 12, 4, 2);
    }
    if(objWork->saiSeNo >= 0) {
        HuAudFXStop(objWork->saiSeNo);
        objWork->saiSeNo = MSM_SENO_NONE;
    }
    Hu3DModelDispOn(obj->mdlId[2]);
    Hu3DModelDispOn(obj->mdlId[3]);
    if(work->saiType != SAITYPE_CHANCE_TRADE && work->saiType != SAITYPE_CHANCE_CHAR) {
        if(work->tutorialVal[work->no] >= 0) {
            work->result[work->no] = work->tutorialVal[work->no]+1;
            MBMotionTimeSet(obj->mdlId[0], work->tutorialVal[work->no]+0.5f);
        } else {
            work->result[work->no] = work->valueTbl[objWork->valueNo]+1;
        }
        if(work->saiType == SAITYPE_PATAPATA_COIN) {
            work->result[work->no] = saiPatapataResultTbl[work->result[work->no]-1];
        }
    } else {
        if(work->tutorialVal[work->no] >= 0) {
            work->result[work->no] = work->tutorialVal[work->no];
            MBMotionTimeSet(obj->mdlId[0], work->result[work->no]-0.5f);
        } else {
            work->result[work->no] = MBMotionTimeGet(obj->mdlId[0])-0.5f;
        }
    }
    MBAudFXPlay(MSM_SE_BOARD_04);
    if(saiHitHook[work->playerNo]) {
        saiHitHook[work->playerNo](work->result[work->no]);
    }
}

BOOL MBSaiKillCheck(int playerNo)
{
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    return saiProc[playerNo] == NULL;
}

BOOL MBSaiKillCheckAll(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBSaiKillCheck(i)) {
            return FALSE;
        }
    }
    return TRUE;
}

int MBSaiResultGet(int playerNo)
{
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    return saiResult[playerNo];
}

void MBSaiHitHookSet(int playerNo, SAIHITHOOK hook)
{
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    saiHitHook[playerNo] = hook;
}

void MBSaiObjHit(int playerNo)
{
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    SaiObjHit(saiProc[playerNo]->property);
}

void MBSaiPadBtnHookSet(int playerNo, SAIPADBTNHOOK hook)
{
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    saiPadBtnHook[playerNo] = hook;
}

void MBSaiMotHookSet(int playerNo, SAIMOTHOOK hook)
{
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    saiMotHook[playerNo] = hook;
}

typedef struct SaiNumWork_s {
    unsigned killF : 1;
    unsigned rotF : 1;
    unsigned updateF : 1;
    unsigned modelNo : 2;
    unsigned bendF : 1;
    unsigned playerNo : 2;
    s8 value;
    s16 bendMode;
    s16 time;
    s16 maxTime;
} SAINUMWORK;

typedef struct SaiNumVtx_s {
    HuVecF pos;
    float weight;
} SAINUMVTX;

static void SaiNumObjExec(OMOBJ *obj);
static SAINUMVTX *SaiNumObjMdlCopy(int modelId);
static void SaiNumObjMdlBend(int modelId, SAINUMVTX *vtx, Mtx mtx1, Mtx mtx2);

static void FixFloatOrder(void)
{
    (void)0.01745329252f;
}

static void MBSaiPosSet(HuVecF *src, HuVecF *dst)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    Mtx lookAt;
    HuVecF normPos;
    HuVecF camPos;
    
    float tanFov;
    float x, y;
    
    
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXMultVec(lookAt, src, &camPos);
    tanFov = HuSin(cameraP->fov*0.5f)/HuCos(cameraP->fov*0.5f);
    x = camPos.x/(tanFov*camPos.z);
    y = camPos.y/(tanFov*camPos.z);
    normPos.x = -2000*(x*tanFov);
    normPos.y = -2000*(y*tanFov);
    normPos.z = -2000;
    MTXInverse(lookAt, lookAt);
    MTXMultVec(lookAt, &normPos, dst);
}

OMOBJ *MBSaiNumObjCreate(int playerNo, HuVecF *pos1, HuVecF *pos2, int value, int color, BOOL followF)
{
    SAIWORK *saiWork = saiProc[playerNo]->property;
    OMOBJ *obj;
    SAINUMWORK *objWork;
    SAINUMVTX **vtx;
    HU3DMODELID tempMdlId;
    MBCAMERA *cameraP;
    int modelNo;
    int modelId;
    int digit;
    Mtx rot;
    
    cameraP = MBCameraGet();
    if(value > 99) {
        value = 99;
    }
    obj = MBAddObj(258, 2, 0, SaiNumObjExec);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->data = vtx = HuMemDirectMallocNum(HEAP_HEAP, sizeof(SAINUMVTX *)*2, HU_MEMNUM_OVL);
    memset(obj->data, 0, sizeof(SAINUMVTX *)*2);
    objWork = omObjGetWork(obj, SAINUMWORK);
    objWork->killF = FALSE;
    objWork->rotF = FALSE;
    objWork->updateF = TRUE;
    objWork->value = value;
    objWork->time = 0;
    objWork->maxTime = 45;
    objWork->playerNo = playerNo;
    modelNo = 0;
    digit = value;
    MTXRotDeg(rot, 'y', cameraP->rot.y);
    while(1) {
        modelId = MBModelCreate(MBDATANUM(numberFileTbl[digit%10]), NULL, 0);
        vtx[modelNo] = SaiNumObjMdlCopy(MBModelIdGet(modelId));
        MBModelPosSetV(modelId, pos1);
        MBMotionNoSet(modelId, MB_MOT_DEFAULT, HU3D_MOTATTR_NONE);
        MBMotionSpeedSet(modelId, 0);
        MBMotionTimeSet(modelId, color+0.5f);
        MBModelScaleSet(modelId, 1, 1, 1);
        MBModelMtxSet(modelId, &rot);
        tempMdlId = MBModelIdGet(modelId);
        Hu3DModelAttrSet(tempMdlId, HU3D_ATTR_ZCMP_OFF);
        Hu3DModelLayerSet(tempMdlId, 3);
        obj->mdlId[modelNo] = modelId;
        modelNo++;
        digit = digit/10;
        if(digit == 0) {
            break;
        }
    }
    objWork->modelNo = modelNo;
    for(; modelNo<2; modelNo++) {
        obj->mdlId[modelNo] = MB_MODEL_NONE;
    }
    obj->trans.x = obj->scale.x = pos1->x;
    obj->trans.y = obj->scale.y = pos1->y;
    obj->trans.z = obj->scale.z = pos1->z;
    if(pos2) {
        obj->rot.x = pos2->x;
        obj->rot.y = pos2->y;
        obj->rot.z = pos2->z;
    } else {
        obj->rot.x = pos1->x;
        obj->rot.y = pos1->y;
        obj->rot.z = pos1->z;
    }
    if(!followF) {
        MBSaiPosSet(&obj->rot, &obj->rot);
        MBSaiPosSet(&obj->trans, &obj->scale);
    }
    return obj;
}

static void SaiNumObjExec(OMOBJ *obj)
{
    SAINUMWORK *objWork = omObjGetWork(obj, SAINUMWORK);
    SAINUMVTX **vtx = obj->data;
    float time;
    HuVecF modelPos;
    HuVecF bendPos;
    HuVecF basePos;
    HuVecF dir;
    int i;
    Mtx rot1, rot2;
    if(objWork->killF || MBKillCheck()) {
        MBSaiNumObjKill(obj);
        return;
    }
    if(!objWork->updateF) {
        return;
    }
    if(++objWork->time >= objWork->maxTime) {
        objWork->updateF = FALSE;
    }
    time = (float)objWork->time/objWork->maxTime;
    dir.z = HuCos(time*90);
    dir.y = HuSin(time*90);
    dir.x = HuSin(time*180);
    for(i=0; i<2; i++) {
        float angle;
        if(obj->mdlId[i] < 0) {
            continue;
        }
        angle = 450*time;
        if(angle > 360) {
            angle = 360;
        }
        MTXRotDeg(rot2, 'y', angle);
        angle = (450*time)-90;
        if(angle < 0) {
            angle = 0;
        }
        MTXRotDeg(rot1, 'y', angle);
        SaiNumObjMdlBend(MBModelIdGet(obj->mdlId[i]), vtx[i], rot1, rot2);
        if(objWork->modelNo < 2) {
            bendPos.x = obj->rot.x;
            bendPos.y = obj->rot.y;
            bendPos.z = obj->rot.z;
        } else {
            Mtx modelMtx;
            MBModelMtxGet(obj->mdlId[i], &modelMtx);
            if(i != 0) {
                basePos.x = -50;
                basePos.y = basePos.z = 0;
            } else {
                basePos.x = 50;
                basePos.y = basePos.z = 0;
            }
            MTXMultVec(modelMtx, &basePos, &bendPos);
            bendPos.x += obj->rot.x;
            bendPos.y += obj->rot.y;
            bendPos.z += obj->rot.z;
        }
        modelPos.x = obj->trans.x+(time*(bendPos.x-obj->trans.x));
        if(!objWork->rotF) {
            modelPos.y = obj->trans.y+(time*(bendPos.y-obj->trans.y))+(100*(2*(HuSin(time*180))));
        } else {
            modelPos.y = obj->trans.y+(time*(bendPos.y-obj->trans.y));
        }
        modelPos.z = obj->trans.z+(time*(bendPos.z-obj->trans.z));
        MBModelPosSetV(obj->mdlId[i], &modelPos);
    }
}

static void SaiNumObjBendExec(OMOBJ *obj)
{
    SAINUMWORK *objWork = omObjGetWork(obj, SAINUMWORK);
    SAINUMVTX **vtx = obj->data;
    float angle;
    float time;
    float posY;
    float rotAngle;
    float maxAngle;
    Mtx rot1, rot2;
    HuVecF pos;
    int i;
    if(objWork->killF || MBKillCheck()) {
        MBSaiNumObjKill(obj);
        return;
    }
    if(!objWork->updateF) {
        return;
    }
    switch(objWork->bendMode) {
        case 0:
            time = (float)objWork->time/objWork->maxTime;
            if(objWork->value == 10) {
                rotAngle = 30;
            } else {
                rotAngle = 60;
            }
            maxAngle = rotAngle;
            angle = time*720;
            MTXRotDeg(rot2, 'z', maxAngle*HuSin(angle));
            MTXIdentity(rot1);
            posY = obj->rot.y;
            if(++objWork->time > objWork->maxTime) {
                if(objWork->bendF) {
                    objWork->bendMode = 1;
                    objWork->time = 0;
                    objWork->maxTime = 45;
                } else {
                    objWork->updateF = FALSE;
                }
            }
            break;
        
        case 1:
            time = (float)objWork->time/objWork->maxTime;
            angle = time*450;
            if(angle > 360) {
                angle = 360;
            }
            MTXRotDeg(rot2, 'y', angle);
            angle = (450*time)-90;
            if(angle < 0) {
                angle = 0;
            }
            MTXRotDeg(rot1, 'y', angle);
            posY = obj->rot.y+(100*(2*(HuSin(time*180))));
            if(++objWork->time > objWork->maxTime) {
                if(objWork->bendF) {
                    objWork->bendMode = 0;
                    objWork->time = 0;
                    objWork->maxTime = 120;
                } else {
                    objWork->updateF = FALSE;
                }
            }
            break;
    }
    for(i=0; i<2; i++) {
        if(obj->mdlId[i] >= 0) {
            SaiNumObjMdlBend(MBModelIdGet(obj->mdlId[i]), vtx[i], rot1, rot2);
            MBModelPosGet(obj->mdlId[i], &pos);
            pos.y = posY;
            MBModelPosSetV(obj->mdlId[i], &pos);
        }
    }
    
}

void MBSaiNumObjKill(OMOBJ *obj)
{
    SAINUMWORK *objWork = omObjGetWork(obj, SAINUMWORK);
    int i;
    for(i=0; i<2; i++) {
        if(obj->mdlId[i] != MB_MODEL_NONE) {
            MBModelKill(obj->mdlId[i]);
        }
    }
    MBDelObj(obj);
}

static void SaiNumObjShrinkExec(OMOBJ *obj)
{
    SAINUMWORK *objWork = omObjGetWork(obj, SAINUMWORK);
    HuVecF pos;
    float time;
    float scaleX, scaleY;
    float posY;
    int i;
    if(objWork->time > objWork->maxTime) {
        objWork->killF = TRUE;
    }
    if(objWork->killF || MBKillCheck()) {
        MBSaiNumObjKill(obj);
        return;
    }
    time = (float)(objWork->time++)/objWork->maxTime;
    scaleX = HuSin(time*270);
    scaleY = HuSin((time*270)+180);
    posY = obj->rot.y+(100*time);
    for(i=0; i<2; i++) {
        if(obj->mdlId[i] >= 0) {
            MBModelScaleSet(obj->mdlId[i], 1+scaleX, 1+scaleY, 1);
            MBModelPosGet(obj->mdlId[i], &pos);
            pos.y = posY;
            MBModelPosSetV(obj->mdlId[i], &pos);
        }
    }
}

static void SaiNumObjBendStart(int playerNo)
{
    int i;
    for(i=0; i<3; i++) {
        OMOBJ *obj = saiNumObj[playerNo][i];
        if(obj) {
            SAINUMWORK *work = omObjGetWork(obj, SAINUMWORK);
            work->updateF = TRUE;
            work->bendF = TRUE;
            work->time = 0;
            work->maxTime = 120;
            obj->objFunc = SaiNumObjBendExec;
        }
    }
}

static void SaiNumObjBendStop(int playerNo)
{
    int i;
    for(i=0; i<3; i++) {
        OMOBJ *obj = saiNumObj[playerNo][i];
        if(obj) {
            SAINUMWORK *work = omObjGetWork(obj, SAINUMWORK);
            work->bendF = FALSE;
        }
    }
}

static void SaiNumObjReset(int playerNo)
{
    int i;
    for(i=0; i<3; i++) {
        OMOBJ *obj = saiNumObj[playerNo][i];
        if(obj) {
            SAINUMWORK *work = omObjGetWork(obj, SAINUMWORK);
            work->rotF = TRUE;
            work->updateF = TRUE;
            work->time = 0;
            work->maxTime = 30;
            obj->trans.x = obj->rot.x;
            obj->trans.y = obj->rot.y;
            obj->trans.z = obj->rot.z;
            obj->rot.x = obj->scale.x;
            obj->objFunc = SaiNumObjExec;
        }
    }
}

BOOL MBSaiNumStopCheck(int playerNo)
{
    int i;
    for(i=0; i<3; i++) {
        if(saiNumObj[playerNo][i]) {
            SAINUMWORK *work = omObjGetWork(saiNumObj[playerNo][i], SAINUMWORK);
            if(work->updateF) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void MBSaiNumKill(int playerNo)
{
    int i;
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    for(i=0; i<3; i++) {
        if(saiNumObj[playerNo][i]) {
            SAINUMWORK *work = omObjGetWork(saiNumObj[playerNo][i], SAINUMWORK);
            work->killF = TRUE;
            saiNumObj[playerNo][i] = NULL;
        }
    }
}

void MBSaiNumShrinkSet(int playerNo)
{
    int i;
    if(playerNo < 0) {
        playerNo = SAI_PLAYERNO_NULL;
    }
    for(i=0; i<3; i++) {
        OMOBJ *obj = saiNumObj[playerNo][i];
        if(obj) {
            SAINUMWORK *work = omObjGetWork(obj, SAINUMWORK);
            work->time = 0;
            work->maxTime = 30;
            obj->objFunc = SaiNumObjShrinkExec;
        }
    }
}

void MBSaiStub()
{
    
}

static void SaiMatchBonusExec(SAIWORK *work)
{
    int streamNo = MSM_STREAMNO_NONE;
    int coin;
    int streamId;
    HuVecF posPlayer;
    static char matchCoinStr[8];
    switch(work->max) {
        case 2:
            if(work->result[0] == 7) {
                coin = 30;
            } else {
                coin = 10;
            }
            streamId = MSM_STREAM_JNGL_SAMENUM;
            break;
        
        case 3:
            if(work->result[0] == 7) {
                coin = 50;
            } else {
                coin = 30;
            }
            streamId = MSM_STREAM_JNGL_SAMENUM_TRIPLE;
            break;
        
        default:
            return;
    }
    sprintf(matchCoinStr, "%d", coin);
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 1000);
    MBPlayerPosGet(work->playerNo, &posPlayer);
    SaiNumObjBendStart(work->playerNo);
    HuPrcSleep(10);
    streamNo = MBMusPlayJingle(streamId);
    while(MBMusJingleStatGet(streamNo) != MSM_STREAM_DONE) {
        HuPrcVSleep();
    }
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, FALSE, 1000);
    MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARDOPE_MES_SAI_MATCH, HUWIN_SPEAKER_NONE);
    MBTopWinInsertMesSet(MESSNUM_PTR(matchCoinStr), 0);
    MBTopWinWait();
    MBTopWinKill();
    MBCoinAddExec(work->playerNo, coin);
    SaiNumObjBendStop(work->playerNo);
    while(!MBSaiNumStopCheck(work->playerNo)){ 
        HuPrcVSleep();
    }
    MBMusMainPlay();
}

static HU3DPARMANPARAM saiCloudParMan = {
    30,
    264,
    50,
    0,
    65,
    { 0, 0, 0 },
    15,
    0.9,
    60,
    0.95,
    1,
    {
        { 255, 255, 255, 255 },
        { 0, 0, 0, 0 }
    },
    {
        { 255, 255, 255, 0 },
        { 0, 0, 0, 0 }
    }
};

static HU3DPARMANID SaiObjCloudCreate(void)
{
    ANIMDATA *anim = HuSprAnimDataRead(MBDATANUM(BOARD_ANM_effCloud));
    int parManId = Hu3DParManCreate(anim, 100, &saiCloudParMan);
    HU3DMODELID modelId;
    Hu3DParManTimeLimitSet(parManId, 30);
    modelId = Hu3DParManModelIDGet(parManId);
    Hu3DModelLayerSet(modelId, 3);
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    return parManId;
}

static float saiCloudRotYRatio[3] = {
    (1.0f/1.0f), (1.0f/2.0f), (1.0f/3.0f)
};

static void SaiObjEffectCreate(SAIWORK *work)
{
    int hitOrderTbl[20] = {
        0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2
    };
    float posY;
    SAIEFF *eff;
    
    void *dlBuf;
    int i;
    work->eff = eff = HuMemDirectMallocNum(HEAP_HEAP, (SAIEFF_PUFF_MAX+SAIEFF_TRI_MAX)*sizeof(SAIEFF), HU_MEMNUM_OVL);
    memset(eff, 0, (SAIEFF_PUFF_MAX+SAIEFF_TRI_MAX)*sizeof(SAIEFF));
    for(i=0; i<50; i++, eff++) {
        eff->rot.x = (frandf()*15)+30;
        eff->rot.y = frandf()*360;
        eff->radius = (1+frandf())*100;
    }
    for(i=0; i<100; i++) {
        int idx1, idx2;
        int temp;
        idx1 = frandmod(20);
        idx2 = frandmod(20);
        temp = hitOrderTbl[idx1];
        hitOrderTbl[idx1] = hitOrderTbl[idx2];
        hitOrderTbl[idx2] = temp;
    }
    posY = 0;
    for(i=0; i<20; i++, eff++) {
        eff->rot.x = 0;
        eff->rot.y = posY;
        eff->radius = 150;
        eff->scale.x = saiCloudRotYRatio[hitOrderTbl[i]];
        eff->scale.y = 1;
        eff->scale.z = 1+(2*frandf());
        posY += (360*eff->scale.x)/12;
    }
    dlBuf = HuMemDirectMallocNum(HEAP_HEAP, 4096, HU_MEMNUM_OVL);
    DCInvalidateRange(dlBuf, 4096);
    GXBeginDisplayList(dlBuf, 4096);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-20, 0, -20);
    GXTexCoord2f32(0, 0);
    GXPosition3f32(20, 0, -20);
    GXTexCoord2f32(1, 0);
    GXPosition3f32(20, 0, 20);
    GXTexCoord2f32(1, 1);
    GXPosition3f32(-20, 0, 20);
    GXTexCoord2f32(0, 1);
    GXEnd();
    work->dlSize = GXEndDisplayList();
    work->dlBuf = HuMemDirectMallocNum(HEAP_HEAP, work->dlSize, HU_MEMNUM_OVL);
    memcpy(work->dlBuf, dlBuf, work->dlSize);
    HuMemDirectFree(dlBuf);
    DCFlushRangeNoSync(work->dlBuf, work->dlSize);
    work->animEffPuff = HuSprAnimDataRead(MBDATANUM(BOARD_ANM_effPuff));
    work->animEffTri = HuSprAnimDataRead(MBDATANUM(BOARD_ANM_effTri));
}

static void SaiObjEffectKill(SAIWORK *work)
{
    if(work->dlBuf != NULL) {
        HuMemDirectFree(work->dlBuf);
        work->dlBuf = NULL;
    }
    if(work->eff != NULL) {
        HuMemDirectFree(work->eff);
        work->eff = NULL;
    }
    if(work->animEffPuff != NULL) {
        HuSprAnimKill(work->animEffPuff);
        work->animEffPuff = NULL;
    }
    if(work->animEffTri != NULL) {
        HuSprAnimKill(work->animEffTri);
        work->animEffTri = NULL;
    }
}

static void SaiObjEffPuffDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    OMOBJ *obj = modelP->hookData;
    SAIWORK *work = obj->data;
    GXColor color = { 0, 0, 0, 255 };
    GXColor white = { 255, 255, 255, 255 };
    SAIEFF *eff;
    int i;
    float time = work->puffTime/45.0f;
    if(time > 1) {
        time = 1;
    }
    HuSprTexLoad(work->animEffPuff, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_NEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    color.a = (1-time)*255;
    GXSetTevColor(GX_TEVREG0, color);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_NONE);
    for(eff=work->eff, i=0; i<SAIEFF_PUFF_MAX; i++, eff++) {
        Mtx modelview;
        Mtx rot;
        Mtx trans;
        HuVecF pos;
        static HuVecF back = { 0, 0, -1 };
        mtxRot(rot, eff->rot.x, eff->rot.y, eff->rot.z);
        MTXMultVec(rot, &back, &pos);
        VECScale(&pos, &pos, HuSin(time*90)*eff->radius);
        MTXTrans(trans, obj->trans.x+pos.x, obj->trans.y+pos.y+50, obj->trans.z+pos.z);
        MTXConcat(trans, rot, modelview);
        MTXConcat(*mtx, modelview, modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXCallDisplayList(work->dlBuf, work->dlSize);
    }
    if(!MBPauseFlagCheck()) {
        if(++work->puffTime >= 45u) {
            modelP->attr |= HU3D_ATTR_DISPOFF;
        }
    }
}


static void SaiObjEffTriDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    OMOBJ *obj = modelP->hookData;
    SAIWORK *work = obj->data;
    GXColor color = { 255, 192, 64, 255 };
    float time = work->triTime/16.0f;
    float alpha;
    SAIEFF *eff;
    int i;
    if(time > 1) {
        time = 1;
    }
    HuSprTexLoad(work->animEffTri, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_NEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    alpha = 255*(4*(1-time));
    if(alpha > 255) {
        alpha = 255;
    }
    color.a = alpha;
    GXSetChanMatColor(GX_COLOR0A0, color);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_BACK);
    for(eff=work->eff+SAIEFF_PUFF_MAX, i=0; i<SAIEFF_TRI_MAX; i++, eff++) {
        Mtx modelview;
        Mtx trans;
        Mtx rot;
        Mtx scale;
        HuVecF pos;
        static HuVecF back = { 0, 0, -1 };
        MTXScale(scale, (2*eff->scale.x)+time, 2*eff->scale.y, 2*eff->scale.z);
        mtxRot(rot, eff->rot.x, eff->rot.y, eff->rot.z);
        MTXMultVec(rot, &back, &pos);
        VECScale(&pos, &pos, HuSin(time*90)*eff->radius);
        MTXTrans(trans, obj->trans.x+pos.x, obj->trans.y+pos.y-50, obj->trans.z+pos.z);
        MTXConcat(rot, scale, modelview);
        MTXConcat(trans, modelview, modelview);
        MTXConcat(*mtx, modelview, modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-20, 0, -30);
        GXTexCoord2f32(0, 0);
        GXPosition3f32(20, 0, -30);
        GXTexCoord2f32(1, 0);
        GXPosition3f32(20, 0, 10);
        GXTexCoord2f32(1, 1);
        GXPosition3f32(-20, 0, 10);
        GXTexCoord2f32(0, 1);
        GXEnd();
    }
    if(!MBPauseFlagCheck()) {
        if(++work->triTime >= 16) {
            modelP->attr |= HU3D_ATTR_DISPOFF;
        }
    }
}

static float saiCloudRotYRatio2[3] = {
    (1.0f/1.0f), (1.0f/2.0f), (1.0f/3.0f)
};

static SAINUMVTX *SaiNumObjMdlCopy(int modelId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFOBJECT *obj = hsf->object;
    HuVecF *pos;
    SAINUMVTX *vtx;
    int size;
    SAINUMVTX *vtxBuf;
    
    int i, j;
    float posMin, posMax;
    float sizeY;
    for(i=0; i<hsf->objectNum; i++, obj++) {
        if(obj->type == HSF_OBJ_MESH) {
            size = obj->mesh.vertex->count*sizeof(SAINUMVTX);
            pos = obj->mesh.vertex->data;
            posMin = posMax = pos->y;
            pos++;
            for(j=1; j<obj->mesh.vertex->count; j++, pos++) {
                if(pos->y > posMax) {
                    posMax = pos->y;
                }
                if(pos->y < posMin) {
                    posMin = pos->y;
                }
            }
            sizeY = posMax-posMin;
            vtxBuf = vtx = HuMemDirectMallocNum(HEAP_MODEL, size, modelP->mallocNo);
            pos = obj->mesh.vertex->data;
            for(j=0; j<obj->mesh.vertex->count; j++, pos++, vtx++) {
                vtx->pos = *pos;
                vtx->weight = (pos->y-posMin)/sizeY;
            }
            break;
        }
    }
    return vtxBuf;
}

static void SaiNumObjMdlBend(int modelId, SAINUMVTX *vtx, Mtx mtx1, Mtx mtx2)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFOBJECT *obj = hsf->object;
    int i, j;
    for(i=0; i<hsf->objectNum; i++, obj++) {
        if(obj->type == HSF_OBJ_MESH) {
            HuVecF *pos;
            pos = obj->mesh.vertex->data;
            for(j=0; j<obj->mesh.vertex->count; j++, vtx++, pos++) {
                HuVecF vtx1, vtx2;
                MTXMultVec(mtx1, &vtx->pos, &vtx1);
                MTXMultVec(mtx2, &vtx->pos, &vtx2);
                pos->x = vtx1.x+(vtx->weight*(vtx2.x-vtx1.x));
                pos->y = vtx1.y+(vtx->weight*(vtx2.y-vtx1.y));
                pos->z = vtx1.z+(vtx->weight*(vtx2.z-vtx1.z));
            }
            DCStoreRangeNoSync(obj->mesh.vertex->data, obj->mesh.vertex->count*sizeof(HuVecF));
            break;
        }
    }
}