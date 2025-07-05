#include "game/object.h"
#include "game/hu3d.h"
#include "game/disp.h"
#include "game/sprite.h"
#include "game/audio.h"
#include "game/main.h"
#include "game/window.h"
#include "game/wipe.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/charman.h"
#include "game/pad.h"
#include "game/thpmain.h"
#include "game/init.h"

#include "datanum/e3boot.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void ObjectSetup(void);

int _prolog(void) {
    const VoidFunc* ctors = _ctors;
    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

void _epilog(void) {
    const VoidFunc* dtors = _dtors;
    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

#define HU_PAD_BTN_ALL (HuPadBtn[0] | HuPadBtn[1] | HuPadBtn[2] | HuPadBtn[3])
#define HU_PAD_BTNDOWN_ALL (HuPadBtnDown[0] | HuPadBtnDown[1] | HuPadBtnDown[2] | HuPadBtnDown[3])
#define HU_PAD_DSTK_ALL (HuPadDStkRep[0] | HuPadDStkRep[1] | HuPadDStkRep[2] | HuPadDStkRep[3])

static OMOBJ *outViewObj;
OMOBJMAN *objman;
Vec cameraRot[2];
Vec cameraCenter[2];
float cameraZoom[2];
static Vec lbl_1_bss_74;
static float titleStartScale;
static float titleStartScale2;
s16 lbl_1_bss_66[3];
HUSPRGRPID titleSprGrpId;
static HU3DMODELID titleCharMdlId[8];
static HU3DMOTID titleCharMotId[8][3];
static HU3DMODELID titleStarMdlId[14];
static s16 titleDoneF;
static s16 lbl_1_bss_4;

void BootExec(void);
void BootInterlaceSet(void);
void CameraOutView(OMOBJ *obj);
void CameraMove(OMOBJ *obj);
void TitleCreate(void);
BOOL TitleExec(void);
void *NintendoDataDecode(void);

void ObjectSetup(void)
{
    OMOVLHIS *his;
    int lightId;
    OSReport("******* E3Boot ObjectSetup *********\n");
    objman = omInitObjMan(50, 512);
    cameraRot[0].x = 0;
    cameraRot[0].y = 0;
    cameraRot[0].z = 0;
    cameraCenter[0].x = 0;
    cameraCenter[0].y = 140;
    cameraCenter[0].z = 0;
    cameraZoom[0] = 2600;
    lightId = Hu3DGLightCreate(-1000, 1500, 1000, 1, -1.5, -1, 160, 160, 160);
    Hu3DGLightInfinitytSet(lightId);
    lightId = Hu3DGLightCreate(0, 0, 1000, 0, 0, -1, 128, 128, 128);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 10, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    HuPrcCreate(BootExec, 100, 12288, 0);
    outViewObj = omAddObjEx(objman, 32730, 0, 0, -1, CameraOutView);
    Hu3DBGColorSet(0, 0, 0);
    his = omOvlHisGet(0);
    omOvlHisChg(0, his->ovl, 1, his->stat);
}

void BootExec(void)
{
    s16 i;
    HUSPRGRPID gid;
    ANIMDATA *anim;
    OSTick tickPrev;
    HUSPRID sprId;
    HUSPRGRPID movieGrpId;
    BOOL movieEndF;
    void *sampBuf;
    HUSPRID nintendoSprId;
    HUSPRID movieSprId;
    
    HUPROCESS *process = HuPrcCurrentGet();
    
    if(omovlevtno == 0) {
        BootInterlaceSet();
    }
    gid = HuSprGrpCreate(2);
    anim = HuSprAnimRead(NintendoDataDecode());
    nintendoSprId = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(gid, 0, nintendoSprId);
    HuSprPosSet(gid, 0, 288, 240);
    HuSprDispOff(gid, 0);
    if(omovlevtno != 0) {
        HuAudSndGrpSetSet(MSM_GRP_INIT);
        anim = HuSprAnimDataRead(E3BOOT_ANM_hudson);
        sprId = HuSprCreate(anim, 0, 0);
        HuSprGrpMemberSet(gid, 1, sprId);
        HuSprPosSet(gid, 1, 288, 240);
        HuSprDispOff(gid, 1);
        HuWinInit(1);
        TitleCreate();
        NintendoDispF = TRUE;
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            GwPlayerConf[i].padNo = i;
        }
        NintendoDispF = FALSE;
        repeatLogo:
        HuSprDispOn(gid, 0);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        if(!NintendoDispF) {
            tickPrev = OSGetTick();
            CharInit();
            HuWindowInit();
            GMesInit();
            HuWinInit(1);
            TitleCreate();
            anim = HuSprAnimDataRead(E3BOOT_ANM_hudson);
            sprId = HuSprCreate(anim, 0, 0);
            HuSprGrpMemberSet(gid, 1, sprId);
            HuSprPosSet(gid, 1, 288, 240);
            HuSprDispOff(gid, 1);
            while(OSTicksToMilliseconds(OSGetTick()-tickPrev) < 3000) {
                HuPrcVSleep();
            }
        } else {
            for(i=0; i<180; i++) {
                if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                    break;
                }
                HuPrcVSleep();
            }
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 30);
        WipeWait();
        HuSprDispOff(gid, 0);
        HuSprDispOn(gid, 1);
        HuPrcSleep(60);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_WHITEFADE, 30);
        WipeWait();
        if(!NintendoDispF) {
            tickPrev = OSGetTick();
            sampBuf = HuMemDirectMalloc(HEAP_MODEL, msmSysGetSampSize(MSM_GRP_INIT));
            msmSysLoadGroup(MSM_GRP_INIT, sampBuf, FALSE);
            HuMemDirectFree(sampBuf);
            while(OSTicksToMilliseconds(OSGetTick()-tickPrev) < 3000) {
                HuPrcVSleep();
            }
        } else {
            for(i=0; i<180; i++) {
                if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                    break;
                }
                HuPrcVSleep();
            }
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 30);
        WipeWait();
        tickPrev = OSGetTick();
        if(!NintendoDispF) {
            HuAudSndGrpSetSet(MSM_GRP_INIT);
            NintendoDispF = TRUE;
        }
        while(OSTicksToMilliseconds(OSGetTick()-tickPrev) < 1000) {
            HuPrcVSleep();
        }
        HuSprDispOff(gid, 0);
        HuSprDispOff(gid, 1);
    }
    while(1) {
        if(TitleExec() != TRUE) {
            HuPrcSleep(60);
            movieGrpId = HuSprGrpCreate(1);
            movieSprId = HuTHPSprCreateVol("movie/e3demo.thp", 0, 100, 122);
            HuSprGrpMemberSet(movieGrpId, 0, movieSprId);
            HuSprPosSet(movieGrpId, 0, 288, 240);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
            WipeWait();
            movieEndF = FALSE;
            while(!HuTHPEndCheck()) {
                if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                    movieEndF = TRUE;
                    break;
                }
                HuPrcVSleep();
            }
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
            WipeWait();
            HuTHPClose();
            HuPrcVSleep();
            HuSprGrpKill(movieGrpId);
            if(!movieEndF) {
                HuPrcSleep(60);
                movieEndF = FALSE;
                goto repeatLogo;
            }
        } else {
            WipeColorSet(255, 255, 255);
            HuAudSStreamAllFadeOut(1000);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            HuPrcVSleep();
            if(omSysExitReq) {
                omOvlReturn(1);
            } else {
                omOvlCall(DLL_e3setupdll, 0, 0);
            }
            for(i=0; i<GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].charNo = CHARNO_NONE;
            }
            HuPrcEnd();
            while(1) {
                HuPrcVSleep();
            }
        }
    }
}

void BootInterlaceSet(void)
{
    s16 choice = 0;
    s16 i;
    if(OSGetResetCode() != 0) {
        return;
    }
    if(VIGetDTVStatus() != VI_NON_INTERLACE || (HU_PAD_BTN_ALL & PAD_BUTTON_B)) {
        OSReport("interless\n");
        OSSetProgressiveMode(OS_PROGRESSIVE_MODE_OFF);
        VIConfigure(&GXNtsc480IntDf);
        RenderMode = &GXNtsc480IntDf;
        GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE, RenderMode->vfilter);
        VIFlush();
        HuPrcSleep(60);
        for(i=0; i<8;) {
            return;
        }
    }
    OSSetProgressiveMode(OS_PROGRESSIVE_MODE_ON);
    VIConfigure(&GXNtsc480Prog);
    RenderMode = &GXNtsc480IntDf;
    GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE, RenderMode->vfilter);
    VIFlush();
    HuPrcSleep(60);
}

void CameraOutView(OMOBJ *obj)
{
    static u16 cameraMask[] = { HU3D_CAM0, HU3D_CAM1 };
    s16 i;
    for(i=0; i<1; i++) {
        Vec pos, target, up;
        float x, y, z;

        x = cameraRot[i].x;
        y = cameraRot[i].y;
        z = cameraRot[i].z;

        pos.x = (((HuSin(y) * HuCos(x)) * cameraZoom[i]) + cameraCenter[i].x);
        pos.y = (-HuSin(x) * cameraZoom[i]) + cameraCenter[i].y;
        pos.z = ((HuCos(y) * HuCos(x)) * cameraZoom[i]) + cameraCenter[i].z;
        target.x = cameraCenter[i].x;
        target.y = cameraCenter[i].y;
        target.z = cameraCenter[i].z;
        up.x = HuSin(y) * HuSin(x);
        up.y = HuCos(x);
        up.z = HuCos(y) * HuSin(x);
        Hu3DCameraPosSet(cameraMask[i], pos.x, pos.y, pos.z, up.x, up.y, up.z, target.x, target.y, target.z);
    }
}

void CameraMove(OMOBJ *obj)
{
    Vec pos;
    Vec offset;
    Vec dir;
    Vec yOfs;

    f32 rotZ;
    s8 stickPos;
    static BOOL moveF = FALSE;
    
    if ((HuPadBtnDown[0] & PAD_BUTTON_Y)) {
        moveF = (moveF) ? 0 : 1;
    }
    if(moveF) {
        cameraRot[0].y += 0.1f * HuPadStkX[0];
        cameraRot[0].x += 0.1f * HuPadStkY[0];
        cameraZoom[0] += HuPadTrigL[0] / 2;
        cameraZoom[0] -= HuPadTrigR[0] / 2;
        if (cameraZoom[0] < 100.0f) {
            cameraZoom[0] = 100.0f;
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_B) {
            OSReport("rot %f,%f,%f\n", cameraRot[0].x, cameraRot[0].y, cameraRot[0].z);
            OSReport("center %f,%f,%f\n", cameraCenter[0].x, cameraCenter[0].y, cameraCenter[0].z);
            OSReport("zoom %f\n", cameraZoom[0]);
        }
        pos.x = cameraCenter[0].x + (cameraZoom[0] * (HuSin(cameraRot[0].y) * HuCos(cameraRot[0].x)));
        pos.y = (cameraCenter[0].y + (cameraZoom[0] * -HuSin(cameraRot[0].x)));
        pos.z = (cameraCenter[0].z + (cameraZoom[0] * (HuCos(cameraRot[0].y) * HuCos(cameraRot[0].x))));
        offset.x = cameraCenter[0].x - pos.x;
        offset.y = cameraCenter[0].y - pos.y;
        offset.z = cameraCenter[0].z - pos.z;
        dir.x = (HuSin(cameraRot[0].y) * HuSin(cameraRot[0].x));
        dir.y = HuCos(cameraRot[0].x);
        dir.z = (HuCos(cameraRot[0].y) * HuSin(cameraRot[0].x));
        rotZ = cameraRot[0].z;
        yOfs.x = dir.x * (offset.x * offset.x + (1.0f - offset.x * offset.x) * HuCos(rotZ))
            + dir.y * (offset.x * offset.y * (1.0f - HuCos(rotZ)) - offset.z * HuSin(rotZ))
            + dir.z * (offset.x * offset.z * (1.0f - HuCos(rotZ)) + offset.y * HuSin(rotZ));

        yOfs.y = dir.y * (offset.y * offset.y + (1.0f - offset.y * offset.y) * HuCos(rotZ))
            + dir.x * (offset.x * offset.y * (1.0f - HuCos(rotZ)) + offset.z * HuSin(rotZ))
            + dir.z * (offset.y * offset.z * (1.0f - HuCos(rotZ)) - offset.x * HuSin(rotZ));

        yOfs.z = dir.z * (offset.z * offset.z + (1.0f - offset.z * offset.z) * HuCos(rotZ))
            + (dir.x * (offset.x * offset.z * (1.0 - HuCos(rotZ)) - offset.y * HuSin(rotZ))
                + dir.y * (offset.y * offset.z * (1.0 - HuCos(rotZ)) + offset.x * HuSin(rotZ)));

        VECCrossProduct(&dir, &offset, &offset);
        VECNormalize(&offset, &offset);
        stickPos = (HuPadSubStkX[0] & 0xF8);
        if (stickPos != 0) {
            cameraCenter[0].x += 0.05f * (offset.x * stickPos);
            cameraCenter[0].y += 0.05f * (offset.y * stickPos);
            cameraCenter[0].z += 0.05f * (offset.z * stickPos);
        }
        VECNormalize(&yOfs, &offset);
        stickPos = -(HuPadSubStkY[0] & 0xF8);
        if (stickPos != 0) {
            cameraCenter[0].x += 0.05f * (offset.x * stickPos);
            cameraCenter[0].y += 0.05f * (offset.y * stickPos);
            cameraCenter[0].z += 0.05f * (offset.z * stickPos);
        }
    }
}

static void PressStartUpdate(void);

void TitleCreate(void)
{
    s16 i;
    
    HUSPRID sprId;
    ANIMDATA *anim;
    HU3DPROJID projId;
    
    HuVecF pos, target, up; 
    HuSprExecLayerCameraSet(64, HU3D_CAM0, HU3D_LAYER_HOOK_POST);
    titleSprGrpId = HuSprGrpCreate(6);
    anim = HuSprAnimDataRead(E3BOOT_ANM_back);
    sprId = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(titleSprGrpId, 0, sprId);
    HuSprDispOff(titleSprGrpId, 0);
    HuSprDrawNoSet(titleSprGrpId, 0, 127);
    HuSprPosSet(titleSprGrpId, 0, 288, 240);
    anim = HuSprAnimDataRead(E3BOOT_ANM_copyright);
    sprId = HuSprCreate(anim, 1, 0);
    HuSprGrpMemberSet(titleSprGrpId, 1, sprId);
    HuSprDispOff(titleSprGrpId, 1);
    HuSprPosSet(titleSprGrpId, 1, 288, 420);
    anim = HuSprAnimDataRead(E3BOOT_ANM_pressstart);
    sprId = HuSprCreate(anim, 2, 0);
    HuSprGrpMemberSet(titleSprGrpId, 2, sprId);
    HuSprAttrSet(titleSprGrpId, 2, HUSPR_ATTR_DISPOFF|HUSPR_ATTR_LINEAR);
    HuSprPosSet(titleSprGrpId, 2, 288, 340);
    anim = HuSprAnimDataRead(E3BOOT_ANM_logo);
    sprId = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(titleSprGrpId, 3, sprId);
    HuSprAttrSet(titleSprGrpId, 3, HUSPR_ATTR_DISPOFF|HUSPR_ATTR_LINEAR);
    HuSprPosSet(titleSprGrpId, 3, 288, 120);
    HuSprDrawNoSet(titleSprGrpId, 3, 64);
    anim = HuSprAnimDataRead(E3BOOT_ANM_btnmg);
    sprId = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(titleSprGrpId, 4, sprId);
    HuSprAttrSet(titleSprGrpId, 4, HUSPR_ATTR_DISPOFF|HUSPR_ATTR_LINEAR);
    anim = HuSprAnimDataRead(E3BOOT_ANM_btnmovie);
    sprId = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(titleSprGrpId, 5, sprId);
    HuSprAttrSet(titleSprGrpId, 5, HUSPR_ATTR_DISPOFF|HUSPR_ATTR_LINEAR);
    HuPrcChildCreate(PressStartUpdate, 2000, 4096, 0, HuPrcCurrentGet());
    projId = Hu3DProjectionCreate(HuSprAnimDataRead(E3BOOT_ANM_fade), 1, 100, 10000);
    pos.x = 0;
    pos.y = 50;
    pos.z = 5000;
    target.x = target.z = 0;
    target.y = 50;
    up.x = 0;
    up.z = 0;
    up.y = 1;
    Hu3DProjectionPosSet(projId, &pos, &up, &target);
    Hu3DProjectionTPLvlSet(projId, 1);
    for(i=0; i<8; i++) {
        titleCharMdlId[i] = Hu3DModelCreateData(E3BOOT_HSF_mario+(i*4));
        titleCharMotId[i][0] = Hu3DJointMotionData(titleCharMdlId[i], E3BOOT_HSF_marioMot1+(i*4));
        titleCharMotId[i][1] = Hu3DJointMotionData(titleCharMdlId[i], E3BOOT_HSF_marioMot2+(i*4));
        titleCharMotId[i][2] = Hu3DJointMotionData(titleCharMdlId[i], E3BOOT_HSF_marioMot3+(i*4));
        Hu3DMotionSet(titleCharMdlId[i], titleCharMotId[i][0]);
        Hu3DModelAttrSet(titleCharMdlId[i], HU3D_MOTATTR_LOOP);
        Hu3DModelDispOff(titleCharMdlId[i]);
        Hu3DModelProjectionSet(titleCharMdlId[i], projId);
        Hu3DModelLayerSet(titleCharMdlId[i], 1);
    }
    for(i=0; i<7; i++) {
        titleStarMdlId[i] = Hu3DModelCreateData(E3BOOT_HSF_starChorl+i);
        Hu3DModelScaleSet(titleStarMdlId[i], 0.3f, 0.3f, 0.3f);
        Hu3DModelAttrSet(titleStarMdlId[i], HU3D_MOTATTR_LOOP);
        Hu3DModelDispOff(titleStarMdlId[i]);
        Hu3DModelProjectionSet(titleStarMdlId[i], projId);
    }
    for(i=7; i<14; i++) {
        titleStarMdlId[i] = Hu3DModelLink(titleStarMdlId[i-7]);
        Hu3DModelScaleSet(titleStarMdlId[i], 0.4f, 0.4f, 0.4f);
        Hu3DModelAttrSet(titleStarMdlId[i], HU3D_MOTATTR_LOOP);
        Hu3DModelDispOff(titleStarMdlId[i]);
        Hu3DModelProjectionSet(titleStarMdlId[i], projId);
        Hu3DModelLayerSet(titleStarMdlId[i], 1);
    }
}

void TitleMdlExec(void);

BOOL TitleExec(void)
{
    s16 i;
    s16 j;
    s16 k;
    s16 choice;
    s16 result;
    HUPROCESS *mdlProc;
    float scale;
    float temp;
    
    float btnScale[2];
    float btnAngle[2];
    float btnSpeed[2];
    
    HuSprDispOn(titleSprGrpId, 0);
    HuSprDispOn(titleSprGrpId, 1);
    titleDoneF = FALSE;
    mdlProc = HuPrcChildCreate(TitleMdlExec, 2000, 12288, 0, HuPrcCurrentGet());
    HuAudSStreamPlay(MSM_STREAM_TITLE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 30);
    WipeWait();
    HuSprDispOn(titleSprGrpId, 3);
    for(i=1; i<=50; i++) {
        scale = (10*HuCos(i*1.8))+1;
        HuSprScaleSet(titleSprGrpId, 3, scale, scale);
        HuSprTPLvlSet(titleSprGrpId, 3, i/50.0);
        HuPrcVSleep();
    }
    repeat:
    HuSprDispOn(titleSprGrpId, 2);
    HuSprTPLvlSet(titleSprGrpId, 2, 1);
    result = 0;
    for(i=temp=0; i<1800; i++) {
        if(i <= 10) {
            HuSprTPLvlSet(titleSprGrpId, 2, i/10.0);
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_START) {
            HuAudFXPlay(MSM_SE_CMN_01);
            HuSprDispOff(titleSprGrpId, 2);
            break;
        }
        HuSprScaleSet(titleSprGrpId, 2, titleStartScale, titleStartScale);
        HuPrcVSleep();
    }
    if(i != 1800) {
        HuSprDispOn(titleSprGrpId, 4);
        HuSprDispOn(titleSprGrpId, 5);
        HuSprScaleSet(titleSprGrpId, 4, 1, 1);
        HuSprScaleSet(titleSprGrpId, 5, 0.7f, 0.7f);
        choice = 0;
        for(i=0; i<=20; i++) {
            if(i < 10) {
                scale = i/10.0;
                HuSprTPLvlSet(titleSprGrpId, 2, 1.0-scale);
            }
            if(i == 10) {
                HuSprDispOff(titleSprGrpId, 2);
            }
            scale = i/20.0;
            HuSprTPLvlSet(titleSprGrpId, 4, scale);
            HuSprTPLvlSet(titleSprGrpId, 5, scale);
            HuSprPosSet(titleSprGrpId, 4, 288, 340-(30*scale));
            HuSprPosSet(titleSprGrpId, 5, 288, 340+(30*scale));
            HuPrcVSleep();
        }
        btnScale[0] = btnScale[1] = 1;
        btnAngle[0] = btnAngle[1] = 90;
        btnSpeed[0] = 3;
        btnSpeed[1] = 1;
        for(i=0; i<1800; i++) {
            if(HuPadDStkRep[0] & (PAD_BUTTON_UP|PAD_BUTTON_DOWN)) {
                HuAudFXPlay(MSM_SE_CMN_01);
                choice ^= 1;
                if(choice != 0) {
                    for(j=0; j<=10; j++) {
                        scale = j/10.0;
                        btnSpeed[0] = 1;
                        btnSpeed[1] = 3;
                        for(k=0; k<2; k++) {
                            btnAngle[k] += btnSpeed[k];
                            if(btnAngle[k] > 360) {
                                btnAngle[k] -= 360;
                            }
                            btnScale[k] = (0.05*HuSin(btnAngle[k]))+0.95;
                        }
                        HuSprScaleSet(titleSprGrpId, 4, (1-(scale*0.3))*btnScale[0], (1-(scale*0.3))*btnScale[0]);
                        HuSprScaleSet(titleSprGrpId, 5, (0.7+(scale*0.3))*btnScale[1], (0.7+(scale*0.3))*btnScale[1]);
                        HuPrcVSleep();
                    }
                } else {
                    for(j=0; j<=10; j++) {
                        scale = j/10.0;
                        btnSpeed[0] = 3;
                        btnSpeed[1] = 1;
                        for(k=0; k<2; k++) {
                            btnAngle[k] += btnSpeed[k];
                            if(btnAngle[k] > 360) {
                                btnAngle[k] -= 360;
                            }
                            btnScale[k] = (0.05*HuSin(btnAngle[k]))+0.95;
                        }
                        HuSprScaleSet(titleSprGrpId, 4, (0.7+(scale*0.3))*btnScale[0], (0.7+(scale*0.3))*btnScale[0]);
                        HuSprScaleSet(titleSprGrpId, 5, (1-(scale*0.3))*btnScale[1], (1-(scale*0.3))*btnScale[1]);
                        HuPrcVSleep();
                    }
                }
                i=0;
            }
            if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                HuAudFXPlay(MSM_SE_CMN_01);
                titleDoneF = TRUE;
                for(j=0; j<=10; j++) {
                    if(choice != 0) {
                        HuSprTPLvlSet(titleSprGrpId, 4, 1-(j/10.0));
                    } else {
                        HuSprTPLvlSet(titleSprGrpId, 5, 1-(j/10.0));
                    }
                    HuPrcVSleep();
                }
                for(j=0; j<=10; j++) {
                    if(choice != 0) {
                        HuSprTPLvlSet(titleSprGrpId, 5, 1-(j/10.0));
                    } else {
                        HuSprTPLvlSet(titleSprGrpId, 4, 1-(j/10.0));
                    }
                    HuPrcVSleep();
                }
                if(choice == 1) {
                    result = 2;
                    goto end;
                } else {
                    return 1;
                }
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_01);
                for(j=0; j<=10; j++) {
                    HuSprTPLvlSet(titleSprGrpId, 4, 1-(j/10.0));
                    HuSprTPLvlSet(titleSprGrpId, 5, 1-(j/10.0));
                    HuPrcVSleep();
                }
                goto repeat;
            } else {
                for(k=0; k<2; k++) {
                    btnAngle[k] += btnSpeed[k];
                    if(btnAngle[k] > 360) {
                        btnAngle[k] -= 360;
                    }
                    btnScale[k] = (0.05*HuSin(btnAngle[k]))+0.95;
                }
                if(choice) {
                    HuSprScaleSet(titleSprGrpId, 4, 0.7*btnScale[0], 0.7*btnScale[0]);
                    HuSprScaleSet(titleSprGrpId, 5, btnScale[1], btnScale[1]);
                } else {
                    HuSprScaleSet(titleSprGrpId, 4, btnScale[0], btnScale[0]);
                    HuSprScaleSet(titleSprGrpId, 5, 0.7*btnScale[1], 0.7*btnScale[1]);
                }
            }
            HuPrcVSleep();
        }
        for(j=0; j<=10; j++) {
            HuSprTPLvlSet(titleSprGrpId, 4, 1-(j/10.0));
            HuSprTPLvlSet(titleSprGrpId, 5, 1-(j/10.0));
            HuPrcVSleep();
        }
    }
    end:
    HuAudSStreamAllFadeOut(500);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    WipeWait();
    HuSprDispOff(titleSprGrpId, 0);
    HuSprDispOff(titleSprGrpId, 1);
    HuSprDispOff(titleSprGrpId, 2);
    HuSprDispOff(titleSprGrpId, 3);
    for(i=0; i<8; i++) {
        Hu3DModelDispOff(titleCharMdlId[i]);
    }
    for(i=0; i<14; i++) {
        Hu3DModelDispOff(titleStarMdlId[i]);
    }
    HuPrcKill(mdlProc);
    return result;
}

static void PressStartUpdate(void)
{
    float angle1 = 0;
    float angle2 = 0;
    while(1) {
        titleStartScale = (0.1*HuSin(angle1))+0.9;
        titleStartScale2 = (0.1*HuSin(angle2))+0.9;
        angle1 += 2;
        if(angle1 > 360) {
            angle1 -= 360;
        }
        angle2 += 1;
        if(angle2 > 360) {
            angle2 -= 360;
        }
        HuPrcVSleep();
    }
}

static void TitleStarMove(HuVecF *pos, s16 mode, s16 angle);

void TitleMdlExec(void)
{
    s16 i;
    s16 k;
    s16 j;
    s16 time;
    HU3DMODEL *modelP;
    s16 delay;
    
    float diff;
    
    HuVecF starPos[14];
    s16 starMode[14];
    s16 starAngle[14];
    s16 charAngle[8];
    s16 charMode[8];
    
    HuVecF charPos;
    time = 0;
    for(i=0; i<8; i++) {
        charAngle[i] = (int)frandmod(20)+(i*45);
        charMode[i] = 0;
        Hu3DMotionSet(titleCharMdlId[i], titleCharMotId[i][0]);
        Hu3DModelAttrSet(titleCharMdlId[i], HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<7; i++) {
        starPos[i].y = frandmod(500)-300;
        starPos[i].z = ((i*10)*((i < 7) ? -1 : 1))-300;
        for(j=0; j<10; j++) {
            starPos[i].x = frandmod(500)-250;
            for(k=0; k<7; k++) {
                if(k != i) { 
                    diff = starPos[i].x-starPos[k].x;
                    if((diff*diff) < 2500) {
                        break;
                    }
                }
            }
            if(k == 7) {
                break;
            }
        }
        starMode[i] = 0;
        starAngle[i] = 0;
        Hu3DModelPosSetV(titleStarMdlId[i], &starPos[i]);
    }
    for(i=7; i<14; i++) {
        starPos[i].y = frandmod(500)-300;
        starPos[i].z = ((i*10)*((i < 7) ? -1 : 1))-300;
        for(j=0; j<10; j++) {
            starPos[i].x = frandmod(500)-250;
            for(k=0; k<7; k++) {
                if((k+7) != i) { 
                    diff = starPos[i].x-starPos[k+7].x;
                    if((diff*diff) < 2500) {
                        break;
                    }
                }
            }
            if(k == 7) {
                break;
            }
        }
        starMode[i] = 0;
        starAngle[i] = 0;
        Hu3DModelPosSetV(titleStarMdlId[i], &starPos[i]);
    }
    delay = 0;
    while(1) {
        for(i=0; i<7; i++) {
            TitleStarMove(&starPos[i], starMode[i], starAngle[i]);
            Hu3DModelPosSetV(titleStarMdlId[i], &starPos[i]);
            starAngle[i]++;
            if(starPos[i].y > 420) {
                for(j=0; j<10; j++) {
                    starPos[i].x = frandmod(500)-250;
                    for(k=0; k<7; k++) {
                        if(k != i) {
                            diff = starPos[i].x-starPos[k].x;
                            if((diff*diff) < 2500) {
                                break;
                            }
                        }
                    }
                    if(k == 7) {
                        break;
                    }
                }
                starMode[i] = frandmod(5);
                starPos[i].y = -50;
                starAngle[i] = 0;
                Hu3DModelDispOn(titleStarMdlId[i]);
            }
        }
        for(i=7; i<14; i++) {
            TitleStarMove(&starPos[i], starMode[i], starAngle[i]);
            Hu3DModelPosSetV(titleStarMdlId[i], &starPos[i]);
            starAngle[i]++;
            if(starPos[i].y > 420) {
                for(j=0; j<10; j++) {
                    starPos[i].x = frandmod(500)-250;
                    for(k=0; k<7; k++) {
                        if((k+7) != i) {
                            diff = starPos[i].x-starPos[k+7].x;
                            if((diff*diff) < 2500) {
                                break;
                            }
                        }
                    }
                    if(k == 7) {
                        break;
                    }
                }
                starMode[i] = frandmod(5);
                starPos[i].y = -50;
                starAngle[i] = 0;
                Hu3DModelDispOn(titleStarMdlId[i]);
            }
        }
        if(delay > 60) {
            if(!titleDoneF) {
                for(i=0; i<8; i++) {
                    charPos.x = HuSin(charAngle[i])*400;
                    charPos.z = HuCos(charAngle[i])*100;
                    Hu3DModelRotSet(titleCharMdlId[i], 0, charAngle[i]+90, 0);
                    charPos.y = 0;
                    Hu3DModelPosSetV(titleCharMdlId[i], &charPos);
                    charAngle[i]++;
                    if(charAngle[i] > 360) {
                        charAngle[i] -= 360;
                    }
                    if(Hu3DMotionIDGet(titleCharMdlId[i]) != titleCharMotId[i][0] && Hu3DMotionIDGet(titleCharMdlId[i]) != titleCharMotId[i][2]) {
                        Hu3DMotionSet(titleCharMdlId[i], titleCharMotId[i][0]);
                        Hu3DModelAttrSet(titleCharMdlId[i], HU3D_MOTATTR_LOOP);
                    }
                    
                    if((charAngle[i] > 85 && charAngle[i] < 95) || (charAngle[i] > 265 && charAngle[i] < 275)) {
                        Hu3DModelDispOn(titleCharMdlId[i]);
                    }
                    modelP =  &Hu3DData[titleCharMdlId[i]];
                    if(charAngle[i] > 270 && charAngle[i] < 280 && charMode[i] == 0 && frandmod(30) == 0) {
                        charMode[i] = 1;
                        modelP->motId = titleCharMotId[i][2];
                    }
                    if(charAngle[i] > 80 && charAngle[i] < 90 && charMode[i]) {
                        charMode[i] = 0;
                        modelP->motId = titleCharMotId[i][0];
                    }
                }
                time = 0;
            } else {
                if(time == 0) {
                    for(i=0; i<8; i++) {
                        Hu3DMotionShiftSet(titleCharMdlId[i], titleCharMotId[i][1], 0, 8, HU3D_MOTATTR_NONE);
                    }
                }
                if(time <= 10) {
                    for(i=0; i<8; i++) {
                        float ofsAngle = charAngle[i]+90;
                        float angle;
                        if(ofsAngle > 180) {
                            ofsAngle -= 360;
                        }
                        angle = ofsAngle*(1-(time/10.0));
                        Hu3DModelRotSet(titleCharMdlId[i], 0, angle, 0);
                    }
                }
                time++;
            }
        }
        HuPrcVSleep();
        delay++;
    }
}

static void TitleStarMove(HuVecF *pos, s16 mode, s16 angle)
{
    switch(mode) {
        case 0:
            pos->y += 3;
            break;
        
        case 1:
            pos->y = (450*(1-HuCos((angle/60.0)*90.0)))-50;
            break;
        
        case 2:
            pos->y += 2;
            break;
        
        case 3:
            pos->y += 5;
            break;
        
        case 4:
            pos->y += 3;
            pos->x += HuSin(angle*5);
            break;
    }
}

extern u32 nintendoData[];

void *NintendoDataDecode(void)
{
    u32 *src = nintendoData;
    u32 size = *src++;
    void *dst = HuMemDirectMalloc(HEAP_MODEL, size);
    u32 type = *src++;
    if(dst) {
        HuDecodeData(src, dst, size, type);
    }
    return dst;
}