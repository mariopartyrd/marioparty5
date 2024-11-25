#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/camera.h"
#include "game/board/masu.h"
#include "game/hu3d.h"
#include "game/disp.h"
#include "game/audio.h"
#include "game/flag.h"
#include "game/frand.h"

#define MB_CAMERA_STACK_MAX 16

typedef struct listenerParam_s {
    float sndDist;
    float sndSpeed;
    float startDis;
    float frontSurDis;
    float backSurDis;
} LISTENERPARAM;

static MBVIEW viewTbl[] = {
    { -33, 1800, 25 },
    { -33, 2100, 25 },
    { -33, 3200, 25 },
};

static LISTENERPARAM listenerParamTbl[MBNO_MAX] = {
    { 8000, 1000, 0, 4000, 4000 },
    { 8000, 1000, 0, 4000, 4000 },
    { 8000, 1000, 0, 4000, 4000 },
    { 8000, 1000, 0, 4000, 4000 },
    { 8000, 1000, 0, 4000, 4000 },
    { 8000, 1000, 2000, 4000, 4000 },
    { 8000, 1000, 0, 4000, 4000 },
    { 8000, 1000, 0, 4000, 4000 },
    { 8000, 1000, 0, 4000, 4000 },
};

static MBCAMERA cameraWork;
static MBCAMERA cameraStack[MB_CAMERA_STACK_MAX];

static int cameraSp;
static OMOBJ *cameraObj;

static void CameraExec(OMOBJ *obj);
static void CameraMotionMain(MBCAMERA *cameraP);
static float CameraTargetCalc(HuVecF *rot, float fov, HuVecF *target);

static void CameraMain(MBCAMERA *cameraP);

static void DefaultPosHook(MBCAMERA *cameraP);

void MBCameraCreate(void)
{
    MBCAMERA *cameraP = MBCameraGet();
    LISTENERPARAM *listenerParam = &listenerParamTbl[MBBoardNoGet()];
    HuVecF pos;
    HuVecF dir;
    cameraSp = 0;
    memset(&cameraStack[0], 0, sizeof(cameraStack));
    memset(&cameraWork, 0, sizeof(cameraWork));
    cameraP->attrAll = 0;
    cameraP->dispOn = TRUE;
    cameraP->fov = 25;
    cameraP->near = 100;
    cameraP->far = 20000;
    cameraP->aspect = HU_DISP_ASPECT;
    cameraP->viewportX = 0;
    cameraP->viewportY = 0;
    cameraP->viewportH = HU_FB_HEIGHT;
    cameraP->viewportW = HU_FB_WIDTH;
    cameraP->viewportNear = 0;
    cameraP->viewportFar = 1;
    cameraP->up.x = cameraP->up.z = 0;
    cameraP->up.y = 1;
    cameraP->quakeTime = 0;
    cameraP->pos.x = cameraP->pos.y = cameraP->pos.z = 0;
    cameraP->target.x = cameraP->target.y = cameraP->target.z = 0;
    cameraP->offset.x = cameraP->offset.y = cameraP->offset.z = 0;
    cameraP->posHook = DefaultPosHook;
    cameraP->zoom = viewTbl[2].zoom;
    cameraP->rot.x = viewTbl[2].rotX;
    cameraP->rot.y = cameraP->rot.z = 0;
    cameraP->bit = HU3D_CAM0;
    cameraP->speed = 0.15f;
    cameraP->focusNum = 0;
    memset(&cameraP->focus[0], 0, MB_CAMERA_FOCUS_MAX*sizeof(MBCAMERAFOCUS));
    cameraP->motion.curveType = MB_CAMERA_CURVE_LINEAR;
    cameraP->motion.time = 0;
    cameraP->motion.maxTime = -1;
    Hu3DCameraCreate(cameraP->bit);
    Hu3DCameraScissorSet(cameraP->bit, cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH);
    cameraP->target.x = cameraP->target.y = cameraP->target.z = 0;
    pos.x = pos.y = 0;
    pos.z = 100000;
    dir.x = 0;
    dir.y = 0;
    dir.z = -1;
    VECNormalize(&dir, &dir);
    HuAudFXListnerSetEX(&pos, &dir, listenerParam->sndDist, listenerParam->sndSpeed, listenerParam->startDis, listenerParam->frontSurDis, listenerParam->backSurDis);
    cameraObj = MBAddObj(32256, 0, 0, CameraExec);
    omSetStatBit(cameraObj, OM_STAT_NOPAUSE);
}

static void CameraExec(OMOBJ *obj)
{
    MBCAMERA *cameraP = MBCameraGet();
    HuVecF dir;
    if(MBKillCheck()) {
        MBDelObj(obj);
        return;
    }
    if(MBPauseFlagCheck()) {
        return;
    }
    Hu3DCameraPerspectiveSet(cameraP->bit, cameraP->fov, cameraP->near, cameraP->far, cameraP->aspect);
    Hu3DCameraViewportSet(cameraP->bit, cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH, cameraP->viewportNear, cameraP->viewportFar);
    if(!_CheckFlag(FLAG_BOARD_CAMERAMOT)) {
        CameraMotionMain(cameraP);
    } else {
        CameraMain(cameraP);
    }
    cameraP->posHook(cameraP);
    Hu3DCameraPosSetV(cameraP->bit, &cameraP->pos, &cameraP->up, &cameraP->target);
    VECSubtract(&cameraP->target, &cameraP->pos, &dir);
    VECNormalize(&dir, &dir);
    HuAudFXListnerUpdate(&cameraP->pos, &dir);
}

static void CameraMotionMain(MBCAMERA *cameraP)
{
    HuVecF pos;
    float zoom;
    float time;
    if(cameraP->focusNum == 0) {
        return;
    }
    zoom = CameraTargetCalc(&cameraP->rot, cameraP->fov, &pos);
    time = (cameraP->skipF) ? cameraP->speed : 1.0f;
    cameraP->target.x += time*(pos.x-cameraP->target.x);
    cameraP->target.y += time*(pos.y-cameraP->target.y);
    cameraP->target.z += time*(pos.z-cameraP->target.z);
    if(zoom < 0.0f) {
        return;
    }
    if(cameraP->skipF) {
        cameraP->zoom += 0.2f*(zoom-cameraP->zoom);
    } else {
        cameraP->zoom = zoom;
    }
}

static float CameraTargetCalc(HuVecF *rot, float fov, HuVecF *target)
{
    MBCAMERA *cameraP = MBCameraGet();
    int i, j;
    HuVecF focusPos[MB_CAMERA_FOCUS_MAX];
    HuVecF dir[2];
    HuVecF targetBegin;
    HuVecF targetEnd;
    HuVecF ofs2;
    HuVecF ofs1;
    HuVecF rot2;
    HuVecF rot1;
    HuVecF aim;
    float targetDist[2];
    float len;
    float zoomMax;
    float zoom;
    float weight;
    if(rot == NULL) {
        rot = &cameraP->rot;
    }
    if(fov < 0.0f) {
        fov = cameraP->fov;
    }
    for(i=0; i<cameraP->focusNum; i++) {
        if(cameraP->focus[i].type == MB_CAMERA_FOCUS_MODEL) {
            MBModelPosGet(cameraP->focus[i].targetId, &focusPos[i]);
        } else {
            MBMasuPosGet(MASU_LAYER_DEFAULT, cameraP->focus[i].targetId, &focusPos[i]);
        }
        VECAdd(&focusPos[i], &cameraP->offset, &focusPos[i]);
        if(i == 0) {
            targetBegin = targetEnd = focusPos[i];
        } else {
            if(focusPos[i].x < targetBegin.x) {
                targetBegin.x = focusPos[i].x;
            } else if(focusPos[i].x > targetEnd.x) {
                targetEnd.x = focusPos[i].x;
            }
            if(focusPos[i].y < targetBegin.y) {
                targetBegin.y = focusPos[i].y;
            } else if(focusPos[i].y > targetEnd.y) {
                targetEnd.y = focusPos[i].y;
            }
            if(focusPos[i].z < targetBegin.z) {
                targetBegin.z = focusPos[i].z;
            } else if(focusPos[i].z > targetEnd.z) {
                targetEnd.z = focusPos[i].z;
            }
        }
    }
    target->x = targetBegin.x+(0.5f*(targetEnd.x-targetBegin.x));
    target->y = targetBegin.y+(0.5f*(targetEnd.y-targetBegin.y));
    target->z = targetBegin.z+(0.5f*(targetEnd.z-targetBegin.z));
    if(cameraP->focusNum < 2) {
        return -1;
    }
    rot2.x = HuSin(rot->y)*HuSin(rot->x);
    rot2.y = HuCos(rot->x);
    rot2.z = HuCos(rot->y)*HuSin(rot->x);
    rot1.x = -HuSin(rot->y)*HuCos(rot->x);
    rot1.y = HuSin(rot->x);
    rot1.z = -HuCos(rot->y)*HuCos(rot->x);
    VECCrossProduct(&rot2, &rot1, &dir[0]);
    VECNormalize(&dir[0], &dir[0]);
    VECCrossProduct(&dir[0], &rot1, &dir[1]);
    VECNormalize(&dir[1], &dir[1]);
    for(i=0; i<2; i++) {
        targetDist[i] = -((target->x*dir[i].x)+(target->y*dir[i].y)+(target->z*dir[i].z));
    }
    zoomMax = 0;
    for(i=0; i<cameraP->focusNum; i++) {
        for(j=0; j<2; j++) {
            len = ((dir[j].x*focusPos[i].x)+(dir[j].y*focusPos[i].y)+(dir[j].z*focusPos[i].z))+targetDist[j];
            ofs2.x = focusPos[i].x-(dir[j].x*len);
            ofs2.y = focusPos[i].y-(dir[j].y*len);
            ofs2.z = focusPos[i].z-(dir[j].z*len);
            weight = ((ofs2.z * rot1.z) + (((ofs2.y * rot1.y) + ((ofs2.x * rot1.x) - (target->x * rot1.x))) - (target->y * rot1.y))) - (target->z * rot1.z);
            ofs1.x = target->x+(weight*rot1.x);
            ofs1.y = target->y+(weight*rot1.y);
            ofs1.z = target->z+(weight*rot1.z);
            VECSubtract(&ofs2, &ofs1, &aim);
            len = VECMag(&aim);
            if(j != 0) {
                len /= cameraP->aspect;
            }
            zoom = len/(HuSin(fov*0.5f)/HuCos(fov*0.5f));
            zoom += -weight;
            if(zoom > zoomMax) {
                zoomMax = zoom;
            }
        }
    }
    zoomMax *= 1.3f;
    if(zoomMax < viewTbl[2].zoom) {
        zoomMax = viewTbl[2].zoom;
    }
    return zoomMax;
}

static void CameraMain(MBCAMERA *cameraP)
{
    MBCAMERAMOTION *motP = &cameraP->motion;
    MBCAMERAVIEWKEY *key1 = &motP->viewKey[0];
    MBCAMERAVIEWKEY *key2 = &motP->viewKey[1];
    MBCAMERAVIEWKEY *key3 = &motP->viewKey[2];
    float weight;
    if(motP->time >= motP->maxTime) {
        _ClearFlag(FLAG_BOARD_CAMERAMOT);
        return;
    }
    if(motP->maxTime > 1) {
        weight = (float)motP->time/(float)(motP->maxTime-1);
    } else {
        weight = 1.0f;
    }
    motP->time++;
    switch(motP->curveType) {
        case MB_CAMERA_CURVE_LINEAR:
            cameraP->fov = key1->fov+(weight*(key2->fov-key1->fov));
            cameraP->zoom = key1->zoom+(weight*(key2->zoom-key1->zoom));
            cameraP->rot.x = key1->rot.x+(weight*(key2->rot.x-key1->rot.x));
            cameraP->rot.y = key1->rot.y+(weight*(key2->rot.y-key1->rot.y));
            cameraP->rot.z = key1->rot.z+(weight*(key2->rot.z-key1->rot.z));
            cameraP->target.x = key1->pos.x+(weight*(key2->pos.x-key1->pos.x));
            cameraP->target.y = key1->pos.y+(weight*(key2->pos.y-key1->pos.y));
            cameraP->target.z = key1->pos.z+(weight*(key2->pos.z-key1->pos.z));
            break;
       
        case MB_CAMERA_CURVE_BEZIER:
            cameraP->fov = MBBezierCalc(key1->fov, key2->fov, key3->fov, weight);
            cameraP->zoom = MBBezierCalc(key1->zoom, key2->zoom, key3->zoom, weight);
            MBBezierCalcV(&key1->rot, &key2->rot, &key3->rot, &cameraP->rot, weight);
            MBBezierCalcV(&key1->pos, &key2->pos, &key3->pos, &cameraP->target, weight);
            break;
    }
}


static void DefaultPosHook(MBCAMERA *cameraP)
{
    cameraP->pos.x = (HuSin(cameraP->rot.y)*HuCos(cameraP->rot.x)*cameraP->zoom)+cameraP->target.x;
    cameraP->pos.y = (-HuSin(cameraP->rot.x)*cameraP->zoom)+cameraP->target.y;
    cameraP->pos.z = (HuCos(cameraP->rot.y)*HuCos(cameraP->rot.x)*cameraP->zoom)+cameraP->target.z;
    cameraP->up.x = HuSin(cameraP->rot.y)*HuSin(cameraP->rot.x);
    cameraP->up.y = HuCos(cameraP->rot.x);
    cameraP->up.z = HuCos(cameraP->rot.y)*HuSin(cameraP->rot.x);
    if(cameraP->quake) {
        cameraP->pos.x += (frandf()-0.5f)*cameraP->quakePower;
        cameraP->pos.y += (frandf()-0.5f)*cameraP->quakePower;
        cameraP->pos.z += (frandf()-0.5f)*cameraP->quakePower;
        if(--cameraP->quakeTime == 0) {
            cameraP->quake = FALSE;
            cameraP->quakeTime = 0;
            cameraP->quakePower = 0;
        }
    }
}

void MBCameraZoomSet(float zoom)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->zoom = zoom;
}

float MBCameraZoomGet(void)
{
    MBCAMERA *cameraP = MBCameraGet();
    return cameraP->zoom;
}

void MBCameraRotSetV(HuVecF *rot)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->rot.x = rot->x;
    cameraP->rot.y = rot->y;
}

void MBCameraRotSet(float rotX, float rotY, float rotZ)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->rot.x = rotX;
    cameraP->rot.y = rotY;
}

void MBCameraRotGet(HuVecF *rot)
{
    MBCAMERA *cameraP = MBCameraGet();
    *rot = cameraP->rot;
}

void MBCameraPosTargetSetV(HuVecF *target)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->target.x = target->x;
    cameraP->target.y = target->y;
    cameraP->target.z = target->z;
}

void MBCameraPosTargetSet(float targetX, float targetY, float targetZ)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->target.x = targetX;
    cameraP->target.y = targetY;
    cameraP->target.z = targetZ;
}

void MBCameraPosTargetGet(HuVecF *target)
{
    MBCAMERA *cameraP = MBCameraGet();
    *target = cameraP->target;
}

void MBCameraPosSetV(HuVecF *pos)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->pos.x = pos->x;
    cameraP->pos.y = pos->y;
    cameraP->pos.z = pos->z;
}

void MBCameraPosSet(float posX, float posY, float posZ)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->pos.x = posX;
    cameraP->pos.y = posY;
    cameraP->pos.z = posZ;
}

void MBCameraPosGet(HuVecF *pos)
{
    MBCAMERA *cameraP = MBCameraGet();
    *pos = cameraP->pos;
}

void MBCameraOffsetSetV(HuVecF *offset)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->offset.x = offset->x;
    cameraP->offset.y = offset->y;
    cameraP->offset.z = offset->z;
}

void MBCameraOffsetSet(float offsetX, float offsetY, float offsetZ)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->offset.x = offsetX;
    cameraP->offset.y = offsetY;
    cameraP->offset.z = offsetZ;
}

void MBCameraOffsetGet(HuVecF *offset)
{
    MBCAMERA *cameraP = MBCameraGet();
    *offset = cameraP->offset;
}

void MBCameraDirGet(HuVecF *dir)
{
    MBCAMERA *cameraP = MBCameraGet();
    VECSubtract(&cameraP->target, &cameraP->pos, dir);
    if(dir->x != 0.0f || dir->y != 0.0f || dir->z != 0.0f) {
        VECNormalize(dir, dir);
    }
}

void MBCameraPosDirGet(HuVecF *pos, HuVecF *dir)
{
    MBCAMERA *cameraP = MBCameraGet();
    VECSubtract(pos, &cameraP->pos, dir);
    if(dir->x != 0.0f || dir->y != 0.0f || dir->z != 0.0f) {
        VECNormalize(dir, dir);
    }
}

void MBCameraPosHookSet(MBCAMERAPOSHOOK posHook)
{
    MBCAMERA *cameraP = MBCameraGet();
    if(posHook != NULL) {
        cameraP->posHook = posHook;
    } else {
        cameraP->posHook = DefaultPosHook;
    }
}

void MBCameraBitSet(u16 bit)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->bit = bit;
}

void MBCameraFovSet(float fov)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->fov = fov;
}

void MBCameraScissorSet(int x, int y, int w, int h)
{
    MBCAMERA *cameraP = MBCameraGet();
    Hu3DCameraScissorSet(cameraP->bit, x, y, w, h);
}

void MBCameraNearFarSet(float near, float far)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->near = near;
    cameraP->far = far;
}

void MBCameraNearFarGet(float *near, float *far)
{
    MBCAMERA *cameraP = MBCameraGet();
    if(near) {
        *near = cameraP->near;
    }
    if(far) {
        *far = cameraP->far;
    }
}

void MBCameraMotionEnd(void)
{
    MBCAMERA *cameraP = MBCameraGet();
    MBCAMERAMOTION *motP = &cameraP->motion;
    motP->time = motP->maxTime+1;
    _ClearFlag(FLAG_BOARD_CAMERAMOT);
    if(motP->curveType == MB_CAMERA_CURVE_LINEAR) {
        cameraP->fov = motP->viewKey[1].fov;
        cameraP->zoom = motP->viewKey[1].zoom;
        cameraP->rot = motP->viewKey[1].rot;
        cameraP->target = motP->viewKey[1].pos;
    } else {
        cameraP->fov = motP->viewKey[2].fov;
        cameraP->zoom = motP->viewKey[2].zoom;
        cameraP->rot = motP->viewKey[2].rot;
        cameraP->target = motP->viewKey[2].pos;
    }
}

MBCAMERA *MBCameraGet(void)
{
    return &cameraWork;
}

void MBCameraPush(void)
{
    if(cameraSp < MB_CAMERA_STACK_MAX) {
        memcpy(&cameraStack[cameraSp++], &cameraWork, sizeof(MBCAMERA));
    }
}

void MBCameraPop(void)
{
    if(cameraSp > 0) {
        memcpy(&cameraWork, &cameraStack[--cameraSp], sizeof(MBCAMERA));
    }
}

void MBCameraMaxTimeSet(s16 maxTime)
{
    MBCAMERA *cameraP = MBCameraGet();
    MBCAMERAMOTION *motP = &cameraP->motion;
    if(cameraSp > 0) {
        float fov = cameraP->fov;
        float zoom = cameraP->zoom;
        HuVecF rot = cameraP->rot;
        HuVecF pos = cameraP->target;
        MBCameraPop();
        motP->viewKey[1].fov = cameraP->fov;
        motP->viewKey[1].zoom = cameraP->zoom;
        motP->viewKey[1].rot = cameraP->rot;
        motP->viewKey[1].pos = cameraP->target;
        motP->viewKey[0].fov = fov;
        motP->viewKey[0].zoom = zoom;
        motP->viewKey[0].rot = rot;
        motP->viewKey[0].pos = pos;
        motP->curveType = MB_CAMERA_CURVE_LINEAR;
        motP->time = 0;
        motP->maxTime = maxTime;
        _SetFlag(FLAG_BOARD_CAMERAMOT);
    }
}

int MBCameraViewNoGet(void)
{
    MBCAMERA *cameraP = MBCameraGet();
    MBCAMERAMOTION *motP = &cameraP->motion;
    return motP->viewNo;
}

void MBCameraViewNoSet(int playerNo, int viewNo)
{
    MBCAMERA *cameraP = MBCameraGet();
    MBCAMERAMOTION *motP = &cameraP->motion;
    static const HuVecF offset = { 0, 100, 0 };
    HuVecF rot;
    s16 speed;
    if(!cameraObj) {
        return;
    }
    motP->viewNo = viewNo;
    rot.x = viewTbl[viewNo].rotX;
    rot.y = rot.z = 0;
    speed = (cameraP->skipF) ? 21 : 1;
    MBCameraPlayerViewSet(playerNo, &rot, (HuVecF *)&offset, viewTbl[viewNo].zoom, viewTbl[viewNo].fov, speed);
}

void MBCameraSkipSet(BOOL skipF)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->skipF = skipF;
}

void MBCameraSpeedSet(float speed)
{
    MBCAMERA *cameraP = MBCameraGet();
    if(speed < 0.0f) {
        cameraP->speed = 0.15f;
    } else {
        cameraP->speed = speed;
    }
}

void MBCameraFocusPlayerAdd(int playerNo)
{
    MBCameraFocusAdd(MBPlayerModelGet(playerNo));
    MBCameraOffsetSet(0, 100, 0);
}

void MBCameraFocusPlayerSet(int playerNo)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->focusNum = 0;
    if(playerNo >= 0) {
        MBCameraFocusPlayerAdd(playerNo);
    }
}

void MBCameraFocusAdd(MBMODELID modelId)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->focus[cameraP->focusNum].type = MB_CAMERA_FOCUS_MODEL;
    cameraP->focus[cameraP->focusNum].targetId = modelId;
    cameraP->focusNum++;
    cameraP->offset.x = cameraP->offset.y = cameraP->offset.z = 0;
}

void MBCameraFocusSet(MBMODELID modelId)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->focusNum = 0;
    if(modelId >= 0) {
        MBCameraFocusAdd(modelId);
    }
}

void MBCameraFocusMasuAdd(int masuId)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->focus[cameraP->focusNum].type = MB_CAMERA_FOCUS_MASU;
    cameraP->focus[cameraP->focusNum].targetId = masuId;
    cameraP->focusNum++;
    cameraP->offset.x = cameraP->offset.y = cameraP->offset.z = 0;
}

void MBCameraFocusMasuSet(int masuId)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->focusNum = 0;
    if(masuId >= 0) {
        MBCameraFocusMasuAdd(masuId);
    }
}

//TODO: Wrong stack order in MBCameraPlayerViewSet and MBCameraModelViewSet (finalOfs must be allocated with high priority)

void MBCameraPosViewSet(HuVecF *pos, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime)
{
    MBCAMERA *cameraP = MBCameraGet();
    MBCAMERAMOTION *motP = &cameraP->motion;
    MBCAMERAVIEWKEY *key1 = &motP->viewKey[0];
    MBCAMERAVIEWKEY *key2 = &motP->viewKey[1];
    
    HuVecF finalOfs;
    HuVecF newRot;
    HuVecF newPos;
    
    if(cameraObj) {
        motP->curveType = MB_CAMERA_CURVE_LINEAR;
        key1->fov = cameraP->fov;
        key1->zoom = cameraP->zoom;
        key1->rot = cameraP->rot;
        key1->pos = cameraP->target;
        key2->fov = (fov < 0.0f) ? key1->fov : fov;
        key2->zoom = (zoom < 0.0f) ? key1->zoom : zoom;
        if(rot == 0) {
            newRot = key1->rot;
        } else {
            newRot = *rot;
        }
        key2->rot = newRot;
        if(pos == 0) {
            newPos = key1->pos;
        } else {
            newPos = *pos;
        }
        key2->pos = newPos;
        if(offset) {
            MBCameraOffsetSet(offset->x, offset->y, offset->z);
        }
        MBCameraOffsetGet(&finalOfs);
        VECAdd(&finalOfs, &key2->pos, &key2->pos);
        _SetFlag(FLAG_BOARD_CAMERAMOT);
        motP->time = 0;
        motP->maxTime = (maxTime < 0) ? 1 : maxTime;
    }
}

void MBCameraPlayerViewSet(s16 playerNo, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime)
{
    if(playerNo == -1) {
        MBCameraPosViewSet(NULL, rot, offset, zoom, fov, maxTime);
    } else {
        HuVecF pos;
        MBCameraFocusPlayerSet(playerNo);
        MBPlayerPosGet(playerNo, &pos);
        MBCameraPosViewSet(&pos, rot, offset, zoom, fov, maxTime);
    }
}

void MBCameraModelViewSet(MBMODELID modelId, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime)
{
    if(modelId == MB_MODEL_NONE) {
        MBCameraPosViewSet(NULL, rot, offset, zoom, fov, maxTime);
    } else {
        HuVecF pos;
        MBCameraFocusSet(modelId);
        MBModelPosGet(modelId, &pos);
        MBCameraPosViewSet(&pos, rot, offset, zoom, fov, maxTime);
    }
}

static inline void SetCameraKey(HuVecF *pos, MBCAMERAVIEWKEY *key1, MBCAMERAVIEWKEY *key2, s16 maxTime)
{
    MBCAMERA *cameraP = MBCameraGet();
    MBCAMERAMOTION *motP = &cameraP->motion;
    MBCAMERAVIEWKEY *keyStart = &motP->viewKey[0];
    MBCAMERAVIEWKEY *keyExtra;
    int i;
    if(!cameraObj) {
        return;
    }
    motP->curveType = MB_CAMERA_CURVE_BEZIER;
    keyStart->fov = cameraP->fov;
    keyStart->zoom = cameraP->zoom;
    keyStart->rot = cameraP->rot;
    keyStart->pos = cameraP->target;
    keyExtra = &motP->viewKey[1];
    for(i=0; i<2; i++, keyExtra++) {
        MBCAMERAVIEWKEY *key = (i == 0) ? key1 : key2;
        keyExtra->fov = (key->fov < 0) ? keyStart->fov : key->fov;
        keyExtra->zoom = (key->zoom < 0) ? keyStart->zoom : key->zoom;
        keyExtra->rot = key->rot;
        VECAdd(pos, &key->ofs, &keyExtra->pos);
        MBCameraOffsetSet(key->ofs.x, key->ofs.y, key->ofs.z);
    }
    _SetFlag(FLAG_BOARD_CAMERAMOT);
    motP->time = 0;
    motP->maxTime = (maxTime < 0) ? 1 : maxTime;
}

void MBCameraModelKeySet(MBMODELID modelId, MBCAMERAVIEWKEY *key1, MBCAMERAVIEWKEY *key2, s16 maxTime)
{
    if(modelId == MB_MODEL_NONE) {
        SetCameraKey(NULL, key1, key2, maxTime);
    } else {
        HuVecF pos;
        MBCameraFocusSet(modelId);
        MBModelPosGet(modelId, &pos);
        SetCameraKey(&pos, key1, key2, maxTime);
    }
}

void MBCameraMasuViewSet(s16 masuId, HuVecF *rot, HuVecF *offset, float zoom, float fov, s16 maxTime)
{
    if(masuId == MASU_NULL) {
        MBCameraPosViewSet(NULL, rot, offset, zoom, fov, maxTime);
    } else {
        HuVecF pos;
        MBCameraFocusMasuSet(masuId);
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
        MBCameraPosViewSet(&pos, rot, offset, zoom, fov, maxTime);
    }
}

void MBCameraQuakeSet(int maxTime, float power)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->quake = TRUE;
    cameraP->quakePower = power;
    cameraP->quakeTime = maxTime;
}

void MBCameraQuakeReset(void)
{
    MBCAMERA *cameraP = MBCameraGet();
    cameraP->quake = FALSE;
    cameraP->quakePower = 0;
    cameraP->quakeTime = 0;
}

BOOL MBCameraCullCheck(HuVecF *pos, float radius)
{
    MBCAMERA *cameraP = MBCameraGet();
    HuVecF dir;
    HuVecF dir2;
    float dot;
    float minCos;
    float dist;
    if(cameraP->dispOn == FALSE) {
        return FALSE;
    }
    MBCameraDirGet(&dir);
    VECSubtract(pos, &cameraP->pos, &dir2);
    dist = VECMag(&dir2);
    if(25000.0f < (dist-(radius*2.0f))) {
        return FALSE;
    }
    MBCameraPosDirGet(pos, &dir2);
    dot = VECDotProduct(&dir, &dir2);
    if(dot < 0) {
        minCos = -dot;
    } else {
        minCos = dot;
    }
    if(minCos < HuCos(cameraP->fov)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL MBCameraMotionCheck(void)
{
    return _CheckFlag(FLAG_BOARD_CAMERAMOT) ? FALSE : TRUE;
}

void MBCameraMotionWait(void)
{
    while(!MBCameraMotionCheck()) {
        HuPrcVSleep();
    }
    HuPrcVSleep();
}

void MBCameraViewGet(int viewNo, MBVIEW *viewP)
{
    memcpy(viewP, &viewTbl[viewNo], sizeof(MBVIEW));
}

void MBCameraLookAtGet(Mtx lookAt)
{
    MBCAMERA *cameraP = MBCameraGet();
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
}

void MBCameraLookAtGetInv(Mtx lookAtInv)
{
    Mtx temp;
    MBCameraLookAtGet(temp);
    MTXInverse(temp, lookAtInv);
}

void MBCameraFocusPlayerAddAll(void)
{
    MBCAMERA *cameraP = MBCameraGet();
    int comNum;
    MBCAMERAMOTION *motP;
    int comPlayer[GW_PLAYER_MAX];
    int i;
    motP = &cameraP->motion;
    comNum = MBPlayerAliveComGet(comPlayer);
    if(comNum == 0) {
        return;
    }
    if(comNum >= 2) {
        motP->viewNo = MB_CAMERA_VIEW_FOCUSALL;
        MBCameraFocusPlayerSet(-1);
        MBCameraRotSet(-33, 0, 0);
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                MBCameraFocusPlayerAdd(i);
            }
        }
    } else {
        MBCameraViewNoSet(comPlayer[0], MB_CAMERA_VIEW_WALK);
    }
}

//TODO: Fix parameter expansion inside MBCameraOffsetSet inline inside MBCameraPosViewSet inline
void MBCameraPointFocusSet(HuVecF *rot, HuVecF *offset, float fov, s16 maxTime)
{
    MBCAMERA *cameraP = MBCameraGet();
    int comNum;
    MBCAMERAMOTION *motP;
    int comPlayer[GW_PLAYER_MAX];
    HuVecF pos;
    int i;
    float zoom;
    motP = &cameraP->motion;
    comNum = MBPlayerAliveComGet(comPlayer);
    if(comNum == 0) {
        return;
    }
    if(comNum >= 2) {
        static const HuVecF viewOfs = {};
        motP->viewNo = MB_CAMERA_VIEW_FOCUSALL;
        MBCameraFocusPlayerSet(-1);
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                MBCameraFocusPlayerAdd(i);
            }
        }
        zoom = CameraTargetCalc(rot, fov, &pos);
        MBCameraPosViewSet(&pos, rot, (HuVecF *)&viewOfs, zoom, fov, maxTime);
        MBCameraOffsetSet(0, 100, 0);
    } else {
        MBCameraPlayerViewSet(comPlayer[0], rot, offset, viewTbl[2].zoom, fov, maxTime);
    }
}