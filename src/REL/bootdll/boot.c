#include "game/object.h"
#include "game/hu3d.h"
#include "game/disp.h"
#include "game/frand.h"
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

#include "datanum/title.h"

#include "relprolog.inc"

#define HU_PAD_BTN_ALL (HuPadBtn[0] | HuPadBtn[1] | HuPadBtn[2] | HuPadBtn[3])
#define HU_PAD_BTNDOWN_ALL (HuPadBtnDown[0] | HuPadBtnDown[1] | HuPadBtnDown[2] | HuPadBtnDown[3])
#define HU_PAD_DSTK_ALL (HuPadDStkRep[0] | HuPadDStkRep[1] | HuPadDStkRep[2] | HuPadDStkRep[3])


#define MOVIE_MAX 6

static char *movieTbl[MOVIE_MAX] = {
    "movie/st1.thp",
    "movie/st2.thp",
    "movie/st3.thp",
    "movie/st4.thp",
    "movie/st5.thp",
    "movie/st6.thp"
};

static u8 movieVolTbl[MOVIE_MAX] = {
    93,
    90,
    103,
    99,
    99,
    106
};

static OMOBJ *outViewObj;
OMOBJMAN *objman;
Vec cameraRot[2];
Vec cameraCenter[2];
float cameraZoom[2];
static Vec lbl_1_bss_24;
static s16 movieNo;
static s16 movieOrderTbl[MOVIE_MAX];
static HU3DMODELID titleMdlId[4];
static HU3DMOTID titleMotId[2];
static BOOL lbl_1_bss_4;

void BootExec(void);
void BootProgExec(void);
void BootTitleCreate(void);
BOOL BootTitleExec(BOOL fromTitle);
void *NintendoDataDecode(void);

void MovieShuffle(s16 *list, s16 num);

void ObjectSetup(void)
{
    int i;
    OMOVLHIS *his;
    OSReport("******* Boot ObjectSetup *********\n");
    objman = omInitObjMan(50, 8192);
    cameraRot[0].x = -67;
    cameraRot[0].y = 40;
    cameraRot[0].z = 0;
    cameraCenter[0].x = 260;
    cameraCenter[0].y = -103;
    cameraCenter[0].z = -18;
    cameraZoom[0] = 2885;
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 30, 20, 15000, HU_DISP_ASPECT);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
    HuPrcCreate(BootExec, 100, 12288, 0);
    Hu3DBGColorSet(0, 0, 0);
    his = omOvlHisGet(0);
    omOvlHisChg(0, his->ovl, 1, his->stat);
    for(i=0; i<MOVIE_MAX; i++) {
        movieOrderTbl[i] = i;
    }
    MovieShuffle(movieOrderTbl, MOVIE_MAX);
    movieNo = 0;
}

void BootExec(void)
{
    HUPROCESS *parent = HuPrcCurrentGet();
    HUSPRID hudsonSprId;
    HUSPRID dolbySprId;
    HUSPRID nintendoSprId;
    BOOL fromTitle;
    HUSPRGRPID gid;
    ANIMDATA *anim;
    OSTick tickPrev;
    
    s16 i;
    s16 statId;
    fromTitle = FALSE;
    if(omovlevtno == 0 && VIGetTvFormat() != VI_MPAL) {
        BootProgExec();
    }
    gid = HuSprGrpCreate(3);
    anim = HuSprAnimRead(NintendoDataDecode());
    nintendoSprId = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(gid, 0, nintendoSprId);
    HuSprPosSet(gid, 0, HU_DISP_CENTERX, HU_DISP_CENTERY);
    HuSprDispOff(gid, 0);
    if(omovlevtno != 0) {
        HuAudSndGrpSetSet(MSM_GRP_MENU);
        anim = HuSprAnimDataRead(TITLE_ANM_hudson);
        hudsonSprId = HuSprCreate(anim, 0, 0);
        HuSprGrpMemberSet(gid, 1, hudsonSprId);
        HuSprPosSet(gid, 1, HU_DISP_CENTERX, HU_DISP_CENTERY);
        HuSprDispOff(gid, 1);
        dolbySprId = HuSprCreate(HuSprAnimDataRead(TITLE_ANM_dolby), 0, 0);
        HuSprGrpMemberSet(gid, 2, dolbySprId);
        HuSprPosSet(gid, 2, HU_DISP_CENTERX, HU_DISP_CENTERY-34);
        HuSprAttrSet(gid, 2, HUSPR_ATTR_DISPOFF|HUSPR_ATTR_LINEAR);
        HuWinInit(1);
        BootTitleCreate();
        NintendoDispF = TRUE;
        fromTitle = TRUE;
        
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            GwPlayerConf[i].padNo = i;
        }
        HuSprDispOn(gid, 0);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        WipeColorSet(255, 255, 255);
        if(!NintendoDispF) {
            tickPrev = OSGetTick();
            CharInit();
            HuWindowInit();
            GMesInit();
            HuWinInit(1);
            while(OSTicksToMilliseconds(OSGetTick()-tickPrev) < 1500) {
                HuPrcVSleep();
            }
        } else {
            for(i=0; i<90; i++) {
                if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                    break;
                }
                HuPrcVSleep();
            }
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        if(!NintendoDispF) {
            tickPrev = OSGetTick();
            BootTitleCreate();
            anim = HuSprAnimDataRead(TITLE_ANM_hudson);
            hudsonSprId = HuSprCreate(anim, 0, 0);
            HuSprGrpMemberSet(gid, 1, hudsonSprId);
            HuSprPosSet(gid, 1, HU_DISP_CENTERX, HU_DISP_CENTERY);
            HuSprDispOff(gid, 1);
            dolbySprId = HuSprCreate(HuSprAnimDataRead(TITLE_ANM_dolby), 0, 0);
            HuSprGrpMemberSet(gid, 2, dolbySprId);
            HuSprPosSet(gid, 2, HU_DISP_CENTERX, HU_DISP_CENTERY-34);
            HuSprDispOff(gid, 2);
            while(OSTicksToMilliseconds(OSGetTick()-tickPrev) < 1500) {
                HuPrcVSleep();
            }
        } else {
            HuPrcSleep(90);
        }
        HuSprDispOff(gid, 0);
        HuSprDispOn(gid, 1);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        if(!NintendoDispF) {
            HuPrcSleep(90);
        } else {
            for(i=0; i<90; i++) {
                if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                    break;
                }
                HuPrcVSleep();
            }
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        HuPrcSleep(90);
        HuSprDispOff(gid, 1);
        HuSprDispOn(gid, 2);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        if(!NintendoDispF) {
            HuPrcSleep(90);
        } else {
            for(i=0; i<90; i++) {
                if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                    break;
                }
                HuPrcVSleep();
            }
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        if(!NintendoDispF) {
            void *sampBuf;
            tickPrev = OSGetTick();
            sampBuf = HuMemDirectMalloc(HUHEAPTYPE_MODEL, msmSysGetSampSize(MSM_GRP_INIT));
            msmSysLoadGroup(MSM_GRP_INIT, sampBuf, FALSE);
            HuMemDirectFree(sampBuf);
            msmSysSetGroupLoadMode(MSM_GROUP_LOAD_AUTO);
            HuAudSndGrpSetSet(MSM_GRP_MENU);
        }
        NintendoDispF = TRUE;
        HuSprDispOff(gid, 0);
        HuSprDispOff(gid, 1);
        HuSprDispOff(gid, 2);
    }
    repeatTitle:
    if(!BootTitleExec(fromTitle)) {
        fromTitle = FALSE;
        HuPrcSleep(60);
        goto repeatTitle;
    }
    HuDataDirClose(DATA_title);
    statId = HuDataDirReadAsync(DATA_filesel);
    WipeColorSet(255, 255, 255);
    HuAudSStreamAllFadeOut(1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    HuPrcSleep(30);
    HuPrcVSleep();
    while(!HuDataGetAsyncStat(statId)) {
        HuPrcVSleep();
    }
    if(omSysExitReq != 0) {
        HuDataDirClose(DATA_filesel);
        omOvlReturn(1);
    } else {
        omOvlCall(DLL_fileseldll, 0, 0);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayerConf[i].charNo = CHARNO_NONE;
    }
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}


void BootProgExec(void)
{
    static u16 posTbl[] = {
        221, 311,
        357, 311
    };
    s16 choice = 0;
    s16 time;
    HUSPRGRPID gid;
    ANIMDATA *anim;
    HUSPRID sprid;
    if(OSGetResetCode() != 0) {
        return;
    }
    if(VIGetDTVStatus() != VI_NON_INTERLACE) {
        VIConfigure(&GXNtsc480IntDf);
        VIFlush();
        return;
    }
    if(OSGetProgressiveMode() == OS_PROGRESSIVE_MODE_OFF && !(HU_PAD_BTN_ALL & PAD_BUTTON_B)) {
        return;
    }
    if(OSGetProgressiveMode() == OS_PROGRESSIVE_MODE_ON) {
        VIConfigure(&GXNtsc480IntDf);
        VIFlush();
    }
    gid = HuSprGrpCreate(3);
    anim = HuSprAnimDataRead(TITLE_ANM_prog);
    sprid = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(gid, 0, sprid);
    HuSprPosSet(gid, 0, HU_DISP_CENTERX, HU_DISP_CENTERY);
    anim = HuSprAnimDataRead(TITLE_ANM_progCursor);
    sprid = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(gid, 1, sprid);
    HuSprPosSet(gid, 1, posTbl[(choice*2)+0], posTbl[(choice*2)+1]);
    anim = HuSprAnimDataRead(TITLE_ANM_progCursorSel);
    sprid = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(gid, 2, sprid);
    HuSprPosSet(gid, 2, posTbl[(choice*2)+0], posTbl[(choice*2)+1]);
    HuSprDispOff(gid, 2);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    WipeWait();
    for(time=0; time<600; time++) {
        if(HU_PAD_DSTK_ALL & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT)) {
            choice ^= 1;
            HuSprPosSet(gid, 1, posTbl[(choice*2)+0], posTbl[(choice*2)+1]);
            time = 0;
        }
        if(HU_PAD_BTNDOWN_ALL & PAD_BUTTON_A) {
            HuSprDispOff(gid, 1);
            HuSprPosSet(gid, 2, posTbl[(choice*2)+0], posTbl[(choice*2)+1]);
            HuSprDispOn(gid, 2);
            break;
        }
        HuPrcVSleep();
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20);
    WipeWait();
    if(choice == 0) {
        OSSetProgressiveMode(OS_PROGRESSIVE_MODE_ON);
        VIConfigure(&GXNtsc480Prog);
        VIFlush();
        HuPrcSleep(100);
    } else {
        OSSetProgressiveMode(OS_PROGRESSIVE_MODE_OFF);
    }
    HuSprGrpKill(gid);
    gid = HuSprGrpCreate(1);
    if(choice == 0) {
        anim = HuSprAnimDataRead(TITLE_ANM_progOn);
    } else {
        anim = HuSprAnimDataRead(TITLE_ANM_progOff);
    }
    sprid = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(gid, 0, sprid);
    HuSprPosSet(gid, 0, HU_DISP_CENTERX, HU_DISP_CENTERY);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
    WipeWait();
    HuPrcSleep(60);
    for(time=0; time<180; time++) {
        if(HU_PAD_BTNDOWN_ALL & PAD_BUTTON_A) {
            break;
        }
        HuPrcVSleep();
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    WipeWait();
    HuSprGrpKill(gid);
    HuPrcSleep(30);
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

void CameraMove(void)
{
    Vec pos;
    Vec offset;
    Vec dir;
    Vec yOfs;

    f32 rotZ;
    s8 stick_pos;
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
        stick_pos = (HuPadSubStkX[0] & 0xF8);
        if (stick_pos != 0) {
            cameraCenter[0].x += 0.05f * (offset.x * stick_pos);
            cameraCenter[0].y += 0.05f * (offset.y * stick_pos);
            cameraCenter[0].z += 0.05f * (offset.z * stick_pos);
        }
        VECNormalize(&yOfs, &offset);
        stick_pos = -(HuPadSubStkY[0] & 0xF8);
        if (stick_pos != 0) {
            cameraCenter[0].x += 0.05f * (offset.x * stick_pos);
            cameraCenter[0].y += 0.05f * (offset.y * stick_pos);
            cameraCenter[0].z += 0.05f * (offset.z * stick_pos);
        }
    }
}

void BootTitleCreate(void)
{
    HU3DMODELID mdlId;
    titleMdlId[0] = mdlId = Hu3DModelCreateData(TITLE_HSF_title);
    Hu3DModelDispOff(mdlId);
    Hu3DModelLayerSet(mdlId, 1);
    Hu3DModelCameraInfoSet(mdlId, HU3D_CAM0);
    Hu3DModelLightInfoSet(mdlId, 0);
    titleMdlId[1] = mdlId = Hu3DModelCreateData(TITLE_HSF_pressStart);
    Hu3DModelDispOff(mdlId);
    Hu3DModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(mdlId, 1);
    titleMdlId[2] = mdlId = Hu3DModelCreateData(TITLE_HSF_back);
    Hu3DModelDispOff(mdlId);
    Hu3DModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    titleMdlId[3] = mdlId = Hu3DModelCreateData(TITLE_HSF_movie);
    Hu3DModelDispOff(mdlId);
    Hu3DModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    titleMotId[0] = Hu3DJointMotionData(titleMdlId[0], TITLE_HSF_titleMotIn);
    titleMotId[1] = Hu3DJointMotionData(titleMdlId[0], TITLE_HSF_titleMotWait);
}

BOOL BootTitleExec(BOOL fromTitle)
{
    float scale;
    s16 time;
    BOOL cancelF;
    Hu3DModelDispOn(titleMdlId[2]);
    if(fromTitle) {
        Hu3DModelDispOn(titleMdlId[0]);
        Hu3DMotionSet(titleMdlId[0], titleMotId[0]);
        Hu3DModelAttrReset(titleMdlId[0], HU3D_MOTATTR_LOOP);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        HuPrcSleep(10);
        HuAudSStreamPlay(MSM_STREAM_TITLE);
        WipeWait();
    } else {
        Hu3DModelDispOn(titleMdlId[3]);
        HuTHPSprCreateVol(movieTbl[movieOrderTbl[movieNo]], 1, 100, movieVolTbl[movieOrderTbl[movieNo]]);
        Hu3DModelMatHookSet(titleMdlId[3], HuTHPMatHook);
        movieNo++;
        movieNo %= MOVIE_MAX;
        while(!HuTHPStartCheck()) {
            HuPrcVSleep();
        }
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        for(time=1; time<=30; time++) {
            scale = time/30.0;
            Hu3DModelPosSet(titleMdlId[3], 0, 0, (1.0-scale)*800);
            scale = HuSin(scale*90.0f);
            Hu3DModelScaleSet(titleMdlId[3], scale, scale, scale);
            HuPrcVSleep();
        }
        cancelF = FALSE;
        while(!HuTHPEndCheck()) {
            if(HuTHPFrameGet()+40 >= HuTHPTotalFrameGet()) {
                break;
            }
            if(HuPadBtnDown[0] & (PAD_BUTTON_A|PAD_BUTTON_START)) {
                HuTHPSetVolume(0, 500);
                HuPrcSleep(20);
                cancelF = TRUE;
                break;
            }
            HuPrcVSleep();
        }
        Hu3DModelDispOn(titleMdlId[0]);
        Hu3DMotionSet(titleMdlId[0], titleMotId[0]);
        Hu3DModelAttrReset(titleMdlId[0], HU3D_MOTATTR_LOOP);
        for(time=1; time<=40; time++) {
            scale = time/40.0;
            Hu3DModelPosSet(titleMdlId[3], 0, (-HuSin(180.0f*scale)*100)-(70*HuSin(90.0f*scale)), 800*scale);
            scale = HuCos(scale*90.0f);
            Hu3DModelScaleSet(titleMdlId[3], scale, scale, scale);
            if(time == 10) {
                HuAudSStreamPlay(MSM_STREAM_TITLE);
            }
            HuPrcVSleep();
        }
        HuTHPClose();
    }
    while(!Hu3DMotionEndCheck(titleMdlId[0])) {
        HuPrcVSleep();
    }
    Hu3DMotionSet(titleMdlId[0], titleMotId[1]);
    Hu3DModelAttrSet(titleMdlId[0], HU3D_MOTATTR_LOOP);
    Hu3DModelDispOn(titleMdlId[1]);
    Hu3DMotionStartEndSet(titleMdlId[1], 0, 359);
    Hu3DModelAttrSet(titleMdlId[1], HU3D_MOTATTR_LOOP);
    for(time=0; time<1800; time++) {
        if(HuPadBtnDown[0] & PAD_BUTTON_START) {
            Hu3DMotionStartEndSet(titleMdlId[1], 0, Hu3DMotionMaxTimeGet(titleMdlId[1]));
            Hu3DModelAttrReset(titleMdlId[1], HU3D_MOTATTR_LOOP);
            Hu3DMotionTimeSet(titleMdlId[1], 360);
            HuAudFXPlay(MSM_SE_MENU_39);
            HuPrcSleep(20);
            return TRUE;
        }
        HuPrcVSleep();
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    WipeWait();
    Hu3DModelDispOff(titleMdlId[0]);
    Hu3DModelDispOff(titleMdlId[1]);
    return FALSE;
}

extern u32 nintendoData[];

void *NintendoDataDecode(void)
{
    u32 *src = nintendoData;
    u32 size = *src++;
    void *dst = HuMemDirectMalloc(HUHEAPTYPE_MODEL, size);
    u32 type = *src++;
    if(dst) {
        HuDecodeData(src, dst, size, type);
    }
    return dst;
}

void MovieShuffle(s16 *list, s16 num)
{
    s16 i;
    if(num == 1) {
        return;
    }
    for(i=0; i<100; i++) {
        s16 srcIdx = frandmod(num);
        s16 dstIdx = frandmod(num);
        s16 temp = list[srcIdx];
        list[srcIdx] = list[dstIdx];
        list[dstIdx] = temp;
    }
}