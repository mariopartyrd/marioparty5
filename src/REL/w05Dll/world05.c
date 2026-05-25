#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/scroll.h"
#include "game/board/opening.h"
#include "game/board/masu.h"
#include "game/board/branch.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/board/com.h"
#include "game/board/status.h"
#include "game/board/camera.h"
#include "game/board/star.h"
#include "game/board/guide.h"
#include "game/board/coin.h"

#include "game/board/capsule.h"

#include "game/wipe.h"
#include "game/mg/timer.h"

#include "datanum/effect.h"

#include "datanum/w05.h"
#include "messnum/board_w05.h"
#include "messnum/board_capmachine.h"

#define MB5EV_MASU_ROCKET_JET MASU_FLAG_BIT(15)
#define MB5EV_MASU_TAXI_JUMP MASU_FLAG_BIT(14)
#define MB5EV_MASU_TAXI MASU_FLAG_BIT(13)
#define MB5EV_MASU_ROCKET MASU_FLAG_BIT_MULTI(10, 3)
#define MB5EV_MASU_MOVEFAN MASU_FLAG_BIT_MULTI(8, 2)

#define MB5EV_MASU_REEL_R MASU_FLAG_BIT(6)
#define MB5EV_MASU_REEL_L MASU_FLAG_BIT(7)
#define MB5EV_MASU_REEL (MB5EV_MASU_REEL_R|MB5EV_MASU_REEL_L)

#define MB5EV_MASU_DOREMI MASU_FLAG_BIT_MULTI(4, 2)
#define MB5EV_MASU_WARP_JUMP MASU_FLAG_BIT(3)
#define MB5EV_MASU_WARP MASU_FLAG_BIT(2)

#define MB5EV_ROCKET_JET_COIN_MAX 32

typedef struct MB5_Work_s {
    int bgMdlId; //0x00
    int areaBaseMdlId[2]; //0x04
    int bgPillarMdlId; //0x0C
    int startMdlId; //0x10
    int symBaseMdlId; //0x14
    int symMdlId; //0x18
    int rocketPathMdlId[3]; //0x1C
    int launchBMdlId[3]; //0x28
    int rocketMdlId[3]; //0x34
    int launchB2MdlId[4]; //0x40
    int doremiMdlId[3]; //0x50
    int A2KatMdlId[2]; //0x5C
    int taxiMdlId[2]; //0x64
    int autoFanMdlId[2]; //0x6C
    int autoFanJetMdlId[2]; //0x74
    int laserMdlId; //0x7C
    int roboMdlId; //0x80
    int roboArmLMdlId; //0x84
    int roboArmL4MdlId; //0x88
    int roboArmRMdlId; //0x8C
    int roboArmR4MdlId; //0x90
    int roboButtonMdlId; //0x94
    int reelMdlId[3]; //0x98
    int roboLampMdlId[3]; //0xA4
    int RLampMdlId[2]; //0xB0
    int warpA1MdlId[2]; //0xB8
    int warpC2MdlId[2]; //0xC0
    int arrowMdlId[22]; //0xC8
    int warpMdlId[4]; //0x120
    int warpLayerMdlId[4]; //0x130
    int radarBodyMdlId[2]; //0x140
    int treeMdlId[2]; //0x148
    int rocketLightMdlId[2]; //0x150
    int PLampMdlId[2]; //0x158
    int rjetBaseMdlId; //0x160
    int rjetArmMdlId[4]; //0x164
    int rjetArmHookMdlId[4]; //0x174
    int rocketJetMdlId[4]; //0x184
    int taxiPathMdlId[2]; //0x194
    int moveFanMdlId[2]; //0x19C
    int taxi1MdlId[2]; //0x1A4
    int unk1AC[3];
    int taxiRingMdlId[10]; //0x1B8
} MB5_WORK;

typedef struct MB5Ev_RocketJetCoin_s {
    int mdlId; //0x00
    int mode; //0x04
    int time; //0x08
    BOOL enableF; //0x0C
    HuVecF pos; //0x10
    HuVecF rot; //0x1C
    HuVecF scale; //0x28
    HuVecF startPos; //0x34
    HuVecF endPos; //0x40
    int alpha; //0x4C
} MB5EV_ROCKET_JET_COIN;

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

static BOOL mb5ev_DoremiF[3] = {};
static int mb5ev_TaxiPlayerPrev = -1;
static float mb5ev_ReelTime[6] = {};
static float mb5ev_RocketJetYRotVel[4] = { 0.2f, -0.2f, -0.2f, 0.2f };
static float mb5ev_RocketJetYRot[4] = { -30, 0, 30, 0 };
static float mb5ev_RocketJetSpeed = 1;
static int mb5ev_RocketJetPlayerPrev = -1;
static int mb5ev_MoveFanPlayerPrev = -1;
static int mb5ev_MoveFanSeNo = MSM_SENO_NONE;
static s16 mb5ev_MoveFanCurrNo = -1;

static int numberFileTbl[] = {
    BOARD_HSF_num0,
    BOARD_HSF_num1,
    BOARD_HSF_num2,
    BOARD_HSF_num3,
    BOARD_HSF_num4,
    BOARD_HSF_num5,
    BOARD_HSF_num6,
    BOARD_HSF_num7,
    BOARD_HSF_num8,
    BOARD_HSF_num9
};

static HuVecF mb5ev_PlayerOfsTbl[CHARNO_MAX] = {
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { -10, 0, 0 },
    { 0, 0, 0 },
    { 0, -20, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
};

static HuVecF mb5ev_LaunchB2PosTbl[4] = {
    { -3740, 2305, -3200 },
    { -1525, 2100, -2315 },
    { 2935, 3200, -2695 },
    { 4790, 2730, -2430 }
};

static HuVecF mb5ev_RocketPosTbl[3] = {
    { -1086, 750, -811 },
    { 0, 750, -1185 },
    { 1086, 750, -811 },
};

static HuVecF mb5ev_RoboLampPosTbl[3] = {
    { 1335, 3430, -2929 },
    { 1405.5, 3430, -2999.5 },
    { 1476, 3430, -3070 }
};

static HuVecF mb5ev_RLampPosTbl[2] = {
    { 935, 2960, -2735 },
    { 1665, 2960, -3465 }
};

static HuVecF mb5ev_ArrowPosTbl[22] = {
    { 1000, 325, 2200 },
    { -900, 490, 1265 },
    { 900, 490, 1265 },
    { -2315, 1775, -965 },
    { -4005, 1795, -540 },
    { -3415, 1775, -1915 },
    { -2545, 2100, -2365 },
    { 2380, 2585, -1030 },
    { 2390, 2960, -2300 },
    { 5205, 2755, -1450 },
    { 3415, 2755, -1850 },
    { -1500, 490, -55 },
    { -765, 540, -1025 },
    { 765, 540, -1025 },
    { 1500, 490, -55 },
    { -5420, 1900, -2190 },
    { -4400, 2240, -2865 },
    { -2270, 2330, -3425 },
    { 1100, 2960, -2395 },
    { 2400, 3200, -3505 },
    { 4480, 3200, -3475 },
    { -3300, 2305, -3075 }
};

static HuVecF mb5ev_ArrowRotTbl[22] = {
    { 0, 0, 0 },
    { 0, -45, 0 },
    { 0, 45, 0 },
    { 0, 0, 0 },
    { 0, 45, 0 },
    { 0, 0, 0 },
    { 0, 45, 0 },
    { 0, -25, 0 },
    { 0, 0, 0 },
    { 0, 35, 0 },
    { 0, 0, 0 },
    { 0, -115, 0 },
    { 0, -155, 0 },
    { 0, 155, 0 },
    { 0, 115, 0 },
    { 0, -135, 0 },
    { 0, 180, 0 },
    { 0, 180, 0 },
    { 0, -135, 0 },
    { 0, -155, 0 },
    { 0, 180, 0 },
    { 0, 180, 0 }
};

static HuVecF mb5ev_WarpPosTbl[4] = {
    { -1860, 505, 550 },
    { -1990, 1965, -1540 },
    { 1860, 505, 550 },
    { 1785, 2765, -1535 }
};

static HuVecF mb5ev_A2KatPosTbl[2] = {
    { -4825, 1965, -1820 }, 
    { 4615, 2820, -1820 }
};

static HuVecF mb5ev_RadarBodyPosTbl[2] = {
    { -1800, 320, 1390 },
    { 1800, 320, 1390 }
};

static HuVecF mb5ev_RadarBodyRotTbl[2] = {
    { 0, 45, 0 },
    { 0, -45, 0 }
};

static HuVecF mb5ev_TreePosTbl[2] = {
    { -1180, 320, 2000 },
    { 1180, 320, 2000 }
};

static HuVecF mb5ev_TreeRotTbl[2] = {
    { 0, -45, 0 },
    { 0, 45, 0 }
};

static HuVecF mb5ev_RocketLightPosTbl[2] = {
    { -1400, 615, -515 },
    { 1400, 615, -515 }
};

static HuVecF mb5ev_RocketLightRotTbl[2] = {
    { 0, 45, 0 },
    { 0, -45, 0 }
};

static HuVecF mb5ev_WarpA1PosTbl[2] = {
    { -1860, 315, 550 },
    { -1990, 1775, -1540 }
};

static HuVecF mb5ev_WarpC2PosTbl[2] = {
    { 1860, 315, 550 },
    { 1785, 2575, -1535 }
};

static HuVecF mb5ev_PLamp1PosTbl[6] = {
    { -3845, 1875, -1935 },
    { 4135, 2730, -2495 },
    { -3095, 3380, -2610 },
    { -2320, 3380, -3345 },
    { -3055, 3380, -4120 },
    { -3830, 3380, -3385 }
};

static HuVecF mb5ev_Taxi1PosTbl[2] = {
    { -4375, 1965, -1485 },
    { 4165, 2820, -1475 }
};

static HuVecF mb5ev_LaunchBPosTbl[3] = {
    { -800, 700, -525 },
    { 0, 700, -780 },
    { 800, 700, -525 }
};

static HuVecF mb5ev_LaunchBRotTbl[3] = {
    { 0, 45, 0 },
    { 0, 0, 0 },
    { 0, -45, 0 },
};

static HuVecF mb5ev_RoboArm4PosTbl[2] = {
    { 1085, 3305, -2930 },
    { 1474.5, 3305, -3319.5 }
};

static HuVecF mb5ev_TaxiRingPosTbl[10] = {
    { -2400, 3140, -1835 },
    { -1800, 3500, -1835 },
    { -1200, 3800, -1835 },
    { -600, 3985, -1835 },
    { 0, 4045, -1835 },
    { 600, 4070, -1835 },
    { 1200, 4045, -1835 },
    { 1800, 3955, -1835 },
    { 2400, 3775, -1835 },
    { 3000, 3550, -1835 }
};

static MB5_WORK mb5_Work;
static MBMODELID unkMdlId;
static MBMODELID numberMdlId[5];
static HuVecF mb5ev_WarpEndSize;
static unsigned int mb5ev_WarpMasuId;
static int lbl_1_bss_BAC[50];
static unsigned int mb5ev_ReelLMasuId;
static unsigned int mb5ev_ReelRMasuId;
static Mtx mb5ev_RobLArmMtx;
static Mtx mb5ev_RobRArmMtx;
static int mb5ev_RobLArmMdlId;
static int mb5ev_RobRArmMdlId;
static HuVecF mb5ev_RobLArmPos;
static HuVecF mb5ev_RobRArmPos;
static unsigned int mb5ev_DoremiMasuId[3];
static Mtx mb5ev_DoremiMtx[3];
static int mb5ev_DoremiMdlId[3];
static HuVecF mb5ev_DoremiPos[3];
static MB5EV_ROCKET_JET_COIN mb5ev_RocketJetCoin[MB5EV_ROCKET_JET_COIN_MAX];
static int mb5ev_RocketJetCoinTotal;
static ANIMDATA *mb5ev_WarpAnim;
static ANIMDATA *mb5ev_RocketDustAnim;
static HU3DPARMANID mb5ev_WarpParmanId;
static HU3DPARMANID mb5ev_RocketDustParmanId[13];
static BOOL mb5ev_MoveFanResetF;
static BOOL mb5ev_RocketJetResetF;
static BOOL mb5ev_RocketJetCancelF;
static BOOL mb5ev_RocketJetStopF;
static BOOL mb5ev_TaxiResetF;
static BOOL mb5ev_RobRHookF;
static BOOL mb5ev_RobLHookF;
static BOOL lbl_1_bss_10;
static int mb5ev_WarpPathLen;
static BOOL mb5ev_WarpPathEndF;
static BOOL mb5ev_WarpPathValidF;
static OMOBJ *mb5_MainObj;

void MB5_Create(void);
void MB5_Kill(void);
void MB5_LightSet(void);
void MB5_LightReset(void);
int MB5Ev_MasuWalkPre(int playerNo);
int MB5Ev_MasuWalkPost(int playerNo);
int MB5Ev_MasuHatena(int playerNo);
int MB5_MasuPathCheck(s16 id, u32 flag, s16 *linkTbl, BOOL endF);
static void MB5_MainObjUpdate(OMOBJ *obj);

void ObjectSetup(void)
{
    GWPartyFSet(TRUE);
    MBObjectSetup(MBNO_WORLD_5, MB5_Create, MB5_Kill);
}

void MB5_Create(void)
{
    HuVecF openingRot = { -33, 0, 0 };
    HuVecF openingPos = { 416, 1156, -426 };
    HuVecF mapRot = { -78, 0, 0 };
    HuVecF mapPos = { 0, 250, -1180 };
    int boardNo = MBBoardNoGet();
    int i;
    int mdlId;
    for(i=0; i<5; i++) {
        numberMdlId[i] = MBModelCreate(numberFileTbl[i], NULL, FALSE);
        MBModelDispSet(numberMdlId[i], FALSE);
    }
    MBMasuCreate(W05_BIN_masuData);
    MBScrollInit(W05_HSF_scrollCol, 20000, 0);
    MBLightHookSet(MB5_LightSet, MB5_LightReset);
    MBCapsuleColCreate(W05_HSF_capsuleCol);
    mb5_Work.symBaseMdlId = MBModelCreate(W05_HSF_symBase, NULL, FALSE);
    mb5_Work.symMdlId = mdlId = MBModelCreate(W05_HSF_sym, NULL, FALSE);
    MBMotionSpeedSet(mdlId, 1);
    MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    mb5_Work.bgMdlId = MBModelCreate(W05_HSF_bg, NULL, FALSE);
    for(i=0; i<2; i++) {
        mb5_Work.areaBaseMdlId[i] = mdlId = MBModelCreate(W05_HSF_A1Base+i, NULL, FALSE);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    mb5_Work.bgPillarMdlId = mdlId = MBModelCreate(W05_HSF_bgPillar, NULL, FALSE);
    MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    mb5_Work.startMdlId = mdlId = MBModelCreate(W05_HSF_start, NULL, FALSE);
    MBMotionSpeedSet(mdlId, 1);
    MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    for(i=0; i<3; i++) {
        mb5_Work.launchBMdlId[i] = mdlId = MBModelCreate(W05_HSF_launchB, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_LaunchBPosTbl[i]);
        MBModelRotSetV(mdlId, &mb5ev_LaunchBRotTbl[i]);
        MBMotionSpeedSet(mdlId, 0);
    }
    for(i=0; i<3; i++) {
        mb5_Work.rocketPathMdlId[i] = mdlId = MBModelCreate(W05_HSF_rocketPathR+i, NULL, FALSE);
        MBMotionSpeedSet(mdlId, 0);
    }
    for(i=0; i<3; i++) {
        mb5_Work.rocketMdlId[i] = mdlId = MBModelCreate(W05_HSF_rocketR+i, NULL, FALSE);
        MBMotionSpeedSet(mdlId, 0);
        MBModelPosSetV(mdlId, &mb5ev_RocketPosTbl[i]);
    }
    for(i=0; i<4; i++) {
        mb5_Work.launchB2MdlId[i] = mdlId = MBModelCreate(W05_HSF_launchB2, NULL, TRUE);
        MBMotionSpeedSet(mdlId, 0);
        MBModelPosSetV(mdlId, &mb5ev_LaunchB2PosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    MBModelRotSet(mb5_Work.launchB2MdlId[3], 0, 30, 0);
    for(i=0; i<3; i++) {
        mb5_Work.doremiMdlId[i] = mdlId = MBModelCreate(W05_HSF_do+i, NULL, FALSE);
        MBMotionSpeedSet(mdlId, 0);
        MBMotionTimeSet(mdlId, 0);
    }
    for(i=0; i<2; i++) {
        mb5_Work.A2KatMdlId[i] = mdlId = MBModelCreate(W05_HSF_A2kat, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_A2KatPosTbl[i]);
        MBMotionSpeedSet(mdlId, 0);
    }
    MBModelRotSet(mb5_Work.A2KatMdlId[1], 0, 180, 0);
    for(i=0; i<2; i++) {
        mb5_Work.taxiPathMdlId[i] = mdlId = MBModelCreate(W05_HSF_airTaxiPath1+i, NULL, FALSE);
        MBMotionSpeedSet(mdlId, 0);
    }
    for(i=0; i<2; i++) {
        mb5_Work.taxiMdlId[i] = mdlId = MBModelCreate(W05_HSF_airTaxi, NULL, TRUE);
        MBMotionSpeedSet(mdlId, 0);
    }
    for(i=0; i<2; i++) {
        mb5_Work.moveFanMdlId[i] = mdlId = MBModelCreate(W05_HSF_moveFan1+i, NULL, FALSE);
        MBMotionSpeedSet(mdlId, 0);
        MBModelRotSet(mdlId, 0, 0, 0);
        if(i == 0) {
            MBMotionTimeSet(mdlId, 100);
        }
    }
    for(i=0; i<2; i++) {
        mb5_Work.autoFanMdlId[i] = mdlId = MBModelCreate(W05_HSF_autoFan, NULL, TRUE);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<2; i++) {
        mb5_Work.autoFanJetMdlId[i] = mdlId = MBModelCreate(W05_HSF_autoFanJet, NULL, TRUE);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    mb5_Work.laserMdlId = mdlId = MBModelCreate(W05_HSF_laser, NULL, FALSE);
    MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    MBMotionSpeedSet(mdlId, 1);
    MBMotionStartEndSet(mdlId, 2, 20);
    for(i=0; i<2; i++) {
        mb5_Work.warpA1MdlId[i] = mdlId = MBModelCreate(W05_HSF_warpA1, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_WarpA1PosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<2; i++) {
        mb5_Work.warpC2MdlId[i] = mdlId = MBModelCreate(W05_HSF_warpC2, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_WarpC2PosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<4; i++) {
        mb5_Work.warpMdlId[i] = mdlId = MBModelCreate(W05_HSF_warp, NULL, TRUE);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
        MBMotionSpeedSet(mdlId, 1);
        MBModelPosSetV(mdlId, &mb5ev_WarpPosTbl[i]);
        MBModelDispSet(mdlId, FALSE);
    }
    for(i=0; i<4; i++) {
        mb5_Work.warpLayerMdlId[i] = mdlId = MBModelCreate(W05_HSF_warpLayer, NULL, TRUE);
        MBMotionSpeedSet(mdlId, 0);
        MBModelPosSetV(mdlId, &mb5ev_WarpPosTbl[i]);
        MBModelDispSet(mdlId, FALSE);
    }
    mb5_Work.roboMdlId = MBModelCreate(W05_HSF_robo, NULL, FALSE);
    mb5_Work.roboArmLMdlId = mdlId = MBModelCreate(W05_HSF_roboArmL, NULL, FALSE);
    MBMotionSpeedSet(mdlId, 0);
    mb5_Work.roboArmRMdlId = mdlId = MBModelCreate(W05_HSF_roboArmR, NULL, FALSE);
    MBMotionSpeedSet(mdlId, 0);
    mb5_Work.roboArmL4MdlId = mdlId = MBModelCreate(W05_HSF_roboArmR4, NULL, TRUE);
    MBMotionSpeedSet(mdlId, 0);
    MBModelPosSetV(mdlId, &mb5ev_RoboArm4PosTbl[1]);
    mb5_Work.roboArmR4MdlId = mdlId = MBModelCreate(W05_HSF_roboArmR4, NULL, TRUE);
    MBMotionSpeedSet(mdlId, 0);
    MBModelPosSetV(mdlId, &mb5ev_RoboArm4PosTbl[0]);
    mb5_Work.roboButtonMdlId = mdlId = MBModelCreate(W05_HSF_roboButton, NULL, FALSE);
    MBMotionSpeedSet(mdlId, 0);
    for(i=0; i<2; i++) {
        mb5_Work.RLampMdlId[i] = mdlId = MBModelCreate(W05_HSF_rlamp, NULL, TRUE);
        MBMotionSpeedSet(mdlId, 0);
        MBModelPosSetV(mdlId, &mb5ev_RLampPosTbl[i]);
    }
    for(i=0; i<3; i++) {
        mb5_Work.reelMdlId[i] = mdlId = MBModelCreate(W05_HSF_reel1+i, NULL, FALSE);
        MBMotionSpeedSet(mdlId, 0);
        mb5ev_ReelTime[i] = frandmod(8)*20.0f;
        MBMotionTimeSet(mdlId, mb5ev_ReelTime[i]);
    }
    for(i=0; i<3; i++) {
        mb5_Work.roboLampMdlId[i] = mdlId = MBModelCreate(W05_HSF_roboLamp, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_RoboLampPosTbl[i]);
        MBMotionSpeedSet(mdlId, 0);
    }
    for(i=0; i<2; i++) {
        mb5_Work.radarBodyMdlId[i] = mdlId = MBModelCreate(W05_HSF_radarBody, NULL, TRUE);
        MBModelRotSetV(mdlId, &mb5ev_RadarBodyRotTbl[i]);
        MBModelPosSetV(mdlId, &mb5ev_RadarBodyPosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<2; i++) {
        mb5_Work.treeMdlId[i] = mdlId = MBModelCreate(W05_HSF_tree, NULL, TRUE);
        MBModelRotSetV(mdlId, &mb5ev_TreeRotTbl[i]);
        MBModelPosSetV(mdlId, &mb5ev_TreePosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<2; i++) {
        mb5_Work.rocketLightMdlId[i] = mdlId = MBModelCreate(W05_HSF_rocketLight, NULL, TRUE);
        MBModelRotSetV(mdlId, &mb5ev_RocketLightRotTbl[i]);
        MBModelPosSetV(mdlId, &mb5ev_RocketLightPosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<2; i++) {
        mb5_Work.PLampMdlId[i] = mdlId = MBModelCreate(W05_HSF_plamp1, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_PLamp1PosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    mb5_Work.rjetBaseMdlId = mdlId = MBModelCreate(W05_HSF_rjetBase, NULL, FALSE);
    MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    for(i=0; i<4; i++) {
        mb5_Work.rjetArmMdlId[i] = mdlId = MBModelCreate(W05_HSF_rjetArm, NULL, TRUE);
    }
    for(i=0; i<4; i++) {
        mb5_Work.rjetArmHookMdlId[i] = mdlId = MBModelCreate(W05_HSF_rjetArm1Hook+i, NULL, FALSE);
    }
    for(i=0; i<4; i++) {
        if(i%2 == 0) {
            mb5_Work.rocketJetMdlId[i] = mdlId = MBModelCreate(W05_HSF_rjet, NULL, TRUE);
            MBMotionStartEndSet(mdlId, 0, 80);
        } else {
            mb5_Work.rocketJetMdlId[i] = mdlId = MBModelCreate(W05_HSF_coinkar, NULL, TRUE);
        }
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<22; i++) {
        mb5_Work.arrowMdlId[i] = mdlId = MBModelCreate(W05_HSF_arrow1, NULL, TRUE);
        MBModelRotSetV(mdlId, &mb5ev_ArrowRotTbl[i]);
        MBModelPosSetV(mdlId, &mb5ev_ArrowPosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<2; i++) {
        mb5_Work.taxi1MdlId[i] = mdlId = MBModelCreate(W05_HSF_taxi1, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_Taxi1PosTbl[i]);
    }
    MBModelRotSet(mb5_Work.taxi1MdlId[1], 0, 180, 0);
    for(i=0; i<10; i++) {
        mb5_Work.taxiRingMdlId[i] = mdlId = MBModelCreate(W05_HSF_taxiRing, NULL, TRUE);
        MBModelPosSetV(mdlId, &mb5ev_TaxiRingPosTbl[i]);
        MBMotionSpeedSet(mdlId, 1);
        MBModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
        MBModelDispSet(mdlId, FALSE);
    }
    HuDataDirClose(DATA_w05);
    MBMasuWalkPreHookSet(MB5Ev_MasuWalkPre);
    MBMasuWalkPostHookSet(MB5Ev_MasuWalkPost);
    MBMasuHatenaHookSet(MB5Ev_MasuHatena);
    MBBranchFlagSet(MB5EV_MASU_TAXI_JUMP|MB5EV_MASU_WARP_JUMP);
    MBMasuPathCheckHookSet(MB5_MasuPathCheck);
    MBOpeningViewSet(&openingRot, &openingPos, 22648);
    MBMapViewCameraSet(&mapRot, &mapPos, 24188);
    HuAudSndGrpSetSet(MSM_GRP_BD5);
    mb5_MainObj = MBAddObj(32768, 0, 0, MB5_MainObjUpdate);
}

void MB5_Kill(void)
{
    int i;
    if(unkMdlId >= 0) {
        MBModelKill(unkMdlId);
        unkMdlId = MB_MODEL_NONE;
    }
    for(i=0; i<22; i++) {
        if(mb5_Work.arrowMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.arrowMdlId[i]);
        }
    }
    for(i=0; i<10; i++) {
        if(mb5_Work.taxiRingMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.taxiRingMdlId[i]);
        }
    }
    for(i=0; i<4; i++) {
        if(mb5_Work.warpMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.warpMdlId[i]);
        }
        if(mb5_Work.warpLayerMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.warpLayerMdlId[i]);
        }
        if(mb5_Work.rjetArmMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.rjetArmMdlId[i]);
        }
        if(mb5_Work.rocketJetMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.rocketJetMdlId[i]);
        }
        if(mb5_Work.rjetArmHookMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.rjetArmHookMdlId[i]);
        }
    }
    for(i=0; i<3; i++) {
        if(mb5_Work.doremiMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.doremiMdlId[i]);
        }
        if(mb5_Work.launchBMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.launchBMdlId[i]);
        }
        if(mb5_Work.rocketPathMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.rocketPathMdlId[i]);
        }
        if(mb5_Work.rocketMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.rocketMdlId[i]);
        }
        if(mb5_Work.reelMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.reelMdlId[i]);
        }
        if(mb5_Work.roboLampMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.roboLampMdlId[i]);
        }
    }
    for(i=0; i<2; i++) {
        if(mb5_Work.areaBaseMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.areaBaseMdlId[i]);
        }
        if(mb5_Work.A2KatMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.A2KatMdlId[i]);
        }
        if(mb5_Work.taxiMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.taxiMdlId[i]);
        }
        if(mb5_Work.taxiPathMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.taxiPathMdlId[i]);
        }
        if(mb5_Work.autoFanMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.autoFanMdlId[i]);
        }
        if(mb5_Work.moveFanMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.moveFanMdlId[i]);
        }
        if(mb5_Work.autoFanJetMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.autoFanJetMdlId[i]);
        }
        if(mb5_Work.RLampMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.RLampMdlId[i]);
        }
        if(mb5_Work.warpA1MdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.warpA1MdlId[i]);
        }
        if(mb5_Work.warpC2MdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.warpC2MdlId[i]);
        }
        if(mb5_Work.radarBodyMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.radarBodyMdlId[i]);
        }
        if(mb5_Work.treeMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.treeMdlId[i]);
        }
        if(mb5_Work.rocketLightMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.rocketLightMdlId[i]);
        }
        if(mb5_Work.PLampMdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.PLampMdlId[i]);
        }
        if(mb5_Work.taxi1MdlId[i] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.taxi1MdlId[i]);
        }
    }
    if(mb5_Work.symBaseMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.symBaseMdlId);
    }
    if(mb5_Work.symMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.symMdlId);
    }
    if(mb5_Work.bgMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.bgMdlId);
    }
    if(mb5_Work.bgPillarMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.bgPillarMdlId);
    }
    if(mb5_Work.startMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.startMdlId);
    }
    for(i=0; i<4; i++) {
        if(mb5_Work.launchB2MdlId[0] != MB_MODEL_NONE) {
            MBModelKill(mb5_Work.launchB2MdlId[0]);
        }
    }
    if(mb5_Work.roboMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.roboMdlId);
    }
    if(mb5_Work.roboArmLMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.roboArmLMdlId);
    }
    if(mb5_Work.roboArmRMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.roboArmRMdlId);
    }
    if(mb5_Work.roboArmL4MdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.roboArmL4MdlId);
    }
    if(mb5_Work.roboArmR4MdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.roboArmR4MdlId);
    }
    if(mb5_Work.roboButtonMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.roboButtonMdlId);
    }
    if(mb5_Work.laserMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.laserMdlId);
    }
    if(mb5_Work.rjetBaseMdlId != MB_MODEL_NONE) {
        MBModelKill(mb5_Work.rjetBaseMdlId);
    }
    for(i=0; i<5; i++) {
        if(numberMdlId[i] >= 0) {
            MBModelKill(numberMdlId[i]);
            numberMdlId[i] = MB_MODEL_NONE;
        }
    }
    mb5_MainObj = NULL;
}

static void MB5_MainObjUpdate(OMOBJ *obj)
{
    int i; //r31
    int j; //r30
    int reelPlayerNum; //r29
    int warpPlayerNum; //r28
    int moveFanMdlId; //r27
    int taxiMdlId; //r26
    int rjetBaseMdlId; //r25
   
    float RArm1RotY; //f31
    float mfMotTime; //f30

    
    HuVecF doremiPos[3][GW_PLAYER_MAX]; //0x1F8
    int doremiPlayer[3][GW_PLAYER_MAX]; //0x1C8
    Mtx mfMtx; //0x198
    Mtx taxiMtx; //0x168
    Mtx armMtx; //0x138
    Mtx RJetPMtx; //0x108
    Mtx robArmRotMtx; //0xD8
    HuVecF warpPos[GW_PLAYER_MAX]; //0xA8
    HuVecF reelPos[GW_PLAYER_MAX]; //0x78
    int warpPlayer[GW_PLAYER_MAX]; //0x68
    int reelPlayer[GW_PLAYER_MAX]; //0x58
    int doremiPlayerNum[3]; //0x4C
    int rjetArmHookMdlId[4]; //0x3C
    HuVecF mfRot; //0x30
    HuVecF taxiRot; //0x24
    HuVecF armRot; //0x18
    HuVecF RJetPRot; //0xC
    
    
    static char *doremiHookTbl[3] = {
        "tar1hook",
        "tar2hook",
        "tar3hook"
    };

    static char *moveFanHookTbl[2] = {
        "mfLhook",
        "mfRhook"
    };

    static char *taxiHookTbl[2] = {
        "taxiLhook",
        "taxiRhook"
    };

    static char *armHookTbl[4] = {
        "arm1hook",
        "arm2hook",
        "arm3hook",
        "arm4hook"
    };
    
    MBMotionSpeedSet(mb5_Work.rjetBaseMdlId, mb5ev_RocketJetSpeed);
    for(i=0; i<4; i++) {
        if(i != 0 && i != 2) {
            MBMotionSpeedSet(mb5_Work.rocketJetMdlId[i], mb5ev_RocketJetSpeed);
        }
    }
    rjetBaseMdlId = MBModelIdGet(mb5_Work.rjetBaseMdlId);
    Hu3DModelObjMtxGet(rjetBaseMdlId, "Rjctrhook", RJetPMtx);
    Hu3DMtxRotGet(RJetPMtx, &RJetPRot);
    for(i=0; i<4; i++) {
        if(i != 0 || (i == 0 && !mb5ev_RocketJetStopF)) {
            mb5ev_RocketJetYRot[i] += mb5ev_RocketJetSpeed*mb5ev_RocketJetYRotVel[i];
            if(mb5ev_RocketJetYRot[i] <= -30) {
                mb5ev_RocketJetYRotVel[i] = 0.2f;
            } else if(mb5ev_RocketJetYRot[i] >= 30) {
                mb5ev_RocketJetYRotVel[i] = -0.2f;
            }
        }
        if(RJetPRot.x <= -179.0f || RJetPRot.x >= 179.0f) {
            RArm1RotY = RJetPRot.y-(90.0f*i);
        } else {
            RArm1RotY = RJetPRot.y+(90.0f*i);
        }
        MBModelPosSet(mb5_Work.rjetArmMdlId[i], RJetPMtx[0][3], RJetPMtx[1][3], RJetPMtx[2][3]);
        MBModelRotSet(mb5_Work.rjetArmMdlId[i], RJetPRot.x+mb5ev_RocketJetYRot[i], RArm1RotY, RJetPRot.z);
        MBModelPosSet(mb5_Work.rjetArmHookMdlId[i], RJetPMtx[0][3], RJetPMtx[1][3], RJetPMtx[2][3]);
        MBModelRotSet(mb5_Work.rjetArmHookMdlId[i], RJetPRot.x+mb5ev_RocketJetYRot[i], RArm1RotY, RJetPRot.z);
        rjetArmHookMdlId[i] = MBModelIdGet(mb5_Work.rjetArmHookMdlId[i]);
        Hu3DModelObjMtxGet(rjetArmHookMdlId[i], armHookTbl[i], armMtx);
        Hu3DMtxRotGet(armMtx, &armRot);
        MBModelRotSet(mb5_Work.rocketJetMdlId[i], armRot.z, armRot.y, armRot.z);
        MBModelPosSet(mb5_Work.rocketJetMdlId[i], armMtx[0][3], armMtx[1][3], armMtx[2][3]);
    }
    if(mb5ev_RocketJetResetF && GwSystem.turnPlayerNo != mb5ev_RocketJetPlayerPrev) {
        mb5ev_RocketJetSpeed = 1;
        mb5ev_RocketJetResetF = FALSE;
        mb5ev_RocketJetCancelF = FALSE;
        MBMotionSpeedSet(mb5_Work.rjetBaseMdlId, 1);
        MBMotionTimeSet(mb5_Work.rjetBaseMdlId, 0);
        for(i=0; i<4; i++) {
            MBMotionSpeedSet(mb5_Work.rocketJetMdlId[i], 1);
            MBMotionTimeSet(mb5_Work.rocketJetMdlId[i], 0);
        }
        MBMotionStartEndSet(mb5_Work.rocketJetMdlId[0], 0, 80);
        MBModelAttrSet(mb5_Work.rocketJetMdlId[0], HU3D_MOTATTR_LOOP);
        MBMotionStartEndSet(mb5_Work.rocketJetMdlId[2], 0, 80);
        MBModelAttrSet(mb5_Work.rocketJetMdlId[2], HU3D_MOTATTR_LOOP);
        mb5ev_RocketJetYRotVel[1] = mb5ev_RocketJetYRotVel[2] = -0.2f;
        mb5ev_RocketJetYRotVel[0] = mb5ev_RocketJetYRotVel[3] = 0.2f;
        mb5ev_RocketJetYRot[0] = -30;
        mb5ev_RocketJetYRot[2] = 30;
        mb5ev_RocketJetYRot[1] = mb5ev_RocketJetYRot[3] = 0;
    }
    warpPlayerNum = reelPlayerNum = 0;
    doremiPlayerNum[0] = doremiPlayerNum[1] = doremiPlayerNum[2] = 0;
    if(mb5ev_RobRHookF) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(mb5ev_ReelRMasuId == GwPlayer[i].masuId) {
                MBPlayerPosGet(i, &reelPos[reelPlayerNum]);
                reelPlayer[reelPlayerNum] = i;
                reelPlayerNum++;
            }
        }
        mb5ev_RobRArmMdlId = MBModelIdGet(mb5_Work.roboArmRMdlId);
        Hu3DModelObjMtxGet(mb5ev_RobRArmMdlId, "robRhook", mb5ev_RobRArmMtx);
        MTXRotDeg(robArmRotMtx, 'Y', -45);
        MTXConcat(mb5ev_RobRArmMtx, robArmRotMtx, mb5ev_RobRArmMtx);
        MBMasuMtxSet(mb5ev_ReelRMasuId, mb5ev_RobRArmMtx);
        for(i=0; i<reelPlayerNum; i++) {
            reelPos[i].x += mb5ev_RobRArmMtx[0][3]-mb5ev_RobRArmPos.x;
            reelPos[i].y += mb5ev_RobRArmMtx[1][3]-mb5ev_RobRArmPos.y;
            reelPos[i].z += mb5ev_RobRArmMtx[2][3]-mb5ev_RobRArmPos.z;
            MBPlayerPosSetV(reelPlayer[i], &reelPos[i]);
        }
        mb5ev_RobRArmPos.x = mb5ev_RobRArmMtx[0][3];
        mb5ev_RobRArmPos.y = mb5ev_RobRArmMtx[1][3];
        mb5ev_RobRArmPos.z = mb5ev_RobRArmMtx[2][3];
        if(MBMotionEndCheck(mb5_Work.roboArmRMdlId)) {
            mb5ev_RobRHookF = FALSE;
        }
    }
    if(mb5ev_RobLHookF) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(mb5ev_ReelLMasuId == GwPlayer[i].masuId) {
                MBPlayerPosGet(i, &warpPos[warpPlayerNum]);
                warpPlayer[warpPlayerNum] = i;
                warpPlayerNum++;
            }
        }
        mb5ev_RobLArmMdlId = MBModelIdGet(mb5_Work.roboArmLMdlId);
        Hu3DModelObjMtxGet(mb5ev_RobLArmMdlId, "robLhook", mb5ev_RobLArmMtx);
        MTXRotDeg(robArmRotMtx, 'Y', -45);
        MTXConcat(mb5ev_RobLArmMtx, robArmRotMtx, mb5ev_RobLArmMtx);
        MBMasuMtxSet(mb5ev_ReelLMasuId, mb5ev_RobLArmMtx);
        for(i=0; i<warpPlayerNum; i++) {
            warpPos[i].x += mb5ev_RobLArmMtx[0][3]-mb5ev_RobLArmPos.x;
            warpPos[i].y += mb5ev_RobLArmMtx[1][3]-mb5ev_RobLArmPos.y;
            warpPos[i].z += mb5ev_RobLArmMtx[2][3]-mb5ev_RobLArmPos.z;
            MBPlayerPosSetV(warpPlayer[i], &warpPos[i]);
        }
        mb5ev_RobLArmPos.x = mb5ev_RobLArmMtx[0][3];
        mb5ev_RobLArmPos.y = mb5ev_RobLArmMtx[1][3];
        mb5ev_RobLArmPos.z = mb5ev_RobLArmMtx[2][3];
        if(MBMotionEndCheck(mb5_Work.roboArmLMdlId)) {
            mb5ev_RobLHookF = FALSE;
        }
    }
    for(i=0; i<3; i++) {
        if(mb5ev_DoremiF[i]) {
            for(j=0; j<GW_PLAYER_MAX; j++) {
                if(GwPlayer[j].masuId == mb5ev_DoremiMasuId[i]) {
                    MBPlayerPosGet(j, &doremiPos[i][doremiPlayerNum[i]]);
                    doremiPlayer[i][doremiPlayerNum[i]] = j;
                    doremiPlayerNum[i]++;
                }
            }
            mb5ev_DoremiMdlId[i] = MBModelIdGet(mb5_Work.doremiMdlId[i]);
            Hu3DModelObjMtxGet(mb5ev_DoremiMdlId[i], doremiHookTbl[i], mb5ev_DoremiMtx[i]);
            MBMasuMtxSet(mb5ev_DoremiMasuId[i], mb5ev_DoremiMtx[i]);
            for(j=0; j<doremiPlayerNum[i]; j++) {
                doremiPos[i][j].x += mb5ev_DoremiMtx[i][0][3]-mb5ev_DoremiPos[i].x;
                doremiPos[i][j].y += mb5ev_DoremiMtx[i][1][3]-mb5ev_DoremiPos[i].y;
                doremiPos[i][j].z += mb5ev_DoremiMtx[i][2][3]-mb5ev_DoremiPos[i].z;
                MBPlayerPosSetV(doremiPlayer[i][j], &doremiPos[i][j]);
            }
            mb5ev_DoremiPos[i].x = mb5ev_DoremiMtx[i][0][3];
            mb5ev_DoremiPos[i].y = mb5ev_DoremiMtx[i][1][3];
            mb5ev_DoremiPos[i].z = mb5ev_DoremiMtx[i][2][3];
            if(MBMotionEndCheck(mb5_Work.doremiMdlId[i])) {
                mb5ev_DoremiF[i] = FALSE;
            }
        }
    }
    if(mb5ev_TaxiResetF && GwSystem.turnPlayerNo != mb5ev_TaxiPlayerPrev) {
        for(i=0; i<2; i++) {
            MBMotionTimeSet(mb5_Work.A2KatMdlId[i], 0);
            MBMotionSpeedSet(mb5_Work.A2KatMdlId[i], 0);
            MBMotionTimeSet(mb5_Work.taxiMdlId[i], 0);
            MBMotionSpeedSet(mb5_Work.taxiMdlId[i], 0);
            MBMotionTimeSet(mb5_Work.taxiPathMdlId[i], 0);
            MBMotionSpeedSet(mb5_Work.taxiPathMdlId[i], 0);
            MBModelDispSet(mb5_Work.taxiMdlId[i], TRUE);
            MBModelDispSet(mb5_Work.taxiMdlId[i], TRUE);
            mb5ev_TaxiPlayerPrev = -1;
            mb5ev_TaxiResetF = FALSE;
        }
    }
    for(i=0; i<2; i++) {
        moveFanMdlId = MBModelIdGet(mb5_Work.moveFanMdlId[i]);
        Hu3DModelObjMtxGet(moveFanMdlId, moveFanHookTbl[i], mfMtx);
        Hu3DMtxRotGet(mfMtx, &mfRot);
        MBModelPosSet(mb5_Work.autoFanMdlId[i], mfMtx[0][3], mfMtx[1][3], mfMtx[2][3]);
        MBModelPosSet(mb5_Work.autoFanJetMdlId[i], mfMtx[0][3], mfMtx[1][3], mfMtx[2][3]);
        MBModelRotSetV(mb5_Work.autoFanMdlId[i], &mfRot);
        MBModelRotSetV(mb5_Work.autoFanJetMdlId[i], &mfRot);
    }
    for(i=0; i<2; i++) {
        taxiMdlId = MBModelIdGet(mb5_Work.taxiPathMdlId[i]);
        Hu3DModelObjMtxGet(taxiMdlId, taxiHookTbl[i], taxiMtx);
        Hu3DMtxRotGet(taxiMtx, &taxiRot);
        MBModelPosSet(mb5_Work.taxiMdlId[i], taxiMtx[0][3], taxiMtx[1][3]-30, taxiMtx[2][3]);
        MBModelRotSetV(mb5_Work.taxiMdlId[i], &taxiRot);
    }
    if(mb5ev_MoveFanResetF) {
        MBMotionTimeSet(mb5_Work.moveFanMdlId[0], 101);
        MBMotionSpeedSet(mb5_Work.moveFanMdlId[0], 0);
        mb5ev_MoveFanResetF = FALSE;
        mb5ev_MoveFanPlayerPrev = -1;
    }
    if(mb5ev_MoveFanCurrNo != -1) {
        mfMotTime = MBMotionTimeGet(mb5_Work.moveFanMdlId[mb5ev_MoveFanCurrNo]);
        if(260.0f == mfMotTime) {
            mb5ev_MoveFanSeNo = HuAudFXPlay(MSM_SE_BD5_21);
        }
        if(MBMotionEndCheck(mb5_Work.moveFanMdlId[mb5ev_MoveFanCurrNo])) {
            HuAudFXStop(mb5ev_MoveFanSeNo);
            mb5ev_MoveFanSeNo = MSM_SENO_NONE;
            mb5ev_MoveFanCurrNo = -1;
        }
    }
    if(MBKillCheck() || mb5_MainObj == NULL) {
        omDelObj(mbObjMan, obj);
        mb5_MainObj = NULL;
        return;
    }
}

void MB5_LightSet(void)
{
    HuVecF pos;
    HuVecF dir;
    GXColor color;
    int gLightId;
    
    Hu3DShadowCreate(45, 1000, 250000);
    Hu3DShadowTPLvlSet(0.3f);
    Hu3DBGColorSet(255, 255, 255);
    pos.x = -2700;
    pos.y = 4700;
    pos.z = 2300;
    dir.x = 2700;
    dir.y = -4700;
    dir.z = -2300;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    gLightId = Hu3DGLightCreate(pos.x, pos.y, pos.z, dir.x, dir.y, dir.z, color.r, color.g, color.b);
    Hu3DGLightInfinitytSet(gLightId);
}

void MB5_LightReset(void)
{
    
}

void MB5Ev_Taxi(int playerNo);
void MB5Ev_Reel(int playerNo);
void MB5Ev_Rocket(int playerNo);
void MB5Ev_RocketJet(int playerNo);
void MB5Ev_Warp(int playerNo);
void MB5Ev_MoveFan(int playerNo);

int MB5Ev_MasuHatena(int playerNo)
{
    s16 masuId = GwPlayer[playerNo].masuId;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    u32 flag = masuP->flag & MASU_FLAG_MAP;
    if(flag & MB5EV_MASU_TAXI) {
        MB5Ev_Taxi(playerNo);
    } else if(flag & MB5EV_MASU_REEL) {
        MB5Ev_Reel(playerNo);
    } else if(flag & MB5EV_MASU_ROCKET) {
        u32 no = (flag & MB5EV_MASU_ROCKET) >> 10;
        if(no >= 5) {
            MB5Ev_Rocket(playerNo);
        }
        
    } else if(flag & MB5EV_MASU_ROCKET_JET) {
        MB5Ev_RocketJet(playerNo);
    }
}

int MB5_MasuStub()
{
    return FALSE;
}

int MB5Ev_MasuWalkPre(int playerNo)
{
    return FALSE;
}

int MB5Ev_MasuWalkPost(int playerNo)
{
    s16 masuId = GwPlayer[playerNo].masuId;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    u32 flag = masuP->flag & MASU_FLAG_MAP;
    int type = MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId);
    u32 no;
    if(flag & MB5EV_MASU_WARP) {
        MB5Ev_Warp(playerNo);
    }
    if(flag & MB5EV_MASU_DOREMI) {
        no = (flag & MB5EV_MASU_DOREMI) >> 4;
        MBMotionSpeedSet(mb5_Work.doremiMdlId[no-1], 1.1f);
        MBMotionTimeSet(mb5_Work.doremiMdlId[no-1], 0);
        mb5ev_DoremiMasuId[no-1] = masuId;
        mb5ev_DoremiF[no-1] = TRUE;
        MBPlayerPosGet(playerNo, &mb5ev_DoremiPos[no-1]);
    }
    if(flag & MB5EV_MASU_MOVEFAN) {
        MB5Ev_MoveFan(playerNo);
    }
    if(flag & MB5EV_MASU_REEL) {
        HuAudFXPlay(MSM_SE_BD5_14);
        no = (flag & MB5EV_MASU_REEL) >> 6;
        if(no == 1) {
            mb5ev_ReelRMasuId = masuId;
            mb5ev_RobRHookF = TRUE;
            MBPlayerPosGet(playerNo, &mb5ev_RobRArmPos);
            MBMotionSpeedSet(mb5_Work.roboArmRMdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmRMdlId, 0);
            MBMotionSpeedSet(mb5_Work.roboArmR4MdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmR4MdlId, 0);
        } else if(no == 2) {
            mb5ev_ReelLMasuId = masuId;
            mb5ev_RobLHookF = TRUE;
            MBPlayerPosGet(playerNo, &mb5ev_RobLArmPos);
            MBMotionSpeedSet(mb5_Work.roboArmLMdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmLMdlId, 0);
            MBMotionSpeedSet(mb5_Work.roboArmL4MdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmL4MdlId, 0);
        }
    }
    return FALSE;
}

void MB5Ev_WarpPathCheck(s16 masuId);
void MB5Ev_WarpParticleCreate(HuVecF pos);
void MB5Ev_WarpPlayerShrink(int playerNo);
void MB5Ev_WarpPlayerGrow(int playerNo);

void MB5Ev_Warp(int playerNo)
{
    int i; //r31
    int j; //r29
    s16 masuId; //r28
    int choice; //r27
    s16 nextMasuId; //r26
    MASU *masuP; //r25
    BOOL acceptF; //r24
    
    float speed; //f31
    float scaleSpeed; //f30
    
    HuVecF playerPos; //sp+0x40
    HuVecF destPos; //sp+0x34
    HuVecF scale; //sp+0x28
    u32 flag; //sp+0xC
    s16 jumpMasuId[2]; //sp+0x8
    
    choice = 0;
    acceptF = FALSE;
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    flag = masuP->flag & MASU_FLAG_MAP;
    mb5ev_WarpPathValidF = FALSE;
    mb5ev_WarpPathEndF = FALSE;
    mb5ev_WarpMasuId = masuId;
    mb5ev_WarpPathLen = 0;
    MBWalkNumDispSet(playerNo, FALSE);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    HuPrcSleep(10);
    repeatChoice:
    MBWinCreateChoice(MBWIN_TYPE_RESULT, MESS_BOARD_W05_WARP_CHOICE, HUWIN_SPEAKER_NONE, 0);
    if(GwPlayer[playerNo].comF) {
        MB5Ev_WarpPathCheck(masuId);
        if(!mb5ev_WarpPathValidF) {
            MBComChoiceSetUp();
        } else {
            MBComChoiceSetDown();
        }
    }
    MBTopWinWait();
    choice = MBWinLastChoiceGet();
    MBTopWinKill();
    switch(choice) {
        case 0:
            acceptF = TRUE;
            break;
           
        case 2:
            MBScrollExec(playerNo);
            MBStatusDispSetAll(TRUE);
            break;
    }
    if(choice == 2) {
        goto repeatChoice;
    }
    if(acceptF) {
        HuPrcSleep(10);
        jumpMasuId[0] = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, masuId, MB5EV_MASU_WARP_JUMP);
        GwPlayer[playerNo].masuIdPrev = GwPlayer[playerNo].masuId;
        GwPlayer[playerNo].masuIdNext = jumpMasuId[0];
        MBPlayerMasuWalk(playerNo, TRUE);
        GwPlayer[playerNo].masuId = jumpMasuId[0];
        HuPrcSleep(10);
        MBPlayerRotateStart(playerNo, 0, 15);
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        HuPrcSleep(5);
        omVibrate(playerNo, 120, 6, 6);
        MBPlayerPosGet(playerNo, &playerPos);
        MB5Ev_WarpParticleCreate(playerPos);
        scale.x = scale.z  = 1;
        scale.y = 0;
        scaleSpeed = 0.01f;
        for(i=0; i<4; i++) {
            MBModelDispSet(mb5_Work.warpMdlId[i], TRUE);
        }
        for(i=0, speed=1; i<90; i++) {
            speed += 0.3f;
            if((scale.y += scaleSpeed) >= 1) {
                scale.y = 1;
            }
            for(j=0; j<4; j++) {
                MBMotionSpeedSet(mb5_Work.warpMdlId[j], speed);
                MBModelScaleSetV(mb5_Work.warpMdlId[j], &scale);
            }
            if(i == 20) {
                HuAudFXPlay(MSM_SE_BD5_09);
            }
            HuPrcVSleep();
        }
        scale.y = 1;
        for(i=0; i<4; i++) {
            MBModelDispSet(mb5_Work.warpMdlId[i], FALSE);
            MBModelDispSet(mb5_Work.warpLayerMdlId[i], TRUE);
            MBMotionSpeedSet(mb5_Work.warpLayerMdlId[i], 1.1f);
            MBMotionTimeSet(mb5_Work.warpLayerMdlId[i], 0);
        }
        MB5Ev_WarpPlayerShrink(playerNo);
        HuPrcSleep(90);
        Hu3DParManKill(mb5ev_WarpParmanId);
        mb5ev_WarpAnim = NULL;
        jumpMasuId[1] = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, jumpMasuId[0], MB5EV_MASU_WARP_JUMP);
        MBMasuPosGet(MASU_LAYER_DEFAULT, jumpMasuId[1], &destPos);
        MBPlayerPosSetV(playerNo, &destPos);
        MBCameraPlayerViewSet(playerNo, NULL, NULL, -1, -1, 60);
        MBCameraMotionWait();
        MBPlayerRotateStart(playerNo, 0, 1);
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        HuPrcSleep(30);
        MB5Ev_WarpParticleCreate(destPos);
        omVibrate(playerNo, 120, 6, 6);
        for(i=0; i<70; i++) {
            if(i == 40) {
                HuAudFXPlay(MSM_SE_BD5_10);
            }
            HuPrcVSleep();
        }
        HuPrcSleep(10);
        for(i=0; i<4; i++) {
            MBModelAttrSet(mb5_Work.warpLayerMdlId[i], HU3D_MOTATTR_REV);
            MBMotionSpeedSet(mb5_Work.warpLayerMdlId[i], 1.1f);
        }
        MB5Ev_WarpPlayerGrow(playerNo);
        HuPrcSleep(10);
        for(i=0; i<4; i++) {
            MBModelAttrReset(mb5_Work.warpLayerMdlId[i], HU3D_MOTATTR_REV);
            MBMotionSpeedSet(mb5_Work.warpLayerMdlId[i], 0);
            MBMotionTimeSet(mb5_Work.warpLayerMdlId[i], 0);
            MBModelDispSet(mb5_Work.warpLayerMdlId[i], FALSE);
            MBModelDispSet(mb5_Work.warpMdlId[i], TRUE);
            MBMotionSpeedSet(mb5_Work.warpMdlId[i], speed);
            MBModelAttrSet(mb5_Work.warpMdlId[i], HU3D_MOTATTR_REV);
        }
        scale.y -= 0.05f;
        for(i=0; i<60.0f; i++) {
            speed -= 0.2f;
            if((scale.y -= scaleSpeed*2) <= 0) {
                scale.y = 0;
            }
            for(j=0; j<4; j++) {
                MBMotionSpeedSet(mb5_Work.warpMdlId[j], speed);
                MBModelScaleSetV(mb5_Work.warpMdlId[j], &scale);
            }
            HuPrcVSleep();
        }
        MBCameraPlayerViewSet(playerNo, NULL, NULL, -1, -1, 6);
        MBCameraMotionWait();
        nextMasuId = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, jumpMasuId[1], MB5EV_MASU_WARP);
        GwPlayer[playerNo].masuIdPrev = jumpMasuId[1];
        GwPlayer[playerNo].masuIdNext = nextMasuId;
        MBPlayerMasuWalk(playerNo, TRUE);
        GwPlayer[playerNo].masuId = nextMasuId;
        MBCameraSkipSet(TRUE);
        Hu3DParManKill(mb5ev_WarpParmanId);
        mb5ev_WarpAnim = NULL;
    }
    MBWalkNumDispSet(playerNo, TRUE);
    for(i=0; i<4; i++) {
        MBModelAttrReset(mb5_Work.warpMdlId[i], HU3D_MOTATTR_REV);
    }
}

void MB5Ev_RocketDustCreate(HuVecF pos);
u32 MB5_EventMasuFlagGet(int no, u32 flag);

void MB5Ev_Rocket(int playerNo)
{
    s16 no; //r31
    s16 i; //r30
    s16 j; //r28
    s16 countdownNum; //r27
    s16 overlapNum; //r26
    s16 masuId; //r25
    MASU *rocketMasuP; //r24
    int seNo1; //r23
    int seNo2; //r22
    s16 numTime; //r21
    s16 numAlpha; //r20
    int playerMdlId; //r19
    u32 flagNo; //r18
    int pathMdlId; //r17
    
    
    float time; //f31
    float numScale; //f30
    float t; //f29
    float startTime; //f28
    float dustTime; //f27
    float numScaleSpeed; //f26
    
    HuVecF overlapPos[GW_PLAYER_MAX]; //sp+0x108
    Mtx hookMtx; //sp+0xD8
    Mtx masuMtx; //sp+0xA8
    HuVecF playerPos; //sp+0x9C
    HuVecF pos; //sp+0x90
    HuVecF rocketPos; //sp+0x84
    HuVecF cameraRot; //sp+0x78
    HuVecF cameraPos; //sp+0x6C
    HuVecF startPos; //sp+0x60
    
    HuVecF rocketMasuPos; //sp+0x54
    HuVecF cornerPos; //sp+0x48
    
    s16 overlapPlayer[GW_PLAYER_MAX]; //sp+0x28
    int rocketMdlId; //sp+0x24
    float zoom; //sp+0x20
    u32 flag; //sp+0x1C
    MASU *masuP; //sp+0x18
    MBCAMERA *cameraP; //sp+0x14
    s16 numAlphaSpeed; //sp+0xA
    s16 rocketMasuId; //sp+0x8
    
    static HuVecF rocketEndPos[3] = {
        { -1086, 750, -811 },
        { 0, 750, -1185 },
        { 1086, 750, -811 }
    };
    static char *rocketHookTbl[3] = {
        "rockRhook",
        "rockGhook",
        "rockBhook"
    };
    static HuVecF rocketStartPos[3] = {
        { -1086, 1180, -811 },
        { 0, 1180, -1185 },
        { 1086, 1180, -811 }
    };
    static playerRotTbl[3] = { 45, 0, -45 };
    
    cameraP = MBCameraGet();
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    flag = masuP->flag & MASU_FLAG_MAP;
    flagNo = (flag & MB5EV_MASU_ROCKET) >> 10;
    MBPlayerPosFixFlagSet(FALSE);
    countdownNum = 3;
    numAlphaSpeed = 4;
    numScaleSpeed = 1.0f/60.0f;
    switch(flagNo) {
        case 5:
            MBPlayerRotateStart(playerNo, 225, 15);
            while(!MBPlayerRotateCheck(playerNo)) {
                HuPrcVSleep();
            }
            break;
            
        case 6:
            MBPlayerRotateStart(playerNo, 180, 15);
            while(!MBPlayerRotateCheck(playerNo)) {
                HuPrcVSleep();
            }
            break;
        
        case 7:
            MBPlayerRotateStart(playerNo, 135, 15);
            while(!MBPlayerRotateCheck(playerNo)) {
                HuPrcVSleep();
            }
            break;
    }
    no = flagNo-5;
    MBPlayerPosGet(playerNo, &startPos);
    for(i=overlapNum=0; i<GW_PLAYER_MAX; i++) {
        if(i != playerNo && masuId == GwPlayer[i].masuId) {
            overlapPlayer[overlapNum] = i;
            MBPlayerPosGet(i, &overlapPos[i]);
            overlapNum++;
        }
    }
    if(overlapNum > 0 && flagNo != 6) {
        MBPlayerPosGet(overlapPlayer[0], &playerPos);
        MBMasuCornerRotPosGet(masuId, 3, &cornerPos);
        MBPlayerWalkMain(overlapPlayer[0], NULL, &cornerPos, MB_PLAYER_MOT_RUN, 1, HU3D_MOTATTR_LOOP, 20, NULL, TRUE);
        MBPlayerRotateStart(overlapPlayer[0], 0, 15);
    }
    MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_W05_ROCKET_START, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBTopWinKill();
    MBCameraPlayerViewSet(playerNo, NULL, NULL, 3200, -1, 30);
    MBCameraMotionWait();
    pathMdlId = MBModelIdGet(mb5_Work.rocketPathMdlId[no]);
    MBMotionSpeedSet(mb5_Work.rocketPathMdlId[no], 1);
    MBMotionTimeSet(mb5_Work.rocketPathMdlId[no], 1);
    MBMotionSpeedSet(mb5_Work.launchBMdlId[no], 1);
    MBMotionTimeSet(mb5_Work.launchBMdlId[no], 1);
    rocketMdlId = MBModelIdGet(mb5_Work.rocketMdlId[no]);
    MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 1);
    MBMotionTimeSet(mb5_Work.rocketMdlId[no], 0);
    playerMdlId = MBPlayerModelGet(playerNo);
    seNo1 = HuAudFXPlay(MSM_SE_BD5_15);
    for(i=0; i<80; i++) {
        Hu3DModelObjMtxGet(pathMdlId, rocketHookTbl[no], hookMtx);
        MTXTransApply(hookMtx, masuMtx, 0, 3, 0);
        MBMasuMtxSet(masuId, masuMtx);
        MBPlayerPosSet(playerNo, hookMtx[0][3], hookMtx[1][3], hookMtx[2][3]);
        for(j=0; j<overlapNum; j++) {
            MBPlayerPosGet(overlapPlayer[j], &overlapPos[j]);
            overlapPos[j].x += hookMtx[0][3]-startPos.x;
            overlapPos[j].y += hookMtx[1][3]-startPos.y;
            overlapPos[j].z += hookMtx[2][3]-startPos.z;
            MBPlayerPosSetV(overlapPlayer[j], &overlapPos[j]);
        }
        startPos.x = hookMtx[0][3];
        startPos.y = hookMtx[1][3];
        startPos.z = hookMtx[2][3];
        HuPrcVSleep();
    }
    HuAudFXStop(seNo1);
    seNo1 = MSM_SENO_NONE;
    HuAudFXPlay(MSM_SE_BD5_16);
    HuPrcSleep(10);
    MBPlayerPosGet(playerNo, &playerPos);
    rocketPos = rocketStartPos[no];
    HuAudFXPlay(MSM_SE_BD5_01);
    HuPrcSleep(30);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_WALK, 0, 8, HU3D_MOTATTR_LOOP);
    for(i=0; i<60.0f; i++) {
        if(MBMotionTimeGet(mb5_Work.rocketMdlId[no]) >= 150) {
            MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 0);
        }
        t = i/60.0f;
        pos.x = playerPos.x+((rocketPos.x-playerPos.x)*t);
        pos.y = playerPos.y+((rocketPos.y-playerPos.y)*t);
        pos.z = playerPos.z+((rocketPos.z-playerPos.z)*t);
        MBPlayerPosSetV(playerNo, &pos);
        HuPrcVSleep();
    }
    MBPlayerRotateStart(playerNo, playerRotTbl[no], 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 1);
    while(1) {
        time = MBMotionTimeGet(mb5_Work.rocketMdlId[no]);
        if(time >= 200) {
            MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 0);
            break;
        }
        if(time == 160) {
            HuAudFXPlay(MSM_SE_BD5_01);
        } else if(time  == 190) {
            HuAudFXPlay(MSM_SE_BD5_13);
        }
        HuPrcVSleep();
    }
    MBModelDispSet(playerMdlId, FALSE);
    MBCameraFocusPlayerSet(-1);
    MBModelAttrSet(mb5_Work.launchBMdlId[no], HU3D_MOTATTR_REV);
    MBMotionSpeedSet(mb5_Work.launchBMdlId[no], 1.3f);
    MBModelAttrSet(mb5_Work.rocketPathMdlId[no], HU3D_MOTATTR_REV);
    MBMotionSpeedSet(mb5_Work.rocketPathMdlId[no], 1.3f);
    while(1) {
        time = MBMotionTimeGet(mb5_Work.launchBMdlId[no]);
        if(time > 0) {
            Hu3DModelObjMtxGet(pathMdlId, rocketHookTbl[no], hookMtx);
            MTXTransApply(hookMtx, masuMtx, 0, -3, 0);
            MBMasuMtxSet(masuId, masuMtx);
            for(j=0; j<overlapNum; j++) {
                MBPlayerPosGet(overlapPlayer[j], &overlapPos[j]);
                overlapPos[j].x += hookMtx[0][3]-startPos.x;
                overlapPos[j].y += hookMtx[1][3]-startPos.y;
                overlapPos[j].z += hookMtx[2][3]-startPos.z;
                MBPlayerPosSetV(overlapPlayer[j], &overlapPos[j]);
            }
            startPos.x = hookMtx[0][3];
            startPos.y = hookMtx[1][3];
            startPos.z = hookMtx[2][3];
            if(time <= 80) {
                if(seNo1 == MSM_SENO_NONE) {
                    seNo1 = HuAudFXPlay(MSM_SE_BD5_15);
                }
            }
        } else {
            HuAudFXStop(seNo1);
            HuAudFXPlay(MSM_SE_BD5_16);
            break;
        }
        HuPrcVSleep();
    }
    HuPrcSleep(40);
    pos.y += 170;
    pos.z += 200;
    while(1) {
        HuAudFXPlay(MSM_SE_BD5_17);
        MBModelDispSet(numberMdlId[countdownNum], TRUE);
        MBModelRotSet(numberMdlId[countdownNum], -33, 0, 0);
        MBModelPosSetV(numberMdlId[countdownNum], &pos);
        MBModelScaleSet(numberMdlId[countdownNum], 1.5f, 1.5f, 1.5f);
        MBModelAlphaSet(numberMdlId[countdownNum], 255);
        for(numTime=0, numScale=1.5f, numAlpha=255; numTime<=60; numTime++) {
            numScale += numScaleSpeed;
            numAlpha -= numAlphaSpeed;
            MBModelScaleSet(numberMdlId[countdownNum], numScale, numScale, numScale);
            MBModelAlphaSet(numberMdlId[countdownNum], numAlpha);
            HuPrcVSleep();
        }
        MBModelDispSet(numberMdlId[countdownNum], FALSE);
        countdownNum--;
        HuPrcSleep(5);
        if(countdownNum <= 0) {
            break;
        }
        HuPrcVSleep();
    }
    MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 1);
    seNo2 = HuAudFXPlay(MSM_SE_BD5_02);
    MBPlayerPosGet(playerNo, &startPos);
    startPos.y -= 400;
    MB5Ev_RocketDustCreate(startPos);
    omVibrate(playerNo, 160, 4, 2);
    HuPrcSleep(180);
    HuAudFXStop(seNo2);
    startTime = MBMotionTimeGet(mb5_Work.rocketMdlId[no]);
    MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 0);
    MBModelDispSet(mb5_Work.rocketMdlId[no], FALSE);
    i = frandmod(4)+1;
    rocketMasuId = MBMasuFlagFind(MASU_LAYER_DEFAULT, MB5_EventMasuFlagGet(i, MB5EV_MASU_ROCKET), MB5EV_MASU_ROCKET);
    MBMasuPosGet(MASU_LAYER_DEFAULT, rocketMasuId, &pos);
    zoom = MBCameraZoomGet();
    MBCameraSkipSet(FALSE);
    MBCameraViewNoSet(GwSystem.turnPlayerNo, MB_CAMERA_VIEW_WALK);
    MBCameraMotionWait();
    MBStarScrollCreate(&playerPos, &pos, 180);
    MBCameraSkipSet(FALSE);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBStarScrollStart();
    while(!MBStarScrollDoneCheck()) {
        HuPrcVSleep();
    }
    for(i=0; i<13; i++) {
        Hu3DParManKill(mb5ev_RocketDustParmanId[i]);
        mb5ev_RocketDustParmanId[i] = HU3D_PARMANID_NONE;
    }
    mb5ev_RocketDustAnim = NULL;
    MBModelPosSet(playerMdlId, pos.x, pos.y+430, pos.z);
    MBCameraRotGet(&cameraRot);
    cameraRot.x += 15;
    MBCameraPosTargetGet(&cameraPos);
    MBCameraPlayerViewSet(playerNo, &cameraRot, NULL, 3400, -1, 150);
    MBModelRotSet(mb5_Work.rocketMdlId[no], 0, -playerRotTbl[no], 0);
    MBModelDispSet(mb5_Work.rocketMdlId[no], TRUE);
    MBModelPosSetV(mb5_Work.rocketMdlId[no], &pos);
    MBModelAttrSet(mb5_Work.rocketMdlId[no], HU3D_MOTATTR_REV);
    MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 1);
    seNo2 = HuAudFXPlay(MSM_SE_BD5_03);
    omVibrate(playerNo, 160, 4, 2);
    while(1) {
        dustTime = MBMotionTimeGet(mb5_Work.rocketMdlId[no]);
        if(dustTime <= 210) {
            break;
        }
        if(270.0f == dustTime) {
            HuAudFXFadeOut(seNo2, 2500);
        }
        if(fabs(startTime-dustTime) >= 60 && -1 != startTime) {
            MB5Ev_RocketDustCreate(pos);
            startTime = -1;
        }
        cameraPos.y += 5;
        MBCameraPosSetV(&cameraPos);
        HuPrcVSleep();
    }
    MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 0);
    MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_W05_ROCKET_END, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBTopWinKill();
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBModelDispSet(mb5_Work.rocketMdlId[no], FALSE);
    MBStarScrollKill();
    MBPlayerRotateStart(playerNo, 0, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    rocketMasuP = MBMasuGet(MASU_LAYER_DEFAULT, rocketMasuId);
    MBMasuPosGet(MASU_LAYER_DEFAULT, rocketMasuP->linkTbl[0], &rocketMasuPos);
    MBPlayerPosSetV(playerNo, &rocketMasuPos);
    GwPlayer[playerNo].masuId = rocketMasuP->linkTbl[0];
    MBPlayerPosFixCurr(rocketMasuP->linkTbl[0], TRUE);
    MBPlayerRotateStart(playerNo, 0, 1);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCameraSkipSet(FALSE);
    MBCameraViewNoSet(GwSystem.turnPlayerNo, MB_CAMERA_VIEW_ZOOMOUT);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBCameraZoomSet(2200);
    MBModelDispSet(playerMdlId, TRUE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    while(WipeCheckEnd()) {
        MBCameraSkipSet(FALSE);
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    MBMasuMtxSet(masuId, NULL);
    MBModelAttrReset(mb5_Work.launchBMdlId[no], HU3D_MOTATTR_REV);
    MBMotionSpeedSet(mb5_Work.launchBMdlId[no], 0);
    MBMotionTimeSet(mb5_Work.launchBMdlId[no], 0);
    MBModelAttrReset(mb5_Work.rocketPathMdlId[no], HU3D_MOTATTR_REV);
    MBMotionSpeedSet(mb5_Work.rocketPathMdlId[no], 0);
    MBMotionTimeSet(mb5_Work.rocketPathMdlId[no], 0);
    MBMotionSpeedSet(mb5_Work.rocketMdlId[no], 0);
    MBMotionTimeSet(mb5_Work.rocketMdlId[no], 0);
    MBModelPosSetV(mb5_Work.rocketMdlId[no], &rocketEndPos[no]);
    MBModelRotSet(mb5_Work.rocketMdlId[no], 0, 0, 0);
    MBModelAttrReset(mb5_Work.rocketMdlId[no], HU3D_MOTATTR_REV);
    MBModelDispSet(mb5_Work.rocketMdlId[no], TRUE);
    for(i=0; i<13; i++) {
        Hu3DParManKill(mb5ev_RocketDustParmanId[i]);
        mb5ev_RocketDustParmanId[i] = HU3D_PARMANID_NONE;
    }
    mb5ev_RocketDustAnim = NULL;
    HuAudFXStop(seNo2);
    seNo2 = MSM_SENO_NONE;
    MBPlayerPosFixFlagSet(TRUE);
}

static HuVecF lbl_1_data_72C[3] = {
    { 30, 45, 0 },
    { 30, 0, 0 },
    { 30, -45, 0 },
};

void MB5Ev_Taxi(int playerNo)
{
    int no; //r30
    s16 i; //r29
    int guideMdlId; //r28
    int seNo1; //r27
    int motId; //r26
    s8 coinDispNo; //r25
    s16 masuId; //r24
    s16 endMasuId; //r23
    int seNo2; //r22
    int taxiMdlId; //r21
    s16 jumpStartMasuId; //r20
    s16 jumpEndMasuId; //r19
    int seNo3; //r18
    MASU *masuP; //r17
    
    float taxiTime; //f31
    float playerXOfs; //f30
    float A2KatTime; //f29
    
    Mtx taxiMtx; //sp+0x7C
    HuVecF startMasuPos; //sp+0x70
    HuVecF jumpMasuPos; //sp+0x64
    HuVecF guidePos; //sp+0x58
    HuVecF taxiStartWalkPos; //sp+0x4C
    HuVecF taxiEndWalkPos; //sp+0x40
    HuVecF taxiRot; //sp+0x34
    HuVecF coinDispPos; //sp+0x28
    HuVecF playerOfs; //sp+0x1C
    HuVecF taxiOfs; //sp+0x10
    u32 flag; //sp+0xC
    float pathTime; //sp+0x8
    
    static HuVecF guideOfsTbl[2] = {
        { -180, 0, 0 },
        { 180, 0, 0 },
    };
    static char *taxiHookTbl[2] = {
        "taxiLhook",
        "taxiRhook"
    };
    static float playerYRotTbl[2] = {
        90,
        -90
    };
    
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    flag = masuP->flag & MASU_FLAG_MAP;
    seNo1 = seNo3 = seNo2 = MSM_SENO_NONE;
    MBPlayerPosFixFlagSet(FALSE);
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &startMasuPos);
    if(startMasuPos.x < 0) {
        no = 0;
    } else {
        no = 1;
    }
    jumpStartMasuId = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, masuId, MB5EV_MASU_TAXI_JUMP);
    MBMasuPosGet(MASU_LAYER_DEFAULT, jumpStartMasuId, &jumpMasuPos);
    guidePos = jumpMasuPos;
    guidePos.y += 100;
    taxiStartWalkPos.x = 0.5f*(jumpMasuPos.x+startMasuPos.x);
    taxiStartWalkPos.y = (0.5f*(jumpMasuPos.y+startMasuPos.y))+170;
    taxiStartWalkPos.z = 0.5f*(jumpMasuPos.z+startMasuPos.z);
    MBPlayerRotateStart(playerNo, 180, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    guideMdlId = MBGuideCreateFlag(MB_GUIDE_TERL, &guidePos, FALSE, TRUE, TRUE, FALSE);
    MBModelLayerSet(guideMdlId, 2);
    HuDataDirClose(DATA_guide);
    if(MBPlayerCoinGet(playerNo) == 0) {
        HuAudFXPlay(MSM_SE_GUIDE_57);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W05_TAXI_COIN_EMPTY, HUWIN_SPEAKER_TERL);
        MBTopWinWait();
        MBTopWinKill();
        MBGuideEnd(guideMdlId);
        return;
    }
    HuAudFXPlay(MSM_SE_GUIDE_56);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W05_TAXI_START, HUWIN_SPEAKER_TERL);
    MBTopWinWait();
    MBTopWinKill();
    for(i=0; i<30.0f; i++) {
        if(no == 0) {
            guidePos.x += 6;
        } else {
            guidePos.x -= 6;
        }
        MBGuideMdlPosSet(guideMdlId, guidePos.x, guidePos.y, guidePos.z);
        HuPrcVSleep();
    }
    HuPrcSleep(10);
    MBPlayerWalkMain(playerNo, NULL, &taxiStartWalkPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
    MBPlayerWalkMain(playerNo, NULL, &jumpMasuPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    HuPrcSleep(20);
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    motId = MBPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_328);
    MBPlayerMotionNoShiftSet(playerNo, motId, 0, 8, HU3D_MOTATTR_NONE);
    playerOfs = mb5ev_PlayerOfsTbl[GwPlayer[playerNo].charNo];
    if(no == 1) {
        playerOfs.x *= -1;
    }
    taxiMdlId = MBModelIdGet(mb5_Work.taxiMdlId[no]);
    Hu3DModelObjMtxGet(taxiMdlId, "taxihook", taxiMtx);
    MBPlayerPosSet(playerNo, taxiMtx[0][3]+playerOfs.x, taxiMtx[1][3]+playerOfs.y, taxiMtx[2][3]+playerOfs.z);
    MBPlayerRotSet(playerNo, 0, playerYRotTbl[no], 0);
    MBPlayerPosGet(playerNo, &taxiOfs);
    if(no == 0) {
        MBModelDispSet(mb5_Work.taxiMdlId[1], FALSE);
    } else {
        MBModelDispSet(mb5_Work.taxiMdlId[0], FALSE);
    }
    for(i=0; i<10; i++) {
        MBMotionTimeSet(mb5_Work.taxiRingMdlId[i], 0);
        MBModelDispSet(mb5_Work.taxiRingMdlId[i], TRUE);
    }
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    WipeWait();
    HuPrcSleep(20);
    MBCameraPlayerViewSet(playerNo, NULL, NULL, 4200, -1, 60);
    MBMotionTimeSet(mb5_Work.A2KatMdlId[no], 0);
    MBMotionSpeedSet(mb5_Work.A2KatMdlId[no], 1);
    MBMotionTimeSet(mb5_Work.taxiPathMdlId[no], 0);
    MBMotionSpeedSet(mb5_Work.taxiPathMdlId[no], 1);
    MBMotionTimeSet(mb5_Work.taxiMdlId[no], 0);
    MBMotionSpeedSet(mb5_Work.taxiMdlId[no], 1);
    seNo1 = HuAudFXPlay(MSM_SE_BD5_04);
    playerXOfs = 0;
    while(1) {
        A2KatTime = MBMotionTimeGet(mb5_Work.A2KatMdlId[no]);
        pathTime = MBMotionTimeGet(mb5_Work.taxiPathMdlId[no]);
        taxiTime = MBMotionTimeGet(mb5_Work.taxiMdlId[no]);
        if(A2KatTime >= 100 && seNo1 != MSM_SENO_NONE) {
            HuAudFXStop(seNo1);
            seNo1 = MSM_SENO_NONE;
            HuAudFXPlay(MSM_SE_BD5_18);
        }
        if(taxiTime >= 145 && seNo2 == MSM_SENO_NONE) {
            seNo3 = HuAudFXPlay(MSM_SE_BD5_05);
            seNo2 = HuAudFXPlay(MSM_SE_BD5_12);
        }
        if(620 == taxiTime) {
            HuAudFXPlay(MSM_SE_BD5_11);
        }
        if(700 == taxiTime) {
            HuAudFXFadeOut(seNo3, 2000);
            HuAudFXFadeOut(seNo2, 2000);
        }
        if(MBMotionEndCheck(mb5_Work.taxiPathMdlId[no])) {
            break;
        }
        Hu3DModelObjMtxGet(taxiMdlId, "taxihook", taxiMtx);
        MBPlayerPosSet(playerNo, (taxiMtx[0][3]+playerOfs.x)+playerXOfs, taxiMtx[1][3]+playerOfs.y, taxiMtx[2][3]+playerOfs.z);
        Hu3DMtxRotGet(taxiMtx, &taxiRot);
        if(no == 1) {
            taxiRot.z -= 180;
        }
        MBPlayerRotSet(playerNo, 0, playerYRotTbl[no], taxiRot.z);
        playerXOfs = taxiMtx[0][3]-taxiOfs.x;
        taxiOfs.x = taxiMtx[0][3];
        taxiOfs.y = taxiMtx[1][3];
        taxiOfs.z = taxiMtx[2][3];
        HuPrcVSleep();
    }
    MBCameraPlayerViewSet(playerNo, NULL, NULL, 3200, -1, 60);
    MBCameraMotionWait();
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBCameraSkipSet(FALSE);
    jumpEndMasuId = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, jumpStartMasuId, MB5EV_MASU_TAXI_JUMP);
    MBMasuPosGet(MASU_LAYER_DEFAULT, jumpEndMasuId, &startMasuPos);
    MBPlayerPosSetV(playerNo, &startMasuPos);
    MBGuideMdlPosSet(guideMdlId, startMasuPos.x+guideOfsTbl[no].x, startMasuPos.y+100, startMasuPos.z);
    MBPlayerRotSet(playerNo, 0, 0, 0);
    MBMotionTimeSet(mb5_Work.taxiMdlId[no], 0);
    MBMotionSpeedSet(mb5_Work.taxiMdlId[no], 0);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 0.1f, HU3D_MOTATTR_LOOP);
    for(i=0; i<10; i++) {
        MBMotionTimeSet(mb5_Work.taxiRingMdlId[i], 0);
        MBModelDispSet(mb5_Work.taxiRingMdlId[i], FALSE);
    }
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    WipeWait();
    MBCameraSkipSet(TRUE);
    MBPlayerRotateStart(playerNo, 0, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W05_TAXI_COIN, HUWIN_SPEAKER_TERL);
    MBTopWinWait();
    MBTopWinKill();
    if(MBPlayerCoinGet(playerNo) < 10) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W05_TAXI_COIN_LOW, HUWIN_SPEAKER_TERL);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBPlayerPosGet(playerNo, &coinDispPos);
    coinDispPos.y += 250;
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINLOSE, 0, 8, HU3D_MOTATTR_NONE);
    MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
    if(MBPlayerCoinGet(playerNo) >= 10) {
        coinDispNo = MBCoinDispCreateSe(&coinDispPos, -10);
        MBCoinAddExec(playerNo, -10);
    } else {
        coinDispNo = MBCoinDispCreateSe(&coinDispPos, -MBPlayerCoinGet(playerNo));
        MBCoinAddExec(playerNo, -MBPlayerCoinGet(playerNo));
    }
    while(!MBPlayerMotionEndCheck(playerNo) || MBMotionShiftIDGet(MBPlayerModelGet(playerNo)) != HU3D_MOTID_NONE) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    if(coinDispNo != -1) {
        while(!MBCoinDispKillCheck(coinDispNo)) {
            HuPrcVSleep();
        }
    }
    MBGuideEnd(guideMdlId);
    endMasuId = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, jumpEndMasuId, MB5EV_MASU_TAXI);
    MBMasuPosGet(MASU_LAYER_DEFAULT, endMasuId, &jumpMasuPos);
    taxiEndWalkPos.x = 0.5f*(jumpMasuPos.x+startMasuPos.x);
    taxiEndWalkPos.y = (0.5f*(jumpMasuPos.y+startMasuPos.y))+170;
    taxiEndWalkPos.z = 0.5f*(jumpMasuPos.z+startMasuPos.z);
    MBPlayerPosFixCurr(endMasuId, 0);
    MBPlayerWalkMain(playerNo, NULL, &taxiEndWalkPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
    MBPlayerWalkMain(playerNo, NULL, &jumpMasuPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
    GwPlayer[playerNo].masuId = endMasuId;
    while(1) {
        if(MBPlayerMotionEndCheck(playerNo)) {
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
            break;
        }
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    if(motId != -1) {
        MBPlayerMotionKill(playerNo, motId);
    }
    mb5ev_TaxiResetF = TRUE;
    mb5ev_TaxiPlayerPrev = playerNo;
    MBPlayerPosFixFlagSet(TRUE);
}

void MB5Ev_RocketJetCoinInit(void);
void MB5Ev_RocketJetCoinCreate(HuVecF *startPos, HuVecF *endPos);
void MB5Ev_RocketJetCoinClose(void);
void MB5Ev_RocketJetCoinUpdate(int playerNo);
int MB5Ev_RocketJetCoinNumGet(void);
HuVecF MB5Ev_RocketJetCoinPosGet(s16 no);

void MB5Ev_RocketJet(int playerNo)
{
    int seNo1; //r30
    int seNo2; //r29
    MGTIMER *timer; //r28
    s16 jetNo; //r27
    int seNo3; //r26
    int rocketJetMdlId; //r25
    int motId; //r24
    int coinDelay; //r23
    int guideMdlId; //r22
    int coinDispNo; //r21
    int armHookMdlId; //r20
    s16 masuId; //r19
    
    float baseTime; //f31
    float jetDecel; //f30
    float maxTime; //f29
    
    Mtx rocketJetMtx; //sp+0xA0
    Mtx armMtx; //sp+0x70
    HuVecF playerPos; //sp+0x64
    HuVecF guidePos; //sp+0x58
    HuVecF focusPos; //sp+0x4C
    HuVecF rocketJetRot; //sp+0x40
    HuVecF coinPos; //sp+0x34
    HuVecF coinDispPos; //sp+0x28
    HuVecF comCoinPos; //sp+0x1C
    HuVecF playerOfs; //sp+0x10
    float temp; //sp+0xC
    MASU *masuP; //sp+0x8
    static s16 orderTbl[4] = { 0, 2, 3, 1 };
    
    seNo1 = MSM_SENO_NONE;
    seNo2 = MSM_SENO_NONE;
    seNo3 = MSM_SENO_NONE;
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    playerOfs = mb5ev_PlayerOfsTbl[GwPlayer[playerNo].charNo];
    mb5ev_RocketJetCancelF = FALSE;
    MBPlayerPosFixFlagSet(FALSE);
    MBPlayerRotateStart(playerNo, 180, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBPlayerPosGet(playerNo, &playerPos);
    guidePos.x = playerPos.x-150;
    guidePos.y = playerPos.y+200;
    guidePos.z = playerPos.z-200;
    guideMdlId = MBGuideCreateFlag(MB_GUIDE_TERL, &guidePos, FALSE, TRUE, TRUE, FALSE);
    MBModelLayerSet(guideMdlId, 2);
    HuDataDirClose(DATA_guide);
    HuAudFXPlay(MSM_SE_GUIDE_56);
    MBWinCreateChoice(MBWIN_TYPE_RESULT, MESS_BOARD_W05_RJET_CHOICE, HUWIN_SPEAKER_TERL, 0);
    if(GwPlayer[playerNo].comF) {
        MBComChoiceSetUp();
    }
    MBTopWinWait();
    if(MBWinLastChoiceGet() == 0) {
        mb5ev_RocketJetCancelF = TRUE;
    }
    MBTopWinKill();
    if(mb5ev_RocketJetCancelF) {
        temp = 1;
        MB5Ev_RocketJetCoinInit();
        coinDelay = frandmod(60)+30;
        mb5ev_RocketJetCoinTotal = 0;
        MBCameraFocusPlayerSet(-1);
        MBPlayerPosGet(playerNo, &focusPos);
        focusPos.y += 400;
        MBCameraPosViewSet(&focusPos, NULL, NULL, 4000, -1, 60);
        MBCameraMotionWait();
        while(1) {
            baseTime = MBMotionTimeGet(mb5_Work.rjetBaseMdlId);
            if(baseTime >= 200.0f && baseTime < 300.0f) {
                jetNo = 1;
                maxTime = 300;
                jetDecel = 1/fabs(maxTime-baseTime);
                break;
            } else if(baseTime >= 500) {
                jetNo = 0;
                maxTime = 600;
                jetDecel = 1/fabs(maxTime-baseTime);
                break;
            }
            HuPrcVSleep();
        }
        MBModelAttrReset(mb5_Work.rocketJetMdlId[0], HU3D_MOTATTR_LOOP);
        MBMotionStartEndSet(mb5_Work.rocketJetMdlId[0], 81, 160);
        MBModelAttrReset(mb5_Work.rocketJetMdlId[2], HU3D_MOTATTR_LOOP);
        MBMotionStartEndSet(mb5_Work.rocketJetMdlId[2], 81, 160);
        while(1) {
            mb5ev_RocketJetSpeed -= 0.5f*jetDecel;
            if(mb5ev_RocketJetSpeed <= 0) {
                break;
            }
            HuPrcVSleep();
        }
        mb5ev_RocketJetSpeed = 0;
        MBMotionTimeSet(mb5_Work.rjetBaseMdlId, 300.0f*jetNo);
        while(1) {
            if(MBMotionEndCheck(mb5_Work.rocketJetMdlId[0]) && MBMotionEndCheck(mb5_Work.rocketJetMdlId[2])) {
                break;
            }
            HuPrcVSleep();
        }
        MBMotionSpeedSet(mb5_Work.rocketJetMdlId[0], 0);
        MBMotionSpeedSet(mb5_Work.rocketJetMdlId[2], 0);
        seNo3 = HuAudFXPlay(MSM_SE_BD5_19);
        while(1) {
            if(mb5ev_RocketJetYRot[orderTbl[jetNo]] >= 30) {
                break;
            }
            mb5ev_RocketJetYRot[orderTbl[jetNo]] += 0.4f;
            HuPrcVSleep();
        }
        HuAudFXStop(seNo3);
        HuPrcSleep(30);
        MBMotionTimeSet(mb5_Work.rjetBaseMdlId, 0);
        HuPrcVSleep();
        WipeColorSet(0, 0, 0);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBCameraSkipSet(FALSE);
        MBMotionTimeSet(mb5_Work.rocketJetMdlId[0], 160);
        MBMotionTimeSet(mb5_Work.rocketJetMdlId[2], 160);
        mb5ev_RocketJetYRotVel[0] = mb5ev_RocketJetYRotVel[1] = -0.2f;
        mb5ev_RocketJetYRotVel[2] = mb5ev_RocketJetYRotVel[3] = 0.2f;
        rocketJetMdlId = MBModelIdGet(mb5_Work.rocketJetMdlId[0]);
        Hu3DModelObjMtxGet(rocketJetMdlId, "Rjethook", rocketJetMtx);
        Hu3DMtxRotGet(rocketJetMtx, &rocketJetRot);
        MBPlayerPosSet(playerNo, rocketJetMtx[0][3]+playerOfs.x, rocketJetMtx[1][3]+playerOfs.y, rocketJetMtx[2][3]+playerOfs.z);
        MBPlayerRotSet(playerNo, rocketJetRot.x, rocketJetRot.y-90, rocketJetRot.z);
        motId = MBPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_328);
        MBPlayerMotionNoShiftSet(playerNo, motId, 0, 8, HU3D_MOTATTR_NONE);
        MBCameraFocusPlayerSet(playerNo);
        MBCameraPlayerViewSet(playerNo, NULL, NULL, 3000, -1, 6);
        MBCameraMotionWait();
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        WipeWait();
        MBCameraSkipSet(TRUE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W05_RJET_INST, HUWIN_SPEAKER_TERL);
        MBTopWinWait();
        MBTopWinKill();
        MBGuideEnd(guideMdlId);
        HuPrcSleep(20);
        timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
        MgTimerParamSet(timer, 30*60, 0, 0);
        MgTimerModeOnSet(timer, MGTIMER_OFFTYPE_FADEOUT);
        mb5ev_RocketJetStopF = TRUE;
        HuPrcSleep(60);
        mb5ev_RocketJetSpeed = 1;
        MBModelAttrSet(mb5_Work.rocketJetMdlId[0], HU3D_MOTATTR_REV);
        MBModelAttrSet(mb5_Work.rocketJetMdlId[2], HU3D_MOTATTR_REV);
        MBMotionSpeedSet(mb5_Work.rocketJetMdlId[0], 1);
        MBMotionSpeedSet(mb5_Work.rocketJetMdlId[2], 1);
        MBWinCreateHelp(MESS_BOARD_W05_RJET_HELP);
        seNo3 = HuAudFXPlay(MSM_SE_BD5_19);
        while(1) {
            if(MgTimerDoneCheck(timer)) {
                break;
            }
            if(MBMotionTimeGet(mb5_Work.rocketJetMdlId[0]) <= 80.0f) {
                MBMotionStartEndSet(mb5_Work.rocketJetMdlId[0], 0, 80);
                MBModelAttrSet(mb5_Work.rocketJetMdlId[0], HU3D_MOTATTR_LOOP);
                MBModelAttrReset(mb5_Work.rocketJetMdlId[0], HU3D_MOTATTR_REV);
                MBMotionStartEndSet(mb5_Work.rocketJetMdlId[2], 0, 80);
                MBModelAttrSet(mb5_Work.rocketJetMdlId[2], HU3D_MOTATTR_LOOP);
                MBModelAttrReset(mb5_Work.rocketJetMdlId[2], HU3D_MOTATTR_REV);
            }
            baseTime = MBMotionTimeGet(mb5_Work.rjetBaseMdlId);
            if(timer->time >= 180) {
                if(baseTime <= 100 || baseTime >= 200) {
                    armHookMdlId = MBModelIdGet(mb5_Work.rjetArmHookMdlId[3]);
                    Hu3DModelObjMtxGet(armHookMdlId, "arm4hook", armMtx);
                    if(--coinDelay <= 0) {
                        coinPos.x = armMtx[0][3];
                        coinPos.y = armMtx[1][3]+140;
                        coinPos.z = armMtx[2][3];
                        MB5Ev_RocketJetCoinCreate(&coinPos, &coinPos);
                        coinDelay = frandmod(60)+40;
                    }
                }
            }
            MB5Ev_RocketJetCoinUpdate(playerNo);
            Hu3DModelObjMtxGet(rocketJetMdlId, "Rjethook", rocketJetMtx);
            Hu3DMtxRotGet(rocketJetMtx, &rocketJetRot);
            MBPlayerPosSet(playerNo, rocketJetMtx[0][3]+playerOfs.x, rocketJetMtx[1][3]+playerOfs.y, rocketJetMtx[2][3]+playerOfs.z);
            if(rocketJetRot.z >= 0 && rocketJetRot.z <= 0.1f) {
                MBPlayerRotSet(playerNo, rocketJetRot.z, rocketJetRot.y-90, rocketJetRot.z);
            } else {
                MBPlayerRotSet(playerNo, rocketJetRot.z, rocketJetRot.y+90, rocketJetRot.z);
            }
            if(GwPlayer[playerNo].comF) {
                //TODO: Sign extension of constant
                comCoinPos = MB5Ev_RocketJetCoinPosGet(0);
                if(rocketJetMtx[1][3] <= mb5ev_RocketJetCoin[0].pos.y) {
                    if(seNo1 == MSM_SENO_NONE) {
                        seNo1 = HuAudFXPlay(MSM_SE_BD5_06);
                        HuAudFXStop(seNo2);
                        seNo2 = MSM_SENO_NONE;
                    }
                    mb5ev_RocketJetYRot[0] += mb5ev_RocketJetYRotVel[0];
                } else {
                    if(seNo2 == MSM_SENO_NONE) {
                        seNo2 = HuAudFXPlay(MSM_SE_BD5_06);
                        HuAudFXStop(seNo1);
                        seNo1 = MSM_SENO_NONE;
                    }
                    mb5ev_RocketJetYRot[0] -= mb5ev_RocketJetYRotVel[0];
                }
            } else {
                if(HuPadBtn[GwPlayer[playerNo].padNo] & PAD_BUTTON_A) {
                    if(seNo1 == MSM_SENO_NONE) {
                        seNo1 = HuAudFXPlay(MSM_SE_BD5_06);
                        HuAudFXStop(seNo2);
                        seNo2 = MSM_SENO_NONE;
                    }
                    mb5ev_RocketJetYRot[0] += mb5ev_RocketJetYRotVel[0];
                } else {
                    if(seNo2 == MSM_SENO_NONE) {
                        seNo2 = HuAudFXPlay(MSM_SE_BD5_06);
                        HuAudFXStop(seNo1);
                        seNo1 = MSM_SENO_NONE;
                    }
                    mb5ev_RocketJetYRot[0] -= mb5ev_RocketJetYRotVel[0];
                }
            }
            if(mb5ev_RocketJetYRot[0] <= -30.0f) {
                mb5ev_RocketJetYRot[0] = -30.0f;
            } else if(mb5ev_RocketJetYRot[0] >= 30.0f) {
                mb5ev_RocketJetYRot[0] = 30.0f;
            }
            HuPrcVSleep();
        }
        baseTime = MBMotionTimeGet(mb5_Work.rjetBaseMdlId);
        jetDecel = 1/fabs(600.0f-baseTime);
        MBModelAttrReset(mb5_Work.rocketJetMdlId[0], HU3D_MOTATTR_LOOP);
        MBMotionStartEndSet(mb5_Work.rocketJetMdlId[0], 81, 160);
        MBModelAttrReset(mb5_Work.rocketJetMdlId[2], HU3D_MOTATTR_LOOP);
        MBMotionStartEndSet(mb5_Work.rocketJetMdlId[2], 81, 160);
        while(1) {
            mb5ev_RocketJetSpeed -= jetDecel*0.5f;
            Hu3DModelObjMtxGet(rocketJetMdlId, "Rjethook", rocketJetMtx);
            Hu3DMtxRotGet(rocketJetMtx, &rocketJetRot);
            MBPlayerPosSet(playerNo, rocketJetMtx[0][3]+playerOfs.x, rocketJetMtx[1][3]+playerOfs.y, rocketJetMtx[2][3]+playerOfs.z);
            MBPlayerRotSet(playerNo, rocketJetRot.z, rocketJetRot.y-90, rocketJetRot.z);
            MB5Ev_RocketJetCoinUpdate(playerNo);
            if(mb5ev_RocketJetSpeed <= 0) {
                mb5ev_RocketJetSpeed = 0;
            }
            if(mb5ev_RocketJetSpeed <= 0 && MB5Ev_RocketJetCoinNumGet() == 0) {
                break;
            }
            HuPrcVSleep();
        }
        mb5ev_RocketJetSpeed = 0;
        MBMotionTimeSet(mb5_Work.rjetBaseMdlId, 0);
        HuPrcSleep(20);
        HuAudFXStop(seNo3);
        MgTimerKill(timer);
        MBTopWinKill();
        WipeColorSet(0, 0, 0);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBCameraSkipSet(FALSE);
        MBPlayerPosSetV(playerNo, &playerPos);
        MBPlayerRotateStart(playerNo, 0, 15);
        MBCameraPlayerViewSet(playerNo, NULL, NULL, 3200, -1, 6);
        MBCameraMotionWait();
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        WipeWait();
        MBCameraSkipSet(TRUE);
        mb5ev_RocketJetStopF = FALSE;
        if(mb5ev_RocketJetCoinTotal >= 1) {
            MBPlayerPosGet(playerNo, &coinDispPos);
            coinDispPos.y += 250;
            coinDispNo = MBCoinDispCreateSe(&coinDispPos, mb5ev_RocketJetCoinTotal);
            MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINWIN, 0, 8, HU3D_MOTATTR_NONE);
            
        } else {
            MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINLOSE, 0, 8, HU3D_MOTATTR_NONE);
        }
        while(!MBPlayerMotionEndCheck(playerNo) || MBMotionShiftIDGet(MBPlayerModelGet(playerNo)) != HU3D_MOTID_NONE) {
            HuPrcVSleep();
        }
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_NONE);
        if(coinDispNo != -1) {
            while(!MBCoinDispKillCheck((s16)coinDispNo)) {
                HuPrcVSleep();
            }
        }
        if(motId != -1) {
            MBPlayerMotionKill(playerNo, motId);
        }
        MB5Ev_RocketJetCoinClose();
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W05_RJET_END, HUWIN_SPEAKER_TERL);
        MBTopWinWait();
        MBTopWinKill();
        MBPlayerRotateStart(playerNo, 0, 15);
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBGuideEnd(guideMdlId);
    }
    mb5ev_RocketJetResetF = TRUE;
    mb5ev_RocketJetPlayerPrev = playerNo;
    MBPlayerPosFixFlagSet(TRUE);
}

void MB5Ev_ReelCapsuleAdd(int playerNo, int capsuleNo);

void MB5Ev_Reel(int playerNo)
{
    int i; //r31
    int col; //r29
    int row; //r28
    s16 masuId; //r27
    u32 flag; //r26
    MASU *masuP; //r25
    int stopNum; //r24
    MGTIMER *timer; //r23
    BOOL comStopF; //r22
    int seNo; //r21
    int buttonMdlId; //r20
    
    float comDelay; //f31
    float stopTime; //f30
    float zoom; //f29
    
    char insertMes[256]; //sp+0xF8
    Mtx buttonMtx; //sp+0xC8
    float reelTime[3]; //sp+0xBC
    float reelTargetSpeed[3]; //sp+0xB0
    float reelSpeed[3]; //sp+0xA4
    float reelAccel[3]; //sp+0x98
    HuVecF masuPos; //sp+0x8C
    HuVecF jumpPos; //sp+0x80
    HuVecF endViewPos; //sp+0x74
    HuVecF endViewRot; //sp+0x68
    HuVecF playerPos; //sp+0x5C
    HuVecF startViewPos; //sp+0x50
    HuVecF startViewRot; //sp+0x44
    int reelTick[3]; //sp+0x38
    int reelPos[3]; //sp+0x2C
    int capsuleNo[3]; //sp+0x20
    BOOL reelStop[3]; //sp+0x14
    BOOL reelStopDone[3]; //sp+0x8
    
    static HuVecF btnPos = { 1621, 3050, -2779 };
    static s16 capsuleTbl[3][8] = {
        {
            CAPSULE_BOMHEI,
            CAPSULE_BOMHEI,
            CAPSULE_CHANCE,
            CAPSULE_WANWAN,
            CAPSULE_BANK,
            CAPSULE_DUEL,
            CAPSULE_TUMUJIKUN,
            CAPSULE_BANK
        },
        {
            CAPSULE_BANK,
            CAPSULE_TUMUJIKUN,
            CAPSULE_WANWAN,
            CAPSULE_DUEL,
            CAPSULE_CHANCE,
            CAPSULE_BOMHEI,
            CAPSULE_BOMHEI,
            CAPSULE_WANWAN
        },
        {
            CAPSULE_TUMUJIKUN,
            CAPSULE_BOMHEI,
            CAPSULE_WANWAN,
            CAPSULE_BOMHEI,
            CAPSULE_DUEL,
            CAPSULE_BANK,
            CAPSULE_CHANCE,
            CAPSULE_BANK
        },
    };
    static float speedTbl[3] = { 1.0f, 1.2f, 1.4f };
    
    seNo = MSM_SENO_NONE;
    stopNum = 0;
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    flag = masuP->flag & MASU_FLAG_MAP;
    masuPos = masuP->pos;
    zoom = MBCameraZoomGet();
    MBModelPosGet(MBPlayerModelGet(playerNo), &playerPos);
    MBPlayerPosFixFlagSet(FALSE);
    for(i=0; i<3; i++) {
        reelStopDone[i] = FALSE;
        reelStop[i] = FALSE;
        reelPos[i] = -1;
        reelSpeed[i] = 1;
    }
    jumpPos.x = 0.5f*(playerPos.x+btnPos.x);
    jumpPos.y = 3200;
    jumpPos.z = 0.5f*(playerPos.z+btnPos.z);
    MBPlayerWalkMain(playerNo, NULL, &jumpPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
    MBPlayerWalkMain(playerNo, NULL, &btnPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    buttonMdlId = MBModelIdGet(mb5_Work.roboButtonMdlId);
    MBMotionTimeSet(mb5_Work.roboButtonMdlId, 0);
    MBMotionSpeedSet(mb5_Work.roboButtonMdlId, 1);
    HuAudFXPlay(MSM_SE_BD5_20);
    while(1) {
        if(MBMotionEndCheck(mb5_Work.roboButtonMdlId)) {
            break;
        }
        Hu3DModelObjMtxGet(buttonMdlId, "R_btnhook", buttonMtx);
        MBPlayerPosSet(playerNo, buttonMtx[0][3], buttonMtx[1][3], buttonMtx[2][3]);
        HuPrcVSleep();
    }
    HuPrcSleep(20);
    MBPlayerRotateStart(playerNo, -135, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    if(MBPlayerCapsuleNumGet(playerNo) >= MBPlayerCapsuleMaxGet()) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_W05_REEL_CAP_FULL, HUWIN_SPEAKER_NONE);
        sprintf(insertMes, "%d", MBPlayerCapsuleMaxGet());
        MBTopWinInsertMesSet(MESSNUM_PTR(insertMes), 0);
        MBTopWinWait();
        MBTopWinKill();
        MBPlayerWalkMain(playerNo, NULL, &jumpPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
        MBPlayerWalkMain(playerNo, NULL, &masuPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        HuAudFXPlay(MSM_SE_BD5_14);
        if(flag & MB5EV_MASU_REEL_L) {
            mb5ev_ReelLMasuId = masuId;
            mb5ev_RobLHookF = TRUE;
            MBPlayerPosGet(playerNo, &mb5ev_RobLArmPos);
            MBMotionSpeedSet(mb5_Work.roboArmLMdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmLMdlId, 0);
            MBMotionSpeedSet(mb5_Work.roboArmL4MdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmL4MdlId, 0);
        } else if(flag & MB5EV_MASU_REEL_R) {
            mb5ev_ReelRMasuId = masuId;
            mb5ev_RobRHookF = TRUE;
            MBPlayerPosGet(playerNo, &mb5ev_RobRArmPos);
            MBMotionSpeedSet(mb5_Work.roboArmRMdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmRMdlId, 0);
            MBMotionSpeedSet(mb5_Work.roboArmR4MdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmR4MdlId, 0);
        }
        MBPlayerRotateStart(playerNo, 0, 15);
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBModelAttrSet(mb5_Work.roboButtonMdlId, HU3D_MOTATTR_REV);
        MBMotionSpeedSet(mb5_Work.roboButtonMdlId, 1);
        while(1) {
            if(MBMotionTimeGet(mb5_Work.roboButtonMdlId) <= 0) {
                break;
            }
            HuPrcVSleep();
        }
        MBMotionTimeSet(mb5_Work.roboButtonMdlId, 0);
        MBMotionSpeedSet(mb5_Work.roboButtonMdlId, 0);
        MBModelAttrReset(mb5_Work.roboButtonMdlId, HU3D_MOTATTR_REV);
    } else {
        
        MBCameraPosTargetGet(&startViewPos);
        MBCameraRotGet(&startViewRot);
        startViewPos.y -= 30;
        startViewRot.x += 20;
        startViewRot.y += 40;
        endViewPos.x = 1577;
        endViewPos.y = 3185;
        endViewPos.z  = -2783;
        endViewRot.x = -8;
        endViewRot.y = 40;
        endViewRot.z = 0;
        MBCameraFocusPlayerSet(-1);
        MBCameraPosViewSet(&endViewPos, &endViewRot, NULL, 1500, -1, 30);
        MBCameraMotionWait();
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_W05_REEL_START, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_W05_REEL_INST, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_W05_REEL_INST_BTN, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
        MgTimerParamSet(timer, 10*60, 0, 0);
        MgTimerModeOnSet(timer, MGTIMER_OFFTYPE_FADEOUT);
        for(i=0; i<3; i++) {
            MBMotionNoSet(mb5_Work.reelMdlId[i], MB_MOT_DEFAULT, HU3D_MOTATTR_LOOP);
            MBMotionSpeedSet(mb5_Work.reelMdlId[i], speedTbl[i]);
            MBMotionTimeSet(mb5_Work.reelMdlId[i], mb5ev_ReelTime[i]);
            MBMotionSpeedSet(mb5_Work.roboLampMdlId[i], 1);
            MBMotionTimeSet(mb5_Work.roboLampMdlId[i], 0);
        }
        HuPrcVSleep();
        seNo = HuAudFXPlay(MSM_SE_BD5_07);
        MBWinCreateHelp(MESS_BOARD_W05_REEL_HELP);
        stopNum = 0;
        comDelay = 60.0f+(frandmod(60)-30.0f);
        col = 0;
        while(1) {
            stopTime = MBMotionTimeGet(mb5_Work.reelMdlId[2]);
            row = stopTime/20;
            if(GwPlayer[playerNo].comF) {
                if(GwPlayerConf[playerNo].dif <= GW_DIF_NORMAL) {
                    comDelay--;
                    if(!comDelay) {
                        HuPadBtnDown[GwPlayer[playerNo].padNo] |= PAD_BUTTON_A;
                        comDelay = 60.0f+(frandmod(60)-30.0f);
                    }
                } else if(reelStop[0] == FALSE) {
                    comDelay--;
                    if(comDelay <= 0) {
                        HuPadBtnDown[GwPlayer[playerNo].padNo] |= PAD_BUTTON_A;
                        comDelay = 60.0f+(frandmod(60)-30.0f);
                    }
                } else {
                    comDelay--;
                    if(comDelay <= 0 && col < 3) {
                        stopTime = MBMotionTimeGet(mb5_Work.reelMdlId[col]);
                        if(col == 2) {
                            if(comStopF) {
                                row = 1+(stopTime*0.05f);
                            } else {
                                row = stopTime*0.05f;
                            }
                        } else {
                            row = 1+(stopTime*0.05f);
                        }
                        if(row >= 8) {
                            row -= 8;
                        }
                        if(capsuleTbl[col-1][reelPos[col-1]] == capsuleTbl[col][row]) {
                            HuPadBtnDown[GwPlayer[playerNo].padNo] |= PAD_BUTTON_A;
                            comDelay = 60.0f+(frandmod(60)-30.0f);
                        }
                    }
                }
            }
            if((HuPadBtnDown[GwPlayer[playerNo].padNo] & PAD_BUTTON_A) || MgTimerDoneCheck(timer)) {
                comStopF = FALSE;
                for(i=0; i<3; i++) {
                    if(!reelStop[i]) {
                        reelTime[i] = MBMotionTimeGet(mb5_Work.reelMdlId[i]);
                        reelTick[i] = 0.05f*reelTime[i];
                        reelTargetSpeed[i] = reelTime[i]-(reelTick[i]*20);
                        reelAccel[i] = 1/fabs(20.0f-reelTargetSpeed[i]);
                        MBModelAttrSet(mb5_Work.roboLampMdlId[i], HU3D_MOTATTR_REV);
                        MBMotionSpeedSet(mb5_Work.roboLampMdlId[i], 1);
                        reelStop[i] = TRUE;
                        break;
                    }
                }
            }
            if(stopNum >= 3) {
                HuAudFXStop(seNo);
                MgTimerKill(timer);
                MBTopWinKill();
                break;
            }
            for(i=0; i<3; i++) {
                if(!reelStopDone[i] && reelStop[i]) {
                    reelSpeed[i] -= 0.5f*reelAccel[i];
                    if(reelSpeed[i] <= 0.0f) {
                        HuAudFXPlay(MSM_SE_BD5_08);
                        reelTime[i] = MBMotionTimeGet(mb5_Work.reelMdlId[i]);
                        reelTargetSpeed[i] = reelTime[i]-(reelTick[i]*20);
                        if(reelTargetSpeed[i] >= 0.0f) {
                            reelTime[i] += 20.0f-reelTargetSpeed[i];
                        }
                        MBMotionSpeedSet(mb5_Work.reelMdlId[i], 0);
                        MBMotionTimeSet(mb5_Work.reelMdlId[i], reelTime[i]);
                        mb5ev_ReelTime[i] = reelTime[i];
                        reelPos[i] = 0.05f*reelTime[i];
                        if(reelPos[i] >= 8) {
                            reelPos[i] -= 8;
                        }
                        reelStopDone[i] = TRUE;
                        stopNum++;
                        col++;
                        if(GwPlayerConf[playerNo].dif == GW_DIF_HARD && frandmod(5) == 0) {
                            comStopF = TRUE;
                        }
                        if(GwPlayerConf[playerNo].dif == GW_DIF_VERYHARD && frandmod(3) == 0) {
                            comStopF = TRUE;
                        }
                    } else {
                        MBMotionSpeedSet(mb5_Work.reelMdlId[i], reelSpeed[i]);
                    }
                }
            }
            HuPrcVSleep();
        }
        for(i=0; i<3; i++){ 
            capsuleNo[i] = capsuleTbl[i][reelPos[i]];
        }
        if(capsuleNo[0] == capsuleNo[1] && capsuleNo[1] == capsuleNo[2]) {
            MBCameraPosViewSet(&startViewPos, &startViewRot, NULL, 1800, -1, 30);
            for(i=0; i<2; i++) {
                MBMotionSpeedSet(mb5_Work.RLampMdlId[i], 3);
                MBMotionTimeSet(mb5_Work.RLampMdlId[i], 0);
                MBModelAttrSet(mb5_Work.RLampMdlId[i], HU3D_MOTATTR_LOOP);
            }
            MB5Ev_ReelCapsuleAdd(playerNo, capsuleTbl[0][reelPos[0]]);
        } else {
            MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_W05_REEL_KINOKO, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTopWinKill();
            MBCameraPosViewSet(&startViewPos, &startViewRot, NULL, 1800, -1, 30);
            MB5Ev_ReelCapsuleAdd(playerNo, CAPSULE_KINOKO);
        }
        MBCameraPosTargetGet(&endViewPos);
        MBCameraRotGet(&endViewRot);
        endViewPos.y += 30;
        endViewRot.x -= 20;
        endViewRot.y -= 40;
        MBCameraPosViewSet(&endViewPos, &endViewRot, NULL, zoom, -1, 30);
        MBCameraMotionWait();
        MBCameraFocusPlayerSet(playerNo);
        MBPlayerWalkMain(playerNo, NULL, &jumpPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
        MBPlayerWalkMain(playerNo, NULL, &masuPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 10, NULL, TRUE);
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        HuAudFXPlay(MSM_SE_BD5_14);
        if(flag & MB5EV_MASU_REEL_L) {
            mb5ev_ReelLMasuId = masuId;
            mb5ev_RobLHookF = TRUE;
            MBPlayerPosGet(playerNo, &mb5ev_RobLArmPos);
            MBMotionSpeedSet(mb5_Work.roboArmLMdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmLMdlId, 0);
            MBMotionSpeedSet(mb5_Work.roboArmL4MdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmL4MdlId, 0);
        } else if(flag & MB5EV_MASU_REEL_R) {
            mb5ev_ReelRMasuId = masuId;
            mb5ev_RobRHookF = TRUE;
            MBPlayerPosGet(playerNo, &mb5ev_RobRArmPos);
            MBMotionSpeedSet(mb5_Work.roboArmRMdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmRMdlId, 0);
            MBMotionSpeedSet(mb5_Work.roboArmR4MdlId, 1);
            MBMotionTimeSet(mb5_Work.roboArmR4MdlId, 0);
        }
        MBPlayerRotateStart(playerNo, 0, 15);
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        for(i=0; i<3; i++) {
            MBModelAttrReset(mb5_Work.roboLampMdlId[i], HU3D_MOTATTR_REV);
            MBMotionSpeedSet(mb5_Work.roboLampMdlId[i], 0);
            MBMotionTimeSet(mb5_Work.roboLampMdlId[i], 0);
        }
        for(i=0; i<2; i++) {
            MBMotionSpeedSet(mb5_Work.RLampMdlId[i], 0);
            MBMotionTimeSet(mb5_Work.RLampMdlId[i], 0);
            MBModelAttrReset(mb5_Work.RLampMdlId[i], HU3D_MOTATTR_LOOP);
        }
        MBModelAttrSet(mb5_Work.roboButtonMdlId, HU3D_MOTATTR_REV);
        MBMotionSpeedSet(mb5_Work.roboButtonMdlId, 1);
        while(1) {
            if(MBMotionTimeGet(mb5_Work.roboButtonMdlId) <= 0) {
                break;
            }
            HuPrcVSleep();
        }
        MBMotionTimeSet(mb5_Work.roboButtonMdlId, 0);
        MBMotionSpeedSet(mb5_Work.roboButtonMdlId, 0);
        MBModelAttrReset(mb5_Work.roboButtonMdlId, HU3D_MOTATTR_REV);
        HuPrcSleep(20);
        MBPlayerPosFixFlagSet(TRUE);
    }
}

void MB5Ev_MoveFan(int playerNo)
{
    u32 no; //r30
    MASU *endMasuP; //r29
    s16 endMasuId; //r28
    int seNo; //r27
    s16 masuId; //r26
    MASU *masuP; //r25
    u32 flag; //r24
    int fanMdlId; //r23
    
    float laserTime; //f31
    float rotYSpeed; //f30
    
    Mtx fanMtx; //sp+0x38
    HuVecF playerPos; //sp+0x2C
    HuVecF jumpPos; //sp+0x20
    HuVecF fanPos; //sp+0x14
    HuVecF rot; //sp+0x8
    static char *moveFanHookTbl[2] = {
        "mfLhook",
        "mfRhook"
    };
    
    
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    flag = masuP->flag & MASU_FLAG_MAP;
    endMasuId = MBMasuTypeLinkGet(MASU_LAYER_DEFAULT, masuId, MASU_TYPE_NONE);
    no = (flag & MB5EV_MASU_MOVEFAN) >> 8;
    if(no == 1) {
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        MBModelAttrSet(mb5_Work.moveFanMdlId[no-1], HU3D_MOTATTR_REV);
        MBMotionSpeedSet(mb5_Work.moveFanMdlId[no-1], 1);
        MBMotionSpeedSet(mb5_Work.autoFanMdlId[no-1], 1);
        seNo = HuAudFXPlay(MSM_SE_BD5_21);
        while(1) {
            if(MBMotionTimeGet(mb5_Work.moveFanMdlId[no-1]) <= 0) {
                break;
            }
            HuPrcVSleep();
        }
        HuAudFXStop(seNo);
        MBModelAttrReset(mb5_Work.moveFanMdlId[no-1], HU3D_MOTATTR_REV);
        MBMotionSpeedSet(mb5_Work.moveFanMdlId[no-1], 0);
        MBPlayerPosGet(playerNo, &playerPos);
        MBModelPosGet(mb5_Work.autoFanMdlId[no-1], &fanPos);
        jumpPos.x = 0.5f*(playerPos.x+fanPos.x);
        jumpPos.y = (0.5f*(playerPos.y+fanPos.y))+150;
        jumpPos.z = 0.5f*(playerPos.z+fanPos.z);
        MBPlayerWalkMain(playerNo, NULL, &jumpPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 11, NULL, TRUE);
        MBPlayerWalkMain(playerNo, &jumpPos, &fanPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 11, NULL, TRUE);
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMPEND, 0, 4, HU3D_MOTATTR_NONE);
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        MBMotionStartEndSet(mb5_Work.moveFanMdlId[no-1], -1, 290);
    } else if(no == 2) {
        GwPlayer[playerNo].masuIdPrev = GwPlayer[playerNo].masuId;
        GwPlayer[playerNo].masuIdNext = endMasuId;
        MBPlayerMasuWalk(playerNo, TRUE);
        GwPlayer[playerNo].masuId = endMasuId;
    }
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    seNo = HuAudFXPlay(MSM_SE_BD5_21);
    MBMotionSpeedSet(mb5_Work.moveFanMdlId[no-1], 1);
    MBMotionTimeSet(mb5_Work.moveFanMdlId[no-1], 1);
    fanMdlId = MBModelIdGet(mb5_Work.moveFanMdlId[no-1]);
    MBMotionSpeedSet(mb5_Work.autoFanMdlId[no-1], 1);
    MBMotionTimeSet(mb5_Work.autoFanMdlId[no-1], 1);
    rotYSpeed = 6.3f;
    MBPlayerRotGet(playerNo, &rot);
    mb5ev_MoveFanCurrNo = no-1;
    while(1) {
        if(MBMotionTimeGet(mb5_Work.moveFanMdlId[no-1]) >= 200) {
            break;
        }
        Hu3DModelObjMtxGet(fanMdlId, moveFanHookTbl[no-1], fanMtx);
        MBPlayerPosSet(playerNo, fanMtx[0][3], fanMtx[1][3], fanMtx[2][3]);
        rot.y += rotYSpeed;
        MBPlayerRotSetV(playerNo, &rot);
        HuPrcVSleep();
    }
    HuAudFXStop(seNo);
    if(no == 1) {
        MBMotionSpeedSet(mb5_Work.laserMdlId, 1);
        MBMotionStartEndSet(mb5_Work.laserMdlId, 20, 60);
        MBModelAttrReset(mb5_Work.laserMdlId, HU3D_MOTATTR_LOOP);
        while(1) {
            if(MBMotionEndCheck(mb5_Work.laserMdlId)) {
                break;
            }
            laserTime = MBMotionTimeGet(mb5_Work.laserMdlId);
            if(laserTime == 22 || laserTime == 32 || laserTime  == 42) {
                HuAudFXPlay(MSM_SE_BD5_22);
            }
            HuPrcVSleep();
        }
    }
    endMasuP = MBMasuGet(MASU_LAYER_DEFAULT, endMasuId);
    GwPlayer[playerNo].masuIdPrev = GwPlayer[playerNo].masuId;
    MBPlayerPosFixCurr(endMasuP->linkTbl[0], FALSE);
    GwPlayer[playerNo].masuId = endMasuP->linkTbl[0];
    GwPlayer[playerNo].masuIdNext = endMasuP->linkTbl[0];
    MBPlayerPosGet(playerNo, &playerPos);
    MBMasuPosGet(MASU_LAYER_DEFAULT, endMasuP->linkTbl[0], &fanPos);
    jumpPos.x = 0.5f*(playerPos.x+fanPos.x);
    jumpPos.y = (0.5f*(playerPos.y+fanPos.y))+100;
    jumpPos.z = 0.5f*(playerPos.z+fanPos.z);
    MBPlayerWalkMain(playerNo, NULL, &jumpPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 11, NULL, TRUE);
    MBPlayerWalkMain(playerNo, &jumpPos, &fanPos, MB_PLAYER_MOT_JUMP, 1, HU3D_MOTATTR_NONE, 11, NULL, TRUE);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMPEND, 0, 4, HU3D_MOTATTR_NONE);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    if(no == 1) {
        MBModelAttrSet(mb5_Work.laserMdlId, HU3D_MOTATTR_REV);
        while(1) {
            if(MBMotionEndCheck(mb5_Work.laserMdlId)) {
                break;
            }
            laserTime = MBMotionTimeGet(mb5_Work.laserMdlId);
            if(laserTime == 22 || laserTime == 32 || laserTime  == 42) {
                HuAudFXPlay(MSM_SE_BD5_22);
            }
            HuPrcVSleep();
        }
        MBMotionStartEndSet(mb5_Work.laserMdlId, 0, 20);
        MBMotionSpeedSet(mb5_Work.laserMdlId, 0);
        MBMotionTimeSet(mb5_Work.laserMdlId, 0);
        MBModelAttrSet(mb5_Work.laserMdlId, HU3D_MOTATTR_LOOP);
        MBModelAttrReset(mb5_Work.laserMdlId, HU3D_MOTATTR_REV);
    }
    HuPrcSleep(5);
    if(no == 1) {
        mb5ev_MoveFanResetF = TRUE;
        mb5ev_MoveFanPlayerPrev = playerNo;
    }
}

void MB5Ev_ReelCapsuleAdd(int playerNo, int capsuleNo)
{
    int i; //r31
    int capObj; //r29
    s16 winId; //r27
    
    float t; //f31
    
    char insertMes[256]; //sp+0x44
    HuVecF objPos; //sp+0x38
    HuVecF playerPos; //sp+0x2C
    HuVecF endPos; //sp+0x20
    //TODO: Find way to match without wrong type
    HuVecF winCenter; //sp+0x14
    HuVec2f winPos; //sp+0xC
    char baseMes[3] = {}; //sp+0x8
    
    static HuVecF objStartPos = { 1363, 3200, -3031 };
    
    static HuVecF objEndPos = { 1363, 3000, -3031 };
    
    capObj = MBCapsuleObjCreate(capsuleNo, FALSE);
    MBCapsuleObjScaleSet(capObj, 0.5f, 0.5f, 0.5f);
    MBCapsuleObjAttrSet(capObj, 2, 2);
    MBModelLayerSet(capObj, 3);
    MBPlayerPosGet(playerNo, &playerPos);
    for(i=1; i<32; i++) {
        t = i/32.0f;
        objPos.x = objStartPos.x+(t*(objEndPos.x-objStartPos.x));
        objPos.y = objStartPos.y+(t*(objEndPos.y-objStartPos.y))+((40.0f+(t*50.0f))*fabs(HuCos(t*270)));
        objPos.z = objStartPos.z+(t*(objEndPos.z-objStartPos.z));
        MBCapsuleObjPosSetV(capObj, &objPos);
        HuPrcVSleep();
    }
    HuAudFXPlay(MSM_SE_BOARD_110);
    endPos = objEndPos;
    for(i=1; i<32; i++) {
        t = i/32.0f;
        objPos.x = endPos.x+(t*(playerPos.x-endPos.x));
        objPos.y = endPos.y+(t*(playerPos.y-endPos.y))+((40.0f+(t*50.0f))*fabs(HuCos(t*270)));
        objPos.z = endPos.z+(t*(playerPos.z-endPos.z));
        MBCapsuleObjPosSetV(capObj, &objPos);
        HuPrcVSleep();
    }
    MBCapsuleObjKill(capObj);
    MBPlayerCapsuleAdd(playerNo, capsuleNo);
    if(capsuleNo == CAPSULE_MIRACLE) {
        MB3MiracleGetExec(playerNo);
    }
    MBPlayerRotateStart(playerNo, 0, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINWIN, 0, 4, HU3D_MOTATTR_NONE);
    winId = MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_CAPMACHINE_GETCAPSULE, HUWIN_SPEAKER_NONE);
    MBTopWinPosGet(&winPos);
    MBTopWinSizeSet(512, 64);
    baseMes[0] = 30;
    baseMes[1] = 5;
    strcpy(insertMes, baseMes);
    strcat(insertMes, HuWinMesPtrGet(MBCapsuleMesGet(capsuleNo)));
    baseMes[1] = 8;
    strcat(insertMes, baseMes);
    MBTopWinInsertMesSet(MESSNUM_PTR(insertMes), 0);
    
    MBWinCenterGet(winId, (HuVec2f *)&winCenter);
    MBTopWinPosSet(winCenter.x, winPos.y);
    MBTopWinWait();
    MBPlayerMotionEndWait(playerNo);
}


void MB5Ev_WarpParticleCreate(HuVecF pos)
{
    void *animFile;
    
    static HU3DPARMANPARAM param = {
        70,
        0,
        1.5f,
        150,
        90,
        { 0, 0.2f, 0 },
        1.0f,
        1.0f,
        16.0f,
        1.0f,
        1,
        {
            { 255, 255, 255, 255 },
        },
        {
            { 255, 255, 255, 0 },
        },
    };
    animFile = HuDataSelHeapReadNum(EFFECT_ANM_circle, HU_MEMNUM_OVL, HEAP_MODEL);
    mb5ev_WarpAnim = HuSprAnimRead(animFile);
    HuDataDirClose(DATA_effect);
    mb5ev_WarpParmanId = Hu3DParManCreate(mb5ev_WarpAnim, 256, &param);
    Hu3DParManAttrSet(mb5ev_WarpParmanId, HU3D_PARMAN_ATTR_RANDSCALE70|HU3D_PARMAN_ATTR_RANDANGLE);
    Hu3DParManRotSet(mb5ev_WarpParmanId, 0, 90, 0);
    Hu3DParManPosSet(mb5ev_WarpParmanId, pos.x, pos.y, pos.z);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(mb5ev_WarpParmanId), 5);
    Hu3DParManTimeLimitSet(mb5ev_WarpParmanId, 120);
}

void MB5Ev_RocketDustCreate(HuVecF pos)
{
    int i;
    void *animFile;
    static HU3DPARMANPARAM param = {
        30,
        0,
        1.0f,
        180,
        0,
        { 0, -0.2f, 0 },
        10.0f,
        0.8f,
        200.0f,
        1.0f,
        4,
        {
            { 255, 255, 255, 80 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
        },
        {
            { 255, 255, 255, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
        },
    };
    animFile = HuDataSelHeapReadNum(EFFECT_ANM_landDust, HU_MEMNUM_OVL, HEAP_MODEL);
    mb5ev_RocketDustAnim = HuSprAnimRead(animFile);
    HuDataDirClose(DATA_effect);
    mb5ev_RocketDustParmanId[0] = Hu3DParManCreate(mb5ev_RocketDustAnim, 128, &param);
    Hu3DParManRotSet(mb5ev_RocketDustParmanId[0], 0, 0, 0);
    Hu3DParManPosSet(mb5ev_RocketDustParmanId[0], pos.x, pos.y, pos.z);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(mb5ev_RocketDustParmanId[0]), 5);
    Hu3DParManTimeLimitSet(mb5ev_RocketDustParmanId[0], 90);
    for(i=1; i<13; i++) {
        mb5ev_RocketDustParmanId[i] = Hu3DParManCreate(mb5ev_RocketDustAnim, 128, &param);
        Hu3DParManRotSet(mb5ev_RocketDustParmanId[i], 0, 30.0f*i, 0);
        Hu3DParManPosSet(mb5ev_RocketDustParmanId[i], pos.x, pos.y, pos.z);
        Hu3DModelLayerSet(Hu3DParManModelIDGet(mb5ev_RocketDustParmanId[i]), 5);
        Hu3DParManTimeLimitSet(mb5ev_RocketDustParmanId[i], 90);
    }
}

void MB5Ev_WarpPlayerShrink(int playerNo)
{
    HuVecF scale = { 1, 1, 1 };
    float alpha = 255;
    MBMODELID mdlId = MBPlayerModelGet(playerNo);
    float speed = 0.04f;
    float alphaSpeed = alpha*speed;
    while(1) {
        scale.x -= speed;
        scale.y += speed;
        scale.z += speed;
        alpha -= alphaSpeed;
        if(scale.x < 0) {
            mb5ev_WarpEndSize = scale;
            MBModelScaleSet(mdlId, 0, 0, 0);
            MBModelDispSet(mdlId, FALSE);
            break;
        }
        MBModelScaleSet(mdlId, scale.x, scale.y, scale.z);
        HuPrcVSleep();
    }
}

void MB5Ev_WarpPlayerGrow(int playerNo)
{
    
    float alpha = 255;
    MBMODELID mdlId = MBPlayerModelGet(playerNo);
    float speed = 0.04f;
    float alphaSpeed = alpha*speed;
    HuVecF scale = mb5ev_WarpEndSize;
    MBModelScaleSet(mdlId, mb5ev_WarpEndSize.x, mb5ev_WarpEndSize.y, mb5ev_WarpEndSize.z);
    MBModelDispSet(mdlId, TRUE);
    while(1) {
        scale.x += speed;
        scale.y -= speed;
        scale.z -= speed;
        alpha += alphaSpeed;
        if(scale.x > 1) {
            MBModelScaleSet(mdlId, 1, 1, 1);
            break;
        }
        MBModelScaleSet(mdlId, scale.x, scale.y, scale.z);
        HuPrcVSleep();
    }
}

void MB5Ev_WarpPathCheck(s16 masuId)
{
    MASU *masuP; //r31
    int i; //r26
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    if(mb5ev_WarpPathEndF) {
        return;
    }
    mb5ev_WarpPathLen++;
    for(i=0; i<masuP->linkNum; i++) {
        if(!(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & (MBBranchFlagGet()|(MASU_FLAG_BLOCKL|MASU_FLAG_BLOCKR)))) {
            if(masuP->linkTbl[i] == mb5ev_WarpMasuId) {
                mb5ev_WarpPathLen--;
                mb5ev_WarpPathEndF = TRUE;
                return;
            } else if(MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId) == MASU_TYPE_STAR) {
                mb5ev_WarpPathValidF = TRUE;
                mb5ev_WarpPathEndF = TRUE;
                mb5ev_WarpPathLen--;
                return;
            } else {
                MB5Ev_WarpPathCheck(masuP->linkTbl[i]);
            }
        }
    }
    mb5ev_WarpPathLen--;
}

u32 MB5_EventMasuFlagGet(int no, u32 flag)
{
    u32 bitIn;
    int i;
    int bitOut;
    u32 result;
    bitIn = bitOut = (1 << 0);
    result = 0;
    for(i=0; i<32; i++) {
        if(flag & bitIn) {
            if(no & bitOut) {
                result |= bitIn;
            }
            bitOut <<= 1;
        }
        bitIn <<= 1;
    }
    return result;
}

void MB5Ev_RocketJetCoinInit(void)
{
    int i;
    for(i=0; i<MB5EV_ROCKET_JET_COIN_MAX; i++) {
        mb5ev_RocketJetCoin[i].mdlId = MBModelCreate(BOARD_HSF_coin, NULL, TRUE);
        mb5ev_RocketJetCoin[i].mode = -1;
        mb5ev_RocketJetCoin[i].pos.x = mb5ev_RocketJetCoin[i].pos.y = mb5ev_RocketJetCoin[i].pos.z = 0;
        mb5ev_RocketJetCoin[i].rot.x = mb5ev_RocketJetCoin[i].rot.y = mb5ev_RocketJetCoin[i].rot.z = 0;
        mb5ev_RocketJetCoin[i].enableF = FALSE;
        MBModelDispSet(mb5ev_RocketJetCoin[i].mdlId, FALSE);
    }
}

void MB5Ev_RocketJetCoinCreate(HuVecF *startPos, HuVecF *endPos)
{
    int i;
    for(i=0; i<MB5EV_ROCKET_JET_COIN_MAX; i++) {
        if(!mb5ev_RocketJetCoin[i].enableF) {
            MBModelDispSet(mb5ev_RocketJetCoin[i].mdlId, TRUE);
            mb5ev_RocketJetCoin[i].scale.x = mb5ev_RocketJetCoin[i].scale.y = mb5ev_RocketJetCoin[i].scale.z = 0.5f;
            MBModelPosSetV(mb5ev_RocketJetCoin[i].mdlId, startPos);
            MBModelRotSetV(mb5ev_RocketJetCoin[i].mdlId, &mb5ev_RocketJetCoin[i].rot);
            MBModelScaleSetV(mb5ev_RocketJetCoin[i].mdlId, &mb5ev_RocketJetCoin[i].scale);
            mb5ev_RocketJetCoin[i].enableF = TRUE;
            mb5ev_RocketJetCoin[i].alpha = 255;
            mb5ev_RocketJetCoin[i].mode = 0;
            mb5ev_RocketJetCoin[i].time = 0;
            MBModelAlphaSet(mb5ev_RocketJetCoin[i].mdlId, 255);
            mb5ev_RocketJetCoin[i].startPos = *startPos;
            mb5ev_RocketJetCoin[i].endPos = *endPos;
            break;
        }
    }
}

void MB5Ev_RocketJetCoinClose(void)
{
    int i;
    for(i=0; i<MB5EV_ROCKET_JET_COIN_MAX; i++) {
        if(mb5ev_RocketJetCoin[i].mdlId >= 0) {
            MBModelKill(mb5ev_RocketJetCoin[i].mdlId);
            mb5ev_RocketJetCoin[i].mdlId = MB_MODEL_NONE;
        }
    }
}

void MB5Ev_RocketJetCoinUpdate(int playerNo)
{
    int i; //r31
    int j; //r30
    
    float scale; //f27
    float t; //f26
    float rangeXZ; //f23
    float rangeYZ; //f22
    
    HuVecF playerPos; //sp+0x34
    HuVecF dir; //sp+0x28
    HuVecF effectPos; //sp+0x1C
    HuVecF coinPos; //sp+0x10
    rangeXZ = 200;
    rangeYZ = 150;
    for(i=0; i<MB5EV_ROCKET_JET_COIN_MAX; i++) {
        if(!mb5ev_RocketJetCoin[i].enableF) {
            continue;
        }
        switch(mb5ev_RocketJetCoin[i].mode) {
            case 0:
                t = (++mb5ev_RocketJetCoin[i].time)/25.0f;
                VECSubtract(&mb5ev_RocketJetCoin[i].endPos, &mb5ev_RocketJetCoin[i].startPos, &coinPos);
                VECScale(&coinPos, &coinPos, t);
                VECAdd(&coinPos, &mb5ev_RocketJetCoin[i].startPos, &coinPos);
                coinPos.y += 200*HuSin(t*180);
                mb5ev_RocketJetCoin[i].pos = coinPos;
                MBModelPosSetV(mb5ev_RocketJetCoin[i].mdlId, &mb5ev_RocketJetCoin[i].pos);
                if(t >= 1) {
                    mb5ev_RocketJetCoin[i].mode = 1;
                    mb5ev_RocketJetCoin[i].time = 0;
                }
                break;
            
            case 1:
                MBPlayerPosGet(playerNo, &playerPos);
                VECSubtract(&mb5ev_RocketJetCoin[i].pos, &playerPos, &dir);
                if(HuMagXZVecF(&dir) < rangeXZ && HuMagPoint2D(dir.y, dir.z) < rangeYZ) {
                    effectPos = mb5ev_RocketJetCoin[i].pos;
                    for(j=0; j<5; j++) {
                        effectPos.y += frandmod(100)-50.0f;
                        effectPos.z += frandmod(100)-50.0f;
                        CharEffectCoinGlowCreate(HU3D_CAM0, &effectPos);
                    }
                    MBPlayerCoinAdd(playerNo, 1);
                    MBAudFXPlay(MSM_SE_CMN_08);
                    mb5ev_RocketJetCoinTotal++;
                    MBModelDispSet(mb5ev_RocketJetCoin[i].mdlId, FALSE);
                    mb5ev_RocketJetCoin[i].enableF = FALSE;
                    mb5ev_RocketJetCoin[i].time = 0;
                    mb5ev_RocketJetCoin[i].alpha = 255;
                    mb5ev_RocketJetCoin[i].mode = -1;
                    mb5ev_RocketJetCoin[i].pos.x = mb5ev_RocketJetCoin[i].pos.y = mb5ev_RocketJetCoin[i].pos.z = 0;
                    mb5ev_RocketJetCoin[i].rot.x = mb5ev_RocketJetCoin[i].rot.y = mb5ev_RocketJetCoin[i].rot.z = 0;
                } else if(++mb5ev_RocketJetCoin[i].time >= 200) {
                    mb5ev_RocketJetCoin[i].mode = 2;
                }
                break;
            
            case 2:
                mb5ev_RocketJetCoin[i].alpha -= 5;
                MBModelAlphaSet(mb5ev_RocketJetCoin[i].mdlId, mb5ev_RocketJetCoin[i].alpha);
                if(mb5ev_RocketJetCoin[i].alpha <= 0) {
                    MBModelAlphaSet(mb5ev_RocketJetCoin[i].mdlId, 0);
                    MBModelDispSet(mb5ev_RocketJetCoin[i].mdlId, FALSE);
                    mb5ev_RocketJetCoin[i].enableF = FALSE;
                    mb5ev_RocketJetCoin[i].time = 0;
                    mb5ev_RocketJetCoin[i].alpha = 255;
                    mb5ev_RocketJetCoin[i].mode = -1;
                    mb5ev_RocketJetCoin[i].pos.x = mb5ev_RocketJetCoin[i].pos.y = mb5ev_RocketJetCoin[i].pos.z = 0;
                    mb5ev_RocketJetCoin[i].rot.x = mb5ev_RocketJetCoin[i].rot.y = mb5ev_RocketJetCoin[i].rot.z = 0;
                }
                break;
        }
        scale = (mb5ev_RocketJetCoin[i].scale.x += 0.05f);
        if(scale >= 1) {
            scale = 1;
        }
        mb5ev_RocketJetCoin[i].scale.x = mb5ev_RocketJetCoin[i].scale.y = mb5ev_RocketJetCoin[i].scale.z = scale;
        MBModelScaleSetV(mb5ev_RocketJetCoin[i].mdlId, &mb5ev_RocketJetCoin[i].scale);
        mb5ev_RocketJetCoin[i].rot.y += 3;
        MBModelRotSetV(mb5ev_RocketJetCoin[i].mdlId, &mb5ev_RocketJetCoin[i].rot);
    }
}

int MB5Ev_RocketJetCoinNumGet(void)
{
    int i;
    int num;
    num = 0;
    for(i=0; i<MB5EV_ROCKET_JET_COIN_MAX; i++) {
        if(mb5ev_RocketJetCoin[i].enableF) {
            num++;
        }
    }
    return num;
}

HuVecF MB5Ev_RocketJetCoinPosGet(s16 no)
{
    int i;
    int num;
    s16 idx[MB5EV_ROCKET_JET_COIN_MAX];
    HuVecF out;
    
    for(i=0; i<MB5EV_ROCKET_JET_COIN_MAX; i++) {
        idx[i] = -1;
    }
    for(i=num=0; i<MB5EV_ROCKET_JET_COIN_MAX; i++) {
        if(mb5ev_RocketJetCoin[i].enableF) {
            idx[num] = i;
            num++;
        }
    }
    i = idx[no];
    if(i == -1) {
        out.x = out.y = out.z = -1;
    } else {
        out = mb5ev_RocketJetCoin[i].pos;
    }
    return out;
}

int MB5Ev_ReelValueGet(s16 no)
{
    s16 value = -1;
    float time = MBMotionTimeGet(mb5_Work.reelMdlId[no]);
    value = time*0.05f;
    if(value >= 8) {
        value -= 8;
    }
}

int MB5_MasuPathCheck(s16 id, u32 flag, s16 *linkTbl, BOOL endF)
{
    if(flag & MB5EV_MASU_WARP) {
        MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, id);
        int num;
        int i;
        for(i=num=0; i<masuP->linkNum; i++) {
            if(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & MBBranchFlagGet()) {
                s16 link1 = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, id, MB5EV_MASU_WARP_JUMP);
                s16 link2 = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, link1, MB5EV_MASU_WARP_JUMP);
                s16 link3 = MBMasuFlagMatchFindLink(MASU_LAYER_DEFAULT, link2, MB5EV_MASU_WARP);
                linkTbl[num++] = link3;
            } else {
                linkTbl[num++] = masuP->linkTbl[i];
            }
        }
        return num;
    } else {
        return -1;
    }
}