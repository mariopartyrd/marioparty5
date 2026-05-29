#include "REL/m506Dll.h"

#define M506_MODEL_STAGE 0
#define M506_MODEL_MOUND 1
#define M506_MODEL_WINDMILL 2
#define M506_MODEL_MAME_VINE_TOP 3
#define M506_MODEL_MAME_VINE 4
#define M506_MODEL_MAME 5
#define M506_MODEL_MAME_SMALL 6
#define M506_MODEL_MAME_SPIRAL 7
#define M506_MODEL_SEED 8
#define M506_MODEL_STAR 9
#define M506_MODEL_KUMO 10
#define M506_MODEL_AURORA 11
#define M506_MODEL_ROCKET 12
#define M506_MODEL_MOON 13
#define M506_MODEL_SKY 14
#define M506_MODEL_SPACE 15
#define M506_MODEL_STARFIGHTER 16
#define M506_MODEL_NOKO2 17
#define M506_MODEL_STAGE_BORDER 18
#define M506_MODEL_SUN 19
#define M506_MODEL_MAME_ORANGE 20
#define M506_MODEL_KUMO2 21
#define M506_MODEL_SMOKE 22
#define M506_MODEL_MAME_SHADOW 23
#define M506_MODEL_GLOW_SHADOW 24
#define M506_MODEL_MAME_VINE_SHADOW 25

#define M506_MOT_MOUND 0
#define M506_MOT_WINDMILL 1

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

const s16 M506CameraBit[M506_PLAYER_MAX] = {
    HU3D_CAM0,
    HU3D_CAM1,
    HU3D_CAM2,
    HU3D_CAM3
};

static int M506ModelFile[] = {
    M506_HSF_stage,
    M506_HSF_mound,
    M506_HSF_windmill,
    M506_HSF_mame_vine_top,
    M506_HSF_mame_vine,
    M506_HSF_mame,
    M506_HSF_mame_small,
    M506_HSF_mame_spiral,
    M506_HSF_seed,
    M506_HSF_star,
    M506_HSF_kumo,
    M506_HSF_aurora,
    M506_HSF_rocket,
    M506_HSF_moon,
    M506_HSF_sky,
    M506_HSF_space,
    M506_HSF_starfighter,
    M506_HSF_noko2,
    M506_HSF_stage_border,
    M506_HSF_sun,
    M506_HSF_mame_orange,
    M506_HSF_kumo2,
    M506_HSF_smoke,
    M506_HSF_mame_shadow,
    M506_HSF_glow_shadow,
    M506_HSF_mame_vine_shadow
};

static int M506MotionFile[2] = {
    M506_HSF_mot_mound,
    M506_HSF_mot_windmill
};

static u8 MameSideData[6][3] = {
    { 0, 0, 1 },
    { 0, 1, 0 },
    { 0, 1, 1 },
    { 1, 0, 0 },
    { 1, 0, 1 },
    { 1, 1, 0 }
};

static HuVecF StarPosTbl[2] = {
    { 210, 12500, -3700 },
    { -300, 14000, -3000 }
};

static HuVecF KumoPosTbl[4] = {
    { 300, 1700, -2100 },
    { 220, 2700, -1500 },
    { -100, 3700, -1500 },
    { 270, 4700, -2200 }
};

static s16 StarMaxTime[3] = { 100, 170, 150 };
static s16 KumoDirTbl[4] = { -1, 1, 1, -1 };

static int streamNo;
OMOBJMAN *M506ObjMan;
OMOBJ *M506MainObj;
OMOBJ **M506PlayerObj;
OMOBJ *M506UpdateObj;

void M506TimerCreate(M506_WORK *work);
void M506Update(OMOBJ *obj);

void ObjectSetup(void)
{
    int i;
    OMOBJ *obj;
    M506_WORK *work;
    
    M506ObjMan = omInitObjMan(512, 8192);
    omGameSysInit(M506ObjMan);
    M506YCameraInit();
    M506YLightInit();
    M506YShadowInit();
    M506MainObj = omAddObj(M506ObjMan, 101, 0, 0, M506MainCreate);
    M506MainObj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M506_WORK), HU_MEMNUM_OVL);
    work = M506MainObj->data;
    omMakeGroupEx(M506ObjMan, 0, M506_PLAYER_MAX);
    M506PlayerObj = omGetGroupMemberListEx(M506ObjMan, 0);
    for(i=0; i<M506_PLAYER_MAX; i++) {
        M506PlayerObj[i] = obj = omAddObjEx(M506ObjMan, 100, 1, 7, 0, M506PlayerCreate);
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M506_PLAYER_WORK), HU_MEMNUM_OVL);
        obj->work[0] = i;
    }
    M506TimerCreate(work);
    M506MainObj->work[3] = 0;
    M506MainObj->work[2] = 0;
    Hu3DBGColorSet(0, 0, 0);
    M506UpdateObj = omAddObj(M506ObjMan, 102, 0, 0, M506Update);
}

void M506Exit(OMOBJ *obj)
{
    M506_WORK *work = obj->data;
    
    M506ModelUpdate(work);
    if(WipeCheckEnd()) {
        return;
    }
    HuAudAllStop();
    if(work->gameMesId >= 0) {
        GameMesKill(work->gameMesId);
    }
    Hu3DTexScrollAllKill();
    M506YShadowClose();
    M506YLightClose();
    M506YCameraClose();
    GameMesClose();
    MgTimerKill(work->timer);
    omOvlReturn(1);
}

BOOL M506ExitCheck(OMOBJ *obj)
{
    BOOL exitF = FALSE;
    
    if(omSysExitReq) {
        obj->work[3] = 3;
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 60);
        HuAudFadeOut(1000);
        obj->objFunc = M506Exit;
        exitF = TRUE;
    }
    return exitF;
}

void M506Update(OMOBJ *obj)
{
    M506YCameraUpdate();
    M506YShadowUpdate();
}

void M506Create(M506_WORK *m506Work)
{
    float mameSpiralOfs[4] = { 90, 170, 390, 470 };
    float mameSmallOfs[4] = { 200, 280, 500, 580 };
    M506_WORK *work = m506Work; //r31
    HU3DTEXSCRID texScrId; //r25
    int i; //r30
    int no; //r29
    int randNum; //r28
    int mameFirst; //r27
    int posY; //r26
    
    work->recordIdx = (m506Work->recordVal/3)-1;
    work->stageMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_STAGE]);
    Hu3DModelDispOff(work->stageMdlId);
    Hu3DModelAttrSet(work->stageMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->stageMdlId, 1);
    Hu3DModelPosSet(work->stageMdlId, 0, 0, 0);
    Hu3DModelRotSet(work->stageMdlId, 0, 0, 0);
    Hu3DModelDispOn(work->stageMdlId);
    Hu3DModelShadowMapSet(work->stageMdlId);
    work->stageBorderMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_STAGE_BORDER]);
    Hu3DModelDispOff(work->stageBorderMdlId);
    Hu3DModelAttrSet(work->stageBorderMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->stageBorderMdlId, 1);
    Hu3DModelPosSet(work->stageBorderMdlId, 0, 0, 0);
    Hu3DModelRotSet(work->stageBorderMdlId, 0, 0, 0);
    Hu3DModelDispOn(work->stageBorderMdlId);
    Hu3DModelShadowMapSet(work->stageBorderMdlId);
    for(i=0; i<2; i++) {
        work->bgMdlId[i] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_SKY+i]);
        Hu3DModelDispOff(work->bgMdlId[i]);
        Hu3DModelAttrSet(work->bgMdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(work->bgMdlId[i], 0);
        Hu3DModelPosSet(work->bgMdlId[i], 0, 0, 0);
        Hu3DModelRotSet(work->bgMdlId[i], 0, 0, 0);
        Hu3DModelDispOn(work->bgMdlId[i]);
    }
    work->windmillMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_WINDMILL]);
    Hu3DModelDispOff(work->windmillMdlId);
    Hu3DModelAttrSet(work->windmillMdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(work->windmillMdlId, 2);
    Hu3DModelPosSet(work->windmillMdlId, 265, 336, -2360);
    Hu3DModelRotSet(work->windmillMdlId, 0, -30, 0);
    Hu3DModelDispOn(work->windmillMdlId);
    work->windmillMotId = Hu3DJointMotionData(work->windmillMdlId, M506MotionFile[M506_MOT_WINDMILL]);
    Hu3DMotionSet(work->windmillMdlId, work->windmillMotId);
    work->rocketMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_ROCKET]);
    Hu3DModelDispOff(work->rocketMdlId);
    Hu3DModelAttrSet(work->rocketMdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(work->rocketMdlId, 3);
    Hu3DModelPosSet(work->rocketMdlId, 0, 14000, -2500);
    Hu3DModelRotSet(work->rocketMdlId, 0, 0, 0);
    for(no=0; no<M506_PLAYER_MAX; no++) {
        if(no == 0) {
            work->starfighterMdlId[no] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_STARFIGHTER]);
        } else {
            work->starfighterMdlId[no] = Hu3DModelLink(work->starfighterMdlId[0]);
        }
        Hu3DModelDispOff(work->starfighterMdlId[no]);
        Hu3DModelAttrSet(work->starfighterMdlId[no], HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(work->starfighterMdlId[no], 3);
        Hu3DModelPosSet(work->starfighterMdlId[no], 150, 16000, -2500);
        Hu3DModelRotSet(work->starfighterMdlId[no], 0, 0, 0);
        Hu3DModelCameraSet(work->starfighterMdlId[no], M506CameraBit[no]);
        Hu3DModelDispOn(work->starfighterMdlId[no]);
    }
    work->moonMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MOON]);
    Hu3DModelDispOff(work->moonMdlId);
    Hu3DModelAttrSet(work->moonMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->moonMdlId, 2);
    Hu3DModelPosSet(work->moonMdlId, 0, 17400, -4000);
    Hu3DModelRotSet(work->moonMdlId, 0, 0, 0);
    Hu3DModelDispOn(work->moonMdlId);
    for(no=0; no<M506_PLAYER_MAX; no++) {
        work->mameVinePosY[no] = 0;
        work->mameVineRotY[no] = 0;
        if(no == 0) {
            work->mameVineMdlId[0][0] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME_VINE_TOP]);
        } else {
            work->mameVineMdlId[no][0] = Hu3DModelLink(work->mameVineMdlId[0][0]);
        }
        Hu3DModelDispOff(work->mameVineMdlId[no][0]);
        Hu3DModelAttrSet(work->mameVineMdlId[no][0], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(work->mameVineMdlId[no][0], 4);
        Hu3DModelPosSet(work->mameVineMdlId[no][0], 0, 0, 0);
        Hu3DModelRotSet(work->mameVineMdlId[no][0], 0, 0, 0);
        Hu3DModelCameraSet(work->mameVineMdlId[no][0], M506CameraBit[no]);
        Hu3DModelDispOn(work->mameVineMdlId[no][0]);
        for(i=1; i<M506_VINE_MAX; i++) {
            if(no == 0 && i == 1) {
                work->mameVineMdlId[0][1] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME_VINE]);
            } else {
                work->mameVineMdlId[no][i] = Hu3DModelLink(work->mameVineMdlId[0][1]);
            }
            Hu3DModelDispOff(work->mameVineMdlId[no][i]);
            Hu3DModelAttrSet(work->mameVineMdlId[no][i], HU3D_MOTATTR_PAUSE);
            Hu3DModelLayerSet(work->mameVineMdlId[no][i], 4);
            Hu3DModelPosSet(work->mameVineMdlId[no][i], 0, -600.0f*i, 0);
            Hu3DModelRotSet(work->mameVineMdlId[no][i], 0, 0, 0);
            Hu3DModelCameraSet(work->mameVineMdlId[no][i], M506CameraBit[no]);
        }
        Hu3DModelDispOn(work->mameVineMdlId[no][1]);
    }
    for(no=0; no<M506_PLAYER_MAX; no++) {
        work->mameVineTime[no] = 0;
        if(no == 0) {
            for(i=0; i<M506_JUMP_MAX; i += 3) {
                randNum = rand8()/43;
                work->mameSide[no][i] = MameSideData[randNum][0];
                work->mameSide[no][i+1] = MameSideData[randNum][1];
                work->mameSide[no][i+2] = MameSideData[randNum][2];
            }
        } else {
            for(i=0; i<M506_JUMP_MAX; i += 3) {
                work->mameSide[no][i] = work->mameSide[0][i];
                work->mameSide[no][i+1] = work->mameSide[0][i+1];
                work->mameSide[no][i+2] = work->mameSide[0][i+2];
            }
        }
    }
    for(no=0; no<M506_PLAYER_MAX; no++) {
        if(no == 0) {
            for(i=0; i<M506_DECOR_MAX; i++) {
                randNum = (rand8()/128)*16;
                if(i == 0) {
                    randNum |= ((rand8()/32)+4);
                } else {
                    randNum |= (rand8()/22);
                }
                work->mameSmallFlag[no][i] = randNum;
            }
        } else {
            for(i=0; i<M506_DECOR_MAX; i++) {
                work->mameSmallFlag[no][i] = work->mameSmallFlag[0][i];
            }
        }
    }
    for(no=0; no<M506_PLAYER_MAX; no++) {
        if(no == 0) {
            for(i=0; i<M506_DECOR_MAX; i++) {
                randNum = (rand8()/128)*16;
                if(i == 0) {
                    randNum |= ((rand8()/32)+4);
                } else {
                    randNum |= (rand8()/22);
                }
                work->mameSpiralFlag[no][i] = randNum;
            }
        } else {
            for(i=0; i<M506_DECOR_MAX; i++) {
                work->mameSpiralFlag[no][i] = work->mameSpiralFlag[0][i];
            }
        }
    }
    mameFirst = -1;
    for(no=0; no<M506_PLAYER_MAX; no++) {
        for(i=0; i<M506_JUMP_MAX; i++) {
            if(i == work->recordIdx && !_CheckFlag(FLAG_INST_DECA)) {
                if(no == 0) {
                    work->mameMdlId[0][work->recordIdx] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME_ORANGE]);
                } else {
                    work->mameMdlId[no][i] = Hu3DModelLink(work->mameMdlId[0][work->recordIdx]);
                }
            } else {
                if(no == 0 && mameFirst < 0) {
                    mameFirst = i;
                    work->mameMdlId[0][mameFirst] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME]);
                } else {
                    work->mameMdlId[no][i] = Hu3DModelLink(work->mameMdlId[0][mameFirst]);
                }
            }
            Hu3DModelDispOff(work->mameMdlId[no][i]);
            Hu3DModelAttrSet(work->mameMdlId[no][i], HU3D_MOTATTR_PAUSE);
            Hu3DModelLayerSet(work->mameMdlId[no][i], 4);
            Hu3DModelPosSet(work->mameMdlId[no][i], 0, 300.0f+(300.0f*i), 0);
            if(work->mameSide[no][i]) {
                Hu3DModelRotSet(work->mameMdlId[no][i], 0, 20, 0);
            } else {
                Hu3DModelRotSet(work->mameMdlId[no][i], 0, -20, 0);
            }
            
            Hu3DModelCameraSet(work->mameMdlId[no][i], M506CameraBit[no]);
        }
        Hu3DModelDispOn(work->mameMdlId[no][0]);
    }
    work->seedMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_SEED]);
    Hu3DModelDispOff(work->seedMdlId);
    Hu3DModelAttrSet(work->seedMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->seedMdlId, 4);
    Hu3DModelPosSet(work->seedMdlId, 0, 0, 0);
    Hu3DModelRotSet(work->seedMdlId, 0, 0, 0);
    Hu3DModelDispOn(work->seedMdlId);
    work->moundMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MOUND]);
    Hu3DModelDispOff(work->moundMdlId);
    Hu3DModelAttrSet(work->moundMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->moundMdlId, 4);
    Hu3DModelPosSet(work->moundMdlId, 0, 0, 0);
    Hu3DModelRotSet(work->moundMdlId, 0, 0, 0);
    Hu3DModelDispOn(work->moundMdlId);
    work->moundMotId = Hu3DJointMotionData(work->moundMdlId, M506MotionFile[M506_MOT_MOUND]);
    Hu3DMotionSet(work->moundMdlId, work->moundMotId);
    for(i=0; i<M506_PLAYER_MAX; i++) {
        work->mameShadowMdlId[i] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME_SHADOW]);
        Hu3DModelDispOff(work->mameShadowMdlId[i]);
        Hu3DModelAttrSet(work->mameShadowMdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(work->mameShadowMdlId[i], 5);
        Hu3DModelPosSet(work->mameShadowMdlId[i], 0, 10, 0);
        Hu3DModelRotSet(work->mameShadowMdlId[i], 0, 0, 0);
        Hu3DModelCameraSet(work->mameShadowMdlId[i], M506CameraBit[i]);
        work->mameShadowTexScrId[i] = Hu3DTexScrollCreate(work->mameShadowMdlId[i], "I8m506sdw01");
        if(i == 0) {
            Hu3DTexScrollPosSet(work->mameShadowTexScrId[i], 0, 0, 0);
        } else {
            Hu3DTexScrollPosSet(work->mameShadowTexScrId[i], 0, 0.4f, 0);
            Hu3DModelAttrReset(work->mameShadowMdlId[i], HU3D_MOTATTR_PAUSE);
            Hu3DModelDispOn(work->mameShadowMdlId[i]);
        }
    }
    work->glowShadowMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_GLOW_SHADOW]);
    Hu3DModelDispOff(work->glowShadowMdlId);
    Hu3DModelAttrSet(work->glowShadowMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->glowShadowMdlId, 5);
    Hu3DModelPosSet(work->glowShadowMdlId, -135, 10, 400);
    if(work->mameSide[0][0]) {
        Hu3DModelRotSet(work->glowShadowMdlId, 0, 50, 180);
    } else {
        Hu3DModelRotSet(work->glowShadowMdlId, 0, 50, 0);
    }
    work->mameVineShadowMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME_VINE_SHADOW]);
    Hu3DModelDispOff(work->mameVineShadowMdlId);
    Hu3DModelAttrSet(work->mameVineShadowMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->mameVineShadowMdlId, 2);
    Hu3DModelPosSet(work->mameVineShadowMdlId, 0, -5, 0);
    Hu3DModelRotSet(work->mameVineShadowMdlId, 0, 0, 0);
    Hu3DModelDispOn(work->mameVineShadowMdlId);
    for(i=0; i<4; i++) {
        if(i == 0) {
            work->kumoMdlId[i] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_KUMO]);
        } else {
            work->kumoMdlId[i] = Hu3DModelLink(work->kumoMdlId[0]);
        }
        Hu3DModelDispOff(work->kumoMdlId[i]);
        Hu3DModelAttrSet(work->kumoMdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(work->kumoMdlId[i], 3);
        Hu3DModelPosSetV(work->kumoMdlId[i], &KumoPosTbl[i]);
        Hu3DModelRotSet(work->kumoMdlId[i], 0, 0, 0);
        Hu3DModelDispOn(work->kumoMdlId[i]);
        work->kumoPosX[i] = KumoPosTbl[i].x;
        work->kumoDir[i] = KumoDirTbl[i];
        work->kumoAngle[i] = 0;
    }
    for(i=0; i<2; i++) {
        if(i == 0) {
            work->starMdlId[i] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_STAR]);
        } else {
            work->starMdlId[i] = Hu3DModelLink(work->starMdlId[0]);
        }
        Hu3DModelDispOff(work->starMdlId[i]);
        Hu3DModelAttrSet(work->starMdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(work->starMdlId[i], 3);
        Hu3DModelPosSetV(work->starMdlId[i], &StarPosTbl[i]);
        Hu3DModelRotSet(work->starMdlId[i], 0, 0, 0);
        Hu3DModelDispOn(work->starMdlId[i]);
        work->starTime[i] = 0;
    }
    work->auroraMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_AURORA]);
    Hu3DModelDispOff(work->auroraMdlId);
    Hu3DModelAttrSet(work->auroraMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->auroraMdlId, 3);
    Hu3DModelPosSet(work->auroraMdlId, 0, 0, 0);
    Hu3DModelRotSet(work->auroraMdlId, 0, 0, 0);
    Hu3DModelDispOn(work->auroraMdlId);
    texScrId = Hu3DTexScrollCreate(work->auroraMdlId, "m506alora");
    Hu3DTexScrollPosMoveSet(texScrId, 0, 0.0016666667f, 0);
    Hu3DTexScrollPosMoveSet(texScrId = Hu3DTexScrollCreate(work->auroraMdlId, "I8m506aloraz"), 0, 0.0016666667f, 0);
    for(no=0; no<M506_PLAYER_MAX; no++) {
        for(i=0; i<M506_DECOR_MAX; i++) {
            if(no == 0 && i == 0) {
                work->mameSmallMdlId[no][i] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME_SMALL]);
            } else {
                work->mameSmallMdlId[no][i] = Hu3DModelLink(work->mameSmallMdlId[0][0]);
            }
            Hu3DModelDispOff(work->mameSmallMdlId[no][i]);
            Hu3DModelAttrSet(work->mameSmallMdlId[no][i], HU3D_MOTATTR_PAUSE);
            Hu3DModelLayerSet(work->mameSmallMdlId[no][i], 4);
            randNum = work->mameSmallFlag[no][i] & 0xF;
            posY = randNum/4;
            work->mameSmallPosY[no][i] = (600.0f*posY)+((1800.0f*i)+mameSmallOfs[randNum%4]);
            Hu3DModelPosSet(work->mameSmallMdlId[no][i], 0, work->mameSmallPosY[no][i], 0);
            
            if(work->mameSmallFlag[no][i] & 0x10) {
                Hu3DModelRotSet(work->mameSmallMdlId[no][i], 50, -90, 0);
            } else {
                Hu3DModelRotSet(work->mameSmallMdlId[no][i], 50, 90, 0);
            }
            Hu3DModelCameraSet(work->mameSmallMdlId[no][i], M506CameraBit[no]);
            Hu3DModelScaleSet(work->mameSmallMdlId[no][i], 0, 0, 0);
            work->mameSmallTime[no][i] = 0;
        }
    }
    for(no=0; no<M506_PLAYER_MAX; no++) {
        for(i=0; i<M506_DECOR_MAX; i++) {
            if(no == 0 && i == 0) {
                work->mameSpiralMdlId[no][i] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_MAME_SPIRAL]);
            } else {
                work->mameSpiralMdlId[no][i] = Hu3DModelLink(work->mameSpiralMdlId[0][0]);
            }
            Hu3DModelDispOff(work->mameSpiralMdlId[no][i]);
            Hu3DModelAttrSet(work->mameSpiralMdlId[no][i], HU3D_MOTATTR_PAUSE);
            Hu3DModelLayerSet(work->mameSpiralMdlId[no][i], 4);
            randNum = work->mameSpiralFlag[no][i] & 0xF;
            posY = randNum/4;
            work->mameSpiralPosY[no][i] = (600.0f*posY)+((1800.0f*i)+mameSpiralOfs[randNum%4]);
            Hu3DModelPosSet(work->mameSpiralMdlId[no][i], 0, work->mameSpiralPosY[no][i], 0);
            
            if(work->mameSpiralFlag[no][i] & 0x10) {
                Hu3DModelRotSet(work->mameSpiralMdlId[no][i], 0, -90, 0);
            } else {
                Hu3DModelRotSet(work->mameSpiralMdlId[no][i], 0, 90, 0);
            }
            Hu3DModelCameraSet(work->mameSpiralMdlId[no][i], M506CameraBit[no]);
            Hu3DModelScaleSet(work->mameSpiralMdlId[no][i], 0, 0, 0);
            work->mameSpiralTime[no][i] = 0;
        }
    }
    work->noko2MdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_NOKO2]);
    Hu3DModelDispOff(work->noko2MdlId);
    Hu3DModelAttrSet(work->noko2MdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(work->noko2MdlId, 2);
    Hu3DModelPosSet(work->noko2MdlId, 100, 6500, -100);
    Hu3DModelRotSet(work->noko2MdlId, 0, -90, 0);
    Hu3DModelDispOn(work->noko2MdlId);
    work->sunMdlId = Hu3DModelCreateData(M506ModelFile[M506_MODEL_SUN]);
    Hu3DModelDispOff(work->sunMdlId);
    Hu3DModelAttrSet(work->sunMdlId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(work->sunMdlId, 1);
    Hu3DModelPosSet(work->sunMdlId, -210, 10050, -4500);
    Hu3DModelRotSet(work->sunMdlId, 0, 30, 0);
    Hu3DModelDispOn(work->sunMdlId);
    for(no=0; no<M506_PLAYER_MAX; no++) {
        work->mameEnterTime[no] = 0;
        work->unk53C[no] = 0;
        work->mameMissTime[no] = 0;
    }
    work->noko2Time = 0;
    work->noko2Angle =0 ;
    work->noko2CamBit = 0;
    work->rocketCamBit = 0;
    work->kumo2MdlId[0] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_KUMO2]);
    work->kumo2MdlId[1] = Hu3DModelLink(work->kumo2MdlId[0]);
    work->kumo2MdlId[2] = Hu3DModelLink(work->kumo2MdlId[0]);
    work->kumo2MdlId[3] = Hu3DModelLink(work->kumo2MdlId[0]);
    for(i=0; i<M506_PLAYER_MAX; i++) {
        Hu3DModelDispOff(work->kumo2MdlId[i]);
        Hu3DModelAttrSet(work->kumo2MdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(work->kumo2MdlId[i], 6);
        Hu3DModelPosSet(work->kumo2MdlId[i], 0, 9300, 1200);
        Hu3DModelRotSet(work->kumo2MdlId[i], 0, 0, 0);
        Hu3DModelCameraSet(work->kumo2MdlId[i], M506CameraBit[i]);
        Hu3DModelDispOn(work->kumo2MdlId[i]);
    }
    work->smokeMdlId[0] = Hu3DModelCreateData(M506ModelFile[M506_MODEL_SMOKE]);
    work->smokeMdlId[1] = Hu3DModelLink(work->smokeMdlId[0]);
    work->smokeMdlId[2] = Hu3DModelLink(work->smokeMdlId[0]);
    for(i=0; i<3; i++) {
        Hu3DModelDispOff(work->smokeMdlId[i]);
        Hu3DModelAttrSet(work->smokeMdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(work->smokeMdlId[i], 6);
        Hu3DModelPosSet(work->smokeMdlId[i], 0, 0, 0);
        Hu3DModelRotSet(work->smokeMdlId[i], 0, 0, 0);
    }
}

void M506TimerCreate(M506_WORK *work)
{
    work->gameMesId = GAMEMES_ID_NONE;
    work->timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
    MgTimerParamSet(work->timer, 30*60, 0, 0);
    work->recordVal = GWMgRecordGet(0);
    
}

BOOL M506FlowUpdate(M506_WORK *work)
{
    BOOL result = FALSE;
    u32 *mode = &M506MainObj->work[3];
    switch(*mode) {
        case 0:
            if(work->gameMesId < 0) {
                work->gameMesId = GameMesMgStartCreate();
                *mode = 1;
                Hu3DModelDispOff(work->stageBorderMdlId);
                work->gameMesDelay = 0;
                streamNo = HuAudBGMPlay(MSM_STREAM_MGMUS_10);
            }
            break;
       
        case 1:
            if(GameMesStatGet(work->gameMesId) == 0) {
                work->gameMesId = GAMEMES_ID_NONE;
                MgTimerModeOnSet(work->timer, MGTIMER_OFFTYPE_FADEOUT);
                *mode = 2;
            }
            break;
            
        case 2:
            if(MgTimerDoneCheck(work->timer)) {
                *mode = 4;
            }
            break;
            
        case 3:
            if(++work->gameMesDelay > 30) {
                *mode = 5;
            }
            break;
            
        case 4:
            if(work->gameMesId < 0) {
                work->gameMesId = GameMesMgFinishCreate();
                HuAudSStreamFadeOut(streamNo, 1000);
            } else if(GameMesStatGet(work->gameMesId) == 0) {
                work->gameMesId = GAMEMES_ID_NONE;
                *mode = 3;
            }
            break;
            
        case 5:
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            *mode = 6;
            break;
            
        case 6:
            if(!WipeCheckEnd()) {
                result = TRUE;
            }
            break;
    }
    M506ModelUpdate(work);
    return result;
}

void M506MameVineScrollUp(M506_PLAYER_WORK *player)
{
    M506_WORK *work; //r30
    int i; //r29
    M506_WORK *m506Work; //r28
    float scrollY; //f31
    
    m506Work = M506MainObj->data;
    work = m506Work;
    if(player->newHeight == player->height && player->height < M506_JUMP_MAX && player->jumpF) {
        work->mameVinePosY[player->no] += 20;
        Hu3DModelPosSet(work->mameVineMdlId[player->no][0], 0, work->mameVinePosY[player->no], 0);
        work->mameVineRotY[player->no] += 24;
        if(work->mameVineRotY[player->no] > 360) {
            work->mameVineRotY[player->no] -= 360;
        }
        Hu3DModelRotSet(work->mameVineMdlId[player->no][0], 0, work->mameVineRotY[player->no], 0);
        Hu3DModelPosSet(work->mameVineMdlId[player->no][1], 0, work->mameVinePosY[player->no]-600, 0);
        Hu3DModelRotSet(work->mameVineMdlId[player->no][1], 0, work->mameVineRotY[player->no], 0);
        for(i=2; i<M506_VINE_MAX; i++) {
            Hu3DModelPosSet(work->mameVineMdlId[player->no][i], 0, work->mameVinePosY[player->no]-(i*600.0f), 0);
        }
        if(work->mameVinePosY[player->no] < 300) {
            scrollY = 0.4f+(0.4f*(work->mameVinePosY[player->no]/300));
        } else {
            scrollY = 0.8f;
        }
        Hu3DTexScrollPosSet(work->mameShadowTexScrId[player->no], 0, scrollY, 0);
    }
}

void M506MameIn(M506_PLAYER_WORK *player)
{
    M506_WORK *work; //r30
    M506_WORK *m506Work; //r29
    float scale; //f31
    float rotX; //f30
    
    m506Work = M506MainObj->data;
    work = m506Work;
    if(player->newHeight == player->height && player->height < M506_JUMP_MAX && player->jumpF) {
        if(work->mameVineTime[player->no] == 0) {
            Hu3DModelDispOn(work->mameMdlId[player->no][player->height]);
        }
        work->mameVineTime[player->no]++;
        if(work->mameVineTime[player->no] < 5) {
            scale = (1.1f*work->mameVineTime[player->no])/4;
        } else if(work->mameVineTime[player->no] < 7) {
            scale = 1.1f-(0.05f*(work->mameVineTime[player->no]-5));
        } else {
            scale = 1;
            if(work->mameVineTime[player->no] == 15) {
                work->mameVineTime[player->no] = 0;
            }
        }
        if(work->mameVineTime[player->no] < 4) {
            rotX = 2.0f*(-work->mameVineTime[player->no]);
        } else if(work->mameVineTime[player->no] < 7) {
            rotX = -6.0f+((6.0f*(work->mameVineTime[player->no]-3))/3);
        } else {
            rotX = 0;
        }
        Hu3DModelScaleSet(work->mameMdlId[player->no][player->height], scale, scale, scale);
        if(work->mameSide[player->no][player->height]) {
            Hu3DModelRotSet(work->mameMdlId[player->no][player->height], rotX, 20, 0);
        } else {
            Hu3DModelRotSet(work->mameMdlId[player->no][player->height], rotX, -20, 0);
        }
    }
}

void M506MameSpiralIn(M506_PLAYER_WORK *player)
{
    M506_WORK *work; //r31
    int i; //r30
    M506_WORK *m506Work; //r28
    float scaleXZ; //f31
    float scaleY; //f30
    float vineY; //f29
    
    m506Work = M506MainObj->data;
    work = m506Work;
    vineY = work->mameVinePosY[player->no];
    for(i=0; i<M506_DECOR_MAX; i++) {
        if(vineY > work->mameSpiralPosY[player->no][i]-600 && work->mameSpiralTime[player->no][i] < 25) {
            work->mameSpiralTime[player->no][i]++;
            scaleY = work->mameSpiralTime[player->no][i]/25.0f;
            scaleXZ = 3*scaleY;
            if(scaleXZ > 1) {
                scaleXZ = 1;
            }
            Hu3DModelScaleSet(work->mameSpiralMdlId[player->no][i], scaleXZ, scaleY, scaleXZ);
        }
    }
}

void M506MameSmallIn(M506_PLAYER_WORK *player)
{
    M506_WORK *work; //r31
    int i; //r30
    M506_WORK *m506Work; //r28
    int flag; //r27
    float scale; //f31
    float rotX; //f30
    float vineY; //f29
    
    m506Work = M506MainObj->data;
    work = m506Work;
    vineY = work->mameVinePosY[player->no];
    for(i=0; i<M506_DECOR_MAX; i++) {
        if(vineY > work->mameSmallPosY[player->no][i]-600) {
            work->mameSmallTime[player->no][i]++;
            if(work->mameSmallTime[player->no][i] < 40) {
                scale = work->mameSmallTime[player->no][i]/40.0f;
                if(scale > 1) {
                    scale = 1;
                }
                Hu3DModelScaleSet(work->mameSmallMdlId[player->no][i], scale, scale, scale);
            }
            if(work->mameSmallTime[player->no][i] >= 80) {
                work->mameSmallTime[player->no][i] = 40;
            }
            rotX = 10*HuSin(((work->mameSmallTime[player->no][i]-40)*360.0f)/40.0f);
            flag = work->mameSmallFlag[player->no][i];
            if(flag & 0x10) {
                Hu3DModelRotSet(work->mameSmallMdlId[player->no][i], 50-rotX, -90, 0);
            } else {
                Hu3DModelRotSet(work->mameSmallMdlId[player->no][i], 50-rotX, 90, 0);
            }
        }
    }
}

void M506KumoUpdate(M506_WORK *work)
{
    float t; //f31
    float scale; //f30
    int i;
    for(i=0; i<4; i++) {
        t = (300-fabsf(work->kumoPosX[i]))/100;
        if(t > 1) {
            t = 1;
        }
        if(t < 0.2f) {
            t = 0.2f;
        }
        work->kumoPosX[i] += work->kumoDir[i]*t;
        if(work->kumoPosX[i] < -300 || work->kumoPosX[i] > 300) {
            work->kumoDir[i] *= -1;
        }
        Hu3DModelPosSet(work->kumoMdlId[i], work->kumoPosX[i], KumoPosTbl[i].y, KumoPosTbl[i].z);
        work->kumoAngle[i] += 10.0f*KumoDirTbl[i];
        if(work->kumoAngle[i] > 360) {
            work->kumoAngle[i] -= 360;
        }
        scale = (0.05f*HuSin(work->kumoAngle[i]))+1;
        Hu3DModelScaleSet(work->kumoMdlId[i], scale, scale, 1);
    }
}

void M506StarUpdate(M506_WORK *work)
{
    float scale; //f31
    int i; //r31
    for(i=0; i<2; i++) {
        work->starTime[i]++;
        if(work->starTime[i] > StarMaxTime[i]) {
            work->starTime[i] = 0;
        }
        scale = (0.1f*HuSin((work->starTime[i]*360.0f)/StarMaxTime[i]))+0.95f;
        Hu3DModelScaleSet(work->starMdlId[i], scale, scale, 1);
    }
}

void M506Noko2Update(M506_WORK *work)
{
    int i;
    float y;
    float x;
    float z;
    M506_PLAYER_WORK *player;
    if(work->noko2CamBit == 0) {
        for(i=0; i<M506_PLAYER_MAX; i++) {
            player = M506PlayerObj[i]->data;
            if(player->newHeight == 15) {
                work->noko2CamBit |= M506CameraBit[i];
            }
        }
    }
    work->noko2Time++;
    if(work->noko2Time > 360) {
        work->noko2Time -= 360;
    }
    work->noko2Angle++;
    if(work->noko2Angle > 360) {
        work->noko2Angle -= 360;
    }
    y = 5500+(20*HuSin(work->noko2Time));
    x = 120*HuSin(work->noko2Angle);
    z = (20*HuCos(work->noko2Angle))-220;
    Hu3DModelPosSet(work->noko2MdlId, x, y, z);
    Hu3DModelRotSet(work->noko2MdlId, 0, work->noko2Angle, 0);
}

void M506MameUpdate(M506_PLAYER_WORK *player)
{
    if(player->jumpTicks < 16) {
        M506MameVineScrollUp(player);
    } else if(player->jumpTicks < 31) {
        M506MameIn(player);
    }
}

void M506MameModeEnter(M506_PLAYER_WORK *player)
{
    M506_WORK *work; //r30
    int height; //r29
    M506_WORK *m506Work; //r28
    
    float rotX; //f31
    
    m506Work = M506MainObj->data;
    work = m506Work;
    
    if(player->newHeight > 0) {
        height = player->newHeight-1;
        work->mameEnterTime[player->no]++;
        if(work->mameEnterTime[player->no] < 8) {
            rotX = 0;
        } else if(work->mameEnterTime[player->no] < 10) {
            rotX = work->mameEnterTime[player->no]-7.0f;
        } else if(work->mameEnterTime[player->no] < 15) {
            rotX = 5*(1-((work->mameEnterTime[player->no]-9.0f)/5));
        } else {
            work->mameEnterTime[player->no] = 0;
            rotX = 0;
        }
        if(work->mameSide[player->no][height]) {
            Hu3DModelRotSet(work->mameMdlId[player->no][height], rotX, 20, 0);
        } else {
            Hu3DModelRotSet(work->mameMdlId[player->no][height], rotX, -20, 0);
        }
    }
}

void M506MameModeMiss(M506_PLAYER_WORK *player)
{
    M506_WORK *work; //r30
    int height; //r29
    M506_WORK *m506Work; //r28
    
    float rotX;
    
    m506Work = M506MainObj->data;
    work = m506Work;
    height = player->prevHeight;
    
    work->mameMissTime[player->no]++;
    if(work->mameMissTime[player->no] < 2) {
        rotX = 0;
    } else if(work->mameMissTime[player->no] < 6) {
        rotX = ((work->mameMissTime[player->no]-1.0f)*20)/4;
    } else if(work->mameMissTime[player->no] < 20) {
        rotX = 20*HuSin(90.0f*(19.0f-work->mameMissTime[player->no])/13.0f);
    } else if(work->mameMissTime[player->no] < 30) {
        rotX = 0;
    } else {
        work->mameMissTime[player->no] = 0;
        rotX = 0;
    }
    if(work->mameSide[player->no][height]) {
        Hu3DModelRotSet(work->mameMdlId[player->no][height], -rotX, 20, 0);
    } else {
        Hu3DModelRotSet(work->mameMdlId[player->no][height], -rotX, -20, 0);
    }
}

void M506MameModeJump(M506_PLAYER_WORK *player)
{
    M506_WORK *work; //r30
    int height; //r29
    M506_WORK *m506Work; //r28
    
    float rotZ; //f31
    
    m506Work = M506MainObj->data;
    work = m506Work;
    
    if(player->newHeight > 0) {
        height = player->newHeight-1;
        work->mameMissTime[player->no]++;
        if(work->mameMissTime[player->no] < 4) {
            rotZ = 0;
        } else if(work->mameMissTime[player->no] < 6) {
            rotZ = ((work->mameMissTime[player->no]-3.0f)*50)/2;
        } else if(work->mameMissTime[player->no] < 20) {
            rotZ = 50*HuSin(90.0f*(19.0f-work->mameMissTime[player->no])/13.0f);
        } else if(work->mameMissTime[player->no] < 30) {
            rotZ = 0;
        } else {
            work->mameMissTime[player->no] = 0;
            rotZ = 0;
        }
        if(work->mameSide[player->no][height]) {
            Hu3DModelRotSet(work->mameMdlId[player->no][height], 0, 20, rotZ);
        } else {
            Hu3DModelRotSet(work->mameMdlId[player->no][height], 0, -20, -rotZ);
        }
    }
}

void M506ModelUpdate(M506_WORK *m506Work)
{
    M506_WORK *work; //r31
    int i; //r30
    int no; //r29
    u16 stageCamBit; //r21
    u16 moonCamBit; //r18
    u16 auroraCamBit; //r17
    
    
    float cameraPosY; //f31
    float posY; //f30
    float dispMaxY; //f29
    float dispMinY; //f28
    float time; //f27
    
    M506_PLAYER_WORK *player[M506_PLAYER_MAX]; //sp+0x50
    
    u16 noko2CamBit; //sp+0x18
    u16 starCamBit; //sp+0x16
    u16 kumoCamBit; //sp+0x14
    u16 sunCamBit; //sp+0x12
    u16 skyCamBit; //sp+0x10
    u16 spaceCamBit; //sp+0xE
    u16 rocketCamBit; //sp+0xC
    
    moonCamBit = 0;
    stageCamBit = 0;
    auroraCamBit = 0;
    
    noko2CamBit = 0;
    starCamBit = 0;
    kumoCamBit = 0;
    sunCamBit = 0;
    skyCamBit = 0;
    spaceCamBit = 0;
    rocketCamBit = 0;
    for(i=0; i<M506_PLAYER_MAX; i++) {
        player[i] = M506PlayerObj[i]->data;
    }
    work = m506Work;
    for(no=0; no<M506_PLAYER_MAX; no++) {
        cameraPosY = M506YCamera->camera[player[no]->no].target.y;
        dispMaxY = 600+cameraPosY;
        dispMinY = cameraPosY-1800;
        if(cameraPosY < 8300) {
            time = 0;
        } else {
            time = (29*(cameraPosY-8300))/10000;
        }
        for(i=0; i<M506_VINE_MAX; i++) {
            Hu3DMotionTimeSet(work->mameVineMdlId[no][i], time);
            posY = work->mameVinePosY[player[no]->no]-(i*600.0f);
            if(posY < dispMaxY && posY > dispMinY) {
                Hu3DModelDispOn(work->mameVineMdlId[player[no]->no][i]);
            } else {
                Hu3DModelDispOff(work->mameVineMdlId[player[no]->no][i]);
            }
        }
        dispMaxY = 600+cameraPosY;
        dispMinY = cameraPosY-900;
        for(i=0; i<player[no]->height; i++) {
            Hu3DMotionTimeSet(work->mameMdlId[player[no]->no][i], time);
            posY = 300+(i*300.0f);
            if(posY < dispMaxY && posY > dispMinY) {
                Hu3DModelDispOn(work->mameMdlId[player[no]->no][i]);
            } else {
                Hu3DModelDispOff(work->mameMdlId[player[no]->no][i]);
            }
        }
        if(i < M506_JUMP_MAX) {
            Hu3DMotionTimeSet(work->mameMdlId[player[no]->no][i], time);
        }
        dispMaxY = 660+cameraPosY;
        dispMinY = cameraPosY-900;
        for(i=0; i<M506_DECOR_MAX; i++) {
            Hu3DMotionTimeSet(work->mameSpiralMdlId[player[no]->no][i], time);
            posY = work->mameSpiralPosY[no][i];
            if(posY < dispMaxY && posY > dispMinY) {
                Hu3DModelDispOn(work->mameSpiralMdlId[player[no]->no][i]);
            } else {
                Hu3DModelDispOff(work->mameSpiralMdlId[player[no]->no][i]);
            }
        }
        for(i=0; i<M506_DECOR_MAX; i++) {
            Hu3DMotionTimeSet(work->mameSmallMdlId[player[no]->no][i], time);
            posY = work->mameSmallPosY[no][i];
            if(posY < dispMaxY && posY > dispMinY) {
                Hu3DModelDispOn(work->mameSmallMdlId[player[no]->no][i]);
            } else {
                Hu3DModelDispOff(work->mameSmallMdlId[player[no]->no][i]);
            }
        }
        if(cameraPosY > 14500) {
            moonCamBit |= M506CameraBit[no];
        }
        if(cameraPosY < 3000) {
            stageCamBit |= M506CameraBit[no];
        }
        if(cameraPosY > 8000 && cameraPosY < 15000) {
            auroraCamBit |= M506CameraBit[no];
        }
        if(cameraPosY > 4500 && cameraPosY < 6500) {
            noko2CamBit |= M506CameraBit[no];
        }
        if(cameraPosY > 11000) {
            starCamBit |= M506CameraBit[no];
        }
        if(cameraPosY < 6600) {
            kumoCamBit |= M506CameraBit[no];
        }
        if(cameraPosY < 14000) {
            skyCamBit |= M506CameraBit[no];
        }
        if(cameraPosY > 6000) {
            spaceCamBit |= M506CameraBit[no];
        }
        if(cameraPosY > 6000 && cameraPosY < 13000) {
            sunCamBit |= M506CameraBit[no];
        }
        if(cameraPosY > 10000) {
            rocketCamBit |= M506CameraBit[no];
        }
        M506MameSpiralIn(player[no]);
        M506MameSmallIn(player[no]);
    }
    Hu3DModelCameraSet(work->stageMdlId, stageCamBit);
    Hu3DModelCameraSet(work->windmillMdlId, stageCamBit);
    Hu3DModelCameraSet(work->moundMdlId, stageCamBit);
    Hu3DModelCameraSet(work->moonMdlId, moonCamBit);
    Hu3DModelCameraSet(work->auroraMdlId, auroraCamBit);
    Hu3DModelCameraSet(work->noko2MdlId, noko2CamBit & work->noko2CamBit);
    Hu3DModelCameraSet(work->rocketMdlId, rocketCamBit & work->rocketCamBit);
    Hu3DModelCameraSet(work->bgMdlId[0], skyCamBit);
    Hu3DModelCameraSet(work->bgMdlId[1], spaceCamBit);
    Hu3DModelCameraSet(work->sunMdlId, sunCamBit);
    Hu3DModelCameraSet(work->glowShadowMdlId, stageCamBit);
    Hu3DModelCameraSet(work->mameVineShadowMdlId, stageCamBit);
    if(stageCamBit) {
        Hu3DModelDispOn(work->stageMdlId);
        Hu3DModelDispOn(work->windmillMdlId);
        Hu3DModelDispOn(work->moundMdlId);
        Hu3DModelDispOn(work->glowShadowMdlId);
        Hu3DModelDispOn(work->mameVineShadowMdlId);
    } else {
        Hu3DModelDispOff(work->stageMdlId);
        Hu3DModelDispOff(work->windmillMdlId);
        Hu3DModelDispOff(work->moundMdlId);
        Hu3DModelDispOff(work->glowShadowMdlId);
        Hu3DModelDispOff(work->mameVineShadowMdlId);
    }
    if(moonCamBit) {
        Hu3DModelDispOn(work->moonMdlId);
    } else {
        Hu3DModelDispOff(work->moonMdlId);
    }
    if(auroraCamBit) {
        Hu3DModelDispOn(work->auroraMdlId);
    } else {
        Hu3DModelDispOff(work->auroraMdlId);
    }
    if(noko2CamBit) {
        Hu3DModelDispOn(work->noko2MdlId);
    } else {
        Hu3DModelDispOff(work->noko2MdlId);
    }
    if(rocketCamBit) {
        Hu3DModelDispOn(work->rocketMdlId);
    } else {
        Hu3DModelDispOff(work->rocketMdlId);
    }
    if(skyCamBit) {
        Hu3DModelDispOn(work->bgMdlId[0]);
    } else {
        Hu3DModelDispOff(work->bgMdlId[0]);
    }
    if(spaceCamBit) {
        Hu3DModelDispOn(work->bgMdlId[1]);
    } else {
        Hu3DModelDispOff(work->bgMdlId[1]);
    }
    if(sunCamBit) {
        Hu3DModelDispOn(work->sunMdlId);
    } else {
        Hu3DModelDispOff(work->sunMdlId);
    }
    for(i=0; i<4; i++) {
        Hu3DModelCameraSet(work->kumoMdlId[i], kumoCamBit);
    }
    for(i=0; i<2; i++) {
        Hu3DModelCameraSet(work->starMdlId[i], starCamBit);
    }
    M506KumoUpdate(work);
    M506StarUpdate(work);
    M506Noko2Update(work);
    M506RocketUpdate();
    M506StarfighterUpdate();
}