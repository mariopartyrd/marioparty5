#include "REL/m433Dll.h"
#include "game/gamemes.h"
#include "game/mgdata.h"
#include "game/wipe.h"

void M433Init(OMOBJ *obj);
void M433Main(OMOBJ *obj);
void M433GameStart(OMOBJ *obj);
void M433GameExitWait(OMOBJ *obj);
void M433GameExit(OMOBJ *obj);
void M433GameWait(OMOBJ *obj);
void M433GameFinish(OMOBJ *obj);
void M433GameWin(OMOBJ *obj);

static HuVecF lightPos = { 100, 800, -100 };
static HuVecF lightDir = { 0.3f, -0.8f, 0.3f };
static HuVecF lightAimPos = { 10, 45, 3500 };
static GXColor lightColor = { 255, 255, 255, 255 };
static HuVecF shadowPos = { 50, 150000, 50 };
static HuVecF shadowUp = { 0, 1, 0 };
static HuVecF shadowTarget = { 0, 0, 0 };
static HuVecF cameraFocusPos = { 0, 170, 0 };
static HuVecF cameraFocusRot = { 0, -22, 850 };

static OMOBJMAN *objman;
static OMOBJ *gameObj;
static OMOBJ *outview;
static int lbl_1_bss_28[2];

static HU3DLIGHTID lightId;

int gameMaxScore;
int gameBallNumMax;
int gameType;

static GMESID startGMesId, finishGMesId, winGMesId;
static int gameStartSeNo, gameFinishSeNo;
static int gameMusNo, winMusNo;


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

void ObjectSetup(void)
{
    OMOBJMAN *om;
    HU3DLIGHT *lp;
    HuVecF aim;
    HuVecF pos;
    
    HuAudSndGrpSetSet(MSM_GRP_MG433);
    gameStartSeNo = gameFinishSeNo = MSM_SENO_NONE;
    gameMusNo = winMusNo = MSM_STREAMNO_NONE;
    shadowPos.x = 200;
    shadowPos.y = 1200;
    shadowPos.z = 160;
    shadowUp.x = 0;
    shadowUp.y = 1;
    shadowUp.z = 0;
    shadowTarget.x = 0;
    shadowTarget.y = 0;
    shadowTarget.z = 0;
    Hu3DShadowCreate(30, 20, 25000);
    Hu3DShadowTPLvlSet(0.625f);
    Hu3DShadowColSet(55, 60, 110);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    lightId = Hu3DGLightCreateV(&lightPos, &lightDir, &lightColor);
    Hu3DGLightInfinitytSet(lightId);
    lp = &Hu3DGlobalLight[lightId];
    lp->type |= HU3D_LIGHT_TYPE_STATIC;
    lightAimPos.x = 55.74f;
    lightAimPos.y = 0;
    lightAimPos.z = 3500;
    aim.x = aim.y = aim.z = 0;
    pos.x = pos.z = 0;
    pos.y = lightAimPos.z;
    pos.x = 0;
    pos.y = 3480.0002f;
    pos.z = 900.00006f;
    Hu3DGLightPosAimSetV(lightId, &pos, &aim);
    Hu3DShadowPosSet(&pos, &shadowUp, &aim);
    startGMesId = finishGMesId = winGMesId = GMES_ID_NONE;
    om = omInitObjMan(50, 8192);
    objman = om;
    omGameSysInit(om);
    gameType = mgMatchGameType;
    gameMaxScore = mgMatchMaxScore & 0xFF;
    gameBallNumMax = mgMatchMaxScore >> 8;
    if(gameType < 0 || gameType >= 3) {
        gameType = 0;
    }
    if(gameMaxScore <= 0 || gameMaxScore > 99) {
        gameMaxScore = 15;
    }
    if(gameBallNumMax <= 0 || gameBallNumMax > 10) {
        gameBallNumMax = 10;
    }
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, -1, 10, 10000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    outview = omAddObj(om, 32730, 0, 0, omOutView);
    gameObj = omAddObj(om, 10, 0, 0, M433Init);
    M433StageInit(om);
    M433PlayerInit(om);
}

void M433Init(OMOBJ *obj)
{
    M433_GAMEWORK *work;
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M433_GAMEWORK), HU_MEMNUM_OVL);
    work = obj->data;
    memset(work, 0, sizeof(M433_GAMEWORK));
    work->mode = 0;
    work->modeTime = 0;
    work->cameraMode = 0;
    work->startTime = 0;
    work->gamemesMode = 0;
    work->gamemesTimer = 0;
    work->winMode = 0;
    work->winTimer = 0;
    work->modeActive = FALSE;
    work->winType = M433_WINTYPE_NONE;
    work->winFlag = 0;
    work->winFlag2 = 0;
    obj->objFunc = M433Main;
}

void M433ModeWatch(OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    work->modeTime++;
    work->startTime++;
    work->gamemesTimer++;
    work->winTimer++;
    if(omSysExitReq && work->mode != M433_MODE_EXIT) {
        work->mode = M433_MODE_EXITREQ;
        M433ModeUpdate(obj);
    }
}

void M433ModeUpdate(OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    if(work->modeActive) {
        return;
    }
    switch(work->mode) {
        case M433_MODE_INIT:
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
            Hu3DCameraPerspectiveSet(HU3D_CAM0, 41.5f, 10.0f, 10000.0f, 1.2f);
            gameStartSeNo = HuAudFXPlay(MSM_SE_M433_02);
            work->mode = M433_MODE_GAMESTART;
            work->startTime = 0;
            obj->objFunc = M433StartCameraCreate(objman, obj);
            break;
       
        case M433_MODE_GAMESTART:
            work->mode = M433_MODE_GAMEWIN;
            work->gamemesTimer = 0;
            obj->objFunc = M433GameStart;
            break;
        
        case M433_MODE_GAMEWIN:
            work->mode = M433_MODE_EXITAUTO;
            work->winTimer = 0;
            obj->objFunc = M433GameWinCreate(objman, obj);
            break;
       
        case M433_MODE_EXITAUTO:
        case M433_MODE_EXITREQ:
            work->modeActive = TRUE;
            work->mode = M433_MODE_EXIT;
            obj->objFunc = M433GameExitWait;
            break;
       
        case M433_MODE_EXIT:
        default:
            work->mode = M433_MODE_EXIT;
            obj->objFunc = M433GameExit;
            break;
    }
    work->modeTime = 0;
}

void M433Main(OMOBJ *obj)
{
    M433ModeWatch(obj);
    M433ModeUpdate(obj);
}

void M433GameExitWait(OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    M433ModeWatch(obj);
    if(WipeCheckEnd() == FALSE) {
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        work->modeActive = FALSE;
        M433ModeUpdate(obj);
    }
}

void M433GameExit(OMOBJ *obj)
{
    M433ModeWatch(obj);
    if(WipeCheckEnd() == FALSE) {
        M433PlayerClose();
        M433StageClose();
        GMesClose();
        HuAudFadeOut(1);
        omOvlReturn(1);
    }
}

void M433GameStart(OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    M433ModeWatch(obj);
    switch(work->gamemesMode) {
        case M433_GAMEMES_INIT:
            work->second = 30;
            work->frame = 60;
            startGMesId = GMesMgStartCreate();
            GMesPosSet(startGMesId, 320, 240);
            Center.x = 0;
            Center.y = 200;
            Center.z = 0;
            CRot.x = -30;
            CRot.y = 0;
            CRot.z = 0;
            CZoom = 1900;
            work->gamemesMode = M433_GAMEMES_START;
            work->gamemesTimer = 0;
            break;
     
        case M433_GAMEMES_START:
            if(gameMusNo < 0 && GMesFXPlayCheck(startGMesId)) {
                gameMusNo = HuAudSStreamPlay(MSM_STREAM_MGMUS_4);
            }
            if(!GMesStatGet(startGMesId) && !work->modeActive) {
                work->gamemesMode = M433_GAMEMES_WAIT;
                work->gamemesTimer = 0;
                obj->objFunc = M433GameWait;
            }
            break;
    }
}

void M433GameWait(OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    BOOL stopF = FALSE;
    M433ModeWatch(obj);
    work->frame++;
    if(--work->frame == 0) {
        work->frame = 60;
        work->second--;
        if(work->second == 0) {
            stopF = TRUE;
        }
    }
    if(work->winType != M433_WINTYPE_NONE) {
        stopF = TRUE;
    }
    if(stopF) {
        finishGMesId = GMesMgFinishCreate();
        GMesPosSet(finishGMesId, 320, 240);
        HuAudSStreamFadeOut(gameMusNo, 100);
        work->gamemesMode = M433_GAMEMES_FINISH;
        work->gamemesTimer = 0;
        if(!work->modeActive) {
            obj->objFunc = M433GameFinish;
        }
    }
}

void M433GameFinish(OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    M433ModeWatch(obj);
    if(!GMesStatGet(finishGMesId) && work->winFlag == 0x11111) {
        work->gamemesMode = M433_GAMEMES_CLOSE;
        work->gamemesTimer = 0;
        M433ModeUpdate(obj);
    }
}

OMOBJFUNC M433GameWinCreate(OMOBJMAN *om, OMOBJ *obj)
{
    return M433GameWin;
}

void M433GameWin(OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    float rotY = cameraFocusRot.y;
    if(M433WinGroupCheck(1)) {
        rotY *= -1;
    }
    Center.x += 0.15f * (cameraFocusPos.x-Center.x);
    Center.y += 0.15f * (cameraFocusPos.y-Center.y);
    Center.z += 0.15f * (cameraFocusPos.z-Center.z);
    CRot.x = M433AngleLerp(CRot.x, cameraFocusRot.x, 0.15f);
    CRot.y = M433AngleLerp(CRot.y, rotY, 0.15f);
    CZoom += 0.15f * (cameraFocusRot.z-CZoom);
    M433ModeWatch(obj);
    work->winMode = M433_WINMODE_NORMAL;
    if(winMusNo < 0) {
        winMusNo = HuAudJinglePlay(MSM_STREAM_JNGL_MG_WIN);
    }
    if(winGMesId < 0) {
        int winTbl[GW_PLAYER_MAX] = { CHARNO_NONE, CHARNO_NONE, CHARNO_NONE, CHARNO_NONE };
        int i;
        int num;
        int type;
        for(num=0, i=0; i<2; i++) {
            if(work->winPlayer[i] >= 0) {
                winTbl[num++] = GwPlayerConf[work->winPlayer[i]].charNo;
                GWMgCoinBonusAdd(work->winPlayer[i], 10);
            }
        }
        if(winTbl[0] == CHARNO_NONE) {
            winGMesId = GMesMgWinnerDrawCreate();
        } else {
            type = GMES_MG_TYPE_WIN;
            if(GWSubGameNoGet() == 0 && M433WinGroupCheck(0)) {
                type = GMES_MG_TYPE_CHAMPION;
            }
            winGMesId = GMesMgWinnerTypeCreate(type, winTbl[0], winTbl[1], winTbl[2], winTbl[3]);
        }
    } else if(!GMesStatGet(winGMesId) && work->winTimer >= 210.0f) {
        work->winMode = M433_WINMODE_NOSTOP;
        M433ModeUpdate(obj);
    }
}

unsigned int M433GameModeGet(void)
{
    return omObjGetDataAs(gameObj, M433_GAMEWORK)->mode;
}

unsigned int M433CameraModeGet(void)
{
    return omObjGetDataAs(gameObj, M433_GAMEWORK)->cameraMode;
}

unsigned int M433GamemesModeGet(void)
{
    return omObjGetDataAs(gameObj, M433_GAMEWORK)->gamemesMode;
}

unsigned int M433GameWinModeGet(void)
{
    return omObjGetDataAs(gameObj, M433_GAMEWORK)->winMode;
}

void M433GameWinnerSet(int type, int player1, int player2)
{
    M433_GAMEWORK *work = gameObj->data;
    work->winType = type;
    work->winPlayer[0] = player1;
    work->winPlayer[1] = player2;
}

int M433GameWinTypeGet(void)
{
    return omObjGetDataAs(gameObj, M433_GAMEWORK)->winType;
}

void M433GameWinFlagSet(int playerNo)
{
    omObjGetDataAs(gameObj, M433_GAMEWORK)->winFlag |= (1 << ((playerNo & 7) * 4));
}

void M433GameWinFlag2Set(int playerNo)
{
    omObjGetDataAs(gameObj, M433_GAMEWORK)->winFlag2 |= (1 << ((playerNo & 3) * 4));
}

BOOL M433GameFinishEndCheck(void)
{
    return GMesStatGet(finishGMesId) == 0;
}

void M433CameraAimSetV(HuVecF *eye, HuVecF *pos)
{
    HuVecF dir;
    VECSubtract(pos, eye, &dir);
    Center = *pos;
    CRot.x = HuAtan(dir.y, HuMagPoint2D(dir.x, dir.z));
    CRot.y = HuAtan(-dir.x, -dir.z);
    CRot.z = 0;
    CZoom = VECMag(&dir);
}

void M433CameraAimSet(float eyeX, float eyeY, float eyeZ, float posX, float posY, float posZ)
{
    HuVecF eye;
    HuVecF pos;
    eye.x = eyeX;
    eye.y = eyeY;
    eye.z = eyeZ;
    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;
    M433CameraAimSetV(&eye, &pos);
}

float M433AngleLerp(float start, float end, float time)
{
    float result = fmodf(end - start, 360.0);
    if (result < 0.0f) {
        result += 360.0f;
    }
    if (result > 180.0f) {
        result -= 360.0f;
    }
    result = fmodf(start + (time * result), 360.0);
    if (result < 0.0f) {
        result += 360.0f;
    }
    return result;
}

float M433AngleSub(float start, float end)
{
    float result = fmodf(end - start, 360.0);
    if (result < 0.0f) {
        result += 360.0f;
    }
    if (result > 180.0f) {
        result -= 360.0f;
    }
    return result;
}