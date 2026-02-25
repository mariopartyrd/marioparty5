#include "game/board/main.h"
#include "game/board/effect.h"
#include "game/board/audio.h"
#include "game/board/scroll.h"
#include "game/board/model.h"
#include "game/board/masu.h"
#include "game/board/opening.h"
#include "game/board/capsule.h"
#include "game/board/branch.h"
#include "game/board/camera.h"
#include "game/board/player.h"
#include "game/board/guide.h"
#include "game/board/window.h"
#include "game/board/coin.h"
#include "game/board/roulette.h"
#include "game/board/star.h"
#include "game/board/capsule.h"
#include "game/board/status.h"
#include "game/board/com.h"

#include "game/sprite.h"
#include "game/wipe.h"
#include "game/gamemes.h"
#include "game/pad.h"
#include "game/main.h"

#include "datanum/capsule.h"
#include "datanum/capsulechar0.h"

#include "datanum/w01.h"
#include "messnum/board_w01.h"

#define MB1EV_MASU_TRAIN1 MASU_FLAG_BIT(15)
#define MB1EV_MASU_HAKO MASU_FLAG_BIT(14)
#define MB1EV_MASU_TRAIN2 MASU_FLAG_BIT(13)
#define MB1EV_MASU_HASI1 MASU_FLAG_BIT(12)
#define MB1EV_MASU_BKOOPA MASU_FLAG_BIT(11)
#define MB1EV_MASU_HASI2 MASU_FLAG_BIT(10)
#define MB1EV_MASU_TUMIKI MASU_FLAG_BIT(9)
#define MB1EV_MASU_HASI3 MASU_FLAG_BIT(8)
#define MB1EV_MASU_TUMIKI_FALL MASU_FLAG_BIT(7)
#define MB1EV_MASU_HASI4 MASU_FLAG_BIT(6)
#define MB1EV_MASU_HHOUSE1 MASU_FLAG_BIT(5)
#define MB1EV_MASU_CAR MASU_FLAG_BIT(4)
#define MB1EV_MASU_HHOUSE2 MASU_FLAG_BIT(3)
#define MB1EV_MASU_HHOUSE3 MASU_FLAG_BIT(1)

typedef struct MB1_TrainPoint_s {
    int idx; //0x00
    int nextIdx; //0x04
    float endTime; //0x08
    float startTime; //0x0C
    HuVecF pos; //0x10
    HuVecF rot; //0x1C
} MB1_TRAIN_POINT;

typedef struct MB1_Work_s {
    int bkoopaMdlId; //0x00
    int tumikiMdlId; //0x04
    int trainMdlId[3]; //0x08
    int hakoMdlId; //0x14
    int hhouseMdlId[3]; //0x18
    int trainSenroMdlId; //0x24
    int hasiMdlId[2]; //0x28
    int carMdlId; //0x30
    int capsuleColMdlId; //0x34
    int coinMdlId; //0x38
    int hasiMode[2]; //0x3C
    BOOL hasiSeF[2]; //0x44
    int hasiMasu[2][2]; //0x4C
    int carMasu; //0x5C
    int trainDelay; //0x60
    int trainMode; //0x64
    int trainPointNum; //0x68
    float trainMaxTime; //0x6C
    float trainTime; //0x70
    float trainSpeed; //0x74
    float trainMaxSpeed; //0x78
    float coinSpeed; //0x7C
    HuVecF trainRot[3]; //0x80
    BOOL hhouseSeF[3]; //0xA4
    int trainSeNo1; //0xB0
    int trainSeNo2; //0xB4
    int hhousePrevPlayer; //0xB8
    int hhousePrevTurn; //0xBC
    int hhouseNo; //0xC0
    MB1_TRAIN_POINT *trainPoint; //0xC4
    HSFOBJECT *trainObj[7]; //0xC8
} MB1_WORK;

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

static MB1_WORK mb1_Work;
static MBMODELID mb1_MainMdlId;
static s16 mb1_StarPathMasuNo;
static s16 mb1_StarPathLen;
static s16 mb1_StarPathValidF;
static int mb1ev_TumikiArmyTimer;
static OMOBJ *mb1_CoinEffObj;
static OMOBJ *mb1_ExplodeEffObj;
static OMOBJ *mb1_CoinObj;
static OMOBJ *mb1_MainObj;

static HuVecF mb1ev_TrainPath[] = {
    { -1.915, 469.464, 2751.58 },
    { -580.25, 471.633, 2641.25 },
    { -865.361, 468.953, 2628.77 },
    { -1138.152, 466.271, 2618.68 },
    { -1412.862, 460.911, 2705.01 },
    { -1662.488, 418.361, 2805.36 },
    { -1846.859, 375.812, 3001.11 },
    { -2020.446, 277.578, 3193.85 },
    { -2204.011, 179.343, 3393.24 },
    { -2432.948, 109.719, 3506.7 },
    { -2694.274, 93.488, 3556.32 },
    { -2974.024, 89.162, 3501.62 },
    { -3245.914, 84.836, 3394.85 },
    { -3676.448, 168.933, 3072.13 },
    { -3985.845, 271.324, 2627.57 },
    { -4181.565, 490.99, 2129.7 },
    { -4314.454, 685.88, 1650.22 },
    { -4435.715, 921.132, 1124.52 },
    { -4499.265, 1132.735, 610.37 },
    { -4493.411, 1315.934, 78.89 },
    { -4458.717, 1496.253, -464.13 },
    { -4319.93, 1670.751, -966.72 },
    { -4077.319, 1889.773, -1451.99 },
    { -3776.86, 2099.411, -1850.6 },
    { -3384.136, 2342.109, -2145.22 },
    { -2927.919, 2537.273, -2387.85 },
    { -2425.529, 2708.696, -2578.49 },
    { -1893.839, 2804.936, -2705.58 },
    { -1322.038, 2860.622, -2803.79 },
    { -784.705, 2890.001, -2861.56 },
    { -223.365, 2890, -2899.83 },
    { 344.425, 2890.002, -2894.47 },
    { 907.041, 2888.546, -2856.97 },
    { 1461.76, 2852.534, -2784.63 },
    { 2008.285, 2787.41, -2680.14 },
    { 2534.18, 2663.868, -2540.82 },
    { 3026.259, 2506.325, -2350.6 },
    { 3486.987, 2253.289, -2090.72 },
    { 3851.548, 2015.844, -1734.38 },
    { 4143.508, 1797.28, -1319.11 },
    { 4341.732, 1612.933, -823.46 },
    { 4451.56, 1437.517, -303.69 },
    { 4489.071, 1261.281, 234.82 },
    { 4464.979, 1068.554, 757.26 },
    { 4392.669, 854.23, 1266.31 },
    { 4282.809, 644.18, 1764.64 },
    { 4120.136, 417.804, 2265.65 },
    { 3891.377, 228.86, 2761.3 },
    { 3549.03, 124.486, 3167.9 },
    { 3088.843, 91.631, 3441 },
    { 2827.196, 91.214, 3533.01 },
    { 2555.644, 102.471, 3544.42 },
    { 2287.966, 157.495, 3471.38 },
    { 2109.762, 224.851, 3289.3 },
    { 1923.183, 295.461, 3084.75 },
    { 1742.887, 372.159, 2936.54 },
    { 1513.289, 435.319, 2824.36 },
    { 1218.543, 455.064, 2790.22 },
    { 948.879, 469.157, 2796.39 },
    { 419.908, 470.16, 2787.43 }
};

#define MB1EV_TRAINPATH_LEN (sizeof(mb1ev_TrainPath)/sizeof(HuVecF))

void MB1_Create(void);
void MB1_Kill(void);
void MB1_LightSet(void);
void MB1_LightReset(void);
int MB1Ev_MasuWalkPre(int playerNo);
int MB1Ev_MasuWalkPost(int playerNo);
int MB1Ev_MasuHatena(int playerNo);
int MB1_MasuPathCheck(s16 id, u32 flag, s16 *linkTbl, BOOL endF);
HSFOBJECT *MB1_ModelObjPtrGet(HU3DMODEL *modelP, char *name);

void MB1Ev_Tumiki(int playerNo);
void MB1Ev_BKoopa(int playerNo);
void MB1Ev_Hako(int playerNo);
void MB1Ev_Train(int playerNo);

void MB1_PlayerMtxSet(s16 playerNo, Mtx *mtx);

float MB1_AngleStep(float a, float b, float t);

void MB1_BezierCalcV(float t, float *a, float *b, float *c, float *out);

float MB1_PathTimeGetEnd(float t, HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d);
float MB1_PathTimeGet(float t, HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d);

void MB1_TrainInit(void);
void MB1_TrainUpdate(void);
void MB1_ExplodeEffInit(void);
void MB1_ExplodeEffCreateN(HuVecF *pos, HuVecF *rot, int n);
void MB1_ExplodeDustCreate(HuVecF *pos, HuVecF *vel);

void MB1_TrainClose(void);
void MB1_TrainTimeSet(float time);
float MB1_TrainTimeGet(void);
void MB1_TrainSpeedSet(float speed);
void MB1_TrainCoinSpeedSet(float speed);

void MB1_TrainCoinInit(void);
int MB1_TrainCoinSearch(HuVecF *pos, HuVecF *out);
int MB1_TrainCoinCreate(Mtx *srcMtx, HuVecF *srcPos, Mtx *dstMtx, HuVecF *dstPos, float gravity, int maxTime);

void MB1_TrainCoinClose(void);
void MB1_ExplodeEffClose(void);
void MB1_CoinEffInit(void);

void MB1_CoinEffClose(void);
int MB1_CoinEffCreate(HuVecF pos, HuVecF vel, float scale, float speed, float gravity, GXColor color);

s16 MB1_StarPathLenGet(s16 masuId, s16 len);

void MB1_HasiOpen(int no);
void MB1_HasiClose(int no);

void ObjectSetup(void)
{
    GWPartyFSet(TRUE);
    MBObjectSetup(MBNO_WORLD_1, MB1_Create, MB1_Kill);
}

static void MB1_MainObjUpdate(OMOBJ *obj);

void MB1_Create(void)
{
    int boardNo = MBBoardNoGet();
    int id;
    int masuNum;
    
    HuAudSndGrpSetSet(MSM_GRP_BD1);
    MBMasuCreate(W01_BIN_masuData);
    mb1_MainMdlId = MBModelCreate(W01_HSF_map, NULL, FALSE);
    MBModelPosSet(mb1_MainMdlId, 0, 0, 0);
    MBModelAttrSet(mb1_MainMdlId, HU3D_MOTATTR_LOOP);
    MBModelUnkSpeedSet(mb1_MainMdlId, -1);
    MBScrollInit(W01_HSF_scrollCol, 21169, 330);
    MBLightHookSet(MB1_LightSet, MB1_LightReset);
    mb1_Work.bkoopaMdlId = id = MBModelCreate(W01_HSF_bkoopa, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.tumikiMdlId = id = MBModelCreate(W01_HSF_tumiki, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.trainMdlId[0] = id = MBModelCreate(W01_HSF_trainA, NULL, FALSE);
    mb1_Work.trainMdlId[1] = id = MBModelCreate(W01_HSF_trainB, NULL, FALSE);
    mb1_Work.trainMdlId[2] = id = MBModelCreate(W01_HSF_trainC, NULL, FALSE);
    mb1_Work.hakoMdlId = id = MBModelCreate(W01_HSF_hako, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.hhouseMdlId[0] = id = MBModelCreate(W01_HSF_hhouse1, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.hhouseMdlId[1] = id = MBModelCreate(W01_HSF_hhouse2, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.hhouseMdlId[2] = id = MBModelCreate(W01_HSF_hhouse3, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.trainSenroMdlId = id = MBModelCreate(W01_HSF_trainSenro, NULL, FALSE);
    mb1_Work.hasiMdlId[0] = id = MBModelCreate(W01_HSF_hasi01, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.hasiMdlId[1] = id = MBModelCreate(W01_HSF_hasi02, NULL, FALSE);
    MBMotionSpeedSet(id, 0);
    mb1_Work.carMdlId = id = MBModelCreate(W01_HSF_car, NULL, FALSE);
    MBMotionTimeSet(id, MBMotionMaxTimeGet(id));
    mb1_Work.coinMdlId = MB_MODEL_NONE;
    mb1_Work.capsuleColMdlId = MBCapsuleColCreate(W01_HSF_capsuleCol);
    mb1_Work.hasiMode[0] = 0;
    mb1_Work.hasiMode[1] = 0;
    mb1_Work.hasiSeF[0] = FALSE;
    mb1_Work.hasiSeF[1] = FALSE;
    mb1_Work.hhousePrevPlayer = -1;
    mb1_Work.hhousePrevTurn = -1;
    mb1_Work.hhouseNo = 0;
    masuNum = MBMasuNumGet(MASU_LAYER_DEFAULT);
    for(id=1; id<=masuNum; id++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, id) & MB1EV_MASU_HASI1) {
            break;
        }
    }
    mb1_Work.hasiMasu[0][0] = id;
    for(id=1; id<=masuNum; id++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, id) & MB1EV_MASU_HASI2) {
            break;
        }
    }
    mb1_Work.hasiMasu[0][1] = id;
    for(id=1; id<=masuNum; id++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, id) & MB1EV_MASU_HASI3) {
            break;
        }
    }
    mb1_Work.hasiMasu[1][0] = id;
    for(id=1; id<=masuNum; id++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, id) & MB1EV_MASU_HASI4) {
            break;
        }
    }
    mb1_Work.hasiMasu[1][1] = id;
    for(id=1; id<=masuNum; id++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, id) & MB1EV_MASU_CAR) {
            break;
        }
    }
    mb1_Work.carMasu = id;
    MBBranchFlagSet(MB1EV_MASU_TUMIKI_FALL);
    HuDataDirClose(DATA_w01);
    MBMasuWalkPreHookSet(MB1Ev_MasuWalkPre);
    MBMasuWalkPostHookSet(MB1Ev_MasuWalkPost);
    MBMasuHatenaHookSet(MB1Ev_MasuHatena);
    MBMasuPathCheckHookSet(MB1_MasuPathCheck);
    mb1_MainObj = MBAddObj(32768, 0, 0, MB1_MainObjUpdate);
    MB1_TrainInit();
    MB1_ExplodeEffInit();
    {
        HuVecF emitterPos = { 2715, 2385, 510 };
        mb1_Work.trainSeNo2 = MBAudFXEmiterCreate(MSM_SE_BD1_19, &emitterPos, FALSE);
        MBAudFXRefSet(MSM_SE_BD1_19, &mb1_Work.trainSeNo2);
    }
    {
        
        static HuVecF cameraRot = { -33, 0, 0 };
        static HuVecF cameraOfs = { 0, 500, 0 };
        static float cameraZoom = 20000;
        static HuVecF mapCameraRot = { -78, 0, 0 };
        static HuVecF mapCameraPos = { 0, -107, 838 };
        static float mapCameraZoom = 22985;
        MBOpeningViewSet(&cameraRot, &cameraOfs, cameraZoom);
        MBMapViewCameraSet(NULL, &mapCameraPos, mapCameraZoom);
    }
}

void MB1_Kill(void)
{
    if(mb1_MainMdlId >= 0) {
        MBModelKill(mb1_MainMdlId);
        mb1_MainMdlId = MB_MODEL_NONE;
    }
    if(mb1_Work.bkoopaMdlId != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.bkoopaMdlId);
    }
    if(mb1_Work.tumikiMdlId != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.tumikiMdlId);
    }
    if(mb1_Work.trainMdlId[0] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.trainMdlId[0]);
    }
    if(mb1_Work.trainMdlId[1] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.trainMdlId[1]);
    }
    if(mb1_Work.trainMdlId[2] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.trainMdlId[2]);
    }
    if(mb1_Work.hakoMdlId != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.hakoMdlId);
    }
    if(mb1_Work.hhouseMdlId[0] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.hhouseMdlId[0]);
    }
    if(mb1_Work.hhouseMdlId[1] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.hhouseMdlId[1]);
    }
    if(mb1_Work.hhouseMdlId[2] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.hhouseMdlId[2]);
    }
    if(mb1_Work.trainSenroMdlId != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.trainSenroMdlId);
    }
    if(mb1_Work.hasiMdlId[0] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.hasiMdlId[0]);
    }
    if(mb1_Work.hasiMdlId[1] != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.hasiMdlId[1]);
    }
    if(mb1_Work.carMdlId != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.carMdlId);
    }
    if(mb1_Work.capsuleColMdlId != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.capsuleColMdlId);
    }
    MBCapsuleColCreate(HU_DATANUM_NONE);
    if(mb1_Work.coinMdlId != MB_MODEL_NONE) {
        MBModelKill(mb1_Work.coinMdlId);
    }
    mb1_MainObj = NULL;
    MB1_TrainClose();
    MB1_ExplodeEffClose();
    MB1_CoinEffClose();
    MB1_TrainCoinClose();
}

static void MB1_MainObjUpdate(OMOBJ *obj)
{
    int i; //r31
    int j; //r30
    HSFOBJECT *zenmaiObj; //r29
    int hhouseMdlId; //r28
    HU3DMODEL *modelP; //r27
    int modelId; //r26
    
    Mtx mtx; //sp+0x1C
    HuVecF pos; //sp+0x10
    HSFDATA *hsf; //sp+0xC
    if(MBKillCheck() || mb1_MainObj == NULL) {
        omDelObj(mbObjMan, obj);
        mb1_MainObj = NULL;
        return;
    }
    modelId = MBModelIdGet(mb1_Work.bkoopaMdlId);
    modelP = &Hu3DData[modelId];
    hsf = modelP->hsf;
    zenmaiObj = MB1_ModelObjPtrGet(modelP, "k_zenmai");
    if(zenmaiObj != NULL) {
        if((zenmaiObj->mesh.base.rot.z += 0.5f) >= 360) {
            zenmaiObj->mesh.base.rot.z -= 360;
        }
    }
    for(i=0; i<2; i++) {
        static HuVecF sePos[2] = {
            { -2000, 775, 3185 },
            { 2000, 775, 3185 },
        };
        
        if(mb1_Work.hasiMode[i] != 0) {
            for(j=0; j<2; j++) {
                static char *hasiName[2][2] = {
                    { "hasi1", "hasi2" },
                    { "hasi3", "hasi4" }
                };
                Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.hasiMdlId[i]), hasiName[i][j], mtx);
                MBMasuMtxSet(mb1_Work.hasiMasu[i][j], mtx);
            }
        } else {
            for(j=0; j<2; j++) {
                MBMasuMtxSet(mb1_Work.hasiMasu[i][j], NULL);
            }
        }
        switch(mb1_Work.hasiMode[i]) {
            case 0:
                break;
                
            case 1:
                if(MBMotionTimeGet(mb1_Work.hasiMdlId[i]) > 60) {
                    MBMotionSpeedSet(mb1_Work.hasiMdlId[i], 0);
                    mb1_Work.hasiMode[i]++;
                }
                break;
            
            case 2:
                if(mb1_Work.hasiSeF[i]) {
                    HuAudFXEmiterPlay(MSM_SE_BD1_11, &sePos[i]);
                    MBMotionSpeedSet(mb1_Work.hasiMdlId[i], 1);
                    mb1_Work.hasiSeF[i] = FALSE;
                    mb1_Work.hasiMode[i]++;
                }
                break;
            
            case 3:
                if(MBMotionTimeGet(mb1_Work.hasiMdlId[i]) >= MBMotionMaxTimeGet(mb1_Work.hasiMdlId[i])) {
                    MBMotionTimeSet(mb1_Work.hasiMdlId[i], 0);
                    MBMotionSpeedSet(mb1_Work.hasiMdlId[i], 0);
                    mb1_Work.hasiMode[i] = 0;
                    mb1_Work.hasiSeF[i] = FALSE;
                }
                break;
        }
    }
    Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.carMdlId), "car", mtx);
    MBMasuMtxSet(mb1_Work.carMasu, mtx);
    for(i=0; i<3; i++) {
        hhouseMdlId = mb1_Work.hhouseMdlId[i];
        if(MBMotionSpeedGet(hhouseMdlId) > 0) {
            if(mb1_Work.hhouseSeF[i] && MBMotionTimeGet(hhouseMdlId) >= 50) {
                HuAudFXPlay(MSM_SE_BD1_17);
                mb1_Work.hhouseSeF[i] = FALSE;
            }
            if(MBMotionEndCheck(hhouseMdlId)) {
                MBMotionSpeedSet(hhouseMdlId, 0);
                MBMotionTimeSet(hhouseMdlId, 0);
            }
        }
    }
    if(mb1_Work.trainSeNo2 >= 0) {
        Hu3DModelObjMtxGet(MBModelIdGet(mb1_MainMdlId), "hikosen", mtx);
        pos.x = mtx[0][3];
        pos.y = mtx[1][3];
        pos.z = mtx[2][3];
        MBAudFXEmiterUpdate(mb1_Work.trainSeNo2, &pos);
    }
    MB1_TrainUpdate();
    
}

void MB1_LightSet(void)
{
    Hu3DBGColorSet(255, 255, 255);
    Hu3DModelLightInfoSet(MBModelIdGet(mb1_MainMdlId), TRUE);
    
}

void MB1_LightReset(void)
{
    
}

int MB1Ev_MasuHatena(int playerNo)
{
    s16 masuId = GwPlayer[playerNo].masuId;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    if(masuP->flag & MB1EV_MASU_TUMIKI) {
        MB1Ev_Tumiki(playerNo);
    }
    if(masuP->flag & MB1EV_MASU_BKOOPA) {
        MB1Ev_BKoopa(playerNo);
    }
    if(masuP->flag & MB1EV_MASU_HAKO) {
        MB1Ev_Hako(playerNo);
    }
    if(masuP->flag & (MB1EV_MASU_TRAIN1|MB1EV_MASU_TRAIN2)) {
        MBPlayerPosFixFlagSet(TRUE);
        MB1Ev_Train(playerNo);
        MBPlayerPosFixFlagSet(FALSE);
    }
    HuDataDirClose(DATA_guide);
    HuDataDirClose(DATA_capsulechar0);
}

void MB1_MasuGet(void)
{
    int playerNo = GwSystem.turnPlayerNo;
    s16 masuId = GwPlayer[playerNo].masuId;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
}

int MB1Ev_MasuWalkPre(int playerNo)
{
    
}

int MB1Ev_MasuWalkPost(int playerNo)
{
    BOOL hhouseF = FALSE;
    s16 masuId = GwPlayer[playerNo].masuId;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    MBMODELID mdlId;
    
    if(masuP->flag & MB1EV_MASU_HHOUSE3) {
        hhouseF = TRUE;
        mdlId = mb1_Work.hhouseMdlId[2];
        MBMotionSpeedSet(mdlId, 1);
        MBMotionTimeSet(mdlId, 0);
        mb1_Work.hhouseSeF[2] = TRUE;
    }
    if(masuP->flag & MB1EV_MASU_HHOUSE2) {
        hhouseF = TRUE;
        mdlId = mb1_Work.hhouseMdlId[1];
        MBMotionSpeedSet(mdlId, 1);
        MBMotionTimeSet(mdlId, 0);
        mb1_Work.hhouseSeF[1] = TRUE;
    }
    if(masuP->flag & MB1EV_MASU_HHOUSE1) {
        hhouseF = TRUE;
        mdlId = mb1_Work.hhouseMdlId[0];
        MBMotionSpeedSet(mdlId, 1);
        MBMotionTimeSet(mdlId, 0);
        mb1_Work.hhouseSeF[0] = TRUE;
    }
    if(hhouseF) {
        static int seTbl[3] = {
            MSM_SE_BD1_05,
            MSM_SE_BD1_12,
            MSM_SE_BD1_13
        };
        
        if(mb1_Work.hhousePrevPlayer != playerNo || mb1_Work.hhousePrevTurn != GwSystem.turnNo || mb1_Work.hhouseNo >= 3) {
            mb1_Work.hhousePrevPlayer = playerNo;
            mb1_Work.hhousePrevTurn = GwSystem.turnNo;
            mb1_Work.hhouseNo = 0;
        }
        HuAudFXPlay(seTbl[mb1_Work.hhouseNo]);
        mb1_Work.hhouseNo++;
        HuAudFXPlay(MSM_SE_BD1_14);
    }
    if(masuP->flag & MB1EV_MASU_CAR) {
        HuAudFXPlay(MSM_SE_BD1_18);
        mdlId = mb1_Work.carMdlId;
        MBMotionSpeedSet(mdlId, 1);
        MBMotionTimeSet(mdlId, 0);
    }
    return FALSE;
}

int MB1_MasuPathCheck(s16 id, u32 flag, s16 *linkTbl, BOOL endF)
{
    return -1;
}

typedef struct MB1Ev_TumikiPlayer_s {
    int playerNo; //0x00
    int masuId; //0x04
    int fallMasuId; //0x08
    int time; //0x0C
    int maxTime; //0x10
    BOOL doneF; //0x14
    HuVecF playerPos; //0x18
    HuVecF centerPos; //0x24
    HuVecF masuPos; //0x30
    HuVecF cornerOfs; //0x3C
} MB1EV_TUMIKI_PLAYER;

static void MB1_TumikiArmyObjUpdate(OMOBJ *obj);
static void MB1_TumikiArmyPosFix(OMOBJ *obj);

void MB1Ev_Tumiki(int playerNo)
{
    MB1EV_TUMIKI_PLAYER *playerP; //r31
    s16 i; //r30
    s16 j; //r29
    s16 no; //r28
    MASU *fallMasuP; //r27
    s16 num; //r26
    MBMODELID focusMdlId; //r25
    OMOBJ *obj; //r24
    OMOBJ *posFixObj; //r23
    MASU *masuP; //r22
    MBMODELID mdlId; //r21
    s16 doneNum; //r19
    s16 fallMasuId; //r18
    s16 masuId; //r17
    
    float t; //f31
    float oldZoom; //f30
    
    MB1EV_TUMIKI_PLAYER player[GW_PLAYER_MAX]; //sp+0x60
    HuVecF masuPos; //sp+0x54
    HuVecF fallMasuPos; //sp+0x48
    HuVecF centerPos; //sp+0x3C
    HuVecF temp; //sp+0x30
    HuVecF dir; //sp+0x24
    HuVecF oldRot; //sp+0x18
    HuVecF cameraRot; //sp+0xC
    MBCAMERA *cameraP; //sp+0x8
    
    static HuVecF cornerOfs[4] = {
        { -75, 0, 75 },
        { 75, 0, 75 },
        { 75, 0, -75 },
        { -75, 0, -75 },
    };
    static HuVecF rot = { -20.5f, 0, 0 };
    
    
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    cameraP = MBCameraGet();
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBDataAsyncWait(MBDataDirReadAsync(DATA_capsulechar0));
    mb1_Work.coinMdlId = focusMdlId = MBModelCreate(BOARD_HSF_coin, NULL, FALSE);
    MBModelDispSet(focusMdlId, FALSE);
    MBCameraRotGet(&oldRot);
    oldZoom = MBCameraZoomGet();
    for(i=0, num=0, playerP=&player[0]; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, GwPlayer[i].masuId) & MB1EV_MASU_TUMIKI) {
            playerP->playerNo = i;
            playerP->masuId = GwPlayer[playerP->playerNo].masuId;
            fallMasuP = MBMasuGet(MASU_LAYER_DEFAULT, playerP->masuId);
            playerP->time = 0;
            playerP->maxTime = ((MBCapsuleEffRandF()*0.2f)+0.4f)*60;
            playerP->doneF = FALSE;
            for(j=0, playerP->fallMasuId=MASU_NULL; j<fallMasuP->linkNum; j++) {
                if(MBMasuFlagGet(MASU_LAYER_DEFAULT, fallMasuP->linkTbl[j]) & MB1EV_MASU_TUMIKI_FALL) {
                    playerP->fallMasuId = fallMasuP->linkTbl[j];
                    break;
                }
            }
            if(playerP->fallMasuId != MASU_NULL) {
                MBMasuPosGet(MASU_LAYER_DEFAULT, playerP->masuId, &temp);
                MBMasuPosGet(MASU_LAYER_DEFAULT, playerP->fallMasuId, &dir);
                MBPlayerPosGet(playerP->playerNo, &playerP->playerPos);
                VECSubtract(&playerP->playerPos, &temp, &temp);
                playerP->cornerOfs = cornerOfs[num];
                VECAdd(&dir, &playerP->cornerOfs, &playerP->masuPos);
                VECSubtract(&playerP->masuPos, &playerP->playerPos, &temp);
                VECScale(&temp, &temp, 0.5f);
                VECAdd(&playerP->playerPos, &temp, &playerP->centerPos);
                playerP->centerPos.y = playerP->playerPos.y+1000;
                playerP++;
                num++;
                
            }
        }
    }
    for(j=0, fallMasuId=MASU_NULL; j<masuP->linkNum; j++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[j]) & MB1EV_MASU_TUMIKI_FALL) {
            fallMasuId = masuP->linkTbl[j];
            break;
        }
    }
    if(fallMasuId == MASU_NULL) {
        return;
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
    MBMasuPosGet(MASU_LAYER_DEFAULT, fallMasuId, &fallMasuPos);
    MBStatusDispSetAll(FALSE);
    posFixObj = MBAddObj(32768, 0, 0, MB1_TumikiArmyPosFix);
    posFixObj->work[0] = 0;
    posFixObj->work[1] = mb1_Work.tumikiMdlId;
    HuAudFXPlay(MSM_SE_BD1_09);
    temp.x = -300;
    temp.y = 1835;
    temp.z = -470;
    MBModelPosSetV(focusMdlId, &temp);
    MBCameraModelViewSet(focusMdlId, NULL, NULL, 3000, -1, 63);
    MBCameraMotionWait();
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    mdlId = mb1_Work.tumikiMdlId;
    obj = MBAddObj(32768, 0, 0, MB1_TumikiArmyObjUpdate);
    obj->work[0] = 0;
    obj->work[1] = mdlId;
    obj->work[2] = 0;
    centerPos.x = masuPos.x+(0.5f*(fallMasuPos.x-masuPos.x));
    centerPos.y = masuPos.y+1000;
    centerPos.z = masuPos.z+(0.5f*(fallMasuPos.z-masuPos.z));
    for(no=0, playerP=&player[0]; no<num; no++, playerP++) {
        VECSubtract(&playerP->masuPos, &playerP->playerPos, &temp);
        MBPlayerRotateStart(playerP->playerNo, HuAtan(temp.x, temp.z), 15);
    }
    for(i=0; i<120; i++) {
        HuPrcVSleep();
    }
    temp.x = -804.38f;
    temp.y = 2567.96f;
    temp.z = -14.01f;
    cameraRot.x = -16.600006f;
    cameraRot.y = 20.54f;
    cameraRot.z = 0;
    MBModelPosSetV(focusMdlId, &temp);
    MBCameraModelViewSet(focusMdlId, &cameraRot, NULL, 4000, -1, 21);
    while(MBMotionTimeGet(mdlId) < 90.0f) {
        HuPrcVSleep();
    }
    for(no=0, playerP=&player[0]; no<num; no++, playerP++) {
        MBPlayerMotionNoSet(playerP->playerNo, MB_PLAYER_MOT_SHOCK, HU3D_MOTATTR_NONE);
        omVibrate(playerP->playerNo, 12, 12, 0);
    }
    do {
        for(no=0, doneNum=0, playerP=&player[0]; no<num; no++, playerP++) {
            if(playerP->doneF) {
                doneNum++;
                continue;
            }
            t = ((float)++playerP->time)/playerP->maxTime;
            MB1_BezierCalcV(t, (float *)&playerP->playerPos, (float *)&playerP->centerPos, (float *)&playerP->masuPos, (float *)&temp);
            MBPlayerPosSetV(playerP->playerNo, &temp);
            MBPlayerRotGet(playerP->playerNo, &temp);
            VECSubtract(&playerP->masuPos, &playerP->playerPos, &dir);
            temp.y = MB1_AngleStep(HuAtan(dir.x, dir.z), temp.y, 10);
            MBPlayerRotSet(playerP->playerNo, t*360, temp.y, temp.z);
            if(t >= 1) {
                playerP->playerPos = playerP->masuPos;
                playerP->masuId = playerP->fallMasuId;
                MBPlayerPosSetV(playerP->playerNo, &playerP->playerPos);
                GwPlayer[playerP->playerNo].masuId = playerP->masuId;
                if(MBMasuFlagGet(MASU_LAYER_DEFAULT, playerP->masuId) & MB1EV_MASU_TUMIKI_FALL) {
                    fallMasuP = MBMasuGet(MASU_LAYER_DEFAULT, playerP->masuId);
                    for(j=0, playerP->fallMasuId=MASU_NULL; j<fallMasuP->linkNum; j++) {
                        playerP->fallMasuId = fallMasuP->linkTbl[j];
                        break;
                    }
                    if(playerP->fallMasuId == MASU_NULL) {
                        playerP->fallMasuId = playerP->masuId;
                        playerP->doneF = TRUE;
                    }
                    MBMasuPosGet(MASU_LAYER_DEFAULT, playerP->fallMasuId, &dir);
                    VECAdd(&dir, &playerP->cornerOfs, &playerP->masuPos);
                    VECSubtract(&playerP->masuPos, &playerP->playerPos, &temp);
                    VECScale(&temp, &temp, 0.5f);
                    VECAdd(&playerP->playerPos, &temp, &playerP->centerPos);
                    playerP->centerPos.y = playerP->centerPos.y+(VECMag(&temp)*3);
                    VECSubtract(&playerP->masuPos, &playerP->playerPos, &temp);
                    playerP->time = 0;
                    playerP->maxTime = (VECMag(&temp)/20)+((MBCapsuleEffRandF()*0.1f)*60);
                    MBPlayerMotionNoShiftSet(playerP->playerNo, MB_PLAYER_MOT_DIZZY, 0, 8, HU3D_MOTATTR_LOOP);
                } else {
                    playerP->doneF = TRUE;
                }
            }
        }
        HuPrcVSleep();
    } while(doneNum < num);
    MBCameraModelViewSet(MBPlayerModelGet(playerNo), &oldRot, NULL, oldZoom, -1, 42);
    for(i=0; i<120.0f; i++) {
        HuPrcVSleep();
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    for(no=0, playerP=&player[0]; no<num; no++, playerP++) {
        GwPlayer[playerP->playerNo].masuId = playerP->fallMasuId;
        MBPlayerMotionNoSet(playerP->playerNo, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
        MBPlayerRotSet(playerP->playerNo, 0, 0, 0);
        if(playerP->playerNo == playerNo) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, playerP->fallMasuId, &temp);
            MBPlayerPosSetV(playerP->playerNo, &temp);
        }
    }
    MBPlayerPosFixPlayer(playerNo, player[0].fallMasuId, TRUE);
    mdlId = mb1_Work.tumikiMdlId;
    MBMotionSpeedSet(mdlId, 0);
    MBMotionTimeSet(mdlId, 0);
    MBCameraModelViewSet(focusMdlId, &oldRot, NULL, oldZoom, -1, 1);
    MBCameraMotionWait();
    MBStatusDispSetAll(TRUE);
    MBCameraSkipSet(FALSE);
    MBCameraFocusPlayerSet(playerNo);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    if(!MBKillCheck()) {
        posFixObj->work[0]++;
    }
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    WipeWait();
    if(focusMdlId != MB_MODEL_NONE) {
        MBModelKill(focusMdlId);
    }
    mb1_Work.coinMdlId = MB_MODEL_NONE;
    
}

void MB1Ev_BKoopa(int playerNo)
{
    s16 i; //r31
    MBMODELID mdlId; //r30
    s16 j; //r29
    s16 playerNum; //r28
    s16 mode; //r27
    int time; //r26
    int seNo; //r25
    s16 masuId; //r23
    s16 startMasuId; //r22
    int motNo; //r21
    
    float scaleXZ; //f29
    float rotY; //f28
    float t; //f25
    float scaleY; //f24
    
    Mtx mtx; //sp+0xEC
    HuVecF playerPos[GW_PLAYER_MAX]; //sp+0xBC
    HuVecF playerPosOld[GW_PLAYER_MAX]; //sp+0x8C
    HuVecF playerVel[GW_PLAYER_MAX]; //sp+0x5C
    HuVecF ofs; //sp+0x50
    HuVecF pos; //sp+0x44
    HuVecF dir; //sp+0x38
    HuVecF rot; //sp+0x2C
    BOOL stunF[GW_PLAYER_MAX]; //sp+0x1C
    s16 playerTbl[GW_PLAYER_MAX]; //sp+0x14
    MASU *masuP; //sp+0x10
    
    static float charXZScale[CHARNO_MAX] = {
        1,
        1,
        1,
        1,
        0.7f,
        1,
        0.65f,
        1,
        0.7f,
        0.7f,
        0.7f,
        0.7f,
        0.7f,
    };
    static float charYScale[CHARNO_MAX] = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1.4f,
        1.4f,
        1.4f,
        1.4f,
    };
    
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i) && masuId == GwPlayer[i].masuId) {
            playerTbl[playerNum] = i;
            playerNum++;
        }
    }
    MBPlayerRotateStart(playerNo, 0, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    ofs.x = 0;
    ofs.y = 100;
    ofs.z = 0;
    MBCameraMasuViewSet(masuId, NULL, &ofs, 1800, -1, 42);
    motNo = MBPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_325);
    MBPlayerMotionNoShiftSet(playerNo, motNo, 0, 8, HU3D_MOTATTR_NONE);
    HuPrcSleep(120);
    mdlId = mb1_Work.bkoopaMdlId;
    MBMotionSpeedSet(mdlId, 1);
    MBMotionTimeSet(mdlId, 0);
    HuAudFXPlay(MSM_SE_BD1_01);
    do {
        HuPrcVSleep();
    } while(MBMotionTimeGet(mdlId) < 10);
    for(i=0; i<playerNum; i++) {
        MBPlayerDispSet(playerTbl[i], FALSE);
        MBPlayerMotionNoSet(playerTbl[i], MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_NONE);
        MBPlayerMotionSpeedSet(playerTbl[i], 0);
        omVibrate(playerTbl[i], 12, 12, 0);
        CharFXPlay(GwPlayer[playerTbl[i]].charNo, CHARVOICEID(9));
        
    }
    HuPrcSleep(30);
    ofs.x = 0;
    ofs.y = 600;
    ofs.z = 0;
    MBCameraMasuViewSet(masuId, NULL, &ofs, 3200, -1, 42);
    do {
        HuPrcVSleep();
    } while(MBMotionTimeGet(mdlId) < 80);
    time = 0;
    scaleXZ = charXZScale[GwPlayer[playerTbl[0]].charNo];
    scaleY = charYScale[GwPlayer[playerTbl[0]].charNo];
    mode = 0;
    seNo = MSM_SENO_NONE;
    do {
        Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "fook", mtx);
        pos.x = mtx[0][3];
        pos.y = mtx[1][3];
        pos.z = mtx[2][3];
        mtx[0][3] = mtx[1][3] = mtx[2][3] = 0;
        ofs.x = 0;
        ofs.y = 1;
        ofs.z = 0;
        MTXMultVec(mtx, &ofs, &dir);
        VECNormalize(&dir, &dir);
        rot.x = HuAtan(HuMagXZVecF(&dir), dir.y);
        rot.y = HuAtan(dir.x, dir.z);
        rot.z = 0;
        MBPlayerPosSetV(playerTbl[0], &pos);
        MBPlayerRotSetV(playerTbl[0], &rot);
        
        MBPlayerDispSet(playerTbl[0], TRUE);
        if(++time <= 12.0f) {
            t = time/12.0f;
            MBPlayerScaleSet(playerTbl[0], scaleXZ, t*scaleY, scaleXZ);
            if(t >= 1) {
                MBPlayerScaleSet(playerTbl[0], scaleXZ, scaleY, scaleXZ);
            }
        }
        switch(mode) {
            case 0:
                if(MBMotionTimeGet(mdlId) >= 80) {
                    HuAudFXPlay(MSM_SE_BD1_20);
                    mode++;
                }
                break;
            
            case 1:
                if(MBMotionTimeGet(mdlId) >= 110) {
                    seNo = HuAudFXPlay(MSM_SE_BD1_21);
                    mode++;
                }
                break;
            
            case 2:
                if(MBMotionTimeGet(mdlId) >= 156) {
                    if(seNo != MSM_SENO_NONE) {
                        HuAudFXStop(seNo);
                    }
                    
                    seNo = MSM_SENO_NONE;
                    HuAudFXPlay(MSM_SE_BD1_22);
                    mode++;
                }
                
            case 3:
                if(MBMotionTimeGet(mdlId) >= 172) {
                    HuAudFXPlay(MSM_SE_BD1_24);
                    mode++;
                }
                break;
        }
        HuPrcVSleep();
    } while(MBMotionTimeGet(mdlId) < 245);
    MBCameraQuakeSet(12, 50);
    VECScale(&dir, &dir, 40);
    MTXIdentity(mtx);
    HuAudFXPlay(MSM_SE_BD1_25);
    HuAudFXPlay(MSM_SE_BD1_26);
    omVibrate(playerTbl[0], 12, 12, 0);
    CharFXPlay(GwPlayer[playerTbl[0]].charNo, CHARVOICEID(3));
    for(i=0, rotY=0; i<90.0f; i++) {
        if(i == 5) {
            ofs.x = rot.x-90;
            ofs.y = rot.y;
            ofs.z = rot.z;
            MB1_ExplodeEffCreateN(&pos, &ofs, 16);
        } else if(i > 5) {
            MB1_ExplodeDustCreate(&pos, &dir);
        }
        VECAdd(&pos, &dir, &pos);
        MBPlayerPosSetV(playerTbl[0], &pos);
        MBPlayerRotSetV(playerTbl[0], &rot);
        rotY += 20;
        MTXRotDeg(mtx, 'Y', rotY);
        MB1_PlayerMtxSet(playerTbl[0], &mtx);
        HuPrcVSleep();
    }
    MTXIdentity(mtx);
    MB1_PlayerMtxSet(playerTbl[0], &mtx);
    for(j=1; j<playerNum; j++) {
        MBMotionTimeSet(mdlId, 230);
        MBMotionSpeedSet(mdlId, 0);
        time = 0;
        scaleXZ = charXZScale[GwPlayer[playerTbl[j]].charNo];
        scaleY = charYScale[GwPlayer[playerTbl[j]].charNo];
        do {
            Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "fook", mtx);
            pos.x = mtx[0][3];
            pos.y = mtx[1][3];
            pos.z = mtx[2][3];
            mtx[0][3] = mtx[1][3] = mtx[2][3] = 0;
            ofs.x = 0;
            ofs.y = 1;
            ofs.z = 0;
            MTXMultVec(mtx, &ofs, &dir);
            VECNormalize(&dir, &dir);
            rot.x = HuAtan(HuMagXZVecF(&dir), dir.y);
            rot.y = HuAtan(dir.x, dir.z);
            rot.z = 0;
            MBPlayerPosSetV(playerTbl[j], &pos);
            MBPlayerRotSetV(playerTbl[j], &rot);
            
            MBPlayerDispSet(playerTbl[j], TRUE);
            if(++time <= 12.0f) {
                if(time == 1) {
                    seNo = HuAudFXPlay(MSM_SE_BD1_21);
                }
                t = time/12.0f;
                MBPlayerScaleSet(playerTbl[j], scaleXZ, t*scaleY, scaleXZ);
                if(t >= 1) {
                    MBPlayerScaleSet(playerTbl[j], scaleXZ, scaleY, scaleXZ);
                    MBMotionSpeedSet(mdlId, 1);
                    if(seNo != MSM_SENO_NONE) {
                        HuAudFXStop(seNo);
                        
                    }
                    seNo = MSM_SENO_NONE;
                    HuAudFXPlay(MSM_SE_BD1_22);
                }
            }
            HuPrcVSleep();
        } while(MBMotionTimeGet(mdlId) < 245);
        MBCameraQuakeSet(12, 50);
        VECScale(&dir, &dir, 40);
        MTXIdentity(mtx);
        HuAudFXPlay(MSM_SE_BD1_25);
        HuAudFXPlay(MSM_SE_BD1_26);
        omVibrate(playerTbl[j], 12, 12, 0);
        CharFXPlay(GwPlayer[playerTbl[j]].charNo, CHARVOICEID(3));
        for(i=0, rotY=0; i<90.0f; i++) {
            if(i == 5) {
                ofs.x = rot.x-90;
                ofs.y = rot.y;
                ofs.z = rot.z;
                MB1_ExplodeEffCreateN(&pos, &ofs, 16);
            } else if(i > 5) {
                MB1_ExplodeDustCreate(&pos, &dir);
            }
            VECAdd(&pos, &dir, &pos);
            MBPlayerPosSetV(playerTbl[j], &pos);
            MBPlayerRotSetV(playerTbl[j], &rot);
            rotY += 20;
            MTXRotDeg(mtx, 'Y', rotY);
            MB1_PlayerMtxSet(playerTbl[j], &mtx);
            HuPrcVSleep();
        }
        MTXIdentity(mtx);
        MB1_PlayerMtxSet(playerTbl[j], &mtx);
    }
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    mdlId = mb1_Work.bkoopaMdlId;
    MBMotionSpeedSet(mdlId, 0);
    MBMotionTimeSet(mdlId, 0);
    startMasuId = MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, NULL);
    for(i=0; i<playerNum; i++) {
        GwPlayer[playerTbl[i]].masuId = startMasuId;
        MBPlayerScaleSet(playerTbl[i], 1, 1, 1);
    }
    MBPlayerPosFixEvent(playerNo, startMasuId, TRUE);
    MBMasuPosGet(MASU_LAYER_DEFAULT, startMasuId, &ofs);
    MBPlayerPosSetV(playerNo, &ofs);
    for(i=0; i<playerNum; i++) {
        MBPlayerRotSet(playerTbl[i], 0, 0, 0);
    }
    for(i=0; i<playerNum; i++) {
        MBPlayerPosGet(playerTbl[i], &playerPosOld[i]);
        playerPos[i].x = playerPosOld[i].x;
        playerPos[i].y = playerPosOld[i].y+2000+(i*500.0f);
        playerPos[i].z = playerPosOld[i].z;
        playerVel[i].x = playerVel[i].y = playerVel[i].z = 0;
        stunF[i] = FALSE;
    }
    MBCameraSkipSet(FALSE);
    MBCameraFocusMasuSet(startMasuId);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    repeat:
    for(i=0; i<playerNum; i++) {
        if(!stunF[i]) {
            VECAdd(&playerPos[i], &playerVel[i], &playerPos[i]);
            playerVel[i].y -= 3.266667f;
            if(playerPos[i].y <= playerPosOld[i].y) {
                MBPlayerMotionNoShiftSet(playerTbl[i], MB_PLAYER_MOT_DIZZY, 0, 8, HU3D_MOTATTR_LOOP);
                playerPos[i] = playerPosOld[i];
                stunF[i] = TRUE;
            }
            MBPlayerPosSetV(playerTbl[i], &playerPos[i]);
        }
    }
    for(i=0; i<playerNum; i++) {
        if(!stunF[i]) {
            break;
        }
    }
    HuPrcVSleep();
    if(i < playerNum) {
        goto repeat;
    }
    WipeWait();
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    for(i=0; i<playerNum; i++) {
        MBPlayerMotionNoShiftSet(playerTbl[i], MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    }
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    for(i=0; i<30.0f; i++) {
        HuPrcVSleep();
    }
    MBPlayerMotionKill(playerNo, motNo);
}

void MB1Ev_Hako(int playerNo)
{
    s16 j; //r31
    s16 i; //r30
    s16 no; //r29
    s16 moveMasuId; //r28
    s16 playerNum; //r26
    int focusMdlId; //r25
    s16 testPlayer; //r24
    MBMODELID mdlId; //r23
    int masuNum; //r22
    s16 masuId; //r21
    int motNo; //r20
    
    
    float t; //f27
    float velY; //f26
    
    Mtx mtx; //sp+0xD8
    HuVecF playerPos[GW_PLAYER_MAX]; //sp+0xA8
    HuVecF masuPos[GW_PLAYER_MAX]; //sp+0x78
    HuVecF center[GW_PLAYER_MAX]; //sp+0x48
    HuVecF temp; //sp+0x3C
    HuVecF pos; //sp+0x30
    HuVecF rot; //sp+0x24
    s16 playerTbl[GW_PLAYER_MAX]; //sp+0x1C
    s16 playerMasu[GW_PLAYER_MAX]; //sp+0x14
    MASU *masuP; //sp+0x10
    static HuVecF cameraRot = { -20, 0, 0 };
    
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    motNo = MBPlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_325);
    for(i=0, playerNum=0, no=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(masuId != GwPlayer[i].masuId) {
            continue;
        }
        masuNum = MBMasuNumGet(MASU_LAYER_DEFAULT);
        playerMasu[no] = MBCapsuleEffRand(masuNum);
        for(j=0; j<masuNum; j++) {
            moveMasuId = playerMasu[no]+j;
            if(moveMasuId >= masuNum) {
                moveMasuId -= masuNum;
            }
            if(moveMasuId == 0) {
                moveMasuId++;
            }
            if(moveMasuId != masuId) {
                for(testPlayer=0; testPlayer<no; testPlayer++) {
                    if(moveMasuId == playerMasu[testPlayer]) {
                        break;
                    }
                }
                if(testPlayer >= no) {
                    if(MBMasuTypeGet(MASU_LAYER_DEFAULT, moveMasuId) == MASU_TYPE_BLUE
                        || MBMasuTypeGet(MASU_LAYER_DEFAULT, moveMasuId) == MASU_TYPE_RED) {
                            break;
                        }
                }
            }
        }
        if(j < masuNum) {
            playerMasu[no] = moveMasuId;
        } else {
            playerMasu[no] = masuId;
        }
        MBPlayerPosGet(i, &playerPos[no]);
        MBMasuPosGet(MASU_LAYER_DEFAULT, playerMasu[no], &masuPos[no]);
        VECSubtract(&masuPos[no], &playerPos[no], &temp);
        VECScale(&temp, &temp, 0.5f);
        VECAdd(&playerPos[no], &temp, &center[no]);
        if(masuPos[no].y > playerPos[no].y) {
            center[no].y = 5000+masuPos[no].y+VECMag(&temp);
        } else {
            center[no].y = 5000+playerPos[no].y+VECMag(&temp);
        }
        playerTbl[playerNum] = i;
        no++;
        playerNum++;
    }
    MBPlayerRotateStart(playerNo, 0, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(playerNo, motNo, 0, 8, HU3D_MOTATTR_NONE);
    mb1_Work.coinMdlId = focusMdlId = MBModelCreate(BOARD_HSF_coin, NULL, FALSE);
    MBModelDispSet(focusMdlId, FALSE);
    MBPlayerPosGet(playerNo, &temp);
    MBModelPosSetV(focusMdlId, &temp);
    temp.x = 0;
    temp.y = 100;
    temp.z = 0;
    MBCameraModelViewSet(focusMdlId, &cameraRot, &temp, 1800, -1, 42);
    MBCameraMotionWait();
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    do {
        HuPrcVSleep();
    } while(MBPlayerMotionTimeGet(playerNo) < MBPlayerMotionMaxTimeGet(playerNo)*0.66f);
    mdlId = mb1_Work.hakoMdlId;
    MBMotionSpeedSet(mdlId, 1);
    MBMotionTimeSet(mdlId, 0);
    HuAudFXPlay(MSM_SE_BD1_04);
    for(j=0; j<playerNum; j++) {
        MBPlayerMotionNoShiftSet(playerTbl[j], MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        omVibrate(playerTbl[j], 12, 4, 2);
        CharFXPlay(GwPlayer[playerTbl[j]].charNo, CHARVOICEID(9));
    }
    for(i=0; i<45.0f; i++) {
        t = i/45.0f;
        for(j=0; j<playerNum; j++) {
            MB1_BezierCalcV(t*0.5f, (float *)&playerPos[j], (float *)&center[j], (float *)&masuPos[j], (float *)&pos);
            MB1_BezierCalcV((t*0.5f)*0.01f, (float *)&playerPos[j], (float *)&center[j], (float *)&masuPos[j], (float *)&temp);
            rot.x = HuAtan(HuMagXZVecF(&temp), temp.y);
            rot.y = HuAtan(temp.x, temp.z);
            rot.z = 0;
            MBPlayerPosSetV(playerTbl[j], &pos);
            MBPlayerRotSetV(playerTbl[j], &rot);
        }
        HuPrcVSleep();
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    for(j=0; j<playerNum; j++) {
        static HuVecF cameraRot = { -20, 0, 0 };
        GwPlayer[playerTbl[j]].masuId = playerMasu[j];
        MBPlayerPosFixEvent(playerTbl[j], playerMasu[j], TRUE);
        MBPlayerMotionSpeedSet(playerTbl[j], 0);
        MBCameraModelViewSet(focusMdlId, &cameraRot, NULL, -1, -1, 42);
        MBCameraSkipSet(FALSE);
        temp.x = 0;
        temp.y = 100;
        temp.z = 0;
        MBCameraMasuViewSet(playerMasu[j], NULL, &temp, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        for(i=0; i<45.0f; i++) {
            t = i/45.0f;
            MB1_BezierCalcV((t*0.5f)+0.5f, (float *)&playerPos[j], (float *)&center[j], (float *)&masuPos[j], (float *)&pos);
            MB1_BezierCalcV(((t*0.5f)*0.01f)+0.5f, (float *)&playerPos[j], (float *)&center[j], (float *)&masuPos[j], (float *)&temp);
            rot.x = HuAtan(HuMagXZVecF(&temp), -temp.y);
            rot.y = HuAtan(temp.x, temp.z);
            rot.z = 0;
            MTXRotDeg(mtx, 'Y', i*20.0f);
            MBPlayerPosSetV(playerTbl[j], &pos);
            MBPlayerRotSetV(playerTbl[j], &rot);
            MB1_PlayerMtxSet(playerTbl[j], &mtx);
            HuPrcVSleep();
        }
        CharFXPlay(GwPlayer[playerTbl[j]].charNo, CHARSEID(18));
        MBPlayerMotionNoSet(playerTbl[j], MB_PLAYER_MOT_DIZZY, HU3D_MOTATTR_LOOP);
        MTXIdentity(mtx);
        MB1_PlayerMtxSet(playerTbl[j], &mtx);
        pos.x = masuPos[j].x;
        pos.y = masuPos[j].y;
        pos.z = masuPos[j].z;
        rot.x = 0;
        rot.y = 0;
        rot.z = 0;
        velY = 50;
        do {
            pos.y += velY;
            velY -= 5.7166667f;
            MBPlayerPosSetV(playerTbl[j], &pos);
            MBPlayerRotSetV(playerTbl[j], &rot);
            HuPrcVSleep();
        } while(pos.y > masuPos[j].y || velY >= 0);
        MBPlayerPosSetV(playerTbl[j], &masuPos[j]);
        MBPlayerRotSet(playerTbl[j], 0, 0, 0);
        WipeWait();
        for(i=0; i<60.0f; i++) {
            HuPrcVSleep();
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
    }
    for(j=0; j<playerNum; j++) {
        GwPlayer[playerTbl[j]].masuId = playerMasu[j];
        MBPlayerPosFixEvent(playerTbl[j], playerMasu[j], TRUE);
        rot.x = 0;
        rot.y = 0;
        rot.z = 0;
        MBPlayerRotSetV(playerTbl[j], &rot);
        MTXIdentity(mtx);
        MB1_PlayerMtxSet(playerTbl[j], &mtx);
        MBPlayerMotionSpeedSet(playerTbl[j], 1);
        MBPlayerMotionNoSet(playerTbl[j], MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
    }
    MBCameraSkipSet(FALSE);
    MBCameraFocusPlayerSet(playerNo);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    mdlId = mb1_Work.hakoMdlId;
    MBMotionSpeedSet(mdlId, 0);
    MBMotionTimeSet(mdlId, 0);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    WipeWait();
    MBPlayerMotionKill(playerNo, motNo);
    if(focusMdlId != MB_MODEL_NONE) {
        MBModelKill(focusMdlId);
    }
    mb1_Work.coinMdlId = MB_MODEL_NONE;
}

static void MB1_TrainObjUpdate(OMOBJ *obj);

void MB1Ev_Train(int playerNo)
{
    s16 i; //r30
    OMOBJ *obj; //r29
    MASU *masuP; //r28
    MASU *endMasuP; //r27
    s16 btnDelay; //r26
    s16 masuId; //r25
    int jumpMode; //r24
    int jumpTimer; //r23
    s16 endMasuId; //r22
    s16 starLenEnd; //r21
    s16 choice; //r20
    MBMODELID focusMdlId; //r19
    s16 coinDelay; //r18
    s16 starLenStart; //r17
    
    float t; //f31
    
    Mtx mtx; //sp+0xDC
    Mtx trainBMtx; //sp+0xAC
    Mtx trainCMtx; //sp+0x7C
    HuVecF masuPos; //sp+0x70
    HuVecF endMasuPos; //sp+0x64
    HuVecF rot; //sp+0x58
    HuVecF dir; //sp+0x4C
    HuVecF ofs; //sp+0x40
    HuVecF coinSpawn; //sp+0x34
    HuVecF playerPos; //sp+0x28    
    HuVecF tempPos; //sp+0x1C
    int startCoin; //sp+0x18
    int dispNo; //sp+0x14
    s16 starSearchMasuEnd; //sp+0xE
    MBMODELID guideMdlId; //sp+0xA
    s16 spA; //sp+0xA
    s16 starSearchLen; //sp+0x8
    
    masuId = GwPlayer[playerNo].masuId;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBDataAsyncWait(MBDataDirReadAsync(DATA_guide));
    MB1_TrainCoinInit();
    MB1_CoinEffInit();
    mb1_Work.coinMdlId = focusMdlId = MBModelCreate(BOARD_HSF_coin, NULL, FALSE);
    MBModelDispSet(focusMdlId, FALSE);
    ofs.x = 0;
    ofs.y = 1;
    ofs.z = 0;
    MBCameraModelViewSet(MBPlayerModelGet(playerNo), NULL, &ofs, 2000, -1, 21);
    MBPlayerRotateStart(playerNo, 180, 15);
    if(masuP->flag & MB1EV_MASU_TRAIN1) {
        playerPos.x = masuPos.x;
        playerPos.y = masuPos.y+100;
        playerPos.z = masuPos.z-400;
    } else {
        playerPos.x = masuPos.x;
        playerPos.y = masuPos.y+100;
        playerPos.z = masuPos.z-600;
    }
    tempPos.x = masuPos.x;
    tempPos.y = masuPos.y+100;
    tempPos.z = masuPos.z-200;
    ofs.x = masuPos.x;
    ofs.y = masuPos.y+250;
    ofs.z = masuPos.z-100;
    MBDataAsyncWait(MBDataDirReadAsync(DATA_guide));
    guideMdlId = MBGuideCreateFlag(MB_GUIDE_TERL, &ofs, FALSE, TRUE, TRUE, FALSE);
    HuDataDirClose(DATA_guide);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    repeatChoice:
    MBAudFXPlay(MSM_SE_GUIDE_56);
    MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_BOARD_W06_TRAIN_CHOICE, HUWIN_SPEAKER_TERL, 0);
    MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
    if(GwPlayer[playerNo].comF) {
        if(masuP->flag & MB1EV_MASU_TRAIN1) {
            for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
                endMasuP = MBMasuGet(MASU_LAYER_DEFAULT, i);
                if(endMasuP->flag & MB1EV_MASU_TRAIN2) {
                    break;
                }
            }
        } else {
            for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
                endMasuP = MBMasuGet(MASU_LAYER_DEFAULT, i);
                if(endMasuP->flag & MB1EV_MASU_TRAIN1) {
                    break;
                }
            }
        }
        if(i > MBMasuNumGet(MASU_LAYER_DEFAULT)) {
            i = masuId;
        }
        starSearchMasuEnd = i;
        starSearchLen = (GwPlayer[playerNo].dif+1)*10;
        starLenStart = MB1_StarPathLenGet(masuId, starSearchLen);
        starLenEnd = MB1_StarPathLenGet(starSearchMasuEnd, starSearchLen);
        if(starLenStart == -1 && starLenEnd == -1) {
            if(frand() & 0x1) {
                MBComChoiceSetUp();
            } else {
                MBComChoiceSetDown();
            }
        } else if(starLenStart != -1 && starLenEnd != -1) {
            if(starLenEnd < starLenStart) {
                MBComChoiceSetUp();
            } else {
                MBComChoiceSetDown();
            }
        } else {
            if(starLenEnd != -1) {
                MBComChoiceSetUp();
            } else {
                MBComChoiceSetDown();
            }
        }
    }
    MBTopWinWait();
    choice = MBWinLastChoiceGet();
    MBTopWinKill();
    switch(choice) {
        case 2:
            MBScrollExec(playerNo);
            MBStatusDispSetAll(TRUE);
            
        case 0:
        case 1:
            break;

        default:
            break;
    }
    if(choice == 2) {
        goto repeatChoice;
    }
    MBGuideEnd(guideMdlId);
    guideMdlId = MB_MODEL_NONE;
    if(choice == 0) {
        startCoin = MBPlayerCoinGet(playerNo);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBCameraSkipSet(FALSE);
        obj = MBAddObj(32768, 0, 0, MB1_TrainObjUpdate);
        obj->work[0] = 0;
        if(masuP->flag & MB1EV_MASU_TRAIN1) {
            obj->work[1] = 1;
        } else {
            obj->work[1] = 0;
        }
        obj->work[2] = 0;
        obj->work[3] = 0;
        MB1_TrainSpeedSet(0);
        MB1_TrainCoinSpeedSet(0);
        MBCameraModelViewSet(mb1_Work.trainMdlId[1], NULL, NULL, 3000, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        MB1_TrainSpeedSet(1);
        MB1_TrainCoinSpeedSet(1);
        HuAudFXPlay(MSM_SE_BD1_03);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        WipeWait();
        do {
            HuPrcVSleep();
        } while(obj->work[0] < 2);
        if(masuP->flag & MB1EV_MASU_TRAIN1) {
            playerPos.x = masuPos.x+100;
            playerPos.y = masuPos.y;
            playerPos.z = masuPos.z-650;
        } else {
            playerPos.x = masuPos.x+50;
            playerPos.y = masuPos.y;
            playerPos.z = masuPos.z-650;
        }
        if(masuP->flag & MB1EV_MASU_TRAIN1) {
            endMasuPos.x = masuPos.x;
            endMasuPos.y = masuPos.y;
            endMasuPos.z = masuPos.z-600;
        } else {
            endMasuPos.x = masuPos.x-50;
            endMasuPos.y = masuPos.y;
            endMasuPos.z = masuPos.z-600;
        }
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_RUN, 0, 8, HU3D_MOTATTR_LOOP);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            VECSubtract(&endMasuPos, &masuPos, &ofs);
            VECScale(&ofs, &ofs, t);
            VECAdd(&masuPos, &ofs, &ofs);
            MBPlayerPosSetV(playerNo, &ofs);
            MBPlayerRotSet(playerNo, 0, 180, 0);
            HuPrcVSleep();
        }
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        if(masuP->flag & MB1EV_MASU_TRAIN1) {
            for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
                endMasuP = MBMasuGet(MASU_LAYER_DEFAULT, i);
                if(endMasuP->flag & MB1EV_MASU_TRAIN2) {
                    break;
                }
            }
        } else {
            for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
                endMasuP = MBMasuGet(MASU_LAYER_DEFAULT, i);
                if(endMasuP->flag & MB1EV_MASU_TRAIN1) {
                    break;
                }
            }
        }
        if(i > MBMasuNumGet(MASU_LAYER_DEFAULT)) {
            i = masuId;
        }
        endMasuId = i;
        MBPlayerPosFixEvent(playerNo, endMasuId, FALSE);
        do {
            HuPrcVSleep();
        } while(obj->work[0] < 2);
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMP, 0, 8, HU3D_MOTATTR_NONE);
        MBPlayerPosGet(playerNo, &masuPos);
        if(masuP->flag & MB1EV_MASU_TRAIN1) {
            Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.trainMdlId[1]), "cha_l", mtx);
        } else {
            Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.trainMdlId[1]), "cha_r", mtx);
        }
        endMasuPos.x = mtx[0][3];
        endMasuPos.y = mtx[1][3];
        endMasuPos.z = mtx[2][3];
        for(i=0; i<24.0f; i++) {
            t = i/24.0f;
            VECSubtract(&endMasuPos, &masuPos, &dir);
            VECScale(&dir, &ofs, t);
            VECAdd(&masuPos, &ofs, &ofs);
            ofs.y += HuSin(t*180)*200;
            MBPlayerPosSetV(playerNo, &ofs);
            MBPlayerRotSet(playerNo, 0, HuAtan(dir.x, dir.z),0);
            HuPrcVSleep();
        }
        MBPlayerPosSetV(playerNo, &endMasuPos);
        MBPlayerPosGet(playerNo, &masuPos);
        Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.trainMdlId[1]), "cha_pos", mtx);
        endMasuPos.x = mtx[0][3];
        endMasuPos.y = mtx[1][3];
        endMasuPos.z = mtx[2][3];
        for(i=0; i<24.0f; i++) {
            t = i/24.0f;
            VECSubtract(&endMasuPos, &masuPos, &dir);
            VECScale(&dir, &ofs, t);
            VECAdd(&masuPos, &ofs, &ofs);
            ofs.y += HuSin(t*180)*200;
            MBPlayerPosSetV(playerNo, &ofs);
            MBPlayerRotSet(playerNo, 0, HuAtan(dir.x, dir.z),0);
            HuPrcVSleep();
        }
        MBPlayerPosSetV(playerNo, &endMasuPos);
        if(masuP->flag & MB1EV_MASU_TRAIN1) {
            MBPlayerRotateStart(playerNo, -90, 15);
        } else {
            MBPlayerRotateStart(playerNo, 90, 15);
        }
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_TRAIN_INST, HUWIN_SPEAKER_TERL);
        MBTopWinWait();
        MBTopWinKill();
        rot.x = -30;
        rot.y = 0;
        rot.z = 0;
        MBCameraModelViewSet(MBPlayerModelGet(playerNo), &rot, NULL, 3500, -1, 21);
        HuAudFXPlay(MSM_SE_BD1_03);
        mb1_Work.trainSeNo1 = MBAudFXEmiterCreate(MSM_SE_BD1_02, &ofs, FALSE);
        MBAudFXRefSet(MSM_SE_BD1_02, &mb1_Work.trainSeNo1);
        if(!MBKillCheck()) {
            obj->work[0]++;
        }
        spA = ((MBCapsuleEffRandF()*0.2f)+0.5f)*60;
        coinDelay = ((MBCapsuleEffRandF()*0.2f)+0.3f)*60;
        btnDelay = 60;
        jumpMode = 0;
        jumpTimer = 0;
        do {
            Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.trainMdlId[0]), "coin_pos", mtx);
            Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.trainMdlId[1]), "cha_pos", trainBMtx);
            playerPos.x = trainBMtx[0][3];
            playerPos.y = trainBMtx[1][3];
            playerPos.z = trainBMtx[2][3];
            Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.trainMdlId[2]), "cha_pos1", trainCMtx);
            tempPos.x = trainCMtx[0][3];
            tempPos.y = trainCMtx[1][3];
            tempPos.z = trainCMtx[2][3];
            switch(jumpMode) {
                case 0:
                    if(!GwPlayer[playerNo].comF) {
                        if(HuPadBtnDown[GwPlayer[playerNo].padNo] & PAD_BUTTON_A) {
                            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMP, 0, 8, HU3D_MOTATTR_NONE);
                            jumpMode = 1;
                            jumpTimer = 0;
                        }
                    } else {
                        if(btnDelay > 0) {
                            btnDelay--;
                        }
                        if(obj->work[0] == 4 && btnDelay <= 0 && obj->work[3] != 0 && MB1_TrainCoinSearch(&playerPos, &ofs) > 0) {
                            VECSubtract(&playerPos, &ofs, &ofs);
                            t = HuMagXZVecF(&ofs);
                            if(t < 200) {
                                MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMP, 0, 8, HU3D_MOTATTR_NONE);
                                jumpMode = 1;
                                jumpTimer = 0;
                            }
                        }
                    }
                    rot = mb1_Work.trainRot[1];
                    MBPlayerPosSetV(playerNo, &playerPos);
                    MBPlayerRotSetV(playerNo, &rot);
                    break;
                
                case 1:
                    t = ++jumpTimer/24.0f;
                    if(t >= 1) {
                        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
                        jumpMode = 0;
                        jumpTimer = 0;
                    }
                    ofs = playerPos;
                    ofs.y += HuSin(t*180)*200;
                    rot = mb1_Work.trainRot[1];
                    MBPlayerPosSetV(playerNo, &ofs);
                    MBPlayerRotSetV(playerNo, &rot);
                    break;
            }
            if(obj->work[0] == 4 && obj->work[3] != 0 && --coinDelay <= 0) {
                if(btnDelay <= 0) {
                    i = 5-GwPlayer[playerNo].dif;
                    btnDelay = MBCapsuleEffRandF()*(i*0.15f)*60;
                }
                coinDelay = ((MBCapsuleEffRandF()*0.2f)+0.3f)*60;
                ofs.x = ofs.y = ofs.z = 0;
                coinSpawn.x = (0.5f-MBCapsuleEffRandF())*75;
                coinSpawn.y = 50;
                coinSpawn.z = (0.5f-MBCapsuleEffRandF())*75;
                MB1_TrainCoinCreate(&mtx, &ofs, &trainCMtx, &coinSpawn, 200, 45);
            }
            HuPrcVSleep();
        } while(obj->work[0] < 6 || jumpMode > 0);
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMP, 0, 8, HU3D_MOTATTR_NONE);
        MBPlayerPosGet(playerNo, &masuPos);
        MBMasuPosGet(MASU_LAYER_DEFAULT, endMasuId, &endMasuPos);
        endMasuPos.z = masuPos.z+200;
        for(i=0; i<24.0f; i++) {
            t = i/24.0f;
            VECSubtract(&endMasuPos, &masuPos, &dir);
            VECScale(&dir, &ofs, t);
            VECAdd(&masuPos, &ofs, &ofs);
            ofs.y += HuSin(t*180)*200;
            MBPlayerPosSetV(playerNo, &ofs);
            MBPlayerRotSet(playerNo, 0, HuAtan(dir.x, dir.z),0);
            HuPrcVSleep();
        }
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_RUN, 0, 8, HU3D_MOTATTR_LOOP);
        MBPlayerPosGet(playerNo, &masuPos);
        MBMasuPosGet(MASU_LAYER_DEFAULT, endMasuId, &endMasuPos);
        for(i=0; i<90.0f; i++) {
            t = i/90.0f;
            t = HuSin(t*90);
            VECSubtract(&endMasuPos, &masuPos, &dir);
            VECScale(&dir, &ofs, t);
            VECAdd(&masuPos, &ofs, &ofs);
            MBPlayerPosSetV(playerNo, &ofs);
            MBPlayerRotSet(playerNo, 0, HuAtan(dir.x, dir.z),0);
            HuPrcVSleep();
        }
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_NONE);
        GwPlayer[playerNo].masuId = endMasuId;
        MBPlayerRotateStart(playerNo, 0, 15);
        while(!MBPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_W06_TRAIN_END, HUWIN_SPEAKER_TERL);
        MBTopWinWait();
        MBTopWinKill();
        if(!MBKillCheck()) {
            obj->work[0]++;
        }
        if(MBPlayerCoinGet(playerNo) > startCoin) {
            MBPlayerPosGet(playerNo, &ofs);
            ofs.y += 250;
            dispNo = MBCoinDispCreateSe(&ofs, MBPlayerCoinGet(playerNo)-startCoin);
            MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINWIN, 0, 8, HU3D_MOTATTR_NONE);

            do {
                HuPrcVSleep();
            } while(MBMotionShiftIDGet(MBPlayerModelGet(playerNo)) != HU3D_MOTID_NONE);
            do {
                HuPrcVSleep();
            } while(!MBPlayerMotionEndCheck(playerNo));
            HuPrcSleep(60);
        } else {
            MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINLOSE, 0, 8, HU3D_MOTATTR_NONE);
            do {
                HuPrcVSleep();
            } while(MBMotionShiftIDGet(MBPlayerModelGet(playerNo)) != HU3D_MOTID_NONE);
            do {
                HuPrcVSleep();
            } while(!MBPlayerMotionEndCheck(playerNo));
        }
    }
    
    if(focusMdlId != MB_MODEL_NONE) {
        MBModelKill(focusMdlId);
    }
    mb1_Work.coinMdlId = MB_MODEL_NONE;
    MB1_TrainCoinClose();
    MB1_CoinEffClose();
}

static void MB1_TumikiArmyObjUpdate(OMOBJ *obj)
{
    static HuVecF explodePos = { -600, 2060, -630 };
    static HuVecF explodeRot = { -10, -135, 0 };
    
    if(MBKillCheck()) {
        obj->work[0] = 7;
    }
    switch(obj->work[0]) {
        case 0:
            MBMotionSpeedSet(obj->work[1], 1);
            MBMotionTimeSet(obj->work[1], 0);
            obj->work[3] = 0;
            obj->work[0]++;
            mb1ev_TumikiArmyTimer = 0;
        
        case 1:
            if(MBMotionTimeGet(obj->work[1]) >= 38) {
                MBMotionSpeedSet(obj->work[1], 0);
                mb1ev_TumikiArmyTimer = 0;
                obj->work[0]++;
            }
            break;
        
        case 2:
            if(++mb1ev_TumikiArmyTimer >= 30.0f) {
                MBMotionSpeedSet(obj->work[1], 1);
                mb1ev_TumikiArmyTimer = 0;
                obj->work[0]++;
            }
            break;
        
        case 3:
            if(MBMotionTimeGet(obj->work[1]) >= 68) {
                HuAudFXPlay(MSM_SE_BD1_16);
                MBMotionSpeedSet(obj->work[1], 0);
                mb1ev_TumikiArmyTimer = 0;
                obj->work[0]++;
            }
            break;
        
        case 4:
            if(++mb1ev_TumikiArmyTimer >= 75.0f) {
                MBMotionSpeedSet(obj->work[1], 1);
                mb1ev_TumikiArmyTimer = 0;
                obj->work[0]++;
            }
            break;
        
        case 5:
            if(MBMotionTimeGet(obj->work[1]) >= 83) {
                MB1_ExplodeEffCreateN(&explodePos, &explodeRot, 16);
                HuAudFXPlay(MSM_SE_BD1_06);
                mb1ev_TumikiArmyTimer = 0;
                obj->work[0]++;
            }
            break;
        
        case 6:
            if(MBMotionTimeGet(obj->work[1]) >= 87) {
                obj->work[3] = Hu3DModelCreateData(CAPSULECHAR0_HSF_bomhei_explode);
                Hu3DModelCameraSet(obj->work[3], HU3D_CAM0);
                Hu3DModelPosSet(obj->work[3], -1120, 2230, -1122);
                Hu3DModelScaleSet(obj->work[3], 2, 2, 2);
                HuDataDirClose(CAPSULECHAR0_HSF_bomhei_explode);
                HuAudFXPlay(MSM_SE_BD1_07);
                HuAudFXPlay(MSM_SE_BD1_08);
                mb1ev_TumikiArmyTimer = 0;
                obj->work[0]++;
            }
            break;
        
        case 7:
            if(++mb1ev_TumikiArmyTimer >= 120.0f) {
                if(obj->work[3]) {
                    Hu3DModelKill(obj->work[3]);
                }
                omDelObj(mbObjMan, obj);
            }
            break;
    }
}

static void MB1_TumikiArmyPosFix(OMOBJ *obj)
{
    HSFOBJECT *hsfObj; //r31
    HU3DMODEL *modelP; //r29
    int modelId; //r28
    HSFDATA *hsf; //sp+0x8
    if(MBKillCheck()) {
        obj->work[0] = 1;
    }
    switch(obj->work[0]) {
        case 0:
            modelId = MBModelIdGet(obj->work[1]);
            modelP = &Hu3DData[modelId];
            hsf = modelP->hsf;
            hsfObj = MB1_ModelObjPtrGet(modelP, "army_b4");
            if(hsfObj != NULL) {
                if(++hsfObj->mesh.base.rot.z >= 360) {
                    hsfObj->mesh.base.rot.x -= 360;
                }
            }
            hsfObj = MB1_ModelObjPtrGet(modelP, "army_a4");
            if(hsfObj != NULL) {
                if(++hsfObj->mesh.base.rot.z >= 360) {
                    hsfObj->mesh.base.rot.x -= 360;
                }
            }
            break;
        
        case 1:
            omDelObj(mbObjMan, obj);
            break;
    }
    
}

static void MB1_TrainObjUpdate(OMOBJ *obj)
{
    float time;
    float t;
    HuVecF pos;
    if(MBKillCheck()) {
        omDelObj(mbObjMan, obj);
        return;
    }
    time = MB1_TrainTimeGet();
    if(obj->work[1]) {
        switch(obj->work[0]) {
            case 0:
                MB1_TrainTimeSet(0.9f);
                MB1_TrainSpeedSet(1);
                MB1_TrainCoinSpeedSet(1);
                obj->work[0]++;
                obj->work[2] = 0;
                break;
            
            case 1:
                mb1_Work.trainMode = 0;
                if(time >= 1 || time < 0.5f) {
                    t = (++obj->work[2]/30.0f);
                    MB1_TrainSpeedSet(1-t);
                    MB1_TrainCoinSpeedSet(1-t);
                    if(obj->work[2] >= 30.0f) {
                        if(mb1_Work.trainSeNo1 >= 0) {
                            MBModelPosGet(mb1_Work.trainMdlId[0], &pos);
                            HuAudFXEmiterPlay(MSM_SE_BD1_15, &pos);
                            MBAudFXKill(mb1_Work.trainSeNo1);
                            mb1_Work.trainSeNo1 = MSM_SENO_NONE;
                        }
                        MB1_TrainSpeedSet(0);
                        MB1_TrainCoinSpeedSet(0);
                        obj->work[0]++;
                        obj->work[2] = 0;
                    }
                }
                break;
            
            case 3:
                mb1_Work.trainMode = 2;
                t = (++obj->work[2]/30.0f);
                MB1_TrainSpeedSet(t);
                MB1_TrainCoinSpeedSet(t);
                if(obj->work[2] >= 30.0f) {
                    MB1_TrainSpeedSet(1);
                    MB1_TrainCoinSpeedSet(1);
                    obj->work[0]++;
                    obj->work[2] = 0;
                }
                break;
            
            case 4:
                MB1_TrainSpeedSet(1);
                MB1_TrainCoinSpeedSet(1);
                if(time >= 0.05f && time < 0.06f) {
                    MB1_HasiOpen(0);
                }
                if(time >= 0.06f && time < 0.07f) {
                    MB1_HasiClose(0);
                }
                if(time > 0.1f && time < 0.45f) {
                    obj->work[3] = 1;
                } else {
                    obj->work[3] = 0;
                }
                if(time >= 0.49f) {
                    obj->work[0]++;
                    obj->work[2] = 0;
                }
                break;
            
            case 5:
                mb1_Work.trainMode = 0;
                t = (++obj->work[2]/30.0f);
                MB1_TrainSpeedSet(1-t);
                MB1_TrainCoinSpeedSet(1-t);
                if(obj->work[2] >= 30.0f) {
                    if(mb1_Work.trainSeNo1 >= 0) {
                        MBModelPosGet(mb1_Work.trainMdlId[0], &pos);
                        HuAudFXEmiterPlay(MSM_SE_BD1_15, &pos);
                        MBAudFXKill(mb1_Work.trainSeNo1);
                        mb1_Work.trainSeNo1 = MSM_SENO_NONE;
                    }
                    MB1_TrainSpeedSet(0);
                    MB1_TrainCoinSpeedSet(0);
                    obj->work[0]++;
                }
                break;
            
            case 7:
                mb1_Work.trainMode = 1;
                MBModelPosGet(mb1_Work.trainMdlId[0], &pos);
                mb1_Work.trainSeNo1 = MBAudFXEmiterCreate(MSM_SE_BD1_02, &pos, FALSE);
                MBAudFXRefSet(MSM_SE_BD1_02, &mb1_Work.trainSeNo1);
                MB1_TrainSpeedSet(1);
                MB1_TrainCoinSpeedSet(1);
                omDelObj(mbObjMan, obj);
                break;
        }
    } else {
        switch(obj->work[0]) {
            case 0:
                MB1_TrainTimeSet(0.4f);
                MB1_TrainSpeedSet(1);
                MB1_TrainCoinSpeedSet(1);
                obj->work[0]++;
                obj->work[2] = 0;
                break;
            
            case 1:
                mb1_Work.trainMode = 0;
                if(time >= 0.5f) {
                    t = (++obj->work[2]/30.0f);
                    MB1_TrainSpeedSet(1-t);
                    MB1_TrainCoinSpeedSet(1-t);
                    if(obj->work[2] >= 30.0f) {
                        if(mb1_Work.trainSeNo1 >= 0) {
                            MBModelPosGet(mb1_Work.trainMdlId[0], &pos);
                            HuAudFXEmiterPlay(MSM_SE_BD1_15, &pos);
                            MBAudFXKill(mb1_Work.trainSeNo1);
                            mb1_Work.trainSeNo1 = MSM_SENO_NONE;
                        }
                        MB1_TrainSpeedSet(0);
                        MB1_TrainCoinSpeedSet(0);
                        obj->work[0]++;
                        obj->work[2] = 0;
                    }
                }
                break;
            
            case 3:
                mb1_Work.trainMode = 2;
                t = (++obj->work[2]/30.0f);
                MB1_TrainSpeedSet(t);
                MB1_TrainCoinSpeedSet(t);
                if(obj->work[2] >= 30.0f) {
                    MB1_TrainSpeedSet(1);
                    MB1_TrainCoinSpeedSet(1);
                    obj->work[0]++;
                    obj->work[2] = 0;
                }
                break;
            
            case 4:
                MB1_TrainSpeedSet(1);
                MB1_TrainCoinSpeedSet(1);
                if(time >= 0.9f && time < 0.91f) {
                    MB1_HasiOpen(1);
                }
                if(time >= 0.91f && time < 0.92f) {
                    MB1_HasiClose(1);
                }
                if(time > 0.55f && time < 0.9f) {
                    obj->work[3] = 1;
                } else {
                    obj->work[3] = 0;
                }
                if(time >= 0.99f || time < 0.1f) {
                    obj->work[0]++;
                    obj->work[2] = 0;
                }
                break;
            
            case 5:
                mb1_Work.trainMode = 0;
                t = (++obj->work[2]/30.0f);
                MB1_TrainSpeedSet(1-t);
                MB1_TrainCoinSpeedSet(1-t);
                if(obj->work[2] >= 30.0f) {
                    if(mb1_Work.trainSeNo1 >= 0) {
                        MBModelPosGet(mb1_Work.trainMdlId[0], &pos);
                        HuAudFXEmiterPlay(MSM_SE_BD1_15, &pos);
                        MBAudFXKill(mb1_Work.trainSeNo1);
                        mb1_Work.trainSeNo1 = MSM_SENO_NONE;
                    }
                    MB1_TrainSpeedSet(0);
                    MB1_TrainCoinSpeedSet(0);
                    obj->work[0]++;
                }
                break;
            
            case 7:
                mb1_Work.trainMode = 1;
                MBModelPosGet(mb1_Work.trainMdlId[0], &pos);
                mb1_Work.trainSeNo1 = MBAudFXEmiterCreate(MSM_SE_BD1_02, &pos, FALSE);
                MBAudFXRefSet(MSM_SE_BD1_02, &mb1_Work.trainSeNo1);
                MB1_TrainSpeedSet(1);
                MB1_TrainCoinSpeedSet(1);
                omDelObj(mbObjMan, obj);
                break;
                
        }
    }
}

float MB1_AngleDiff(float a, float b)
{
    float delta;
    if(a >= 360.0f) {
        a -= 360.0f;
    } else if(a < 0.0f) {
        a += 360.0f;
    }
    if(b >= 360.0f) {
        b -= 360.0f;
    } else if(b < 0.0f) {
        b += 360.0f;
    }
    delta = a-b;
    if(delta <= -180.0f) {
        delta += 360.0f;
    } else if(delta >= 180.0f) {
        delta -= 360.0f;
    }
    return delta;
}

float MB1_AngleStep(float a, float b, float step)
{
    float ret;
    float delta;
    
    if(a >= 360.0) {
        a -= 360.0;
    } else if(a < 0.0) {
        a += 360.0;
    }
    if(b >= 360.0) {
        b -= 360.0;
    } else if(b < 0.0) {
        b += 360.0;
    }
    delta = 360.0+(a-b);
    if(fabs(delta) >= 360.0) {
        delta = fmod(delta, 360);
    }
    if(delta < 180.0) {
        if(delta <= step) {
            ret = delta;
        } else {
            ret = step;
        }
    } else if(360.0-delta <= step) {
        ret = -(360.0-delta);
    } else {
        ret = -step;
    }
    ret += b;
    if(ret >= 360.0) {
        ret -= 360.0;
    } else if(ret < 0.0) {
        ret += 360.0;
    }
    return ret;
}

float MB1_AngleLerp(float t, float a, float b)
{
    float delta = MB1_AngleDiff(b, a);
    return MB1_AngleStep(b, a, fabs(delta*t));
}

void MB1_ModelMtxSet(MBMODELID mdlId, Mtx *mtx)
{
    HU3DMODEL *modelP;
    int id;
    id = MBModelIdGet(mdlId);
    modelP = &Hu3DData[id];
    MTXCopy(*mtx, modelP->mtx);
    
}

void MB1_PlayerMtxSet(s16 playerNo, Mtx *mtx)
{
    MB1_ModelMtxSet(MBPlayerModelGet(playerNo), mtx);
}

float MB1_BezierCalc(float t, float a, float b, float c)
{
    float invT = 1.0-t;
    float ret = (a*(invT*invT))+(2.0*(b*(invT*t)))+(c*(t*t));
    return ret;
}

void MB1_BezierCalcV(float t, float *a, float *b, float *c, float *out)
{
    int i;
    for(i=0; i<3;i++) {
        *out++ = MB1_BezierCalc(t, *a++, *b++, *c++);
    }
}

void MB1_HermiteCoefGet(float t, float *p0Coef, float *p1Coef, float *m0Coef, float *m1Coef)
{
    float t2 = t*t;
    float t3 = t*t2;
    float coef = ((3.0*t2)-t3)-t3;
    *p0Coef = 1.0-coef;
    *p1Coef = coef;
    *m0Coef = t+((t3-t2)-t2);
    *m1Coef = t3-t2;
}

float MB1_HermiteEval(float t, float p0, float p1, float p2, float p3, float d)
{
    float pDelta;
    
    float p0Coef;
    float p1Coef;
    float m0Coef;
    float m1Coef;
    float ret;
    float d1;
    float d2;
    float m0;
    float m1;
    int pDeltai;
        
    pDelta = p2-p1;
    pDeltai = p2-p1;
    if(p1 == p2) {
        m0 = pDelta;
    } else {
        d1 = d;
        m0 = d1*(pDelta+(p1-p0));
    }
    if(p1 == p3) {
        m1 = pDelta;
    } else {
        d2 = d;
        m1 = d2*(pDelta+(p3-p2));
    }
    MB1_HermiteCoefGet(t, &p0Coef, &p1Coef, &m0Coef, &m1Coef);
    return ret = (p0Coef*p1)+(p1Coef*p2)+(m0Coef*m0)+(m1Coef*m1);
}

void MB1_HermiteCalcV(float t, HuVecF *p0, HuVecF *p1, HuVecF *p2, HuVecF *p3, HuVecF *out)
{
    float d = 0.5f;
    out->x = MB1_HermiteEval(t, p0->x, p1->x, p2->x, p3->x, d);
    out->y = MB1_HermiteEval(t, p0->y, p1->y, p2->y, p3->y, d);
    out->z = MB1_HermiteEval(t, p0->z, p1->z, p2->z, p3->z, d);
}

float MB1_HermiteSlopeCalc(float t, float p0, float p1, float p2, float p3)
{
    float t2 = t*t;
    float p0Coef = (6.0f*t2)-(6.0f*t);
    float p1Coef = (-6.0f*t2)-(6.0f*t);
    float m0Coef = 1+((3.0f*t2)-(4.0f*t));
    float m1Coef = (3.0f*t2)-(2.0f*t);
    return (p0Coef*p0)+(p1Coef*p1)+(m0Coef*p2)+(m1Coef*p3);
}

float MB1_PathTimeGetEnd(float t, HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d)
{
    return MB1_PathTimeGet(1, a, b, c, d);
}

float MB1_QuadCalc(float t, float p0, float p1, float p2, float p3)
{
    float mid1 = 0.5f*((p2+(p1-p0))-p1);
    float mid2 = 0.5f*((p2+(p1-p0))-p1);
    float a = mid1+((2.0f*p1)-(2.0f*p2))+mid2;
    float b = (((-3.0f*p1)+(3.0f*p2))-(2.0f*mid1))-mid2;
    float c = mid1;
    return (t*((3.0f*a)*t))+(2.0f*b*t)+c;
}

float MB1_QuadCalcMag(float t, HuVecF *p0, HuVecF *p1, HuVecF *p2, HuVecF *p3)
{
    float x = MB1_QuadCalc(t, p0->x, p1->x, p2->x, p3->x);
    float y = MB1_QuadCalc(t, p0->y, p1->y, p2->y, p3->y);
    float z = MB1_QuadCalc(t, p0->z, p1->z, p2->z, p3->z);
    return sqrtf((x*x)+(y*y)+(z*z));
}

float MB1_PathTimeGet(float time, HuVecF *p0, HuVecF *p1, HuVecF *p2, HuVecF *p3)
{
    float min = 0;
    float max = time;
    int numStep = 16;
    float stepHalfW = (max-min)/numStep;
    float stepSize = stepHalfW+stepHalfW;
    float totalLen = MB1_QuadCalcMag(min, p0, p1, p2, p3)+MB1_QuadCalcMag(max, p0, p1, p2, p3);
    int i;
    float t;
    for(i=1, t=min+stepHalfW; i<numStep; i += 2, t += stepSize) {
        totalLen += MB1_QuadCalcMag(t, p0, p1, p2, p3)*4;
    }
    for(i=2, t=min+stepSize; i<numStep; i += 2, t += stepSize) {
        totalLen += MB1_QuadCalcMag(t, p0, p1, p2, p3)*2;
    }
    return (stepHalfW/3)*totalLen;
}

void MB1_QuadCalcRot(float t, HuVecF *p0, HuVecF *p1, HuVecF *p2, HuVecF *p3, HuVecF *out)
{
    HuVecF dir;
    dir.x = MB1_QuadCalc(t, p0->x, p1->x, p2->x, p3->x);
    dir.y = MB1_QuadCalc(t, p0->y, p1->y, p2->y, p3->y);
    dir.z = MB1_QuadCalc(t, p0->z, p1->z, p2->z, p3->z);
    
    VECNormalize(&dir, &dir);
    out->x = HuAtan(-dir.y, HuMagXZVecF(&dir));
    out->y = HuAtan(dir.x, dir.z);
    out->z = 0;
}

static HSFOBJECT *objCall(HU3DMODEL *modelP, char *name, HSFOBJECT *obj);

HSFOBJECT *MB1_ModelObjPtrGet(HU3DMODEL *modelP, char *name)
{
    HSFDATA *hsf =  modelP->hsf;
    return objCall(modelP, name, hsf->root);
}

static HSFOBJECT *objCall(HU3DMODEL *modelP, char *name, HSFOBJECT *obj)
{
    int i;
    HSFOBJECT *meshObj;
    HSFOBJECT *replicaObj;
    HSFOBJECT *ret;
    
    switch(obj->type) {
        case HSF_OBJ_NULL1:
        case HSF_OBJ_MESH:
        case HSF_OBJ_ROOT:
        case HSF_OBJ_JOINT:
        case HSF_OBJ_NULL2:
        case HSF_OBJ_NULL3:
        case HSF_OBJ_MAP:
        {
            meshObj = obj;
            if(!strcmp(name, meshObj->name)) {
                return meshObj;
            }
            for(i=0; i<meshObj->mesh.childNum; i++) {
                ret = objCall(modelP, name, meshObj->mesh.child[i]);
                if(ret) {
                    return ret;
                }
            }
        }
            break;
        
        case HSF_OBJ_REPLICA:
        {
            replicaObj = obj;
            if(!strcmp(name, replicaObj->name)) {
                return replicaObj;
            }
            for(i=0; i<replicaObj->mesh.childNum; i++) {
                ret = objCall(modelP, name, replicaObj->mesh.child[i]);
                if(ret) {
                    return ret;
                }
            }
        }
        break;
    }
    return NULL;
}

static s16 PathStarCheck(s16 masuId, s16 len);

s16 MB1_StarPathLenGet(s16 masuId, s16 len)
{
    mb1_StarPathMasuNo = len;
    mb1_StarPathLen = len;
    mb1_StarPathValidF = FALSE;
    PathStarCheck(masuId, len);
    if(mb1_StarPathValidF) {
        return mb1_StarPathLen;
    } else {
        return -1;
    }
}

static s16 PathStarCheck(s16 masuId, s16 len)
{
    s16 newLen;
    s16 result = FALSE;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    s16 i;
    if(MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId) == MASU_TYPE_STAR) {
        newLen = mb1_StarPathMasuNo-len;
        if(newLen < mb1_StarPathLen) {
            mb1_StarPathLen = newLen;
        }
        mb1_StarPathValidF = TRUE;
        return TRUE;
    } else {
        if(MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId) != MASU_TYPE_NONE &&MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId) != MASU_TYPE_STAR) {
            len--;
        }
        if(len >= 0) {
            for(i=0; i<masuP->linkNum; i++) {
                if(!(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & (MBBranchFlagGet()|(MASU_FLAG_BLOCKL|MASU_FLAG_BLOCKR)))) {
                    if(PathStarCheck(masuP->linkTbl[i], len)) {
                        result = TRUE;
                    }
                }
            }
        }
        
        if(result) {
            return TRUE;
        } else{
            return FALSE;
        }
    }
}

void MB1_TrainCoinUpdate(OMOBJ *obj);

#define MB1_TRAIN_COIN_MAX 32

typedef struct MB1_TrainCoin_s {
    int mdlId; //0x00
    BOOL enableF; //0x04
    int mode; //0x08
    float gravity; //0x0C
    int maxTime;  //0x10
    int time; //0x14
    HuVecF pos; //0x18
    HuVecF rot; //0x24
    HuVecF scale; //0x30
    HuVecF srcPos; //0x3C
    HuVecF dstPos; //0x48
    HuVecF vel; //0x54
    Mtx *srcMtx; //0x60
    Mtx *dstMtx; //0x64
} MB1_TRAIN_COIN;

void MB1_TrainCoinInit(void)
{
    MB1_TRAIN_COIN *coin; //r31
    int i; //r30
    OMOBJ *obj; //r29
    obj = mb1_CoinObj = MBAddObj(32768, 0, 0, MB1_TrainCoinUpdate);
    coin = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MB1_TRAIN_COIN)*MB1_TRAIN_COIN_MAX, HU_MEMNUM_OVL);
    memset(coin, 0, sizeof(MB1_TRAIN_COIN)*MB1_TRAIN_COIN_MAX);
    for(i=0; i<MB1_TRAIN_COIN_MAX;i++, coin++) {
        coin->mdlId = Hu3DModelCreateData(BOARD_HSF_coin);
        Hu3DModelCameraSet(coin->mdlId, HU3D_CAM0);
        Hu3DModelDispOff(coin->mdlId);
        coin->enableF = FALSE;
        coin->mode = 0;
        coin->gravity = 0;
        coin->maxTime = coin->time = 0;
        coin->srcMtx = coin->dstMtx = NULL;
    }
}

void MB1_TrainCoinUpdate(OMOBJ *obj)
{
    MB1_TRAIN_COIN *coin; //r31
    int j; //r30
    int i; //r29
    BOOL coinGrabF; //r28
    BOOL coinDisableF; //r27
    
    float angle; //f30
    float t; //f29
    float intensity; //f27
    float angleX; //f26
    
    HuVecF effectPos; //sp+0x68
    HuVecF srcPos; //sp+0x5C
    HuVecF dstPos; //sp+0x50
    HuVecF pos; //sp+0x44
    HuVecF playerPos; //sp+0x38
    HuVecF effectVel; //sp+0x2C
    GXColor color; //sp+0x10
    
    coin = obj->data;
    if(MBKillCheck() || mb1_CoinObj == NULL) {
        for(i=0; i<MB1_TRAIN_COIN_MAX; i++, coin++) {
            Hu3DModelKill(coin->mdlId);
            coin->mdlId = HU3D_MODELID_NONE;
        }
        omDelObj(mbObjMan, obj);
        mb1_CoinObj = NULL;
        return;
    }
    for(i=0; i<MB1_TRAIN_COIN_MAX; i++, coin++) {
        if(!coin->enableF) {
            continue;
        }
        coinDisableF = FALSE;
        coinGrabF = FALSE;
        switch(coin->mode) {
            case 0:
                t = (++coin->time/(float)coin->maxTime);
                MTXMultVec(*coin->srcMtx, &coin->srcPos, &srcPos);
                MTXMultVec(*coin->dstMtx, &coin->dstPos, &dstPos);
                VECSubtract(&dstPos, &srcPos, &pos);
                VECScale(&pos, &pos, t);
                VECAdd(&pos, &srcPos, &pos);
                pos.y += coin->gravity*HuSin(t*180);
                coin->pos = pos;
                coin->rot.y += 10;
                if(t >= 1) {
                    coin->mode++;
                    coin->time = 0;
                }
                break;
            
            case 1:
                MTXMultVec(*coin->dstMtx, &coin->dstPos, &coin->pos);
                coin->rot.y += 10;
                MTXMultVec(*coin->srcMtx, &coin->srcPos, &srcPos);
                MTXMultVec(*coin->dstMtx, &coin->dstPos, &dstPos);
                VECSubtract(&dstPos, &srcPos, &pos);
                VECNormalize(&pos, &pos);
                angle = HuAtan(pos.x, pos.z);
                if(frand() & 0x1) {
                    angle += MBCapsuleEffRandF()*30;
                } else {
                    angle -= MBCapsuleEffRandF()*30;
                }
                coin->vel.x = (HuSin(angle)*100)*0.2f;
                coin->vel.y = 20;
                coin->vel.z = (HuCos(angle)*100)*0.2f;
                coin->mode++;
                coin->time = 0;
                break;
            
            case 2:
                VECAdd(&coin->pos, &coin->vel, &coin->pos);
                coin->vel.y -= 3.266666715343794;
                coin->rot.y += 10;
                t = (++coin->time/30.0f);
                Hu3DModelTPLvlSet(coin->mdlId, 1-t);
                if(++coin->time >= 30.0f) {
                    coin->mode = 0;
                    coin->time = 0;
                    coinDisableF = TRUE;
                }
                break;
        }
        if(coin->mode <= 2) {
            for(j=0; j<GW_PLAYER_MAX; j++) {
                if(!MBPlayerAliveCheck(j)) {
                    continue;
                }
                MBPlayerPosGet(j, &playerPos);
                VECSubtract(&coin->pos, &playerPos, &pos);
                if(coin->pos.y >= playerPos.y && coin->pos.y <= playerPos.y+150 && HuMagXZVecF(&pos) < 100.0f) {
                    MBPlayerCoinAdd(j, 1);
                    MBAudFXPlay(MSM_SE_CMN_08);
                    coinGrabF = TRUE;
                }
            }
        }
        Hu3DModelPosSet(coin->mdlId, coin->pos.x, coin->pos.y, coin->pos.z);
        Hu3DModelRotSet(coin->mdlId, coin->rot.x, coin->rot.y, coin->rot.z);
        Hu3DModelScaleSet(coin->mdlId, coin->scale.x, coin->scale.y, coin->scale.z);
        if(coinGrabF || coinDisableF) { 
            coin->enableF = FALSE;
            Hu3DModelDispOff(coin->mdlId);
            if(coinGrabF) {
                for(j=0, angle=0; j<32; j++) {
                    angle += (MBCapsuleEffRandF()+1)*22.5f;
                    effectPos = coin->pos;
                    t = (MBCapsuleEffRandF()+1)*2.5f;
                    angleX = 45-(MBCapsuleEffRandF()*90);
                    effectVel.x = HuSin(angle)*HuCos(angleX)*t;
                    effectVel.y = HuCos(angle)*HuCos(angleX)*t;
                    effectVel.z = t*HuSin(angleX);
                    intensity = MBCapsuleEffRandF();
                    color.r = 192+(intensity*63);
                    color.g = 192+(intensity*63);
                    color.b = 64+(intensity*63);
                    color.a = 192+(MBCapsuleEffRandF()*63);
                    MB1_CoinEffCreate(effectPos, effectVel,
                        ((MBCapsuleEffRandF()*0.1f)+0.5f)*100,
                        (MBCapsuleEffRandF()+1.0f)*(1.0f/60.0f),
                        0,
                        color);
                }
                break;
            }
        }
    }
}

void MB1_TrainCoinClose(void)
{
    mb1_CoinObj = NULL;
}

int MB1_TrainCoinNumGet(void)
{
    MB1_TRAIN_COIN *coin;
    int i;
    int num;
    OMOBJ *obj;
    obj = mb1_CoinObj;
    num = 0;
    if(mb1_CoinObj == NULL) {
        return 0;
    }
    for(coin=obj->data, i=0; i<MB1_TRAIN_COIN_MAX; i++, coin++) {
        if(coin->enableF) {
            num++;
        }
    }
    return num;
}

int MB1_TrainCoinCreate(Mtx *srcMtx, HuVecF *srcPos, Mtx *dstMtx, HuVecF *dstPos, float gravity, int maxTime)
{
    MB1_TRAIN_COIN *coin; //r31
    int i; //r30
    OMOBJ *obj; //r29
    int temp; //sp+0x18
    obj = mb1_CoinObj;
    temp =0;
    if(mb1_CoinObj == NULL) {
        return -1;
    }
    for(coin=obj->data, i=0; i<MB1_TRAIN_COIN_MAX; i++, coin++) {
        if(!coin->enableF) {
            break;
        }
    }
    if(i >= MB1_TRAIN_COIN_MAX) {
        return -1;
    }
    coin->enableF  = TRUE;
    coin->mode = 0;
    coin->gravity = gravity;
    coin->maxTime = maxTime;
    coin->time = 0;
    coin->srcPos = *srcPos;
    coin->dstPos = *dstPos;
    coin->srcMtx = srcMtx;
    coin->dstMtx = dstMtx;
    MTXMultVec(*coin->srcMtx, &coin->srcPos, &coin->pos);
    coin->rot.x = coin->rot.y = coin->rot.z = 0;
    coin->scale.x = coin->scale.y = coin->scale.z = 1;
    Hu3DModelPosSet(coin->mdlId, coin->pos.x, coin->pos.y, coin->pos.z);
    Hu3DModelRotSet(coin->mdlId, coin->rot.x, coin->rot.y, coin->rot.z);
    Hu3DModelScaleSet(coin->mdlId, coin->scale.x, coin->scale.y, coin->scale.z);
    Hu3DModelTPLvlSet(coin->mdlId, 1);
    Hu3DModelDispOn(coin->mdlId);
    return i;
}

int MB1_TrainCoinSearch(HuVecF *pos, HuVecF *out)
{
    MB1_TRAIN_COIN *coin; //r31
    int i; //r30
    OMOBJ *obj; //r29
    int bestCoin; //r28
    float dist; //f31
    float bestDist; //f30
    HuVecF dir;
    
    obj = mb1_CoinObj;
    if(mb1_CoinObj == NULL) {
        return -1;
    }
    for(coin=obj->data, bestCoin=-1, bestDist=10000, i=0; i<MB1_TRAIN_COIN_MAX; i++, coin++) {
        if(!coin->enableF) {
            continue;
        }
        VECSubtract(&coin->pos, pos, &dir);
        dist = VECMag(&dir);
        if(dist < bestDist) {
            bestCoin = i;
            bestDist = dist;
            *out = coin->pos;
        }
    }
    return bestCoin;
}

#define MB1_PARTICLE_ATTR_NONE 0
#define MB1_PARTICLE_ATTR_COUNTER_RESET (1 << 0)
#define MB1_PARTICLE_ATTR_COUNTER_UPDATE (1 << 1)

#define MB1_PARTICLE_DISPATTR_NONE 0
#define MB1_PARTICLE_DISPATTR_ZBUF_OFF (1 << 0)
#define MB1_PARTICLE_DISPATTR_NOANIM (1 << 1)
#define MB1_PARTICLE_DISPATTR_CAMERA_ROT (1 << 2)
#define MB1_PARTICLE_DISPATTR_ROT3D (1 << 3)
#define MB1_PARTICLE_DISPATTR_ALL 15

typedef struct MB1Particle_s MB1_PARTICLE;
typedef void (*MB1_PARTICLE_HOOK)(HU3DMODEL *modelP, MB1_PARTICLE *particleP, Mtx *matrix);

typedef struct MB1ParticleData_s {
    s16 time; //0x00
    s16 work; //0x02
    s16 mode; //0x04
    s16 cameraBit; //0x06
    HuVecF vel; //0x08
    float baseScale; //0x14
    float scaleFactor; //0x18
    float speed; //0x1C
    float unk20; //0x20
    float gravity; //0x24
    float unk28; //0x28
    float animTime; //0x2C
    float animSpeed; //0x30
    float scale; //0x34
    HuVecF rot; //0x38
    HuVecF pos; //0x44
    GXColor color; //0x50
    int animNo; //0x54
} MB1_PARTICLE_DATA;

struct MB1Particle_s {
    s16 mode; //0x00
    s16 time; //0x02
    HuVecF vel; //0x04
    s16 work[8]; //0x10
    u8 blendMode; //0x20
    u8 attr; //0x21
    u8 dispAttr; //0x22
    u8 unk23;
    HU3DMODELID modelId; //0x24
    s16 num; //0x26
    u32 count; //0x28
    u32 prevCounter; //0x2C
    u32 prevCount; //0x30
    u32 dlSize; //0x34
    ANIMDATA *anim; //0x38
    MB1_PARTICLE_DATA *data; //0x3C
    HuVecF *vertex; //0x40
    HuVec2F *st; //0x44
    void *dl; //0x48
    MB1_PARTICLE_HOOK hook; //0x4C
    HU3DMODEL *hookMdlP; //0x50
};

HU3DMODELID MB1_ParticleCreate(ANIMDATA *animP, s16 num);

void MB1_ExplodeEffUpdate(OMOBJ *obj);

typedef struct MB1ExplodeEffWork_s {
    int mdlId;
    int num;
    ANIMDATA *animP;
} MB1_EXPLODE_EFF_WORK;

void MB1_ExplodeEffInit(void)
{
    MB1_EXPLODE_EFF_WORK *work; //r31
    OMOBJ *obj; //r30
    MB1_PARTICLE *particleP; //r29
    HU3DMODEL *modelP; //r28
    ANIMDATA *animP; //r27
    int mdlId;
    
    obj = mb1_ExplodeEffObj = MBAddObj(32768, 0, 0, MB1_ExplodeEffUpdate);
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MB1_EXPLODE_EFF_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(MB1_EXPLODE_EFF_WORK));
    work->animP = animP = HuSprAnimRead(HuDataReadNum(CAPSULE_ANM_effExplode, HU_MEMNUM_OVL));
    work->mdlId = mdlId = MB1_ParticleCreate(animP, 256);
    Hu3DModelLayerSet(work->mdlId, 3);
    work->num = 0;
    modelP = &Hu3DData[work->mdlId];
    particleP = modelP->hookData;
    particleP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
}

void MB1_ExplodeEffUpdate(OMOBJ *obj)
{
    MB1_PARTICLE_DATA *particleDataP; //r31
    MB1_EXPLODE_EFF_WORK *work; //r30
    MB1_PARTICLE *particleP; //r29
    int i; //r28
    HU3DMODEL *modelP; //r27
    work = obj->data;
    
    if(MBKillCheck() || mb1_ExplodeEffObj == NULL) {
        Hu3DModelKill(work->mdlId);
        work->mdlId = HU3D_MODELID_NONE;
        HuSprAnimKill(work->animP);
        work->animP = NULL;
        omDelObj(mbObjMan, obj);
        mb1_ExplodeEffObj = NULL;
        return;
    }
    if(work->num <= 0) {
        Hu3DModelDispOff(work->mdlId);
        return;
    }
    Hu3DModelDispOn(work->mdlId);
    modelP = &Hu3DData[work->mdlId];
    particleP = modelP->hookData;
    particleDataP = particleP->data;
    particleP->unk23 = 0;
    for(i=0; i<particleP->num; i++, particleDataP++) {
        if(particleDataP->scale <= 0.0f) {
            continue;
        }
        particleDataP->pos.x += particleDataP->vel.x;
        particleDataP->pos.y += particleDataP->vel.y;
        particleDataP->pos.z += particleDataP->vel.z;
        particleDataP->rot.z += particleDataP->speed;
        if(particleDataP->rot.z >= 360.0f) {
            particleDataP->rot.z -= 360.0f;
        }
        particleDataP->animNo = particleDataP->animTime;
        particleDataP->animTime += particleDataP->animSpeed;
        if(particleDataP->animNo >= 16) {
            particleDataP->animNo = 0;
            particleDataP->time = 0;
            particleDataP->scale = 0;
            work->num--;
        }
    }
}

void MB1_ExplodeEffClose(void)
{
    mb1_ExplodeEffObj = NULL;
}

int MB1_ExplodeEffNumGet(void)
{
    OMOBJ *obj;
    MB1_EXPLODE_EFF_WORK *work;
    obj = mb1_ExplodeEffObj;
    if(mb1_ExplodeEffObj == NULL) {
        return 0;
    } else {
        work = obj->data;
        return work->num;
    }
}

static int MB1_ExplodeEffCreate(HuVecF pos, HuVecF vel, float scale, float speed, float animSpeed, GXColor color)
{
    MB1_PARTICLE_DATA *particleDataP; //r31
    MB1_PARTICLE *particleP; //r30
    MB1_EXPLODE_EFF_WORK *work; //r28
    int i; //r29
    
    OMOBJ *obj; //r27
    HU3DMODEL *modelP; //r26
    
    obj = mb1_ExplodeEffObj;
    work = obj->data;
    modelP = &Hu3DData[work->mdlId];
    particleP = modelP->hookData;
    for(particleDataP=particleP->data, i=0; i<particleP->num; i++, particleDataP++) {
        if(particleDataP->scale <= 0.0f) {
            break;
        }
    }
    if(i >= particleP->num) {
        return -1;
    }
    particleDataP->time = particleDataP->work = 0;
    particleDataP->pos.x = pos.x;
    particleDataP->pos.y = pos.y;
    particleDataP->pos.z = pos.z;
    particleDataP->vel.x = vel.x;
    particleDataP->vel.y = vel.y;
    particleDataP->vel.z = vel.z;
    particleDataP->speed = speed;
    particleDataP->scale = scale;
    particleDataP->color = color;
    particleDataP->rot.z = 0;
    particleDataP->animNo = 0;
    particleDataP->time = 0;
    particleDataP->animTime = 0;
    particleDataP->animSpeed = animSpeed;
    work->num++;
    return i;
}

void MB1_ExplodeEffCreateN(HuVecF *pos, HuVecF *rot, int n)
{
    Mtx rotMtx;
    HuVecF effPos;
    HuVecF effVel;
    GXColor effColor;
    int i;
    int colorOfs;
    int angle;
    mtxRot(rotMtx, rot->x, rot->y, rot->z);
    for(i=0; i<n; i++) {
        effPos.x = pos->x;
        effPos.y = pos->y;
        effPos.z = pos->z;
        angle = (360.0f/n)*i;
        effVel.x = 100.0*HuSin(angle)*0.075f*((MBCapsuleEffRandF()*0.1f)+0.9f);
        effVel.y = 100.0*HuCos(angle)*0.075f*((MBCapsuleEffRandF()*0.1f)+0.9f);
        effVel.z = 0;
        MTXMultVec(rotMtx, &effVel, &effVel);
        colorOfs = MBCapsuleEffRandF()*63.0f;
        effColor.r = colorOfs+192;
        effColor.g = colorOfs+192;
        effColor.b = colorOfs+192;
        effColor.a = (MBCapsuleEffRandF()*63.0f)+192;
        MB1_ExplodeEffCreate(effPos, effVel, 200, 0.1f, 0.33f, effColor);
    }
}

void MB1_ExplodeDustCreate(HuVecF *pos, HuVecF *vel)
{
    HuVecF effPos;
    HuVecF effVel;
    GXColor effColor;
    int i;
    int colorOfs;
    int angle;
    for(i=0; i<2; i++) {
        effPos.x = pos->x+(((MBCapsuleEffRandF()-0.5f)*100)*0.5f);
        effPos.y = pos->y+(((MBCapsuleEffRandF()-0.5f)*100)*0.5f);
        effPos.z = pos->z+(((MBCapsuleEffRandF()-0.5f)*100)*0.5f);
        VECNormalize(vel, &effVel);
        VECScale(&effVel, &effVel, -10*((MBCapsuleEffRandF()*0.5f)+1));
        colorOfs = MBCapsuleEffRandF()*63.0f;
        effColor.r = colorOfs+192;
        effColor.g = colorOfs+192;
        effColor.b = colorOfs+192;
        effColor.a = (MBCapsuleEffRandF()*32.0f)+128;
        MB1_ExplodeEffCreate(effPos, effVel, 200, 0.1f, 0.33f, effColor);
    }
}

typedef struct MB1CoinEffWork_s {
    int mdlId;
    int num;
    ANIMDATA *animP;
} MB1_COIN_EFF_WORK;

void MB1_CoinEffUpdate(OMOBJ *obj);

void MB1_CoinEffInit(void)
{
    MB1_COIN_EFF_WORK *work; //r31
    OMOBJ *obj; //r30
    MB1_PARTICLE *particleP; //r29
    HU3DMODEL *modelP; //r28
    ANIMDATA *animP; //r27
    int mdlId;
    
    obj = mb1_CoinEffObj = MBAddObj(32768, 0, 0, MB1_CoinEffUpdate);
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MB1_COIN_EFF_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(MB1_COIN_EFF_WORK));
    work->animP = animP = HuSprAnimRead(HuDataReadNum(CAPSULE_ANM_effCoin, HU_MEMNUM_OVL));
    work->mdlId = mdlId = MB1_ParticleCreate(animP, 256);
    Hu3DModelLayerSet(work->mdlId, 3);
    work->num = 0;
    modelP = &Hu3DData[work->mdlId];
    particleP = modelP->hookData;
    particleP->blendMode = HU3D_PARTICLE_BLEND_ADDCOL;
    particleP->dispAttr = MB1_PARTICLE_DISPATTR_NOANIM;
}

void MB1_CoinEffUpdate(OMOBJ *obj)
{
    MB1_PARTICLE_DATA *particleDataP; //r31
    MB1_COIN_EFF_WORK *work; //r30
    MB1_PARTICLE *particleP; //r29
    int i; //r28
    HU3DMODEL *modelP; //r27
    work = obj->data;
    
    if(MBKillCheck() || mb1_CoinEffObj == NULL) {
        Hu3DModelKill(work->mdlId);
        work->mdlId = HU3D_MODELID_NONE;
        HuSprAnimKill(work->animP);
        work->animP = NULL;
        omDelObj(mbObjMan, obj);
        mb1_CoinEffObj = NULL;
        return;
    }
    if(work->num <= 0) {
        Hu3DModelDispOff(work->mdlId);
        return;
    }
    Hu3DModelDispOn(work->mdlId);
    modelP = &Hu3DData[work->mdlId];
    particleP = modelP->hookData;
    particleDataP = particleP->data;
    particleP->unk23 = 0;
    for(i=0; i<particleP->num; i++, particleDataP++) {
        if(particleDataP->scale <= 0.0f) {
            continue;
        }
        particleDataP->pos.x += particleDataP->vel.x;
        particleDataP->pos.y += particleDataP->vel.y;
        particleDataP->pos.z += particleDataP->vel.z;
        if(particleDataP->gravity) {
            particleDataP->vel.y -= particleDataP->gravity;
        }
        particleDataP->scale = particleDataP->baseScale*particleDataP->scaleFactor;
        particleDataP->scaleFactor -= particleDataP->speed;
        if(particleDataP->scaleFactor <= 0.0f) {
            particleDataP->animNo = 0;
            particleDataP->time = 0;
            particleDataP->scale = 0;
            work->num--;
        }
    }
}

void MB1_CoinEffClose(void)
{
    mb1_CoinEffObj = NULL;
}

int MB1_CoinEffNumGet(void)
{
    OMOBJ *obj;
    MB1_COIN_EFF_WORK *work;
    obj = mb1_CoinEffObj;
    if(mb1_CoinEffObj == NULL) {
        return 0;
    } else {
        work = obj->data;
        return work->num;
    }
}

void MB1_CoinEffBlendModeSet(int blendMode)
{
    OMOBJ *obj;
    MB1_COIN_EFF_WORK *work;
    MB1_PARTICLE *particleP;
    HU3DMODEL *modelP;
    obj = mb1_CoinEffObj;
    if(mb1_CoinEffObj == NULL) {
        return;
    } else {
        work = obj->data;
        modelP = &Hu3DData[work->mdlId];
        particleP = modelP->hookData;
        particleP->blendMode = blendMode;
    }
}

int MB1_CoinEffCreate(HuVecF pos, HuVecF vel, float scale, float speed, float gravity, GXColor color)
{
    MB1_PARTICLE_DATA *particleDataP; //r31
    MB1_PARTICLE *particleP; //r30
    int i; //r29
    MB1_COIN_EFF_WORK *work; //r26
    OMOBJ *obj; //r25
    HU3DMODEL *modelP; //r24
    if(mb1_CoinEffObj == NULL){
        return -1;
    }
    obj = mb1_CoinEffObj;
    work = obj->data;
    modelP = &Hu3DData[work->mdlId];
    particleP = modelP->hookData;
    for(particleDataP=particleP->data, i=0; i<particleP->num; i++, particleDataP++) {
        if(particleDataP->scale <= 0.0f) {
            break;
        }
    }
    if(i >= particleP->num) {
        return -1;
    }
    particleDataP->time = particleDataP->work = 0;
    particleDataP->pos.x = pos.x;
    particleDataP->pos.y = pos.y;
    particleDataP->pos.z = pos.z;
    particleDataP->vel.x = vel.x;
    particleDataP->vel.y = vel.y;
    particleDataP->vel.z = vel.z;
    
    particleDataP->baseScale = scale;
    particleDataP->scaleFactor = 1;
    particleDataP->speed = speed;
    particleDataP->gravity = gravity;
    particleDataP->scale = scale;
    particleDataP->color = color;
    particleDataP->rot.z = MBCapsuleEffRandF()*360;
    particleDataP->animNo = 0;
    particleDataP->time = 0;
    
    work->num++;
    return i;
}

static void ParticleDraw(HU3DMODEL *modelP, Mtx *mtx);

HU3DMODELID MB1_ParticleCreate(ANIMDATA *animP, s16 num)
{
    MB1_PARTICLE *effP;
    MB1_PARTICLE_DATA *effDataP;
    s16 i;
    HuVec2F *st;
    HU3DMODEL *modelP;
    HuVecF *vtx;
    HU3DMODELID modelId;
    void *dlBuf;
    void *dlBegin;
    modelId = Hu3DHookFuncCreate(ParticleDraw);
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    modelP = &Hu3DData[modelId];
    modelP->hookData = effP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(MB1_PARTICLE), modelP->mallocNo);
    effP->anim = animP;
    effP->num = num;
    effP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
    effP->dispAttr = MB1_PARTICLE_DISPATTR_NONE;
    effP->hook = NULL;
    effP->hookMdlP = NULL;
    effP->count = 0;
    effP->attr = MB1_PARTICLE_ATTR_NONE;
    effP->unk23 = 0;
    effP->prevCount = 0;
    effP->mode = effP->time = 0;
    effP->data = effDataP = HuMemDirectMallocNum(HEAP_MODEL, num*sizeof(MB1_PARTICLE_DATA), modelP->mallocNo);
    memset(effDataP, 0, num*sizeof(MB1_PARTICLE_DATA));
    for(i=0; i<num; i++, effDataP++) {
        effDataP->scale = 0;
        effDataP->rot.x = effDataP->rot.y = effDataP->rot.z = 0;
        effDataP->animTime = 0;
        effDataP->animSpeed = 1;
        effDataP->pos.x = ((frand() & 0x7F)-64)*20;
        effDataP->pos.y = ((frand() & 0x7F)-64)*30;
        effDataP->pos.z = ((frand() & 0x7F)-64)*20;
        effDataP->color.r = effDataP->color.g = effDataP->color.b = effDataP->color.a = 255;
        effDataP->animNo = 0;
    }
    effP->vertex = vtx = HuMemDirectMallocNum(HEAP_MODEL, num*sizeof(HuVecF)*4, modelP->mallocNo);
    for(i=0; i<num*4; i++, vtx++) {
        vtx->x = vtx->y = vtx->z = 0;
    }
    effP->st = st = HuMemDirectMallocNum(HEAP_MODEL, num*sizeof(HuVec2F)*4, modelP->mallocNo);
    for(i=0; i<num; i++) {
        st->x = 0;
        st->y = 0;
        st++;
        
        st->x = 1;
        st->y = 0;
        st++;
        
        st->x = 1;
        st->y = 1;
        st++;
        
        st->x = 0;
        st->y = 1;
        st++;
    }
    dlBegin = dlBuf = HuMemDirectMallocNum(HEAP_MODEL, 0x10000, modelP->mallocNo);
    DCFlushRange(dlBuf, 0x10000);
    GXBeginDisplayList(dlBegin, 0x10000);
    GXBegin(GX_QUADS, GX_VTXFMT0, num*4);
    for(i=0; i<num; i++) {
        GXPosition1x16(i*4);
        GXColor1x16(i);
        GXTexCoord1x16(i*4);
        
        GXPosition1x16((i*4)+1);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+1);
        
        GXPosition1x16((i*4)+2);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+2);
        
        GXPosition1x16((i*4)+3);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+3);
    }
    GXEnd();
    effP->dlSize = GXEndDisplayList();
    effP->dl = HuMemDirectMallocNum(HEAP_MODEL, effP->dlSize, modelP->mallocNo);
    memcpy(effP->dl, dlBuf, effP->dlSize);
    DCFlushRange(effP->dl, effP->dlSize);
    HuMemDirectFree(dlBuf);
    return modelId;
}

HU3DMODELID MB1_ParticleCopy(HU3DMODELID oldMdlId)
{
    MB1_PARTICLE *newParticleP; //r31
    MB1_PARTICLE_DATA *particleDataP; //r30
    HuVec2F *st; //r29
    s16 i; //r28
    HU3DMODEL *newModelP; //r27
    MB1_PARTICLE *oldParticleP; //r26
    HuVecF *pos; //r25
    int num; //r24
    HU3DMODEL *oldModelP; //r23
    HU3DMODELID newMdlId; //r22
    
    oldModelP = &Hu3DData[oldMdlId];
    oldParticleP = oldModelP->hookData;
    newMdlId = Hu3DHookFuncCreate(oldModelP->hookFunc);
    Hu3DModelCameraSet(newMdlId, HU3D_CAM0);
    newModelP = &Hu3DData[newMdlId];
    newModelP->hookData = newParticleP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(MB1_PARTICLE), newModelP->mallocNo);
    newParticleP->anim = oldParticleP->anim;
    newParticleP->num = oldParticleP->num;
    num = newParticleP->num;
    newParticleP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
    newParticleP->dispAttr = oldParticleP->dispAttr;
    newParticleP->hook = NULL;
    newParticleP->hookMdlP = NULL;
    newParticleP->count = 0;
    newParticleP->attr = MB1_PARTICLE_ATTR_NONE;
    newParticleP->prevCount = 0;
    newParticleP->mode = newParticleP->time = 0;
    newParticleP->data = particleDataP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(MB1_PARTICLE_DATA)*num, newModelP->mallocNo);
    for(i=0; i<num; i++, particleDataP++) {
        particleDataP->scale = 0;
        particleDataP->rot.x = particleDataP->rot.y = particleDataP->rot.z = 0;
        particleDataP->animTime = 0;
        particleDataP->animSpeed = 1;
        particleDataP->pos.x = ((frand()&0x7F)-64)*20;
        particleDataP->pos.y = ((frand()&0x7F)-64)*30;
        particleDataP->pos.z = ((frand()&0x7F)-64)*20;
        particleDataP->color.r = particleDataP->color.g = particleDataP->color.b = particleDataP->color.a = 255;
        particleDataP->animNo = 0;
    }
    newParticleP->vertex = pos = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HuVecF)*num*4, newModelP->mallocNo);
    for(i=0; i<num*4; i++, pos++) {
        pos->x = pos->y = pos->z = 0;
    }
    newParticleP->st = st = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HuVec2F)*num*4, newModelP->mallocNo);
    for(i=0; i<num; i++) {
        st->x = 0;
        st->y = 0;
        st++;
        st->x = 1;
        st->y = 0;
        st++;
        st->x = 1;
        st->y = 1;
        st++;
        st->x = 0;
        st->y = 1;
        st++;
    }
    newParticleP->dlSize = oldParticleP->dlSize;
    newParticleP->dl = oldParticleP->dl;
    return newMdlId;
}

static HuVecF basePos[] = {
    { -0.5f,  0.5f, 0.0f },
    {  0.5f,  0.5f, 0.0f },
    {  0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f }
};

static HuVec2F baseST[] = {
    { 0.0f, 0.0f },
    { 0.25f, 0.0f },
    { 0.25f, 0.25f },
    { 0.0f, 0.25f },
};
static void ParticleDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    MB1_PARTICLE *effP;
    MB1_PARTICLE_DATA *effDataP;
    HuVecF *vtx;
    HuVec2F *st;
    HuVecF *scaleVtxP;
    s16 i;
    s16 j;
    HuVecF *initVtxP;
    s16 bmpFmt;
    s16 row;
    s16 col;
    MB1_PARTICLE_HOOK hook;
    
    Mtx mtxInv;
    Mtx mtxPos;
    Mtx mtxRotZ;
    HuVecF scaleVtx[4];
    HuVecF finalVtx[4];
    HuVecF initVtx[4];
    ROMtx basePosMtx;
    
    effP = modelP->hookData;
    if(!shadowModelDrawF) {
        if(effP->hookMdlP && effP->hookMdlP != modelP){
            ParticleDraw(effP->hookMdlP, mtx);
        }
        GXLoadPosMtxImm(*mtx, GX_PNMTX0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        if(shadowModelDrawF) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        } else {
            bmpFmt = (effP->anim->bmp->dataFmt & 0xF);
            if(bmpFmt == ANIM_BMP_I8 || bmpFmt == ANIM_BMP_I4) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_ZERO);
            } else {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
            }
            if(effP->dispAttr & MB1_PARTICLE_DISPATTR_ZBUF_OFF) {
                GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
            } else if(modelP->attr & HU3D_ATTR_ZWRITE_OFF) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            } else {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
            }
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        HuSprTexLoad(effP->anim, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(GX_FALSE);
        switch (effP->blendMode) {
            case HU3D_PARTICLE_BLEND_NORMAL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_ADDCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_INVCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
        }
        if(HmfInverseMtxF3X3(*mtx, mtxInv) == FALSE) {
            PSMTXIdentity(mtxInv);
        }
        PSMTXReorder(mtxInv, basePosMtx);
        if(effP->hook) {
            hook = effP->hook;
            hook(modelP, effP, mtx);
        }
        effDataP = effP->data;
        vtx = effP->vertex;
        st = effP->st;
        if(effP->dispAttr & MB1_PARTICLE_DISPATTR_CAMERA_ROT) {
            MTXIdentity(mtxInv);
            MTXIdentity(*(Mtx *)(&basePosMtx));
            initVtx[0] = basePos[0];
            initVtx[1] = basePos[1];
            initVtx[2] = basePos[2];
            initVtx[3] = basePos[3];
        } else {
            PSMTXROMultVecArray(basePosMtx, &basePos[0], initVtx, 4);
        }
        for(i=0; i<effP->num; i++, effDataP++) {
            if(!effDataP->scale) {
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
            } else if(effP->dispAttr & MB1_PARTICLE_DISPATTR_ROT3D) {
                VECScale(&basePos[0], &scaleVtx[0], effDataP->scale);
                VECScale(&basePos[1], &scaleVtx[1], effDataP->scale);
                VECScale(&basePos[2], &scaleVtx[2], effDataP->scale);
                VECScale(&basePos[3], &scaleVtx[3], effDataP->scale);
                mtxRot(mtxPos, effDataP->rot.x, effDataP->rot.y, effDataP->rot.z);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);
            } else if(!effDataP->rot.z) {
                scaleVtxP = scaleVtx;
                initVtxP = initVtx;
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
            } else {
                VECScale(&basePos[0], &scaleVtx[0], effDataP->scale);
                VECScale(&basePos[1], &scaleVtx[1], effDataP->scale);
                VECScale(&basePos[2], &scaleVtx[2], effDataP->scale);
                VECScale(&basePos[3], &scaleVtx[3], effDataP->scale);
                MTXRotRad(mtxRotZ, 'Z', effDataP->rot.z);
                PSMTXConcat(mtxInv, mtxRotZ, mtxPos);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);

            }
        }
        effDataP = effP->data;
        st = effP->st;
        if(!(effP->dispAttr & MB1_PARTICLE_DISPATTR_NOANIM)) {
            for(i=0; i<effP->num; i++, effDataP++) {
                row = effDataP->animNo & 0x3;
                col = (effDataP->animNo >> 2) & 0x3;
                for(j=0; j<4; j++, st++) {
                    st->x = (0.25f*row)+baseST[j].x;
                    st->y = (0.25f*col)+baseST[j].y;
                }
            }
        } else {
            for(i=0; i<effP->num; i++, effDataP++) {
                for(j=0; j<4; j++, st++) {
                    st->x = 4*baseST[j].x;
                    st->y = 4*baseST[j].y;
                }
            }
        }
        DCFlushRangeNoSync(effP->vertex, effP->num*sizeof(HuVecF)*4);
        DCFlushRangeNoSync(effP->st, effP->num*sizeof(HuVec2F)*4);
        DCFlushRangeNoSync(effP->data, effP->num*sizeof(MB1_PARTICLE_DATA));
        PPCSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetArray(GX_VA_POS, effP->vertex, sizeof(HuVecF));
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetArray(GX_VA_CLR0, &effP->data->color, sizeof(MB1_PARTICLE_DATA));
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetArray(GX_VA_TEX0, effP->st, sizeof(HuVec2F));
        GXCallDisplayList(effP->dl, effP->dlSize);
        if(shadowModelDrawF == FALSE) {
            if(!(effP->attr & MB1_PARTICLE_ATTR_COUNTER_UPDATE)) {
                effP->count++;
            }
            if(effP->prevCount != 0 && effP->prevCount <= effP->count) {
                if(effP->attr & MB1_PARTICLE_ATTR_COUNTER_RESET) {
                    effP->count = 0;
                }
                effP->count = effP->prevCount;
            }
        }
    }
}

typedef struct MB1TrainInfo_s {
    int no;
    char *objName;
} MB1_TRAIN_INFO;

void MB1_TrainInit(void)
{
    int i;//r31
    MB1_TRAIN_POINT *trainPoint; //r30
    int indexA; //r29
    int indexD; //r28
    HuVecF *vtxBuf; //r27
    int indexC; //r26
    int j; //r25
    HU3DMODEL *modelP; //r24
    HuVecF *vtx; //r23
    int indexB; //r22
    int mdlId; //r21
    
    float t; //f28
    
    HuVecF pointA; //sp+0x40
    HuVecF pointB; //sp+0x34
    HuVecF pointC; //sp+0x28
    HuVecF pointD; //sp+0x1C
    HuVecF temp; //sp+0x10
    HSFDATA *hsf; //sp+0xC
    
    static MB1_TRAIN_INFO trainInfo[7] = {
        { 0, "train_a2" },
        { 0, "train_a3" },
        { 0, "train_a4" },
        { 1, "train_b2" },
        { 1, "train_b3" },
        { 2, "train_c2" },
        { 2, "train_c3" },
    };
    mb1_Work.trainPointNum = MB1EV_TRAINPATH_LEN*20;
    mb1_Work.trainMaxTime = 0;
    trainPoint = mb1_Work.trainPoint = HuMemDirectMallocNum(HEAP_HEAP, mb1_Work.trainPointNum*sizeof(MB1_TRAIN_POINT), HU_MEMNUM_OVL);
    memset(trainPoint, 0, mb1_Work.trainPointNum*sizeof(MB1_TRAIN_POINT));
    vtx = vtxBuf = HuMemDirectMallocNum(HEAP_HEAP, mb1_Work.trainPointNum*sizeof(HuVecF), HU_MEMNUM_OVL);
    memset(vtxBuf, 0, mb1_Work.trainPointNum*sizeof(HuVecF));
    for(i=0; i<MB1EV_TRAINPATH_LEN; i++) {
        indexA = i-1;
        if(indexA < 0) {
            indexA += MB1EV_TRAINPATH_LEN;
        }
        indexB = i;
        indexC = i+1;
        if(indexC >= MB1EV_TRAINPATH_LEN) {
            indexC -= MB1EV_TRAINPATH_LEN;
        }
        indexD = i+2;
        if(indexD >= MB1EV_TRAINPATH_LEN) {
            indexD -= MB1EV_TRAINPATH_LEN;
        }
        pointA.x = mb1ev_TrainPath[indexA].x;
        pointA.y = mb1ev_TrainPath[indexA].y;
        pointA.z = mb1ev_TrainPath[indexA].z;
        pointB.x = mb1ev_TrainPath[indexB].x;
        pointB.y = mb1ev_TrainPath[indexB].y;
        pointB.z = mb1ev_TrainPath[indexB].z;
        pointC.x = mb1ev_TrainPath[indexC].x;
        pointC.y = mb1ev_TrainPath[indexC].y;
        pointC.z = mb1ev_TrainPath[indexC].z;
        pointD.x = mb1ev_TrainPath[indexD].x;
        pointD.y = mb1ev_TrainPath[indexD].y;
        pointD.z = mb1ev_TrainPath[indexD].z;
        for(j=0; j<20; vtx++, j++) {
            t = j/20.0f;
            MB1_HermiteCalcV(t, &pointA, &pointB, &pointC, &pointD, vtx);
        }
    }
    for(i=0; i<mb1_Work.trainPointNum; i++, trainPoint++) {
        if(i < mb1_Work.trainPointNum-1) {
            indexC = i+1;
        } else {
            indexC = 0;
        }
        pointB = vtxBuf[i];
        pointC = vtxBuf[indexC];
        VECSubtract(&pointC, &pointB, &temp);
        indexA = i-1;
        if(indexA < 0) {
            indexA += mb1_Work.trainPointNum;
        }
        pointA = vtxBuf[indexA];
        indexD = i+2;
        if(indexD >= mb1_Work.trainPointNum) {
            indexD -= mb1_Work.trainPointNum;
        }
        pointD = vtxBuf[indexD];
        trainPoint->idx = i;
        trainPoint->nextIdx = indexC;
        trainPoint->endTime = MB1_PathTimeGetEnd(1, &pointA, &pointB, &pointC, &pointD);
        trainPoint->startTime = mb1_Work.trainMaxTime;
        trainPoint->pos = pointA;
        trainPoint->rot.x = HuAtan(-temp.y, HuMagXZVecF(&temp));
        trainPoint->rot.y  = HuAtan(temp.x, temp.z);
        mb1_Work.trainMaxTime += trainPoint->endTime;
    }
    HuMemDirectFree(vtxBuf);
    mb1_Work.trainDelay = ((MBCapsuleEffRandF()*0.4f)+1)*60;
    mb1_Work.trainMode = 1;
    mb1_Work.trainTime = 0;
    mb1_Work.trainSpeed = mb1_Work.trainMaxTime/1800;
    mb1_Work.trainMaxSpeed = mb1_Work.trainSpeed;
    mb1_Work.coinSpeed = 7.5f;
    temp.x = mb1ev_TrainPath[0].x;
    temp.y = mb1ev_TrainPath[0].y;
    temp.z = mb1ev_TrainPath[0].z;
    mb1_Work.trainSeNo1 = MBAudFXEmiterCreate(MSM_SE_BD1_02, &temp, FALSE);
    MBAudFXRefSet(MSM_SE_BD1_02, &mb1_Work.trainSeNo1);
    for(i=0; i<7;i++) {
        mdlId = MBModelIdGet(mb1_Work.trainMdlId[trainInfo[i].no]);
        modelP = &Hu3DData[mdlId];
        hsf = modelP->hsf;
        mb1_Work.trainObj[i] = MB1_ModelObjPtrGet(modelP, trainInfo[i].objName);
    }
}

int MB1_TrainPosGet(float time, HuVecF *pos, HuVecF *rot);

void MB1_TrainUpdate(void)
{
    int i; //r30
    HSFOBJECT *obj; //r28
    
    float intensity; //f31
    float t; //f30
    float angle; //f29
    
    float cRotX; //f28
    float sRotX; //f27
    float sRotY; //f26
    float cRotY; //f25
    
    Mtx trainMtx; //sp+0x5C
    HuVecF trainPos; //sp+0x50
    HuVecF trainRot; //sp+0x44
    HuVecF pos; //sp+0x38
    HuVecF vel; //sp+0x2C
    GXColor color; //sp+0x10
    int posIdx; //sp+0xC
    
    mb1_Work.trainTime += mb1_Work.trainSpeed;
    if(mb1_Work.trainTime > mb1_Work.trainMaxTime) {
        mb1_Work.trainTime -= mb1_Work.trainMaxTime;
    }
    posIdx = MB1_TrainPosGet(mb1_Work.trainTime, &trainPos, &trainRot);
    MBModelPosSetV(mb1_Work.trainMdlId[0], &trainPos);
    MBModelRotSetV(mb1_Work.trainMdlId[0], &trainRot);
    mb1_Work.trainRot[0] = trainRot;
    MB1_TrainPosGet(mb1_Work.trainTime-350, &trainPos, &trainRot);
    MBModelPosSetV(mb1_Work.trainMdlId[1], &trainPos);
    MBModelRotSetV(mb1_Work.trainMdlId[1], &trainRot);
    mb1_Work.trainRot[1] = trainRot;
    MB1_TrainPosGet(mb1_Work.trainTime-650, &trainPos, &trainRot);
    MBModelPosSetV(mb1_Work.trainMdlId[2], &trainPos);
    MBModelRotSetV(mb1_Work.trainMdlId[2], &trainRot);
    mb1_Work.trainRot[2] = trainRot;
    for(i=0; i<7; i++) {
        obj = mb1_Work.trainObj[i];
        if(obj != NULL) {
            if((obj->mesh.base.rot.x += mb1_Work.coinSpeed) >= 360) {
                obj->mesh.base.rot.x -= 360;
            }
        }
    }
    if(--mb1_Work.trainDelay <= 0 && mb1_Work.trainMode != 0) {
        Hu3DModelObjMtxGet(MBModelIdGet(mb1_Work.trainMdlId[0]), "coin_pos", trainMtx);
        switch(mb1_Work.trainMode) {
            case 1:
                mb1_Work.trainDelay = ((MBCapsuleEffRandF()*0.4f)+1.0f)*60;
                break;
            
            case 2:
                mb1_Work.trainDelay = ((MBCapsuleEffRandF()*0.2f)+0.5f)*60;
                break;
        }
        pos.x = trainMtx[0][3];
        pos.y = trainMtx[1][3];
        pos.z = trainMtx[2][3];
        trainMtx[0][3] = 0;
        trainMtx[1][3] = 0;
        trainMtx[2][3] = 0;
        sRotX = HuSin(-mb1_Work.trainRot[0].x);
        cRotX = HuCos(-mb1_Work.trainRot[0].x);
        sRotY = HuSin(mb1_Work.trainRot[0].y);
        cRotY = HuCos(mb1_Work.trainRot[0].y);
        t = mb1_Work.trainSpeed/mb1_Work.trainMaxSpeed;
        for(i=0; i<16; i++) {
            trainPos.x = pos.x;
            trainPos.y = pos.y;
            trainPos.z = pos.z;
            angle = i*22.5f;
            vel.x = ((HuSin(angle)*100)*0.022f)*((MBCapsuleEffRandF()*0.1f)+0.9f);
            vel.y = 7.5000005f*((MBCapsuleEffRandF()*0.1f)+0.9f);
            vel.z = ((HuCos(angle)*100)*0.022f)*((MBCapsuleEffRandF()*0.1f)+0.9f);
            MTXMultVec(trainMtx, &vel, &vel);
            vel.x += t*(10*(sRotY*cRotX));
            vel.y += (sRotX*10)*t;
            vel.z += t*(10*(cRotY*cRotX));
            intensity = MBCapsuleEffRandF()*63;
            color.r = intensity+192.0f;
            color.g = intensity+192.0f;
            color.b = intensity+192.0f;
            color.a = (MBCapsuleEffRandF()*63)+192.0f;
            MB1_ExplodeEffCreate(trainPos, vel, 100, 0.1f, 0.33f, color);
        }
    }
    if(mb1_Work.trainSeNo1 >= 0) {
        MBModelPosGet(mb1_Work.trainMdlId[0], &pos);
        MBAudFXEmiterUpdate(mb1_Work.trainSeNo1, &pos);
    }
}

void MB1_TrainClose(void)
{
    if(mb1_Work.trainPoint != NULL) {
        HuMemDirectFree(mb1_Work.trainPoint);
    }
    mb1_Work.trainPoint = NULL;
}

int MB1_TrainPosGet(float time, HuVecF *pos, HuVecF *rot)
{
    MB1_TRAIN_POINT *point; //r31
    int i; //r30
    MB1_TRAIN_POINT *nextPoint; //r28
    int idxPrev; //r27
    MB1_TRAIN_POINT *nextPoint2; //r26
    MB1_TRAIN_POINT *prevPoint; //r25
    int idxNext2; //r24
    
    
    float t; //f27
    
    HuVecF posCur; //sp+0x24
    HuVecF posNext; //sp+0x18
    HuVecF facePos; //sp+0xC
    
    point = mb1_Work.trainPoint;
    if(fabs(time) >= mb1_Work.trainMaxTime) {
        time = fmod(time, mb1_Work.trainMaxTime);
    }
    if(time < 0) {
        time += mb1_Work.trainMaxTime;
    }
    for(i=0; i<mb1_Work.trainPointNum; i++, point++) {
        if(time >= point->startTime && time < point->startTime+point->endTime) {
            break;
        }
    }
    if(i >= mb1_Work.trainPointNum) {
        point = mb1_Work.trainPoint;
        time = 0;
    }
    nextPoint = &mb1_Work.trainPoint[point->nextIdx];
    t = (time-point->startTime)/point->endTime;
    posCur = point->pos;
    posNext = nextPoint->pos;
    idxNext2 = nextPoint->nextIdx;
    if(i <= 0) {
        idxPrev = mb1_Work.trainPointNum-1;
    } else {
        idxPrev = i-1;
    }
    nextPoint2 = &mb1_Work.trainPoint[idxNext2];
    prevPoint = &mb1_Work.trainPoint[idxPrev];
    MB1_HermiteCalcV(t, &prevPoint->pos, &point->pos, &nextPoint->pos, &nextPoint2->pos, pos);
    if(rot != NULL) {
        MB1_TrainPosGet(time+0.1f, &facePos, NULL);
        VECSubtract(&facePos, pos, &facePos);
        rot->x = HuAtan(-facePos.y, HuMagXZVecF(&facePos));
        rot->y = HuAtan(facePos.x, facePos.z);
        rot->z = 0;
    }
    return i;
}

void MB1_TrainTimeSet(float time)
{
    mb1_Work.trainTime = mb1_Work.trainMaxTime*time;
}

float MB1_TrainTimeGet(void)
{
    return mb1_Work.trainTime/mb1_Work.trainMaxTime;
}

void MB1_TrainSpeedSet(float speed)
{
    mb1_Work.trainSpeed = mb1_Work.trainMaxSpeed*speed;
}

void MB1_TrainCoinSpeedSet(float speed)
{
    mb1_Work.coinSpeed = 7.5f*speed;
}

void MB1_HasiOpen(int no)
{
    static HuVecF pos[2] = {
        { -2000, 775, 3185 },
        { 2000, 775, 3185 },
    };
    if(mb1_Work.hasiMode[no] == 0) {
        HuAudFXEmiterPlay(MSM_SE_BD1_10, &pos[no]);
        MBMotionSpeedSet(mb1_Work.hasiMdlId[no], 1);
        mb1_Work.hasiMode[no]++;
    }
}

void MB1_HasiClose(int no)
{
    static HuVecF pos[2] = {
        { -2000, 775, 3185 },
        { 2000, 775, 3185 },
    };
    if(mb1_Work.hasiMode[no] == 2) {
        HuAudFXEmiterPlay(MSM_SE_BD1_11, &pos[no]);
        MBMotionSpeedSet(mb1_Work.hasiMdlId[no], 1);
        mb1_Work.hasiMode[no]++;
    } else {
        mb1_Work.hasiSeF[no] = TRUE;
    }
}