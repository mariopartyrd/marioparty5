#include "game/board/coin.h"
#include "game/board/audio.h"

#include "game/sprite.h"

#define COIN_EFF_MAX 16

static MBCOINEFF coinEffWork[COIN_EFF_MAX];

static OMOBJ *coinDispObj[GW_PLAYER_MAX] = {};
static HU3DPARMANPARAM coinEffParam = {
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
    2,
    {
        { 255, 255, 128, 255 },
        { 0, 0, 0, 0 }
    },
    {
        { 255, 255, 128, 0 },
        { 0, 0, 0, 0 }
    }
};

void MBCoinInit(void)
{
    MBCOINEFF *eff;
    int i;
    memset(&coinEffWork[0], 0, sizeof(coinEffWork));
    for(eff=&coinEffWork[0], i=0; i<COIN_EFF_MAX; i++, eff++) {
        eff->count = -1;
    }
}

void MBCoinClose(void)
{
    int i;
    for(i=0; i<COIN_EFF_MAX; i++) {
        if(coinEffWork[i].count >= 0) {
            MBCoinEffKill(i);
        }
    }
}

static void CoinEffProc(void);
static void CoinEffKill(void);

s16 MBCoinEffCreate(int count, MBCOINEFFHOOK hook)
{
    MBCOINEFF *eff;
    MBCOINEFFDATA *effData;
    int i;
    s16 effNo;
    for(eff=&coinEffWork[0], i=0; i<COIN_EFF_MAX; i++, eff++) {
        if(eff->count < 0) {
            break;
        }
    }
    if(i >= COIN_EFF_MAX) {
        return MBCOIN_EFF_NONE;
    }
    effNo = i;
    eff->effNo = effNo;
    eff->count = count;
    eff->data = effData = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, eff->count*sizeof(MBCOINEFFDATA), HU_MEMNUM_OVL);
    eff->attr = MBCOIN_ATTR_NONE;
    memset(effData, 0, eff->count*sizeof(MBCOINEFFDATA));
    for(i=0; i<count; i++, effData++) {
        effData->dispF = TRUE;
        effData->modelId = MBModelCreate(BOARD_HSF_coin, NULL, TRUE);
        effData->scale.x = effData->scale.y = effData->scale.z = 0.5f;
        effData->no = i;
    }
    eff->hook = hook;
    eff->proc = MBPrcCreate(CoinEffProc, 260, 8192);
    eff->proc->property = eff;
    MBPrcDestructorSet(eff->proc, CoinEffKill);
    return effNo;
}

void MBCoinEffKill(s16 effNo)
{
    MBCOINEFF *eff = &coinEffWork[effNo];
    HuPrcKill(eff->proc);
}

static void CoinEffProc(void)
{
    MBCOINEFF *eff = HuPrcCurrentGet()->property;
    MBCOINEFFDATA *effData;
    int i;
    int num;
    do {
        while(eff->attr & MBCOIN_ATTR_IDLE) {
            HuPrcVSleep();
        }
        num = 0;
        if(eff->hook) {
            eff->hook(eff);
        }
        for(effData=eff->data, i=0; i<eff->count; i++, effData++) {
            if(effData->modelId < 0) {
                continue;
            }
            if(!effData->dispF) {
                MBModelDispSet(effData->modelId, FALSE);
            } else {
                if(!effData->timeEndF) {
                    if(0 == effData->scale.x || 0 == effData->scale.y || 0 == effData->scale.z) {
                        MBModelDispSet(effData->modelId, FALSE);
                    } else {
                        MBModelDispSet(effData->modelId, TRUE);
                    }
                    MBCoinPosSetV(effData->modelId, &effData->pos);
                    MBCoinRotSetV(effData->modelId, &effData->rot);
                    MBCoinScaleSetV(effData->modelId, &effData->scale);
                    if(effData->hitF) {
                        effData->parManId = MBCoinHitExec(effData->modelId);
                        effData->timeEndF = TRUE;
                        effData->hitF = FALSE;
                        effData->timeHit = 30;
                    }
                } else {
                    if(--effData->timeHit == 0) {
                        effData->dispF = FALSE;
                        MBCoinParManKill(effData->parManId);
                    }
                }
                num++;
            }
        }
        HuPrcVSleep();
    } while(num != 0);
    HuPrcEnd();
}

static void CoinEffKill(void)
{
    MBCOINEFF *eff = HuPrcCurrentGet()->property;
    MBCOINEFFDATA *effData;
    int i;
    for(effData=eff->data, i=0; i<eff->count; i++, effData++) {
        if(effData->modelId > 0) {
            MBModelKill(effData->modelId);
        }
    }
    HuMemDirectFree(eff->data);
    eff->count = -1;
    eff->proc = NULL;
}

BOOL MBCoinEffCheck(s16 effNo)
{
    MBCOINEFF *eff = &coinEffWork[effNo];
    return eff->proc == NULL;
}

MBCOINEFF *MBCoinEffGet(s16 effNo)
{
    return &coinEffWork[effNo];
}

void MBCoinPosSetV(MBMODELID modelId, HuVecF *pos)
{
    MBModelPosSetV(modelId, pos);
}

void MBCoinPosSet(MBMODELID modelId, float posX, float posY, float posZ)
{
    MBModelPosSet(modelId, posX, posY, posZ);
}

void MBCoinPosGet(MBMODELID modelId, HuVecF *pos)
{
    MBModelPosGet(modelId, pos);
}

void MBCoinRotSetV(MBMODELID modelId, HuVecF *rot)
{
    MBModelRotSetV(modelId, rot);
}

void MBCoinRotSet(MBMODELID modelId, float rotX, float rotY, float rotZ)
{
    MBModelRotSet(modelId, rotX, rotY, rotZ);
}

void MBCoinRotGet(MBMODELID modelId, HuVecF *rot)
{
    MBModelRotGet(modelId, rot);
}

void MBCoinScaleSetV(MBMODELID modelId, HuVecF *scale)
{
    MBModelScaleSetV(modelId, scale);
}

void MBCoinScaleSet(MBMODELID modelId, float scaleX, float scaleY, float scaleZ)
{
    MBModelScaleSet(modelId, scaleX, scaleY, scaleZ);
}

void MBCoinScaleGet(MBMODELID modelId, HuVecF *scale)
{
    MBModelScaleGet(modelId, scale);
}

void MBCoinEffAttrSet(s16 effNo, u32 attr)
{
    MBCOINEFF *eff = &coinEffWork[effNo];
    eff->attr |= attr;
}

void MBCoinEffAttrReset(s16 effNo, u32 attr)
{
    MBCOINEFF *eff = &coinEffWork[effNo];
    eff->attr &= ~attr;
}

HU3DPARMANID MBCoinHitExec(MBMODELID modelId)
{
    HU3DPARMANID parManId;
    HuVecF pos;
    MBModelPosGet(modelId, &pos);
    MBModelDispSet(modelId, FALSE);
    parManId = MBCoinParManCreate();
    Hu3DParManPosSet(parManId, pos.x, pos.y, pos.z);
    MBAudFXPlay(MSM_SE_CMN_08);
    return parManId;
}

void MBCoinEffDispOn(s16 effNo, s16 coinNo)
{
    MBCOINEFF *eff = &coinEffWork[effNo];
    MBCOINEFFDATA *effData = &eff->data[coinNo];
    effData->dispF = TRUE;
    effData->hitF = FALSE;
    effData->timeEndF = FALSE;
    effData->scale.x = effData->scale.y = effData->scale.z = 0.5f;
    effData->timeHit = effData->time = 0;
    MBModelDispSet(effData->modelId, TRUE);
}

HU3DPARMANID MBCoinParManCreate(void)
{
    ANIMDATA *anim = HuSprAnimDataRead(BOARD_ANM_effCoin);
    HU3DPARMANID parManId = Hu3DParManCreate(anim, 10, &coinEffParam);
    HU3DMODELID modelId;
    Hu3DParManTimeLimitSet(parManId, 30);
    modelId = Hu3DParManModelIDGet(parManId);
    Hu3DModelLayerSet(modelId, 3);
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    return parManId;
}

void MBCoinParManKill(HU3DPARMANID parManId)
{
    Hu3DParManKill(parManId);
}

typedef struct coinDispWork_s {
    u8 killF : 1;
    u8 sign : 1;
    u8 mode : 3;
    u8 modelNum;
    s16 no;
    u16 delay;
    u16 time;
    u16 maxTime;
} COINDISPWORK;

typedef struct coinDispModel_s {
    HuVecF pos;
    HuVecF scale;
} COINDISPMODEL;

#define COINDISP_MODE_ON 0
#define COINDISP_MODE_MAIN 1
#define COINDISP_MODE_NONE 2
#define COINDISP_MODE_OFF 3
#define COINDISP_MODEL_MAX 5

static void CoinDispInit(OMOBJ *obj, int coinNum);
static void CoinDispExec(OMOBJ *obj);
static void CoinDispModelUpdate(OMOBJ *obj);

s16 MBCoinDispCreate(HuVecF *pos, int coinNum, BOOL playSe)
{
    OMOBJ *obj;
    COINDISPWORK *work;
    s8 i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!coinDispObj[i]) {
            break;
        }
    }
    if(i >= GW_PLAYER_MAX) {
        return MBCOIN_DISP_NONE;
    }
    if(coinNum > 999) {
        coinNum = 999;
    } else if(coinNum < -999) {
        coinNum = -999;
    }
    obj = MBAddObj(261, 5, 0, CoinDispExec);
    obj->data = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, COINDISP_MODEL_MAX*sizeof(COINDISPMODEL), HU_MEMNUM_OVL);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    coinDispObj[i] = obj;
    work = omObjGetWork(obj, COINDISPWORK);
    work->killF = FALSE;
    work->sign = (coinNum < 0) ? 1 : 0;
    work->mode = COINDISP_MODE_ON;
    work->no = i+1;
    work->delay = 0;
    work->time = 0;
    obj->trans.x = pos->x;
    obj->trans.y = pos->y;
    obj->trans.z = pos->z;
    obj->rot.x = 0;
    obj->rot.y = 0.01f;
    CoinDispInit(obj, coinNum);
    CoinDispModelUpdate(obj);
    if(playSe && coinNum != 0) {
        if(coinNum >= 0) {
            MBAudFXPlay(MSM_SE_BOARD_123);
        } else {
            MBAudFXPlay(MSM_SE_BOARD_124);
        }
    }
    return work->no;
}

s16 MBCoinDispCreateSe(HuVecF *pos, int coinNum)
{
    return MBCoinDispCreate(pos, coinNum, TRUE);
}


static int signMdlTbl[] = {
    BOARD_HSF_numPlus,
    BOARD_HSF_numMinus
};

static int numberFileTbl[] = {
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

static void CoinDispInit(OMOBJ *obj, int coinNum)
{
    int digitVal;
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    COINDISPMODEL *model = obj->data;
    BOOL dispF = FALSE;
    float motTime;
    int i;
    int modelNum;
    
    obj->mdlId[0] = MBModelCreate(BOARD_HSF_coin, NULL, FALSE);
    if(work->sign) {
        motTime = 2.5f;
    } else {
        motTime = 1.5f;
    }
    obj->mdlId[1] = MBModelCreate(signMdlTbl[work->sign], NULL, FALSE);
    MBMotionNoSet(obj->mdlId[1], MB_MOT_DEFAULT, HU3D_MOTATTR_NONE);
    MBMotionTimeSet(obj->mdlId[1], motTime);
    MBMotionSpeedSet(obj->mdlId[1], 0);
    digitVal = 100;
    work->modelNum = 0;
    modelNum = 2;
    coinNum = abs(coinNum);
    for(i=0; i<3; i++) {
        int digit = coinNum/digitVal;
        if(i == 2) {
            dispF = TRUE;
        }
        if(dispF || digit != 0) {
            dispF = TRUE;
            obj->mdlId[modelNum] = MBModelCreate(numberFileTbl[digit], NULL, FALSE);
            MBMotionTimeSet(obj->mdlId[modelNum], motTime);
            MBMotionSpeedSet(obj->mdlId[modelNum], 0);
            MBModelDispSet(obj->mdlId[modelNum], dispF);
            modelNum++;
        }
        coinNum -= digit*digitVal;
        digitVal /= 10;
    }
    work->modelNum = modelNum;
    for(i=0; i<work->modelNum; i++, model++) {
        model->pos.x = obj->trans.x;
        model->pos.y = obj->trans.y;
        model->pos.z = obj->trans.z;
        model->scale.x = model->scale.y = model->scale.z = 0.001f;
        MBModelLayerSet(obj->mdlId[i], 3);
    }
}

static void CoinDispOn(OMOBJ *obj);
static void CoinDispMain(OMOBJ *obj);
static void CoinDispOff(OMOBJ *obj);

static void CoinDispModelKill(OMOBJ *obj);

static void CoinDispExec(OMOBJ *obj)
{
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    if(work->killF || MBKillCheck()) {
        CoinDispModelKill(obj);
        coinDispObj[work->no-1] = NULL;
        MBDelObj(obj);
        return;
    }
    if(work->delay == 0) {
        switch(work->mode) {
            case COINDISP_MODE_ON:
                CoinDispOn(obj);
                break;
            
            case COINDISP_MODE_MAIN:
                CoinDispMain(obj);
                break;
           
            case COINDISP_MODE_OFF:
                CoinDispOff(obj);
                break;
        }
    } else {
        work->delay--;
    }
    CoinDispModelUpdate(obj);
}

static void CoinDispModelUpdate(OMOBJ *obj)
{
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    COINDISPMODEL *model = obj->data;
    int i;
    for(i=0; i<work->modelNum; i++, model++) {
        HuVecF pos, pos2D;
        float scale;
        float z;
        MB3Dto2D(&model->pos, &pos2D);
        z = pos2D.z;
        pos2D.z = 800;
        MB2Dto3D(&pos2D, &pos);
        scale = 800/z;
        MBModelPosSetV(obj->mdlId[i], &pos);
        MBModelScaleSet(obj->mdlId[i], model->scale.x*scale, model->scale.y*scale, model->scale.z*scale);
    }
}

static void CoinDispOn(OMOBJ *obj)
{
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    COINDISPMODEL *model = obj->data;
    float s, scale;
    int i;
    s = HuSin((float)work->time);
    scale = s;
    obj->rot.x = 405*s;
    model->scale.x = model->scale.y = model->scale.z = scale;
    model->pos.x = obj->trans.x;
    model->pos.y = obj->trans.y;
    model->pos.z = obj->trans.z;
    MBModelRotYSet(obj->mdlId[0], obj->rot.x);
    if(work->time < 90) {
        work->time += 6;
        return;
    }
    work->mode = COINDISP_MODE_MAIN;
    work->time = 0;
    work->maxTime = 30;
    model = ((COINDISPMODEL *)obj->data)+1;
    for(i=1; i<work->modelNum; i++, model++) {
        model->scale.x = model->scale.y = model->scale.z = scale;
        model->pos.x = obj->trans.x;
        model->pos.y = obj->trans.y;
        model->pos.z = obj->trans.z;
        MBModelRotYSet(obj->mdlId[i], obj->rot.x);
    }
}

static void CoinDispMain(OMOBJ *obj)
{
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    COINDISPMODEL *model = obj->data;
    float weight = (float)work->time/work->maxTime;
    float ofsX = -0.5f*((work->modelNum-1)*90.0f);
    float s = HuSin(weight*90);
    float angle;
    float sy;
    
    int i;
    obj->rot.x = 45+(s*315);
    angle = weight*(((work->modelNum-1)*30.0f)+180);
    for(i=0; i<work->modelNum; i++, model++) {
        sy = HuSin(angle);
        if(sy < 0) {
            sy = 0;
        }
        model->pos.x = (s*ofsX)+obj->trans.x;
        model->pos.y = obj->trans.y+(200*sy);
        model->pos.z = obj->trans.z;
        MBModelRotYSet(obj->mdlId[i], obj->rot.x);
        ofsX += 90;
        angle -= 30;
    }
    if(++work->time > work->maxTime) {
        work->mode = COINDISP_MODE_OFF;
        work->time = 0;
        work->maxTime = 24;
        work->delay = 30;
    }
}

static void CoinDispOff(OMOBJ *obj)
{
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    COINDISPMODEL *model = obj->data;
    float weight = (float)work->time/work->maxTime;
    float angle;
    float s;
    float posY;
    float angleBase;
    int i;
    obj->rot.x = (270*HuSin(weight*90))+90;
    angleBase = weight*(((work->modelNum-1)*30.0f)+90);
    for(i=0; i<work->modelNum; i++, model++) {
        angle = angleBase-(i*30.0f);
        if(angle < 0) {
            angle = 0;
        } else if(angle > 90) {
            angle = 90;
        }
        s = HuSin(angle);
        if(work->sign) {
            posY = (-100*s)+obj->trans.y;
        } else {
            posY = (100*s)+obj->trans.y;
        }
        model->pos.y = posY;
        MBModelAlphaSet(obj->mdlId[i], HuCos(angle)*255);
        MBModelRotYSet(obj->mdlId[i], obj->rot.x);
    }
    if(++work->time > work->maxTime) {
        for(i=0; i<work->modelNum; i++) {
            MBModelDispSet(obj->mdlId[i], FALSE);
        }
        work->killF = TRUE;
    }
}

static void CoinDispModelKill(OMOBJ *obj)
{
    COINDISPWORK *work = omObjGetWork(obj, COINDISPWORK);
    int i;
    for(i=0; i<work->modelNum; i++) {
        if(obj->mdlId[i] >= 0) {
            MBModelKill(obj->mdlId[i]);
            obj->mdlId[i] = MB_MODEL_NONE;
        }
    }
}

void MBCoinDispKill(s16 no)
{
    if(no <= 0 || no > GW_PLAYER_MAX) {
        return;
    }
    if(coinDispObj[no-1]) {
        omObjGetWork(coinDispObj[no-1], COINDISPWORK)->killF = TRUE;
    }
}

BOOL MBCoinDispKillCheck(s16 no)
{
    if(no <= 0 || no > GW_PLAYER_MAX) {
        return;
    }
    if(coinDispObj[no-1]) {
        COINDISPWORK *work = omObjGetWork(coinDispObj[no-1], COINDISPWORK);
        return FALSE;
    } else {
        return TRUE;
    }
}

#undef COINDISP_MODE_ON
#undef COINDISP_MODE_MAIN
#undef COINDISP_MODE_NONE
#undef COINDISP_MODE_OFF
#undef COINDISP_MODEL_MAX

int MBCoinAddProcExec(int playerNo, int coinNum, BOOL dispF, BOOL fastF)
{
    int coinDiff;
    int dispNo;
    int i;
    int delay;
    int coinChg;
    int coinNew;
    s16 seId;
    
    if(abs(coinNum) >= 50) {
        delay = 1;
    } else if(abs(coinNum) >= 20) {
        delay = 3;
    } else {
        delay = 6;
    }
    coinNew = coinNum+MBPlayerCoinGet(playerNo);
    if(coinNew > 999) {
        coinNum = 999-MBPlayerCoinGet(playerNo);
    } else if(coinNew < 0) {
        coinNum = -MBPlayerCoinGet(playerNo);
    }
    coinDiff = coinNum;
    if(!fastF) {
        coinChg = (coinNum >= 0) ? 1 : -1;
        seId = (coinChg > 0) ? MSM_SE_CMN_08 : MSM_SE_CMN_15;
        for(i=0; i<abs(coinNum); i++) {
            MBPlayerCoinAdd(playerNo, coinChg);
            MBAudFXPlay(seId);
            HuPrcSleep(delay);
        }
    } else {
        MBPlayerCoinAdd(playerNo, coinDiff);
    }
    if(coinDiff != 0) {
        MBAudFXPlay(MSM_SE_CMN_16);
    }
    if(dispF && coinDiff != 0) {
        HuVecF pos;
        MBPlayerPosGet(playerNo, &pos);
        pos.y += 250;
        dispNo = MBCoinDispCreateSe(&pos, coinDiff);
        while(!MBCoinDispKillCheck(dispNo)) {
            HuPrcVSleep();
        }
    }
    return coinDiff;
}

int MBCoinAddDispExec(int playerNo, int coinNum, BOOL dispF)
{
    return MBCoinAddProcExec(playerNo, coinNum, dispF, FALSE);
}

int MBCoinAddExec(int playerNo, int coinNum)
{
    return MBCoinAddProcExec(playerNo, coinNum, FALSE, FALSE);
}

void MBCoinAddAllProcExec(int coinNumP1, int coinNumP2, int coinNumP3, int coinNumP4, BOOL dispF, BOOL fastF)
{
    int coinNum[GW_PLAYER_MAX];
    int coinCurr[GW_PLAYER_MAX];
    int delay[GW_PLAYER_MAX];
    int time[GW_PLAYER_MAX];
    int coinChg[GW_PLAYER_MAX];
    int coinDiff[GW_PLAYER_MAX];
    int i;
    coinNum[0] = coinNumP1;
    coinNum[1] = coinNumP2;
    coinNum[2] = coinNumP3;
    coinNum[3] = coinNumP4;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        coinCurr[i] = MBPlayerCoinGet(i);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        int coinNew = coinNum[i]+MBPlayerCoinGet(i);
        if(coinNew > 999) {
            coinNum[i] = 999-MBPlayerCoinGet(i);
        } else if(coinNew < 0) {
            coinNum[i] = -MBPlayerCoinGet(i);
        }
        coinDiff[i] = coinNum[i];
    }
    if(!fastF) {
        int playerNum;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(abs(coinNum[i]) >= 50) {
                delay[i] = 1;
            } else if(abs(coinNum[i]) >= 20) {
                delay[i] = 3;
            } else {
                delay[i] = 6;
            }
            time[i] = delay[i];
            coinChg[i] = (coinNum[i] >= 0) ? 1 : -1;
        }
        
        do {
            playerNum = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(coinNum[i] == 0) {
                    continue;
                }
                if(--time[i] == 0) {
                    MBPlayerCoinAdd(i, coinChg[i]);
                    coinNum[i] -= coinChg[i];
                    MBAudFXPlay(MSM_SE_CMN_08);
                    time[i] = delay[i];
                }
                playerNum++;
            }
            HuPrcVSleep();
        } while(playerNum != 0);
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBPlayerCoinAdd(i, coinDiff[i]);
        }
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(coinDiff[i]) {
            break;
        }
    }
    if(i < GW_PLAYER_MAX) {
        MBAudFXPlay(MSM_SE_CMN_16);
    }
    if(dispF) {
        HuVecF pos;
        int dispNo;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(coinDiff[i]) {
                MBPlayerPosGet(i, &pos);
                pos.y += 250;
                dispNo = MBCoinDispCreateSe(&pos, coinDiff[i]);
            }
        }
        while(!MBCoinDispKillCheck(dispNo)) {
            HuPrcVSleep();
        }
    }
}

void MBCoinAddAllDispExec(int coinNumP1, int coinNumP2, int coinNumP3, int coinNumP4, BOOL dispF)
{
    MBCoinAddAllProcExec(coinNumP1, coinNumP2, coinNumP3, coinNumP4, dispF, FALSE);
}

void MBCoinAddAllExec(int coinNumP1, int coinNumP2, int coinNumP3, int coinNumP4)
{
    MBCoinAddAllProcExec(coinNumP1, coinNumP2, coinNumP3, coinNumP4, FALSE, FALSE);
}