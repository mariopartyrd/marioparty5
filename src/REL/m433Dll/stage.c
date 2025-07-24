#include "REL/m433Dll.h"

#define STAGEOBJ_MAX 12
#define STAGEOBJ_MODE_NORMAL 0
#define STAGEOBJ_MODE_TURN 1
#define STAGEOBJ_MODE_DIVE 2

typedef struct StageObj_s {
    u8 modelId;
    u8 motId;
    u8 motIdPrev;
    u8 origMode;
    u8 mode;
    float speed;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    HuVecF homePos;
    HuVecF targetRot;
    HuVecF vel;
    union {
       struct {
           float angleSpeed;
           float angle;
           float unk5C;
           float maxSpeed;
       };
       s16 workBuf[32];
    };
} STAGEOBJ;

typedef struct SunEffWork_s {
    HU3DMODELID modelId;
    s16 no;
    s16 mode;
    s16 work[33];
} SUNEFFWORK;

typedef struct StageCamera_s {
    u8 disableF;
    HuVecF pos;
    HuVecF rot;
    HuVecF startPos;
    HuVecF startRot;
    HuVecF endPos;
    HuVecF endRot;
    HuVecF unk4C[19];
    float zoom;
    float startZoom;
    float endZoom;
    float unk13C;
    float speed;
    float maxTime;
    float time;
    float centerRadius[3];
    float centerSpeed[3];
    float centerRot[3];
    float centerRotSpeed[3];
    float centerPos[3];
    u16 centerTime[3];
} STAGECAMERA;

#define STAGE_MALLOC_MAX 32

static OMOBJ *cameraObj;
static OMOBJ *stageObj;
static OMOBJ *guideObj;
static void *stageMallocTbl[STAGE_MALLOC_MAX];
static STAGEOBJ stageObjBird[STAGEOBJ_MAX];
static BOOL guideKuriboF;
static BOOL lbl_1_bss_40;

static HuVecF lbl_1_data_68[2] = {
    { 0, 5500, -3950 },
    { 0, -0.7f, 0.6f },
};

static GXColor lbl_1_data_80 = { 255, 255, 255, 255 };

static HuVecF lbl_1_data_84[3] = {
    { 0, 5500, -3950 },
    { 0, 1, 0 },
    { 0, 0, 0 },
};

void M433StageObjCreate(OMOBJ *obj);
void M433GuideCreate(OMOBJ *obj);
void M433StageCameraCreate(OMOBJ *obj);
void M433StageMallocInit(void);
void *M433StageMalloc(int size);
void M433StageFree(void *ptr);

void M433StageMallocClose(void);

void M433StageInit(OMOBJMAN *om)
{
    int i;
    int num;
    guideKuriboF = TRUE;
    stageObj = omAddObj(om, 30, 8, 0, M433StageObjCreate);
    guideObj = omAddObj(om, 31, 3, 6, M433GuideCreate);
    cameraObj = omAddObj(om, 29, 1, 0, M433StageCameraCreate);
    num = frand() & 0x1F;
    for(i=0; i<num; i++) {
        M433StageRandom();
    }
    M433StageMallocInit();
}

void M433StageClose(void)
{
    M433StageMallocClose();
}

void M433StartCamera(OMOBJ *obj);

OMOBJFUNC M433StartCameraCreate(OMOBJMAN *om, OMOBJ *obj)
{
    M433_GAMEWORK *work = obj->data;
    CRot.x = 54.3f;
    CRot.y = 22.3f;
    CRot.z = 0.0f;
    Center.x = -130.1f;
    Center.y = 885.0f;
    Center.z = -900.3f;
    CZoom = 1100.0f;
    M433StageCameraFocusSet(0, &Center, &CRot, CZoom);
    obj->work[0] = 180;
    work->cameraMode = M433_CAMERAMODE_START;
    return M433StartCamera;
}

void M433StartCamera(OMOBJ *obj)
{
    Vec pos;
    Vec rot;
    float zoom;
    M433_GAMEWORK *work = obj->data;

    M433ModeWatch(obj);
    if (obj->work[0] == 120.0f) {
        pos.x = 0.0f;
        pos.y = -10.0f;
        pos.z = -207.5f;
        rot.x = -26.5f;
        rot.y = 0.0f;
        rot.z = 0.0f;
        zoom = 1400.0f;
        M433StageCameraFocusSet(90.0f, &pos, &rot, zoom);
    }
    if (obj->work[0] == 48.0f) {
        pos.x = 0.0f;
        pos.y = 200.0f;
        pos.z = 0.0f;
        rot.x = -30.0f;
        rot.y = 0.0f;
        rot.z = 0.0f;
        zoom = 1900.0f;
        M433StageCameraFocusSet(obj->work[0], &pos, &rot, zoom);
    }
    if (--obj->work[0] == 0) {
        work->cameraMode = M433_CAMERAMODE_GAME;
        M433ModeUpdate(obj);
        cameraObj->objFunc = NULL;
    }
}

HU3DMODELID M433SunEffCreate(int dataNum, s16 num, HU3DPARTICLEHOOK hook);
void M433SunEffHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);
void M433StageObjMain(OMOBJ *obj);

void M433StageObjCreate(OMOBJ *obj)
{
    HuVecF birdPos[16] = {
        { 475, 800, -1400 },
        { -300, 700, -1400 },
        { 300, 875, -1450 },
        { -575, 850, -1550 },
        { 350, 750, -1500 },
        { -450, 775, -1550 },
        { 325, 725, -1450 },
        { -525, 725, -1500 },
        { 575, 850, -1550 },
        { -350, 750, -1500 },
        { 450, 775, -1550 },
        {-325, 725, -1450 },
        { 525, 725, -1500 },
        { -475, 800, -1400 },
        { 300, 700, -1400 },
        { -300, 875, -1450 }
    };
    int modelId;
    SUNEFFWORK *data;
    int i, j;
    
    Hu3DBGColorSet(0, 0, 255);
    modelId = Hu3DModelCreateData(M433_HSF_stage);
    obj->mdlId[0] = modelId;
    modelId = Hu3DModelCreateData(M433_HSF_stage_bg);
    obj->mdlId[1] = modelId;
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(modelId, 0.5f);
    modelId = Hu3DModelCreateData(M433_HSF_stage_yasi);
    obj->mdlId[2] = modelId;
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    modelId = M433SunEffCreate(M433_ANM_sun1, 1, M433SunEffHook);
    obj->mdlId[3] = modelId;
    Hu3DParticleBlendModeSet(modelId, HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DModelLayerSet(modelId, 6);
    data = ((HU3DPARTICLE *)Hu3DData[modelId].hookData)->work;
    data->no = 0;
    modelId = M433SunEffCreate(M433_ANM_sun4, 10, M433SunEffHook);
    obj->mdlId[4] = modelId;
    Hu3DParticleBlendModeSet(modelId, HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DModelLayerSet(modelId, 6);
    data = ((HU3DPARTICLE *)Hu3DData[modelId].hookData)->work;
    data->no = 1;
    modelId = M433SunEffCreate(M433_ANM_sun3, 4, M433SunEffHook);
    obj->mdlId[5] = modelId;
    Hu3DParticleBlendModeSet(modelId, HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DModelLayerSet(modelId, 6);
    data = ((HU3DPARTICLE *)Hu3DData[modelId].hookData)->work;
    data->no = 2;
    modelId = M433SunEffCreate(M433_ANM_sun2, 6, M433SunEffHook);
    obj->mdlId[6] = modelId;
    Hu3DParticleBlendModeSet(modelId, HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DModelLayerSet(modelId, 6);
    data = ((HU3DPARTICLE *)Hu3DData[modelId].hookData)->work;
    data->no = 3;
    for(i=0; i<STAGEOBJ_MAX; i++) {
        STAGEOBJ *bird = &stageObjBird[i];
        modelId = Hu3DModelCreateData(M433_HSF_bird);
        bird->modelId = modelId;
        bird->pos.x = birdPos[i].x;
        bird->pos.y = birdPos[i].y-300;
        bird->pos.z = birdPos[i].z+800;
        Hu3DModelPosSet(modelId, bird->pos.x, bird->pos.y, bird->pos.z);
        Hu3DMotionSpeedSet(modelId, 6);
        bird->origMode = bird->mode = STAGEOBJ_MODE_NORMAL;
        bird->rot.x = bird->rot.y = bird->rot.z = 0;
        bird->scale.x = bird->scale.y = bird->scale.z = 1;
        bird->vel.x = bird->vel.y = bird->vel.z = 0;
        bird->homePos.x = bird->pos.x;
        bird->homePos.y = bird->pos.y;
        bird->homePos.z = bird->pos.z;
        if(i & 1) {
            bird->rot.y = 90;
            bird->pos.y += 650.0;
            bird->pos.x -= 1000.0+(200.0f*M433StageSRandF());
        } else {
            bird->rot.y = 90;
            bird->pos.x += 200.0f*M433StageSRandF();
        }
        bird->rot.y += 10.0f*M433StageSRandF();
        bird->pos.z -= 400.0;
        bird->speed = 100.0f*(0.1f+(0.02f*M433StageRandF()));
        for(j=0; j<32; j++) {
            bird->workBuf[j] = 0;
        }
        bird->maxSpeed = bird->speed;
    }
    HuAudFXPlay(MSM_SE_M433_01);
    obj->objFunc = M433StageObjMain;
}

void M433StageObjMain(OMOBJ *obj)
{
    Mtx rotMtx;
    Vec rot;
    STAGEOBJ *bird;
    s32 i;

    for (i=0; i<STAGEOBJ_MAX; i++) {
        bird = &stageObjBird[i];
        switch (bird->mode) {
            case STAGEOBJ_MODE_NORMAL:
                if (bird->speed < bird->maxSpeed) {
                    bird->speed += 0.4f;
                }
                if ((s32)(64.0f * M433StageRandF()) == 0) {
                    Hu3DMotionTimeSet(bird->modelId, 0.0f);
                }
                if (Hu3DMotionEndCheck(bird->modelId) != 0) {
                    bird->vel.y = -3.8933334f;
                }
                else {
                    bird->vel.y = -1.2166667f;
                }
                bird->angleSpeed += 0.1f * M433StageSRandF();
                bird->angle += bird->angleSpeed;
                if (bird->angle > 180.0f) {
                    bird->angle -= 180.0f;
                }
                if (bird->angle < -180.0f) {
                    bird->angle += 180.0f;
                }
                bird->rot.x = -30.0 * HuSin(bird->angle) * HuSin(bird->rot.y);
                bird->rot.z = -30.0 * HuSin(bird->angle) * HuCos(bird->rot.y);
                if (bird->pos.y <= 500.0f || fabs(bird->pos.x) > 1600.0) {
                    bird->vel.y = 0.0f;
                    bird->workBuf[6] = 0xB4;
                    bird->speed *= 0.75f;
                    bird->mode++;
                }
                break;

            case STAGEOBJ_MODE_TURN:
                bird->rot.y += 1.0f;
                bird->rot.x = 0.0f;
                bird->rot.z = 60.0 * HuSin(bird->workBuf[6]) * HuCos(bird->rot.y);
                if (--bird->workBuf[6] == 0) {
                    Hu3DModelAttrSet(bird->modelId, HU3D_MOTATTR_LOOP);
                    Hu3DMotionSpeedSet(bird->modelId, 6.0f);
                    bird->vel.y = 4.866667f;
                    bird->mode++;
                }
                break;

            case STAGEOBJ_MODE_DIVE:
                if (bird->speed > 4.0f) {
                    bird->speed -= 0.1f;
                    bird->vel.y += 0.73f;
                }
                bird->vel.y += -0.5475f;
                if (bird->vel.y < 0.0f) {
                    Hu3DModelAttrReset(bird->modelId, HU3D_MOTATTR_LOOP);
                    Hu3DMotionSpeedSet(bird->modelId, 2.0f);
                    bird->maxSpeed = 100.0f * (0.1f + (0.02f * M433StageRandF()));
                    bird->mode = STAGEOBJ_MODE_NORMAL;
                }
                break;
        }
        bird->vel.x = bird->speed * HuSin(bird->rot.y);
        bird->vel.z = bird->speed * HuCos(bird->rot.y);
        bird->pos.x += bird->vel.x;
        bird->pos.y += bird->vel.y;
        bird->pos.z += bird->vel.z;
        if (bird->pos.z >= -500.0f) {
            bird->pos.z = -500.0f;
        }
        Hu3DModelPosSet(bird->modelId, bird->pos.x, bird->pos.y, bird->pos.z);
        M433StageRotMtxCalc(rotMtx, bird->rot.x, -bird->rot.y, bird->rot.z);
        M433StageMtxRotGet(rotMtx, &rot);
        Hu3DModelRotSetV(bird->modelId, &rot);
    }
}

void M433GuideMain(OMOBJ *obj);

#define GUIDE_MOT_IDLE 0
#define GUIDE_MOT_READY 1
#define GUIDE_MOT_JUMP 2
#define GUIDE_MOT_RUN 3

void M433GuideCreate(OMOBJ *obj)
{
    int motFile[4] = {
        M433_HSF_kuribo_idle,
        M433_HSF_kuribo_ready,
        M433_HSF_kuribo_jump,
        M433_HSF_kuribo_run
    };
    STAGEOBJ *work;
    int modelId;
    int i;
    
    obj->data = work = M433StageMalloc(sizeof(STAGEOBJ));
    modelId = Hu3DModelCreateData(M433_HSF_kuribo);
    obj->mdlId[0] = modelId;
    for(i=0; i<4; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(modelId, motFile[i]);
    }
    Hu3DMotionSet(modelId, obj->mtnId[0]);
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    work->motId = work->motIdPrev = 0;
    work->mode = 0;
    work->workBuf[0] = 0;
    work->pos.x = 0;
    work->pos.y = 180;
    work->pos.z = -490;
    work->homePos = work->pos;
    work->rot.x = work->rot.y = work->rot.z = 0;
    work->targetRot = work->rot;
    omSetTra(obj, work->pos.x, work->pos.y, work->pos.z);
    omSetRot(obj, work->rot.x, work->rot.y, work->rot.z);
    obj->objFunc = M433GuideMain;
}

float M433StageAngleLerp(float start, float end, float time);

void M433GuideMain(OMOBJ *obj)
{
    STAGEOBJ *work;
    u32 attr;
    int modelId;

    work = obj->data;
    modelId = obj->mdlId[0];
    switch (work->mode) {
        case M433_GUIDEMODE_IDLE:
            work->motId = GUIDE_MOT_IDLE;
            attr = HU3D_MOTATTR_LOOP;
            break;
            
        case M433_GUIDEMODE_READY:
            work->targetRot.y = 0.0f;
            work->motId = GUIDE_MOT_READY;
            attr = HU3D_MOTATTR_LOOP;
            if (work->workBuf[0]++ >= 90) {
                work->workBuf[0] = 0;
                work->mode = M433_GUIDEMODE_IDLE;
            }
            break;
            
        case M433_GUIDEMODE_JUMP:
            work->targetRot.y = 0.0f;
            work->motId = GUIDE_MOT_JUMP;
            attr = HU3D_MOTATTR_NONE;
            if (Hu3DMotionEndCheck(modelId) != 0) {
                work->mode = M433_GUIDEMODE_IDLE;
            }
            break;
            
        case M433_GUIDEMODE_RUN:
            work->targetRot.y = 0.0f;
            work->motId = GUIDE_MOT_RUN;
            attr = HU3D_MOTATTR_LOOP;
            break;
            
        case M433_GUIDEMODE_FACE_CENTER:
            work->targetRot.y = 0.0f;
            work->mode = M433_GUIDEMODE_IDLE;
            break;
            
        case M433_GUIDEMODE_FACE_RIGHT:
            work->targetRot.y = 50.0f;
            work->mode = M433_GUIDEMODE_IDLE;
            break;
            
        case M433_GUIDEMODE_FACE_LEFT:
            work->targetRot.y = -50.0f;
            work->mode = M433_GUIDEMODE_IDLE;
            break;
    }
    if (work->motId != work->motIdPrev) {
        work->motIdPrev = work->motId;
        Hu3DMotionShiftSet(modelId, obj->mtnId[work->motId], 0.0f, 8.0f, attr);
        switch (work->motId) {
            case GUIDE_MOT_READY:
                break;
            case GUIDE_MOT_JUMP:
                break;
        }
    }
    work->rot.y = M433StageAngleLerp(work->rot.y, work->targetRot.y, 0.9f);
    omSetTra(obj, work->pos.x, work->pos.y, work->pos.z);
    omSetRot(obj, work->rot.x, work->rot.y, work->rot.z);
}

#undef GUIDE_MOT_IDLE
#undef GUIDE_MOT_READY
#undef GUIDE_MOT_JUMP
#undef GUIDE_MOT_RUN

HU3DMODELID M433SunEffCreate(int dataNum, s16 num, HU3DPARTICLEHOOK hook)
{
    ANIMDATA *animP = HuSprAnimRead(HuDataReadNum(dataNum, HU_MEMNUM_OVL));
    int modelId = Hu3DParticleCreate(animP, num);
    SUNEFFWORK *sunWork;
    HU3DPARTICLE *particleP;
    Hu3DParticleHookSet(modelId, hook);
    particleP = Hu3DData[modelId].hookData;
    sunWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(SUNEFFWORK), HU_MEMNUM_OVL);
    particleP->work = sunWork;
    sunWork->modelId = modelId;
    sunWork->mode = 0;
    return modelId;
    
}

void M433SunEffHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    float scale;
    float radius;
    float size;
    float spacing;
    HU3DPARTICLEDATA *dataP;
    int i;
    float baseScale[4] = { 1000.0f, 200.0f, 600.0f, 1200.0f };
    float baseAlpha[4] = { 1.0f, 0.6f, 0.3f, 0.425f };
    Vec cameraDir;
    Vec cameraOfs;
    Vec basePos2;
    Vec basePos1;
    s16 temp = 0;
    SUNEFFWORK *sunWork = particleP->work;
    switch (sunWork->mode) {
        case 0:
            particleP->dataCnt = 84;
            dataP = particleP->data;
            if (sunWork->no) {
                basePos1.x = 0.0f;
                basePos1.y = -4700.0f;
                basePos1.z = 4450.0f;
                scale = 1.0f / HuMagVecF(&basePos1);
                basePos1.x *= scale;
                basePos1.y *= scale;
                basePos1.z *= scale;
                spacing = 4000.0f / particleP->maxCnt;
                for(i=0; i<particleP->maxCnt; i++, dataP++) {
                    radius = 500.0f + ((spacing * i) + (0.25f * spacing * M433StageRandF()));
                    size = radius / 4000.0f;
                    dataP->accel.x = radius;
                    dataP->pos.x = radius * basePos1.x;
                    dataP->pos.y = 5500.0f + (radius * basePos1.y);
                    dataP->pos.z = -3950.0f + (radius * basePos1.z);
                    dataP->scale = (0.3f + (0.7f * (size * size))) * (baseScale[sunWork->no] * (0.7f + (0.3f * M433StageRandF())));
                    dataP->color.r = (s32)(160.0f * M433StageRandF()) + 32;
                    dataP->color.g = (s32)(96.0f * M433StageRandF()) + 32;
                    dataP->color.b = (s32)(128.0f * M433StageRandF()) + 32;
                    dataP->color.a = ((1.0 - (0.4 * size)) * (255.0f * baseAlpha[sunWork->no])) - (s32)(16.0f * M433StageRandF());
                }
                sunWork->mode++;
            } else {
                dataP->pos.x = 0.0f;
                dataP->pos.y = 5500.0f;
                dataP->pos.z = -3950.0f;
                dataP->scale = 1000.0f;
            }
            break;

        case 1:
            cameraDir.x = HuSin(CRot.y) * HuCos(CRot.x);
            cameraDir.y = -HuSin(CRot.x);
            cameraDir.z = HuCos(CRot.y) * HuCos(CRot.x);
            cameraOfs.x = -(Center.x + (1.25f * (cameraDir.x * CZoom)));
            cameraOfs.y = 5500.0f - (Center.y + (cameraDir.y * CZoom));
            cameraOfs.z = -3950.0f - (Center.z + (1.25f * (cameraDir.z * CZoom)));
            scale = 1.0f / HuMagVecF(&cameraOfs);
            cameraOfs.x *= scale;
            cameraOfs.y *= scale;
            cameraOfs.z *= scale;
            scale = -((-cameraOfs.z * -cameraDir.z) + ((-cameraOfs.x * -cameraDir.x) + (-cameraOfs.y * -cameraDir.y)));
            basePos2.x = -cameraDir.x + (-2.0f * cameraOfs.x * scale);
            basePos2.y = -cameraDir.y + (-2.0f * cameraOfs.y * scale);
            basePos2.z = -cameraDir.z + (-2.0f * cameraOfs.z * scale);
            for (dataP = particleP->data, i = 0; i < particleP->maxCnt; i++, dataP++) {
                dataP->pos.x = basePos2.x * dataP->accel.x;
                dataP->pos.y = 5500.0f + (basePos2.y * dataP->accel.x);
                dataP->pos.z = -3950.0f + (basePos2.z * dataP->accel.x);
                if (particleP->dataCnt < 10) {
                    dataP->color.a *= 0.8f;
                }
            }
            particleP->dataCnt--;
            if (particleP->dataCnt == 0) {
                Hu3DModelAttrReset(sunWork->modelId, HU3D_ATTR_PARTICLE);
                Hu3DModelKill(sunWork->modelId);
                return;
            }
            break;
    }
    DCFlushRange(particleP->data, particleP->maxCnt * sizeof(HU3DPARTICLEDATA));
}

void M433StageCameraMain(OMOBJ *obj);

void M433StageCameraCreate(OMOBJ *obj)
{
    STAGECAMERA *work = M433StageMalloc(sizeof(STAGECAMERA));
    obj->data = work;
    work->time = work->maxTime = 0;
    work->speed = 1;
    work->rot.x = work->startRot.x = -30;
    work->rot.y = work->startRot.y = 0;
    work->rot.z = work->startRot.z = 0;
    work->pos.x = work->startPos.x = 0;
    work->pos.y = work->startPos.y = 200;
    work->pos.z = work->startPos.z = 0;
    work->zoom = work->startZoom = 1900;
    work->centerTime[0] = work->centerTime[1] = work->centerTime[2] = 0;
    obj->objFunc = M433StageCameraMain;
}

void M433StageCameraMain(OMOBJ *obj)
{
    float t;
    int i;
    STAGECAMERA *work = obj->data;
    if (work->time > 0.0f) {
        work->time = work->time - work->speed;
        t = (work->maxTime - work->time) / work->maxTime;
        t = HuSin(90.0f*t);
        if (work->disableF == FALSE) {
            work->pos.x = work->startPos.x + (t*(work->endPos.x - work->startPos.x));
            work->pos.y = work->startPos.y + (t*(work->endPos.y - work->startPos.y));
            work->pos.z = work->startPos.z + (t*(work->endPos.z - work->startPos.z));
            work->rot.x = work->startRot.x + (t*(work->endRot.x - work->startRot.x));
            work->rot.y = work->startRot.y + (t*(work->endRot.y - work->startRot.y));
            work->rot.z = work->startRot.z + (t*(work->endRot.z - work->startRot.z));
            work->zoom = work->startZoom + (t*(work->endZoom - work->startZoom));
        }
    }
    CRot.x = work->rot.x;
    CRot.y = work->rot.y;
    CRot.z = work->rot.z;
    Center.x = work->pos.x;
    Center.y = work->pos.y;
    Center.z = work->pos.z;
    CZoom = work->zoom;
    for(i=0; i<3; i++) {
        work->centerPos[i] = 0.0f;
        if(work->centerTime[i] != 0) {
            work->centerPos[i] = work->centerRadius[i] * HuSin(work->centerRot[i]);
            work->centerRot[i] += work->centerRotSpeed[i];
            work->centerRadius[i] *= work->centerSpeed[i];
            work->centerTime[i] -= 1;
        }
    }
    Center.x += work->centerPos[0];
    Center.y += work->centerPos[1];
    Center.z += work->centerPos[2];
}

void M433StageCameraFocusSet(float time, Vec *pos, Vec *rot, float zoom)
{
    STAGECAMERA *work = cameraObj->data;
    work->disableF = FALSE;
    work->maxTime = time;
    work->time = time;
    if (time == 0.0f) {
        work->pos.x = pos->x;
        work->pos.y = pos->y;
        work->pos.z = pos->z;
        work->rot.x = rot->x;
        work->rot.y = rot->y;
        work->rot.z = rot->z;
        work->zoom = zoom;
        return;
    }
    work->startPos = work->pos;
    work->endPos.x = pos->x;
    work->endPos.y = pos->y;
    work->endPos.z = pos->z;
    work->startRot = work->rot;
    work->endRot.x = rot->x;
    work->endRot.y = rot->y;
    work->endRot.z = rot->z;
    work->startZoom = work->zoom;
    work->endZoom = zoom;
}

void M433StageCameraSpeedSet(float speed)
{
    STAGECAMERA *work = cameraObj->data;
    work->speed = speed;
}

void M433StageCameraOfsSet(u8 no, u16 time, float radius, float speed, float rotSpeed)
{
    STAGECAMERA *work = cameraObj->data;
    work->centerTime[no] = time;
    work->centerRadius[no] = radius;
    work->centerSpeed[no] = speed;
    work->centerRotSpeed[no] = rotSpeed;
    work->centerRot[no] = 90.0f;
}

void M433StageMallocInit(void)
{
    int i;
    for(i=0; i<STAGE_MALLOC_MAX; i++) {
        stageMallocTbl[i] = NULL;
    }
}

void *M433StageMalloc(int size)
{
    int i;
    for(i=0; i<STAGE_MALLOC_MAX; i++) {
        if(!stageMallocTbl[i]) {
            break;
        }
    }
    if(i == STAGE_MALLOC_MAX) {
        return NULL;
    }
    stageMallocTbl[i] = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    return stageMallocTbl[i];
}

void M433StageFree(void *ptr)
{
    int i;
    for(i=0; i<STAGE_MALLOC_MAX; i++) {
        if(stageMallocTbl[i] == ptr) {
            break;
        }
    }
    if(i == STAGE_MALLOC_MAX) {
        return;
    }
    HuMemDirectFree(stageMallocTbl[i]);
    stageMallocTbl[i] = NULL;
}

void M433StageMallocClose(void)
{
    int i;
    for(i=0; i<STAGE_MALLOC_MAX; i++) {
        if(stageMallocTbl[i]) {
            HuMemDirectFree(stageMallocTbl[i]);
            stageMallocTbl[i] = NULL;
        }
    }
}

int M433StageRandom(void)
{
    static unsigned int rnd_seed = 0x41C64E6D;
    rnd_seed *= 0x41C64E6D;
    rnd_seed += 0x3039;
    return rnd_seed >> 16;
}

float M433StageAngleLerp(float start, float end, float time)
{
    float diff;

    if (start > 180.0f) {
        start -= 360.0f;
    }
    else if (start <= -180.0f) {
        start += 360.0f;
    }
    if (end > 180.0f) {
        end -= 360.0f;
    }
    else if (end <= -180.0f) {
        end += 360.0f;
    }
    diff = start - end;
    if (diff > 180.0f) {
        diff -= 360.0f;
    }
    else if (diff <= -180.0f) {
        diff += 360.0f;
    }
    start = end + (diff * time);
    if (start > 180.0f) {
        return start - 360.0f;
    }
    if (start <= -180.0f) {
        start += 360.0f;
    }
    return start;
}

void M433StageRotMtxCalc(Mtx matrix, float x, float y, float z)
{
    Mtx mtxRotX;
    Mtx mtxRotY;

    if(z != 0.0f) {
        MTXRotRad(matrix, 'Z', MTXDegToRad(z));
    } else {
        MTXIdentity(matrix);
    }
    if(x != 0.0f) {
        MTXRotRad(mtxRotX, 'X', MTXDegToRad(x));
        MTXConcat(mtxRotX, matrix, matrix);
    }
    if(y != 0.0f) {
        MTXRotRad(mtxRotY, 'Y', MTXDegToRad(y));
        MTXConcat(mtxRotY, matrix, matrix);
    }
}

void M433StageMtxRotGet(Mtx matrix, HuVecF *result)
{
    float z;
    float mag;

    result->x = HuAtan(matrix[1][2], matrix[2][2]);
    if (result->x < 0.0f) {
        result->x += 360.0f;
    }
    result->z = HuAtan(matrix[0][1], matrix[0][0]);
    if (result->z < 0.0f) {
        result->z += 360.0f;
    }
    z = -matrix[0][2];
    mag = sqrtf((1.0 - (z * z)) < 0.0 ? (-(1.0 - (z * z))) : (1.0 - (z * z)));
    if ((result->x > 90.0f) && (result->x < 270.0f) && (result->z > 90.0f) && (result->z < 270.0f)) {
        result->x = fmod(180.0f + result->x, 360.0);
        result->z = fmod(180.0f + result->z, 360.0);
        mag = -mag;
    }
    result->y = HuAtan(z, mag);
    if (result->y < 0.0f) {
        result->y += 360.0f;
    }
}

void M433GuideModeSet(s16 mode)
{
    STAGEOBJ *work = guideObj->data;
    work->mode = mode;
}