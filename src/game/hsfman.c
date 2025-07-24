#include "string.h"
#include "game/init.h"
#include "game/disp.h"

#include "game/hu3d.h"
#include "game/hsfload.h"

#include "game/sprite.h"
#include "game/perf.h"
#include "game/ClusterExec.h"
#include "game/EnvelopeExec.h"
#include "game/ShapeExec.h"

#define HOOKFUNC_MALLOCNO 10000
#define OBJNAME_MAX_LEN 256

#define REFLECT_TEX_W 128
#define REFLECT_TEX_H 128

#define LIGHT_TYPE_SET(lightP, lightType) ((lightP)->type &= 0xFF00); \
    ((lightP)->type |= (lightType))

#define LIGHT_TYPE_GET(lightP) ((lightP)->type & 0xFF)


#include "refMapData0.inc"
#include "refMapData1.inc"
#include "refMapData2.inc"
#include "refMapData3.inc"
#include "refMapData4.inc"
#include "toonMapData.inc"
#include "toonMapData2.inc"
#include "hiliteData.inc"
#include "hiliteData2.inc"
#include "hiliteData3.inc"
#include "hiliteData4.inc"

typedef struct FbCopyLayer_s {
    s16 layerNo;
    s16 x;
    s16 y;
    s16 w;
    s16 h;
    GXTexFmt texFmt;
    BOOL mipmap;
    void *buf;
} FBCOPYLAYER;

static FBCOPYLAYER FbCopyLayer[HU3D_LAYER_HOOK_MAX];
HU3DLIGHT Hu3DLocalLight[HU3D_LLIGHT_MAX];
HU3DLIGHT Hu3DGlobalLight[HU3D_GLIGHT_MAX];
Mtx Hu3DCameraMtxXPose;
Mtx Hu3DCameraMtx;
HSFSCENE FogData;
HU3DSHADOW Hu3DShadowBuf[HU3D_CAM_MAX];
HU3DPROJECTION Hu3DProjection[HU3D_PROJ_MAX];
ANIMDATA *hiliteAnim[4];
ANIMDATA *reflectAnim[5];
static HU3DLAYERHOOK layerHook[HU3D_LAYER_HOOK_MAX];
static s16 layerNum[HU3D_LAYER_MAX];
HU3DCAMERA Hu3DCamera[HU3D_CAM_MAX];
HU3DMODEL Hu3DData[HU3D_MODEL_MAX];

GXColor BGColor;
s16 reflectMapNo;
ANIMDATA *toonAnim;
s16 shadowNum;
s16 Hu3DShadowCamBit;
BOOL Hu3DShadowF;
BOOL shadowModelDrawF;
HU3DSHADOW *Hu3DShadow;
s16 Hu3DProjectionNum;
s16 Hu3DCameraNo;
s16 Hu3DCameraBit;
u32 Hu3DMallocNo;
s16 Hu3DPauseF;
u16 Hu3DCameraExistF;
static u16 NoSyncF;
HU3DMODELID Hu3DReflectModelId;
ANIMDATA *Hu3DReflectModelAnim;
float Hu3DAmbColR;
float Hu3DAmbColG;
float Hu3DAmbColB;
static BOOL modelKillAllF;

void Hu3DInit(void)
{
    s16 i;
    HU3DMODEL *modelP;
    HU3DCAMERA *cameraP;
    for(modelP = &Hu3DData[0], i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        modelP->hsf = NULL;
    }
    for(cameraP = &Hu3DCamera[0], i=0; i<HU3D_CAM_MAX; i++, cameraP++) {
        cameraP->fov = -1;
    }
    Hu3DMotionInit();
    Hu3DLighInit();
    BGColor.r = BGColor.g = BGColor.b = BGColor.a = 0;
    for(i=0; i<HU3D_LAYER_MAX; i++) {
        layerNum[i] = 0;
    }
    for(i=0; i<HU3D_LAYER_HOOK_MAX; i++) {
        layerHook[i] = NULL;
    }
    reflectAnim[0] = HuSprAnimRead(refMapData0);
    reflectAnim[1] = HuSprAnimRead(refMapData1);
    reflectAnim[2] = HuSprAnimRead(refMapData2);
    reflectAnim[3] = HuSprAnimRead(refMapData3);
    reflectAnim[4] = HuSprAnimRead(refMapData4);
    reflectMapNo = 0;
    toonAnim = HuSprAnimRead(toonMapData);
    hiliteAnim[0] = HuSprAnimRead(hiliteData);
    hiliteAnim[1] = HuSprAnimRead(hiliteData2);
    hiliteAnim[2] = HuSprAnimRead(hiliteData3);
    hiliteAnim[3] = HuSprAnimRead(hiliteData4);
    Hu3DFogClear();
    Hu3DAnimInit();
    Hu3DParManInit();
    for(i=0; i<HU3D_PROJ_MAX; i++) {
        Hu3DProjection[i].anim = NULL;
    }
    shadowNum = 0;
    Hu3DShadowCamBit = 0;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        Hu3DShadowBuf[i].buf = NULL;
    }
    Hu3DShadowF = FALSE;
    Hu3DProjectionNum = 0;
    Hu3DCameraExistF = 0;
    modelKillAllF = FALSE;
    Hu3DPauseF = FALSE;
    Hu3DReflectModelId = HU3D_MODELID_NONE;
    Hu3DAmbColorSet(1, 1, 1);
    NoSyncF = FALSE;
}

void Hu3DPreProc(void)
{
    GXColor shadowClear = { 0, 0, 0, 255 };
    s16 i;
    HU3DMODEL *modelP;
    if(shadowNum && Hu3DShadowF) {
        GXSetCopyClear(shadowClear, GX_MAX_Z24);
    } else {
        GXSetCopyClear(BGColor, GX_MAX_Z24);
    }
    for(modelP = &Hu3DData[0], i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        if(modelP->hsf) {
            modelP->attr &= ~(HU3D_ATTR_MOTION_MODEL|HU3D_ATTR_MOT_EXEC);
        }
    }
    totalPolyCnted = totalPolyCnt;
    totalMatCnted = totalMatCnt;
    totalTexCnted = totalTexCnt;
    totalTexCacheCnted = totalTexCacheCnt;
    totalPolyCnt = totalMatCnt = totalTexCnt = totalTexCacheCnt = 0;
    GXSetAlphaUpdate(GX_TRUE);
}

static void Hu3DShadowExec(BOOL bgColorF);
static void Hu3DReflectModelExec(void);

#define HU3D_ATTR_CAMERA_UPDATE (HU3D_ATTR_CAMERA_MOTON|HU3D_ATTR_DISPOFF)
#define HU3D_ATTR_NOUPDATE_ALL (HU3D_ATTR_REFLECT_MODEL|HU3D_ATTR_MOTION_OFF|HU3D_ATTR_DISPOFF)

void Hu3DExec(void)
{
    GXColor clearColor = {};
    HU3DMODEL *modelP;
    s16 i;
    HU3DCAMERA *cameraP;
    s16 layer;
    HU3DPROJECTION *projP;
    s16 vtxInvalidateF;
    s16 syncF;
    HU3DLAYERHOOK hookFunc;
    s16 cameraBit;
    s16 layerMdlNum;
    s16 shadowCameraNo;
    
    HuPerfBegin(HUPERF_USR1);
    GXSetCurrentMtx(GX_PNMTX0);
    shadowModelDrawF = FALSE;
    HuSprBegin();
    syncF = FALSE;
    if(Hu3DReflectModelId != HU3D_MODELID_NONE) {
        Hu3DReflectModelExec();
    }
    if(shadowNum != 0 && Hu3DShadowF) {
        cameraP = &Hu3DCamera[0];
        GXInvalidateVtxCache();
        if(Hu3DShadowCamBit == HU3D_CAM0) {
            Hu3DShadow = &Hu3DShadowBuf[0];
            Hu3DShadowExec(TRUE);
        } else {
            for(cameraP = &Hu3DCamera[0], i=shadowCameraNo=0; i<HU3D_CAM_MAX; i++, cameraP++) {
                if(cameraP->fov == -1) {
                    continue;
                }
                if(Hu3DShadowCamBit & Hu3DCameraBit) {
                    shadowCameraNo = i;
                }
            }
            for(cameraP = &Hu3DCamera[0], Hu3DCameraNo=0; Hu3DCameraNo<HU3D_CAM_MAX; Hu3DCameraNo++, cameraP++) {
                if(cameraP->fov == -1) {
                    continue;
                }
                cameraBit = (1 << Hu3DCameraNo);
                Hu3DCameraBit = cameraBit;
                if(Hu3DShadowCamBit & Hu3DCameraBit) {
                    Hu3DShadow = &Hu3DShadowBuf[Hu3DCameraNo];
                    if(shadowCameraNo == Hu3DCameraNo) {
                        Hu3DShadowExec(TRUE);
                    } else {
                        Hu3DShadowExec(FALSE);
                    }
                }
            }
        }
        if(NoSyncF == FALSE) {
            syncF = TRUE;
            GXSetDrawDone();
        }
    }
    for(cameraP = &Hu3DCamera[0], Hu3DCameraNo=0; Hu3DCameraNo<HU3D_CAM_MAX; Hu3DCameraNo++, cameraP++) {
        if(cameraP->fov == -1) {
            continue;
        }
        GXInvalidateVtxCache();
        cameraBit = (1 << Hu3DCameraNo);
        Hu3DCameraBit = cameraBit;
        if(Hu3DShadowCamBit == HU3D_CAM0) {
            Hu3DShadow = &Hu3DShadowBuf[0];
        } else if(Hu3DShadowCamBit & Hu3DCameraBit) {
            Hu3DShadow = &Hu3DShadowBuf[Hu3DCameraNo];
        }
        for(projP=&Hu3DProjection[0], i=0; i<HU3D_PROJ_MAX; i++, projP++) {
            if(projP->anim) {
                MTXLookAt(projP->lookAtMtx, &projP->camPos, &projP->camUp, &projP->camTarget);
            }
        }
        if(Hu3DCameraNo == 0) {
            HuSprDispInit();
            HuSprExec(HUSPR_DRAWNO_BACK);
        }
        if(FogData.fogType != GX_FOG_NONE) {
            GXSetFog(FogData.fogType, FogData.fogStart, FogData.fogEnd, cameraP->near, cameraP->far, FogData.fogColor);
        }
        for(layer=0; layer<HU3D_LAYER_MAX; layer++) {
            if(layerHook[layer]) {
                Hu3DCameraSet(Hu3DCameraNo, Hu3DCameraMtx);
                MTXInvXpose(Hu3DCameraMtx, Hu3DCameraMtxXPose);
                hookFunc = layerHook[layer];
                hookFunc(layer);
            }
            if(layerNum[layer]) {
                Hu3DDrawPreInit();
                Hu3DCameraSet(Hu3DCameraNo, Hu3DCameraMtx);
                MTXInvXpose(Hu3DCameraMtx, Hu3DCameraMtxXPose);
                for(modelP=&Hu3DData[0], layerMdlNum=i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
                    if(!modelP->hsf) {
                        continue;
                    }
                    if(modelP->attr & HU3D_ATTR_CAMERA) {
                        Hu3DCameraMotionExec(i);
                        continue;
                    }
                    if((modelP->attr & HU3D_ATTR_CAMERA_UPDATE) == HU3D_ATTR_CAMERA_UPDATE && modelP->motId != HU3D_MOTID_NONE) {
                        Hu3DMotionExec(i, modelP->motId, modelP->motWork.time, FALSE);
                    }
                    if(modelP->attr & HU3D_ATTR_NOUPDATE_ALL) {
                        continue;
                    }
                    if((modelP->cameraBit & cameraBit) == 0) {
                        continue;
                    }
                    if(modelP->layerNo != layer) {
                        continue;
                    }
                    if(((modelP->attr & HU3D_ATTR_MOT_EXEC) == 0 && (modelP->attr & HU3D_ATTR_MOT_SLOW) == 0) ||
                        ((modelP->attr & HU3D_ATTR_MOT_SLOW) != 0 && (modelP->tick & 1) != 0)) {
                        vtxInvalidateF = FALSE;
                        modelP->motAttr &= ~HU3D_MOTATTR;
                        if(modelP->motId != HU3D_MOTID_NONE) {
                            Hu3DMotionExec(i, modelP->motId, modelP->motWork.time, FALSE);
                        }
                        if(modelP->motIdShift != HU3D_MOTID_NONE) {
                            Hu3DSubMotionExec(i);
                        }
                        if(modelP->motIdOvl != HU3D_MOTID_NONE) {
                            Hu3DMotionExec(i, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
                        }
                        if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                            ClusterMotionExec(modelP);
                            vtxInvalidateF = TRUE;
                        }
                        if(modelP->motIdShape != HU3D_MOTID_NONE) {
                            if(modelP->motId == HU3D_MOTID_NONE) {
                                Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time, FALSE);
                            } else {
                                Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time, TRUE);
                            }
                            vtxInvalidateF = TRUE;
                        }
                        if ((modelP->attr & (HU3D_ATTR_ENVELOPE_OFF|HU3D_ATTR_HOOKFUNC)) == 0 || (modelP->attr & HU3D_ATTR_MOTION_MODEL)) {
                            vtxInvalidateF = TRUE;
                            InitVtxParm(modelP->hsf);
                            if(modelP->motIdShape != HU3D_MOTID_NONE) {
                                ShapeProc(modelP->hsf);
                            }
                            if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                                ClusterProc(modelP);
                            }
                            if(modelP->hsf->cenvNum) {
                                EnvelopeProc(modelP->hsf);
                            }
                            PPCSync();
                        }
                        if(vtxInvalidateF) {
                            GXInvalidateVtxCache();
                        }
                        modelP->attr |= HU3D_ATTR_MOT_EXEC;
                    }
                    if(syncF && (modelP->attr & HU3D_ATTR_HOOKFUNC)) {
                        GXWaitDrawDone();
                        syncF = FALSE;
                    }
                    if ((modelP->attr & HU3D_ATTR_HOOK) == 0 &&
                        (0.0f != modelP->scale.x || 0.0f != modelP->scale.y || 0.0f != modelP->scale.z)) {
                        Mtx temp;
                        Mtx final;
                        mtxRot(temp, modelP->rot.x, modelP->rot.y, modelP->rot.z);
                        mtxScaleCat(temp, modelP->scale.x, modelP->scale.y, modelP->scale.z);
                        mtxTransCat(temp, modelP->pos.x, modelP->pos.y, modelP->pos.z);
                        PSMTXConcat(Hu3DCameraMtx, temp, final);
                        PSMTXConcat(final, modelP->mtx, final);
                        Hu3DDraw(modelP, final, &modelP->scale);
                    }
                    modelP->tick++;
                    layerMdlNum++;
                    if(layerMdlNum >= layerNum[layer]) {
                        break;
                    }
                }
                Hu3DDrawPost();
            }
            if(layerHook[layer+HU3D_LAYER_HOOK_POST]) {
                Hu3DCameraSet(Hu3DCameraNo, Hu3DCameraMtx);
                MTXInvXpose(Hu3DCameraMtx, Hu3DCameraMtxXPose);
                hookFunc = layerHook[layer+HU3D_LAYER_HOOK_POST];
                hookFunc(layer+HU3D_LAYER_HOOK_POST);
            }
        }
        if(!NoSyncF) {
            syncF = TRUE;
            GXSetDrawDone();
        }
    }
    HuSprDispInit();
    HuSprExec(HUSPR_DRAWNO_FRONT);
    for(modelP=&Hu3DData[0], i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        if(!modelP->hsf) {
            continue;
        }
        if ((modelP->motId != HU3D_MOTID_NONE || (modelP->attr & HU3D_ATTR_CLUSTER_ON) != 0 || modelP->motIdShape != HU3D_MOTID_NONE) &&
            (Hu3DPauseF == 0 || (modelP->attr & HU3D_ATTR_NOPAUSE) != 0)) {
            Hu3DMotionNext(i);
        }
    }
    HuSprFinish();
    Hu3DAnimExec();
    HuPerfEnd(HUPERF_USR1);
    (void)hookFunc;
}

void Hu3DAllKill(void)
{
    s16 i;
    Hu3DModelAllKill();
    Hu3DMotionAllKill();
    Hu3DCameraAllKill();
    Hu3DLightAllKill();
    Hu3DAnimAllKill();
    if(reflectAnim[0] != (void *)refMapData0) {
        HuMemDirectFree(reflectAnim[0]);
    }
    reflectAnim[0] = HuSprAnimRead(refMapData0);
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if(Hu3DShadowBuf[i].buf) {
            HuMemDirectFree(Hu3DShadowBuf[i].buf);
            Hu3DShadowBuf[i].buf = NULL;
        }
    }
    shadowNum = 0;
    Hu3DShadowF = 0;
    Hu3DFogClear();
    for(i=0; i<HU3D_LAYER_MAX; i++) {
        layerNum[i] = 0;
    }
    for(i=0; i<HU3D_LAYER_HOOK_MAX; i++) {
        layerHook[i] = NULL;
    }
    for(i=0; i<HU3D_PROJ_MAX; i++) {
        if(Hu3DProjection[i].anim) {
            Hu3DProjectionKill(i);
        }
        Hu3DProjection[i].anim = NULL;
    }
    Hu3DAmbColorSet(1, 1, 1);
    Hu3DBGColorSet(0, 0, 0);
    NoSyncF = FALSE;
}

void Hu3DBGColorSet(u8 r, u8 g, u8 b)
{
    BGColor.r = r;
    BGColor.g = g;
    BGColor.b = b;
}

void Hu3DLayerHookSet(s16 layerNo, HU3DLAYERHOOK hook)
{
    layerHook[layerNo] = hook;
}

void Hu3DLayerHookReset(s16 layerNo)
{
    layerHook[layerNo] = NULL;
}

void Hu3DPauseSet(BOOL pauseF)
{
    Hu3DPauseF = pauseF;
}

void Hu3DNoSyncSet(BOOL noSync)
{
    NoSyncF = noSync;
}

HU3DMODELID Hu3DModelCreate(void *data)
{
    HU3DMODEL *modelP;
    s16 modelId;
    s16 i;
    for(modelP=&Hu3DData[0], modelId=0; modelId<HU3D_MODEL_MAX; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == HU3D_MODEL_MAX) {
        OSReport("Error: Create Model Over!\n");
        return HU3D_MODELID_NONE;
    }
    modelP->hsf = LoadHSF(data);
    modelP->mallocNo = Hu3DMallocNo = (u32)modelP->hsf;
    modelP->attr = HU3D_ATTR_NONE;
    modelP->motAttr = HU3D_MOTATTR_NONE;
    modelP->projBit = 0;
    MakeDisplayList(modelId, modelP->mallocNo);
    modelP->motWork.speed = 1.0f;
    for(i=0; i<HU3D_CLUSTER_MAX; i++) {
        modelP->motIdCluster[i] = HU3D_MOTID_NONE;
    }
    modelP->motIdOvl = HU3D_MOTID_NONE;
    modelP->motIdShift = HU3D_MOTID_NONE;
    modelP->motIdShape = HU3D_MOTID_NONE;
    modelP->motWork.time = 0.0f;
    modelP->timingHook = NULL;
    modelP->matHook = NULL;
    if(modelP->hsf->motionNum) {
        modelP->attr |= HU3D_ATTR_MOTION_MODEL;
        modelP->motId = modelP->motIdSrc = Hu3DMotionModelCreate(modelId);
        if(modelP->hsf->cenvNum) {
            Hu3DMotionExec(modelId, modelP->motId, 0, FALSE);
            EnvelopeProc(modelP->hsf);
            PPCSync();
        }
        if(modelP->hsf->clusterNum) {
            Hu3DMotionClusterSet(modelId, modelP->motId);
        }
        if(modelP->hsf->shapeNum) {
            Hu3DMotionShapeSet(modelId, modelP->motId);
        }
        modelP->motWork.start = 0;
        modelP->motWork.end = Hu3DMotionMaxTimeGet(modelId);
    } else {
        modelP->motIdSrc = modelP->motId = HU3D_MOTID_NONE;
    }
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->cameraBit = HU3D_CAM_ALL;
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->ambR = modelP->ambG = modelP->ambB = 1;
    modelP->reflectType = HU3D_REFLECT_TYPE_NONE;
    modelP->linkMdlId = HU3D_MODELID_NONE;
    for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
        modelP->lLightId[i] = HU3D_LIGHTID_NONE;
    }
    modelP->camInfoBit = 0;
    modelP->tick = modelId;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    if(modelP->hsf->sceneNum && (modelP->hsf->scene->fogStart || modelP->hsf->scene->fogEnd)) {
        Hu3DFogSet(modelP->hsf->scene->fogStart, modelP->hsf->scene->fogEnd, modelP->hsf->scene->fogColor.r, modelP->hsf->scene->fogColor.g, modelP->hsf->scene->fogColor.b);
    }
    return modelId;
}

HU3DMODELID Hu3DModelLink(HU3DMODELID linkMdlId)
{
    HU3DMODEL *linkModelP = &Hu3DData[linkMdlId];
    HU3DMODEL *modelP;
    HSFOBJECT *objtop;
    s16 i;
    s16 modelId;
    for(modelP=&Hu3DData[0], modelId=0; modelId<HU3D_MODEL_MAX; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == HU3D_MODEL_MAX) {
        return HU3D_MODELID_NONE;
    }
    modelP->hsfLink = linkModelP->hsf;
    modelP->hsf = HuMemDirectMalloc(HEAP_MODEL, sizeof(HSFDATA));
    modelP->mallocNoLink = (u32)modelP->hsf;
    *modelP->hsf = *linkModelP->hsf;
    objtop = Hu3DObjDuplicate(modelP->hsf, modelP->mallocNoLink);
    modelP->hsf->root = (HSFOBJECT *)((char *)objtop+((u32)modelP->hsf->root-(u32)modelP->hsf->object));
    modelP->hsf->object = objtop;
    Hu3DAttrDuplicate(modelP->hsf, modelP->mallocNoLink);
    Hu3DMatDuplicate(modelP->hsf, modelP->mallocNoLink);
    modelP->mallocNo = linkModelP->mallocNo;
    modelP->attr = linkModelP->attr;
    if(linkModelP->attr & HU3D_ATTR_SHADOW) {
        shadowNum++;
    }
    linkModelP->attr |= HU3D_ATTR_LINK;
    modelP->motAttr = linkModelP->motAttr;
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->motId = linkModelP->motId;
    if(modelP->motId != HU3D_MOTID_NONE) {
        modelP->motWork.start = 0;
        modelP->motWork.end = Hu3DMotionMaxTimeGet(modelId);
    }
    modelP->motIdShift = modelP->motIdOvl = HU3D_MOTID_NONE;
    modelP->motIdShape = linkModelP->motIdShape;
    modelP->motShapeWork.time = 0;
    modelP->motShapeWork.speed = linkModelP->motShapeWork.speed;
    modelP->motShapeWork.start = linkModelP->motShapeWork.start;
    modelP->motShapeWork.end = linkModelP->motShapeWork.end;
    for(i=0; i<HU3D_CLUSTER_MAX; i++) {
        modelP->motIdCluster[i] = linkModelP->motIdCluster[i];
        if(modelP->motIdCluster[i] != HU3D_MOTID_NONE) {
            modelP->clusterTime[i] = 0;
            modelP->clusterSpeed[i] = linkModelP->clusterSpeed[i];
            modelP->clusterAttr[i] = linkModelP->clusterAttr[i];
            modelP->attr |= HU3D_ATTR_CLUSTER_ON;
            ClusterAdjustObject(modelP->hsf, Hu3DMotion[modelP->motIdCluster[i]].hsf);
        }
    }
    modelP->motWork.time = linkModelP->motWork.time;
    modelP->motWork.speed = linkModelP->motWork.speed;
    modelP->motIdSrc = HU3D_MOTID_NONE;
    modelP->cameraBit = HU3D_CAM_ALL;
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->projBit = 0;
    modelP->ambR = modelP->ambG = modelP->ambB = 1;
    modelP->reflectType = HU3D_REFLECT_TYPE_NONE;
    modelP->linkMdlId = linkMdlId;
    for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
        modelP->lLightId[i] = HU3D_LIGHTID_NONE;
    }
    modelP->camInfoBit = 0;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    return modelId;
}

HU3DMODELID Hu3DHookFuncCreate(HU3DMODELHOOK hookFunc)
{
    HU3DMODEL *modelP;
    s16 i;
    s16 modelId;
    for(modelP=&Hu3DData[0], modelId=0; modelId<HU3D_MODEL_MAX; modelId++, modelP++) {
        if(!modelP->hsf) {
            break;
        }
    }
    if(modelId == HU3D_MODEL_MAX) {
        return HU3D_MODELID_NONE;
    }
    modelP->hookFunc = hookFunc;
    modelP->mallocNo = modelId+HOOKFUNC_MALLOCNO;
    modelP->attr = HU3D_ATTR_HOOKFUNC;
    modelP->motAttr = HU3D_MOTATTR_NONE;
    modelP->pos.x = modelP->pos.y = modelP->pos.z = 0;
    modelP->rot.x = modelP->rot.y = modelP->rot.z = 0;
    modelP->scale.x = modelP->scale.y = modelP->scale.z = 1;
    modelP->motId = modelP->motIdShift = modelP->motIdOvl = modelP->motIdShape = HU3D_MOTID_NONE;
    for(i=0; i<HU3D_CLUSTER_MAX; i++) {
        modelP->motIdCluster[i] = HU3D_MOTID_NONE;
    }
    modelP->motWork.time = 0;
    modelP->motWork.speed = 1;
    modelP->motIdSrc = HU3D_MOTID_NONE;
    modelP->timingHook = NULL;
    modelP->matHook = NULL;
    modelP->cameraBit = HU3D_CAM_ALL;
    modelP->layerNo = 0;
    modelP->hookData = NULL;
    modelP->lightNum = 0;
    modelP->hiliteIdx = 0;
    modelP->linkMdlId = HU3D_MODELID_NONE;
    modelP->projBit = 0;
    modelP->reflectType = HU3D_REFLECT_TYPE_NONE;
    for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
        modelP->lLightId[i] = HU3D_LIGHTID_NONE;
    }
    modelP->camInfoBit = 0;
    MTXIdentity(modelP->mtx);
    layerNum[0]++;
    return modelId;
}

void Hu3DModelKill(HU3DMODELID modelId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HU3DMODEL *iterModelP;
    s16 i;
    s16 linkNum;
    if(!hsf) {
        return;
    }
    layerNum[modelP->layerNo]--;
    if(modelP->attr & HU3D_ATTR_HOOKFUNC) {
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
        if(modelP->attr & HU3D_ATTR_PARTICLE) {
            HU3DPARTICLE *particleP = modelP->hookData;
            HuSprAnimKill(particleP->anim);
        }
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hookFunc = NULL;
        return;
    }
    if(modelP->attr & HU3D_ATTR_CAMERA) {
        if(modelP->motId != HU3D_MOTID_NONE) {
            Hu3DMotionKill(modelP->motId);
        }
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hsf = NULL;
        return;
    }
    Hu3DAnimModelKill(modelId);
    if(modelP->linkMdlId != HU3D_MODELID_NONE) {
        HuMemDirectFree(modelP->hsf);
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNoLink);
        hsf = modelP->hsfLink;
        modelP->hsf = hsf;
    }
    for(iterModelP=&Hu3DData[0], linkNum=i=0; i<HU3D_MODEL_MAX; i++, iterModelP++) {
        if(!iterModelP->hsf) {
            continue;
        }
        if(iterModelP->hsf == hsf || (iterModelP->linkMdlId != HU3D_MODELID_NONE && iterModelP->hsfLink == hsf)) {
            linkNum++;
        }
    }
    if(linkNum > 1) {
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hsf = NULL;
        iterModelP=&Hu3DData[0];
        if(modelP->motIdSrc != HU3D_MOTID_NONE) {
            for(i=0; i<HU3D_MODEL_MAX; i++, iterModelP++) {
                if(iterModelP->hsf && iterModelP->linkMdlId != HU3D_MODELID_NONE && iterModelP->hsfLink == hsf) {
                    Hu3DMotion[modelP->motIdSrc].modelId = i;
                    
                    break;
                }
            }
        }
        return;
    }
    if(modelP->motIdSrc != HU3D_MOTID_NONE && Hu3DMotionKill(modelP->motIdSrc) == FALSE) {
        Hu3DMotion[modelP->motIdSrc].modelId = HU3D_MOTID_NONE;
        HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
        if(modelP->attr & HU3D_ATTR_SHADOW) {
            shadowNum--;
        }
        modelP->hsf = NULL;
        return;
    }
    HuMemDirectFree(modelP->hsf);
    HuMemDirectFreeNum(HEAP_MODEL, modelP->mallocNo);
    for(i=0; i<modelP->lightNum; i++) {
        Hu3DGLightKill(modelP->lightId[i]);
    }
    for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
        if(modelP->lLightId[i] != HU3D_LIGHTID_NONE) {
            Hu3DLLightKill(modelId, i);
        }
    }
    if(modelP->attr & HU3D_ATTR_SHADOW) {
        shadowNum--;
    }
    modelP->hsf = NULL;
    return;
}

void Hu3DModelAllKill(void)
{
    HU3DMODEL *modelP;
    s16 i;
    modelKillAllF = TRUE;
    for(modelP=&Hu3DData[0], i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        if(modelP->hsf) {
            Hu3DModelKill(i);
        }
    }
    modelKillAllF = FALSE;
    for(i=0; i<HU3D_LAYER_MAX; i++) {
        layerNum[i] = 0;
    }
    for(i=0; i<HU3D_LAYER_HOOK_MAX; i++) {
        layerHook[i] = NULL;
    }
    Hu3DParManAllKill();
    HuMemDCFlush(HEAP_MODEL);
    Hu3DReflectModelId = HU3D_MODELID_NONE;
}

void Hu3DModelPosSet(HU3DMODELID modelId, float posX, float posY, float posZ)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->pos.x = posX;
    modelP->pos.y = posY;
    modelP->pos.z = posZ;
}

void Hu3DModelPosSetV(HU3DMODELID modelId, HuVecF *pos)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->pos = *pos;
}

void Hu3DModelRotSet(HU3DMODELID modelId, float rotX, float rotY, float rotZ)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->rot.x = rotX;
    modelP->rot.y = rotY;
    modelP->rot.z = rotZ;
}

void Hu3DModelRotSetV(HU3DMODELID modelId, HuVecF *rot)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->rot = *rot;
}

void Hu3DModelScaleSet(HU3DMODELID modelId, float scaleX, float scaleY, float scaleZ)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->scale.x = scaleX;
    modelP->scale.y = scaleY;
    modelP->scale.z = scaleZ;
}

void Hu3DModelScaleSetV(HU3DMODELID modelId, HuVecF *scale)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->scale = *scale;
}

void Hu3DModelAttrSet(HU3DMODELID modelId, u32 attr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    if(attr & HU3D_MOTATTR) {
        modelP->motAttr |= attr & ~HU3D_MOTATTR;
        modelP->attr |= HU3D_ATTR_MOTION_MODEL;
    } else {
        modelP->attr |= attr;
    }
}

void Hu3DModelAttrReset(HU3DMODELID modelId, u32 attr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    if(attr & HU3D_MOTATTR) {
        modelP->motAttr &= ~attr;
    } else {
        modelP->attr &= ~attr;
    }
}

u32 Hu3DModelAttrGet(HU3DMODELID modelId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    return modelP->attr;
}

u32 Hu3DModelMotionAttrGet(HU3DMODELID modelId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    return modelP->motAttr;
}

void Hu3DModelClusterAttrSet(HU3DMODELID modelId, s16 clusterNo, s32 clusterAttr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->clusterAttr[clusterNo] |= clusterAttr;
}

void Hu3DModelClusterAttrReset(HU3DMODELID modelId, s16 clusterNo, s32 clusterAttr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->clusterAttr[clusterNo] &= ~clusterAttr;
}

void Hu3DModelCameraSet(HU3DMODELID modelId, u16 cameraBit)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->cameraBit = cameraBit;
}

void Hu3DModelLayerSet(HU3DMODELID modelId, s16 layerNo)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    layerNum[modelP->layerNo]--;
    modelP->layerNo = layerNo;
    layerNum[layerNo]++;
}

HSFOBJECT *Hu3DModelObjPtrGet(HU3DMODELID modelId, char *objName)
{
    char name[OBJNAME_MAX_LEN];
    HSFDATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSFOBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName(objName));
    for(i=0; i<hsf->objectNum; objPtr++, i++) {
        HSFOBJECT *obj = objPtr;
        if(strcmp(name, obj->name) == 0) {
            return objPtr;
        }
    }
    if(i == hsf->objectNum) {
        OSReport("Error: OBJPtr Error! %s\n", objName);
    }
    return NULL;
}

void Hu3DModelTPLvlSet(HU3DMODELID modelId, float tpLvl)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFMATERIAL *matPtr;
    HSFOBJECT *objPtr;
    s16 i;
    for(matPtr = hsf->material, i=0; i<hsf->materialNum; i++, matPtr++) {
        matPtr->invAlpha = 1.0f-tpLvl;
        if(tpLvl != 1.0f) {
            HSF_MATERIAL_SETPASS(matPtr, 1);
        } else {
            HSF_MATERIAL_SETPASS(matPtr, 0);
        }
    }
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; objPtr++, i++) {
        HSFOBJECT *obj = objPtr;
        if(obj->type == HSF_OBJ_MESH) {
            HSFCONSTDATA *constData = obj->constData;
            constData->attr |= HU3D_CONST_XLU;
        }
    }
    modelP->attr |= HU3D_ATTR_TPLVL_SET;
}

void Hu3DModelHiliteMapSet(HU3DMODELID modelId, ANIMDATA *hiliteMap)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    s16 i;
    HSFOBJECT *objPtr;
    
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; objPtr++, i++) {
        HSFOBJECT *obj = objPtr;
        if(obj->type == HSF_OBJ_MESH) {
            HSFCONSTDATA *constData;
            obj->flags |= HSF_MATERIAL_HILITE;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_HILITE;
            constData->hiliteMap = hiliteMap;
        }
    }
}

void Hu3DModelShadowSet(HU3DMODELID modelId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    s16 i;
    HSFOBJECT *objPtr;
    if(!(modelP->attr & HU3D_ATTR_SHADOW)) {
        shadowNum++;
    }
    modelP->attr |= HU3D_ATTR_SHADOW;
    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData) {
            HSFCONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW;
        }
    }
}

void Hu3DModelShadowReset(HU3DMODELID modelId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    s16 i;
    HSFOBJECT *objPtr;
    if(modelP->attr & HU3D_ATTR_SHADOW) {
        shadowNum--;
    }
    modelP->attr &= ~HU3D_ATTR_SHADOW;
    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData) {
            HSFCONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~HU3D_CONST_SHADOW;
        }
    }
}

void Hu3DModelShadowDispOn(HU3DMODELID modelId)
{
    Hu3DModelAttrSet(modelId, HU3D_ATTR_SHADOW);
}

void Hu3DModelShadowDispOff(HU3DMODELID modelId)
{
    Hu3DModelAttrReset(modelId, HU3D_ATTR_SHADOW);
}

void Hu3DModelShadowMapSet(HU3DMODELID modelId)
{
    HSFDATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSFOBJECT *objPtr;

    for(objPtr = hsf->object, i=0; i<hsf->objectNum;  i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData) {
            HSFCONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW_MAP;
        }
    }
}

void Hu3DModelShadowMapObjSet(HU3DMODELID modelId, char *objName)
{
    char name[OBJNAME_MAX_LEN];
    HSFDATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSFOBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName(objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSFCONSTDATA *constData;
            constData = obj->constData;
            constData->attr |= HU3D_CONST_SHADOW_MAP;
            break;
        }
    }
}

void Hu3DModelShadowMapObjReset(HU3DMODELID modelId)
{
    HSFDATA *hsf = Hu3DData[modelId].hsf;
    s32 i;
    HSFOBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData) {
            HSFCONSTDATA *constData;
            constData = obj->constData;
            constData->attr &= ~HU3D_CONST_SHADOW_MAP;
        }
    }
}

void Hu3DModelAmbSet(HU3DMODELID modelId, float ambR, float ambG, float ambB)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->ambR = ambR;
    modelP->ambG = ambG;
    modelP->ambB = ambB;
}

void Hu3DModelHookSet(HU3DMODELID modelId, char *objName, HU3DMODELID hookMdlId)
{
    char name[OBJNAME_MAX_LEN];
    HSFDATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSFOBJECT *objPtr;
    
    Hu3DMotionCalc(modelId);
    objPtr = hsf->object;
    strcpy(name, MakeObjectName(objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSFCONSTDATA *constData = obj->constData;
            constData->hookMdlId = hookMdlId;
            Hu3DModelAttrSet(hookMdlId, HU3D_ATTR_HOOK);
            return;
        }
    }
    OSReport( "Error: Not Found %s for HookSet\n", objName);
}

void Hu3DModelHookReset(HU3DMODELID modelId)
{
    HSFDATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSFOBJECT *objPtr = hsf->object;
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData) {
            HSFCONSTDATA *constData = obj->constData;
            if(constData->hookMdlId != HU3D_MODELID_NONE) {
                Hu3DModelAttrReset(constData->hookMdlId, HU3D_ATTR_HOOK);
                constData->hookMdlId = HU3D_MODELID_NONE;
            }
        }
    }
}

void Hu3DModelHookObjReset(HU3DMODELID modelId, char *objName)
{
    char name[OBJNAME_MAX_LEN];
    HSFDATA *hsf = Hu3DData[modelId].hsf;
    s16 i;
    HSFOBJECT *objPtr = hsf->object;
    strcpy(name, MakeObjectName(objName));
    for(i=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->constData && !strcmp(name, obj->name)) {
            HSFCONSTDATA *constData = obj->constData;
            Hu3DModelAttrReset(constData->hookMdlId, HU3D_ATTR_HOOK);
            constData->hookMdlId = HU3D_MODELID_NONE;
            return;
        }
    }
    OSReport("Error: Not Found %s for HookReset\n", objName);
}

void Hu3DModelProjectionSet(HU3DMODELID modelId, HU3DPROJID projId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->projBit |= (1 << projId);
}

void Hu3DModelProjectionReset(HU3DMODELID modelId, HU3DPROJID projId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->projBit &= ~(1 << projId);
}

void Hu3DModelHiliteTypeSet(HU3DMODELID modelId, s16 hiliteType)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFMATERIAL *matPtr = hsf->material;
    s16 i;
    hiliteType = hiliteType << 4;
    hiliteType &= 0xF0;
    for(i=0; i<hsf->materialNum; i++, matPtr++) {
        HSF_MATERIAL_SETHILITETYPE(matPtr, hiliteType);
        matPtr->flags |= HSF_MATERIAL_HILITE;
    }
    Hu3DModelAttrSet(modelId, HU3D_ATTR_HILITE);
}

void Hu3DModelReflectTypeSet(HU3DMODELID modelId, s16 reflectType)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->reflectType = reflectType;
}

void Hu3DReflectModelSet(HU3DMODELID modelId)
{
    HU3DMODEL *modelP;
    Hu3DReflectModelId = modelId;
    modelP = &Hu3DData[modelId];
    modelP->attr |= HU3D_ATTR_REFLECT_MODEL;
    Hu3DReflectModelAnim = HuSprAnimMake(REFLECT_TEX_W, REFLECT_TEX_H, ANIM_BMP_RGB5A3);
    Hu3DReflectModelAnim->bmp->data = HuMemDirectMalloc(HEAP_MODEL, REFLECT_TEX_W*REFLECT_TEX_H*2);
    Hu3DReflectMapSet(Hu3DReflectModelAnim);
}

void Hu3DModelMatHookSet(HU3DMODELID modelId, HU3DMATHOOK matHook)
{
    Hu3DData[modelId].matHook = matHook;
}

HU3DCAMERA defCamera = {
    45.0f,
    20.0f,
    5000.0f,
    HU_DISP_ASPECT,
    0.0f,
    {0.0f, 0.0f, 100.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    0, 0, HU_FB_WIDTH, HU_FB_HEIGHT,
    0.0f, 0.0f, HU_FB_WIDTH, HU_FB_HEIGHT,
    0.0f, 1.0f
};

void Hu3DCameraCreate(int cameraBit)
{
    s16 i;
    s16 bit;
    defCamera.viewportW = RenderMode->fbWidth;
    defCamera.viewportH = RenderMode->efbHeight;
    defCamera.scissorW = RenderMode->fbWidth;
    defCamera.scissorH = RenderMode->efbHeight;
    Hu3DCameraExistF |= cameraBit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            *cameraP = defCamera;
        }
    }
}

void Hu3DCameraPerspectiveSet(int cameraBit, float fov, float near, float far, float aspect)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            cameraP->fov = fov;
            cameraP->near = near;
            cameraP->far = far;
            cameraP->aspect = aspect;
        }
    }
}

void Hu3DCameraViewportSet(int cameraBit, float vpX, float vpY, float vpW, float vpH, float vpNearZ, float vpFarZ)
{
    s16 i;
    s16 bit;
    if(RenderMode->xfbHeight == 240) {
        vpH *= 0.5f;
    }
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            cameraP->viewportX = vpX;
            cameraP->viewportY = vpY;
            cameraP->viewportW = vpW;
            cameraP->viewportH = vpH;
            cameraP->viewportNear = vpNearZ;
            cameraP->viewportFar = vpFarZ;
        }
    }
}

void Hu3DCameraScissorSet(int cameraBit, unsigned int scissorX, unsigned int scissorY, unsigned int scissorW, unsigned int scissorH)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            cameraP->scissorX = scissorX;
            cameraP->scissorY = scissorY;
            cameraP->scissorW = scissorW;
            cameraP->scissorH = scissorH;
        }
    }
}

void Hu3DCameraPosSet(int cameraBit, float posX, float posY, float posZ, float upX, float upY, float upZ, float targetX, float targetY, float targetZ)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            cameraP->pos.x = posX;
            cameraP->pos.y = posY;
            cameraP->pos.z = posZ;
            cameraP->up.x = upX;
            cameraP->up.y = upY;
            cameraP->up.z = upZ;
            cameraP->target.x = targetX;
            cameraP->target.y = targetY;
            cameraP->target.z = targetZ;
        }
    }
}

void Hu3DCameraPosSetV(int cameraBit, Vec *pos, Vec *up, Vec *target)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            cameraP->pos = *pos;
            cameraP->up = *up;
            cameraP->target = *target;
        }
    }
}

void Hu3DCameraPosGet(int cameraBit, Vec *pos, Vec *up, Vec *target)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            *pos = cameraP->pos;
            *up = cameraP->up;
            *target = cameraP->target;
            break;
        }
    }
}

void Hu3DCameraPerspectiveGet(int cameraBit, float *fov, float *near, float *far)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            *fov = cameraP->fov;
            *near = cameraP->near;
            *far = cameraP->far;
            break;
        }
    }
}

void Hu3DCameraKill(int cameraBit)
{
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            HU3DCAMERA *cameraP = &Hu3DCamera[i];
            cameraP->fov = -1;
        }
    }
}

void Hu3DCameraAllKill(void)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    s16 i;
    s16 bit;
    for(i=0, bit=1; i<HU3D_CAM_MAX; i++, bit <<= 1, cameraP++) {
        if(-1 != cameraP->fov) {
            Hu3DCameraKill(bit);
        }
    }
    Hu3DCameraExistF = 0;
}

void Hu3DCameraSet(s32 cameraNo, Mtx modelView)
{
    Mtx44 proj;
    HU3DCAMERA *cameraP = &Hu3DCamera[cameraNo];
    MTXPerspective(proj, cameraP->fov, cameraP->aspect, cameraP->near, cameraP->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    if(RenderMode->field_rendering) {
        GXSetViewportJitter(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH, cameraP->viewportNear, cameraP->viewportFar, VIGetNextField());
    } else {
        GXSetViewport(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH, cameraP->viewportNear, cameraP->viewportFar);
    }
    GXSetScissor(cameraP->scissorX, cameraP->scissorY, cameraP->scissorW, cameraP->scissorH);
    MTXLookAt(modelView, &cameraP->pos, &cameraP->up, &cameraP->target);
}

BOOL Hu3DModelCameraInfoSet(HU3DMODELID modelId, u16 cameraBit)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HU3DCAMERA *cameraP = &Hu3DCamera[cameraBit];
    HSFOBJECT *objPtr;
    s16 i;
    for(objPtr = hsf->object, i=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        float upRot;
        HuVecF upOfs;
        HuVecF up;
        
        if(obj->type != HSF_OBJ_CAMERA) {
            continue;
        }
        upRot = obj->camera.upRot;
        cameraP->upRot = upRot;
        HuSubVecF(&up, &obj->camera.pos, &obj->camera.target);
        upOfs.x = ((up.x * up.y * (1-HuCos(upRot))) - (up.z * HuSin(upRot)));
        upOfs.y = ((up.y * up.y) + (1-HuSquare(up.y)) * HuCos(upRot));
        upOfs.z = (((up.y * up.z) * (1-HuCos(upRot))) + (up.x * HuSin(upRot)));
        HuNormVecF(&upOfs, &up);
        Hu3DCameraPosSet(cameraBit, obj->camera.pos.x, obj->camera.pos.y, obj->camera.pos.z,
            up.x, up.y, up.z,
            obj->camera.target.x, obj->camera.target.y, obj->camera.target.z);
        Hu3DCameraPerspectiveSet(cameraBit, obj->camera.fov, obj->camera.near, obj->camera.far, HU_DISP_ASPECT);
        modelP->camInfoBit = cameraBit;
        Hu3DModelAttrSet(modelId, HU3D_ATTR_CAMERA_MOTON);
        return TRUE;
    }
    return FALSE;
}

s16 Hu3DModelCameraCreate(HU3DMOTID motId, u16 cameraBit)
{
    HU3DMODELID modelId = Hu3DHookFuncCreate((HU3DMODELHOOK)-1);
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->attr &= ~HU3D_ATTR_HOOKFUNC;
    modelP->attr |= HU3D_ATTR_CAMERA | HU3D_ATTR_CAMERA_MOTON;
    modelP->motId = motId;
    modelP->camInfoBit = cameraBit;
    
    return modelId;
}

void Hu3DCameraMotionOn(HU3DMODELID modelId, u16 cameraBit)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    modelP->camInfoBit = cameraBit;
    Hu3DModelAttrSet(modelId, HU3D_ATTR_CAMERA_MOTON);
}

void Hu3DCameraMotionStart(HU3DMODELID modelId, u16 cameraBit)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    Hu3DCameraMotionOn(modelId, cameraBit);
    Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DMotionStartEndSet(modelId, 0,  Hu3DMotionMotionMaxTimeGet(modelP->motId));
    Hu3DMotionTimeSet(modelId, 0);
}

void Hu3DCameraMotionOff(HU3DMODELID modelId)
{
    Hu3DModelAttrReset(modelId, HU3D_ATTR_CAMERA_MOTON);
}

void Hu3DLighInit(void)
{
    HU3DLIGHT *lightP;
    s16 i;
    lightP = &Hu3DGlobalLight[0];
    for(i=0; i<HU3D_GLIGHT_MAX; i++, lightP++) {
        lightP->type = HU3D_LIGHT_TYPE_NONE;
    }
    lightP = &Hu3DLocalLight[0];
    for(i=0; i<HU3D_LLIGHT_MAX; i++, lightP++) {
        lightP->type = HU3D_LIGHT_TYPE_NONE;
    }
}

static void Hu3DLightCreate(HU3DLIGHT *lightP, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    lightP->type = HU3D_LIGHT_TYPE_SPOT;
    lightP->pos = *pos;
    lightP->dir = *dir;
    lightP->offset.x = lightP->offset.y = lightP->offset.z = 0;
    lightP->cutoff = 30;
    lightP->func = GX_SP_COS;
    HuNormVecF(&lightP->dir, &lightP->dir);
    lightP->color = *color;
}

HU3DLIGHTID Hu3DGLightCreate(float posX, float posY, float posZ, float dirX, float dirY, float dirZ, u8 colorR, u8 colorG, u8 colorB)
{
    Vec pos;
    Vec dir;
    GXColor color;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    dir.x = dirX;
    dir.y = dirY;
    dir.z = dirZ;
    color.r = colorR;
    color.g = colorG;
    color.b = colorB;
    color.a = 128;
    return Hu3DGLightCreateV(&pos, &dir, &color);
}

HU3DLIGHTID Hu3DGLightCreateV(HuVecF *pos, HuVecF *dir, GXColor *color)
{
    HU3DLIGHTID lightId;
    HU3DLIGHT *lightP;
    for(lightP=&Hu3DGlobalLight[0], lightId=0; lightId<HU3D_GLIGHT_MAX; lightId++, lightP++) {
        if(lightP->type == HU3D_LIGHT_TYPE_NONE) {
            break;
        }
    }
    if(lightId == HU3D_GLIGHT_MAX) {
        return HU3D_LIGHTID_NONE;
    }
    Hu3DLightCreate(lightP, pos, dir, color);
    return lightId;
}

HU3DLLIGHTID Hu3DLLightCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float dirX, float dirY, float dirZ, u8 colorR, u8 colorG, u8 colorB)
{
    Vec pos;
    Vec dir;
    GXColor color;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    dir.x = dirX;
    dir.y = dirY;
    dir.z = dirZ;
    color.r = colorR;
    color.g = colorG;
    color.b = colorB;
    color.a = 255;
    return Hu3DLLightCreateV(modelId, &pos, &dir, &color);
}

HU3DLLIGHTID Hu3DLLightCreateV(HU3DMODELID modelId, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHTID lightId;
    HU3DLLIGHTID lLightId;
    HU3DLIGHT *lightP;
    for(lightP=&Hu3DLocalLight[0], lightId=0; lightId<HU3D_LLIGHT_MAX; lightId++, lightP++) {
        if(lightP->type == HU3D_LIGHT_TYPE_NONE) {
            break;
        }
    }
    if(lightId == HU3D_LLIGHT_MAX) {
        return HU3D_LIGHTID_NONE;
    }
    Hu3DLightCreate(lightP, pos, dir, color);
    for(lLightId=0; lLightId<HU3D_MODEL_LLIGHT_MAX; lLightId++) {
        if(modelP->lLightId[lLightId] == HU3D_LIGHTID_NONE) {
            break;
        }
    }
    if(lLightId == HU3D_MODEL_LLIGHT_MAX) {
        return HU3D_LIGHTID_NONE;
    }
    modelP->lLightId[lLightId] = lightId;
    modelP->attr |= HU3D_ATTR_LLIGHT;  
    return lLightId;
}

static void Hu3DLightSpotSet(HU3DLIGHT *lightP, GXSpotFn spotFunc, float cutoff)
{
    LIGHT_TYPE_SET(lightP, HU3D_LIGHT_TYPE_SPOT);
    lightP->cutoff = cutoff;
    lightP->func = spotFunc;
}

void Hu3DGLightSpotSet(HU3DLIGHTID lightId, GXSpotFn spotFunc, float cutoff)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightSpotSet(lightP, spotFunc, cutoff);
}

void Hu3DLLightSpotSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, GXSpotFn spotFunc, float cutoff)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightSpotSet(lightP, spotFunc, cutoff);
}

static void Hu3DLightInfinitytSet(HU3DLIGHT *lightP)
{
    LIGHT_TYPE_SET(lightP, HU3D_LIGHT_TYPE_INFINITYT);
}

void Hu3DGLightInfinitytSet(HU3DLIGHTID lightId)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightInfinitytSet(lightP);
}

void Hu3DLLightInfinitytSet(HU3DMODELID modelId, HU3DLLIGHTID lightId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightInfinitytSet(lightP);
}

static void Hu3DLightPointSet(HU3DLIGHT *lightP, float refDistance, float refBrightness, GXDistAttnFn distFunc)
{
    LIGHT_TYPE_SET(lightP, HU3D_LIGHT_TYPE_POINT);
    lightP->cutoff = refDistance;
    lightP->brightness = refBrightness;
    lightP->func = distFunc;
}

void Hu3DGLightPointSet(HU3DLIGHTID lightId, float refDistance, float refBrightness, GXDistAttnFn distFunc)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPointSet(lightP, refDistance, refBrightness, distFunc);
}

void Hu3DLLightPointSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float refDistance, float refBrightness, GXDistAttnFn distFunc)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightPointSet(lightP, refDistance, refBrightness, distFunc);
}

void Hu3DGLightKill(HU3DLIGHTID lightId)
{
    Hu3DGlobalLight[lightId].type = HU3D_LIGHT_TYPE_NONE;
}

void Hu3DLLightKill(HU3DMODELID modelId, HU3DLLIGHTID lightId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    s16 i;
    lightP->type = HU3D_LIGHT_TYPE_NONE;
    modelP->lLightId[lightId] = HU3D_LIGHTID_NONE;
    for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
        if(modelP->lLightId[i] == HU3D_LIGHTID_NONE) {
            break;
        }
    }
    if(i == HU3D_MODEL_LLIGHT_MAX) {
        modelP->attr &= ~HU3D_ATTR_LLIGHT;
    }
}

void Hu3DLightAllKill(void)
{
    HU3DLIGHTID lightId;
    HU3DLIGHT *lightP;
    for(lightP=&Hu3DGlobalLight[0], lightId=0; lightId<HU3D_GLIGHT_MAX; lightId++, lightP++) {
        if(lightP->type != HU3D_LIGHT_TYPE_NONE) {
            Hu3DGLightKill(lightId);
        }
    }
}

static void Hu3DLightColSet(HU3DLIGHT *lightP, u8 r, u8 g, u8 b, u8 a)
{
    lightP->color.r = r;
    lightP->color.g = g;
    lightP->color.b = b;
    lightP->color.a = a;
}

void Hu3DGLightColorSet(HU3DLIGHTID lightId, u8 r, u8 g, u8 b, u8 a)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightColSet(lightP, r, g, b, a);
}

void Hu3DLLightColorSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, u8 r, u8 g, u8 b, u8 a)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightColSet(lightP, r, g, b, a);
}

static void Hu3DLightPSetV(HU3DLIGHT *lightP, HuVecF *pos, HuVecF *dir)
{
    lightP->pos = *pos;
    HuNormVecF(dir, &lightP->dir);
}

void Hu3DGLightPosSetV(HU3DLIGHTID lightId, HuVecF *pos, HuVecF *dir)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPSetV(lightP, pos, dir);
}

void Hu3DLLightPosSetV(HU3DMODELID modelId, HU3DLLIGHTID lightId, HuVecF *pos, HuVecF *dir)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightPSetV(lightP, pos, dir);
}

static void Hu3DLightPSet(HU3DLIGHT *lightP, float posX, float posY, float posZ, float dirX, float dirY, float dirZ)
{
    lightP->pos.x = posX;
    lightP->pos.y = posY;
    lightP->pos.z = posZ;
    lightP->dir.x = dirX;
    lightP->dir.y = dirY;
    lightP->dir.z = dirZ;
    HuNormVecF(&lightP->dir, &lightP->dir);
}

void Hu3DGLightPosSet(HU3DLIGHTID lightId, float posX, float posY, float posZ, float dirX, float dirY, float dirZ)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPSet(lightP, posX, posY, posZ, dirX, dirY, dirZ);
}

void Hu3DLLightPosSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float posX, float posY, float posZ, float dirX, float dirY, float dirZ)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightPSet(lightP, posX, posY, posZ, dirX, dirY, dirZ);
}

static void Hu3DLightPASet(HU3DLIGHT *lightP, float posX, float posY, float posZ, float angleX, float angleY)
{
    lightP->pos.x = posX;
    lightP->pos.y = posY;
    lightP->pos.z = posZ;
    lightP->dir.x = HuSin((float)(angleY-180.0))*HuCos(angleX);
    lightP->dir.y = -HuSin(angleX);
    lightP->dir.z = HuCos((float)(angleY-180.0))*HuCos(angleX);
    HuNormVecF(&lightP->dir, &lightP->dir);
}

void Hu3DGLightPosAngleSet(HU3DLIGHTID lightId, float posX, float posY, float posZ, float angleX, float angleY)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPASet(lightP, posX, posY, posZ, angleX, angleY);
}

void Hu3DLLightPosAngleSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float posX, float posY, float posZ, float angleX, float angleY)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightPASet(lightP, posX, posY, posZ, angleX, angleY);
}

static void Hu3DLightPAimSetV(HU3DLIGHT *lightP, HuVecF *pos, HuVecF *aim)
{
    lightP->pos = *pos;
    HuSubVecF(&lightP->dir, aim, pos);
    HuNormVecF(&lightP->dir, &lightP->dir);
}

void Hu3DGLightPosAimSetV(HU3DLIGHTID lightId, HuVecF *pos, HuVecF *aim)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightPAimSetV(lightP, pos, aim);
}

void Hu3DLLightPosAimSetV(HU3DMODELID modelId, HU3DLLIGHTID lightId, HuVecF *pos, HuVecF *aim)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightPAimSetV(lightP, pos, aim);
}

void Hu3DGLightPosAimSet(HU3DLIGHTID lightId, float posX, float posY, float posZ, float aimX, float aimY, float aimZ)
{
    Vec pos;
    Vec aim;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    aim.x = aimX;
    aim.y = aimY;
    aim.z = aimZ;
    Hu3DGLightPosAimSetV(lightId, &pos, &aim);
}

void Hu3DLLightPosAimSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float posX, float posY, float posZ, float aimX, float aimY, float aimZ)
{
    Vec pos;
    Vec aim;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    aim.x = aimX;
    aim.y = aimY;
    aim.z = aimZ;
    Hu3DLLightPosAimSetV(modelId, lightId,  &pos, &aim);
}

static void Hu3DLightStatSet(HU3DLIGHT *lightP, BOOL staticF)
{
    if(staticF) {
        lightP->type |= HU3D_LIGHT_TYPE_STATIC;
    } else {
        lightP->type &= ~HU3D_LIGHT_TYPE_STATIC;
    }
}
void Hu3DGLightStaticSet(HU3DLIGHTID lightId, BOOL staticF)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[lightId];
    Hu3DLightStatSet(lightP, staticF);
}

void Hu3DLLightStaticSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, BOOL staticF)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DLIGHT *lightP = &Hu3DLocalLight[modelP->lLightId[lightId]];
    Hu3DLightStatSet(lightP, staticF);
}

s16 Hu3DLLightParamGet(HU3DLIGHTID lLightId, Vec *pos, Vec *dir, GXColor *color)
{
    HU3DLIGHT *lightP = &Hu3DLocalLight[lLightId];
    *pos = lightP->pos;
    *dir = lightP->dir;
    *color = lightP->color;
    return lightP->type;
}

s16 Hu3DGLightParamGet(HU3DLIGHTID gLightId, Vec *pos, Vec *dir, GXColor *color)
{
    HU3DLIGHT *lightP = &Hu3DGlobalLight[gLightId];
    *pos = lightP->pos;
    *dir = lightP->dir;
    *color = lightP->color;
    return lightP->type;
}

s32 Hu3DModelLightInfoSet(HU3DMODELID modelId, s16 staticF)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFOBJECT *objPtr;
    s16 lightNum;
    s16 i;
    if(modelP->lightNum) {
        return modelP->lightNum;
    }
    for(objPtr = hsf->object, i=lightNum=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        HU3DLIGHTID lightId;
        HU3DLIGHT *lightP;
        HuVecF lightDir;
        if(obj->type != HSF_OBJ_LIGHT) {
            continue;
        }
        lightDir.x = obj->light.target.x - obj->light.pos.x;
        lightDir.y = obj->light.target.y - obj->light.pos.y;
        lightDir.z = obj->light.target.z - obj->light.pos.z;
        lightId = Hu3DGLightCreate(obj->light.pos.x, obj->light.pos.y, obj->light.pos.z, lightDir.x, lightDir.y, lightDir.z, obj->light.r, obj->light.g, obj->light.b);
        modelP->lightId[lightNum] = lightId;
        lightP = &Hu3DGlobalLight[lightId];
        Hu3DGLightStaticSet(lightId, staticF);
        switch (obj->light.type) {
            case 0:
                Hu3DGLightSpotSet(lightId, GX_SP_COS, obj->light.cutoff);
                break;
            
            case 1:
                Hu3DGLightPointSet(lightId, obj->light.refBrightness-obj->light.refDistance, 1.0f, GX_DA_MEDIUM);
                Hu3DGLightPosSet(lightId, obj->light.pos.x, obj->light.pos.y, obj->light.pos.z, 0, 1, 0);
                break;
            
            case 2:
                Hu3DGLightInfinitytSet(lightId);
                break;
        }
        lightNum++;
        if(lightNum >= HU3D_GLIGHT_MAX) {
            break;
        }
    }
    modelP->lightNum = lightNum;
    return lightNum;
}

HU3DLIGHTID Hu3DModelLightIdGet(HU3DMODELID modelId, char *objName)
{
    char name[OBJNAME_MAX_LEN];
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFOBJECT *objPtr;
    s16 i;
    s16 lightNo;
    strcpy(name, MakeObjectName(objName));
    for(objPtr = hsf->object, i=lightNo=0; i<hsf->objectNum; i++, objPtr++) {
        HSFOBJECT *obj = objPtr;
        if(obj->type != HSF_OBJ_LIGHT) {
            continue;
        }
        if(strcmp(name, obj->name) == 0) {
            return modelP->lightId[lightNo];
        }
        lightNo++;
    }
    OSReport("Error: Not Find Light Name.(%s)\n", objName);
    return HU3D_LIGHTID_NONE;
}

static void lightSet(HU3DLIGHT *lightP, s16 bit, Mtx cameraMtxXPose, Mtx cameraMtx, float hilitePower);

s16 Hu3DLightSet(HU3DMODEL *modelP, Mtx cameraMtx, Mtx cameraMtxXPose, float hilitePower)
{
    s16 bit;
    HU3DLIGHT *lightP;
    s16 lightBit;
    s16 i;
    
    lightBit = 0;
    bit = (1 << 0);
    for(lightP = &Hu3DGlobalLight[0], i=0; i<HU3D_GLIGHT_MAX; i++, lightP++) {
        if(lightP->type != HU3D_LIGHT_TYPE_NONE) {
            lightSet(lightP, bit, cameraMtxXPose, cameraMtx, hilitePower);
            lightBit |= bit;
            bit <<= 1;
        }
    }
    if(modelP->attr & HU3D_ATTR_LLIGHT) {
        for(i=0; i<HU3D_MODEL_LLIGHT_MAX; i++) {
            if(modelP->lLightId[i] != HU3D_LIGHTID_NONE) {
                lightP = &Hu3DLocalLight[modelP->lLightId[i]];
                lightSet(lightP, bit, cameraMtxXPose, cameraMtx, hilitePower);
                lightBit |= bit;
                bit <<= 1;
            }
        }
    }
    return lightBit;
}

static void lightSet(HU3DLIGHT *lightP, s16 lightBit, Mtx cameraMtx, Mtx cameraMtxXPose, float hilitePower)
{
    GXLightObj lightObj;
    HuVecF dir;
    HuVecF pos;
    switch(LIGHT_TYPE_GET(lightP)) {
        case HU3D_LIGHT_TYPE_SPOT:
            GXInitLightAttn(&lightObj, 1, 0, 0, 1, 0, 0);
            GXInitLightSpot(&lightObj, lightP->cutoff, lightP->func);
            break;
            
        case HU3D_LIGHT_TYPE_INFINITYT:
            GXInitLightAttnK(&lightObj, 1, 0, 0);
            GXInitLightDistAttn(&lightObj, 0, 1, GX_DA_OFF);
            HuScaleVecF(&lightP->dir, &lightP->pos, -1000000.0f);
            break;
        
        case HU3D_LIGHT_TYPE_POINT:
            GXInitLightAttn(&lightObj, 1, 0, 0, 0, 0, 0);
            GXInitLightDistAttn(&lightObj, lightP->cutoff, lightP->brightness, lightP->func);
            break;
    }
    if(lightP->type & HU3D_LIGHT_TYPE_STATIC) {
        MTXMultVec(cameraMtx, &lightP->dir, &dir);
        MTXMultVec(cameraMtxXPose, &lightP->pos, &pos);
        GXInitLightPos(&lightObj, pos.x, pos.y, pos.z);
    } else {
        GXInitLightPos(&lightObj, lightP->pos.x, lightP->pos.y, lightP->pos.z);
        dir = lightP->dir;
    }
    if(0.0f == hilitePower) {
        GXInitLightDir(&lightObj, dir.x, dir.y, dir.z);
    } else {
        GXInitSpecularDir(&lightObj, dir.x, dir.y, dir.z);
        GXInitLightAttn(&lightObj, 0, 0, 1, hilitePower/2, 0, 1-(hilitePower/2));
    }
    GXInitLightColor(&lightObj, lightP->color);
    GXLoadLightObjImm(&lightObj, lightBit);
}

void Hu3DReflectMapSet(ANIMDATA *anim)
{
    if (reflectAnim[0] != (void *)refMapData0) {
        HuMemDirectFree(reflectAnim[0]);
    }
    reflectAnim[0] = HuSprAnimRead(anim);
    reflectMapNo = 0;
}

void Hu3DReflectNoSet(s16 no)
{
    reflectMapNo = no;
}

void Hu3DFogSet(float start, float end, u8 r, u8 g, u8 b)
{
    FogData.fogType = GX_FOG_PERSP_EXP;
    FogData.fogStart = start;
    FogData.fogEnd = end;
    FogData.fogColor.r = r;
    FogData.fogColor.g = g;
    FogData.fogColor.b = b;
    FogData.fogColor.a = 255;
}

void Hu3DFogClear(void)
{
    FogData.fogType = GX_FOG_NONE;
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, BGColor);
}

void Hu3DAmbColorSet(float r, float g, float b)
{
    Hu3DAmbColR = r;
    Hu3DAmbColG = g;
    Hu3DAmbColB = b;
}

#define SHADOW_DEFAULT_SIZE 192

void Hu3DShadowMultiCreate(float fov, float near, float far, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & cameraBit) {
            HU3DSHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->size = SHADOW_DEFAULT_SIZE;
            if(!shadowP->buf) {
                shadowP->buf = HuMemDirectMalloc(HEAP_MODEL, SHADOW_DEFAULT_SIZE*SHADOW_DEFAULT_SIZE);
            }
            shadowP->fov = fov;
            shadowP->near = near;
            shadowP->far = far;
            shadowP->camPos.x = 300;
            shadowP->camPos.y = 300;
            shadowP->camPos.z = 0;
            shadowP->camTarget.x = shadowP->camTarget.y = shadowP->camTarget.z = 0;
            shadowP->camUp.x = -1;
            shadowP->camUp.y = 1;
            shadowP->camUp.z = 0;
            MTXLightPerspective(shadowP->projMtx, fov, HU_DISP_ASPECT, 0.5f, -0.5f, 0.5f, 0.5f);
            HuNormVecF(&shadowP->camUp, &shadowP->camUp);
            shadowP->color.r = shadowP->color.g = shadowP->color.b = 0;
            shadowP->color.a = 128;
        }
    }
    Hu3DShadowCamBit = cameraBit;
    Hu3DShadowF = TRUE;
}

void Hu3DShadowCreate(float fov, float near, float far)
{
    Hu3DShadowMultiCreate(fov, near, far, HU3D_CAM0);
}

void Hu3DShadowMultiPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            HU3DSHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->camPos = *camPos;
            shadowP->camTarget = *camTarget;
            shadowP->camUp = *camUp;
        }
    }
}

void Hu3DShadowPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget)
{
    Hu3DShadowMultiPosSet(camPos, camUp, camTarget, HU3D_CAM0);
}

void Hu3DShadowMultiTPLvlSet(float tpLvl, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            Hu3DShadowBuf[i].color.a = tpLvl*255.0f;
        }
    }
}

void Hu3DShadowTPLvlSet(float tpLvl)
{
    Hu3DShadowMultiTPLvlSet(tpLvl, HU3D_CAM0);
}

void Hu3DShadowMultiSizeSet(u16 size, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            HU3DSHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->size = size;
            if(shadowP->buf) {
                HuMemDirectFree(shadowP->buf);
            }
            shadowP->buf = HuMemDirectMalloc(HEAP_MODEL, size*size);
        }
    }
}

void Hu3DShadowSizeSet(u16 size)
{
    Hu3DShadowMultiSizeSet(size, HU3D_CAM0);
}

void Hu3DShadowMultiColSet(u8 r, u8 g, u8 b, s16 cameraBit)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if((1 << i) & (cameraBit & Hu3DShadowCamBit)) {
            HU3DSHADOW *shadowP = &Hu3DShadowBuf[i];
            shadowP->color.r = r;
            shadowP->color.g = g;
            shadowP->color.b = b;
        }
    }
}

void Hu3DShadowColSet(u8 r, u8 g, u8 b)
{
    Hu3DShadowMultiColSet(r, g, b, HU3D_CAM0);
}

static void Hu3DShadowExec(BOOL bgColorF)
{
    Mtx temp;
    Mtx modelview;
    Mtx rot;
    Mtx44 proj;
    GXColor black = { 0, 0, 0, 255 };
    HU3DMODEL *modelP;
    s16 i;
    s32 dataSize;
    Hu3DDrawPreInit();
    GXSetCopyClear(black, GX_MAX_Z24);
    MTXPerspective(proj, Hu3DShadow->fov, HU_DISP_ASPECT, Hu3DShadow->near, Hu3DShadow->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    if(Hu3DShadow->size <= 240) {
        GXSetScissor(2, 2, (Hu3DShadow->size*2)-4, (Hu3DShadow->size*2)-4);
        GXSetViewport(0, 0, (Hu3DShadow->size*2), (Hu3DShadow->size*2), 0, 1);
        dataSize = (Hu3DShadow->size/2)*(Hu3DShadow->size/2);
    } else {
        GXSetScissor(1, 1, Hu3DShadow->size-2, Hu3DShadow->size-2);
        GXSetViewport(0, 0, Hu3DShadow->size, Hu3DShadow->size, 0, 1);
        dataSize = Hu3DShadow->size*Hu3DShadow->size;
    }
    MTXLookAt(Hu3DCameraMtx, &Hu3DShadow->camPos, &Hu3DShadow->camUp, &Hu3DShadow->camTarget);
    MTXCopy(Hu3DCameraMtx, Hu3DShadow->lookAtMtx);
    modelP = &Hu3DData[0];
    shadowModelDrawF = TRUE;
    GXInvalidateTexAll();
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, BGColor);
    for(i=0; i<HU3D_MODEL_MAX; i++, modelP++) {
        if(!modelP->hsf) {
            continue;
        }
        if((modelP->attr & HU3D_ATTR_SHADOW) && (modelP->attr & (HU3D_ATTR_DISPOFF|HU3D_ATTR_REFLECT_MODEL)) == 0 && (modelP->attr & HU3D_ATTR_HOOK) == 0) {
            if(Hu3DShadowCamBit == HU3D_CAM0 || (Hu3DCameraBit & modelP->cameraBit)) {
                BOOL vtxInvalidateF;
                if(modelP->attr & HU3D_ATTR_MOTION_OFF) {
                    vtxInvalidateF = FALSE;
                    if(modelP->motId != HU3D_MOTID_NONE) {
                        Hu3DMotionExec(i, modelP->motId, modelP->motWork.time, FALSE);
                    }
                    if(modelP->motIdShift != HU3D_MOTID_NONE) {
                        Hu3DSubMotionExec(i);
                    }
                    if(modelP->motIdOvl != HU3D_MOTID_NONE) {
                        Hu3DMotionExec(i, modelP->motIdOvl, modelP->motOvlWork.time, TRUE);
                    }
                    if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                        ClusterMotionExec(modelP);
                        vtxInvalidateF = TRUE;
                    }
                    if(modelP->motIdShape != HU3D_MOTID_NONE) {
                        if(modelP->motId == HU3D_MOTID_NONE) {
                            Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time, FALSE);
                        } else {
                            Hu3DMotionExec(i, modelP->motIdShape, modelP->motShapeWork.time, TRUE);
                        }
                    }
                    if ((modelP->attr & (HU3D_ATTR_ENVELOPE_OFF|HU3D_ATTR_HOOKFUNC)) == 0 || (modelP->attr & HU3D_ATTR_MOTION_MODEL)) {
                        vtxInvalidateF = TRUE;
                        InitVtxParm(modelP->hsf);
                        if(modelP->motIdShape != HU3D_MOTID_NONE) {
                            ShapeProc(modelP->hsf);
                        }
                        if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
                            ClusterProc(modelP);
                        }
                        if(modelP->hsf->cenvNum) {
                            EnvelopeProc(modelP->hsf);
                        }
                        PPCSync();
                    }
                    modelP->attr |= HU3D_ATTR_MOT_EXEC;
                }
                mtxRot(rot, modelP->rot.x, modelP->rot.y, modelP->rot.z);
                MTXScale(temp, modelP->scale.x, modelP->scale.y, modelP->scale.z);
                MTXConcat(rot, temp, temp);
                mtxTransCat(temp, modelP->pos.x, modelP->pos.y, modelP->pos.z);
                PSMTXConcat(Hu3DCameraMtx, temp, modelview);
                PSMTXConcat(modelview, modelP->mtx, modelview);
                Hu3DDraw(modelP, modelview, &modelP->scale);
            }
        }
    }
    Hu3DDrawPost();
    shadowModelDrawF = FALSE;
    if(Hu3DShadow->size <= 240) {
        GXSetTexCopySrc(0, 0, Hu3DShadow->size*2, Hu3DShadow->size*2);
        GXSetTexCopyDst(Hu3DShadow->size, Hu3DShadow->size, GX_CTF_R8, GX_TRUE);
    } else {
        GXSetTexCopySrc(0, 0, Hu3DShadow->size, Hu3DShadow->size);
        GXSetTexCopyDst(Hu3DShadow->size, Hu3DShadow->size, GX_CTF_R8, GX_FALSE);
    }
    GXCopyTex(Hu3DShadow->buf, GX_TRUE);
    GXSetViewport(0.0f, 0.0f, RenderMode->fbWidth, RenderMode->xfbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    MTXOrtho(proj, 0, 1, 0, 1, 0, 1);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_U8, 0);
    if(bgColorF) {
        GXSetTevColor(GX_TEVREG0, BGColor);
    } else {
        GXSetTevColor(GX_TEVREG0, black);
    }
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(0);
    MTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3u8(0, 0, 0);
    GXPosition3u8(1, 0, 0);
    GXPosition3u8(1, 1, 0);
    GXPosition3u8(0, 1, 0);
    GXEnd();
}

static void Hu3DReflectModelExec(void)
{
    Mtx temp;
    Mtx modelview;
    Mtx44 proj;
    Vec pos;
    Vec target;
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    HU3DMODEL *modelP;
    GXColor clearColor = {};
    Hu3DDrawPreInit();
    GXSetCopyClear(clearColor, GX_MAX_Z24);
    MTXPerspective(proj, 50.0f, ((float)REFLECT_TEX_W/(float)REFLECT_TEX_H), 20.0f, 1000.0f);
    GXSetProjection(proj, GX_PERSPECTIVE);
    GXSetScissor(0, 0, REFLECT_TEX_W, REFLECT_TEX_H);
    GXSetViewport(0, 0, REFLECT_TEX_W, REFLECT_TEX_H, 0, 1);
    HuSubVecF(&pos, &cameraP->pos, &cameraP->target);
    HuNormVecF(&pos, &pos);
    HuScaleVecF(&pos, &pos, -100);
    target.x = target.y = target.z = 0;
    MTXLookAt(Hu3DCameraMtx, &pos, &cameraP->up, &target);
    modelP = &Hu3DData[Hu3DReflectModelId];
    GXInvalidateTexAll();
    GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, BGColor);
    if(!modelP->hsf) {
        return;
    }
    if((modelP->attr & HU3D_ATTR_DISPOFF) || (modelP->attr & HU3D_ATTR_HOOK)) {
        return;
    }
    if(modelP->motId != HU3D_MOTID_NONE) {
        Hu3DMotionExec(Hu3DReflectModelId, modelP->motId, modelP->motWork.time, FALSE);
    }
    if ((modelP->attr & (HU3D_ATTR_ENVELOPE_OFF|HU3D_ATTR_HOOKFUNC)) == 0 || (modelP->attr & HU3D_ATTR_MOTION_MODEL)) {
        InitVtxParm(modelP->hsf);
        if(modelP->motIdShape != HU3D_MOTID_NONE) {
            ShapeProc(modelP->hsf);
        }
        if(modelP->attr & HU3D_ATTR_CLUSTER_ON) {
            ClusterProc(modelP);
        }
        if(modelP->hsf->cenvNum) {
            EnvelopeProc(modelP->hsf);
        }
        PPCSync();
    }
    MTXConcat(Hu3DCameraMtx, temp, modelview); //BUG: Use of uninitialized temporary matrix
    MTXConcat(Hu3DCameraMtx, modelP->mtx, modelview);
    Hu3DDraw(modelP, modelview, &modelP->scale);
    Hu3DDrawPost();
    GXSetTexCopySrc(0, 0, REFLECT_TEX_W, REFLECT_TEX_H);
    GXSetTexCopyDst(REFLECT_TEX_W, REFLECT_TEX_H, GX_TF_RGB5A3, GX_FALSE);
    GXCopyTex(Hu3DReflectModelAnim->bmp->data, GX_TRUE);
    GXPixModeSync();
    GXSetViewport(0.0f, 0.0f, RenderMode->fbWidth, RenderMode->xfbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    MTXOrtho(proj, 0, 1, 0, 1, 0, 1);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetTevColor(GX_TEVREG0, BGColor);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(0);
    MTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(0, 0, 0);
    GXPosition3f32(1, 0, 0);
    GXPosition3f32(1, 1, 0);
    GXPosition3f32(0, 1, 0);
    GXEnd();
}

HU3DPROJID Hu3DProjectionCreate(ANIMDATA *anim, float fov, float near, float far)
{
    HU3DPROJECTION *projP;
    HU3DPROJID projId;
    for(projP=&Hu3DProjection[0], projId=0; projId<HU3D_PROJ_MAX; projId++, projP++) {
        if(!projP->anim) {
            break;
        }
    }
    if(projId == HU3D_PROJ_MAX) {
        return HU3D_PROJID_NONE;
    }
    projP->anim = anim;
    projP->fov = fov;
    projP->near = near;
    projP->far = far;
    projP->camPos.x = 1000;
    projP->camPos.y = 1000;
    projP->camPos.z = 0;
    projP->camTarget.x = projP->camTarget.y = projP->camTarget.z = 0;
    projP->camUp.x = -1;
    projP->camUp.y = 1;
    projP->camUp.z = 0;
    MTXLightPerspective(projP->projMtx, fov, HU_DISP_ASPECT, 0.5f, -0.5f, 0.5f, 0.5f);
    HuNormVecF(&projP->camUp, &projP->camUp);
    projP->alpha = 128;
    Hu3DProjectionNum++;
    return projId;
}

void Hu3DProjectionKill(HU3DPROJID projId)
{
    HuSprAnimKill(Hu3DProjection[projId].anim);
    Hu3DProjection[projId].anim = NULL;
}

void Hu3DProjectionPosSet(HU3DPROJID projId, HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget)
{
    Hu3DProjection[projId].camPos = *camPos;
    Hu3DProjection[projId].camTarget = *camTarget;
    Hu3DProjection[projId].camUp = *camUp;
}

void Hu3DProjectionTPLvlSet(HU3DPROJID projId, float tpLvl)
{
    Hu3DProjection[projId].alpha = tpLvl*255;
}

void Hu3DMipMapSet(void *animData, HU3DMODELID modelId, char *bmpName)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFATTRIBUTE *attrPtr;
    HSFBITMAP *bmp;
    ANIMBMP *animBmp;
    s16 i;
    ANIMDATA *anim;
    s32 dataSize;
    char *data;
    char *dataPtr;
    
    for(attrPtr=hsf->attribute, i=0; i<hsf->attributeNum; i++, attrPtr++) {
        if(!strcmp(bmpName, attrPtr->bitmap->name)) {
            break;
        }
    }
    if(i == hsf->attributeNum) {
        OSReport("Error: Not Found %s for MipMapSet\n", bmpName);
        return;
    }
    bmp = attrPtr->bitmap;
    anim = HuSprAnimRead(animData);
    animBmp = anim->bmp;
    for(dataSize=i=0; i<anim->bmpNum; i++, animBmp++) {
        dataSize += animBmp->dataSize;
    }
    data = HuMemDirectMallocNum(HEAP_MODEL, dataSize, modelP->mallocNo);
    dataPtr = data;
    animBmp = anim->bmp;
    bmp->data = dataPtr;
    bmp->sizeX = animBmp->sizeX;
    bmp->sizeY = animBmp->sizeY;
    bmp->palSize = animBmp->palNum;
    bmp->palData = animBmp->palData;
    bmp->maxLod = anim->bmpNum;
    switch (animBmp->dataFmt) { 
        case ANIM_BMP_RGBA8:
            bmp->dataFmt = HSF_BMPFMT_RGBA8;
            break;
        
        case ANIM_BMP_RGB5A3:
            bmp->dataFmt = HSF_BMPFMT_RGB5A3;
            break;
        
        case ANIM_BMP_C8:
            bmp->dataFmt = HSF_BMPFMT_CI_RGB5A3;
            bmp->pixSize = 8;
            break;
        
        case ANIM_BMP_C4:
            bmp->dataFmt = HSF_BMPFMT_CI_RGB5A3;
            bmp->pixSize = 4;
            break;
    }
    for(i=0; i<anim->bmpNum; i++, animBmp++) {
        memcpy(data, animBmp->data, animBmp->dataSize);
        data += animBmp->dataSize;
    }
    DCFlushRange(dataPtr, dataSize);
}

void Hu3DFbCopyExec(s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF, void *buf)
{
    GXSetTexCopySrc(x, y, w, h);
    if(mipmapF) {
        GXSetTexCopyDst(w/2, h/2, texFmt, mipmapF);
    } else {
        GXSetTexCopyDst(w, h, texFmt, mipmapF);
    }
    GXCopyTex(buf, GX_FALSE);
}

static void FbCopyLayerHook(s16 layerNo);

void Hu3DFbCopyLayerSet(s16 layerNo, s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF, void *buf)
{
    FBCOPYLAYER *copyLayerP = &FbCopyLayer[layerNo];
    copyLayerP->layerNo = layerNo;
    copyLayerP->x = x;
    copyLayerP->y = y;
    copyLayerP->w = w;
    copyLayerP->h = h;
    copyLayerP->texFmt = texFmt;
    copyLayerP->mipmap = mipmapF;
    copyLayerP->buf = buf;
    layerHook[layerNo] = FbCopyLayerHook;
}

static void FbCopyLayerHook(s16 layerNo)
{
    FBCOPYLAYER *copyLayerP = &FbCopyLayer[layerNo];
    Hu3DFbCopyExec(copyLayerP->x, copyLayerP->y, copyLayerP->w, copyLayerP->h, copyLayerP->texFmt, copyLayerP->mipmap, copyLayerP->buf);
}

void Hu3DZClear(void)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    Mtx44 proj;
    Mtx modelview;
    Vec pos;
    Vec target;
    Vec up;
    float z;
    
    MTXPerspective(proj, cameraP->fov, cameraP->aspect, cameraP->near, cameraP->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    GXSetViewport(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH, cameraP->viewportNear, cameraP->viewportFar);
    GXSetScissor(cameraP->scissorX, cameraP->scissorY, cameraP->scissorW, cameraP->scissorH);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 0, GX_AOP_AND, GX_GEQUAL, 0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetColorUpdate(GX_FALSE);
    pos.x = pos.y = pos.z = 0;
    target.x = target.y = 0;
    target.z = -100;
    up.x = up.z = 0;
    up.y = 1;
    MTXLookAt(modelview, &pos, &up, &target);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    z = cameraP->far-1;
    pos.x = pos.y = z*HuTan(cameraP->fov/2);
    pos.x *= HU_DISP_ASPECT;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-pos.x, -pos.y, -z);
    GXPosition3f32(pos.x, -pos.y, -z);
    GXPosition3f32(pos.x, pos.y, -z);
    GXPosition3f32(-pos.x, pos.y, -z);
    GXSetColorUpdate(GX_TRUE);
}

void Hu3DZClearLayerSet(s16 layerNo)
{
    layerHook[layerNo] = (HU3DLAYERHOOK)Hu3DZClear;
}