#include "game/board/scroll.h"
#include "game/board/model.h"
#include "game/board/pause.h"
#include "game/board/audio.h"
#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/status.h"
#include "game/board/window.h"
#include "game/board/effect.h"
#include "game/board/player.h"
#include "game/board/star.h"
#include "game/board/branch.h"
#include "game/board/pad.h"

#include "game/wipe.h"
#include "game/sprite.h"
#include "game/esprite.h"

#include "messnum/boardope.h"

//Move to separate header
s8 MBPadStkXGet(int padNo);
s8 MBPadStkYGet(int padNo);

#define MAPSPR_PLAYER 0
#define MAPSPR_STAR 1

#define MAPSPR_MAX 12

typedef struct MapSpr_s {
    short sprId;
    short sprIdShadow;
    int charNo;
    int masuId;
    BOOL unkC;
    BOOL unk10;
    GXColor color;
    short sprIdLine;
    short sprIdArrow;
    HuVecF pos;
    HuVecF posOrig;
} MAPSPR;

typedef struct MapScroll_s {
    MBMODELID focusMdlId;
    HuVecF pos;
    float unk10;
} MAPSCROLL;

static s16 scrollPlayer;
static HU3DMODELID scrollColMdlId;
static BOOL starScrollDone;
static float mapViewZoom;
static BOOL pauseDisableOld;
static MAPVIEWHOOK mapViewHook;
static int mapSprNum;
static BOOL mapDispAllF;
static ANIMDATA *masuAnim;
static ANIMDATA *pathAnim;
static int pathTime;
static int yajiSprNum;
static int mapSprTime;
static int lbl_80288B30;
static HUPROCESS *starScrollProc;
static HUPROCESS *scrollMainProc;

static u8 mapSprType[MAPSPR_MAX];
static MAPSPR mapSprWork[MAPSPR_MAX];
static MAPSCROLL mapScrollWork;
static HuVecF mapViewRot;
static HuVecF mapViewPos;

static MAPSCROLL *mapScrollWorkP = &mapScrollWork;

void MBScrollInit(unsigned int dataNum, float zoom, float posZ)
{
    if(dataNum == 0) {
        scrollColMdlId = HU3D_MODELID_NONE;
    } else {
        scrollColMdlId = Hu3DModelCreateData(dataNum);
        Hu3DModelDispOff(scrollColMdlId);
    }
    mapViewZoom = zoom;
    mapViewPos.x = mapViewPos.y = 0;
    mapViewPos.z = posZ;
    mapViewRot.x = -78;
    mapViewRot.y = 0;
    mapViewRot.z = 0;
    mapViewHook = NULL;
    scrollMainProc = NULL;
    starScrollProc = NULL;
    lbl_80288B30 = 0;
}

void MBScrollClose(void)
{
    if(scrollColMdlId >= 0) {
        Hu3DModelKill(scrollColMdlId);
        scrollColMdlId = HU3D_MODELID_NONE;
    }
}

static void ScrollMain(void);

void MBScrollExec(int playerNo)
{
    pauseDisableOld = MBPauseDisableGet();
    MBPauseDisableSet(TRUE);
    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
        scrollPlayer = playerNo;
        memset(mapScrollWorkP, 0, sizeof(MAPSCROLL));
        mapScrollWorkP->focusMdlId = MB_MODEL_NONE;
        scrollMainProc = MBPrcCreate(ScrollMain, 8201, 16384);
        MBPrcDestructorSet(scrollMainProc, NULL);
        while(scrollMainProc != NULL) {
            HuPrcVSleep();
        }
    } else {
        MBMapViewExec(playerNo);
    }
    MBPauseDisableSet(pauseDisableOld);
}

static void InitScroll(HuVecF *pos, float zoom);
static void RotateScrollView(MBCAMERA *camera, HuVecF *pos, HuVecF *posOut);
static void UpdateScrollView(MBCAMERA *camera, HuVecF *pos, BOOL smoothColF);
static void KillScroll(void);
static BOOL DoScrollCol(HuVecF *target, HuVecF *pos, HuVecF *endPos);
static void DoScrollColSmooth(HuVecF *dir, HuVecF *pos1, HuVecF *pos2, HuVecF *endPos);

static void ExecViewMap(void);

static void ScrollMain(void)
{
    int mode = 0;
    s16 masuId;
    BOOL exitF;
    float stkX;
    float stkY;
    float speed;
    float maxSpeed;
    HuVecF pos;
    HuVecF starPos;
    HuVecF scrollPos;
    HuVecF scrollDir;
    
    MBMusParamSet(MB_MUS_CHAN_BG, 96, 500);
    MBPlayerPosGet(scrollPlayer, &pos);
    InitScroll(&pos, 3000);
    exitF = FALSE;
    masuId = MBMasuPathMasuWrapGet(GwPlayer[scrollPlayer].masuId, 5, TRUE);
    if(masuId >= 0) {
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
        RotateScrollView(MBCameraGet(), &pos, &starPos);
    }
    MBStatusDispBackup();
    while(1) {
        if(MBStatusDispGet(scrollPlayer)) {
            MBStatusDispSetAll(FALSE);
            while(!MBStatusOffCheckAll()) {
                HuPrcVSleep();
            }
        }
        MBStatusDispFocusSet(scrollPlayer, TRUE);
        while(!MBStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        if(GWPartyFGet() != FALSE && !_CheckFlag(FLAG_MG_CIRCUIT)) {
            MBWinCreateHelp(MESS_BOARDOPE_PAD_SCROLL);
        } else {
            MBWinCreateHelp(MESS_BOARDOPE_PAD_SCROLL_CIRCUIT);
        }
        while(1) {
            s16 padNo = GwPlayer[scrollPlayer].padNo;
            u16 btn = MBPadBtnDown(padNo);
            if(btn == PAD_BUTTON_B) {
                exitF = TRUE;
                break;
            }
            if(btn == PAD_BUTTON_Y) {
                MBTopWinKill();
                ExecViewMap();
                HuPrcVSleep();
                break;
            }
            switch(mode) {
                case 0:
                    maxSpeed = (MBPadBtn(padNo) & PAD_BUTTON_A) ? 40.0f : 20.0f;
                    stkX = MBPadStkXGet(padNo);
                    stkY = -((float)MBPadStkYGet(padNo));
                    speed = HuMagPoint2D(stkX, stkY);
                    if(speed > 0) {
                        stkX /= speed;
                        stkY /= speed;
                        mapScrollWorkP->pos.x += stkX*maxSpeed;
                        mapScrollWorkP->pos.z += stkY*maxSpeed;
                    }
                    UpdateScrollView(MBCameraGet(), &mapScrollWorkP->pos, TRUE);
                    if(MBPadTrigR(padNo) > 8 && masuId >= 0) {
                        mode = 1;
                        scrollPos = mapScrollWorkP->pos;
                    }
                    break;
                
                case 1:
                    if(MBPadTrigR(padNo) > 8) {
                        VECSubtract(&starPos, &scrollPos, &scrollDir);
                        if(VECMag(&scrollDir) < 50) {
                            pos = starPos;
                        } else {
                            VECNormalize(&scrollDir, &scrollDir);
                            scrollPos.x += 50*scrollDir.x;
                            scrollPos.z += 50*scrollDir.z;
                        }
                    } else {
                        VECSubtract(&mapScrollWorkP->pos, &scrollPos, &scrollDir);
                        if(VECMag(&scrollDir) < 4*50) {
                            pos = mapScrollWorkP->pos;
                            mode = 0;
                        } else {
                            VECNormalize(&scrollDir, &scrollDir);
                            scrollPos.x += 4*(50*scrollDir.x);
                            scrollPos.z += 4*(50*scrollDir.z);
                        }
                    }
                    UpdateScrollView(MBCameraGet(), &scrollPos, TRUE);
                    break;
            }
            HuPrcVSleep();
        }
        if(exitF) {
            break;
        }
    }
    MBTopWinKill();
    KillScroll();
    MBStatusDispFocusSet(scrollPlayer, FALSE);
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MBStatusDispRestore();
    while(!MBStatusOffCheckAll()) {
        HuPrcVSleep();
    }
    MBMusParamSet(MB_MUS_CHAN_BG, 127, 500);
    scrollMainProc = NULL;
    HuPrcEnd();
}

static void UpdateScrollView(MBCAMERA *camera, HuVecF *pos, BOOL smoothColF)
{
    HuVecF target;
    HuVecF posOut;
    HuVecF dir;
    dir.x = (HuSin(camera->rot.y) * HuCos(camera->rot.x));
    dir.y = -HuSin(camera->rot.x);
    dir.z = (HuCos(camera->rot.y) * HuCos(camera->rot.x));
    target.x = pos->x+(10000*dir.x);
    target.y = pos->y+(10000*dir.y);
    target.z = pos->z+(10000*dir.z);
    if(!DoScrollCol(&target, pos, &posOut)) {
        if(smoothColF) {
            DoScrollColSmooth(&dir, pos, pos, &posOut);
        } else {
            return;
        }
    }
    MBModelPosSetV(mapScrollWorkP->focusMdlId, &posOut);
}


static void InitScroll(HuVecF *pos, float zoom)
{
    MBCAMERA camera;
    HuVecF rot = { -45, 0, 0 };
    MAPSCROLL *work = &mapScrollWork;
    MBCameraPush();
    work->focusMdlId = MBModelCreate(BOARD_HSF_coin, NULL, FALSE);
    MBModelDispSet(work->focusMdlId, FALSE);
    camera.rot.x = rot.x;
    camera.rot.y = rot.y;
    RotateScrollView(&camera, pos, &work->pos);
    UpdateScrollView(&camera, &work->pos, TRUE);
    MBCameraSkipSet(FALSE);
    MBCameraModelViewSet(work->focusMdlId, &rot, NULL, zoom, -1, 21);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
}

static void KillScroll()
{
    if(mapScrollWorkP->focusMdlId >= 0) {
        MBModelKill(mapScrollWorkP->focusMdlId);
        mapScrollWorkP->focusMdlId = MB_MODEL_NONE;
    }
    MBCameraPop();
    MBCameraSkipSet(TRUE);
}

static void RotateScrollView(MBCAMERA *camera, HuVecF *pos, HuVecF *posOut)
{
    posOut->x = pos->x+(HuSin(camera->rot.y)*(pos->y/(HuSin(camera->rot.x)/HuCos(camera->rot.x))));
    posOut->z = pos->z+(HuCos(camera->rot.y)*(pos->y/(HuSin(camera->rot.x)/HuCos(camera->rot.x))));
    posOut->y = 0;
}

static BOOL DoScrollCol(HuVecF *target, HuVecF *pos, HuVecF *endPos)
{
    float maxArea;
    float area;
    float triArea;
    
    HuVecF dir;
    HuVecF cross;
    HuVecF *vtxP[4];
    HuVecF edge;
    HuVecF up;
    HuVecF out;
    
    HSFFACE *faceP;
    HSFBUFFER *normBufP;
    HSFBUFFER *vtxBufP;
    HSFOBJECT *objP;
    HSFBUFFER *faceBufP;
    int i;
    HU3DMODEL *modelP;
    
    maxArea = -1;
    if(scrollColMdlId < 0) {
        return FALSE;
    }
    modelP = &Hu3DData[scrollColMdlId];
    objP = modelP->hsf->root;
    if(objP->type != HSF_OBJ_MESH) {
        return FALSE;
    }
    faceBufP = objP->mesh.face;
    vtxBufP = objP->mesh.vertex;
    normBufP = objP->mesh.normal;
    VECSubtract(pos, target, &dir);
    for(faceP = faceBufP->data, i=0; i<faceBufP->count; i++, faceP++) {
        if(faceP->type == HSF_FACE_TRI || faceP->type == HSF_FACE_QUAD) {
            vtxP[0] = ((HuVecF *)vtxBufP->data)+faceP->index[0].vertex;
            triArea = (faceP->nbt[0]*vtxP[0]->x)+(faceP->nbt[1]*vtxP[0]->y)+(faceP->nbt[2]*vtxP[0]->z);
            area = ((triArea-(faceP->nbt[0]*target->x))-(faceP->nbt[1]*target->y)-(faceP->nbt[2]*target->z))/((faceP->nbt[0]*dir.x)+(faceP->nbt[1]*dir.y)+(faceP->nbt[2]*dir.z));
            if(area < 0) {
                continue;
            }
            if(maxArea >= 0 && area >= maxArea) {
                continue;
            }
            out.x = target->x+(area*dir.x);
            out.y = target->y+(area*dir.y);
            out.z = target->z+(area*dir.z);
            if(faceP->type == HSF_FACE_TRI) {
                vtxP[1] = ((HuVecF *)vtxBufP->data)+faceP->index[1].vertex;
                vtxP[2] = ((HuVecF *)vtxBufP->data)+faceP->index[2].vertex;
                VECSubtract(vtxP[1], vtxP[0], &edge);
                VECSubtract(&out, vtxP[1], &up);
                VECCrossProduct(&edge, &up, &cross);
                if(cross.y < 0) {
                    continue;
                }
                VECSubtract(vtxP[2], vtxP[1], &edge);
                VECSubtract(&out, vtxP[2], &up);
                VECCrossProduct(&edge, &up, &cross);
                if(cross.y < 0) {
                    continue;
                }
                VECSubtract(vtxP[0], vtxP[2], &edge);
                VECSubtract(&out, vtxP[0], &up);
                VECCrossProduct(&edge, &up, &cross);
                if(cross.y < 0) {
                    continue;
                }
            } else {
                vtxP[1] = ((HuVecF *)vtxBufP->data)+faceP->index[1].vertex;
                vtxP[2] = ((HuVecF *)vtxBufP->data)+faceP->index[2].vertex;
                vtxP[3] = ((HuVecF *)vtxBufP->data)+faceP->index[3].vertex;
                VECSubtract(vtxP[1], vtxP[0], &edge);
                VECSubtract(&out, vtxP[1], &up);
                VECCrossProduct(&edge, &up, &cross);
                if(cross.y < 0) {
                    continue;
                }
                VECSubtract(vtxP[2], vtxP[1], &edge);
                VECSubtract(&out, vtxP[2], &up);
                VECCrossProduct(&edge, &up, &cross);
                if(cross.y < 0) {
                    continue;
                }
                VECSubtract(vtxP[3], vtxP[2], &edge);
                VECSubtract(&out, vtxP[3], &up);
                VECCrossProduct(&edge, &up, &cross);
                if(cross.y < 0) {
                    continue;
                }
                VECSubtract(vtxP[0], vtxP[3], &edge);
                VECSubtract(&out, vtxP[0], &up);
                VECCrossProduct(&edge, &up, &cross);
                if(cross.y < 0) {
                    continue;
                }
            }
            maxArea = area;
        }
    }
    if(maxArea >= 0) {
        endPos->x = target->x+(maxArea*dir.x);
        endPos->y = target->y+(maxArea*dir.y);
        endPos->z = target->z+(maxArea*dir.z);
        return TRUE;
    } else {
        return FALSE;
    }
}

static void DoScrollColSmooth(HuVecF *dir, HuVecF *pos1, HuVecF *pos2, HuVecF *endPos)
{
    HSFFACE *faceP;
    int i;
    HSFBUFFER *vtxBufP;
    int faceNo;
    HSFOBJECT *objP;
    HSFBUFFER *faceBufP;
    int no;
    HU3DMODEL *modelP;
    
    float scale;
    float mag;
    float xzMag;
    float minMag;
    float scaleY;
    
    HuVecF inVtx[3];
    HuVecF edge;
    HuVecF edge2;
    HuVecF outPos2;
    
    faceNo = -1;
    if(scrollColMdlId < 0) {
        return;
    }
    modelP = &Hu3DData[scrollColMdlId];
    objP = modelP->hsf->root;
    if(objP->type != HSF_OBJ_MESH) {
        return;
    }
    faceBufP = objP->mesh.face;
    vtxBufP = objP->mesh.vertex;
    
    for(faceP = faceBufP->data, no=0; no<faceBufP->count; no++, faceP++) {
        if(faceP->type != HSF_FACE_TRI) {
            continue;
        }
        if(VECDotProduct(dir, (HuVecF *)faceP->nbt) < 0) {
            continue;
        }
        for(i=0; i<3; i++) {
            xzMag = HuMagXZVecF(dir); 
            scaleY = (((HuVecF *)vtxBufP->data)+faceP->index[i].vertex)->y/(dir->y/xzMag);
            inVtx[i].x = (((HuVecF *)vtxBufP->data)+faceP->index[i].vertex)->x-(scaleY*(dir->x/xzMag));
            inVtx[i].z = (((HuVecF *)vtxBufP->data)+faceP->index[i].vertex)->z-(scaleY*(dir->z/xzMag));
            inVtx[i].y = 0;
        }
        for(i=0; i<3; i++) {
            int nextVtx = (i+1)%3;
            VECSubtract(&inVtx[nextVtx], &inVtx[i], &edge);
            scale = ((pos1->x*edge.x)-(edge.x*inVtx[i].x)+(pos1->y*edge.y)-(edge.y*inVtx[i].y)+(pos1->z*edge.z)-(edge.z*inVtx[i].z))/VECSquareMag(&edge);
            if(scale >= 0 && scale < 1) {
                edge2.x = inVtx[i].x+(scale*edge.x);
                edge2.y = inVtx[i].y+(scale*edge.y);
                edge2.z = inVtx[i].z+(scale*edge.z);
                VECSubtract(&edge2, pos1, &edge);
                mag = VECMag(&edge);
                if(faceNo < 0 || mag < minMag) {
                    faceNo = no;
                    minMag = mag;
                    outPos2 = edge2;
                }
            }
        }
        for(i=0; i<3; i++) {
            VECSubtract(&inVtx[i], pos1, &edge);
            mag = VECMag(&edge);
            if(faceNo < 0 || mag < minMag) {
                faceNo = no;
                minMag = mag;
                outPos2 = inVtx[i];
            }
        }
    }
    if(faceNo >= 0) {
        HuVecF *vtxP;
        float dot;
        faceP = ((HSFFACE *)faceBufP->data)+faceNo;
        vtxP = ((HuVecF *)vtxBufP->data)+faceP->index[0].vertex;
        dot = (faceP->nbt[0]*vtxP->x)+(faceP->nbt[1]*vtxP->y)+(faceP->nbt[2]*vtxP->z);
        scale = ((dot-(faceP->nbt[0]*outPos2.x))-(faceP->nbt[1]*outPos2.y)-(faceP->nbt[2]*outPos2.z))/((faceP->nbt[0]*dir->x)+(faceP->nbt[1]*dir->y)+(faceP->nbt[2]*dir->z));
        endPos->x = outPos2.x+(scale*dir->x);
        endPos->y = outPos2.y+(scale*dir->y);
        endPos->z = outPos2.z+(scale*dir->z);
        pos2->x = outPos2.x;
        pos2->y = outPos2.y;
        pos2->z = outPos2.z;
    }
}

typedef struct StarScroll_s {
    HuVecF startPos;
    HuVecF endPos;
    int time;
    int maxTime;
} STARSCROLL;

static void StarScrollMain(void);
static void StarScrollDestroy(void);

void MBStarScrollCreate(HuVecF *startPos, HuVecF *endPos, s16 maxTime)
{
    STARSCROLL *work;
    starScrollProc = MBPrcCreate(StarScrollMain, 8201, 16384);
    MBPrcDestructorSet(starScrollProc, StarScrollDestroy);
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(STARSCROLL), HU_MEMNUM_OVL);
    starScrollProc->property = work;
    work->startPos.x = startPos->x;
    work->startPos.y = startPos->y;
    work->startPos.z = startPos->z;
    work->endPos.x = endPos->x;
    work->endPos.y = endPos->y;
    work->endPos.z = endPos->z;
    if(maxTime < 0) {
        maxTime = 120;
    }
    work->time = work->maxTime = maxTime;
}

static void StarScrollMain(void)
{
    HUPROCESS *proc = HuPrcCurrentGet();
    STARSCROLL *work = proc->property;
    starScrollDone = TRUE;
    InitScroll(&work->startPos, 6000);
    RotateScrollView(MBCameraGet(), &work->endPos, &work->endPos);
    while(starScrollDone == TRUE) {
        HuPrcVSleep();
    }
    while(work->time > 0) {
        float weight = 1-((float)work->time/work->maxTime);
        mapScrollWorkP->pos.x = work->startPos.x+(weight*(work->endPos.x-work->startPos.x));
        mapScrollWorkP->pos.y = work->startPos.y+(weight*(work->endPos.y-work->startPos.y));
        mapScrollWorkP->pos.z = work->startPos.z+(weight*(work->endPos.z-work->startPos.z));
        UpdateScrollView(MBCameraGet(), &mapScrollWorkP->pos, FALSE);
        HuPrcVSleep(1);
        work->time--;
    }
    MBCameraMotionWait();
    starScrollDone = TRUE;
    HuPrcSleep(-1);
}

static void StarScrollDestroy(void)
{
    HUPROCESS *proc = HuPrcCurrentGet();
    KillScroll();
    HuMemDirectFree(proc->property);
    starScrollProc = NULL;
}

BOOL MBStarScrollDoneCheck(void)
{
    return starScrollDone;
}

void MBStarScrollStart(void)
{
    starScrollDone = FALSE;
}

void MBStarScrollKill(void)
{
    if(starScrollProc != NULL) {
        HuPrcKill(starScrollProc);
    }
}

void MBMapViewExec(int playerNo)
{
    scrollPlayer = playerNo;
    ExecViewMap();
}

static void SetMapCamera(void);
static void MasuDraw(HU3DMODEL *modelP, Mtx *mtx);
static void InitMapSpr(void);
static void ResetMapSpr(void);
static void UpdateMapSpr(void);
static void ResetMapCamera(void);

static void ExecViewMap(void)
{
    int i;
    HU3DMODELID mdlId;
    MBStatusDispForceSetAll(FALSE);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 60);
    WipeWait();
    SetMapCamera();
    if(mapViewHook) {
        mapViewHook(TRUE);
    }
    MBMasuDispSet(FALSE);
    masuAnim = HuSprAnimDataRead(BOARD_ANM_masuMapView);
    pathAnim = HuSprAnimDataRead(BOARD_ANM_mapViewPath);
    mdlId = Hu3DHookFuncCreate(MasuDraw);
    Hu3DModelCameraSet(mdlId, HU3D_CAM0);
    pathTime = 0;
    Hu3DModelLayerSet(mdlId, 6);
    memset(&mapSprWork[0], 0, MAPSPR_MAX*sizeof(MAPSPR));
    for(i=0; i<MAPSPR_MAX; i++) {
        mapSprWork[i].sprId = mapSprWork[i].sprIdShadow = -1;
    }
    mapDispAllF = TRUE;
    InitMapSpr();
    mapSprTime = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerDispSet(i, FALSE);
    }
    MBEffFadeCreate(1, 128);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 12);
    WipeWait();
    MBWinCreateHelp(MESS_BOARDOPE_PAD_SWITCH);
    MBTopWinPosSet(216, 384);
    while(1) {
        int padNo = GwPlayer[scrollPlayer].padNo;
        u16 btn = MBPadBtnDown(padNo);
        if(btn & PAD_BUTTON_A) {
            mapDispAllF ^= 1;
            ResetMapSpr();
            InitMapSpr();
        }
        if(btn == PAD_BUTTON_B) {
            break;
        }
        UpdateMapSpr();
        mapSprTime++;
        HuPrcVSleep();
    }
    MBTopWinKill();
    ResetMapCamera();
    MBMasuDispSet(TRUE);
    if(mdlId >= 0) {
        Hu3DModelKill(mdlId);
    }
    if(masuAnim != NULL) {
        HuSprAnimKill(masuAnim);
    }
    if(pathAnim != NULL) {
        HuSprAnimKill(pathAnim);
    }
    ResetMapSpr();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerDispSet(i, TRUE);
    }
    MBEffFadeOutSet(1);
}

static void SetMapCamera(void)
{
    static const HuVecF offset = { 0, 0, 0 };
    
    MBCameraPush();
    MBCameraNearFarSet(10, 30000);
    MBCameraSkipSet(FALSE);
    MBCameraFocusSet(-1);
    MBCameraPosViewSet(&mapViewPos, &mapViewRot, (HuVecF *)&offset, mapViewZoom, -1, 1);
    MBCameraMotionWait();
}

static void ResetMapCamera(void)
{
    MBCameraPop();
    if(mapViewHook) {
        mapViewHook(FALSE);
    }
}

static int InitYajiSpr(int no, u8 *playerNo);

static void YajiGetXPos(float baseX, float *xPos, int playerNum)
{
    
}

static void InitMapSpr(void)
{
    MAPSPR *work = &mapSprWork[0];
    int no;
    int i;
    int j;
    int yajiPlayerNum;
    MAPSPR *workOther;
    int temp;
    u8 starFlag;
    
    static const int charFileTbl[CHARNO_MAX] = {
        BOARD_ANM_statusKaoMario,
        BOARD_ANM_statusKaoLuigi,
        BOARD_ANM_statusKaoPeach,
        BOARD_ANM_statusKaoYoshi,
        BOARD_ANM_statusKaoWario,
        BOARD_ANM_statusKaoDaisy,
        BOARD_ANM_statusKaoWaluigi,
        BOARD_ANM_statusKaoKinopio,
        BOARD_ANM_statusKaoTeresa,
        BOARD_ANM_statusKaoMinikoopa,
        BOARD_ANM_statusKaoMinikoopaR,
        BOARD_ANM_statusKaoMinikoopaG,
        BOARD_ANM_statusKaoMinikoopaB
    };
    
    HuVecF yajiDir;
    u8 yajiPlayerNo[MAPSPR_MAX];
    
    float posX;
    float len;
    float scaleY;
    float posY;
    
    if(mapDispAllF) {
        HuVecF masuPos;
        for(no=0; no<GW_PLAYER_MAX; no++) {
            if(!MBPlayerAliveCheck(no)) {
                continue;
            }
            work->charNo = GwPlayer[no].charNo;
            work->sprId = espEntry(charFileTbl[work->charNo], 2000, 0);
            work->sprIdShadow = -1;
            work->masuId = GwPlayer[no].masuId;
            work->color = MBPlayerColorGet(no);
            work->unk10 = FALSE;
            MBMasuPosGet(MASU_LAYER_DEFAULT, work->masuId, &masuPos);
            Hu3D3Dto2D(&masuPos, HU3D_CAM0, &work->pos);
            work->posOrig = work->pos;
            work->posOrig.y = (work->pos.y > 240) ? (work->pos.y-64) : (work->pos.y+64);
            work++;
        }
    } else {
        HuVecF masuPos;
        work->charNo = GwPlayer[scrollPlayer].charNo;
        work->sprId = espEntry(charFileTbl[work->charNo], 2000, 0);
        work->sprIdShadow = -1;
        work->masuId = GwPlayer[scrollPlayer].masuId;
        work->color = MBPlayerColorGet(scrollPlayer);
        work->unk10 = FALSE;
        MBMasuPosGet(MASU_LAYER_DEFAULT, work->masuId, &masuPos);
        Hu3D3Dto2D(&masuPos, HU3D_CAM0, &work->pos);
        work->posOrig = work->pos;
        work->posOrig.y = (work->pos.y > 240) ? (work->pos.y-64) : (work->pos.y+64);
        work++;
    }
    starFlag = MBStarFlagGet();
    for(no=0; no<8; no++) {
        static GXColor color = { 255, 255, 192, 0 };
        HuVecF masuPos;
        if(starFlag & (1 << no)) {
            work->charNo = CHARNO_NONE;
            work->sprId = espEntry(BOARD_ANM_mapViewStar, 2500, 0);
            work->sprIdShadow = espEntry(BOARD_ANM_mapViewStar, 1900, 0);
            work->masuId = MBMasuStarGet(no);
            work->color = color;
            work->unk10 = FALSE;
            MBMasuPosGet(MASU_LAYER_DEFAULT, work->masuId, &masuPos);
            Hu3D3Dto2D(&masuPos, HU3D_CAM0, &work->pos);
            work->posOrig = work->pos;
            work->posOrig.y = (work->pos.y > 240) ? (work->pos.y-64) : (work->pos.y+64);
            espPosSet(work->sprId, work->pos.x, work->pos.y);
            espScaleSet(work->sprId, 0.6f, 0.6f);
            espPosSet(work->sprIdShadow, work->pos.x, work->pos.y);
            espScaleSet(work->sprIdShadow, 0.6f, 0.6f);
            espTPLvlSet(work->sprIdShadow, 0.5f);
            work->posOrig = work->pos;
            work->sprIdLine = work->sprIdArrow = -1;
            work++;
        }
    }
    mapSprNum = work-mapSprWork;
    work = mapSprWork;
    for(no=0; no<mapSprNum; no++, work++) {
        if(work->charNo >= 0) {
            mapSprType[no] = MAPSPR_PLAYER;
        } else {
            mapSprType[no] = MAPSPR_STAR;
        }
    }
    work = mapSprWork;
    for(no=0; no<mapSprNum; no++, work++) {
        if(mapSprType[no] == MAPSPR_PLAYER) {
            yajiPlayerNum = InitYajiSpr(no, yajiPlayerNo);
            for(i=0; i<yajiPlayerNum-1; i++) { 
                for(j=i+1; j<yajiPlayerNum; j++) {
                    if(mapSprWork[yajiPlayerNo[i]].posOrig.x > mapSprWork[yajiPlayerNo[j]].posOrig.x) {
                        temp = yajiPlayerNo[i];
                        yajiPlayerNo[i] = yajiPlayerNo[j];
                        yajiPlayerNo[j] = temp;
                    }
                }
            }
            {
                float baseX = mapSprWork[yajiPlayerNo[0]].posOrig.x+(0.5f*(mapSprWork[yajiPlayerNo[yajiPlayerNum-1]].posOrig.x-mapSprWork[yajiPlayerNo[0]].posOrig.x));
                posX = baseX-(32.0f*(yajiPlayerNum-1));
                if(posX < 64) {
                    posX = 64;
                }        
                if(posX+(64.0f*(yajiPlayerNum-1)) > 512) {
                    posX = 512-(64.0f*(yajiPlayerNum-1));
                }
            }
            for(i=0; i<yajiPlayerNum; i++) { 
                workOther = &mapSprWork[yajiPlayerNo[i]];
                workOther->posOrig.x = posX+(64.0f*i);
            }
        }
        
    }
    work = mapSprWork;
    for(no=0; no<mapSprNum; no++, work++) {
        if(work->charNo >= 0) {
            work->sprIdLine = espEntry(BOARD_ANM_mapViewYajiLine, 2020+(no*10), 0);
            work->sprIdArrow = espEntry(BOARD_ANM_mapViewYaji, 2020+(no*10), 0);
            espColorSet(work->sprIdLine, work->color.r, work->color.g, work->color.b);
            espColorSet(work->sprIdArrow, work->color.r, work->color.g, work->color.b);
            posX = work->posOrig.x;
            posY = work->posOrig.y;
            espPosSet(work->sprId, posX, posY);
            yajiDir.x = work->posOrig.x-work->pos.x;
            yajiDir.y = work->posOrig.y-work->pos.y;
            len = HuMagXYVecF(&yajiDir);
            scaleY = (len-8)/8;
            if(scaleY < 0) {
                scaleY = 0;
            }
            espPosSet(work->sprIdLine, work->pos.x+(0.5f*yajiDir.x), work->pos.y+(0.5f*yajiDir.y));
            espZRotSet(work->sprIdLine, HuAtan(yajiDir.x, -yajiDir.y));
            espScaleSet(work->sprIdLine, 1, scaleY);
            espPosSet(work->sprIdArrow, work->pos.x+((7.0f*yajiDir.x)/len), work->pos.y+((7.0f*yajiDir.y)/len));
            espZRotSet(work->sprIdArrow, HuAtan(yajiDir.x, -yajiDir.y));
        }
    }
    
}

static int InitYajiSpr(int no, u8 *playerNo)
{
    HuVecF pos;
    HuVecF ofs = { 64, 64, 0 };
    int i;
    
    BOOL repeatF;
    yajiSprNum = 0;
    playerNo[yajiSprNum++] = no;
    mapSprType[no] = MAPSPR_STAR;
    pos = mapSprWork[no].posOrig;
    while(1) {
        repeatF = FALSE;
        for(i=0; i<mapSprNum; i++) {
            if(mapSprType[i] == MAPSPR_PLAYER) {
                HuVecF dir;
                VECSubtract(&mapSprWork[i].posOrig, &pos, &dir);
                if(fabs(dir.x) < ofs.x && fabs(dir.y) < ofs.y) {
                    ofs.x += 64;
                    ofs.y += 64;
                    pos.x += dir.x*0.5f;
                    pos.y += dir.y*0.5f;
                    mapSprType[i] = MAPSPR_STAR;
                    playerNo[yajiSprNum++] = i;
                    repeatF = TRUE;
                    break;
                }
            }
        }
        if(!repeatF) {
            break;
        }
    }
    return yajiSprNum;
}

static void UpdateMapSpr(void)
{
    float angle = mapSprTime/60.0f;
    float zRot = HuSin(360*angle)*20.0;
    MAPSPR *work;
    int i;
    for(work=mapSprWork, i=0; i<mapSprNum; i++, work++) {
        if(work->charNo < 0) {
            espZRotSet(work->sprId, zRot);
            espZRotSet(work->sprIdShadow, zRot);
        }
    }
}

static void ResetMapSpr(void)
{
    MAPSPR *work;
    int i;
    for(work=mapSprWork, i=0; i<mapSprNum; i++, work++) {
        if(work->sprId >= 0) {
            espKill(work->sprId);
        }
        if(work->sprIdShadow >= 0) {
            espKill(work->sprIdShadow);
        }
        if(work->sprIdLine >= 0) {
            espKill(work->sprIdLine);
        }
        if(work->sprIdArrow >= 0) {
            espKill(work->sprIdArrow);
        }
    }
}

static void CameraProject(Mtx mtx, HuVecF *in, HuVecF *out)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    HuVecF posCamera;
    float x, y;
    MTXMultVec(mtx, in, &posCamera);
    x = posCamera.z*(HuSin(cameraP->fov/2)/HuCos(cameraP->fov/2))*1.2f;
    y = posCamera.z*(HuSin(cameraP->fov/2)/HuCos(cameraP->fov/2));
    out->x = 288+(posCamera.x*(288/(-x)));
    out->y = 240+(posCamera.y*(240/(y)));
    out->z = 0;
}

static void PathDraw(HuVecF *end, HuVecF *start)
{
    
}

static void MasuDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    Mtx modelview;
    Mtx trans;
    Mtx uvMtx;
    Mtx44 proj;
    HuVecF pos;
    HuVecF posEnd;
    HuVecF pathDir;
    static GXColor colorN = { 255, 255, 255, 255 };
    MASU *masuP;
    int i;
    MASU *masuLink;
    int masuSize;
    int j;
    
    float pathScroll;
    float pathYScale;
    float uvX;
    float uvY;
    float pathX;
    float pathY;
    float pathLen;
    
    
    MTXOrtho(proj, 0, 480, 0, 576, 0, 100);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    HuSprTexLoad(masuAnim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    HuSprTexLoad(pathAnim, 0, GX_TEXMAP1, GX_CLAMP, GX_REPEAT, GX_LINEAR);
    GXSetTevColor(GX_TEVREG0, colorN);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetNumTevStages(1);
    
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanAmbColor(GX_COLOR0A0, colorN);
    GXSetChanMatColor(GX_COLOR0A0, colorN);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    masuSize = (GWPartyFGet() == FALSE) ? 12 : 8;
    if(pathTime > 30u) {
        pathTime = 0;
    }
    pathScroll = (pathTime++)/30.0f;
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    for(i=0; i<MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, i+1);
        if(masuP->flag & MBMasuDispMaskGet()) {
            continue;
        }
        if(masuP->flag & MBBranchFlagGet()) {
            continue;
        }
        CameraProject(*mtx, &masuP->pos, &pos);
        MTXTrans(trans, pos.x, pos.y, 0);
        for(j=0; j<masuP->linkNum; j++) {
            masuLink = MBMasuGet(MASU_LAYER_DEFAULT, masuP->linkTbl[j]);
            if(masuLink->flag & MASU_FLAG_BLOCKL) {
                continue;
            }
            if(masuLink->flag & MASU_FLAG_BLOCKR) {
                continue;
            }
            if(masuLink->flag & MBBranchFlagGet()) {
                continue;
            }
            if(!_CheckFlag(FLAG_MG_CIRCUIT) || (masuLink->flag & MASU_FLAG_JUMPFROM)) {
                CameraProject(*mtx, &masuLink->pos, &posEnd);
                VECSubtract(&posEnd, &pos, &pathDir);
                pathDir.z = 0;
                pathLen = VECMag(&pathDir);
                pathYScale = pathLen/16;
                pathX = pos.x+(0.5f*pathDir.x);
                pathY = pos.y+(0.5f*pathDir.y);
                mtxRot(modelview, 0, 0, HuAtan(pathDir.x, -pathDir.y));
                mtxScaleCat(modelview, 1, pathYScale, 0);
                mtxTransCat(modelview, pathX, pathY, 0);
                GXLoadPosMtxImm(modelview, GX_PNMTX0);
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition2f32(-4, -8);
                GXTexCoord2f32(0, pathScroll);
                GXPosition2f32(4, -8);
                GXTexCoord2f32(1, pathScroll);
                GXPosition2f32(4, 8);
                GXTexCoord2f32(1, pathScroll+pathYScale);
                GXPosition2f32(-4, 8);
                GXTexCoord2f32(0, pathScroll+pathYScale);
                GXEnd();
            }
        }
    }
    if(!MBMasuDispGet()) {
        return;
    }
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    for(i=0; i<MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        static int patNoTbl[9] = {
            -1, 0, 1,
            3, 2, 5,
            2, 6, 4
        };
        int patNo;
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, i+1);
        if(masuP->type == MASU_TYPE_NONE) {
            continue;
        }
        if(masuP->flag & MBMasuDispMaskGet()) {
            continue;
        }
        patNo = patNoTbl[masuP->type];
        uvX = (patNo%4)/4.0f;
        uvY = (patNo/4)/2.0f;
        MTXScale(uvMtx, 0.25f, 0.5f, 0);
        mtxTransCat(uvMtx, uvX, uvY, 0);
        GXLoadTexMtxImm(uvMtx, GX_TEXMTX0, GX_MTX2x4);
        CameraProject(*mtx, &masuP->pos, &pos);
        MTXTrans(trans, pos.x, pos.y, 0);
        GXLoadPosMtxImm(trans, GX_PNMTX0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(-masuSize, -masuSize);
        GXTexCoord2f32(0, 0);
        GXPosition2f32(masuSize, -masuSize);
        GXTexCoord2f32(1, 0);
        GXPosition2f32(masuSize, masuSize);
        GXTexCoord2f32(1, 1);
        GXPosition2f32(-masuSize, masuSize);
        GXTexCoord2f32(0, 1);
        GXEnd();
    }
}

void MBMapViewCameraSet(HuVecF *rot, HuVecF *pos, float zoom)
{
    if(rot) {
        mapViewRot = *rot;
    }
    if(pos) {
        mapViewPos = *pos;
    }
    if(zoom >= 0) {
        mapViewZoom = zoom;
    }
}

void MBMapViewHookSet(MAPVIEWHOOK hook)
{
    mapViewHook = hook;
}