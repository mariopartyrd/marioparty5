#include "game/hsfex.h"
#include "game/hu3d.h"
#include "game/disp.h"

#include "math.h"

#define DISP_HALF_W (HU_DISP_WIDTH/2.0f)
#define DISP_HALF_H (HU_DISP_HEIGHT/2.0f)

static void SetObjCamMotion(HU3DMODELID modelId, HSFTRACK *trackP, float curve, CAMMOTIONWORK *work);


void CamMotionEx2(HU3DMODELID modelId, s16 cameraBit, float maxTime, s16 mode)
{
    s16 i;
    HU3DCAMERA *cameraP;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if(cameraBit & (1 << i)) {
            break;
        }
    }
    cameraP = &Hu3DCamera[i];
    CamMotionEx(modelId, cameraBit, &cameraP->pos, &cameraP->up, &cameraP->target, maxTime, mode);
}

void CamMotionEx(HU3DMODELID modelId, s16 cameraBit, HuVecF *pos, HuVecF *up, HuVecF *target, float maxTime, s16 mode)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DMOTION *motP = &Hu3DMotion[modelP->motId];
    HSFDATA *hsf = modelP->hsf;
    HSFDATA  *motHsf = motP->hsf;
    HSFMOTION *hsfMotP = motHsf->motion;
    s16 j;
    s16 i;
    
    HSFTRACK *trackP;
    HSFTRACK *trackEnd;
    s16 cameraNo;
    HU3DCAMERA *cameraP;
    float time;
    float motTime;
    float motMaxTime;
    float dist;
    float distMax;
    HuVecF diff;
    float p2[4];
    float p1[4];
    s16 stepNum;
    CAMMOTIONWORK *workBegin;
    CAMMOTIONWORK *work;
    HSFOBJECT *objP;
    for(cameraNo=0; cameraNo<HU3D_CAM_MAX; cameraNo++) {
        if(cameraBit & (1 << cameraNo)) {
            break;
        }
    }
    cameraP = &Hu3DCamera[cameraNo];
    motMaxTime = hsfMotP->maxTime;
    stepNum = 1+(motMaxTime/6.0)+1;
    
    work = workBegin = HuMemDirectMallocNum(HEAP_HEAP, (stepNum+1)*sizeof(CAMMOTIONWORK), HU_MEMNUM_OVL);
    work->time = 0;
    work->pos = *pos;
    work->target = *target;
    work->up = *up;
    work++;
    for(i=1, time=0; time<=motMaxTime; time += 6, work++, i++) {
        work->time = time;
        trackP = hsfMotP->track;
        trackEnd = &trackP[hsfMotP->numTracks];
        if(!(modelP->attr & HU3D_ATTR_CAMERA)) {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                    objP = &hsf->object[trackP->target];
                    if(objP->type == HSF_OBJ_CAMERA) {
                        SetObjCamMotion(modelId, trackP, GetCurve(trackP, time), work);
                    }
                }
            }
        } else {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                   if(trackP->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, trackP, GetCurve(trackP, time), work);
                   }
                }
            }
        }
    }
    if(time != motMaxTime) {
        work->time = motMaxTime;
        trackP = hsfMotP->track;
        trackEnd = &trackP[hsfMotP->numTracks];
        if(!(modelP->attr & HU3D_ATTR_CAMERA)) {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                    objP = &hsf->object[trackP->target];
                    if(objP->type == HSF_OBJ_CAMERA) {
                        SetObjCamMotion(modelId, trackP, GetCurve(trackP, motMaxTime), work);
                    }
                }
            }
        } else {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                   if(trackP->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, trackP, GetCurve(trackP, motMaxTime), work);
                   }
                }
            }
        }
        i++;
    }
    workBegin[0].fov = workBegin[1].fov;
    work = workBegin;
    work[0].dist = 0;
    for(time=distMax=0; time<i-1; time++, work++) {
        VECSubtract(&work[1].pos, &work[0].pos, &diff);
        work[1].dist = VECMag(&diff);
        distMax += work[1].dist;
    }
    work = workBegin;
    for(time=dist=0; time<i; time++, work++) {
        dist += work->dist;
        work->time = maxTime*(dist/distMax);
    }
    for(time=0; time<=maxTime; time++) {
        switch(mode) {
            case 0:
                motTime = time;
                break;
           
            case 1:
                motTime = maxTime*HuSin((time/maxTime)*90);
                break;
           
            case 2:
                motTime = maxTime*(1-HuCos((time/maxTime)*90));
                break;
        }
        for(work=workBegin, j=0; j<i; j++, work++) {
            if(work->time <= motTime && work[1].time > motTime) {
                break;
            }

        }
        if(j == i) {
            break;
        }
        if(j == 0) {
            p1[0] = -1;
        } else {
            p1[0] = work[-1].time;
        }
        p1[1] = work[0].time;
        if(j >= i-1) {
            p1[2] = 1+work[0].time;
        }
        p1[2] = work[1].time;
        if(j >= i-2) {
            p1[3] = 1+p1[2];
        } else {
            p1[3] = work[2].time;
        }
        if(j == 0) {
            p2[0] = work[0].pos.x;
        } else {
            p2[0] = work[-1].pos.x;
        }
        p2[1] = work[0].pos.x;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].pos.x;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].pos.x;
        }
        cameraP->pos.x = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].pos.y;
        } else {
            p2[0] = work[-1].pos.y;
        }
        p2[1] = work[0].pos.y;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].pos.y;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].pos.y;
        }
        cameraP->pos.y = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].pos.z;
        } else {
            p2[0] = work[-1].pos.z;
        }
        p2[1] = work[0].pos.z;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].pos.z;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].pos.z;
        }
        cameraP->pos.z = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].target.x;
        } else {
            p2[0] = work[-1].target.x;
        }
        p2[1] = work[0].target.x;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].target.x;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].target.x;
        }
        cameraP->target.x = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].target.y;
        } else {
            p2[0] = work[-1].target.y;
        }
        p2[1] = work[0].target.y;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].target.y;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].target.y;
        }
        cameraP->target.y = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].target.z;
        } else {
            p2[0] = work[-1].target.z;
        }
        p2[1] = work[0].target.z;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].target.z;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].target.z;
        }
        cameraP->target.z = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].up.x;
        } else {
            p2[0] = work[-1].up.x;
        }
        p2[1] = work[0].up.x;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].up.x;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].up.x;
        }
        cameraP->up.x = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].up.y;
        } else {
            p2[0] = work[-1].up.y;
        }
        p2[1] = work[0].up.y;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].up.y;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].up.y;
        }
        cameraP->up.y = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].up.z;
        } else {
            p2[0] = work[-1].up.z;
        }
        p2[1] = work[0].up.z;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].up.z;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].up.z;
        }
        cameraP->up.z = InterpolateBMLine(p2, p1, motTime);
        if(j == 0) {
            p2[0] = work[0].fov;
        } else {
            p2[0] = work[-1].fov;
        }
        p2[1] = work[0].fov;
        if(j >= i-1) {
            p2[2] = p2[1];
        } else {
            p2[2] = work[1].fov;
        }
        if(j >= i-2) {
            p2[3] = p2[2];
        } else {
            p2[3] = work[2].fov;
        }
        cameraP->fov = InterpolateBMLine(p2, p1, motTime);
        HuPrcVSleep();
    }
    HuMemDirectFree(workBegin);
}

static void SetObjCamMotion(HU3DMODELID modelId, HSFTRACK *trackP, float curve, CAMMOTIONWORK *work)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    Vec up;
    Vec dir;

    switch (trackP->channel) {
        case HSF_CHANNEL_POSX:
            work->pos.x = modelP->scale.x * (curve + modelP->pos.x);
            break;
            
        case HSF_CHANNEL_POSY:
            work->pos.y = modelP->scale.y * (curve + modelP->pos.y);
            break;
            
        case HSF_CHANNEL_POSZ:
            work->pos.z = modelP->scale.z * (curve + modelP->pos.z);
            break;
            
        case HSF_CHANNEL_TARGETX:
            work->target.x = modelP->scale.x * (curve + modelP->pos.x);
            break;
            
        case HSF_CHANNEL_TARGETY:
            work->target.y = modelP->scale.y * (curve + modelP->pos.y);
            break;
            
        case HSF_CHANNEL_TARGETZ:
            work->target.z = modelP->scale.z * (curve + modelP->pos.z);
            break;
        
        case HSF_CHANNEL_FOV:
            work->fov = curve;
            break;
            
        case HSF_CHANNEL_UPROT:
            VECSubtract(&work->pos, &work->target, &dir);
            VECNormalize(&dir, &dir);
            up.x = dir.x * dir.y * (1.0 - HuCos(curve)) - dir.z * HuSin(curve);
            up.y = dir.y * dir.y + (1.0f - dir.y * dir.y) * HuCos(curve);
            up.z = dir.y * dir.z * (1.0 - HuCos(curve)) + dir.x * HuSin(curve);
            VECNormalize(&up, &work->up);
            break;
    }
}

int CamMotionExPathGet(HU3DMODELID modelId, s16 stepSpeed, float maxTime, s16 mode, CAMMOTIONWORK **workBuf, int *stepMax)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HU3DMOTION *motP = &Hu3DMotion[modelP->motId];
    HSFDATA *hsf = modelP->hsf;
    HSFDATA  *motHsf = motP->hsf;
    HSFMOTION *hsfMotP = motHsf->motion;
    
    HSFTRACK *trackP;
    HSFTRACK *trackEnd;
    HSFOBJECT *objP;
    CAMMOTIONWORK *work;
    s16 num;
    
    float time;
    float motMaxTime;
    
    float *distBufBegin;
    float *distBuf;
    float distMax;
    float dist;
    
    s16 stepNum;
    
    motMaxTime = hsfMotP->maxTime;
    stepNum = 1+(motMaxTime/stepSpeed)+1;
    if(!workBuf) {
        *workBuf = HuMemDirectMallocNum(HEAP_HEAP, (stepNum+1)*sizeof(CAMMOTIONWORK), HU_MEMNUM_OVL);
        *stepMax = stepNum+1;
    } else {
        if(*stepMax < stepNum+1) {
            return -1;
        }
    }
    for(work=*workBuf, num=0, time=0; time<=motMaxTime; time+=stepSpeed, work++, num++) {
        work->time = time;
        trackP = hsfMotP->track;
        trackEnd = &trackP[hsfMotP->numTracks];
        if(!(modelP->attr & HU3D_ATTR_CAMERA)) {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                    objP = &hsf->object[trackP->target];
                    if(objP->type == HSF_OBJ_CAMERA) {
                        SetObjCamMotion(modelId, trackP, GetCurve(trackP, time), work);
                    }
                }
            }
        } else {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                   if(trackP->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, trackP, GetCurve(trackP, time), work);
                   }
                }
            }
        }
    }
    if(time != motMaxTime) {
        work->time = motMaxTime;
        trackP = hsfMotP->track;
        trackEnd = &trackP[hsfMotP->numTracks];
        if(!(modelP->attr & HU3D_ATTR_CAMERA)) {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                    objP = &hsf->object[trackP->target];
                    if(objP->type == HSF_OBJ_CAMERA) {
                        SetObjCamMotion(modelId, trackP, GetCurve(trackP, motMaxTime), work);
                    }
                }
            }
        } else {
            for(; trackP<trackEnd; trackP++) {
                if(trackP->type == HSF_TRACK_TRANSFORM) {
                   if(trackP->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, trackP, GetCurve(trackP, motMaxTime), work);
                   }
                }
            }
        }
        num++;
    }
    distBufBegin = HuMemDirectMallocNum(HEAP_HEAP, num*4, HU_MEMNUM_OVL);
    work = *workBuf;
    distBuf = distBufBegin;
    distBuf[0] = 0;
    for(distMax=0, time=0; time<num-1; time++, work++, distBuf++) {
        HuVecF temp;
        VECSubtract(&work[1].pos, &work[0].pos, &temp);
        distBuf[1] = VECMag(&temp);
        distMax += distBuf[1];
    }
    if(fabsf(distMax) > 0.0001) {
        work = *workBuf;
        distBuf = distBufBegin;
        for(dist=0, time=0; time<num; time++, work++, distBuf++) {
            dist += distBuf[0];
            work->time = maxTime*(dist/distMax);
            switch(mode) {
                case 0:
                    break;
                    
                case 1:
                    work->time = maxTime*HuSin((work->time/maxTime)*90);
                    break;
               
                case 2:
                    work->time = maxTime*(1-HuCos((work->time/maxTime)*90));
                    break;
            }
        }
    }
    HuMemDirectFree(distBufBegin);
    return num;
}

void CamMotionExPath(s16 cameraBit, CAMMOTIONWORK *workBuf, int num)
{
    CAMMOTIONWORK *work;
    s16 i, j;
    HU3DCAMERA *cameraP;
    float time;
    float timeEnd;
    
    float *ptrOut[10];
    float *ptrIn[10];
    float p2[4];
    float p1[4];
    
    for(i=0; i<HU3D_CAM_MAX; i++) {
        if(cameraBit & (1 << i)) {
            cameraP = &Hu3DCamera[i];
            break;
        }
    }
    timeEnd = workBuf[num-1].time;
    ptrOut[0] = &cameraP->fov;
    ptrOut[1] = &cameraP->pos.x;
    ptrOut[2] = &cameraP->pos.y;
    ptrOut[3] = &cameraP->pos.z;
    ptrOut[4] = &cameraP->up.x;
    ptrOut[5] = &cameraP->up.y;
    ptrOut[6] = &cameraP->up.z;
    ptrOut[7] = &cameraP->target.x;
    ptrOut[8] = &cameraP->target.y;
    ptrOut[9] = &cameraP->target.z;
    for(time=0; time<=timeEnd; time++) {
        work = workBuf;
        for(i=0; i<num; i++, work++) {
            if(work[0].time <= time && work[1].time > time) {
                break;
            }
        }
        if(i == num) {
            break;
        }
        ptrIn[0] = &work->fov;
        ptrIn[1] = &work->pos.x;
        ptrIn[2] = &work->pos.y;
        ptrIn[3] = &work->pos.z;
        ptrIn[4] = &work->up.x;
        ptrIn[5] = &work->up.y;
        ptrIn[6] = &work->up.z;
        ptrIn[7] = &work->target.x;
        ptrIn[8] = &work->target.y;
        ptrIn[9] = &work->target.z;
        if(i == 0) {
            p1[0] = -1;
        } else {
            p1[0] = work[-1].time;
        }
        p1[1] = work[0].time;
        if(i >= num-1) {
            p1[2] = 1+p1[1];
        } else {
            p1[2] = work[1].time;
        }
        if(i >= num-2) {
            p1[3] = 1+p1[2];
        } else {
            p1[3] = work[2].time;
        }
        for(j=10; j-- != 0;) {
            if(i == 0) {
                p2[0] = *ptrIn[j];
            } else {
                p2[0] = *(float *)((u8 *)ptrIn[j]-sizeof(CAMMOTIONWORK));
            }
            p2[1] = *ptrIn[j];
            if(i >= num-1) {
                p2[2] = p2[1];
            } else {
                p2[2] = *(float *)((u8 *)ptrIn[j]+(1*sizeof(CAMMOTIONWORK)));
            }
            if(i >= num-2) {
                p2[3] = p2[2];
            } else {
                p2[3] = *(float *)((u8 *)ptrIn[j]+(2*sizeof(CAMMOTIONWORK)));
            }
            *ptrOut[j] = InterpolateBMLine(p2, p1, time);
        }
        HuPrcVSleep();
    }
}

float InterpolateBMLine(float *p1, float *p2, float time)
{
    float diff[3];
    float out[2];
    float temp_f22;
    float var_f21;
    float temp_f20;
    float temp_f29;
    float temp_f31;
    float var_f28;
    float var_f27;
    float var_f26;
    float var_f25;
    float var_f24;
    float var_f23;
    float var_f30;
    s32 var_r30;
    s32 var_r29;
    s32 i;

    if (p1[0] == p1[1] && p1[0] == p1[2] && p1[0] == p1[3]) {
        return p1[0];
    }
    diff[0] = (p2[0] != p2[1]) ? (p2[1]-p2[0]) : 0.001;
    diff[1] = (p2[1] != p2[2]) ? (p2[2]-p2[1]) : 0.001;
    diff[2] = (p2[2] != p2[3]) ? (p2[3]-p2[2]) : 0.001;
    for(i=1; i<=2; i++) {
        out[i-1] = 0.5f * ((p1[i] - p1[i - 1]) / diff[i-1] + (p1[i + 1] - p1[i]) / diff[i]);
    }
    temp_f29 = 0.5f * (p2[2] + p2[1]);
    var_r30 = 0;
    if (out[1] - out[0] != 0.0f) {
        temp_f22 = (out[1] * p2[2] - out[0] * p2[1] - (p1[2] - p1[1])) / (out[1] - out[0]);
        var_r30 = (p2[1] <= temp_f22 && temp_f22 <= p2[2]) ? TRUE : FALSE;
    }
    if (var_r30 == 1) {
        temp_f31 = temp_f29 - p2[1];
        temp_f20 = (p1[2] - p1[1]) / diff[1] - (out[1] - out[0]) / 2;
        var_f28 = p1[1] + ((temp_f31 * (temp_f31 * ((out[1]-out[0]) / (2.0f * diff[1])))) + (temp_f20 * temp_f31));
        var_f27 = temp_f20 + (temp_f31 * ((out[1]-out[0]) / diff[1]));
    } else {
        temp_f31 = temp_f29 - p2[1];
        var_f28 = (p1[2] + p1[1]) * (temp_f31 / diff[1]);
        var_f27 = ((2.0f * (p1[2]-p1[1])) / diff[1]) - ((out[1]+out[0]) * (temp_f31 / diff[1]));
    }
    if (time < temp_f29) {
        var_f30 = p2[1];
        var_f26 = p1[1];
        var_f25 = out[0];
        var_f24 = temp_f29;
        var_f21 = var_f28;
        var_f23 = var_f27;
    } else {
        var_f30 = temp_f29;
        var_f26 = var_f28;
        var_f25 = var_f27;
        var_f24 = p2[2];
        var_f21 = p1[2];
        var_f23 = out[1];
    }
    return ((var_f23 - var_f25) / (2.0f * (var_f24 - var_f30))) * (time - var_f30) * (time - var_f30) + (time - var_f30) * ((var_f21 - var_f26) / (var_f24 - var_f30) - (var_f23 - var_f25) / 2) + var_f26;
}

void Hu3D2Dto3D(HuVecF *src, s16 cameraBit, HuVecF *dst)
{
    HU3DCAMERA *cameraP;
    float height;
    float tanFov;
    float width;
    float posX;
    float posY;
    s16 i;
    Mtx matrix;

    for (i = 0; i < HU3D_CAM_MAX; i++) {
        if (cameraBit & (1 << i)) {
            break;
        }
    }
    cameraP = &Hu3DCamera[i];
    tanFov = HuSin(cameraP->fov/2)/HuCos(cameraP->fov/2);
    height = tanFov * src->z * 2.0f;
    width = height * HU_DISP_ASPECT;
    posX = src->x / HU_DISP_WIDTH;
    posY = src->y / HU_DISP_HEIGHT;
    dst->x = (posX-0.5)*width;
    dst->y = -(posY-0.5)*height;
    dst->z = -src->z;
    MTXLookAt(matrix, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXInverse(matrix, matrix);
    MTXMultVec(matrix, dst, dst);
}

void Hu3D3Dto2D(HuVecF *src, s16 cameraBit, HuVecF *dst)
{
    Vec pos;
    HU3DCAMERA *cameraP;
    float width;
    float height;
    s16 i;
    Mtx lookAt;

    for (i = 0; i < HU3D_CAM_MAX; i++) {
        if (cameraBit & (1 << i)) {
            break;
        }
    }
    cameraP = &Hu3DCamera[i];
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXMultVec(lookAt, src, &pos);
    width = (HuSin(cameraP->fov/2) / HuCos(cameraP->fov/2)) * pos.z * HU_DISP_ASPECT;
    height = (HuSin(cameraP->fov/2) / HuCos(cameraP->fov/2)) * pos.z;
    dst->x = DISP_HALF_W + pos.x * (DISP_HALF_W / -width);
    dst->y = DISP_HALF_H + pos.y * (DISP_HALF_H / height);
    dst->z = 0.0f;
}

void Hu3DMtxTransGet(Mtx src, HuVecF *dst)
{
    dst->x = src[0][3];
    dst->y = src[1][3];
    dst->z = src[2][3];
}

static inline float GetAngleXY(float x, float y) {
    if (y == 0.0f) {
        if (x >= 0.0f) {
            return M_PI / 2;
        } else {
            return -(M_PI / 2);
        }
    } else {
        return atan2f(x, y);
    }
}

void Hu3DMtxRotGet(Mtx src, HuVecF *dst)
{
    float cosY;
    float scaleX;
    float scaleY;
    float scaleZ;
    float scaleX2;
    float scaleY2;
    float scaleZ2;
    float angleY;
    float dot;

    scaleX2 = src[0][0] * src[0][0] + src[1][0] * src[1][0] + src[2][0] * src[2][0];
    scaleX = sqrtf(scaleX2);
    if (!(scaleX < 0.00000001f)) {
        scaleY2 = src[0][1] * src[0][1] + src[1][1] * src[1][1] + src[2][1] * src[2][1];
        scaleY = sqrtf(scaleY2);
        if (!(scaleY < 0.00000001f)) {
            scaleZ2 = src[0][2] * src[0][2] + src[1][2] * src[1][2] + src[2][2] * src[2][2];
            scaleZ = sqrtf(scaleZ2);
            if (!(scaleZ < 0.00000001f)) {
                dot = -src[2][0] / scaleX;
                if (dot >= 1.0f) {
                    angleY = M_PI / 2;
                } else if (dot <= -1.0f) {
                    angleY = -(M_PI / 2);
                } else {
                    angleY = asinf(dot);
                }
                dst->y = angleY;
                cosY = cos(dst->y);
                if (cosY >= 0.00000001f) {
                    dst->x = GetAngleXY(src[2][1] / scaleY, src[2][2] / scaleZ);
                    dst->z = GetAngleXY(src[1][0], src[0][0]);
                } else {
                    dst->x = GetAngleXY(src[0][1], src[1][1]);
                    dst->z = 0.0f;
                }
                dst->x = MTXRadToDeg(dst->x);
                dst->y = MTXRadToDeg(dst->y);
                dst->z = MTXRadToDeg(dst->z);
                return;
            }
        }
    }
    dst->x = 0.0f;
    dst->y = 0.0f;
    dst->z = 0.0f;
}

void Hu3DMtxScaleGet(Mtx src, HuVecF *dst)
{
    Vec dot;
    Vec vecX;
    Vec vecY;
    Vec vecZ;
    Vec scaleVec;

    vecX.x = src[0][0];
    vecX.y = src[1][0];
    vecX.z = src[2][0];
    dst->x = VECMag(&vecX);
    VECNormalize(&vecX, &vecX);
    vecY.x = src[0][1];
    vecY.y = src[1][1];
    vecY.z = src[2][1];
    dot.x = VECDotProduct(&vecX, &vecY);
    VECScale(&vecX, &scaleVec, dot.x);
    VECSubtract(&vecY, &scaleVec, &vecY);
    dst->y = VECMag(&vecY);
    VECNormalize(&vecY, &vecY);
    dot.x /= dst->y;
    vecZ.x = src[0][2];
    vecZ.y = src[1][2];
    vecZ.z = src[2][2];
    dot.z = VECDotProduct(&vecY, &vecZ);
    VECScale(&vecY, &scaleVec, dot.z);
    VECSubtract(&vecZ, &scaleVec, &vecZ);
    dot.y = VECDotProduct(&vecX, &vecZ);
    VECScale(&vecX, &scaleVec, dot.y);
    VECSubtract(&vecZ, &scaleVec, &vecZ);
    dst->z = VECMag(&vecZ);
    VECNormalize(&vecZ, &vecZ);
    VECCrossProduct(&vecY, &vecZ, &scaleVec);
    if (VECDotProduct(&vecX, &scaleVec) < 0.0) {
        dst->x *= -1.0;
        dst->y *= -1.0;
        dst->z *= -1.0;
    }

}