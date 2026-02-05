#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "datanum/actman.h"

static void SeqModeInit(s16 mode, s16 frameNo);
static void SeqModeFadeIn(s16 mode, s16 frameNo);
static void SeqModeStart(s16 mode, s16 frameNo);
static void SeqModeMain(s16 mode, s16 frameNo);
static void SeqModeFinish(s16 mode, s16 frameNo);
static void SeqModePreWin(s16 mode, s16 frameNo);
static void SeqModeWin(s16 mode, s16 frameNo);
static void SeqModeFadeOut(s16 mode, s16 frameNo);
static void SeqModeClose(s16 mode, s16 frameNo);

static MGSEQ_PARAM seqParam = {
    0,
    MGSEQ_TIMER_TOP,
    SeqModeInit,
    SeqModeFadeIn,
    SeqModeStart,
    SeqModeMain,
    SeqModeFinish,
    SeqModePreWin,
    SeqModeWin,
    SeqModeFadeOut,
    SeqModeClose
};

static MGPLAYER *player[GW_PLAYER_MAX];
static OMOBJMAN *objman;

static void CreateMap(OMOBJ *obj);

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
    HU3DLIGHTID lightId;
    
    objman = MgActorObjectSetup();
    CRot.x = -20;
    CRot.y = 0;
    CRot.z = 0;
    Center.x = 0;
    Center.y = 150;
    Center.z = 0;
    CZoom = 1000;
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    omAddObj(objman, 32730, 0, 0, omOutView);
    
    lightId = Hu3DGLightCreate(0, 1000, 1000, 0, -1, -1, 255, 255, 255);
    omAddObj(objman, 10, 3, 0, CreateMap);
    MgSeqCreate(&seqParam);
}

static void CreateMap(OMOBJ *obj)
{
    MGACTOR_PARAM param;
    HU3DMODELID mdlId[4];
    
    obj->mdlId[0] = mdlId[0] = Hu3DModelCreateData(ACTMAN_HSF_ground);
    param.height = 150;
    param.radius = 40;
    param.param = 0;
    param.type = 0;
    param.attr = 0;
    param.correctHookParam = 0;
    param.narrowHook = NULL;
    param.correctHook = NULL;
    MgActorColMapInit(mdlId, 1, 40);
    player[0] = MgPlayerCreate(0, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[1] = MgPlayerCreate(1, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[2] = MgPlayerCreate(2, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[3] = MgPlayerCreate(3, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[0]->actor->pos.x = -300.211f;
    player[0]->actor->pos.y = 300;
    player[0]->actor->pos.z = -85.369f;
    player[1]->actor->pos.x = 200;
    player[1]->actor->pos.y = 300;
    player[1]->actor->pos.z = 0;
    player[2]->actor->pos.x = 4300;
    player[2]->actor->pos.y = 1000;
    player[2]->actor->pos.z = 0;
    player[3]->actor->pos.x = 400;
    player[3]->actor->pos.y = 300;
    player[3]->actor->pos.z = 110;
    obj->objFunc = NULL;
}

static void SeqModeInit(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
    MgActorExec();
}

static void SeqModeFadeIn(s16 mode, s16 frameNo)
{
    if(frameNo > 90) {
        MgSeqModeNext();
    }
}

static void SeqModeStart(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

static void SeqModeMain(s16 mode, s16 frameNo)
{
    MgActorExec();
    CZoom += HuPadTrigL[0]/2;
    CZoom -= HuPadTrigR[0]/2;
    Center.x = player[0]->actor->pos.x;
    Center.y = player[0]->actor->pos.y+100;
    Center.z = player[0]->actor->pos.z;
    if(HuPadBtnDown[0] & PAD_BUTTON_X) {
        MgSeqModeNext();
    }
}

static void SeqModeFinish(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

static void SeqModePreWin(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

static void SeqModeWin(s16 mode, s16 frameNo)
{
    if(frameNo == 1) {
        int motId = CharMotionCreate(player[0]->charNo, CHARMOT_HSF_c000m1_306);
        Hu3DModelPosSet(player[0]->actor->mdlId, Center.x, Center.y, Center.z);
        CharMotionShiftSet(player[0]->actor->mdlId, motId, 0, 5, HU3D_MOTATTR_LOOP);
    } else {
        Hu3DModelPosSet(player[0]->actor->mdlId, Center.x, Center.y+(frameNo*2), Center.z);
    }
}

static void SeqModeFadeOut(s16 mode, s16 frameNo)
{
    if(frameNo >= 60) {
        MgSeqModeNext();
    }
}

static void SeqModeClose(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}