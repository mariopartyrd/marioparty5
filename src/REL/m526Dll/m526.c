#include "REL/m526Dll/math.c"

#include "game/charman.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/esprite.h"
#include "game/wipe.h"
#include "game/gamemes.h"
#include "game/pad.h"

#include "msm.h"
#include "msm_stream.h"

#include "math.h"
#include "humath.h"

#include "datanum/m526.h"

typedef struct M526ObjProc_s {
    int prio;
    int stackSize;
} M526_OBJPROC;

typedef struct M526StageLight_s {
    HuVecF pos;
    HuVecF target;
    GXColor color;
} M526_STAGELIGHT;

typedef struct M526GameFlow_s {
    int mode;
    HUPROCESS *process;
    s16 sprId[1];
} M526_GAMEFLOW;

typedef struct M526Stage_s {
    int mode;
    HUPROCESS *process;
    HU3DMODELID modelId[8];
    s16 cameraBit;
    int side;
    HuVecF pos;
    HuVecF cloudPos[2];
} M526_STAGE;

typedef struct M526CameraView_s {
    HuVecF center;
    HuVecF rot;
    float zoom;
    HuVecF eye;
    HuVecF up;
} M526_CAMERAVIEW;

typedef struct M526Camera_s {
    int mode;
    HUPROCESS *process;
    M526_CAMERAVIEW view[3];
    int side;
    int moveSpeed;
    s16 cameraBit;
} M526_CAMERA;

typedef struct M526Fuse_s {
    int mode;
    int lane;
    int side;
    int delay;
    HUPROCESS *process;
    HU3DMODELID fuseMdlId;
    HU3DMODELID hibanaMdlId;
    HuVecF pos;
    HuVecF hookPos;
    float speed;
    BOOL doneF;
    BOOL stopF;
    int seId;
} M526_FUSE;

typedef struct M526Player_s {
    int mode;
    HUPROCESS *process;
    HuVecF pos;
    HuVecF walkPos;
    HuVecF rot;
    HuVecF vel;
    int unk38;
    BOOL comF;
    int playerNo;
    int side;
    int sidePlayerNo;
    int charNo;
    HU3DMODELID modelId;
    HU3DMOTID motId[10];
    s16 motNo;
    float radius;
    BOOL collideF;
    BOOL hipdropF;
    BOOL jumpF;
    BOOL inputEnableF;
    float velY;
    s16 padNo;
    int comMode;
    int comDelay;
    M526_FUSE *fuse;
    s16 diff;
} M526_PLAYER;

typedef struct M526KingBomb_s {
    int mode;
    HUPROCESS *process;
    int side;
    HuVecF pos;
    HU3DMODELID modelId[6];
    HU3DMOTID motId[2];
} M526_KINGBOMB;

typedef struct M526ExplosionBomb_s {
    int mode;
    HUPROCESS *process;
    HuVecF pos;
    HU3DMODELID modelId[2];
    HU3DMOTID motId[3];
} M526_EXPLOSIONBOMB;

typedef struct M526Bomb_s {
    int mode;
    HUPROCESS *process;
    int side;
    int launchDelay;
    int no;
    HuVecF pos;
    HuVecF rot;
    HuVecF vel;
    HU3DMODELID modelId;
    int stageSide;
} M526_BOMB;

typedef struct M526Smoke_s {
    int mode;
    HUPROCESS *process;
    int side;
    HuVecF pos;
    HU3DMODELID modelId;
} M526_SMOKE;

M526_GAMEFLOW *M526GameFlowCreate(void);
void M526GameFlowWaitSet(M526_GAMEFLOW *gameflow);
void M526GameFlowInitSet(M526_GAMEFLOW *gameflow);
void M526GameFlowStartSet(M526_GAMEFLOW *gameflow);
void M526GameFlowFinishSet(M526_GAMEFLOW *gameflow);
void M526GameFlowPlaySet(M526_GAMEFLOW *gameflow);
void M526GameFlowSplitSet(M526_GAMEFLOW *gameflow, float pos);
void M526GameFlowBombCull(M526_GAMEFLOW *gameflow);
void M526GameFlowBombFollow(M526_GAMEFLOW *gameflow);
void M526GameFlowFuseStart(M526_GAMEFLOW *gameflow);
void M526GameFlowPush(M526_GAMEFLOW *gameflow);

M526_STAGE *M526StageCreate(int side);
void M526StagePosSet(M526_STAGE *stage, HuVecF *pos);
HuVecF M526StagePosGet(M526_STAGE *stage);
void M526StageShadowOff(M526_STAGE *stage);
void M526StageShadowOn(M526_STAGE *stage);
void M526StageWaitSet(M526_STAGE *stage);
void M526StageSmokeStartSet(M526_STAGE *stage);
void M526StageCameraMoveSet(M526_STAGE *stage);
void M526StageCloudUpdate(M526_STAGE *stage);

M526_CAMERA *M526CameraCreate(int side);
void M526CameraOutView(M526_CAMERA *camera);
void M526CameraPosSet(M526_CAMERA *camera);
HuVecF M526CameraPosGet(M526_CAMERA *camera);
void M526CameraCenterSet(M526_CAMERA *camera, HuVecF *pos);
HuVecF M526CameraCenterGet(M526_CAMERA *camera);
void M526CameraWaitSet(M526_CAMERA *camera);
void M526CameraMoveSet(M526_CAMERA *camera, const HuVecF *center, const HuVecF *rot, const float *zoom, int speed);
void M526CameraPerspectiveSet(M526_CAMERA *camera, float fov, float near, float far, float aspect);
void M526CameraViewportSet(M526_CAMERA *camera, float vpX, float vpY, float vpW, float vpH, float vpNearZ, float vpFarZ);
void M526CameraScissorSet(M526_CAMERA *camera, float scissorX, float scissorY, float scissorW, float scissorH);

M526_PLAYER *M526PlayerCreate(int side, int sidePlayerNo, int playerNo);
void M526PlayerDispOn(M526_PLAYER *player);
void M526PlayerDispOff(M526_PLAYER *player);
void M526PlayerMotionSet(M526_PLAYER *player, int motNo, int start, int end);
void M526PlayerPosSet(M526_PLAYER *player, HuVecF *pos);
HuVecF M526PlayerPosGet(M526_PLAYER *player);
void M526PlayerRotSet(M526_PLAYER *player, HuVecF *rot);
HuVecF M526PlayerRotGet(M526_PLAYER *player);
void M526PlayerMove(M526_PLAYER *player);
float M526AngleLerp(float start, float end, float t);
void M526PlayerCollideCheck(M526_PLAYER *player);
void M526PlayerWallHit(M526_PLAYER *player);
void M526PlayerInput(M526_PLAYER *player);
BOOL M526PlayerFuseCheck(M526_PLAYER *player, M526_FUSE *fuse);
void M526PlayerVibrate(M526_PLAYER *player, int mode, int time);
void M526PlayerWaitSet(M526_PLAYER *player);
void M526PlayerModeMoveSet(M526_PLAYER *player);
void M526PlayerHipDropSet(M526_PLAYER *player);
void M526PlayerHipDropFuse(M526_PLAYER *player);
void M526PlayerCrossWalkSet(M526_PLAYER *player, HuVecF *pos);
void M526PlayerWinnerWalkSet(M526_PLAYER *player, HuVecF *pos);
void M526PlayerWinnerSet(M526_PLAYER *player);
void M526PlayerLaunchSet(M526_PLAYER *player);

M526_FUSE *M526FuseCreate(int side, int lane);
void M526FusePosSet(M526_FUSE *fuse, HuVecF *pos);
void M526FuseDispOn(M526_FUSE *fuse);
void M526FuseDispOff(M526_FUSE *fuse);
BOOL M526FuseCheckActive(M526_FUSE *fuse);
BOOL M526FuseCheckDone(M526_FUSE *fuse);
HuVecF M526FusePosGet(M526_FUSE *fuse);
HuVecF M526FuseHookPosGet(M526_FUSE *fuse);
void M526FuseSpeedSet(M526_FUSE *fuse, float speed);
void M526FuseWaitSet(M526_FUSE *fuse);
void M526FuseStopSet(M526_FUSE *fuse);
void M526FuseIdleSet(M526_FUSE *fuse);
void M526FuseExplodeSet(M526_FUSE *fuse);
int M526FuseDelayGet(M526_FUSE *fuse);

M526_KINGBOMB *M526KingBombCreate(int side);
void M526KingBombPosSet(M526_KINGBOMB *kingbomb, HuVecF *pos);
HuVecF M526KingBombPosGet(M526_KINGBOMB *kingbomb);
void M526KingBombMotionSet(M526_KINGBOMB *kingbomb, int motNo, int start, int end);
void M526KingBombIdleSet(M526_KINGBOMB *kingbomb);
void M526KingBombExplodeSet(M526_KINGBOMB *kingbomb);
void M526KingBombWinSet(M526_KINGBOMB *kingbomb);

M526_EXPLOSIONBOMB *M526ExplosionBombCreate(void);
void M526ExplosionBombPosSet(M526_EXPLOSIONBOMB *explosionBomb, HuVecF *pos);
HuVecF M526ExplosionBombPosGet(M526_EXPLOSIONBOMB *explosionBomb);
void M526ExplosionBombMotionSet(M526_EXPLOSIONBOMB *explosionBomb, int motNo, int start, int end, u32 attr);
void M526ExplosionBombWalkSet(M526_EXPLOSIONBOMB *explosionBomb);
void M526ExplosionBombExplodeSet(M526_EXPLOSIONBOMB *explosionBomb);

M526_BOMB *M526BombCreate(int side, int stageSide);
void M526BombPosSet(M526_BOMB *bomb, HuVecF *pos);
HuVecF M526BombPosGet(M526_BOMB *bomb);
void M526BombRotSet(M526_BOMB *bomb, HuVecF *rot);
HuVecF M526BombRotGet(M526_BOMB *bomb);
void M526BombDispOn(M526_BOMB *bomb);
void M526BombDispOff(M526_BOMB *bomb);
void M526BombDispOff(M526_BOMB *bomb);
void M526BombModeRandomSet(M526_BOMB *bomb);
void M526BombLaunchSet(M526_BOMB *bomb, int delay);
void M526BombExitSet(M526_BOMB *bomb);
void M526BombFollowSet(M526_BOMB *bomb);

M526_SMOKE *M526SmokeCreate(int side);
void M526SmokePosSet(M526_SMOKE *smoke, HuVecF *pos);
HuVecF M526SmokePosGet(M526_SMOKE *smoke);
void M526SmokeHideSet(M526_SMOKE *smoke);
void M526SmokeShowSet(M526_SMOKE *smoke);
BOOL M526SmokeDispGet(M526_SMOKE *smoke);
void M526SmokeSidePosSet(int side, HuVecF *pos);

s16 M526Audio3DPanGet(s16 side, HuVecF pos);
void M526Audio3DPlay(int id, s16 side, HuVecF pos);

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

static const M526_OBJPROC ObjProcData[] = {
    { 4096, 8192 },
    { 4096, 8192 },
    { 4096, 4096 },
    { 4096, 8192 },
    { 4096, 6144 },
    { 4096, 4096 },
    { 4096, 4096 },
    { 4096, 4096 },
    { 4096, 4096 },
};

static const int PlayerMotFileTbl[CHARNO_MAX][10] = {
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, mario),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, luigi),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, peach),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, yoshi),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, wario),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, daisy),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, waluigi),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, kinopio),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, teresa),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, minikoopa),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, minikoopa),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, minikoopa),
    
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_301, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_302, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_304, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_308, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_309, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_376, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_310, minikoopa),
};

static const int ExplosionBombMotFileTbl[3] = {
    M526_HSF_explosion_bomhei_fuse,
    M526_HSF_explosion_bomhei_walk,
    M526_HSF_explosion_bomhei_glow
};

static const s16 CameraBitTbl[2] = {
    HU3D_CAM0,
    HU3D_CAM1
};

static char *FuseHookTbl[2][3] = {
    "itemhook_a",
    "itemhook_b",
    "itemhook_c",
    "itemhook_d",
    "itemhook_e",
    "itemhook_f",
};

static M526_FUSE *FuseTbl[2][3] = {};
static M526_FUSE *FuseStartTbl[2][3] = {};

static HuVecF ShadowPosTbl[2][3] = {
    {
        { 0, 3000, 3000 },
        { 0, 1, 0 },
        { 0, 0, 1000 },
    },
    {
        { 0, 2500, 5100 },
        { 0, 1, 0 },
        { 0, 0, 3100 },
    }
};

static const M526_STAGELIGHT StageLightTbl = {
    { 0, 2000, 6000 },
    { 0, 0, 0 },
    { 255, 255, 255, 255 }
};


static M526_GAMEFLOW *GameFlowObj;
static M526_STAGE *StageObj[2];
static M526_FUSE *FuseObj[2][3];
static M526_CAMERA *CameraObj[2];
static M526_PLAYER *PlayerObj[2][2];
static M526_KINGBOMB *KingBombObj[2];
static M526_BOMB *BombObj[2][56];
static M526_EXPLOSIONBOMB *ExplosionBombObj;
static M526_SMOKE *SmokeObj[2][6];
static float FuseSpeed;
static int lbl_1_bss_18[2][3];
static BOOL GameActiveF[2];
static float GameDist[2];
static int GameStreamNo;
static int BombNum;

static void M526Main(void);

void ObjectSetup(void)
{
    OMOBJMAN *objman = omInitObjMan(1, 8192);
    int side;
    int i;
    int no;
    int grpNo[GW_PLAYER_MAX];
    omGameSysInit(objman);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        grpNo[i] = GwPlayerConf[i].grp;
    }
    no = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(grpNo[i] == 0) {
            no++;
        }
    }
    if(no == 2) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(grpNo[i]) {
                grpNo[i] = 1;
            }
        }
    } else {
        grpNo[0] = 0;
        grpNo[1] = 0;
        grpNo[2] = 1;
        grpNo[3] = 1;
    }
    HuPrcChildCreate(M526Main, 4095, 4096, 0, HuPrcCurrentGet());
    for(side=0; side<2; side++) {
        StageObj[side] = M526StageCreate(side);
    }
    for(side=0; side<2; side++) {
        for(i=0; i<3; i++) {
            FuseObj[side][i] = M526FuseCreate(side, i);
        }
    }
    for(side=0; side<2; side++) {
        KingBombObj[side] = M526KingBombCreate(side);
    }
    for(side=0; side<2; side++) {
        for(i=0; i<56; i++) {
            int stageSide;
            if(i < 20) {
                stageSide = 1;
            } else {
                stageSide = 0;
            }
            BombObj[side][i] = M526BombCreate(side, stageSide);
        }
    }
    for(side=0; side<2; side++) {
        for(i=0; i<6; i++) {
            SmokeObj[side][i] = M526SmokeCreate(side);
        }
    }
    for(side=0; side<2; side++) {
        no = 0;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(side == grpNo[i]) {
                PlayerObj[side][no++] = M526PlayerCreate(side, no, i);
            }
        }
    }
    for(side=0; side<2; side++) {
        CameraObj[side] = M526CameraCreate(side);
    }
    for(side=0; side<2; side++) {
        GameActiveF[side] = TRUE;
    }
    ExplosionBombObj = M526ExplosionBombCreate();
    GameFlowObj = M526GameFlowCreate();
    M526GameFlowInitSet(GameFlowObj);
}

static void M526Main(void)
{
    while(!omSysExitReq) {
        HuPrcVSleep();
    }
    HuAudFadeOut(1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
    WipeWait();
    omOvlReturn(1);
    HuPrcEnd();
}

static void M526GameFlowMain(void);

M526_GAMEFLOW *M526GameFlowCreate(void)
{
    M526_GAMEFLOW *gameflow = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_GAMEFLOW), HU_MEMNUM_OVL);
    int i;
    HU3DLIGHTID lightId;
    static const int sprFileTbl[1] = {
        M526_ANM_split
    };
    if(gameflow == NULL) {
        OSReport("M526GameFlowCreate Error...\n");
        return NULL;
    }
    for(i=0; i<1; i++) {
        gameflow->sprId[i] = espEntry(sprFileTbl[i], 0, 0);
    }
    espPriSet(gameflow->sprId[0], 256);
    gameflow->process = HuPrcChildCreate(M526GameFlowMain, ObjProcData[0].prio, ObjProcData[0].stackSize, 0, HuPrcCurrentGet());
    gameflow->process->property = gameflow;
    lightId = Hu3DGLightCreate(0, 0, 0, 0, 0, 0, 0, 0, 0);
    Hu3DGLightPosAimSetV(lightId, (HuVecF *)&StageLightTbl.pos, (HuVecF *)&StageLightTbl.target);
    Hu3DGLightColorSet(lightId, StageLightTbl.color.r, StageLightTbl.color.g, StageLightTbl.color.b, StageLightTbl.color.a);
    Hu3DGLightStaticSet(lightId, TRUE);
    gameflow->mode = 0;
    return gameflow;
}

typedef void (*M526_GAMEFLOW_FUNC)(M526_GAMEFLOW *gameflow);

static void M526GameFlowWait(M526_GAMEFLOW *gameflow);
static void M526GameFlowInit(M526_GAMEFLOW *gameflow);
static void M526GameFlowStart(M526_GAMEFLOW *gameflow);
static void M526GameFlowPlay(M526_GAMEFLOW *gameflow);
static void M526GameFlowFinish(M526_GAMEFLOW *gameflow);

static M526_GAMEFLOW_FUNC GameFlowModeTbl[] = {
    M526GameFlowWait,
    M526GameFlowInit,
    M526GameFlowStart,
    M526GameFlowPlay,
    M526GameFlowFinish
};

static void M526GameFlowMain(void)
{
    M526_GAMEFLOW *gameflow = HuPrcCurrentGet()->property;
    while(gameflow->mode != 5) {
        GameFlowModeTbl[gameflow->mode](gameflow);
    }
    omOvlReturn(1);
    HuPrcEnd();
}


void M526GameFlowWaitSet(M526_GAMEFLOW *gameflow)
{
    gameflow->mode = 0;
}

#define SLEEP_MODECHG(type, tick) \
do { \
    int mode; \
    int i; \
    int num; \
    num = tick; \
    for(i=0; i<num; i++) { \
        mode = ((type *)HuPrcCurrentGet()->property)->mode; \
        HuPrcVSleep(); \
        if(mode != ((type *)HuPrcCurrentGet()->property)->mode) { \
            return; \
        } \
    } \
} while (0)
    
static void M526GameFlowWait(M526_GAMEFLOW *gameflow)
{
    while(1) {
        SLEEP_MODECHG(M526_GAMEFLOW, 1);
    }
}

void M526GameFlowInitSet(M526_GAMEFLOW *gameflow)
{
    gameflow->mode = 1;
}

static void M526GameFlowInit(M526_GAMEFLOW *gameflow)
{
    static const M526_CAMERAVIEW playerCameraTbl[2] = {
        {  
            { -0.0f, -60, 0 },
            { -7.5, 0, 0 },
            3520,
        },
        {  
            { 0.0f, -60, 0 },
            { -7.5, 0, 0 },
            3520,
        },
    };
    int i;
    int no;
    int side;
    HuVecF pos;
    
    Hu3DShadowMultiCreate(40, 30, 10000, HU3D_CAM0|HU3D_CAM1);
    Hu3DShadowMultiTPLvlSet(0.6f, HU3D_CAM_ALL);
    Hu3DShadowMultiPosSet(&ShadowPosTbl[1][0], &ShadowPosTbl[1][1], &ShadowPosTbl[1][2], HU3D_CAM_ALL);
   
    for(side=0; side<2; side++) {
        pos.x = 0;
        pos.y = 0;
        pos.z = 0;
        M526StagePosSet(StageObj[side], &pos);
        M526FusePosSet(FuseObj[side][0], &pos);
        M526FusePosSet(FuseObj[side][1], &pos);
        M526FusePosSet(FuseObj[side][2], &pos);
        M526KingBombPosSet(KingBombObj[side], &pos);
        no=0;
        for(i=0; i<10; i++, no++) {
            pos.x = (frandmod(20)-10)+(-320.0f-((i%2)*100));
            pos.z = (frandmod(20)-10)+((200.0f*i)+100.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        for(i=0; i<10; i++, no++) {
            pos.x = (frandmod(20)-10)+(320.0f+((i%2)*100));
            pos.z = (frandmod(20)-10)+((200.0f*i)+100.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        for(i=0; i<10; i++, no++) {
            pos.x = (frandmod(20)-10)+(-520.0f-((i%2)*100));
            pos.z = (frandmod(20)-10)+((200.0f*i)+150.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        for(i=0; i<10; i++, no++) {
            pos.x = (frandmod(20)-10)+(520.0f+((i%2)*100));
            pos.z = (frandmod(20)-10)+((200.0f*i)+150.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        for(i=0; i<5; i++, no++) {
            pos.x = (frandmod(20)-10)+(-720.0f-((i%2)*100));
            pos.z = (frandmod(20)-10)+((200.0f*i)+100.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        for(i=0; i<5; i++, no++) {
            pos.x = (frandmod(20)-10)+(720.0f+((i%2)*100));
            pos.z = (frandmod(20)-10)+((200.0f*i)+100.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        for(i=0; i<3; i++, no++) {
            pos.x = (frandmod(20)-10)+(-920.0f-((i%2)*100));
            pos.z = (frandmod(20)-10)+((200.0f*i)+150.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        for(i=0; i<3; i++, no++) {
            pos.x = (frandmod(10)-10)+(920.0f+((i%2)*100));
            pos.z = (frandmod(10)-10)+((200.0f*i)+150.0f);
            M526BombPosSet(BombObj[side][no], &pos);
        }
        pos.x = 0;
        pos.y = 0;
        pos.z = 2100;
        pos.x -= 100;
        M526PlayerPosSet(PlayerObj[side][0], &pos);
        pos.x = 0;
        pos.y = 0;
        pos.z = 2100;
        pos.x += 100;
        M526PlayerPosSet(PlayerObj[side][1], &pos);
        M526PlayerWaitSet(PlayerObj[side][0]);
        M526PlayerWaitSet(PlayerObj[side][1]);
        M526CameraMoveSet(CameraObj[side], &playerCameraTbl[side].center, &playerCameraTbl[side].rot, &playerCameraTbl[side].zoom, 1);
    }
    M526GameFlowSplitSet(gameflow, 0);
    gameflow->mode = 2;
}

void M526GameFlowStartSet(M526_GAMEFLOW *gameflow)
{
    gameflow->mode = 2;
}

static void M526GameFlowStart(M526_GAMEFLOW *gameflow)
{
    static const M526_CAMERAVIEW playerCameraTbl[2] = {
        {  
            { 0, 0, 3000 },
            { -24, 0, 0 },
            650,
        },
        {  
            { 0.0f, -60, 0 },
            { -7.5, 0, 0 },
            3520,
        },
    };
    int i;
    int side;
    GAMEMESID GameMesId;
    
    HuVecF pos;
    M526GameFlowSplitSet(gameflow, 0);
    pos.x = 0;
    pos.y = 0;
    pos.z = 3100;
    M526ExplosionBombPosSet(ExplosionBombObj, &pos);
    M526CameraMoveSet(CameraObj[0], &playerCameraTbl[0].center, &playerCameraTbl[0].rot, &playerCameraTbl[0].zoom, 1);
    for(i=0; i<2; i++) {
        M526StageShadowOff(StageObj[i]);
    }
    GameStreamNo = HuAudSStreamPlay(MSM_STREAM_MGMUS_22);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 60);
    while(WipeCheckEnd()) {
        SLEEP_MODECHG(M526_GAMEFLOW, 1);
    }
    HuAudFXPlay(MSM_SE_GUIDE_26);
    M526ExplosionBombExplodeSet(ExplosionBombObj);
    SLEEP_MODECHG(M526_GAMEFLOW, 80);
    for(side=0; side<2; side++) {
        for(i=0; i<3; i++) {
            M526FuseSpeedSet(FuseObj[side][i], 0.08f);
            M526FuseIdleSet(FuseObj[side][i]);
        }
    }
    SLEEP_MODECHG(M526_GAMEFLOW, 30);
    Hu3DShadowMultiCreate(40, 30, 10000, HU3D_CAM0|HU3D_CAM1);
    Hu3DShadowMultiTPLvlSet(0.6f, HU3D_CAM_ALL);
    Hu3DShadowMultiPosSet(&ShadowPosTbl[0][0], &ShadowPosTbl[0][1], &ShadowPosTbl[0][2], HU3D_CAM_ALL);
    for(i=0; i<2; i++) {
        M526CameraMoveSet(CameraObj[i], &playerCameraTbl[1].center, &playerCameraTbl[1].rot, &playerCameraTbl[1].zoom, 120);
    }
    SLEEP_MODECHG(M526_GAMEFLOW, 120);
    for(side=0; side<60; side++) {
        float t = 0.016666668f*(side+1);
        float split = HuSin(t*90);
        split = split*split;
        M526GameFlowSplitSet(gameflow, 0.5f*split);
        SLEEP_MODECHG(M526_GAMEFLOW, 1);
    }
    M526GameFlowSplitSet(gameflow, 0.5f);
    SLEEP_MODECHG(M526_GAMEFLOW, 60);
    M526GameFlowBombCull(gameflow);
    M526GameFlowBombFollow(gameflow);
    GameMesId = GameMesMgStartCreate();
    while(GameMesStatGet(GameMesId)) {
        SLEEP_MODECHG(M526_GAMEFLOW, 1);
    }
    gameflow->mode = 3;
}

void M526GameFlowFinishSet(M526_GAMEFLOW *gameflow)
{
    gameflow->mode = 4;
}

static void M526GameFlowFinish(M526_GAMEFLOW *gameflow)
{
    HuVecF posP1;
    HuVecF posP2;
    HuVecF center;
    HuVecF rot;
    float zoom;
    
    int side;
    int time;
    int i;
    GAMEMESID GameMesId;
    
    float splitTime;
    float split;
    
    HuAudSStreamFadeOut(GameStreamNo, 100);
    GameMesId = GameMesMgFinishCreate();
    while(GameMesStatGet(GameMesId)) {
        SLEEP_MODECHG(M526_GAMEFLOW, 1);
    }
    if(!GameActiveF[0] && !GameActiveF[1]) {
        SLEEP_MODECHG(M526_GAMEFLOW, 90);
        HuAudJinglePlay(MSM_STREAM_JNGL_MG_DRAW);
        GameMesId = GameMesMgDrawCreate();
    } else {
        side = (GameActiveF[0]) ? 0 : 1;
        GWMgCoinSet(PlayerObj[side][0]->playerNo, 10);
        GWMgCoinSet(PlayerObj[side][1]->playerNo, 10);
        for(i=0; i<56; i++) {
            M526BombExitSet(BombObj[side][i]);
        }
        if(side == 0) {
            for(time=0; time<60; time++) {
                splitTime = 0.016666668f*(time+1);
                split = HuSin(splitTime*90);
                split = split*split;
                M526GameFlowSplitSet(gameflow, 0.5f-(split*0.5f));
                SLEEP_MODECHG(M526_GAMEFLOW, 1);
            }
            M526GameFlowSplitSet(gameflow, 0);
        } else {
            for(time=0; time<60; time++) {
                splitTime = 0.016666668f*(time+1);
                split = HuSin(splitTime*90);
                split = split*split;
                M526GameFlowSplitSet(gameflow, 0.5f+(split*0.5f));
                SLEEP_MODECHG(M526_GAMEFLOW, 1);
            }
            M526GameFlowSplitSet(gameflow, 1);
        }
        posP1 = M526PlayerPosGet(PlayerObj[side][0]);
        posP2 = M526PlayerPosGet(PlayerObj[side][1]);
        center.z = 0.5f*(posP1.z+posP2.z);
        center.y = 0;
        center.x = -100;
        M526PlayerWinnerWalkSet(PlayerObj[side][0], &center);
        SLEEP_MODECHG(M526_GAMEFLOW, 10);
        center.x = 100;
        center.z -= 5;
        M526PlayerWinnerWalkSet(PlayerObj[side][1], &center);
        SLEEP_MODECHG(M526_GAMEFLOW, 30);
        center.x = 0;
        center.y = 60;
        center.z -= 100;
        rot.x = -14;
        rot.y = 0;
        rot.z = 0;
        zoom = 1200;
        M526CameraMoveSet(CameraObj[side], &center, &rot, &zoom, 70);
        SLEEP_MODECHG(M526_GAMEFLOW, 70);
        HuAudJinglePlay(MSM_STREAM_JNGL_MG_WIN);
        GameMesId = GameMesMgWinnerCreate(PlayerObj[side][0]->charNo, PlayerObj[side][1]->charNo, GAMEMES_MG_WINNER_NONE, GAMEMES_MG_WINNER_NONE);
        M526PlayerWinnerSet(PlayerObj[side][0]);
        M526PlayerWinnerSet(PlayerObj[side][1]);
    }
    SLEEP_MODECHG(M526_GAMEFLOW, 210);
    HuAudFadeOut(1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
    while(WipeCheckEnd()) {
        SLEEP_MODECHG(M526_GAMEFLOW, 1);
    }
    gameflow->mode = 5;
}

void M526GameFlowPlaySet(M526_GAMEFLOW *gameflow)
{
    gameflow->mode = 3;
}

static void M526GameFlowPlay(M526_GAMEFLOW *gameflow)
{
    int side;
    int i;
    BOOL endF;
    FuseSpeed = 0.057f;
    for(side=0; side<2; side++) {
        for(i=0; i<3; i++) {
            M526FuseSpeedSet(FuseObj[side][i], 0.057f);
        }
    }
    for(side=0; side<2; side++) {
        for(i=0; i<3; i++) {
            lbl_1_bss_18[side][i] = 0;
        }
    }
    for(side=0; side<2; side++) {
        for(i=0; i<2; i++) {
            PlayerObj[side][i]->inputEnableF = TRUE;
            M526PlayerModeMoveSet(PlayerObj[side][i]);
        }
    }
    for(side=0; side<2; side++) {
        M526StageCameraMoveSet(StageObj[side]);
    }
    endF = FALSE;
    while(!endF) {
        M526GameFlowPush(gameflow);
        M526GameFlowFuseStart(gameflow);
        for(side=0; side<2; side++) {
            for(i=0; i<3; i++) {
                M526_FUSE *fuse = FuseObj[side][i];
                if(M526FuseCheckDone(fuse)) {
                    GameActiveF[side] = FALSE;
                    endF = TRUE;
                }
            }
        }
        SLEEP_MODECHG(M526_GAMEFLOW, 1);
    }
    for(side=0; side<2; side++) {
        for(i=0; i<2; i++) {
            PlayerObj[side][i]->inputEnableF = FALSE;
        }
    }
    for(side=0; side<2; side++) {
        for(i=0; i<3; i++) {
            M526FuseExplodeSet(FuseObj[side][i]);
        }
    }
    if(GameActiveF[0] && GameActiveF[1]) {
        M526KingBombExplodeSet(KingBombObj[0]);
        M526KingBombExplodeSet(KingBombObj[1]);
        HuAudFXPlay(MSM_SE_M526_08);
    } else {
        for(side=0; side<2; side++) {
            if(!GameActiveF[side]) {
                M526KingBombExplodeSet(KingBombObj[side]);
                if(side == 0) {
                    HuAudFXPlay(MSM_SE_M526_02);
                } else {
                    HuAudFXPlay(MSM_SE_M526_03);
                }
            } else {
                M526KingBombWinSet(KingBombObj[side]);
            }
        }
    }
    for(side=0; side<2; side++) {
        M526StageWaitSet(StageObj[side]);
    }
    for(side=0; side<2; side++) {
        M526CameraWaitSet(CameraObj[side]);
    }
    gameflow->mode = 4;
}

void M526GameFlowSplitSet(M526_GAMEFLOW *gameflow, float pos)
{
    espPosSet(gameflow->sprId[0], 576*(1-pos), 240);
    M526CameraPerspectiveSet(CameraObj[0], 30, 10, 8000, (576*(1-pos))/480);
    M526CameraPerspectiveSet(CameraObj[1], 30, 10, 8000, (576*pos)/480);
    M526CameraViewportSet(CameraObj[0], 0, 0, 640*(1-pos), 480, 0, 1);
    M526CameraViewportSet(CameraObj[1], 640*(1-pos), 0, 640*pos, 480, 0, 1);
    M526CameraScissorSet(CameraObj[0], 0, 0, 640*(1-pos), 480);
    M526CameraScissorSet(CameraObj[1], 640*(1-pos), 0, 640*pos, 480);
}

void M526GameFlowBombCull(M526_GAMEFLOW *gameflow)
{
    int i, side;
    for(side=0; side<2; side++) {
        for(i=40; i<56; i++) {
            M526BombDispOff(BombObj[side][i]);
        }
    }
}

void M526GameFlowBombFollow(M526_GAMEFLOW *gameflow)
{
    HuVecF pos;
    int i, side;
    for(side=0; side<2; side++) {
        for(i=0; i<5; i++) {
            int no = 55-i;
            pos.x = (i*70)-140;
            pos.y = 0;
            pos.z = 2800;
            M526BombPosSet(BombObj[side][no], &pos);
            M526BombFollowSet(BombObj[side][no]);
            M526BombDispOn(BombObj[side][no]);
        }
    }
}

void M526GameFlowFuseStart(M526_GAMEFLOW *gameflow)
{
    int i, side;
    BOOL doneF;
    M526_FUSE *fuseTemp;
    HuVecF pos1, pos2;
    for(side=0; side<2; side++) {
        for(i=0; i<3; i++) {
            FuseTbl[side][i] = FuseObj[side][i];
        }
        while(1) {
            doneF = FALSE;
            for(i=0; i<2; i++) {
                if(M526FuseDelayGet(FuseTbl[side][i]) < M526FuseDelayGet(FuseTbl[side][i+1])) {
                    fuseTemp = FuseTbl[side][i];
                    FuseTbl[side][i] = FuseTbl[side][i+1];
                    FuseTbl[side][i+1] = fuseTemp;
                    doneF = TRUE;
                }
            }
            if(!doneF) {
                break;
            }
        }
        for(i=0; i<3; i++) {
            FuseStartTbl[side][i] = FuseObj[side][i];
        }
        while(1) {
            doneF = FALSE;
            for(i=0; i<2; i++) {
                pos1 = M526FuseHookPosGet(FuseStartTbl[side][i]);
                pos2 = M526FuseHookPosGet(FuseStartTbl[side][i+1]);
                if(pos1.z > pos2.z) {
                    fuseTemp = FuseStartTbl[side][i];
                    FuseStartTbl[side][i] = FuseStartTbl[side][i+1];
                    FuseStartTbl[side][i+1] = fuseTemp;
                    doneF = TRUE;
                }
            }
            if(!doneF) {
                break;
            }
        }
    }
}

void M526GameFlowPush(M526_GAMEFLOW *gameflow)
{
    float dist = 0;
    int i, side;
    HuVecF pos;
    for(side=0; side<2; side++) {
        for(i=0; i<3; i++) {
            pos = M526FuseHookPosGet(FuseObj[side][i]);
            if(pos.z > dist) {
                dist = pos.z;
            }
        }
        GameDist[side] = dist;
    }
}

static void M526StageMain(void);

M526_STAGE *M526StageCreate(int side)
{
    M526_STAGE *stage = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_STAGE), HU_MEMNUM_OVL);
    int i;
    static const int mdlFileTbl[8] = {
        M526_HSF_stage,
        M526_HSF_mountain,
        M526_HSF_cloud,
        M526_HSF_cloud,
        M526_HSF_tree,
        M526_HSF_glow,
        M526_HSF_glow,
        M526_HSF_glow
    };
    if(stage == NULL) {
        OSReport("M526StageCreate Error...\n");
        return NULL;
    }
    stage->pos.x = 0;
    stage->pos.y = 0;
    stage->pos.z = 0;
    stage->cameraBit = CameraBitTbl[side];
    stage->side = side;
    for(i=0; i<8; i++) {
        stage->modelId[i] = Hu3DModelCreateData(mdlFileTbl[i]);
        Hu3DModelCameraSet(stage->modelId[i], stage->cameraBit);
        Hu3DModelAttrSet(stage->modelId[i], HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<3; i++) {
        Hu3DMotionTimeSet(stage->modelId[i+5], i*5);
    }
    Hu3DModelScaleSet(stage->modelId[5], 1.8f, 1.8f, 1);
    Hu3DModelDispOff(stage->modelId[4]);
    Hu3DModelDispOff(stage->modelId[5]);
    Hu3DModelDispOff(stage->modelId[6]);
    Hu3DModelDispOff(stage->modelId[7]);
    Hu3DModelScaleSet(stage->modelId[2], 0.8f, 0.8f, 0.8f);
    Hu3DModelShadowMapSet(stage->modelId[0]);
    stage->process = HuPrcChildCreate(M526StageMain, ObjProcData[1].prio, ObjProcData[1].stackSize, 0, HuPrcCurrentGet());
    stage->process->property = stage;
    stage->mode = 0;
    return stage;
}

typedef void (*M526_STAGE_FUNC)(M526_STAGE *stage);

static void M526StageWait(M526_STAGE *stage);
static void M526StageCameraMove(M526_STAGE *stage);
static void M526StageSmokeStart(M526_STAGE *stage);

static M526_STAGE_FUNC StageModeTbl[] = {
    M526StageWait,
    M526StageCameraMove,
    M526StageSmokeStart,
};

static void M526StageMain(void)
{
    M526_STAGE *stage = HuPrcCurrentGet()->property;
    while(stage->mode != 3) {
        StageModeTbl[stage->mode](stage);
    }
    HuPrcEnd();
}

void M526StagePosSet(M526_STAGE *stage, HuVecF *pos)
{
    int i;
    static const HuVecF mdlPosTbl[8] = {
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 800, 680, -2000 },
        { -800, 900, -2000 },
        { 0, 0, 0 },
        { -400, 700, -1000 },
        { -600, 500, -1000 },
        { 500, 750, -1000 },
    };
    
    stage->pos = *pos;
    for(i=0; i<8; i++) {
        Hu3DModelPosSet(stage->modelId[i], stage->pos.x+mdlPosTbl[i].x, stage->pos.y+mdlPosTbl[i].y-1, stage->pos.z+mdlPosTbl[i].z);
    }
    stage->cloudPos[2] = mdlPosTbl[2];
    stage->cloudPos[3] = mdlPosTbl[3];
}

HuVecF M526StagePosGet(M526_STAGE *stage)
{
    return stage->pos;
}

void M526StageShadowOff(M526_STAGE *stage)
{
    Hu3DModelAttrSet(stage->modelId[0], HU3D_MOTATTR_PAUSE);
    Hu3DMotionTimeSet(stage->modelId[0], 0);
}

void M526StageShadowOn(M526_STAGE *stage)
{
    Hu3DModelAttrSet(stage->modelId[0], HU3D_MOTATTR_PAUSE);
    Hu3DMotionTimeSet(stage->modelId[0], 5);
}

void M526StageWaitSet(M526_STAGE *stage)
{
    stage->mode = 0;
}

static void M526StageWait(M526_STAGE *stage)
{
    while(1) {
        M526StageCloudUpdate(stage);
        SLEEP_MODECHG(M526_STAGE, 1);
    }
}

void M526StageSmokeStartSet(M526_STAGE *stage)
{
    stage->mode = 2;
}

static void M526StageSmokeStart(M526_STAGE *stage)
{
    int i;
    HuVecF pos;
    
    for(i=0; i<12; i++) {
        pos.x = frandmod(300)-150;
        pos.y = 0;
        pos.z = frandmod(1000);
        M526SmokeSidePosSet(stage->side, &pos);
        SLEEP_MODECHG(M526_STAGE, frandmod(30)+40);
    }
    stage->mode = 0;
}

void M526StageCameraMoveSet(M526_STAGE *stage)
{
    stage->mode = 1;
}

static void M526StageCameraMove(M526_STAGE *stage)
{
    M526_CAMERA *camera = CameraObj[stage->side];
    HuVecF ofs, pos;
    float dist;
    int time;
    ofs.x = 0;
    ofs.y = -60;
    ofs.z = GameDist[stage->side];
    while(1) {
        pos.z = GameDist[stage->side]-ofs.z;
        pos.x = ofs.x;
        pos.y = ofs.y;
        dist = ofs.z-pos.z;
        time = dist/10.0f;
        if(time < 1) {
            time = 1;
        }
        M526CameraMoveSet(camera, &pos, NULL, NULL, time);
        M526StageCloudUpdate(stage);
        SLEEP_MODECHG(M526_STAGE, 1);
    }
}

void M526StageCloudUpdate(M526_STAGE *stage)
{
    int i;
    for(i=0; i<2; i++) {
        stage->cloudPos[i+2].x += 0.5f;
        Hu3DModelPosSet(stage->modelId[i+2], stage->cloudPos[i+2].x, stage->cloudPos[i+2].y, stage->cloudPos[i+2].z);
    }
}

static void M526CameraMain(void);

M526_CAMERA *M526CameraCreate(int side)
{
    M526_CAMERAVIEW *view;
    M526_CAMERA *camera;
    int i;
    camera = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_CAMERA), HU_MEMNUM_OVL);
    if(camera == NULL) {
        OSReport("M526CameraCreate Error...\n");
        return NULL;
    }
    for(i=0; i<3; i++) {
        view = &camera->view[i];
        view->center.x = 0;
        view->center.y = 0;
        view->center.z = 0;
        view->rot.x = 0;
        view->rot.y = 30;
        view->rot.z = 0;
        view->zoom = 500;
    }
    camera->cameraBit = CameraBitTbl[side];
    camera->side = side;
    camera->process = HuPrcChildCreate(M526CameraMain, ObjProcData[2].prio, ObjProcData[2].stackSize, 0, HuPrcCurrentGet());
    camera->process->property = camera;
    camera->mode = 0;
    return camera;
}

typedef void (*M526_CAMERA_FUNC)(M526_CAMERA *player);

static void M526CameraWait(M526_CAMERA *camera);
static void M526CameraMove(M526_CAMERA *camera);

static M526_CAMERA_FUNC CameraModeTbl[] = {
    M526CameraWait,
    M526CameraMove,
};

static void M526CameraMain(void)
{
    M526_CAMERA *camera = HuPrcCurrentGet()->property;
    while(camera->mode != 2) {
        CameraModeTbl[camera->mode](camera);
    }
    HuPrcEnd();
}

void M526CameraOutView(M526_CAMERA *camera)
{
    M526_CAMERAVIEW *view;
    int i;
    for(i=0; i<3; i++) {
        view = &camera->view[i];
        view->eye.x = view->center.x+(view->zoom*(HuSin(view->rot.y)*HuCos(view->rot.x)));
        view->eye.y = view->center.y+(view->zoom*(-HuSin(view->rot.x)));
        view->eye.z = view->center.z+(view->zoom*(HuCos(view->rot.y)*HuCos(view->rot.x)));
        view->up.x = HuSin(view->rot.y)*HuSin(view->rot.x);
        view->up.y = HuCos(view->rot.x);
        view->up.z = HuCos(view->rot.y)*HuSin(view->rot.x);
    }
}

void M526CameraPosSet(M526_CAMERA *camera)
{
    M526_CAMERAVIEW *view = &camera->view[0];
    Hu3DCameraPosSet(camera->cameraBit,
        view->eye.x, view->eye.y, view->eye.z,
        view->up.x, view->up.y, view->up.z,
        view->center.x, view->center.y, view->center.z);
}

HuVecF M526CameraPosGet(M526_CAMERA *camera)
{
    
}

HuVecF M526CameraCenterGet(M526_CAMERA *camera)
{
    return camera->view[0].center;
}

void M526CameraCenterSet(M526_CAMERA *camera, HuVecF *pos)
{
    
}

void M526CameraWaitSet(M526_CAMERA *camera)
{
    camera->mode = 0;
}

static void M526CameraWait(M526_CAMERA *camera)
{
    while(1) {
        M526CameraPosSet(camera);
        SLEEP_MODECHG(M526_CAMERA, 1);
    }
}

void M526CameraMoveSet(M526_CAMERA *camera, const HuVecF *center, const HuVecF *rot, const float *zoom, int speed)
{
    M526_CAMERAVIEW *view = &camera->view[0];
    M526_CAMERAVIEW *view2 = &camera->view[1];
    M526_CAMERAVIEW *view3 = &camera->view[2];
    *view2 = *view;
    *view3 = *view;
    if(center != NULL) {
        view3->center = *center;
    }
    if(rot != NULL) {
        view3->rot = *rot;
    }
    if(zoom != NULL) {
        view3->zoom = *zoom;
    }
    M526CameraOutView(camera);
    camera->moveSpeed = speed;
    camera->mode = 1;
}

static void M526CameraMove(M526_CAMERA *camera)
{
    M526_CAMERAVIEW *view = &camera->view[0];
    M526_CAMERAVIEW *view2 = &camera->view[1];
    M526_CAMERAVIEW *view3 = &camera->view[2];
    int i;
    for(i=0; i<camera->moveSpeed; i++) {
        float t = (1.0f/camera->moveSpeed)*(i+1);
        float s = HuSin(t*90);
        s = s*s;
        view->center.x = view2->center.x+(s*(view3->center.x-view2->center.x));
        view->center.y = view2->center.y+(s*(view3->center.y-view2->center.y));
        view->center.z = view2->center.z+(s*(view3->center.z-view2->center.z));
        view->eye.x = view2->eye.x+(s*(view3->eye.x-view2->eye.x));
        view->eye.y = view2->eye.y+(s*(view3->eye.y-view2->eye.y));
        view->eye.z = view2->eye.z+(s*(view3->eye.z-view2->eye.z));
        view->up.x = view2->up.x+(s*(view3->up.x-view2->up.x));
        view->up.y = view2->up.y+(s*(view3->up.y-view2->up.y));
        view->up.z = view2->up.z+(s*(view3->up.z-view2->up.z));
        M526CameraPosSet(camera);
        SLEEP_MODECHG(M526_CAMERA, 1);
    }
    *view = *view3;
    M526CameraPosSet(camera);
    camera->mode = 0;
}

void M526CameraViewportSet(M526_CAMERA *camera, float vpX, float vpY, float vpW, float vpH, float vpNearZ, float vpFarZ)
{
    Hu3DCameraViewportSet(camera->cameraBit, vpX, vpY, vpW, vpH, vpNearZ, vpFarZ);
}

void M526CameraPerspectiveSet(M526_CAMERA *camera, float fov, float near, float far, float aspect)
{
    Hu3DCameraPerspectiveSet(camera->cameraBit, fov, near, far, aspect);
}

void M526CameraScissorSet(M526_CAMERA *camera, float scissorX, float scissorY, float scissorW, float scissorH)
{
    Hu3DCameraScissorSet(camera->cameraBit, scissorX, scissorY, scissorW, scissorH);
}

static void M526PlayerMain(void);


M526_PLAYER *M526PlayerCreate(int side, int sidePlayerNo, int playerNo)
{
    M526_PLAYER *player = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_PLAYER), HU_MEMNUM_OVL);
    int i;
    if(player == NULL) {
        OSReport("M526PlayerCreate Error...\n");
        return NULL;
    }
    player->side = side;
    player->sidePlayerNo = sidePlayerNo;
    player->playerNo = playerNo;
    player->charNo = GwPlayerConf[playerNo].charNo;
    player->padNo = GwPlayerConf[playerNo].padNo;
    player->comF = (GwPlayerConf[playerNo].type != GW_TYPE_MAN) ? TRUE : FALSE;
    player->diff = GwPlayerConf[playerNo].dif;
    player->comMode = 0;
    player->radius = 45;
    player->fuse = NULL;
    player->comDelay = 0;
    player->inputEnableF = FALSE;
    player->pos.x = 0;
    player->pos.y = 0;
    player->pos.z = 0;
    player->rot.x = 0;
    player->rot.y = 0;
    player->rot.z = 0;
    player->modelId = CharModelCreate(player->charNo, CHAR_MODEL3);
    Hu3DModelCameraSet(player->modelId, CameraBitTbl[side]);
    for(i=0; i<10; i++) {
        player->motId[i] = (int)CharMotionCreate(player->charNo, PlayerMotFileTbl[player->charNo][i]);
    }
    CharMotionDataClose(player->charNo);
    CharModelFxFlagSet(player->charNo, TRUE);
    Hu3DModelShadowSet(player->modelId);
    M526PlayerMotionSet(player, 0, 0, 0);
    M526PlayerPosSet(player, &player->pos);
    M526PlayerPosSet(player, &player->pos);
    M526PlayerRotSet(player, &player->rot);
    player->process = HuPrcChildCreate(M526PlayerMain, ObjProcData[3].prio, ObjProcData[3].stackSize, 0, HuPrcCurrentGet());
    player->process->property = player;
    player->mode = 0;
    return player;
}

typedef void (*M526_PLAYER_FUNC)(M526_PLAYER *player);

static void M526PlayerWait(M526_PLAYER *player);
static void M526PlayerModeMove(M526_PLAYER *player);
static void M526PlayerHipDrop(M526_PLAYER *player);
static void M526PlayerCrossWalk(M526_PLAYER *player);
static void M526PlayerWinnerWalk(M526_PLAYER *player);
static void M526PlayerWinner(M526_PLAYER *player);
static void M526PlayerLaunch(M526_PLAYER *player);

static M526_PLAYER_FUNC PlayerModeTbl[] = {
    M526PlayerWait,
    M526PlayerModeMove,
    M526PlayerHipDrop,
    M526PlayerCrossWalk,
    M526PlayerWinnerWalk,
    M526PlayerWinner,
    M526PlayerLaunch
};

static void M526PlayerMain(void)
{
    M526_PLAYER *player = HuPrcCurrentGet()->property;
    while(player->mode != 7) {
        PlayerModeTbl[player->mode](player);
    }
    HuPrcEnd();
}

void M526PlayerDispOn(M526_PLAYER *player)
{
    Hu3DModelDispOn(player->modelId);
}

void M526PlayerDispOff(M526_PLAYER *player)
{
    Hu3DModelDispOff(player->modelId);
}

void M526PlayerMotionSet(M526_PLAYER *player, int motNo, int start, int end)
{
    u32 attr;
    Hu3DModelAttrReset(player->modelId, HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
    switch(motNo) {
        case 0:
        case 1:
        case 2:
            attr = HU3D_MOTATTR_LOOP;
            break;
            
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            attr = HU3D_MOTATTR_NONE;
            break;
    }
    CharMotionShiftSet(player->charNo, player->motId[player->motNo = motNo], start, end, attr);
}

void M526PlayerPosSet(M526_PLAYER *player, HuVecF *pos)
{
    player->pos = *pos;
    Hu3DModelPosSet(player->modelId, player->pos.x, player->pos.y, player->pos.z);
}

HuVecF M526PlayerPosGet(M526_PLAYER *player)
{
    return player->pos;
}

void M526PlayerRotSet(M526_PLAYER *player, HuVecF *rot)
{
    player->rot = *rot;
    Hu3DModelRotSet(player->modelId, player->rot.x, player->rot.y, player->rot.z);
}

HuVecF M526PlayerRotGet(M526_PLAYER *player)
{
    return player->rot;
}

void M526PlayerMove(M526_PLAYER *player)
{
    HuVecF pos = player->pos;
    HuVecF rot = player->rot;
    int nextMot;
    float speed;
    if(player->jumpF) {
        nextMot = 3;
    } else {
        nextMot = 0;
    }
    speed = HuMagXZVecF(&player->vel);
    if(speed >= 0.4f) {
        if(speed >= 10.0f) {
            player->vel.x /= speed;
            player->vel.z /= speed;
            speed = 10;
            player->vel.x *= 10;
            player->vel.z *= 10;
        }
        rot.y = M526AngleLerp(rot.y, HuAtan(player->vel.x, player->vel.z), 0.4f);
        if(!player->jumpF) {
            if(speed >= 8) {
                nextMot = 2;
            } else {
                nextMot = 1;
            }
        }
    }
    pos.x += player->vel.x;
    pos.z += player->vel.z;
    pos.y += player->vel.y;
    if(player->jumpF && pos.y <= 0) {
        player->jumpF = FALSE;
        player->hipdropF = FALSE;
        pos.y = 0;
        CharModelLandDustCreate(player->charNo, &player->pos);
    }
    M526PlayerPosSet(player, &pos);
    M526PlayerRotSet(player, &rot);
    if(nextMot != player->motNo) {
        M526PlayerMotionSet(player, nextMot, 0, 8);
    }
    M526PlayerCollideCheck(player);
    if(GameFlowObj->mode != 4) {
        M526PlayerWallHit(player);
    }
    CharModelVoicePanSet(player->charNo, M526Audio3DPanGet(player->side, player->pos));
}

float M526AngleLerp(float start, float end, float t)
{
    float angleDiff;
    float result;
    angleDiff = fmod(end-start, 360);
    if(angleDiff < 0) {
        angleDiff += 360;
    }
    if(angleDiff > 180) {
        angleDiff -= 360;
    }
    result = fmod(start+(angleDiff*t), 360);
    if(result < 0) {
        result += 360;
    }
    return result;
}

void M526PlayerCollideCheck(M526_PLAYER *player)
{
    int side = player->side;
    M526_PLAYER *self = player;
    MathCircle circleSelf;
    int i;
    circleSelf.center.x = self->pos.x;
    circleSelf.center.y = self->pos.z;
    circleSelf.radius = self->radius;
    for(i=0; i<2; i++) {
        M526_PLAYER *other = PlayerObj[side][i];
        if(other != self) {
            MathCircle circleOther;
            circleOther.center.x = other->pos.x;
            circleOther.center.y = other->pos.z;
            circleOther.radius = other->radius;
            if(MathCircleCollide(&circleSelf, &circleOther)) { 
                self->pos.x = circleSelf.center.x;
                self->pos.z = circleSelf.center.y;
                M526PlayerPosSet(player, &self->pos);
                player->collideF = TRUE;
                break;
            }
        }
    }
}

static MathBBox PlayerWallTbl[6] = {
    { { -270, 30 }, { -270, 2400 } },
    { { 270, 30 }, { 270, 2400 } },
    { { -600, 130 }, { 600, 130 } },
    { { -600, 2400 }, { 600, 2400 } },
    { { -170, 2400 }, { -330, 1400 } },
    { { 170, 2400 }, { 330, 1400 } }
};

void M526PlayerWallHit(M526_PLAYER *player)
{
    int i;
    PlayerWallTbl[3].min.y = 300+GameDist[player->side];
    PlayerWallTbl[3].max.y = 300+GameDist[player->side];
    PlayerWallTbl[4].min.y = 500+GameDist[player->side];
    PlayerWallTbl[4].max.y = GameDist[player->side]-500;
    PlayerWallTbl[5].min.y = 500+GameDist[player->side];
    PlayerWallTbl[5].max.y = GameDist[player->side]-300;
    for(i=0; i<6; i++) {
        MathCircle circle;
        circle.center.x = player->pos.x;
        circle.center.y = player->pos.z;
        circle.radius = player->radius;
        if(MathCircleRectCollide(&circle, &PlayerWallTbl[i])) {
            player->pos.x = circle.center.x;
            player->pos.z = circle.center.y;
            player->collideF = TRUE;
        }
    }
    M526PlayerPosSet(player, &player->pos);
}

static void M526PlayerInputMan(M526_PLAYER *player);
static void M526PlayerInputCom(M526_PLAYER *player);

static M526_PLAYER_FUNC PlayerInputFuncTbl[2] = {
    M526PlayerInputMan,
    M526PlayerInputCom
};

void M526PlayerInput(M526_PLAYER *player)
{
    player->vel.x = 0;
    player->vel.y = 0;
    player->vel.z = 0;
    player->collideF = FALSE;
    if(player->inputEnableF) {
        PlayerInputFuncTbl[player->comF](player);
    }
}

static void M526PlayerInputMan(M526_PLAYER *player)
{
    s16 padNo = player->padNo;
    player->vel.x = HuPadStkX[padNo]/4.0f;
    player->vel.z = -(float)HuPadStkY[padNo]/4.0f;
    player->vel.y = 0;
    player->collideF = FALSE;
    if(player->jumpF && !player->hipdropF) {
        if(HuPadBtnDown[padNo] & (PAD_BUTTON_A|PAD_TRIGGER_L)) {
            player->hipdropF = TRUE;
            player->velY = 0;
        }
    } else {
        if(HuPadBtnDown[padNo] & PAD_BUTTON_A) {
            if(!player->jumpF) {
                player->jumpF = TRUE;
                player->velY = 0;
            }
        }
    }
}

static void M526PlayerComWait(M526_PLAYER *player);
static void M526PlayerComWalk(M526_PLAYER *player);
static void M526PlayerComJump(M526_PLAYER *player);
static void M526PlayerComHipDrop(M526_PLAYER *player);

static M526_PLAYER_FUNC PlayerComModeTbl[4] = {
    M526PlayerComWait,
    M526PlayerComWalk,
    M526PlayerComJump,
    M526PlayerComHipDrop
};

static void M526PlayerInputCom(M526_PLAYER *player)
{
    PlayerComModeTbl[player->comMode](player);
}

static void M526PlayerComWait(M526_PLAYER *player)
{
    if(player->fuse != NULL) {
        player->fuse->stopF = FALSE;
        player->fuse = NULL;
    }
    player->vel.x = 0;
    player->vel.y = 0;
    player->vel.z = 0;
    player->jumpF = FALSE;
    player->hipdropF = FALSE;
    if(player->comDelay <= 0 || player->comDelay-- <= 0) {
        player->comMode = 1;
    }
}

static void M526PlayerComWalk(M526_PLAYER *player)
{
    int side = player->side;
    M526_PLAYER *other = PlayerObj[side][player->sidePlayerNo^1];
    M526_FUSE *fuse;
    int i;
    MathCircle playerCol;
    MathCircle fuseCol;
    HuVecF pos;
    
    switch(player->diff) {
        case 0:
            for(i=0; i<3; i++) {
                fuse = FuseStartTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF && M526FuseCheckActive(fuse)) {
                    if(other->comF == TRUE) {
                        if(frandmod(2) == 0) {
                            goto nextMode;
                        }
                    } else {
                        if(frandmod(50) == 0) {
                            goto nextMode;
                        }
                    }
                }
            }
            goto end;
            
        case 1:
            for(i=0; i<3; i++) {
                fuse = FuseStartTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF && M526FuseCheckActive(fuse)) {
                    if(other->comF == TRUE) {
                        if(frandmod(2) == 0) {
                            goto nextMode;
                        }
                    } else {
                        if(frandmod(10) == 0) {
                            goto nextMode;
                        }
                    }
                }
            }
            goto end;
        
        case 2:
            for(i=0; i<3; i++) {
                fuse = FuseStartTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF && M526FuseCheckActive(fuse)) {
                    goto nextMode;
                }
            }
            playerCol.center.x = player->pos.x;
            playerCol.center.y = player->pos.z;
            playerCol.radius = 5;
            for(i=0; i<3; i++) {
                fuse = FuseTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF) {
                    pos = M526FusePosGet(fuse);
                    fuseCol.center.x = pos.x;
                    fuseCol.center.y = pos.z;
                    fuseCol.radius = 350;
                    if(MathCircleCircleCheck(&playerCol, &fuseCol) && M526FuseDelayGet(fuse) >= 160) {
                        goto nextMode;
                    }
                }
            }
            for(i=0; i<3; i++) {
                fuse = FuseTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF && frandmod(8) == 0) {
                    goto nextMode;
                }
            }
            goto end;
        
        case 3:
            for(i=0; i<3; i++) {
                fuse = FuseStartTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF && M526FuseCheckActive(fuse)) {
                    goto nextMode;
                }
            }
            playerCol.center.x = player->pos.x;
            playerCol.center.y = player->pos.z;
            playerCol.radius = 5;
            for(i=0; i<3; i++) {
                fuse = FuseTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF) {
                    pos = M526FusePosGet(fuse);
                    fuseCol.center.x = pos.x;
                    fuseCol.center.y = pos.z;
                    fuseCol.radius = 350;
                    if(MathCircleCircleCheck(&playerCol, &fuseCol) && M526FuseDelayGet(fuse) >= 160) {
                        goto nextMode;
                    }
                }
            }
            for(i=0; i<3; i++) {
                fuse = FuseTbl[side][i];
                if(!M526PlayerFuseCheck(other, fuse) && !fuse->stopF && frandmod(4) == 0) {
                    goto nextMode;
                }
            }
            goto end;
            
        default:
            end:
            return;
    }
    nextMode:
    fuse->stopF = TRUE;
    player->fuse = fuse;
    player->collideF = FALSE;
    player->comMode = 2;
}

BOOL M526PlayerFuseCheck(M526_PLAYER *player, M526_FUSE *fuse)
{
    MathCircle playerCol;
    MathCircle fuseCol;
    HuVecF pos;
    
    playerCol.center.x = player->pos.x;
    playerCol.center.y = player->pos.z;
    playerCol.radius = player->radius;
    pos = M526FuseHookPosGet(fuse);
    fuseCol.center.x = pos.x;
    fuseCol.center.y = pos.z;
    fuseCol.radius = 50;
    return MathCircleCircleCheck(&playerCol, &fuseCol);
}

static void M526PlayerComJump(M526_PLAYER *player)
{
    MathCircle playerCol;
    MathCircle fuseCol;
    HuVecF fusePos;
    HuVecF playerPos;
    float speed;
    float maxSpeed;
    static const int radiusTbl[4] = {
        45,
        43,
        40,
        40
    };
    static const int minYTbl[4] = {
        195,
        200,
        203,
        205
    };
    static const float maxSpeedTbl[4] = {
        8.4f,
        8.5f,
        8.6f,
        8.8f
    };
    player->vel.x = 0;
    player->vel.y = 0;
    player->vel.z = 0;
    if(player->collideF) {
        M526_PLAYER *playerOther = PlayerObj[player->side][player->sidePlayerNo^1];
        if(M526PlayerFuseCheck(playerOther, player->fuse)) {
            player->comDelay = 0;
            player->comMode = 0;
            return;
        }
    }
    fusePos = M526FuseHookPosGet(player->fuse);
    playerPos = player->pos;
    player->vel.x = fusePos.x-playerPos.x;
    player->vel.z = fusePos.z-playerPos.z;
    player->vel.y = 0;
    speed = HuMagXZVecF(&player->vel);
    if(speed >= 0.4f) {
        maxSpeed = maxSpeedTbl[player->diff];
        if(speed >= maxSpeed) {
            player->vel.x /= speed;
            player->vel.z /= speed;
            player->vel.x *= maxSpeed;
            player->vel.z *= maxSpeed;
        }
    }
    if(!player->jumpF) {
        playerCol.center.x = playerPos.x+player->vel.x;
        playerCol.center.y = playerPos.z+player->vel.z;
        playerCol.radius = player->radius;
        fuseCol.center.x = fusePos.x;
        fuseCol.center.y = fusePos.z;
        fuseCol.radius = frandmod(30)+radiusTbl[player->diff];
        if(MathCircleCircleCheck(&playerCol, &fuseCol) && M526FuseCheckActive(player->fuse)) {
            player->jumpF = TRUE;
            player->velY = 0;
        }
    } else {
        if(playerPos.y > frandmod(20)+minYTbl[player->diff]) {
            player->hipdropF = TRUE;
            player->velY = 0;
            player->comMode = 3;
        }
    }
}

static void M526PlayerComHipDrop(M526_PLAYER *player)
{
    static const int waitTbl[4] = {
        40,
        35,
        35,
        30
    };
    
    if(player->mode != 2) {
        player->vel.x = 0;
        player->vel.y = 0;
        player->vel.z = 0;
        player->comDelay = frandmod(waitTbl[player->diff]);
        player->comMode = 0;
    }
}

void fn_1_62F0(void)
{
    
}

void M526PlayerVibrate(M526_PLAYER *player, int mode, int time)
{
    switch(mode) {
        case 0:
            omVibrate(player->playerNo, time, 6, 6);
            break;
            
        case 1:
            omVibrate(player->playerNo, time, 4, 2);
            break;
            
        case 2:
            omVibrate(player->playerNo, time, 12, 0);
            break;
    }
}


void M526PlayerWaitSet(M526_PLAYER *player)
{
    player->mode = 0;
}

static void M526PlayerWait(M526_PLAYER *player)
{
    M526PlayerMotionSet(player, 0, 0, 8);
    while(player->mode == 0) {
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
}

void M526PlayerModeMoveSet(M526_PLAYER *player)
{
    player->mode = 1;
}

static void M526PlayerModeMove(M526_PLAYER *player)
{
    player->jumpF = FALSE;
    player->hipdropF = FALSE;
    while(player->mode == 1) {
        M526PlayerInput(player);
        if(player->jumpF) {
            player->velY += 2.2f;
            player->vel.y = 35-(0.98f*player->velY);
        }
        if(player->hipdropF) {
            M526PlayerHipDropSet(player);
        }
        M526PlayerMove(player);
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
}

void M526PlayerHipDropSet(M526_PLAYER *player)
{
    player->mode = 2;
}

static void M526PlayerHipDrop(M526_PLAYER *player)
{
    int i;
    HuVecF pos;
    M526PlayerMotionSet(player, 5, 0, 8);
    for(i=0; i<20; i++) {
        M526PlayerCollideCheck(player);
        M526PlayerWallHit(player);
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
    player->velY = 0;
    while(1) {
        pos = M526PlayerPosGet(player);
        pos.y -= 40+(0.98f*player->velY);
        if(pos.y < 0) {
            break;
        }
        M526PlayerPosSet(player, &pos);
        player->velY++;
        M526PlayerCollideCheck(player);
        M526PlayerWallHit(player);
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
    pos.y = 0;
    M526PlayerPosSet(player, &pos);
    M526PlayerHipDropFuse(player);
    M526PlayerVibrate(player, 0, 20);
    M526PlayerMotionSet(player, 9, 0, 8);
    for(i=0; i<10; i++) {
        M526PlayerCollideCheck(player);
        M526PlayerWallHit(player);
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
    M526PlayerMotionSet(player, 6, 0, 8);
    for(i=0; i<30; i++) {
        M526PlayerCollideCheck(player);
        M526PlayerWallHit(player);
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
    player->mode = 1;
}

void M526PlayerHipDropFuse(M526_PLAYER *player)
{
    MathCircle playerCol;
    MathCircle fuseCol;
    HuVecF pos;
    int i;
    M526_FUSE *fuse;
    
    playerCol.center.x = player->pos.x;
    playerCol.center.y = player->pos.z;
    playerCol.radius = player->radius;
    for(i=0; i<3; i++) {
        fuse = FuseObj[player->side][i];
        if(M526FuseCheckActive(fuse)) {
            pos = M526FuseHookPosGet(fuse);
            fuseCol.center.x = pos.x;
            fuseCol.center.y = pos.z;
            fuseCol.radius = 35;
            if(MathCircleCircleCheck(&playerCol, &fuseCol)) {
                if(player->side == 0) {
                    HuAudFXPlay(MSM_SE_M526_06);
                } else {
                    HuAudFXPlay(MSM_SE_M526_07);
                }
                M526FuseStopSet(fuse);
            }
        }
    }
}

void M526PlayerCrossWalkSet(M526_PLAYER *player, HuVecF *pos)
{
    player->walkPos = *pos;
    player->mode = 3;
}

static void M526PlayerCrossWalk(M526_PLAYER *player)
{
    HuVecF walkPos;
    HuVecF pos;
    MathCircle circle;
    MathCircle walkCircle;
    float speed;
    
    speed = 8;
    while(player->mode == 3 || player->mode == 4) {
        walkPos = player->walkPos;
        pos = player->pos;
        player->vel.x = (walkPos.x-pos.x)/speed;
        player->vel.z = (walkPos.z-pos.z)/speed;
        player->vel.y = 0;
        M526PlayerMove(player);
        circle.center.x = pos.x;
        circle.center.y = pos.z;
        circle.radius = 1;
        walkCircle.center.x = walkPos.x;
        walkCircle.center.y = walkPos.z;
        walkCircle.radius = 1;
        if(MathCircleCircleCheck(&circle, &walkCircle)) {
            break;
        }
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
    player->mode = 0;
}

void M526PlayerWinnerWalkSet(M526_PLAYER *player, HuVecF *pos)
{
    player->walkPos = *pos;
    player->mode = 4;
}

static void M526PlayerWinnerWalk(M526_PLAYER *player)
{
    int i;
    M526PlayerCrossWalk(player);
    for(i=0; i<10; i++) {
        player->vel.x = 0;
        player->vel.z = 0.4f;
        player->vel.y = 0;
        M526PlayerMove(player);
        SLEEP_MODECHG(M526_PLAYER, 2);
    }
    player->mode = 0;
}

void M526PlayerWinnerSet(M526_PLAYER *player)
{
    player->mode = 5;
}

static void M526PlayerWinner(M526_PLAYER *player)
{
    CharModelVoicePanSet(player->charNo, 64);
    M526PlayerMotionSet(player, 7, 0, 8);
    while(player->mode == 5) {
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
}

void M526PlayerLaunchSet(M526_PLAYER *player)
{
    player->mode = 6;
}

static void M526PlayerLaunch(M526_PLAYER *player)
{
    HuVecF dist;
    HuVecF pos;
    HuVecF rot;
    HuVecF rotVel;
    
    int i;
    
    M526PlayerDispOff(player);
    M526PlayerMotionSet(player, 8, 0, 8);
    dist.x = 0;
    dist.y = -200;
    dist.z = 990+GameDist[player->side];
    pos.x = frandmod(10)-5;
    pos.y = 0;
    pos.z = 0;
    rot.x = 0;
    rot.y = 0;
    rot.z = 0;
    rotVel.x = frandmod(10)-5;
    rotVel.y = frandmod(30)-15;
    rotVel.z = frandmod(30)-15;
    SLEEP_MODECHG(M526_PLAYER, player->sidePlayerNo*30);
    for(i=0; i<120; i++) {
        float t = 0.008333334f*(i+1);
        pos.y = HuSin(t*180)*600;
        pos.z = dist.z*HuSin(t*90);
        rot.x += rotVel.x;
        rot.y += rotVel.y;
        rot.z += rotVel.z;
        M526PlayerPosSet(player, &pos);
        M526PlayerRotSet(player, &rot);
        if(t > 0.5f) {
            M526PlayerDispOn(player);
        }
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
    M526PlayerDispOff(player);
    while(1) {
        SLEEP_MODECHG(M526_PLAYER, 1);
    }
}

static void M526FuseMain(void);

M526_FUSE *M526FuseCreate(int side, int lane)
{
    M526_FUSE *fuse = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_FUSE), HU_MEMNUM_OVL);
    
    static const int fuseFileTbl[2][3] = {
        M526_HSF_fuse_red_line1,
        M526_HSF_fuse_red_line2,
        M526_HSF_fuse_red_line3,
        M526_HSF_fuse_blue_line1,
        M526_HSF_fuse_blue_line2,
        M526_HSF_fuse_blue_line3
    };
    
    if(fuse == NULL) {
        OSReport("M526FuseCreate Error...\n");
        return NULL;
    }
    fuse->pos.x = 0;
    fuse->pos.y = 0;
    fuse->pos.z = 0;
    fuse->hookPos.x = 0;
    fuse->hookPos.y = 0;
    fuse->hookPos.z = 0;
    fuse->speed = 1;
    fuse->side = side;
    fuse->lane = lane;
    fuse->doneF = FALSE;
    fuse->stopF = FALSE;
    fuse->seId = MSM_SENO_NONE;
    fuse->fuseMdlId = Hu3DModelCreateData(fuseFileTbl[side][lane]);
    Hu3DModelAttrSet(fuse->fuseMdlId, HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
    Hu3DModelCameraSet(fuse->fuseMdlId, CameraBitTbl[side]);
    fuse->hibanaMdlId = Hu3DModelCreateData(M526_HSF_hibana);
    Hu3DModelCameraSet(fuse->hibanaMdlId, CameraBitTbl[side]);
    Hu3DModelScaleSet(fuse->hibanaMdlId, 3, 3, 1);
    Hu3DModelAttrSet(fuse->hibanaMdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelHookSet(fuse->fuseMdlId, FuseHookTbl[fuse->side][fuse->lane], fuse->hibanaMdlId);
    fuse->process = HuPrcChildCreate(M526FuseMain, ObjProcData[4].prio, ObjProcData[4].stackSize, 0, HuPrcCurrentGet());
    fuse->process->property = fuse;
    fuse->mode = 0;
    return fuse;
}

typedef void (*M526_FUSE_FUNC)(M526_FUSE *fuse);

static void M526FuseWait(M526_FUSE *fuse);
static void M526FuseStop(M526_FUSE *fuse);
static void M526FuseIdle(M526_FUSE *fuse);
static void M526FuseExplode(M526_FUSE *fuse);

static M526_FUSE_FUNC FuseModeTbl[] = {
    M526FuseWait,
    M526FuseStop,
    M526FuseIdle,
    M526FuseExplode
};

static void M526FuseMain(void)
{
    M526_FUSE *fuse = HuPrcCurrentGet()->property;
    while(fuse->mode != 4) {
        FuseModeTbl[fuse->mode](fuse);
    }
    HuPrcEnd();
}

void M526FusePosSet(M526_FUSE *fuse, HuVecF *pos)
{
    fuse->pos = *pos;
    Hu3DModelPosSet(fuse->fuseMdlId, fuse->pos.x, fuse->pos.y, fuse->pos.z);
}

void M526FuseDispOn(M526_FUSE *fuse)
{
    Hu3DModelDispOn(fuse->fuseMdlId);
}

void M526FuseDispOff(M526_FUSE *fuse)
{
    Hu3DModelDispOff(fuse->fuseMdlId);
}

BOOL M526FuseCheckActive(M526_FUSE *fuse)
{
    return fuse->mode == 2;
}

BOOL M526FuseCheckDone(M526_FUSE *fuse)
{
    return fuse->doneF;
}

HuVecF M526FusePosGet(M526_FUSE *fuse)
{
    return fuse->pos;
}

HuVecF M526FuseHookPosGet(M526_FUSE *fuse)
{
    return fuse->hookPos;
}

void M526FuseSpeedSet(M526_FUSE *fuse, float speed)
{
    Hu3DMotionSpeedSet(fuse->fuseMdlId, fuse->speed*speed);
    FuseSpeed = speed;
}

void M526FuseWaitSet(M526_FUSE *fuse)
{
    fuse->mode = 0;
}

static void M526FuseWait(M526_FUSE *fuse)
{
    Hu3DModelAttrSet(fuse->fuseMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelDispOff(fuse->hibanaMdlId);
    while(fuse->mode == 0) {
        SLEEP_MODECHG(M526_FUSE, 1);
    }
}

void M526FuseStopSet(M526_FUSE *fuse)
{
    fuse->mode = 1;
}

static void M526FuseStop(M526_FUSE *fuse)
{
    int i;
    Hu3DModelAttrSet(fuse->fuseMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelDispOff(fuse->hibanaMdlId);
    M526SmokeSidePosSet(fuse->side, &fuse->hookPos);
    if(fuse->seId != MSM_SENO_NONE) {
        HuAudFXStop(fuse->seId);
        fuse->seId = MSM_SENO_NONE;
    }
    fuse->delay = 0;
    for(i=0; i<120; i++) {
        fuse->delay++;
        SLEEP_MODECHG(M526_FUSE, 1);
    }
    M526SmokeSidePosSet(fuse->side, &fuse->hookPos);
    for(i=0; i<30; i++) {
        fuse->delay++;
        SLEEP_MODECHG(M526_FUSE, 1);
    }
    fuse->delay = 0;
    fuse->mode = 2;
}

void M526FuseIdleSet(M526_FUSE *fuse)
{
    fuse->mode = 2;
}

static void M526FuseIdle(M526_FUSE *fuse)
{
    Mtx matrix;
    Hu3DModelAttrReset(fuse->fuseMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DMotionSpeedSet(fuse->fuseMdlId, fuse->speed*FuseSpeed);
    Hu3DModelDispOn(fuse->hibanaMdlId);
    Hu3DModelAttrSet(fuse->hibanaMdlId, HU3D_MOTATTR_LOOP);
    if(fuse->seId == MSM_SENO_NONE) {
        if(fuse->side == 0) {
            fuse->seId = HuAudFXPlayPan(MSM_SE_M526_04, 0);
        } else {
            fuse->seId = HuAudFXPlayPan(MSM_SE_M526_05, 127);
        }
    }
    while(fuse->mode == 2) {
        Hu3DModelObjMtxGet(fuse->fuseMdlId, FuseHookTbl[fuse->side][fuse->lane], matrix);
        fuse->hookPos.x = matrix[0][3];
        fuse->hookPos.y = matrix[1][3];
        fuse->hookPos.z = matrix[2][3];
        if(fuse->hookPos.z <= -200) {
            fuse->doneF = TRUE;
            fuse->mode = 0;
        }
        SLEEP_MODECHG(M526_FUSE, 1);
    }
}

void M526FuseExplodeSet(M526_FUSE *fuse)
{
    fuse->mode = 3;
}

static void M526FuseExplode(M526_FUSE *fuse)
{
    Hu3DModelAttrSet(fuse->fuseMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelDispOff(fuse->hibanaMdlId);
    M526SmokeSidePosSet(fuse->side, &fuse->hookPos);
    if(fuse->seId != MSM_SENO_NONE) {
        HuAudFXStop(fuse->seId);
        fuse->seId = MSM_SENO_NONE;
    }
    SLEEP_MODECHG(M526_FUSE, 180);
    fuse->mode = 0;
}

int M526FuseDelayGet(M526_FUSE *fuse)
{
    return fuse->delay;
}

static void M526KingBombMain(void);


M526_KINGBOMB *M526KingBombCreate(int side)
{
    M526_KINGBOMB *kingbomb = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_KINGBOMB), HU_MEMNUM_OVL);
    int i;
    static const int mdlFileTbl[6] = {
        M526_HSF_kingbomb,
        M526_HSF_explosion_bomhei_fire,
        M526_HSF_kingbomb_effect,
        M526_HSF_kingbomb_effect,
        M526_HSF_kingbomb_effect,
        M526_HSF_kingbomb_effect
    };
    static const int motFileTbl[2] = {
        M526_HSF_kingbomb_idle,
        M526_HSF_kingbomb_win
    };
    
    if(kingbomb == NULL) {
        OSReport("M526KingBombCreate Error...\n");
        return NULL;
    }
    kingbomb->pos.x = 0;
    kingbomb->pos.y = 0;
    kingbomb->pos.z = 0;
    kingbomb->side = side;
    for(i=0; i<6; i++) {
        kingbomb->modelId[i] = Hu3DModelCreateData(mdlFileTbl[i]);
        Hu3DModelCameraSet(kingbomb->modelId[i], CameraBitTbl[side]);
    }
    for(i=0; i<5; i++) {
        Hu3DModelAttrSet(kingbomb->modelId[i+1], HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
        Hu3DModelPosSet(kingbomb->modelId[i+1], 0, -100, 0);
    }
    for(i=0; i<2; i++) {
        kingbomb->motId[i] = Hu3DJointMotionData(kingbomb->modelId[0], motFileTbl[i]);
    }
    Hu3DMotionShiftSpeedSet(kingbomb->modelId[0], 1);
    M526KingBombMotionSet(kingbomb, 0, 0, 8);
    kingbomb->process = HuPrcChildCreate(M526KingBombMain, ObjProcData[5].prio, ObjProcData[5].stackSize, 0, HuPrcCurrentGet());
    kingbomb->process->property = kingbomb;
    Hu3DMotionSpeedSet(kingbomb->modelId[0], 0.25f);
    kingbomb->mode = 0;
    return kingbomb;
}

typedef void (*M526_KINGBOMB_FUNC)(M526_KINGBOMB *kingbomb);

static void M526KingBombIdle(M526_KINGBOMB *kingbomb);
static void M526KingBombExplode(M526_KINGBOMB *kingbomb);
static void M526KingBombWin(M526_KINGBOMB *kingbomb);

static M526_KINGBOMB_FUNC KingBombModeTbl[] = {
    M526KingBombIdle,
    M526KingBombExplode,
    M526KingBombWin
};

static void M526KingBombMain(void)
{
    M526_KINGBOMB *kingbomb = HuPrcCurrentGet()->property;
    while(kingbomb->mode != 3) {
        KingBombModeTbl[kingbomb->mode](kingbomb);
    }
    HuPrcEnd();
}

void M526KingBombPosSet(M526_KINGBOMB *kingbomb, HuVecF *pos)
{
    kingbomb->pos = *pos;
    Hu3DModelPosSet(kingbomb->modelId[0], kingbomb->pos.x, kingbomb->pos.y, kingbomb->pos.z);
}

HuVecF M526KingBombPosGet(M526_KINGBOMB *kingbomb)
{
    return kingbomb->pos;
}

void M526KingBombMotionSet(M526_KINGBOMB *kingbomb, int motNo, int start, int end)
{
    u32 attr;
    Hu3DModelAttrReset(kingbomb->modelId[0], HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
    switch(motNo) {
        case 0:
        case 1:
            attr = HU3D_MOTATTR_LOOP;
            break;
    }
    Hu3DMotionShiftSet(kingbomb->modelId[0], kingbomb->motId[motNo], start, end, attr);
}

void M526KingBombIdleSet(M526_KINGBOMB *kingbomb)
{
    kingbomb->mode = 0;
    Hu3DMotionSpeedSet(kingbomb->modelId[0], 0.25f);
}

static void M526KingBombIdle(M526_KINGBOMB *kingbomb)
{
    M526_FUSE *fuse;
    HuVecF pos;
    float speed;
    Hu3DMotionShiftSpeedSet(kingbomb->modelId[0], 1);
    M526KingBombMotionSet(kingbomb, 0, 0, 8);
    while(1) {
        fuse = FuseStartTbl[kingbomb->side][0];
        if(fuse != NULL) {
            pos = M526FuseHookPosGet(fuse);
            speed = 1.2f-(0.0005f*pos.z);
        } else {
            speed = 0.18f;
        }
        Hu3DMotionSpeedSet(kingbomb->modelId[0], speed);
        SLEEP_MODECHG(M526_KINGBOMB, 1);
    }
}

void M526KingBombExplodeSet(M526_KINGBOMB *kingbomb)
{
    kingbomb->mode = 1;
}

static void M526KingBombExplode(M526_KINGBOMB *kingbomb)
{
    M526_CAMERA *camera = CameraObj[kingbomb->side];
    M526_CAMERAVIEW *view = &camera->view[0];
    int j;
    int i;
    HU3DMODELID modelId;
    int bombNo;
    HuVecF pos;
    static float yOfsTbl[4] = {
        250,
        350,
        150,
        450
    };
    
    for(i=0; i<2; i++) {
        M526PlayerVibrate(PlayerObj[kingbomb->side][i], 2, 240);
    }
    pos = M526KingBombPosGet(kingbomb);
    modelId = kingbomb->modelId[1];
    Hu3DModelPosSet(modelId, pos.x, pos.y+200, view->eye.z-800);
    Hu3DModelScaleSet(modelId, 2, 2, 1);
    Hu3DModelAttrReset(modelId, HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
    Hu3DMotionTimeSet(modelId, 0);
    for(i=0; i<17; i++) {
        modelId = kingbomb->modelId[(i%4)+2];
        Hu3DModelPosSet(modelId, pos.x, pos.y+yOfsTbl[i%4], view->eye.z-800);
        Hu3DModelScaleSet(modelId, 1.75f, 1.75f, 1);
        Hu3DModelAttrReset(modelId, HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(modelId, 0);
        SLEEP_MODECHG(M526_KINGBOMB, 13);
        if(i == 0) {
            Hu3DModelDispOff(kingbomb->modelId[0]);
            M526StageShadowOn(StageObj[kingbomb->side]);
            for(j=0; j<3; j++) {
                M526FuseDispOff(FuseObj[kingbomb->side][j]);
            }
            bombNo = 0;
            for(j=55; j>40; j--) {
                M526BombLaunchSet(BombObj[kingbomb->side][j], ((bombNo++)*10)+30);
            }
            for(; j>=0; j--) {
                M526BombDispOff(BombObj[kingbomb->side][j]);
            }
            for(j=0; j<2; j++) {
                M526PlayerLaunchSet(PlayerObj[kingbomb->side][j]);
            }
        }
    }
    M526StageSmokeStartSet(StageObj[kingbomb->side]);
    SLEEP_MODECHG(M526_KINGBOMB, 60);
    for(i=0; i<5; i++) {
        Hu3DModelDispOff(kingbomb->modelId[i+1]);
    }
    M526KingBombIdleSet(kingbomb);
}

void M526KingBombWinSet(M526_KINGBOMB *kingbomb)
{
    kingbomb->mode = 2;
}

static void M526KingBombWin(M526_KINGBOMB *kingbomb)
{
    Hu3DMotionSpeedSet(kingbomb->modelId[0], 10);
    M526KingBombMotionSet(kingbomb, 1, 0, 8);
    while(1) {
        Hu3DMotionSpeedSet(kingbomb->modelId[0], 0.5f);
        SLEEP_MODECHG(M526_KINGBOMB, 1);
    }
}

static void M526ExplosionBombMain(void);

M526_EXPLOSIONBOMB *M526ExplosionBombCreate(void)
{
    M526_EXPLOSIONBOMB *explosionBomb = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_EXPLOSIONBOMB), HU_MEMNUM_OVL);
    int i;
    static const int mdlFileTbl[2] = {
        M526_HSF_explosion_bomhei,
        M526_HSF_explosion_bomhei_fire
    };
    if(explosionBomb == NULL) {
        OSReport("M526ExplosionBombCreate Error...\n");
        return NULL;
    }
    explosionBomb->pos.x = 0;
    explosionBomb->pos.y = 0;
    explosionBomb->pos.z = 0;
    for(i=0; i<2; i++){ 
        explosionBomb->modelId[i] = Hu3DModelCreateData(mdlFileTbl[i]);
        Hu3DModelCameraSet(explosionBomb->modelId[i], HU3D_CAM0);
    }
    Hu3DModelAttrSet(explosionBomb->modelId[1], HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(explosionBomb->modelId[1], 1);
    for(i=0; i<3; i++){ 
        explosionBomb->motId[i] = Hu3DJointMotionData(explosionBomb->modelId[0], ExplosionBombMotFileTbl[i]);
    }
    Hu3DModelAttrSet(explosionBomb->modelId[1], HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
    Hu3DModelShadowSet(explosionBomb->modelId[0]);
    explosionBomb->process = HuPrcChildCreate(M526ExplosionBombMain, ObjProcData[7].prio, ObjProcData[7].stackSize, 0, HuPrcCurrentGet());
    explosionBomb->process->property = explosionBomb;
    explosionBomb->mode = 0;
    return explosionBomb;
}

typedef void (*M526_EXPLOSIONBOMB_FUNC)(M526_EXPLOSIONBOMB *explosionBomb);

static void M526ExplosionBombWalk(M526_EXPLOSIONBOMB *explosionBomb);
static void M526ExplosionBombExplode(M526_EXPLOSIONBOMB *explosionBomb);

static M526_EXPLOSIONBOMB_FUNC ExplosionBombModeTbl[] = {
    M526ExplosionBombWalk,
    M526ExplosionBombExplode,
};

static void M526ExplosionBombMain(void)
{
    M526_EXPLOSIONBOMB *explosionBomb = HuPrcCurrentGet()->property;
    while(explosionBomb->mode != 2) {
        ExplosionBombModeTbl[explosionBomb->mode](explosionBomb);
    }
    HuPrcEnd();
}

void M526ExplosionBombPosSet(M526_EXPLOSIONBOMB *explosionBomb, HuVecF *pos)
{
    int i;
    explosionBomb->pos = *pos;
    for(i=0; i<2; i++) {
        Hu3DModelPosSet(explosionBomb->modelId[i], explosionBomb->pos.x, explosionBomb->pos.y, explosionBomb->pos.z);
    }
}

HuVecF M526ExplosionBombPosGet(M526_EXPLOSIONBOMB *explosionBomb)
{
    return explosionBomb->pos;
}

void M526ExplosionBombMotionSet(M526_EXPLOSIONBOMB *explosionBomb, int motNo, int start, int end, u32 attr)
{
    Hu3DMotionShiftSet(explosionBomb->modelId[0], explosionBomb->motId[motNo], start, end, attr);
}

void M526ExplosionBombWalkSet(M526_EXPLOSIONBOMB *explosionBomb)
{
    explosionBomb->mode = 0;
}

static void M526ExplosionBombWalk(M526_EXPLOSIONBOMB *explosionBomb)
{
    M526ExplosionBombMotionSet(explosionBomb, 0, 0, 8, HU3D_MOTATTR_LOOP);
    while(1) {
        SLEEP_MODECHG(M526_EXPLOSIONBOMB, 1);
    }
}

void M526ExplosionBombExplodeSet(M526_EXPLOSIONBOMB *explosionBomb)
{
    explosionBomb->mode = 1;
}

static void M526ExplosionBombExplode(M526_EXPLOSIONBOMB *explosionBomb)
{
    int i, side;
    M526ExplosionBombMotionSet(explosionBomb, 2, 0, 8, HU3D_MOTATTR_LOOP);
    SLEEP_MODECHG(M526_EXPLOSIONBOMB, 60);
    Hu3DModelAttrReset(explosionBomb->modelId[1], HU3D_MOTATTR_LOOP|HU3D_MOTATTR_PAUSE);
    Hu3DMotionTimeSet(explosionBomb->modelId[1], 0);
    HuAudFXPlay(MSM_SE_M526_01);
    for(side=0; side<2; side++) {
        for(i=0; i<2; i++) {
            M526PlayerVibrate(PlayerObj[side][i], 1, 30);
        }
    }
    SLEEP_MODECHG(M526_EXPLOSIONBOMB, 14);
    Hu3DModelDispOff(explosionBomb->modelId[0]);
    SLEEP_MODECHG(M526_EXPLOSIONBOMB, 11);
    Hu3DModelDispOff(explosionBomb->modelId[1]);
    explosionBomb->mode = 0;
}

static void M526BombMain(void);

M526_BOMB *M526BombCreate(int side, int stageSide)
{
    M526_BOMB *bomb = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_BOMB), HU_MEMNUM_OVL);
    if(bomb == NULL) {
        OSReport("M526BombCreate Error...\n");
        return NULL;
    }
    bomb->pos.x = 0;
    bomb->pos.y = 0;
    bomb->pos.z = 0;
    bomb->rot.x = 0;
    bomb->rot.y = frandmod(50)-25;
    bomb->rot.z = 0;
    bomb->side = side;
    bomb->launchDelay = 0;
    bomb->no = BombNum++;
    bomb->stageSide = stageSide;
    bomb->modelId = Hu3DModelCreateData((stageSide != 0) ? M526_HSF_bomhei2_right : M526_HSF_bomhei2);
    Hu3DModelCameraSet(bomb->modelId, CameraBitTbl[side]);
    Hu3DModelAttrSet(bomb->modelId, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(bomb->modelId, HU3D_ATTR_NOCULL);
    Hu3DMotionTimeSet(bomb->modelId, frandmod(20));
    bomb->process = HuPrcChildCreate(M526BombMain, ObjProcData[6].prio, ObjProcData[6].stackSize, 0, HuPrcCurrentGet());
    bomb->process->property = bomb;
    M526BombPosSet(bomb, &bomb->pos);
    M526BombRotSet(bomb, &bomb->rot);
    bomb->mode = 0;
    return bomb;
}

typedef void (*M526_BOMB_FUNC)(M526_BOMB *bomb);

static void M526BombModeRandom(M526_BOMB *bomb);
static void M526BombLaunch(M526_BOMB *bomb);
static void M526BombExit(M526_BOMB *bomb);
static void M526BombFollow(M526_BOMB *bomb);

static M526_BOMB_FUNC BombModeTbl[] = {
    M526BombModeRandom,
    M526BombLaunch,
    M526BombExit,
    M526BombFollow
};

static void M526BombMain(void)
{
    M526_BOMB *bomb = HuPrcCurrentGet()->property;
    while(bomb->mode != 4) {
        BombModeTbl[bomb->mode](bomb);
    }
    HuPrcEnd();
}

void M526BombPosSet(M526_BOMB *bomb, HuVecF *pos)
{
    bomb->pos = *pos;
    Hu3DModelPosSet(bomb->modelId, bomb->pos.x, bomb->pos.y, bomb->pos.z);
}

HuVecF M526BombPosGet(M526_BOMB *bomb)
{
    return bomb->pos;
}

void M526BombRotSet(M526_BOMB *bomb, HuVecF *rot)
{
    bomb->rot = *rot;
    Hu3DModelRotSet(bomb->modelId, bomb->rot.x, bomb->rot.y, bomb->rot.z);
}

HuVecF M526BombRotGet(M526_BOMB *bomb)
{
    return bomb->rot;
}

void M526BombDispOn(M526_BOMB *bomb)
{
    Hu3DModelDispOn(bomb->modelId);
}

void M526BombDispOff(M526_BOMB *bomb)
{
    Hu3DModelDispOff(bomb->modelId);
}

void M526BombModeRandomSet(M526_BOMB *bomb)
{
    bomb->mode = 0;
}

static void M526BombModeWait(M526_BOMB *bomb);
static void M526BombModeRotate(M526_BOMB *bomb);
static void M526BombModelUpdate(M526_BOMB *bomb);

static void M526BombModeRandom(M526_BOMB *bomb)
{
    while(bomb->mode == 0) {
        switch(frandmod(2)) {
            case 0:
                M526BombModeWait(bomb);
                break;
            
            case 1:
                M526BombModeRotate(bomb);
                break;
        }
    }
}

static void M526BombModeWait(M526_BOMB *bomb)
{
    SLEEP_MODECHG(M526_BOMB, frandmod(600)+60);
}

static void M526BombModeRotate(M526_BOMB *bomb)
{
    HuVecF rot;
    
    int i;
    int side;
    
    rot = M526BombRotGet(bomb);
    side = frandmod(2);
    for(i=0; i<120; i++) {
        if(side == 0) {
            rot.y += 3;
        } else {
            rot.y -= 3;
        }
        M526BombRotSet(bomb, &rot);
        SLEEP_MODECHG(M526_BOMB, 1);
    }
}

void M526BombLaunchSet(M526_BOMB *bomb, int delay)
{
    bomb->launchDelay = delay;
    bomb->mode = 1;
}

static void M526BombLaunch(M526_BOMB *bomb)
{
    M526_CAMERAVIEW *view;
    int i;
    M526_CAMERA *camera;
    
    HuVecF startPos;
    HuVecF endPos;
    HuVecF pos;
    HuVecF rot;
    M526BombDispOff(bomb);
    camera = CameraObj[bomb->side];
    view = &camera->view[0];
    startPos.x = frandmod(200)-100;
    startPos.y = 250;
    startPos.z = view->eye.z-2200;
    if(frandmod(2) == 0) {
        endPos.x = frandmod(100)+100;
        endPos.x = (frandmod(2) == 0) ? endPos.x : (-endPos.x);
        endPos.y = startPos.y+frandmod(250);
        endPos.y = (frandmod(2) == 0) ? endPos.y : (-endPos.y+300);
    } else {
        endPos.x = frandmod(300);
        endPos.x = (frandmod(2) == 0) ? endPos.x : (-endPos.x);
        endPos.y = startPos.y+(frandmod(100)+100);
        endPos.y = (frandmod(2) == 0) ? endPos.y : (-endPos.y+300);
    }
    endPos.z = view->eye.z;
    SLEEP_MODECHG(M526_BOMB, bomb->launchDelay);
    Hu3DMotionTimeSet(bomb->modelId, 0);
    Hu3DModelAttrSet(bomb->modelId, HU3D_MOTATTR_PAUSE);
    M526BombDispOn(bomb);
    rot.x = 0;
    rot.y = frandmod(60)-30;
    rot.z = 0;
    M526BombRotSet(bomb, &rot);
    for(i=0; i<40; i++) {
        float t = 0.025f*(i+1);
        pos.x = startPos.x+(t*(endPos.x-startPos.x));
        pos.y = startPos.y+(t*(endPos.y-startPos.y));
        pos.z = startPos.z+(t*(endPos.z-startPos.z));
        M526BombPosSet(bomb, &pos);
        SLEEP_MODECHG(M526_BOMB, 1);
    }
    M526BombDispOff(bomb);
    while(1) {
        SLEEP_MODECHG(M526_BOMB, 1);
    }
}

void M526BombExitSet(M526_BOMB *bomb)
{
    bomb->mode = 2;
}

static void M526BombExit(M526_BOMB *bomb)
{
    HuVecF pos;
    int i;
    pos = M526BombPosGet(bomb);
    bomb->vel.x = (pos.x > 0) ? 3.5f : -3.5f;
    bomb->vel.y = 0;
    bomb->vel.z = 0;
    for(i=0; i<240; i++) {
        M526BombModelUpdate(bomb);
        SLEEP_MODECHG(M526_BOMB, 1);
    }
    M526BombDispOff(bomb);
    while(1) {
        SLEEP_MODECHG(M526_BOMB, 1);
    }
}

void M526BombFollowSet(M526_BOMB *bomb)
{
    bomb->mode = 3;
}

static void M526BombFollow(M526_BOMB *bomb)
{
    float dist = (bomb->no%2) ? 0 : 70;
    M526_CAMERA *camera;
    M526_CAMERAVIEW *view;
    while(1) {
        camera = CameraObj[bomb->side];
        view = &camera->view[0];
        while((view->eye.z-850)-dist < bomb->pos.z) {
            bomb->vel.x = 0;
            bomb->vel.y = 0;
            bomb->vel.z = -2;
            M526BombModelUpdate(bomb);
            SLEEP_MODECHG(M526_BOMB, 1);
        }
        SLEEP_MODECHG(M526_BOMB, frandmod(40)+20);
    }
    
}

static void M526BombModelUpdate(M526_BOMB *bomb)
{
    HuVecF pos = bomb->pos;
    HuVecF rot = bomb->rot;
    float speed = HuMagXZVecF(&bomb->vel);
    rot.x = 0;
    rot.z = 0;
    rot.y = M526AngleLerp(rot.y, HuAtan(bomb->vel.x, bomb->vel.z), 0.4f);
    pos.x += bomb->vel.x;
    pos.z += bomb->vel.z;
    pos.y += bomb->vel.y;
    M526BombPosSet(bomb, &pos);
    M526BombRotSet(bomb, &rot);
}

static void M526SmokeMain(void);

M526_SMOKE *M526SmokeCreate(int side)
{
    M526_SMOKE *smoke = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M526_SMOKE), HU_MEMNUM_OVL);
    if(smoke == NULL) {
        OSReport("M526SmokeCreate Error...\n");
        return NULL;
    }
    smoke->pos.x = 0;
    smoke->pos.y = 0;
    smoke->pos.z = 0;
    smoke->side = side;
    smoke->modelId = Hu3DModelCreateData(M526_HSF_smoke);
    Hu3DModelCameraSet(smoke->modelId, CameraBitTbl[side]);
    smoke->process = HuPrcChildCreate(M526SmokeMain, ObjProcData[8].prio, ObjProcData[8].stackSize, 0, HuPrcCurrentGet());
    smoke->process->property = smoke;
    M526SmokePosSet(smoke, &smoke->pos);
    smoke->mode = 0;
    return smoke;
}

typedef void (*M526_SMOKE_FUNC)(M526_SMOKE *smoke);

static void M526SmokeHide(M526_SMOKE *smoke);
static void M526SmokeShow(M526_SMOKE *smoke);

static M526_SMOKE_FUNC SmokeModeTbl[] = {
    M526SmokeHide,
    M526SmokeShow,
};

static void M526SmokeMain(void)
{
    M526_SMOKE *smoke = HuPrcCurrentGet()->property;
    while(1) {
        SmokeModeTbl[smoke->mode](smoke);
    }
}

void M526SmokePosSet(M526_SMOKE *smoke, HuVecF *pos)
{
    smoke->pos = *pos;
    Hu3DModelPosSet(smoke->modelId, smoke->pos.x, smoke->pos.y, smoke->pos.z);
}

HuVecF M526SmokePosGet(M526_SMOKE *smoke)
{
    return smoke->pos;
}

void M526SmokeHideSet(M526_SMOKE *smoke)
{
    smoke->mode = 0;
}

static void M526SmokeHide(M526_SMOKE *smoke)
{
    Hu3DModelDispOff(smoke->modelId);
    while(1) {
        SLEEP_MODECHG(M526_SMOKE, 1);
    }
}

void M526SmokeShowSet(M526_SMOKE *smoke)
{
    smoke->mode = 1;
}

static void M526SmokeShow(M526_SMOKE *smoke)
{
    Hu3DModelPosSet(smoke->modelId, smoke->pos.x, smoke->pos.y, smoke->pos.z);
    Hu3DMotionTimeSet(smoke->modelId, 0);
    Hu3DModelDispOn(smoke->modelId);
    SLEEP_MODECHG(M526_SMOKE, 120);
    smoke->mode = 0;
}

BOOL M526SmokeDispGet(M526_SMOKE *smoke)
{
    return (smoke->mode) ? TRUE : FALSE;
}

void M526SmokeSidePosSet(int side, HuVecF *pos)
{
    int i;
    for(i=0; i<6; i++) {
        M526_SMOKE *smoke = SmokeObj[side][i];
        if(!M526SmokeDispGet(smoke)) {
            M526SmokePosSet(smoke, pos);
            smoke->mode = 1;
            return;
        }
    }
}

static float AudioRadius = 300;

s16 M526Audio3DPanGet(s16 side, HuVecF pos)
{
    float posX = pos.x;
    s16 result;
    if(side == 0) {
        result = ((HuAbs(posX) < AudioRadius) ? ((s16)((16/AudioRadius)*posX)+48) : ((posX > 0) ? 64 : 32));
    } else {
        result = ((HuAbs(posX) < AudioRadius) ? ((s16)((16/AudioRadius)*posX)+80) : ((posX > 0) ? 96 : 64));
    }
    return result;
}

void M526Audio3DPlay(int id, s16 side, HuVecF pos)
{
    int result = HuAudFXPlayPan(id, M526Audio3DPanGet(side, pos));
}
