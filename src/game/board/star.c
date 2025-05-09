#include "game/board/star.h"
#include "game/board/model.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#include "game/board/guide.h"
#include "game/board/camera.h"
#include "game/board/window.h"
#include "game/board/status.h"
#include "game/board/com.h"
#include "game/board/coin.h"
#include "game/board/audio.h"
#include "game/board/tutorial.h"
#include "game/board/scroll.h"

#include "game/sprite.h"
#include "game/wipe.h"

#include "datanum/effect.h"
#include "datanum/w10.h"

#include "messnum/board_star.h"
#include "messnum/board_tutorial.h"

#define STAROBJ_MAX 1000
#define STAR_MAX 8

static OMOBJ *starObj[1000];
static u8 starNoTbl[1000];

static HUPROCESS *starMasuProc;
static int starNum;
static int starObjNo;
static int starMaxNo;

static const u32 starBaseMesTbl[1] = { MESS_BOARD_STAR_CHORL_1 };

#define STAR_MODE_IDLE 0
#define STAR_MODE_FALL 1
#define STAR_MODE_SHRINK 2
#define STAR_MODE_ROT 3

typedef struct StarWork_s {
    unsigned killF : 1;
    unsigned rotF : 1;
    unsigned dispF : 1;
    int objNo;
    int no;
    int masuId;
    s8 mode;
    s8 rotSpeed;
    s16 rotY;
    HU3DPARMANID parManId;
    float baseY;
    s16 time;
    ANIMDATA *effAnim;
} STARWORK;

static void StarObjUpdate(OMOBJ *obj);
static void StarObjKill(OMOBJ *obj);
static void StarRotUpdate(STARWORK *work, OMOBJ *obj);
static void StarFallStart(OMOBJ *obj);
static void StarShrinkStart(OMOBJ *obj);
static void StarShrinkWait(OMOBJ *obj);
static HU3DPARMANID StarEffCreate(ANIMDATA *anim);
static void StarEffKill(HU3DPARMANID parManId);

void MBStarNoInit(void)
{
    starObjNo = 0;
    memset(starNoTbl, 0xFF, STAR_MAX*sizeof(u8));
}

void MBStarInit(void)
{
    int i;
    memset(starObj, 0, STAR_MAX*sizeof(OMOBJ *));
    starMaxNo = starNum-1;
    if(starMaxNo < 1) {
        starMaxNo = 1;
    }
    for(i=0; i<starNum; i++) {
        if((1 << i) & MBStarFlagGet()) {
            MBStarCreate(i);
        }
    }
    starMasuProc = NULL;
}

void MBStarClose(void)
{
    int i;
    for(i=0; i<STAR_MAX; i++) {
        if(starObj[i]) {
            StarObjKill(starObj[i]);
            starObj[i] = NULL;
        }
    }
}

static int StarObjCreate(int no, HuVecF *pos)
{
    int i;
    for(i=0; i<STAR_MAX; i++) {
        if(!starObj[i]) {
            OMOBJ *obj = MBAddObj(257, 1, 0, StarObjUpdate);
            STARWORK *work;
            starObj[i] = obj;
            work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(STARWORK), HU_MEMNUM_OVL);
            obj->data = work;
            memset(work, 0, sizeof(STARWORK));
            obj->mdlId[0] = MBModelCreate(MBDATANUM(BOARD_HSF_star), NULL, TRUE);
            MBModelRotSet(obj->mdlId[0], -90, 0, 0);
            MBModelZWriteOffSet(obj->mdlId[0], FALSE);
            MBMotionNoSet(obj->mdlId[0], MB_MOT_DEFAULT, HU3D_MOTATTR_LOOP);
            MBMotionSpeedSet(obj->mdlId[0], 1);
            MBModelLayerSet(obj->mdlId[0], 2);
            work->no = no;
            if(work->no >= 0) {
                work->masuId = MBMasuStarGet(work->no);
            } else {
                work->masuId = MASU_NULL;
            }
            work->objNo = i;
            work->mode = STAR_MODE_IDLE;
            work->effAnim = HuSprAnimDataRead(EFFECT_ANM_glow);
            HuDataDirClose(DATA_effect);
            work->parManId = StarEffCreate(work->effAnim);
            work->time = 0;
            work->baseY = pos->y-350;
            work->dispF = TRUE;
            obj->trans.x = pos->x;
            obj->trans.y = pos->y;
            obj->trans.z = pos->z;
            obj->rot.z = obj->trans.y;
            omSetStatBit(obj, OM_STAT_MODELPAUSE);
            return i;
            
        }
    }
    return -1;
}

static int StarMasuNoGet(int masuId)
{
    int i;
    for(i=0; i<STAR_MAX; i++) {
        if(starObj[i]) {
            STARWORK *work = starObj[i]->data;
            if(work->masuId == masuId) {
                return i;
            }
        }
    }
    return -1;
}

static OMOBJ *StarMasuObjGet(int masuId)
{
    int no = StarMasuNoGet(masuId);
    if(no >= 0) {
        return starObj[no];
    } else {
        return NULL;
    }
}

static void StarGetMain(int playerNo)
{
    int starNo = MBStarNoRandGet();
    if(starNo < 0) {
        OMOBJ *obj = StarMasuObjGet(GwPlayer[playerNo].masuId);
        STARWORK *work;
        work = obj->data;
        starNo = work->no;
    }
    MBStarGetExec(playerNo, FALSE);
    MBStarMapViewExec(playerNo, starNo);
}

void MBStarNoReset(int no)
{
    int i;
    for(i=0; i<STAR_MAX; i++) {
        if(starObj[i]) {
            STARWORK *work = starObj[i]->data;
            if(no == work->no) {
                StarObjKill(starObj[i]);
                MBStarFlagReset(no);
                return;
            }
        }
    }
}

void MBStarCreate(int no)
{
    int masuId = MBMasuStarGet(no);
    HuVecF pos;
    int id;
    OMOBJ *obj;
    MBMasuTypeSet(MASU_LAYER_DEFAULT, masuId, MASU_TYPE_STAR);
    if(masuId == GwSystem.saiMasuId) {
        MBMasuSaiHiddenSet();
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
    pos.y += 350;
    id = StarObjCreate(no, &pos);
    obj = starObj[id];
    MBStarNextNoSet(no);
    MBStarFlagSet(no);
    starNoTbl[starObjNo] = no;
    starObjNo = (starObjNo+1)%starMaxNo;
    MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, CAPSULE_NULL);
}

static void MasuStarExec(void);
static void MasuStarKill(void);

void MBMasuStarExec(int playerNo, int masuId)
{
    MBWalkNumDispSet(playerNo, FALSE);
    starMasuProc = MBPrcCreate(MasuStarExec, 8195, 14336);
    MBPrcDestructorSet(starMasuProc, MasuStarKill);
    while(starMasuProc != NULL) {
        HuPrcVSleep();
    }
    MBWalkNumDispSet(playerNo, TRUE);
}

static void MasuStarExec(void)
{
    int guideNo = MBGuideNoGet();
    u32 messBase = starBaseMesTbl[guideNo];
    int playerNo = GwSystem.turnPlayerNo;
    int starNo;
    MBAudFXPlay(MSM_SE_BOARD_94);
    MBPlayerMotIdleSet(playerNo);
    MBPlayerRotateStart(playerNo, 0, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBCameraViewNoSet(GwSystem.turnPlayerNo, MB_CAMERA_VIEW_ZOOMOUT);
    MBCameraMotionWait();
    if(MBPlayerStarGet(playerNo) >= 999) {
        MBAudFXPlay(MSM_SE_GUIDE_51);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, messBase+5, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
    } else {
        if(MBPlayerCoinGet(playerNo) < 20) {
            MBAudFXPlay(MSM_SE_GUIDE_51);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, messBase+2, MBGuideSpeakerNoGet());
            MBTopWinWait();
            MBTopWinKill();
        } else {
            MBStatusDispSetAll(FALSE);
            while(!MBStatusOffCheckAll()) {
                HuPrcVSleep();
            }
            MBAudFXPlay(MSM_SE_GUIDE_49);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, messBase+0, MBGuideSpeakerNoGet());
            MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 0);
            MBTopWinWait();
            MBTopWinKill();
            MBStatusDispFocusSet(playerNo, TRUE);
            while(!MBStatusMoveCheck(playerNo)) {
                HuPrcVSleep();
            }
            MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, messBase+1, MBGuideSpeakerNoGet(), 0);
            MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
            if(GwPlayer[playerNo].comF) {
                if(MBPlayerCoinGet(playerNo) >= 20) {
                    MBComChoiceSetUp();
                } else {
                    MBComChoiceSetDown();
                }
            }
            MBTopWinWait();
            switch(MBWinLastChoiceGet()) {
                case 0:
                    if(MBPlayerCoinGet(playerNo) < 20) {
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, messBase+2, MBGuideSpeakerNoGet());
                        MBTopWinWait();
                        MBStatusDispFocusSet(playerNo, FALSE);
                        while(!MBStatusMoveCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        MBStatusDispSetAll(TRUE);
                    } else {
                        goto starBuy;
                    }
                    break;
               
                case 1:
                    MBAudFXPlay(MSM_SE_GUIDE_50);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, messBase+4, MBGuideSpeakerNoGet());
                    MBTopWinWait();
                    MBStatusDispFocusSet(playerNo, FALSE);
                    while(!MBStatusMoveCheck(playerNo)) {
                        HuPrcVSleep();
                    }
                    MBStatusDispSetAll(TRUE);
                    break;
               
                default:
                    starBuy:
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, messBase+3, MBGuideSpeakerNoGet());
                    MBTopWinWait();
                    MBTopWinKill();
                    MBCoinAddExec(playerNo, -20);
                    _SetFlag(FLAG_BOARD_STARMUS_RESET);
                    HuPrcSleep(30);
                    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
                    StarGetMain(playerNo);
                    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
                    }
                    WipeWait();
                    MBMusMainPlay();
                    MBTopWinKill();
                    break;
            }
        }
    }
    _ClearFlag(FLAG_BOARD_STARMUS_RESET);
    MBCameraViewNoSet(GwSystem.turnPlayerNo, MB_CAMERA_VIEW_WALK);
    MBCameraFocusPlayerSet(GwSystem.turnPlayerNo);
    MBCameraMotionWait();
    HuPrcEnd();
}

static void MasuStarKill(void)
{
    starMasuProc = NULL;
}

void MBStarGetProcExec(int playerNo, HuVecF *pos, int starNum, BOOL focusF)
{
    int time;
    OMOBJ *obj;
    STARWORK *work;
    int objNo;
    int masuId;
    int seNo;
    int no;
    int streamNo;
    masuId = GwPlayer[playerNo].masuId;
    seNo = MSM_SENO_NONE;
    _SetFlag(FLAG_BOARD_TURN_NOSTART);
    if(pos == NULL) {
        obj = StarMasuObjGet(masuId);
        work = obj->data;
        no = work->no;
    } else {
        HuVecF playerPos;
        objNo = StarObjCreate(-1, pos);
        
        obj = starObj[objNo];
        work = obj->data;
        MBPlayerPosGet(playerNo, &playerPos);
        work->baseY = playerPos.y;
        no = -1;
        seNo = MBAudFXPlay(MSM_SE_BOARD_91);
        MBAudFXPlay(MSM_SE_BOARD_92);
        HuPrcVSleep();
    }
    StarFallStart(obj);
    StarShrinkWait(obj);
    MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_JUMPSAI, HU3D_MOTATTR_NONE);
    time = 0;
    while(1) {
        if(time++ == 27) {
            StarShrinkStart(obj);
        }
        if(time == 30) {
            if(seNo >= 0) {
                MBAudFXStop(seNo);
            }
            MBAudFXPlay(MSM_SE_BOARD_93);
        }
        HuPrcVSleep();
        if(MBPlayerMotionEndCheck(playerNo)) {
            break;
        }
    }
    MBPlayerMotIdleSet(playerNo);
    omVibrate(playerNo, 12, 4, 2);
    streamNo = MBMusPlayJingle(MSM_STREAM_JNGL_STARGET);
    MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_WIN, CHARVOICEID(0));
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_WIN, 0, 8, HU3D_MOTATTR_NONE);
    if(focusF) {
        static const HuVecF offset = { 0, 100, 0 };
        float weight;
        float deltaZoom;
        float zoom;
        float zoomBase;
        MBCameraSkipSet(FALSE);
        MBCameraModelViewSet(MBPlayerModelGet(playerNo), NULL, (HuVecF *)&offset, 1000, -1, 102);
        MBCameraMotionWait();
        zoomBase = MBCameraZoomGet();
        MBCameraFocusSet(MB_MODEL_NONE);
        for(time=0; time<30u; time++) {
            weight = time/30.0f;
            deltaZoom = (1+HuSin(weight*720.0f))*0.5;
            zoom = zoomBase+(-250.0f*deltaZoom);
            MBCameraZoomSet(zoom);
            HuPrcVSleep();
        }
    }
    MBPlayerStarAdd(playerNo, starNum);
    GwSystem.starTotal = GwSystem.starTotal+starNum;
    if(no >= 0) {
        MBStarNextNoSet(no);
        MBStarFlagReset(no);
    }
    while(MBMusJingleStatGet(streamNo)) {
        HuPrcVSleep();
    }
    MBPlayerMotionEndWait(playerNo);
    if(focusF) {
        MBCameraMotionWait();
    }
    HuPrcSleep(30);
    _ClearFlag(FLAG_BOARD_TURN_NOSTART);
}

void MBStarGetExec(int playerNo, HuVecF *pos)
{
    MBStarGetProcExec(playerNo, pos, 1, TRUE);
}

static void StarObjUpdate(OMOBJ *obj)
{
    STARWORK *work = obj->data;
    float rotY;
    if(work->killF || MBKillCheck()) {
        StarEffKill(work->parManId);
        work->parManId = HU3D_PARMANID_NONE;
        work->effAnim = NULL;
        MBModelKill(obj->mdlId[0]);
        obj->mdlId[0] = MB_MODEL_NONE;
        MBDelObj(obj);
        starObj[work->objNo] = NULL;
        return;
    }
    switch(work->mode) {
        case STAR_MODE_IDLE:
            obj->trans.y = obj->rot.z+(100*(0.2f*HuSin(work->time*4.0f)));
            work->time++;
            break;
            
        case STAR_MODE_FALL:
            obj->trans.y += -4.0f;
            if(obj->trans.y <= 250.0f+work->baseY) {
                obj->trans.y = 250.0f+work->baseY;
                work->mode = STAR_MODE_ROT;
            }
            break;
            
        case STAR_MODE_SHRINK:
            obj->trans.y += -4.0f;
            if(obj->trans.y > 80.0f+work->baseY) {
                if(work->rotY < 90) {
                    work->rotY += 2;
                    if(work->rotY > 90) {
                        work->rotY  = 90;
                    }
                }
                rotY = work->rotY;
                obj->scale.x = HuSin(rotY+90.0f);
                if(obj->scale.x <= 0) {
                    obj->scale.x = 0.001f;
                }
                obj->scale.y = obj->scale.x;
                obj->scale.z = obj->scale.x;
            } else {
                work->killF = TRUE;
                MBModelDispSet(obj->mdlId[0], FALSE);
                return;
            }
            break;
    }
    StarRotUpdate(work, obj);
    MBModelPosSet(obj->mdlId[0], obj->trans.x, obj->trans.y, obj->trans.z);
    MBModelRotSet(obj->mdlId[0], 0, obj->rot.y, 0);
    MBModelScaleSet(obj->mdlId[0], obj->scale.x, obj->scale.y, obj->scale.z);
    if(work->parManId >= 0) {
        Hu3DParManPosSet(work->parManId, obj->trans.x, obj->trans.y, obj->trans.z);
    }
}

static void StarObjKill(OMOBJ *obj)
{
    STARWORK *work = obj->data;
    work->killF = TRUE;
}

static void StarRotUpdate(STARWORK *work, OMOBJ *obj)
{
    float rotSpeed;
    if(!work->rotF) {
        return;
    }
    if(work->rotSpeed < 16.0f) {
        work->rotSpeed += 2.0f;
        if(work->rotSpeed > 16.0f) {
            work->rotSpeed = 16;
        }
    }
    OSs8tof32(&work->rotSpeed, &rotSpeed);
    obj->rot.y = MBAngleWrap(obj->rot.y+rotSpeed);
}

static void StarFallStart(OMOBJ *obj)
{
    STARWORK *work = obj->data;
    work->mode = STAR_MODE_FALL;
    if(work->dispF) {
        return;
    }
    MBStarDispSet(work->no, TRUE);
    obj->trans.y = 800+work->baseY;
    if(work->parManId >= 0) {
        HU3DPARTICLE *particle;
        HU3DPARTICLEDATA *particleData;
        int i;
        HU3DPARMAN *parMan;
        HU3DMODEL *modelP;
        Hu3DParManPosSet(work->parManId, obj->trans.x, obj->trans.y, obj->trans.z);
        parMan = Hu3DParManPtrGet(work->parManId);
        modelP = &Hu3DData[parMan->modelId];
        particle = modelP->hookData;
        for(particleData = particle->data, i=0; i<particle->maxCnt; i++, particleData++) {
            particleData->scale = 0;
        }
    }
}

static void StarShrinkStart(OMOBJ *obj)
{
    STARWORK *work = obj->data;
    work->rotF = TRUE;
    work->rotY = 0;
    work->mode = STAR_MODE_SHRINK;
}

static void StarShrinkWait(OMOBJ *obj)
{
    STARWORK *work = obj->data;
    while(work->mode != STAR_MODE_ROT) {
        HuPrcVSleep();
    }
}

static HU3DPARMANID StarEffCreate(ANIMDATA *anim)
{
    static HU3DPARMANPARAM effParam = {
        30, 0,
        3.3f,
        70.0f,
        7.0f,
        { 0, -0.05f, 0 },
        2,
        1,
        30,
        0.98f,
        2,
        {
            { 255, 255, 255, 255 },
            { 255, 255, 64, 255 },
        },
        {
            { 255, 128, 128, 0 },
            { 255, 64, 32, 0 },
        },
    };
    HU3DPARMANID parManId = Hu3DParManCreate(anim, 100, &effParam);
    HU3DMODELID modelId;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_SCALEJITTER|HU3D_PARMAN_ATTR_RANDSPEED70|HU3D_PARMAN_ATTR_RANDSCALE70);
    Hu3DParManRotSet(parManId, 90, 0, 0);
    modelId = Hu3DParManModelIDGet(parManId);
    Hu3DParticleBlendModeSet(modelId, HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    return parManId;
    
}

static void StarEffKill(HU3DPARMANID parManId)
{
    if(parManId >= 0) {
        Hu3DParManKill(parManId);
    }
}

static const starFirstMesTbl[1][2] = { MESS_BOARD_STAR_FIRST_1, MESS_BOARD_STAR_FIRST_2 };

void MBStarMapViewProcExec(int playerNo, HuVecF *pos, int no)
{
    BOOL playerOffF = FALSE;
    int guideNo = MBGuideNoGet();
    u32 mess;
    int scrollTime;
    WIPEWORK *wipe;
    MBMODELID guideMdlId;
    HuVecF endPos;
    HuVecF startPos;
    HuVecF scrollDir;
    HuVecF guidePos;
    float scrollMag;
    
    static HuVecF cameraRot = { -33, 0, 0 };
    static HuVecF cameraOfs = { 0, 100, 0 };
    _SetFlag(FLAG_BOARD_TURN_NOSTART);
    wipe = &wipeData;
    if(wipe->mode == WIPE_MODE_DUMMY) {
        WipeColorSet(0, 0, 0);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
        }
        WipeWait();
    } else {
        HuPrcSleep(18);
    }
    MBStatusDispForceSetAll(FALSE);
    if(playerNo == -1) {
        playerOffF = TRUE;
        playerNo = 0;
        MBPlayerPosReset(playerNo);
        MBPlayerPosFixPlayer(playerNo, GwPlayer[playerNo].masuId, TRUE);
    } else {
        MBMasuTypeSet(MASU_LAYER_DEFAULT, MBMasuStarGet(MBStarNextNoGet()), MASU_TYPE_BLUE);
    }
    MBStarNoSet(no);
    if(!_CheckFlag(FLAG_BOARD_STARMUS_RESET)) {
        MBMusFadeOut(MB_MUS_CHAN_BG, 100);
        while(MBMusCheck(MB_MUS_CHAN_BG)) {
            HuPrcVSleep();
        }
    }
    MBPlayerMotIdleSet(playerNo);
    if(pos == NULL) {
        MBPlayerPosGet(playerNo, &startPos);
    } else {
        startPos = *pos;
    }
    MBCameraFocusSet(-1);
    MBCameraSkipSet(FALSE);
    MBCameraPosViewSet(&startPos, &cameraRot, &cameraOfs, 1800, -1, 1);
    MBCameraMotionWait();
    MBMasuPosGet(MASU_LAYER_DEFAULT, MBMasuStarGet(no), &endPos);
    VECSubtract(&endPos, &startPos, &scrollDir);
    scrollMag = VECMag(&scrollDir);
    if(scrollMag > 3000) {
        scrollTime = 240;
    } else if(scrollMag > 1500) {
        scrollTime = 120;
    } else {
        scrollTime = 120;
    }
    MBStarScrollCreate(&startPos, &endPos, scrollTime);
    guidePos.x = 68;
    guidePos.y = 384;
    guidePos.z = 1000;
    guideMdlId = MBGuideCreateIn(&guidePos, TRUE, FALSE, FALSE);
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_STAR, 127, 0);
    MBCameraSkipSet(FALSE);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    HuPrcSleep(1);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    if(!playerOffF) {
        mess = starBaseMesTbl[guideNo]+6;
    } else {
        mess = starFirstMesTbl[guideNo][0];
    } 
    MBAudFXPlay(MSM_SE_GUIDE_49);
    MBWinCreate(MBWIN_TYPE_GUIDE, mess, HUWIN_SPEAKER_NONE);
    MBTopWinPause();
    MBStarScrollStart();
    while(!MBStarScrollDoneCheck()) {
        HuPrcVSleep();
    }
    MBCameraMasuViewSet(MBMasuStarGet(no), NULL, NULL, 3200, -1, 60);
    MBCameraMotionWait();
    MBTopWinKill();
    HuPrcSleep(3);
    if(!playerOffF) {
        mess = starBaseMesTbl[guideNo]+7;
    } else {
        mess = starFirstMesTbl[guideNo][1];
    }
    MBWinCreate(MBWIN_TYPE_GUIDE, mess, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBTopWinKill();
    MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    WipeColorSet(0, 0, 0);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    MBStarScrollKill();
    MBGuideEnd(guideMdlId);
    MBCameraSkipSet(FALSE);
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMOUT);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBStatusDispForceSetAll(TRUE);
    while(MBMusCheck(MB_MUS_CHAN_FG)) {
        HuPrcVSleep();
    }
    if(!playerOffF) {
        if(!_CheckFlag(FLAG_BOARD_STARMUS_RESET)) {
            MBMusMainPlay();
        }
        _ClearFlag(FLAG_BOARD_TURN_NOSTART);
    }
}

void MBStarMapViewExec(int playerNo, int no)
{
    MBStarMapViewProcExec(playerNo, NULL, no);
}

void MBStarNumSet(int num)
{
    starNum = num;
}

int MBStarNumGet(void)
{
    return starNum;
}

void MBStarFlagInit(void)
{
    GwSystem.starFlag = 0;
}

void MBStarFlagSet(int no)
{
    GwSystem.starFlag |= (1 << no);
}

void MBStarFlagReset(int no)
{
    GwSystem.starFlag &= ~(1 << no);
}

u8 MBStarFlagGet(void)
{
    return GwSystem.starFlag;
}

void MBStarNextNoSet(s8 nextNo)
{
    GwSystem.starPos = nextNo;
}

u8 MBStarNextNoGet(void)
{
    return GwSystem.starPos;
}

void MBStarNoSet(int no)
{
    MBStarCreate(no);
}

int MBStarNoRandGet(void)
{
    int i;
    int objNo;
    int num;
    int j;
    int lastStar;
    int prevStar;
    u8 flag;
    s8 no[STAR_MAX];
    prevStar = 9999;
    num = 0;
    flag = MBStarFlagGet();
    
    for(i=0; i<starNum; i++) {
        if(!(flag & (1 << i))) {
            lastStar = -1;
            for(objNo=starObjNo, j=0; j<starMaxNo; objNo=(objNo+1)%starMaxNo, j++) {
                if(starNoTbl[objNo] != 0xFF && starNoTbl[objNo] == i) {
                    lastStar = j;
                }
            }
            if(lastStar <= prevStar) {
                if(lastStar < prevStar) {
                    prevStar = lastStar;
                    num = 0;
                }
                no[num++] = i;
            }
        }
    }
    if(num == 0) {
        return -1;
    } else {
        return no[frandmod(num)];
    }
}

void MBStarObjDispSet(int objNo, BOOL dispF)
{
    OMOBJ *obj = starObj[objNo];
    if(obj) {
        STARWORK *work;
        HU3DMODELID modelId;
        MBModelDispSet(obj->mdlId[0], dispF);
        work = obj->data;
        modelId = Hu3DParManModelIDGet(work->parManId);
        if(dispF) {
            Hu3DModelDispOn(modelId);
        } else {
            Hu3DModelDispOff(modelId);
        }
        work->dispF = dispF;
    }
}

void MBStarDispSet(int no, BOOL dispF)
{
    int i;
    for(i=0; i<999; i++) {
        OMOBJ *obj = starObj[i];
        if(obj) {
            STARWORK *work;
            work = obj->data;
            if(no == work->no) {
                MBStarObjDispSet(i, dispF);
                return;
            }
        }
    }
}

void MBStarMasuDispSet(int masuId, BOOL dispF)
{
    int no = StarMasuNoGet(masuId);
    if(no >= 0) {
        MBStarObjDispSet(no, dispF);
    }
}

void MBStarDispSetAll(BOOL dispF)
{
    int i;
    for(i=0; i<STAR_MAX; i++) {
        MBStarObjDispSet(i, dispF);
    }
}

static void StarObjTutorialUpdate(OMOBJ *obj);
static void StarObjTutorialGrow(OMOBJ *obj);

static void StarObjTutorialStop(OMOBJ *obj);
static void StarObjTutorialShrink(OMOBJ *obj);

void MBStarTutorialExec(void)
{
    OMOBJ *obj;
    int i;
    int masuIdStar;
    ANIMDATA *effAnim;
    HuVecF pos;
    HuVecF guidePos;

    static const HuVecF cameraOfs = { 0, 100, 0 };
    static const int starFileTbl[4] = {
        MBDATANUM(BOARD_HSF_star),
        W10_HSF_miniStar,
        W10_HSF_coinStar,
        W10_HSF_hatenaStar
    };
    static const u32 starMesTbl[3] = {
        MESS_BOARD_TUTORIAL_STAR_MINI,
        MESS_BOARD_TUTORIAL_STAR_COIN,
        MESS_BOARD_TUTORIAL_STAR_HATENA
    };
    MBStarDispSetAll(FALSE);
    masuIdStar = MBMasuStarGet(MBStarNextNoGet());
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdStar, &pos);
    MBCameraMasuViewSet(masuIdStar, NULL, (HuVecF *)&cameraOfs, 2100, -1, 1);
    MBCameraMotionWait();
    obj = MBAddObj(257, 5, 0, StarObjTutorialUpdate);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdStar, &obj->trans);
    for(i=0; i<4; i++) {
        obj->mdlId[i] = MBModelCreate(starFileTbl[i], NULL, FALSE);
    }
    effAnim = HuSprAnimDataRead(EFFECT_ANM_glow);
    HuDataDirClose(DATA_effect);
    obj->mdlId[4] = StarEffCreate(effAnim);
    StarObjTutorialStop(obj);
    guidePos.x = 68;
    guidePos.y = 308;
    guidePos.z = 1000;
    MBGuideScreenPosSet(&guidePos);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    }
    WipeWait();
    MBAudFXPlay(MSM_SE_GUIDE_49);
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_STAR, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_STAR_END, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_STAR_BONUS, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    for(i=0; i<3; i++) {
        StarObjTutorialShrink(obj);
        while((s32)obj->work[0] >= 0) {
            HuPrcVSleep();
        }
        StarObjTutorialGrow(obj);
        MBWinCreate(MBWIN_TYPE_TUTORIAL, starMesTbl[i], HUWIN_SPEAKER_NONE);
        MBTopWinWait();
    }
    StarObjTutorialShrink(obj);
    while((s32)obj->work[0] >= 0) {
        HuPrcVSleep();
    }
    Hu3DParManAttrSet(obj->mdlId[4], HU3D_PARMAN_ATTR_TIMEUP);
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_STAR_WINNER, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBAudFXPlay(MSM_SE_GUIDE_50);
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_END, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
}

static void StarObjTutorialUpdate(OMOBJ *obj)
{
    int i;
    float scaleFactor, scaleGrow;
    HuVecF pos;
    if(MBKillCheck()) {
        for(i=0; i<4; i++) {
            MBModelKill(obj->mdlId[i]);
        }
        if(obj->mdlId[4] >= 0) {
            StarEffKill(obj->mdlId[4]);
        }
        
        MBDelObj(obj);
        return;
    }
    switch(obj->work[0]) {
        case 0:
            if(obj->work[1] > 30) {
                obj->work[0] = 1;
                obj->work[1] = 0;
            } else {
                scaleGrow = (obj->work[1]++)/30.0f;
                pos.x = obj->trans.x;
                pos.y = obj->trans.y+(350.0f*scaleGrow);
                pos.z = obj->trans.z;
                MBModelScaleSet(obj->mdlId[obj->work[2]], 2*scaleGrow, 2*scaleGrow, 2*scaleGrow);
                MBModelPosSetV(obj->mdlId[obj->work[2]], &pos);
                MBModelRotSet(obj->mdlId[obj->work[2]], 0, 360*HuSin(scaleGrow*90.0f), 0);
                Hu3DParManPosSet(obj->mdlId[4], pos.x, pos.y, pos.z);
            }
            break;
        
        case 1:
            pos.x = obj->trans.x;
            pos.y = obj->trans.y+350+(100*(0.2f*HuSin(obj->work[1]*4.0f)));
            pos.z = obj->trans.z;
            obj->work[1]++;
            MBModelPosSetV(obj->mdlId[obj->work[2]], &pos);
            Hu3DParManPosSet(obj->mdlId[4], pos.x, pos.y, pos.z);
            break;
        
        case 2:
            if(obj->work[1] > 30) {
                MBModelDispSet(obj->mdlId[obj->work[2]++], FALSE);
                obj->work[0] = -1;
            } else {
                scaleGrow = 1-((obj->work[1]++)/30.0f);
                pos.x = obj->trans.x;
                pos.y = obj->trans.y+(350.0f*scaleGrow);
                pos.z = obj->trans.z;
                scaleFactor = (obj->work[2] == 0) ? 1.0f : 2.0f;
                MBModelScaleSet(obj->mdlId[obj->work[2]], scaleGrow*scaleFactor, scaleGrow*scaleFactor, scaleGrow*scaleFactor);
                MBModelPosSetV(obj->mdlId[obj->work[2]], &pos);
                MBModelRotSet(obj->mdlId[obj->work[2]], 0, 360*HuSin(scaleGrow*90.0f), 0);
                Hu3DParManPosSet(obj->mdlId[4], pos.x, pos.y, pos.z);
            }
            break;
    }
}

static void StarObjTutorialGrow(OMOBJ *obj)
{
    obj->work[0] = 0;
    obj->work[1] = 0;
}

static void StarObjTutorialStop(OMOBJ *obj)
{
    obj->work[0] = 1;
    obj->work[1] = 0;
}

static void StarObjTutorialShrink(OMOBJ *obj)
{
    obj->work[0] = 2;
    obj->work[1] = 0;
}