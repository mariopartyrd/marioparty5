#include "game/board/guide.h"
#include "game/board/camera.h"
#include "game/board/audio.h"
#include "game/board/effect.h"
#include "game/frand.h"

#include "datanum/guide.h"
#include "datanum/effect.h"

typedef struct guideWork_s {
    MBMODELID modelId;
    HU3DMODELID effModelId;
    u32 attr;
    OMOBJ *inObj;
    OMOBJ *screenObj;
    OMOBJ *outObj;
    HuVecF rotModel;
    HuVecF scale;
    HuVecF rot;
    HuVecF posOld;
    s16 time;
    s16 timeMax;
} GUIDEWORK;

#define GUIDE_FILE(charName) \
    GUIDE_HSF_##charName, \
    GUIDE_HSF_##charName##Mot1, \
    GUIDE_HSF_##charName##Mot2, \
    GUIDE_HSF_##charName##Mot3, \
    HU_DATANUM_NONE

static const int guideFileTbl[MB_GUIDE_MAX][5] = {
    GUIDE_FILE(chorl),
    GUIDE_FILE(nerl),
    GUIDE_FILE(terl),
    GUIDE_FILE(nirl),
};

#undef GUIDE_FILE


static GUIDEWORK guideWork;
static GUIDEWORK *guideWorkP = &guideWork;

void MBGuideInit(void)
{
    memset(&guideWork, 0, sizeof(GUIDEWORK));
    guideWork.modelId = MB_MODEL_NONE;
}

static void GuideInObjExec(OMOBJ *obj);
static BOOL GuideInObjCheck(void);
static void GuideOutObjExec(OMOBJ *obj);
static BOOL GuideOutObjCheck(void);
static void GuideExecScreen(OMOBJ *obj);
static HU3DMODELID GuideEffCreate(void);

MBMODELID MBGuideCreate(int guideNo, HuVecF *pos, HuVecF *rot, u32 attr)
{
    MBCAMERA *cameraP = MBCameraGet();
    Mtx lookAt;
    HuVecF posModel;
    if(!(attr & MB_GUIDE_ATTR_MODELOFF)) {
        guideWorkP->modelId = MBModelCreate(guideFileTbl[guideNo][0], (int *)&guideFileTbl[guideNo][1], FALSE);
    }
    if(attr & MB_GUIDE_ATTR_ALTMTX) {
        MBModelPosGet(guideWorkP->modelId, &guideWorkP->posOld);
        guideWorkP->time = 0;
        guideWorkP->timeMax = 12;
    }
    if(pos == NULL) {
        MBModelPosSet(guideWorkP->modelId, 0, 0, 0);
    } else {
        MBModelPosSetV(guideWorkP->modelId, pos);
    }
    guideWorkP->scale.x = guideWorkP->scale.y = guideWorkP->scale.z = 1.5f;
    MBModelScaleSetV(guideWorkP->modelId, &guideWorkP->scale);
    MBMotionNoSet(guideWorkP->modelId, 1, HU3D_MOTATTR_LOOP);
    guideWorkP->rotModel.x = guideWorkP->rotModel.y = guideWorkP->rotModel.z = 0;
    guideWorkP->rot = *rot;
    MBModelRotSetV(guideWorkP->modelId, &guideWorkP->rot);
    if(attr & MB_GUIDE_ATTR_SCREEN) {
        guideWorkP->screenObj = MBAddObj(32257, 0, 0, GuideExecScreen);
        omSetStatBit(guideWorkP->screenObj, OM_STAT_MODEL_PAUSED);
        guideWorkP->screenObj->trans.x = pos->x;
        guideWorkP->screenObj->trans.y = pos->y;
        guideWorkP->screenObj->trans.z = pos->z;
        guideWorkP->scale.x = guideWorkP->scale.y = guideWorkP->scale.z = 1.0f;
        MBModelScaleSetV(guideWorkP->modelId, &guideWorkP->scale);
        Hu3D2Dto3D(&guideWorkP->screenObj->trans, HU3D_CAM0, &posModel);
        MBModelPosSetV(guideWorkP->modelId, &posModel);
    } else {
        guideWorkP->screenObj = NULL;
    }
    if(attr & MB_GUIDE_ATTR_EFFIN) {
        guideWorkP->effModelId = GuideEffCreate();
        MBModelPosGet(guideWorkP->modelId, &posModel);
        Hu3DModelPosSet(guideWorkP->effModelId, posModel.x, posModel.y, posModel.z);
        if(attr & MB_GUIDE_ATTR_SCREEN) {
            HU3DMODEL *modelP;
            MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &posModel);
            MTXInverse(lookAt, lookAt);
            lookAt[0][3] = lookAt[1][3] = lookAt[2][3] = 0;
            modelP = &Hu3DData[guideWorkP->effModelId];
            MTXCopy(lookAt, modelP->mtx);
        }
        MBModelScaleSet(guideWorkP->modelId, 0, guideWorkP->scale.y, 0);
        guideWorkP->inObj = MBAddObj(32257, 0, 0, GuideInObjExec);
        omSetStatBit(guideWorkP->inObj, OM_STAT_MODEL_PAUSED);
        MBAudFXPlay(MSM_SE_BOARD_116);
        MBModelRotGet(guideWorkP->modelId, &guideWorkP->inObj->rot);
        while(!GuideInObjCheck()) {
            HuPrcVSleep();
        }
    } else {
        guideWorkP->effModelId = HU3D_MODELID_NONE;
        guideWorkP->inObj = NULL;
    }
    guideWorkP->attr = attr;
    return guideWorkP->modelId;
}

MBMODELID MBGuideCreateFlag(int guideNo, HuVecF *pos, BOOL screenF, BOOL effInF, BOOL effOutF, BOOL modelOffF)
{
    HuVecF rot = { 0, 0, 0 };
    u32 attr = MB_GUIDE_ATTR_NONE;
    attr |= (screenF) ? MB_GUIDE_ATTR_SCREEN : MB_GUIDE_ATTR_NONE;
    attr |= (effInF) ? MB_GUIDE_ATTR_EFFIN : MB_GUIDE_ATTR_NONE;
    attr |= (effOutF) ? MB_GUIDE_ATTR_EFFOUT : MB_GUIDE_ATTR_NONE;
    attr |= (modelOffF) ? MB_GUIDE_ATTR_MODELOFF : MB_GUIDE_ATTR_NONE;
    return MBGuideCreate(guideNo, pos, &rot, attr);
}


MBMODELID MBGuideCreateIn(HuVecF *pos, BOOL screenF, BOOL effInF, BOOL modelOffF)
{
    int no = (GWPartyFGet() == TRUE) ? MB_GUIDE_CHORL : MB_GUIDE_NERL;
    return MBGuideCreateFlag(no, pos, screenF, effInF, FALSE, modelOffF);
}

MBMODELID MBGuideCreateDirect(void)
{
    return MBGuideCreateIn(NULL, FALSE, FALSE, FALSE);
}

void MBGuideEnd(MBMODELID modelId)
{
    MBCAMERA *cameraP = MBCameraGet();
    Mtx lookAt;
    HuVecF posModel;
    if(guideWorkP->modelId < 0) {
        return;
    }
    if(guideWorkP->attr & MB_GUIDE_ATTR_EFFOUT) {
        if(guideWorkP->effModelId >= 0) {
            MBEffKill(guideWorkP->effModelId);
        }
        guideWorkP->effModelId = GuideEffCreate();
        MBModelPosGet(guideWorkP->modelId, &posModel);
        Hu3DModelPosSet(guideWorkP->effModelId, posModel.x, posModel.y, posModel.z);
        if(guideWorkP->attr & MB_GUIDE_ATTR_SCREEN) {
            HU3DMODEL *modelP;
            MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &posModel);
            MTXInverse(lookAt, lookAt);
            lookAt[0][3] = lookAt[1][3] = lookAt[2][3] = 0;
            modelP = &Hu3DData[guideWorkP->effModelId];
            MTXCopy(lookAt, modelP->mtx);
        }
        guideWorkP->outObj = MBAddObj(32257, 0, 0, GuideOutObjExec);
        omSetStatBit(guideWorkP->outObj, OM_STAT_MODEL_PAUSED);
        MBAudFXPlay(MSM_SE_BOARD_117);
        MBModelRotGet(guideWorkP->modelId, &guideWorkP->outObj->rot);
        while(!GuideOutObjCheck()) {
            HuPrcVSleep();
        }
        HuPrcSleep(60);
    }
    MBModelKill(modelId);
    if(guideWorkP->effModelId >= 0) {
        MBEffKill(guideWorkP->effModelId);
    }
    if(guideWorkP->screenObj) {
        guideWorkP->screenObj->work[0] = TRUE;
    }
    guideWorkP->modelId = MB_MODEL_NONE;
}

void MBGuideKill(void)
{
    MBGuideEnd(guideWorkP->modelId);
}

void MBGuideMdlPosSet(MBMODELID modelId, float posX, float posY, float posZ)
{
    MBModelPosSet(modelId, posX, posY, posZ);
}

void MBGuideMdlPosSetV(MBMODELID modelId, HuVecF *pos)
{
    MBModelPosSet(modelId, pos->x, pos->y, pos->z);
}

void MBGuideMdlPosGet(MBMODELID modelId, HuVecF *pos)
{
    MBModelPosGet(modelId, pos);
}

void MBGuideScreenPosSet(HuVecF *pos)
{
    guideWorkP->screenObj->trans.x = pos->x;
    guideWorkP->screenObj->trans.y = pos->y;
    guideWorkP->screenObj->trans.z = pos->z;
}

void MBGuideScreenPosGet(HuVecF *pos)
{
    pos->x = guideWorkP->screenObj->trans.x;
    pos->y = guideWorkP->screenObj->trans.y;
    pos->z = guideWorkP->screenObj->trans.z;
}

static void GuideExecScreen(OMOBJ *obj)
{
    MBCAMERA *cameraP = MBCameraGet();
    Mtx rotMtx;
    Mtx matrix;
    Mtx scaleMtx;
    HuVecF pos;
    float scale;
    if(obj->work[0] || MBKillCheck()) {
        MBDelObj(obj);
        guideWorkP->screenObj = NULL;
        MBGuideKill();
        return;
    }
    if(!(guideWorkP->attr & MB_GUIDE_ATTR_ALTMTX)) {
        Hu3D2Dto3D(&obj->trans, HU3D_CAM0, &pos);
        MBModelPosSetV(guideWorkP->modelId, &pos);
        MTXLookAt(matrix, &cameraP->pos, &cameraP->up, &pos);
        MTXInverse(matrix, matrix);
        matrix[0][3] = matrix[1][3] = matrix[2][3] = 0;
        mtxRot(rotMtx, guideWorkP->rotModel.x, guideWorkP->rotModel.y, guideWorkP->rotModel.z);
        MTXConcat(matrix, rotMtx, matrix);
        MBModelMtxSet(guideWorkP->modelId, &matrix);
        return;
    }
    if(guideWorkP->time > guideWorkP->timeMax) {
        guideWorkP->attr &= ~MB_GUIDE_ATTR_ALTMTX;
    }
    Hu3D2Dto3D(&obj->trans, HU3D_CAM0, &pos);
    scale = (float)(guideWorkP->time++)/guideWorkP->timeMax;
    pos.x = guideWorkP->posOld.x+(scale*(pos.x-guideWorkP->posOld.x));
    pos.y = guideWorkP->posOld.y+(scale*(pos.y-guideWorkP->posOld.y));
    pos.z = guideWorkP->posOld.z+(scale*(pos.z-guideWorkP->posOld.z));
    MBModelPosSetV(guideWorkP->modelId, &pos);
    MTXLookAt(matrix, &cameraP->pos, &cameraP->up, &pos);
    MTXInverse(matrix, matrix);
    matrix[0][3] = matrix[1][3] = matrix[2][3] = 0;
    MTXIdentity(scaleMtx);
    mtxScaleCat(scaleMtx, 1-scale, 1-scale, 1-scale);
    mtxScaleCat(matrix, scale, scale, scale);
    MTXAdd(scaleMtx, matrix, matrix);
    mtxRot(rotMtx, guideWorkP->rotModel.x, guideWorkP->rotModel.y, guideWorkP->rotModel.z);
    MTXConcat(matrix, rotMtx, matrix);
    MBModelMtxSet(guideWorkP->modelId, &matrix);
}

int MBGuideNoGet(void)
{
    switch(GWPartyFGet()) {
        case TRUE:
            return MB_GUIDE_CHORL;
       
        case FALSE:
            return MB_GUIDE_NERL;
        
        default:
            return MB_GUIDE_CHORL;
    }
}

int MBGuideSpeakerNoGet(void)
{
    static int speakerTbl[2] = {
        HUWIN_SPEAKER_CHORL,
        HUWIN_SPEAKER_NERL
    };
    return speakerTbl[MBGuideNoGet()];
}

static void GuideEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix);

static HU3DMODELID GuideEffCreate(void)
{
    ANIMDATA *anim = HuSprAnimDataRead(EFFECT_ANM_glow);
    HU3DMODELID modelId = MBEffCreate(anim, 20);
    MBEffHookSet(modelId, GuideEffHook);
    Hu3DModelLayerSet(modelId, 3);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_ZWRITE_OFF);
    HuDataDirClose(DATA_effect);
    return modelId;
}

static void GuideEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix)
{
    MBEFFECTDATA *effDataP;
    int i;
    float weight;
    if(effP->count == FALSE) {
        for(effDataP=effP->data, i=0; i<effP->num; i++, effDataP++) {
            effDataP->pos.x = 0;
            effDataP->pos.y = (200*frandf())-100;
            effDataP->pos.z = 0;
            effDataP->vel.x = 360*frandf();
            effDataP->vel.y = (10*frandf())+15;
            effDataP->vel.z = (100*(0.5f*frandf()))+50;
            effDataP->scale = 0;
        }
        effP->count = TRUE;
    }
    switch(effP->mode) {
        case 0:
            if(effP->time > 60u) {
                effP->mode = 1;
                effP->time = 0;
                for(effDataP=effP->data, i=0; i<effP->num; i++, effDataP++) {
                    effDataP->accel.x = 20*HuCos(effDataP->vel.x-90.0f);
                    effDataP->accel.y = 0;
                    effDataP->accel.z = 20*HuSin(effDataP->vel.x-90.0f);
                }
            } else {
                weight = (float)(effP->time++)/60;
                if(weight > 1) {
                    weight = 1;
                }
                for(effDataP=effP->data, i=0; i<effP->num; i++, effDataP++) {
                    if(effDataP->scale < 50) {
                        effDataP->scale += 5;
                    }
                    effDataP->pos.x = effDataP->vel.z*(weight*HuCos(effDataP->vel.x));
                    effDataP->pos.z = effDataP->vel.z*(weight*HuSin(effDataP->vel.x));
                    effDataP->vel.x -= effDataP->vel.y;
                }
            }
            break;
       
        case 1:
            if(effP->time++ > 60u) {
                MBEffKill(guideWorkP->effModelId);
                guideWorkP->effModelId = HU3D_MODELID_NONE;
            } else {
                weight = 0;
                if(effP->time > 30u) {
                    weight = (float)(effP->time-30u)/30;
                }
                for(effDataP=effP->data, i=0; i<effP->num; i++, effDataP++) {
                    VECAdd(&effDataP->pos, &effDataP->accel, &effDataP->pos);
                    effDataP->scale = 50-(50*weight);
                }
            }
            break;
    }
    mtxScaleCat(matrix, guideWorkP->scale.x,guideWorkP->scale.y, guideWorkP->scale.z);
}

#define GUIDE_KILL -1
#define GUIDE_SCALE 0
#define GUIDE_ROTATE 1

static void GuideInObjExec(OMOBJ *obj)
{
    float weight;
    float s;
    HuVecF scale;
    if(obj->work[0] == GUIDE_KILL || MBKillCheck()) {
        MBDelObj(obj);
        guideWorkP->inObj = NULL;
        return;
    }
    switch(obj->work[0]) {
        case GUIDE_SCALE:
            if(obj->work[1] > 60) {
                obj->work[0] = GUIDE_ROTATE;
                obj->work[1] = 0;
                obj->work[2] %= 360;
            } else {
                weight = (float)(obj->work[1]++)/60;
                MBModelScaleGet(guideWorkP->modelId, &scale);
                scale.x = guideWorkP->scale.x*weight;
                scale.z = guideWorkP->scale.z*weight;
                MBModelScaleSetV(guideWorkP->modelId, &scale);
                guideWorkP->rotModel.y = obj->work[2];
                obj->work[2] += 15;
            }
            break;
       
        case GUIDE_ROTATE:
            if(obj->work[1] > 60) {
                obj->work[0] = GUIDE_KILL;
                break;
            }
            weight = (float)(obj->work[1]++)/60;
            s = HuSin(weight*90.0f);
            guideWorkP->rotModel.y = obj->work[2]+(s*(360.0f-obj->work[2]));
            obj->work[2] += 5;
            break;
    }
    if(!guideWorkP->screenObj) {
        HuVecF rot = guideWorkP->rotModel;
        rot.y += guideWorkP->rot.y;
        MBModelRotSetV(guideWorkP->modelId, &rot);
    }
}

static BOOL GuideInObjCheck(void)
{
    return guideWorkP->inObj == NULL;
}

static void GuideOutObjExec(OMOBJ *obj)
{
    float s;
    float weight;
    HuVecF scale;
    
    if(obj->work[0] == GUIDE_KILL || MBKillCheck()) {
        MBDelObj(obj);
        guideWorkP->outObj = NULL;
        return;
    }
    if(obj->work[1] > 60) {
        obj->work[0] = GUIDE_KILL;
        MBModelDispSet(guideWorkP->modelId, FALSE);
        return;
    }
    weight = (float)(obj->work[1]++)/60;
    MBModelScaleGet(guideWorkP->modelId, &scale);
    s = 1-weight;
    scale.x = guideWorkP->scale.x*s;
    scale.z = guideWorkP->scale.z*s;
    MBModelScaleSetV(guideWorkP->modelId, &scale);
    guideWorkP->rotModel.y = obj->work[2];
    obj->work[2] += 15;
    if(!guideWorkP->screenObj) {
        HuVecF rot = guideWorkP->rotModel;
        rot.y += guideWorkP->rot.y;
        MBModelRotSetV(guideWorkP->modelId, &rot);
    }
}

static BOOL GuideOutObjCheck(void)
{
    return guideWorkP->outObj == NULL;
}