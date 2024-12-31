#include "game/object.h"
#include "game/pad.h"
#include "game/audio.h"
#include "game/sprite.h"
#include "game/mgdata.h"
#include "game/gamemes.h"

float CZoomM[HU3D_CAM_MAX];
HuVecF CenterM[HU3D_CAM_MAX];
HuVecF CRotM[HU3D_CAM_MAX];
float CZoom;
HuVecF Center;
HuVecF CRot;

s32 lbl_80288888;

void omOutView(OMOBJ *obj)
{
    Vec pos;
    Vec target;
    Vec up;
    Vec rotDir;
    Vec dir;
    float rotX = CRot.x;
    float rotY = CRot.y;
    float rotZ = CRot.z;
    pos.x = Center.x+(CZoom*(HuSin(rotY)*HuCos(rotX)));
    pos.y = Center.y+(CZoom*-HuSin(rotX));
    pos.z = Center.z+(CZoom*(HuCos(rotY)*HuCos(rotX)));
    target.x = Center.x;
    target.y = Center.y;
    target.z = Center.z;
    rotDir.x = HuSin(rotY)*HuSin(rotX);
    rotDir.y = HuCos(rotX);
    rotDir.z = HuCos(rotY)*HuSin(rotX);
    VECSubtract(&pos, &target, &dir);
    VECNormalize(&dir, &dir);
    up.x = (rotDir.x*((dir.x*dir.x)+((1.0f-(dir.x * dir.x))*HuCos(rotZ))))
        + (rotDir.y*(((dir.x*dir.y)*(1.0-HuCos(rotZ)))-(dir.z*HuSin(rotZ))))
        + (rotDir.z*(((dir.x*dir.z)*(1.0-HuCos(rotZ)))+(dir.y*HuSin(rotZ))));
    up.y = (rotDir.y*((dir.y*dir.y)+((1.0f-(dir.y * dir.y))*HuCos(rotZ))))
        + (rotDir.x*(((dir.x*dir.y)*(1.0-HuCos(rotZ)))+(dir.z*HuSin(rotZ))))
        + (rotDir.z*(((dir.y*dir.z)*(1.0-HuCos(rotZ)))-(dir.x*HuSin(rotZ))));
    up.z = (rotDir.z*((dir.z * dir.z)+((1.0f-(dir.z * dir.z))*HuCos(rotZ))))
        + ((rotDir.x*(((dir.x * dir.z)*(1.0-HuCos(rotZ)))-(dir.y*HuSin(rotZ))))
        + (rotDir.y*(((dir.y * dir.z)*(1.0-HuCos(rotZ)))+(dir.x*HuSin(rotZ)))));
    VECNormalize(&up, &up);
    Hu3DCameraPosSet(HU3D_CAM0, pos.x, pos.y, pos.z, up.x, up.y, up.z, target.x, target.y, target.z);
}

void omOutViewMulti(OMOBJ *obj)
{
    u8 i;
    for(i=0; i<obj->work[0]; i++) {
        Vec pos;
        Vec target;
        Vec up;
        Vec rotDir;
        Vec dir;
        float rotX = CRotM[i].x;
        float rotY = CRotM[i].y;
        float rotZ = CRotM[i].z;
        pos.x = CenterM[i].x+(CZoomM[i]*(HuSin(rotY)*HuCos(rotX)));
        pos.y = CenterM[i].y+(CZoomM[i]*-HuSin(rotX));
        pos.z = CenterM[i].z+(CZoomM[i]*(HuCos(rotY)*HuCos(rotX)));
        target.x = CenterM[i].x;
        target.y = CenterM[i].y;
        target.z = CenterM[i].z;
        up.x = HuSin(rotY) * HuSin(rotX);
        up.y = HuCos(rotX);
        up.z = HuCos(rotY)*HuSin(rotX);
        rotDir.x = HuSin(rotY)*HuSin(rotX);
        rotDir.y = HuCos(rotX);
        rotDir.z = HuCos(rotY)*HuSin(rotX);
        VECSubtract(&pos, &target, &dir);
        VECNormalize(&dir, &dir);
        up.x = (rotDir.x*((dir.x*dir.x)+((1.0f-(dir.x*dir.x))*HuCos(rotZ))))
            + (rotDir.y*(((dir.x*dir.y)*(1.0-HuCos(rotZ)))-(dir.z*HuSin(rotZ))))
            + (rotDir.z*(((dir.x*dir.z)*(1.0-HuCos(rotZ)))+(dir.y*HuSin(rotZ))));
        up.y = (rotDir.y*((dir.y*dir.y)+((1.0f-(dir.y*dir.y))*HuCos(rotZ))))
            + (rotDir.x*(((dir.x*dir.y)*(1.0-HuCos(rotZ)))+(dir.z*HuSin(rotZ))))
            + (rotDir.z*(((dir.y*dir.z)*(1.0-HuCos(rotZ)))-(dir.x*HuSin(rotZ))));
        up.z = (rotDir.z*((dir.z * dir.z)+((1.0f-(dir.z*dir.z))*HuCos(rotZ))))
            + ((rotDir.x*(((dir.x * dir.z)*(1.0-HuCos(rotZ)))-(dir.y*HuSin(rotZ))))
            + (rotDir.y*(((dir.y * dir.z)*(1.0-HuCos(rotZ)))+(dir.x*HuSin(rotZ)))));
        VECNormalize(&up, &up);
        Hu3DCameraPosSetV((1 << i), &pos, &up, &target);
    }
}

#define SYSKEY_PAD_NONE 0xFFFF
#define SYSKEY_ATTR_PAUSEON (1 << 0)
#define SYSKEY_ATTR_PAUSE (1 << 7)
#define SYSKEY_ATTR_UPAUSE (1 << 8)
#define SYSKEY_ATTR_PAUSEKEY (1 << 9)

void omDBGSystemKeyCheckSetup(OMOBJMAN *objman)
{
    
}

void omSystemKeyCheckSetup(OMOBJMAN *objman)
{
    OMOBJ *sysKeyObj = omAddObj(objman, 32731, 0, 0, omSystemKeyCheck);
    omDBGSysKeyObj = sysKeyObj;
    omSetStatBit(sysKeyObj, OM_STAT_NOPAUSE|OM_STAT_SPRPAUSE);
    sysKeyObj->work[0] = 0;
    sysKeyObj->work[1] = 0;
    sysKeyObj->work[2] = 0;
}

void omSystemKeyCheck(OMOBJ *obj)
{
    if(!omSysPauseEnableFlag) {
        return;
    }
    if(obj->work[0] & SYSKEY_ATTR_PAUSEON) {
        u32 padNo = obj->work[1];
        if(padNo != SYSKEY_PAD_NONE) {
            if(!HuPadStatGet(padNo) && (HuPadBtnDown[padNo] & PAD_BUTTON_START)) {
                obj->work[0] |= SYSKEY_ATTR_PAUSEKEY;
            }
        }
        if(obj->work[0] & SYSKEY_ATTR_PAUSEKEY) {
            if(MgNoGet(omcurovl) != DLL_NONE) {
                GMesPauseCancel();
            } else {
                obj->work[0] |= SYSKEY_ATTR_UPAUSE;
            }
        }
        if(obj->work[0] & SYSKEY_ATTR_UPAUSE) {
            obj->work[0] &= ~(SYSKEY_ATTR_UPAUSE|SYSKEY_ATTR_PAUSEKEY|SYSKEY_ATTR_PAUSEON);
            omAllPause(FALSE);
            HuPrcAllPause(FALSE);
            Hu3DPauseSet(FALSE);
            HuSprPauseSet(FALSE);
            HuAudFXPauseAll(FALSE);
            HuAudSeqPauseAll(FALSE);
            HuAudSStreamPauseAll(FALSE);
        }
    } else {
        s16 pauseF = FALSE;
        s32 padNo;
        if(WipeCheckEnd() || omCurrentOvlGet() == DLL_NONE || omSysExitReq) {
            return;
        }
        if(!WipeCheck()) {
            for(padNo=0; padNo<4; padNo++) {
                if(!HuPadStatGet(padNo) && (HuPadBtnDown[padNo] & PAD_BUTTON_START)) {
                    pauseF = TRUE;
                    break;
                }
            }
        }
        if(obj->work[0] & SYSKEY_ATTR_PAUSE) {
            obj->work[0] &= ~SYSKEY_ATTR_PAUSE;
            pauseF = TRUE;
            padNo = SYSKEY_PAD_NONE;
        }
        if(pauseF) {
            obj->work[0] |= SYSKEY_ATTR_PAUSEON;
            obj->work[1] = padNo;
            omAllPause(TRUE);
            HuPrcAllPause(TRUE);
            Hu3DPauseSet(TRUE);
            HuSprPauseSet(TRUE);
            HuAudFXPauseAll(TRUE);
            HuAudSeqPauseAll(TRUE);
            HuAudSStreamPauseAll(TRUE);
            if(MgNoGet(omcurovl) != DLL_NONE) {
                GMesPauseCreate();
            }
            HuPadRumbleAllStop();
        }
    }
}

void omSysPauseEnable(u8 flag)
{
    omSysPauseEnableFlag = flag;
}

void omSysPauseCtrl(s16 flag)
{
    if(!omDBGSysKeyObj) {
        return;
    }
    omDBGSysKeyObj->work[0] &= ~(SYSKEY_ATTR_PAUSE|SYSKEY_ATTR_UPAUSE);
    if(flag) {
        omDBGSysKeyObj->work[0] |= SYSKEY_ATTR_PAUSE;
    } else {
        omDBGSysKeyObj->work[0] |= SYSKEY_ATTR_UPAUSE;
    }
}

#define CAMERA_ATTR_NONE 0
#define CAMERA_ATTR_SINGLE (1 << 0)

typedef struct CameraViewWork_s {
    s16 no;
    s16 attr;
    s16 moveType;
    HuVecF centerEnd;
    HuVecF rotEnd;
    float zoomEnd;
    u32 maxTime;
    HUPROCESS *process;
} CAMERAVIEWWORK;

static CAMERAVIEWWORK cameraViewWork[HU3D_CAM_MAX];

static void CameraMoveProc(void);

void omCameraViewInit(void)
{
    s16 i;
    for(i=0; i<HU3D_CAM_MAX; i++) {
        cameraViewWork[i].no = -1;
    }
}

void omCameraViewSetMulti(s16 cameraBit, OMCAMERAVIEW *cameraView)
{
    s16 i;
    s16 bit;
    for(i=0, bit=HU3D_CAM0; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            CRotM[i] = cameraView->rot;
            CenterM[i] = cameraView->center;
            CZoomM[i] = cameraView->zoom;
            if(cameraViewWork[i].no != -1) {
                HuPrcKill(cameraViewWork[i].process);
            }
        }
    }
}

void omCameraViewSet(OMCAMERAVIEW *cameraView)
{
    CRot = cameraView->rot;
    Center = cameraView->center;
    CZoom = cameraView->zoom;
    if(cameraViewWork[0].no != -1) {
        HuPrcKill(cameraViewWork[0].process);
    }
}

s16 omCameraViewMoveMulti(u32 camera, OMCAMERAVIEW *cameraView, s32 time, s16 moveType)
{
    s16 cameraBit;
    s16 bit;
    s16 cameraNum;
    s16 i;
    if(camera == OM_CAMERA_SINGLE) {
        cameraBit = HU3D_CAM0;
    } else {
        cameraBit = camera;
    }
    for(i=cameraNum=0, bit=HU3D_CAM0; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            CAMERAVIEWWORK *viewWork = &cameraViewWork[i];
            HUPROCESS *process;
            if(cameraViewWork[i].no != -1) {
                HuPrcKill(viewWork->process);
            }
            process = HuPrcChildCreate(CameraMoveProc, 100, 4096, 0, HuPrcCurrentGet());
            process->property = (void *)i;
            viewWork->process = process;
            viewWork->no = i;
            if(camera == OM_CAMERA_SINGLE) {
                viewWork->attr = CAMERA_ATTR_SINGLE;
            } else {
                viewWork->attr = CAMERA_ATTR_NONE;
            }
            viewWork->centerEnd = cameraView->center;
            viewWork->rotEnd = cameraView->rot;
            viewWork->zoomEnd = cameraView->zoom;
            viewWork->maxTime = time;
            viewWork->moveType = moveType;
            cameraNum++;
        }
    }
    return cameraNum;
}

s16 omCameraViewMove(OMCAMERAVIEW *cameraView, s32 time, s16 moveType)
{
    return omCameraViewMoveMulti(OM_CAMERA_SINGLE, cameraView, time, moveType);
}

s16 omCameraViewMoveSimpleMulti(u32 camera, OMCAMERAVIEW *cameraView, s32 time)
{
    return omCameraViewMoveMulti(camera, cameraView, time, OM_CAMERAMOVE_SIMPLE);
}

s16 omCameraViewMoveSimple(OMCAMERAVIEW *cameraView, s32 time)
{
    return omCameraViewMove(cameraView, time, OM_CAMERAMOVE_SIMPLE);
}

BOOL omCameraViewCheck(u32 cameraBit)
{
    s16 i;
    s16 bit;
    s16 cameraNum;
    for(i=cameraNum=0, bit=HU3D_CAM0; i<HU3D_CAM_MAX; i++, bit <<= 1) {
        if(cameraBit & bit) {
            if(cameraViewWork[i].no != -1) {
                cameraNum++;
            }
        }
    }
    if(cameraNum != 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}


static void CameraMoveProc(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    s16 no = (s16)process->property;
    CAMERAVIEWWORK *viewWork = &cameraViewWork[no];
    u32 i;
    float angle;
    float weight;
    float zoomOrig;
    float zoomNew;
    Vec centerOrig;
    Vec rotOrig;
    Vec centerNew;
    Vec rotNew;
    
    Vec angleDiff;
    
    if(viewWork->attr & CAMERA_ATTR_SINGLE) {
        rotOrig = CRot;
        centerOrig = Center;
        zoomOrig = CZoom;
    } else {
        rotOrig = CRotM[no];
        centerOrig = CenterM[no];
        zoomOrig = CZoomM[no];
    }
    angle = viewWork->rotEnd.x-rotOrig.x;
    if(HuAbs(angle) > 180.0f) {
        if(viewWork->rotEnd.x < rotOrig.x) {
            angleDiff.x = 360+angle;
        } else {
            angleDiff.x = angle-360;
        }
    } else {
        angleDiff.x = angle;
    }
    angle = viewWork->rotEnd.y-rotOrig.y;
    if(HuAbs(angle) > 180.0f) {
        if(viewWork->rotEnd.y < rotOrig.y) {
            angleDiff.y = 360+angle;
        } else {
            angleDiff.y = angle-360;
        }
    } else {
        angleDiff.y = angle;
    }
    angle = viewWork->rotEnd.z-rotOrig.z;
    if(HuAbs(angle) > 180.0f) {
        if(viewWork->rotEnd.z < rotOrig.z) {
            angleDiff.z = 360+angle;
        } else {
            angleDiff.z = angle-360;
        }
    } else {
        angleDiff.z = angle;
    }
    for(i=1; i<=viewWork->maxTime; i++) {
        if(viewWork->moveType == OM_CAMERAMOVE_SIMPLE) {
            angle = HuSin(90.0f*((float)i/(float)viewWork->maxTime));
            weight = HuSin(90.0f*angle);
        } else if(viewWork->moveType == OM_CAMERAMOVE_COS) {
            weight = 1-HuCos(90.0f*((float)i/(float)viewWork->maxTime));
        } else if(viewWork->moveType == OM_CAMERAMOVE_LINEAR) {
            weight = (float)i/(float)viewWork->maxTime;
        } else if(viewWork->maxTime/2 > i) {
            angle = (float)i/(float)(viewWork->maxTime/2.0);
            weight = 0.5*(1-HuCos(90.0f*angle));
        } else {
            angle = (float)(i-(viewWork->maxTime/2))/(float)(viewWork->maxTime/2.0);
            weight = 0.5+(0.5*HuSin(90.0*HuSin(90.0f*angle)));
        }
        zoomNew = zoomOrig+(weight*(viewWork->zoomEnd-zoomOrig));
        rotNew.x = rotOrig.x+(weight*angleDiff.x);
        rotNew.y = rotOrig.y+(weight*angleDiff.y);
        rotNew.z = rotOrig.z+(weight*angleDiff.z);
        centerNew.x = centerOrig.x+(weight*(viewWork->centerEnd.x-centerOrig.x));
        centerNew.y = centerOrig.y+(weight*(viewWork->centerEnd.y-centerOrig.y));
        centerNew.z = centerOrig.z+(weight*(viewWork->centerEnd.z-centerOrig.z));
        if(viewWork->attr & CAMERA_ATTR_SINGLE) {
            CZoom = zoomNew;
            CRot = rotNew;
            Center = centerNew;
        } else {
            CZoomM[no] = zoomNew;
            CRotM[no] = rotNew;
            CenterM[no] = centerNew;
        }
        HuPrcVSleep();
    }
    viewWork->no = -1;
    HuPrcEnd();
}