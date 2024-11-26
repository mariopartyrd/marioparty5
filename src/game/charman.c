#include "game/charman.h"
#include "game/process.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/sprite.h"

#include "datanum/effect.h"

#define EFFECT_WARN 0
#define EFFECT_DUST 1
#define EFFECT_SMOKE 2
#define EFFECT_STAR 3
#define EFFECT_GLOW 4
#define EFFECT_HIT 5
#define EFFECT_BIRD 6
#define EFFECT_CRY 7
#define EFFECT_LANDDUST 8
#define EFFECT_MAX 9

typedef struct CharWork_s {
    HU3DMODELID modelId;
    s16 model;
    s16 motNoCurr;
    s16 motNoShiftCurr;
    s16 motNoPrev;
    s16 motNoShiftPrev;
    HU3DMOTID motId[CHAR_MOT_MAX];
    s16 motNoTbl[CHAR_MOT_MAX];
    s16 vol;
    s16 pan;
    u8 voiceFlag[CHAR_MOT_MAX];
    u32 attr;
    s8 stepFx;
    HuVecF pos;
    AMEM_PTR motAMemP;
    HUPROCESS *process;
    s16 timingHookNo;
    s16 motNoPlay[2];
    s16 motPlayTime[2];
} CHARWORK;

typedef struct EffectData_s {
    unsigned int dataNum;
    s16 maxCnt;
    s16 blendMode;
    s32 motCnt;
    unsigned int motDataNum[16];
} EFFECTDATA;

typedef struct EffectParam_s {
    u32 attr;
    GXColor colorBegin;
    GXColor colorEnd;
    HuVecF vel;
    HuVecF velDecay;
    float gravity;
    u32 zero;
    float scaleVel;
    float alphaBase;
    float colorWeight;
} EFFECTPARAM;

static EFFECTDATA effectDataTbl[] = {
    {
        EFFECT_ANM_danger,
        10,
        HU3D_PARTICLE_BLEND_NORMAL,
        2,
        {CHARMOT_HSF_c000m1_326, CHARMOT_HSF_c000m1_327}
    },
    {
        EFFECT_ANM_dust,
        150,
        HU3D_PARTICLE_BLEND_ADDCOL,
        0,
        { 
            CHARMOT_HSF_c000m1_301,
            CHARMOT_HSF_c000m1_302,
            CHARMOT_HSF_c000m1_355,
            CHARMOT_HSF_c000m1_326,
            CHARMOT_HSF_c000m1_343,
            CHARMOT_HSF_c000m1_378
        }
    },
    {
        EFFECT_ANM_smoke,
        70,
        HU3D_PARTICLE_BLEND_NORMAL,
        0,
        { 
            CHARMOT_HSF_c000m1_321
        }
    },
    {
        EFFECT_ANM_star,
        300,
        HU3D_PARTICLE_BLEND_ADDCOL,
        0,
        { 
            CHARMOT_HSF_c000m1_310
        }
    },
    {
        EFFECT_ANM_glow,
        300,
        HU3D_PARTICLE_BLEND_ADDCOL,
        0,
        {}
    },
    {
        EFFECT_ANM_circle,
        200,
        HU3D_PARTICLE_BLEND_ADDCOL,
        0,
        { 
            CHARMOT_HSF_c000m1_305
        }
    },
    {
        EFFECT_ANM_bird,
        12,
        HU3D_PARTICLE_BLEND_NORMAL,
        2,
        { 
            CHARMOT_HSF_c000m1_322,
            CHARMOT_HSF_c000m1_323,
            CHARMOT_HSF_c000m1_391
        }
    },
    {
        EFFECT_ANM_cry,
        100,
        HU3D_PARTICLE_BLEND_ADDCOL,
        0,
        { 
            CHARMOT_HSF_c000m1_354,
            CHARMOT_HSF_c000m1_355
        }
    },
    {
        EFFECT_ANM_landDust,
        100,
        HU3D_PARTICLE_BLEND_NORMAL,
        0,
        { 
            CHARMOT_HSF_c000m1_310,
            CHARMOT_HSF_c000m1_304
        }
    },
};

unsigned int charDataDirTbl[CHARNO_MAX][6] = {
    {
        DATA_mariomdl0,
        DATA_mariomdl1,
        DATA_mariomdl2,
        DATA_mariomdl3,
        DATA_mariomot,
        DATA_mario
    },
    {
        DATA_luigimdl0,
        DATA_luigimdl1,
        DATA_luigimdl2,
        DATA_luigimdl3,
        DATA_luigimot,
        DATA_luigi
    },
    {
        DATA_peachmdl0,
        DATA_peachmdl1,
        DATA_peachmdl2,
        DATA_peachmdl3,
        DATA_peachmot,
        DATA_peach
    },
    {
        DATA_yoshimdl0,
        DATA_yoshimdl1,
        DATA_yoshimdl2,
        DATA_yoshimdl3,
        DATA_yoshimot,
        DATA_yoshi
    },
    {
        DATA_wariomdl0,
        DATA_wariomdl1,
        DATA_wariomdl2,
        DATA_wariomdl3,
        DATA_wariomot,
        DATA_wario
    },
    {
        DATA_daisymdl0,
        DATA_daisymdl1,
        DATA_daisymdl2,
        DATA_daisymdl3,
        DATA_daisymot,
        DATA_daisy
    },
    {
        DATA_waluigimdl0,
        DATA_waluigimdl1,
        DATA_waluigimdl2,
        DATA_waluigimdl3,
        DATA_waluigimot,
        DATA_waluigi
    },
    {
        DATA_kinopiomdl0,
        DATA_kinopiomdl1,
        DATA_kinopiomdl2,
        DATA_kinopiomdl3,
        DATA_kinopiomot,
        DATA_kinopio
    },
    {
        DATA_teresamdl0,
        DATA_teresamdl1,
        DATA_teresamdl2,
        DATA_teresamdl3,
        DATA_teresamot,
        DATA_teresa
    },
    {
        DATA_minikoopamdl0,
        DATA_minikoopamdl1,
        DATA_minikoopamdl2,
        DATA_minikoopamdl3,
        DATA_minikoopamot,
        DATA_minikoopa
    },
    {
        DATA_minikoopaRmdl0,
        DATA_minikoopaRmdl1,
        DATA_minikoopaRmdl2,
        DATA_minikoopaRmdl3,
        DATA_minikoopamot,
        DATA_minikoopa
    },
    {
        DATA_minikoopaGmdl0,
        DATA_minikoopaGmdl1,
        DATA_minikoopaGmdl2,
        DATA_minikoopaGmdl3,
        DATA_minikoopamot,
        DATA_minikoopa
    },
    {
        DATA_minikoopaBmdl0,
        DATA_minikoopaBmdl1,
        DATA_minikoopaBmdl2,
        DATA_minikoopaBmdl3,
        DATA_minikoopamot,
        DATA_minikoopa
    }
};

static EFFECTPARAM dustEffParam = {
    0,
    { 128, 128, 128, 255 },
    { 64, 32, 0, 255 },
    { 0, 2, 1 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    1,
    -5,
    0.02f
};

static EFFECTPARAM hitEffParam = {
    0,
    { 255, 64, 64, 128 },
    { 255, 64, 64, 128 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    0,
    0,
    -5,
    0,
    0
};

static EFFECTPARAM npcHitEffParam = {
    0,
    { 255, 255, 0, 255 },
    { 255, 255, 0, 255 },
    { 0, 0, 0 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    -0.5,
    -10,
    0
};

static EFFECTPARAM warnEffParam = {
    0,
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 0, 20, 0 },
    { 0.95f, 0.85f, 0.95f },
    0,
    0,
    1,
    -5,
    0
};

static EFFECTPARAM smokeEffParam = {
    0,
    { 128, 32, 32, 255 },
    { 0, 0, 0, 255 },
    { 0, 10, 0 },
    { 1.0f, 0.95f, 1.0f },
    0,
    0,
    5,
    -13,
    0.1f
};

static s8 walkVoiceTimeTbl[CHARNO_MAX*4] = {
    10, 32, -1, -1,
    20, 40, -1, -1,
    12, 37, -1, -1,
    4, 25, -1, -1,
    5, 30, -1, -1,
    23, 50, -1, -1,
    19, 39, -1, -1,
    17, 38, -1, -1,
    10, 32, -1, -1,
    21, 44, -1, -1,
    21, 44, -1, -1,
    21, 44, -1, -1,
    21, 44, -1, -1
};

static s8 runVoiceTimeTbl[CHARNO_MAX*4] = {
    15, 29, -1, -1,
    5, 19, -1, -1,
    5, 22, -1, -1,
    12, 28, -1, -1,
    18, 35, -1, -1,
    0, 16, -1, -1,
    12, 28, -1, -1,
    14, 30, -1, -1,
    10, 32, -1, -1,
    16, 32, -1, -1,
    16, 32, -1, -1,
    16, 32, -1, -1,
    16, 32, -1, -1
};

static s8 jumpVoiceTimeTbl[CHARNO_MAX*2] = {
    5, 7,
    6, 9,
    10, 11,
    9, -1,
    9, 11,
    8, -1,
    10, -1,
    10, -1,
    10, -1,
    10, -1,
    10, -1,
    10, -1,
    10, -1
};

static u8 boardWinAnimLen[CHARNO_MAX] = {
    14,
    0,
    46,
    72,
    20,
    34,
    41,
    0,
    0,
    17,
    17,
    17,
    17
};

static u8 winAnimLen[CHARNO_MAX] = {
    18,
    0,
    30,
    106,
    0,
    0,
    24,
    24,
    90,
    60,
    60,
    60,
    60
};

static u8 handUpAnimLen[CHARNO_MAX] = {};

static u8 okAnimLen[CHARNO_MAX] = {
    0,
    0,
    24,
    90,
    30,
    0,
    18,
    42,
    96,
    30,
    30,
    30,
    30
};

static u8 winAnimLen2[CHARNO_MAX] = {
    18,
    0,
    30,
    106,
    0,
    0,
    24,
    24,
    90,
    60,
    60,
    60,
    60
};

static u8 holdUpAnimLen[CHARNO_MAX] = {
    0,
    6,
    0,
    78,
    0,
    16,
    50,
    0,
    54,
    38,
    38,
    38,
    38
};

static u8 winJumpUpAnimLen[CHARNO_MAX] = {};

static u8 loseAnimLen[CHARNO_MAX] = {
    18,
    0,
    36,
    106,
    0,
    0,
    24,
    24,
    90,
    60,
    60,
    60,
    60
};

static s8 jumpAnimLen[CHARNO_MAX] = {
    42,
    42,
    43,
    52,
    42,
    42,
    -1,
    42,
    42,
    42,
    42,
    42,
    42
};

static s32 lbl_80233D78[CHARNO_MAX][4];
static u16 dustFlags[CHAR_NPC_MAX];
static HUPROCESS *hookDustProc[CHAR_MOT_MAX];
static EFFECTPARAM *effParamAll[EFFECT_MAX];
static HU3DMODELID effModelId[EFFECT_MAX][HU3D_CAM_MAX];
static CHARWORK charWork[CHARNO_MAX];

static s16 effectLayer;
static BOOL motShiftF;
static AMEM_PTR effectAMemP;


static s16 specialWinVoiceTbl[][2] = {
    DLL_m566dll, CHARVOICEID(2),
    DLL_NONE, 0
};

void CharInit(void)
{
    s16 i;
    s16 j;
    for(i=0; i<CHARNO_MAX; i++) {
        CHARWORK *workP = &charWork[i];
        workP->motAMemP = 0;
        for(j=0; j<CHAR_MOT_MAX; j++) {
            workP->motId[j] = HU3D_MOTID_NONE;
        }
        workP->modelId = HU3D_MODELID_NONE;
        workP->process = NULL;
        workP->vol = MSM_VOL_MAX;
        workP->pan = MSM_PAN_CENTER;
    }
    if(!effectAMemP) {
        effectAMemP = HuAR_DVDtoARAM(DATA_effect);
    }
    for(i=0; i<EFFECT_MAX; i++) {
        for(j=0; j<HU3D_CAM_MAX; j++) {
            effModelId[i][j] = HU3D_MODELID_NONE;
        }
        effParamAll[i] = NULL;
    }
    for(i=0; i<CHAR_MOT_MAX; i++) {
        hookDustProc[i] = NULL;
    }
    effectLayer = 0;
}

AMEM_PTR CharMotionAMemPGet(s16 charNo)
{
    return charWork[charNo].motAMemP;
}

void CharMotionInit(s16 charNo)
{
    if(charNo >= CHARNO_MAX || charNo < 0 || charNo == (u8)CHARNO_NONE) {
        return;
    } else {
        CHARWORK *workP = &charWork[charNo];
        if(!workP->motAMemP) {
            workP->motAMemP = HuAR_DVDtoARAM(charDataDirTbl[charNo][4]);
        }
    }
}

void CharMotionInitAsync(s16 charNo)
{
    if(charNo >= CHARNO_MAX || charNo < 0 || charNo == (u8)CHARNO_NONE) {
        return;
    } else {
        CHARWORK *workP = &charWork[charNo];
        if(!workP->motAMemP) {
            s16 statId = HuDataDirReadAsync(charDataDirTbl[charNo][4]);
            while(!HuDataGetAsyncStat(statId)) {
                HuPrcVSleep();
            }
            workP->motAMemP = HuAR_MRAMtoARAM(charDataDirTbl[charNo][4]);
            while(HuARDMACheck()) {
                HuPrcVSleep();
            }
            HuDataDirClose(charDataDirTbl[charNo][4]);
        }
    }
}

void CharMotionClose(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    if(workP->motAMemP) {
        if(HuARDirCheck(charDataDirTbl[charNo][4])) {
            HuARFree(workP->motAMemP);
        }
        workP->motAMemP = 0;
    }
}

void CharDataClose(s16 charNo)
{
    s16 i;
    if(charNo == CHARNO_NONE) {
        for(i=0; i<CHARNO_MAX; i++) {
            CharDataClose(i);
        }
    } else {
        for(i=0; i<5; i++) {
            HuDataDirClose(charDataDirTbl[charNo][i]);
        }
        CharMotionClose(charNo);
    }
}

void CharClose(s16 charNo)
{
    CharModelKill(CHARNO_NONE);
    CharDataClose(CHARNO_NONE);
    HuARFree(effectAMemP);
    effectAMemP = 0;
}

static void CharTimingHook(HU3DMODELID modelId, HU3DMOTID motId, BOOL lagF);
static void UpdateChar(void);
static s16 GetStepFxNo(void);

HU3DMODELID CharModelCreate(s16 charNo, s16 model)
{
    s16 sp8 = 0;
    CHARWORK *workP = &charWork[charNo];
    unsigned int dataNum;
    HU3DMODELID modelId;
    s16 *property;
    void *dataP;

    if(workP->modelId != HU3D_MODELID_NONE) {
        Hu3DModelKill(workP->modelId);
    }
    if(model & CHAR_MODEL0) {
        dataNum = charDataDirTbl[charNo][0];
    } else if(model & CHAR_MODEL1) {
        dataNum = charDataDirTbl[charNo][1];
    } else if(model & CHAR_MODEL2) {
        dataNum = charDataDirTbl[charNo][2];
    } else {
        dataNum = charDataDirTbl[charNo][3];
    }
    dataP = HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
    workP->modelId = modelId = Hu3DModelCreate(dataP);
    workP->process = HuPrcCreate(UpdateChar, 99, 16384, 0);
    workP->process->property = property = HuMemDirectMalloc(HUHEAPTYPE_HEAP, sizeof(s16));
    workP->model = model;
    workP->attr = 0;
    *property = charNo;
    workP->stepFx = GetStepFxNo();
    Hu3DMotionTimingHookSet(modelId, CharTimingHook);
    workP->timingHookNo = 0;
    workP->motNoPlay[0] = workP->motNoPlay[1] = 32767;
    return modelId;
}

static void CharTimingHook(HU3DMODELID modelId, HU3DMOTID motId, BOOL lagF)
{
    CHARWORK *workP = &charWork[0];
    s16 i;
    for(i=0; i<CHARNO_MAX; i++, workP++) {
        if(workP->modelId == modelId) {
            workP->timingHookNo++;
            break;
        }
    }
}

static void UpdateMotPlay(CHARWORK *workP, s16 motNo, s16 motNoShift);
static void UpdateCharAnim(s16 charNo, HU3DMODELID modelId, s16 motNo, u8 voiceFlag, s16 frameNo, HuVecF *ofs);
static void EyeBmpUpdate(s16 charNo);

static void UpdateChar(void)
{
    s16 *property = HuPrcCurrentGet()->property;
    CHARWORK *workP = &charWork[*property];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    s16 updateBmpF = FALSE;
    HuVecF pos;
    s16 timingHookNo;
    pos.y = 100000.0f;
    workP->timingHookNo = timingHookNo = 0;
    
    while(1) {
        s16 motNo;
        s16 motNoShift;
        s16 motId;
        s16 motIdShift;
        s16 i;
        motNo = motNoShift = -1;
        for(i=0; i<CHAR_MOT_MAX; i++) {
            if(workP->motId[i] == modelP->motId) {
                motNo = workP->motNoTbl[i];
                motId = i;
                break;
            }
        }
        if(modelP->motIdShift != HU3D_MOTID_NONE) {
            for(i=0; i<CHAR_MOT_MAX; i++) {
                if(workP->motId[i] == modelP->motIdShift) {
                    motNoShift = workP->motNoTbl[i];
                    motIdShift = i;
                    break;
                }
                
            }
        }
        UpdateMotPlay(workP, motNo, motNoShift);
        motShiftF = FALSE;
        if(motNo != -1) {
            workP->motNoCurr = motNo;
            UpdateCharAnim(*property, workP->modelId, motNo, workP->voiceFlag[motId], modelP->motWork.time, &pos);
            workP->motNoPrev = motNo;
        } else {
            workP->motNoCurr = -1;
        }
        if(modelP->motIdShift != HU3D_MOTID_NONE) {
            motShiftF = TRUE;
            updateBmpF = TRUE;
            if(motNoShift != -1) {
                workP->motNoShiftCurr = motNoShift;
                UpdateCharAnim(*property, workP->modelId, motNoShift, workP->voiceFlag[motIdShift], modelP->motShiftWork.time, &pos);
                workP->motNoShiftPrev = motNoShift;
            } else {
                workP->motNoShiftCurr = -1;
            }
        } else {
            if(updateBmpF) {
                EyeBmpUpdate(*property);
                updateBmpF = FALSE;
            }
        }
        workP->pos = modelP->pos;
        if(timingHookNo != workP->timingHookNo) {
            timingHookNo = workP->timingHookNo;
        } else {
            workP->timingHookNo = 0;
        }
        HuPrcVSleep();
    }
}

static void UpdateMotPlay(CHARWORK *workP, s16 motNo, s16 motNoShift)
{
    s16 i;
    if(motNo != -1) {
        if(workP->motNoPlay[0] != motNo && workP->motNoPlay[1] != motNo) {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] != motNoShift) {
                    workP->motNoPlay[i] = motNo;
                    workP->motPlayTime[i] = 0;
                    break;
                }
            }
        } else {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] == motNo) {
                    workP->motPlayTime[i]++;
                    break;
                }
            }
        }
    }
    if(motNoShift != -1) {
        if(workP->motNoPlay[0] != motNoShift && workP->motNoPlay[1] != motNoShift) {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] != motNo) {
                    workP->motNoPlay[i] = motNoShift;
                    workP->motPlayTime[i] = 0;
                    break;
                }
            }
        } else {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] == motNoShift) {
                    workP->motPlayTime[i]++;
                    break;
                }
            }
        }
    }
    if(workP->motNoPlay[0] != motNo && workP->motNoPlay[0] != motNoShift) {
        workP->motNoPlay[0] = 32767;
    }
    if(workP->motNoPlay[1] != motNo && workP->motNoPlay[1] != motNoShift) {
        workP->motNoPlay[1] = 32767;
    }
}

static s32 GetMotNoPlayTime(CHARWORK *workP, s16 motNo)
{
    s16 i;
    for(i=0; i<2; i++) {
        if(workP->motNoPlay[i] == motNo) {
            return workP->motPlayTime[i];
        }
    }
    return -1;
}

static s32 PlayVoice(s16 charNo, s16 seNo, u8 voiceFlag);
static s16 PlayWinnerVoice(s16 charNo, u8 voiceFlag);
static s16 EffectDustCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, EFFECTPARAM *param);
static s16 EffectHitCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, EFFECTPARAM *param);
static s16 EffectSmokeCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, EFFECTPARAM *param);

static s16 EffectWarnCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, float ofsY, EFFECTPARAM *param);
static s16 EffectBirdCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, s16 charNo, float ofsY, EFFECTPARAM *param);

static void CreateHipDropEffect(s16 charNo, HuVecF *pos);

static s32 PlayStepVoice(s16 charNo, s16 seId, u8 voiceFlag);

static void UpdateCharAnim(s16 charNo, HU3DMODELID modelId, s16 motNo, u8 voiceFlag, s16 frameNo, HuVecF *ofs)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    CHARWORK *workP = &charWork[charNo];
    u32 attrOld = 0;
    s16 i;
    s16 dotMax;
    Mtx hitMtx;
    HuVecF pos;
    HuVecF hitPos;
    float warnSize;
    if(!motShiftF && (modelP->motAttr & HU3D_MOTATTR_PAUSE)) {
        return;
    }
    if(voiceFlag & 0x2) {
        return;
    }
    switch(motNo) {
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_315):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_318):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(7), voiceFlag);
            }
            break;
            
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_301):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_343):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_470):
            if(!motShiftF && modelP->motWork.speed <= 0.5) {
                break;
            }
            if(motShiftF && modelP->motShiftWork.speed <= 0.5) {
                break;
            }
            if((frameNo & 0xF) == 0 && !(workP->attr & 0x10)) {
                dustEffParam.vel.x = -HuSin(modelP->rot.y)*2.0;
                dustEffParam.vel.y = 1+(0.1*frandmod(10));
                dustEffParam.vel.z = -HuCos(modelP->rot.y)*2.0;
                pos.x = modelP->pos.x+(modelP->scale.x*(frandmod(50)-25));
                pos.y = modelP->pos.y;
                pos.z = modelP->pos.z+(modelP->scale.x*(frandmod(50)-25));
                if(charNo == CHARNO_MINIKOOPA || charNo == CHARNO_MINIKOOPAR || charNo == CHARNO_MINIKOOPAG || charNo == CHARNO_MINIKOOPAB) {
                    pos.x += -HuSin(modelP->rot.y)*40.0f;
                    pos.z += -HuCos(modelP->rot.y)*40.0f;
                }
                EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
            }
            for(i=0; i<4; i++) {
                if(frameNo == walkVoiceTimeTbl[(charNo*4)+i]) {
                    PlayStepVoice(charNo, CHARSEID(0), voiceFlag);
                    break;
                }
            }
            break;
            
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_302):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_355):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_378):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_471):
            if(!motShiftF && modelP->motWork.speed <= 0.5) {
                break;
            }
            if(motShiftF && modelP->motShiftWork.speed <= 0.5) {
                break;
            }
            if((frameNo & 0x3) == 0 && !(workP->attr & 0x10)) {
                dustEffParam.vel.x = -HuSin(modelP->rot.y)*4.0;
                dustEffParam.vel.y = 2+(0.1*frandmod(10));
                dustEffParam.vel.z = -HuCos(modelP->rot.y)*4.0;
                pos.x = modelP->pos.x+(modelP->scale.x*(frandmod(50)-25));
                pos.y = modelP->pos.y;
                pos.z = modelP->pos.z+(modelP->scale.x*(frandmod(50)-25));
                if(charNo == CHARNO_MINIKOOPA || charNo == CHARNO_MINIKOOPAR || charNo == CHARNO_MINIKOOPAG || charNo == CHARNO_MINIKOOPAB) {
                    pos.x += -HuSin(modelP->rot.y)*40.0f;
                    pos.z += -HuCos(modelP->rot.y)*40.0f;
                }
                EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
            }
            if(motNo == CHAR_MOTNO(CHARMOT_HSF_c000m1_355) && (frameNo & 0x1)) {
                CharModelCryCreate(charNo, 30, 0.8f);
            }
            for(i=0; i<4; i++) {
                if(frameNo == runVoiceTimeTbl[(charNo*4)+i]) {
                    PlayStepVoice(charNo, CHARSEID(4), voiceFlag);
                    break;
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_304):
            if(frameNo == 5 && !(workP->attr & 0x10)) {
                CharModelLandDustCreate(charNo, &modelP->pos);
            }
            for(i=0; i<2; i++) {
                if(frameNo == jumpVoiceTimeTbl[(charNo*2)+i]) {
                    PlayStepVoice(charNo, CHARSEID(8), voiceFlag);
                    break;
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_305):
            Hu3DModelObjMtxGet(modelId, CharModelItemHookGet(charNo, workP->model, 0), hitMtx);
            if(frameNo == 0) {
                PlayVoice(charNo, CHARSEID(22), voiceFlag);
            }
            dotMax = 10;
            if(frameNo <= dotMax && !(workP->attr & 0x10)) {
                hitEffParam.vel.x = 0.0f;
                hitEffParam.vel.y = 0.0f;
                hitEffParam.vel.z = 0.0f;
                hitEffParam.scaleVel = -5.0f;
                pos.x = hitMtx[0][3];
                pos.y = hitMtx[1][3];
                pos.z = hitMtx[2][3];
                if(frameNo != 0 && ofs->y != 100000.0f) {
                    VECSubtract(&pos, ofs, &hitPos);
                    dotMax = 0.2*HuMagVecF(&hitPos);
                    if(dotMax > 5) {
                        dotMax = 5;
                    }
                    if(dotMax < 1) {
                        dotMax = 1;
                    }
                    for(i=1; i<=dotMax; i++) {
                        hitPos.x = ofs->x+(pos.x-ofs->x)*((float)i/dotMax);
                        hitPos.y = ofs->y+(pos.y-ofs->y)*((float)i/dotMax);
                        hitPos.z = ofs->z+(pos.z-ofs->z)*((float)i/dotMax);
                        EffectHitCreate(modelId, hitPos.x, hitPos.y, hitPos.z, 50, &hitEffParam);
                    }
                } else {
                    EffectHitCreate(modelId, pos.x, pos.y, pos.z, 40, &hitEffParam);
                }
                *ofs = pos;
            } else {
                ofs->y = 100000.0f;
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_404):
            if(frameNo == 0) {
                PlayVoice(charNo, CHARSEID(23), voiceFlag);
            }
            if(frameNo < 10) {
                Hu3DModelObjMtxGet(modelId, CharModelItemHookGet(charNo, workP->model, 2), hitMtx);
                hitEffParam.vel.x = 0.0f;
                hitEffParam.vel.y = 0.0f;
                hitEffParam.vel.z = 0.0f;
                hitEffParam.scaleVel = -8.0f;

                pos.x = hitMtx[0][3];
                pos.y = hitMtx[1][3];
                pos.z = hitMtx[2][3];
                if(frameNo != 0) {
                    VECSubtract(&pos, ofs, &hitPos);
                    dotMax = 0.2*HuMagVecF(&hitPos);
                    if(dotMax > 5) {
                        dotMax = 5;
                    }
                    if(dotMax < 1) {
                        dotMax = 1;
                    }
                    for(i=1; i<=dotMax; i++) {
                        hitPos.x = ofs->x+(pos.x-ofs->x)*((float)i/dotMax);
                        hitPos.y = ofs->y+(pos.y-ofs->y)*((float)i/dotMax);
                        hitPos.z = ofs->z+(pos.z-ofs->z)*((float)i/dotMax);
                        EffectHitCreate(modelId, hitPos.x, hitPos.y, hitPos.z, 80, &hitEffParam);
                    }
                } else {
                    EffectHitCreate(modelId, pos.x, pos.y, pos.z, 80, &hitEffParam);
                }
                *ofs = pos;
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_310):
            if(frameNo == 0) {
                if(!(workP->attr & 0x10)) {
                    CreateHipDropEffect(charNo, &modelP->pos);
                }
                PlayVoice(charNo, CHARSEID(18), voiceFlag);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_326):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(6), voiceFlag);
            }
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_479):
            if(modelP->motIdShift == HU3D_MOTID_NONE || motShiftF) {
                if(!(workP->attr & 0x10)) {
                    if(frameNo == 10) {
                        if(charNo == CHARNO_WALUIGI) {
                            warnSize = 190;
                        } else if(charNo == CHARNO_PEACH || charNo == CHARNO_DAISY) {
                            warnSize = 140;
                        } else {
                            warnSize = 120;
                        }
                        EffectWarnCreate(modelId, modelP->pos.x, modelP->pos.y+100.0f, modelP->pos.z, 20, warnSize, &warnEffParam);
                    }
                    if(frameNo == 30) {
                        for(i=0; i<8; i++) {
                            dustEffParam.vel.x = 4.0*HuSin(45.0f * i + 22.5);
                            dustEffParam.vel.y = 0.0f;
                            dustEffParam.vel.z = 4.0*HuCos(45.0f * i + 22.5);
                            EffectDustCreate(modelId, modelP->pos.x, modelP->pos.y+(10.0f*modelP->scale.x), modelP->pos.z, 20.0f, &dustEffParam);
                        }
                    }
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_327):
            if(modelP->motIdShift == HU3D_MOTID_NONE || motShiftF) {
                if(!(workP->attr & 0x10)) {
                    if(frameNo == 0) {
                        if(charNo == CHARNO_WALUIGI) {
                            warnSize = 150;
                        } else if(charNo == CHARNO_PEACH || charNo == CHARNO_DAISY) {
                            warnSize = 120;
                        } else {
                            warnSize = 100;
                        }
                        EffectWarnCreate(modelId, modelP->pos.x, modelP->pos.y+100.0f, modelP->pos.z, 20, warnSize, &warnEffParam);
                    }
                    if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                        PlayVoice(charNo, CHARVOICEID(7), voiceFlag);
                    }
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_322):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_323):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_391):
            if(!(workP->attr & 0x1) && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARSEID(19), voiceFlag);
                for(i=0; i<3; i++) {
                    EffectBirdCreate(modelId, modelP->pos.x, modelP->pos.y+(100.0f*modelP->scale.x), modelP->pos.z, 1.0f, charNo, i*120, &warnEffParam);
                }
                workP->attr |= 0x1;
            }
            break;
            
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_321):
            if((frameNo & 0x1) && !(workP->attr & 0x10)) {
                Hu3DModelObjMtxGet(modelId, CharModelItemHookGet(charNo, workP->model, 4), hitMtx);
                pos.x = hitMtx[0][3];
                pos.y = hitMtx[1][3];
                pos.z = hitMtx[2][3];
                EffectSmokeCreate(modelId, pos.x, pos.y, pos.z, 20, &smokeEffParam);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_303):
            if(frameNo == 0) {
                PlayVoice(charNo, CHARSEID(16), voiceFlag);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_308):
            if(frameNo == 0) {
                PlayVoice(charNo, CHARSEID(17), voiceFlag);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_306):
            if(!(workP->attr & 0x12)) {
                if(omcurovl < DLL_w01dll || omcurovl > DLL_w20dll) {
                    if(frameNo == winAnimLen[charNo]) {
                        PlayWinnerVoice(charNo, voiceFlag);
                        workP->attr |= 0x2;
                        attrOld |= 0x2;
                    }
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_356):
            if(!(workP->attr & 0x12)) {
                if(frameNo == handUpAnimLen[charNo]) {
                    PlayWinnerVoice(charNo, voiceFlag);
                    workP->attr |= 0x2;
                    attrOld |= 0x2;
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_358):
            if(!(workP->attr & 0x12)) {
                if(frameNo == okAnimLen[charNo]) {
                    PlayWinnerVoice(charNo, voiceFlag);
                    workP->attr |= 0x2;
                    attrOld |= 0x2;
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_359):
            if(!(workP->attr & 0x12)) {
                if(frameNo == winAnimLen2[charNo]) {
                    PlayWinnerVoice(charNo, voiceFlag);
                    workP->attr |= 0x2;
                    attrOld |= 0x2;
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_380):
            if(!(workP->attr & 0x12)) {
                if(frameNo == holdUpAnimLen[charNo]) {
                    PlayWinnerVoice(charNo, voiceFlag);
                    workP->attr |= 0x2;
                    attrOld |= 0x2;
                }
            }
            break;
            
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_385):
            if(!(workP->attr & 0x12)) {
                if(frameNo == winJumpUpAnimLen[charNo]) {
                    PlayVoice(charNo, CHARVOICEID(2), voiceFlag);
                    workP->attr |= 0x2;
                    attrOld |= 0x2;
                }
            }
            break;
  
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_361):
            if(!(workP->attr & 0x12)) {
                if(frameNo == loseAnimLen[charNo]) {
                    PlayWinnerVoice(charNo, voiceFlag);
                    workP->attr |= 0x2;
                    attrOld |= 0x2;
                }
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_307):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_357):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_360):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_362):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_364):
            if(frameNo == 0 && !(workP->attr & 0x14)) {
                workP->attr |= 0x4;
                attrOld |= 0x4;
            }
            break;
            
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_354):
            if(frameNo & 0x1) {
                CharModelCryCreate(charNo, 0, 0);
            }
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(3), voiceFlag);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_345):
            if(workP->timingHookNo == 0) {
                break;
            }
            if((frameNo & 0x3) == 0) {
                CharModelCryCreate(charNo, 0, -0.3f);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_400):
            if(workP->timingHookNo == 0) {
                break;
            }
            CharModelCryCreate(charNo, 30, 0.8f);
            CharModelCryCreate(charNo, 30, 0.8f);
            CharModelCryCreate(charNo, 30, 0.8f);
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_406):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(3), voiceFlag);
            }
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_414):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_415):
            if(workP->timingHookNo != 0) {
                CharModelCryCreate(charNo, 30, 0.8f);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_311):
            if(frameNo == jumpAnimLen[charNo] && !(workP->attr & 0x10)) {
                CharModelLandDustCreate(charNo, &modelP->pos);
                for(i=0; i<2; i++) {
                    if(frameNo == jumpVoiceTimeTbl[(charNo*2)+i]) {
                        PlayStepVoice(charNo, CHARSEID(8), voiceFlag);
                        break;
                    }
                }
            }
            
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_346):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(9), voiceFlag);
            }
            break;
            
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_316):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_319):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_429):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_430):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_438):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_473):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_482):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(7), voiceFlag);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_344):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_368):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_437):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(12), voiceFlag);
            }
            break;
            
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_376):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(3), voiceFlag);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_392):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_442):
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_457):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(9), voiceFlag);
            }
            break;
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_423):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & 0x10)) {
                PlayVoice(charNo, CHARVOICEID(6), voiceFlag);
            }
            break;
    }
    if(!motShiftF) {
        if(!(attrOld & 0x4)) {
            workP->attr &= ~0x4;
        }
        if(!(attrOld & 0x2)) {
            workP->attr &= ~0x2;
        }
    }
}

static s32 PlayVoice(s16 charNo, s16 seNo, u8 voiceFlag)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    if(voiceFlag & 0x1) {
        return;
    }
    if(workP->attr & 0x8) {
        return HuAudCharFXPlayPos(charNo, seNo, &modelP->pos);
    } else {
        return HuAudCharFXPlayVolPan(charNo, seNo, workP->vol, workP->pan);
    }
}

static s16 PlayWinnerVoice(s16 charNo, u8 voiceFlag)
{
    s16 seNo;
    s16 i;
    s16 ret;
    for(i=0; specialWinVoiceTbl[i][0] != DLL_NONE; i++) {
        if(omcurovl == specialWinVoiceTbl[i][0]) {
            break;
        }
    }
    if(specialWinVoiceTbl[i][0] != DLL_NONE) {
        seNo = specialWinVoiceTbl[i][1];
    } else {
        seNo = CHARVOICEID(4);
    }
    PlayVoice(charNo, seNo, voiceFlag);
    return ret;
}

static void EffectParticleHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);

static void EffectInit(void)
{
    s16 effInitF = FALSE;
    s16 i;
    s16 cameraNo;
    ANIMDATA *anim[EFFECT_MAX];
    
    for(i=0; i<EFFECT_MAX; i++) {
        anim[i] = NULL;
    }
    for(cameraNo=0; cameraNo<HU3D_CAM_MAX; cameraNo++) {
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        for(i=0; i<EFFECT_MAX; i++) {
            if(effModelId[i][cameraNo] != HU3D_MODELID_NONE) {
                continue;
            }
            if(!anim[i]) {
                void *data = HuDataSelHeapReadNum(effectDataTbl[i].dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
                anim[i] = HuSprAnimRead(data);
            }
            effModelId[i][cameraNo] = Hu3DParticleCreate(anim[i], effectDataTbl[i].maxCnt);
            if(i == EFFECT_BIRD) {
                Hu3DParticleAnimModeSet(effModelId[i][cameraNo], 0);
            }
            Hu3DParticleHookSet(effModelId[i][cameraNo], EffectParticleHook);
            Hu3DModelCameraSet(effModelId[i][cameraNo], (1 << cameraNo));
            if(!effParamAll[i]) {
                effParamAll[i] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, effectDataTbl[i].maxCnt*sizeof(EFFECTPARAM));
            }
            Hu3DParticleBlendModeSet(effModelId[i][cameraNo], effectDataTbl[i].blendMode);
            {
                HU3DPARTICLE *particleP = Hu3DData[effModelId[i][cameraNo]].hookData;
                HU3DPARTICLEDATA *particleDataP;
                s16 j;
                particleP->emitCnt = 0;
                particleP->work = effParamAll[i];
                particleP->count = 1;
                for(particleDataP = particleP->data, j=0; j<particleP->maxCnt; j++, particleDataP++) {
                    particleDataP->scale = 0;
                }
                effInitF = TRUE;
            }
        }
    }
    if(effInitF) {
        HuDataDirClose(DATA_effect);
    }
}

static void EffectParticleCreate(u8 type)
{
    ANIMDATA *anim = NULL;
    s16 cameraNo;
    HU3DPARTICLE *particleP;
    
    for(cameraNo=0; cameraNo<HU3D_CAM_MAX; cameraNo++) {
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        if(effModelId[type][cameraNo] != HU3D_MODELID_NONE) {
            particleP = Hu3DData[effModelId[type][cameraNo]].hookData;
            if(particleP->anim) {
                anim = particleP->anim;
                continue;
            }
        } else {
            if(!anim) {
                void *data = HuDataSelHeapReadNum(effectDataTbl[type].dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
                anim = HuSprAnimRead(data);
            }
            effModelId[type][cameraNo] = Hu3DParticleCreate(anim, effectDataTbl[type].maxCnt);
            Hu3DModelLayerSet(effModelId[type][cameraNo], effectLayer);
            if(type == EFFECT_BIRD) {
                Hu3DParticleAnimModeSet(effModelId[type][cameraNo], 0);
            }
            Hu3DParticleHookSet(effModelId[type][cameraNo], EffectParticleHook);
            Hu3DModelCameraSet(effModelId[type][cameraNo], (1 << cameraNo));
            if(!effParamAll[type]) {
                effParamAll[type] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, effectDataTbl[type].maxCnt*sizeof(EFFECTPARAM));
            }
            Hu3DParticleBlendModeSet(effModelId[type][cameraNo], effectDataTbl[type].blendMode);
            {
                HU3DPARTICLEDATA *particleDataP;
                s16 j;
                particleP = Hu3DData[effModelId[type][cameraNo]].hookData;
                particleP->emitCnt = 0;
                particleP->work = effParamAll[type];
                particleP->count = 1;
                for(particleDataP = particleP->data, j=0; j<particleP->maxCnt; j++, particleDataP++) {
                    particleDataP->scale = 0;
                }
            }
        }
    }
}

static s16 EffectCreate(s16 type, s16 cameraBit, float posX, float posY, float posZ, float scale, EFFECTPARAM *param);

static s16 EffectDustCreate(s16 modelId, float posX, float posY, float posZ, float scale, EFFECTPARAM *param)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_DUST, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_DUST, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

static s16 EffectSmokeCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, EFFECTPARAM *param)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_SMOKE, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_SMOKE, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

static s16 EffectHitCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, EFFECTPARAM *param)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_HIT, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_HIT, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

static s16 EffectStarCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, EFFECTPARAM *param)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_STAR, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_STAR, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

static s16 EffectWarnCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, float ofsY, EFFECTPARAM *param)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    EFFECTPARAM paramNew = *param;
    s16 cameraNo, cameraBit;
    paramNew.vel.x *= modelP->scale.x;
    paramNew.vel.y *= modelP->scale.x;
    paramNew.vel.z *= modelP->scale.x;
    paramNew.scaleVel *= modelP->scale.x;
    scale *= modelP->scale.x;
    for(cameraNo=0, cameraBit=HU3D_CAM0; cameraNo<HU3D_CAM_MAX; cameraNo++, cameraBit <<= 1) {
        HU3DMODEL *effModelP;
        HU3DPARTICLE *particleP;
        HU3DPARTICLEDATA *particleDataP;
        s16 particleId;
        if(!(cameraBit & modelP->cameraBit)) {
            continue;
        }
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        particleId = EffectCreate(EFFECT_WARN, cameraBit, posX, posY, posZ, scale, &paramNew);
        if(particleId == -1) {
            return;
        } else {
            effModelP = &Hu3DData[effModelId[EFFECT_WARN][cameraNo]];
            particleP = effModelP->hookData;
            particleDataP = &particleP->data[particleId];
            particleDataP->parManId = 0;
            particleDataP->vel.x = modelId;
            particleDataP->vel.y = ofsY;
            particleDataP->accel.x = particleDataP->accel.z = 0;
            particleDataP->accel.y = 100;
        }
    }
}

static s16 EffectBirdCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float scale, s16 charNo, float ofsY, EFFECTPARAM *param)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    EFFECTPARAM paramNew = *param;
    s16 cameraNo, cameraBit;
    paramNew.vel.x *= modelP->scale.x;
    paramNew.vel.y *= modelP->scale.x;
    paramNew.vel.z *= modelP->scale.x;
    paramNew.scaleVel *= modelP->scale.x;
    scale *= modelP->scale.x;
    for(cameraNo=0, cameraBit=HU3D_CAM0; cameraNo<HU3D_CAM_MAX; cameraNo++, cameraBit <<= 1) {
        HU3DMODEL *effModelP;
        HU3DPARTICLE *particleP;
        HU3DPARTICLEDATA *particleDataP;
        s16 particleId;
        if(!(cameraBit & modelP->cameraBit)) {
            continue;
        }
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        particleId = EffectCreate(EFFECT_BIRD, cameraBit, posX, posY, posZ, scale, &paramNew);
        if(particleId == -1) {
            return;
        } else {
            effModelP = &Hu3DData[effModelId[EFFECT_BIRD][cameraNo]];
            particleP = effModelP->hookData;
            particleDataP = &particleP->data[particleId];
            particleDataP->parManId = 1;
            particleDataP->time = 0;
            particleDataP->vel.x = charNo;
            particleDataP->vel.y = ofsY;
        }
    }
}

static s16 EffectCreate(s16 type, s16 cameraBit, float posX, float posY, float posZ, float scale, EFFECTPARAM *param)
{
    s16 cameraNo, bit;
    s16 id;
    EffectParticleCreate(type);
    id = -1;
    for(cameraNo=0, bit=HU3D_CAM0; cameraNo<HU3D_CAM_MAX; cameraNo++, bit <<= 1) {
        HU3DPARTICLEDATA *particleDataP;
        HU3DPARTICLE *particleP;
        s16 nextId;
        HU3DMODEL *modelP;
        EFFECTPARAM *effParam;
        s16 modelId;
        if(!(bit & cameraBit)) {
            continue;
        }
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        modelId = effModelId[type][cameraNo];
        if(modelId == HU3D_MODELID_NONE) {
            return -1;
        }
        modelP = &Hu3DData[modelId];
        particleP = modelP->hookData;
        effParam = particleP->work;
        for(particleDataP=&particleP->data[particleP->emitCnt], nextId=particleP->emitCnt; nextId<particleP->maxCnt; nextId++, particleDataP++) {
            if(!particleDataP->scale) {
                break;
            }
        }
        if(nextId >= particleP->maxCnt) {
            for(particleDataP=&particleP->data[0], nextId=0; nextId<particleP->maxCnt; nextId++, particleDataP++) {
                if(!particleDataP->scale) {
                    break;
                }
            }
        }
        if(nextId != particleP->maxCnt) {
            effParam[nextId] = *param;
            particleDataP->cameraBit = cameraBit;
            particleDataP->pos.x = posX;
            particleDataP->pos.y = posY;
            particleDataP->pos.z = posZ;
            particleDataP->vel = param->vel;
            particleDataP->color = param->colorBegin;
            particleDataP->scaleBase = scale;
            particleDataP->scale = scale;
            particleDataP->time = 0;
            particleDataP->parManId = HU3D_PARMANID_NONE;
            particleP->emitCnt = nextId;
            id = nextId;
        } else {
            id = -1;
        }
    }
    return id;
}

static void UpdateEffect(HU3DPARTICLEDATA *particleDataP);
static void UpdateBirdEffect(HU3DPARTICLEDATA *particleDataP);
static void UpdateModelEffect(HU3DPARTICLEDATA *particleDataP);

static void EffectParticleHook(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    EFFECTPARAM *param = particleP->work;
    HU3DPARTICLEDATA *particleDataP;
    s16 color;
    s16 i;
    if(particleP->count == 0) {
        for(particleDataP=&particleP->data[0], i=0; i<particleP->maxCnt; i++, particleDataP++) {
            particleDataP->scale = 0;
        }
    }
    for(particleDataP=&particleP->data[0], i=0; i<particleP->maxCnt; i++, particleDataP++) {
        if(!particleDataP->scale) {
            continue;
        }
        if(particleDataP->parManId == HU3D_PARMANID_NONE) {
            particleDataP->vel.x *= param[i].velDecay.x;
            particleDataP->vel.y *= param[i].velDecay.y;
            particleDataP->vel.z *= param[i].velDecay.z;
            VECAdd(&particleDataP->vel, &particleDataP->pos, &particleDataP->pos);
            particleDataP->vel.y += param[i].gravity;
            color = particleDataP->color.r+(param[i].colorWeight*(param[i].colorEnd.r-param[i].colorBegin.r));
            if(color < 0) {
                color = 0;
            } else if(color > 255) {
                color = 255;
            }
            particleDataP->color.r = color;
            color = particleDataP->color.g+(param[i].colorWeight*(param[i].colorEnd.g-param[i].colorBegin.g));
            if(color < 0) {
                color = 0;
            } else if(color > 255) {
                color = 255;
            }
            particleDataP->color.g = color;
            color = particleDataP->color.b+(param[i].colorWeight*(param[i].colorEnd.b-param[i].colorBegin.b));
            if(color < 0) {
                color = 0;
            } else if(color > 255) {
                color = 255;
            }
            particleDataP->color.b = color;
            color = particleDataP->color.a+param[i].alphaBase;
            if(color < 1) {
                particleDataP->scale = 0;
            }
            particleDataP->color.a = color;
            if(particleDataP->scale) {
                if(param[i].attr & 0x1) {
                    particleDataP->scale = particleDataP->scaleBase*(((particleDataP->time+i) & 0x1) ? 1.0 : 0.5);
                } else {
                    particleDataP->scale = particleDataP->scaleBase;
                }
                particleDataP->scaleBase += param[i].scaleVel;
                if(particleDataP->scaleBase <= 0.01f) {
                    particleDataP->scale = 0;
                }
            }
            particleDataP->time++;
        } else {
            switch(particleDataP->parManId) {
                case 0:
                    UpdateEffect(particleDataP);
                    break;
                
                case 1:
                    UpdateBirdEffect(particleDataP);
                    break;
                    
                case 2:
                    UpdateModelEffect(particleDataP);
                    break;
                
            }
        }
    }
    DCStoreRangeNoSync(particleP->data, particleP->maxCnt*sizeof(HU3DPARTICLEDATA));
}

static void UpdateEffect(HU3DPARTICLEDATA *particleDataP)
{
    HU3DMODEL *modelP = &Hu3DData[(int)particleDataP->vel.x];
    float radius;
    if(particleDataP->time < 8) {
        radius = 0.3+HuSin(40.0f+(10.0f*(particleDataP->time+1)));
        particleDataP->scale = (50.0f*radius)*modelP->scale.x;
        particleDataP->color.a = 255;
        radius = 0.3+HuSin(15.0f*(particleDataP->time+1));
    } else {
        radius = 0.3+HuSin(135.0f);
    }
    radius *= modelP->scale.x;
    particleDataP->pos.x = modelP->pos.x+(particleDataP->accel.x*radius);
    particleDataP->pos.y = (modelP->pos.y+(particleDataP->vel.y*modelP->scale.x))+(particleDataP->accel.y*radius);
    particleDataP->pos.z = modelP->pos.z+(particleDataP->accel.z*radius);
    if(particleDataP->time > 20) {
        particleDataP->color.a -= 32;
        particleDataP->scale -= 8.0f*modelP->scale.x;
        if(particleDataP->scale < 0.0f) {
            particleDataP->scale = 0.0f;
        }
    }
    particleDataP->time++;
}

static float birdYOfs[CHARNO_MAX*2] = {
    110, 160,
    110, 160,
    150, 180,
    130, 160,
    130, 160,
    150, 180,
    120, 210,
    110, 160,
    110, 160,
    130, 160,
    130, 160,
    130, 160,
    130, 160
};

static void UpdateBirdEffect(HU3DPARTICLEDATA *particleDataP)
{
    s16 charNo = (s16)particleDataP->vel.x;
    s16 angle;
    s16 birdY;
    CHARWORK *workP;
    HU3DMODEL *modelP;
    s16 effectNo;
    BOOL birdMotPlayF;
    
    birdMotPlayF = TRUE;
    workP = &charWork[charNo];
    modelP = &Hu3DData[workP->modelId];
    
    for(effectNo=0; effectDataTbl[EFFECT_BIRD].motDataNum[effectNo]; effectNo++) {
        if(workP->motNoCurr == CHAR_MOTNO(effectDataTbl[EFFECT_BIRD].motDataNum[effectNo])) {
            break;
        }
    }
    if(effectDataTbl[EFFECT_BIRD].motDataNum[effectNo] == 0) {
        birdMotPlayF = FALSE;
    }
    if(particleDataP->time < 20 && particleDataP->scale < 40.0f*modelP->scale.x) {
        particleDataP->scale += 4.0f*modelP->scale.x;
    }
    particleDataP->color.a = 255;
    if(workP->motNoCurr == CHAR_MOTNO(CHARMOT_HSF_c000m1_323) || workP->motNoShiftCurr == CHAR_MOTNO(CHARMOT_HSF_c000m1_323)) {
        birdY = birdYOfs[(charNo*2)+0];
    } else {
        birdY = birdYOfs[(charNo*2)+1];
    }
    angle = (particleDataP->time*5)%360;
    particleDataP->pos.x = modelP->pos.x+(modelP->scale.x*(40.0*HuSin(particleDataP->vel.y+angle)));
    particleDataP->pos.y = modelP->pos.y+(birdY*modelP->scale.x);
    particleDataP->pos.z = modelP->pos.z+(modelP->scale.x*(40.0*HuCos(particleDataP->vel.y+angle)));
    particleDataP->time++;
    if(particleDataP->time >= 143) {
        particleDataP->time = 72;
    }
    if(!birdMotPlayF && particleDataP->time > 30) {
        particleDataP->scale -= 8.0f*modelP->scale.x;
        if(particleDataP->scale < 0.0f) {
            particleDataP->scale = 0.0f;
            workP->attr &= ~0x1;
            if(particleDataP->vel.y == 0.0) {
                PlayVoice(charNo, CHARSEID(24), workP->attr);

            }
        }
    }
}

static s16 stepFxTbl[] = {
    DLL_m501dll, 0,
    DLL_m502dll, 0,
    DLL_m503dll, 2,
    DLL_m504dll, 0,
    DLL_m505dll, 0,
    DLL_m506dll, 2,
    DLL_m507dll, 0,
    DLL_m508dll, 0,
    DLL_m509dll, 0,
    DLL_m510dll, 0,
    DLL_m511dll, 0,
    DLL_m512dll, 0,
    DLL_m513dll, 2,
    DLL_m514dll, 0,
    DLL_m515dll, 2,
    DLL_m516dll, 0,
    DLL_m517dll, 0,
    DLL_m518dll, 0,
    DLL_m519dll, 0,
    DLL_m520dll, 0,
    DLL_m521dll, 0,
    DLL_m522dll, 0,
    DLL_m523dll, 2,
    DLL_m524dll, 0,
    DLL_m525dll, 1,
    DLL_m526dll, 0,
    DLL_m527dll, 0,
    DLL_m528dll, 0,
    DLL_m529dll, 1,
    DLL_m530dll, 0,
    DLL_m531dll, 1,
    DLL_m532dll, 1,
    DLL_m533dll, 1,
    DLL_m534dll, 2,
    DLL_m535dll, 0,
    DLL_m536dll, 1,
    DLL_m537dll, 1,
    DLL_m538dll, 2,
    DLL_m539dll, 0,
    DLL_m540dll, 1,
    DLL_m541dll, 0,
    DLL_m542dll, 0,
    DLL_m543dll, 0,
    DLL_m544dll, 1,
    DLL_m545dll, 0,
    DLL_m546dll, 0,
    DLL_m547dll, 0,
    DLL_m548dll, 0,
    DLL_m549dll, 1,
    DLL_m550dll, 0,
    DLL_m551dll, 1,
    DLL_m552dll, 0,
    DLL_m553dll, 1,
    DLL_m554dll, 1,
    DLL_m555dll, 0,
    DLL_m559dll, 0,
    DLL_m560dll, 0,
    DLL_m561dll, 0,
    DLL_m562dll, 0,
    DLL_m563dll, 0,
    DLL_m564dll, 0,
    DLL_m565dll, 0,
    DLL_m566dll, 0,
    DLL_m567dll, 0,
    DLL_m568dll, 1,
    DLL_m569dll, 0,
    DLL_m570dll, 1,
    DLL_m571dll, 1,
    DLL_m572dll, 1,
    DLL_m573dll, 0,
    DLL_m574dll, 0,
    DLL_m575dll, 0,
    DLL_m576dll, 0,
    DLL_m577dll, 1,
    DLL_m579dll, 1,
    DLL_m580dll, 1,
    DLL_NONE, -1
};

static s16 GetStepFxNo(void)
{
    s16 i;
    for(i=0; stepFxTbl[i] != DLL_NONE; i++) {
        if(omcurovl == stepFxTbl[(i*2)+0]) {
            return stepFxTbl[(i*2)+1];
        }
    }
    return 0;
}

static void MotionParticleInit(unsigned int dataNum)
{
    s16 i;
    s16 j;
    dataNum &= 0xFFFF;
    for(i=0; i<EFFECT_MAX; i++) {
        for(j=0; effectDataTbl[i].motDataNum[j]; j++) {
            if(dataNum == CHAR_MOTNO(effectDataTbl[i].motDataNum[j])) {
                EffectParticleCreate(i);
                break;
            }
        }
    }
}

HU3DMOTID CharMotionCreate(s16 charNo, unsigned int dataNum)
{
    CHARWORK *workP = &charWork[charNo];
    s16 i;
    s16 motNo;
    void *data;
    unsigned int dir;
    if(workP->modelId == HU3D_MODELID_NONE) {
        return HU3D_MOTID_NONE;
    }
    for(motNo=0; motNo<CHAR_MOT_MAX; motNo++) {
        if(workP->motId[motNo] == HU3D_MOTID_NONE) {
            break;
        }
    }
    if(motNo == CHAR_MOT_MAX) {
        return HU3D_MOTID_NONE;
    }
    dir = dataNum & 0xFFFF0000;
    for(i=0; i<CHARNO_MAX; i++) {
        if(dir == charDataDirTbl[i][4]) {
            break;
        }
    }
    if(i != CHARNO_MAX || dir == 0) {
        dataNum = FILENUM(dataNum);
        data = HuAR_ARAMtoMRAMFileRead(DATANUM(charDataDirTbl[charNo][4], dataNum), HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
        if(!data) {
            data = HuDataSelHeapReadNum(DATANUM(charDataDirTbl[charNo][4], dataNum), HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
        }
        workP->motNoTbl[motNo] = dataNum;
    } else {
        for(i=0; i<CHARNO_MAX; i++) {
            if(dir == charDataDirTbl[i][5]) {
                break;
            }
        }
        if(i != CHARNO_MAX) {
            dataNum = DATANUM(charDataDirTbl[charNo][5], dataNum & 0xFFFF);
            data = HuDataReadNumHeapShortForce(dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
        } else {
            data = HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
        }
        workP->motNoTbl[motNo] = HU3D_MOTID_NONE;
    }
    workP->motId[motNo] = Hu3DJointMotion(workP->modelId, data);
    workP->voiceFlag[motNo] = 0;
    MotionParticleInit(dataNum);
    return workP->motId[motNo];
}

void CharMotionNoSet(s16 charNo, HU3DMOTID motId, unsigned int motNo)
{
    CHARWORK *workP = &charWork[charNo];
    s16 i;
    for(i=0; i<CHAR_MOT_MAX; i++) {
        if(workP->motId[i] == motId) {
            break;
        }
    }
    if(i != CHAR_MOT_MAX) {
        workP->motNoTbl[i] = motNo;
    }
}

void CharMotionKill(s16 charNo, unsigned int motId)
{
    CHARWORK *workP = &charWork[charNo];
    s16 i;
    for(i=0; i<CHAR_MOT_MAX; i++) {
        if(workP->motId[i] == motId) {
            break;
        }
    }
    workP->motId[i] = HU3D_MOTID_NONE;
    Hu3DMotionKill(motId);
    
}

void CharMotionDataClose(s16 charNo)
{
    s16 i;
    if(charNo == CHARNO_NONE) {
        for(i=0; i<CHARNO_MAX; i++) {
            CharMotionDataClose(i);
        }
    } else {
        HuDataDirClose(charDataDirTbl[charNo][4]);
        HuDataDirClose(DATA_effect);
    }
}

void CharModelDataClose(s16 charNo)
{
    s16 i;
    if(charNo == CHARNO_NONE) {
        for(i=0; i<CHARNO_MAX; i++) {
            CharModelDataClose(i);
        }
    } else {
        for(i=0; i<5; i++) {
            HuDataDirClose(charDataDirTbl[charNo][i]);
        }
        HuDataDirClose(DATA_effect);
    }
}

void CharModelKill(s16 charNo)
{
    s16 i;
    s16 cameraNo;
    if(charNo == CHARNO_NONE) {
        for(i=0; i<CHARNO_MAX; i++) {
            CharModelKill(i);
        }
        for(i=0; i<CHAR_NPC_MAX; i++) {
            dustFlags[i] = 0;
        }
    } else {
        CHARWORK *workP;
        CharMotionDataClose(charNo);
        workP = &charWork[charNo];
        for(i=0; i<CHAR_MOT_MAX; i++) {
            if(workP->motId[i] != HU3D_MOTID_NONE) {
                Hu3DMotionKill(workP->motId[i]);
            }
            workP->motId[i] = HU3D_MOTID_NONE;
        }
        if(workP->modelId != HU3D_MODELID_NONE) {
            Hu3DModelKill(workP->modelId);
        }
        workP->modelId = HU3D_MODELID_NONE;
        for(i=0; i<CHARNO_MAX; i++) {
            if(charWork[i].modelId != HU3D_MODELID_NONE) {
                break;
            }                
        }
        if(i == CHARNO_MAX) {
            for(i=0; i<EFFECT_MAX; i++) {
                for(cameraNo=0; cameraNo<HU3D_CAM_MAX; cameraNo++) {
                    if(effModelId[i][cameraNo] != HU3D_MODELID_NONE) {
                        Hu3DModelKill(effModelId[i][cameraNo]);
                    }
                    effModelId[i][cameraNo] = HU3D_MODELID_NONE;
                }
            }
            effectLayer = 0;
            for(i=0; i<EFFECT_MAX; i++) {
                if(effParamAll[i]) {
                    HuMemDirectFree(effParamAll[i]);
                }
                effParamAll[i] = NULL;
            }
        }
        if(workP->process) {
            HuMemDirectFree(workP->process->property);
            HuPrcKill(workP->process);
            for(i=0; i<CHAR_MOT_MAX; i++) {
                if(hookDustProc[i]) {
                    HuPrcKill(hookDustProc[i]);
                }
                hookDustProc[i] = NULL;
            }
            workP->process = NULL;
        }
        workP->vol = MSM_VOL_MAX;
        workP->pan = MSM_PAN_CENTER;
    }
}

void CharMotionSet(s16 charNo, HU3DMOTID motId)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMOTION *motP = &Hu3DMotion[motId];
    EyeBmpUpdate(charNo);
    Hu3DMotionSet(workP->modelId, motId);
}

static void EyeBmpUpdate(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    HSFATTRIBUTE *attrP = modelP->hsf->attribute;
    s16 i;
    s16 modelBit;
    char **eyeBmp;
    for(i=0, modelBit=1; i<CHAR_MODEL_MAX; i++, modelBit <<= 1) {
        if(modelBit & workP->model) {
            break;
        }
    }
    if(i >= CHAR_MODEL_MAX) {
        return;
    }
    eyeBmp = CharModelEyeBmpGet(charNo, workP->model);
    for(i=0; i<modelP->hsf->attributeNum; i++, attrP++) {
        if(attrP->bitmap->name[0] != eyeBmp[0][0] || strcmp(attrP->bitmap->name, eyeBmp[0])) {
            if(attrP->bitmap->name[0] != eyeBmp[1][0] || strcmp(attrP->bitmap->name, eyeBmp[1])) {
                continue;
            }
        }
        if(attrP->animWorkP) {
            HU3DATTRANIM *attrAnimP = attrP->animWorkP;
            attrAnimP->trans3D.x = attrAnimP->trans3D.y = attrAnimP->trans3D.z = 0;
            attrAnimP->rot.x = attrAnimP->rot.y = attrAnimP->rot.z = 0;
        }
    }
}

char *charEyeBmpNameTbl[CHARNO_MAX*CHAR_MODEL_MAX*2] = {
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m2_eyes",
    "s3c000m2_eyes",
    "s3c000m3_eyes",
    "s3c000m3_eyes",
    
    "S3c001m0_eye",
    "S3c001m0_eye",
    "S3c001m1_eye",
    "S3c001m1_eye",
    "c001m3_eye",
    "c001m3_eye",
    "c001m3_eye",
    "c001m3_eye",
    
    "s3c002m0_r_eye",
    "s3c002m0_l_eye",
    "s3c002m1_r_eye",
    "s3c002m1_l_eye",
    "s3c002m2_r_eye",
    "s3c002m2_l_eye",
    "",
    "",
    
    "eye1",
    "eye2",
    "S3c003m1",
    "S3c003m1",
    "eye1",
    "eye2",
    "eye1",
    "eye2",
    
    "GC-eyes",
    "GC-eyes",
    "s3c004m1_eye",
    "s3c004m1_eye",
    "Clswario_eye_l1_AUTO12",
    "Clswario_eye_l1_AUTO13",
    "",
    "",
    
    "GC-eyes",
    "GC-eyes",
    "s3c007m1_Eye_L",
    "s3c007m1_Eye_R",
    "mat87",
    "mat89",
    "",
    "",
    
    "clswaluigi_eye_l1_AUTO1",
    "clswaluigi_eye_l1_AUTO2",
    "s3c007_m1_eye",
    "s3c007_m1_eye",
    "clswaluigi_eye_l1_AUTO9",
    "clswaluigi_eye_l1_AUTO10",
    "",
    "",
    
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m2_eyes",
    "s3c000m2_eyes",
    "s3c000m3_eyes",
    "s3c000m3_eyes",
    
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m2_eyes",
    "s3c000m2_eyes",
    "s3c000m3_eyes",
    "s3c000m3_eyes",
    
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m2_eyes",
    "s3c000m2_eyes",
    "s3c000m3_eyes",
    "s3c000m3_eyes",
    
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m2_eyes",
    "s3c000m2_eyes",
    "s3c000m3_eyes",
    "s3c000m3_eyes",
    
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m2_eyes",
    "s3c000m2_eyes",
    "s3c000m3_eyes",
    "s3c000m3_eyes",
    
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m1_eyes",
    "s3c000m2_eyes",
    "s3c000m2_eyes",
    "s3c000m3_eyes",
    "s3c000m3_eyes",
};

char **CharModelEyeBmpGet(s16 charNo, s16 model)
{
    s16 i;
    s16 bit;
    for(i=0, bit=CHAR_MODEL0; i<CHAR_MODEL_MAX-1; i++, bit <<= 1) {
        if(bit & model) {
            break;
        }
    }
    return &charEyeBmpNameTbl[(charNo*(CHAR_MODEL_MAX*2))+(i*2)];
}

static char *hookNameTbl[CHARNO_MAX*5] = {
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
    
    "a-itemhook-r",
    "a-itemhook-l",
    "a-itemhook-fr",
    "a-itemhook-fl",
    "a-itemhook-body",
};

char *CharModelItemHookGet(s16 charNo, s16 model, s16 hookNo)
{
    s16 i;
    s16 bit;
    for(i=0, bit=CHAR_MODEL0; i<CHAR_MODEL_MAX-1; i++, bit <<= 1) {
        if(bit & model) {
            break;
        }
    }
    return hookNameTbl[(charNo*5)+hookNo];
}

void CharMotionTimeSet(s16 charNo, float time)
{
    CHARWORK *workP = &charWork[charNo];
    Hu3DMotionTimeSet(workP->modelId, time);
}

float CharMotionTimeGet(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    return Hu3DMotionTimeGet(workP->modelId);
}

float CharMotionMaxTimeGet(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    return Hu3DMotionMaxTimeGet(workP->modelId);
}

BOOL CharMotionEndCheck(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    return Hu3DMotionEndCheck(workP->modelId);
}

HU3DMOTID CharMotionShiftIDGet(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    return Hu3DMotionShiftIDGet(workP->modelId);
}

void CharMotionShiftSet(s16 charNo, HU3DMOTID motId, float start, float end, u32 attr)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMOTION *motP = &Hu3DMotion[motId];
    Hu3DMotionShiftSet(workP->modelId, motId, start, end, attr);
}

float CharMotionShiftTimeGet(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    return Hu3DMotionShiftTimeGet(workP->modelId);
}

void CharMotionSpeedSet(s16 charNo, float speed)
{
    CHARWORK *workP = &charWork[charNo];
    Hu3DMotionSpeedSet(workP->modelId, speed);
}

void CharMotionStartEndSet(s16 charNo, float start, float end)
{
    CHARWORK *workP = &charWork[charNo];
    Hu3DMotionStartEndSet(workP->modelId, start, end);
}

void CharModelAttrSet(s16 charNo, u32 attr)
{
    CHARWORK *workP = &charWork[charNo];
    Hu3DModelAttrSet(workP->modelId, attr);
}

void CharModelAttrReset(s16 charNo, u32 attr)
{
    CHARWORK *workP = &charWork[charNo];
    Hu3DModelAttrReset(workP->modelId, attr);
}

float charHeightTbl[CHARNO_MAX] = {
    150,
    170,
    188,
    169,
    164,
    180,
    210,
    120,
    150,
    150,
    150,
    150,
    150
};

float CharModelHeightGet(s16 charNo)
{
    return charHeightTbl[charNo];
}

HU3DMODELID CharModelMotListCreate(s16 charNo, s16 model, unsigned int *motDataNum, HU3DMOTID *motId)
{
    HU3DMODELID modelId = CharModelCreate(charNo, model);
    s16 no = 0;
    do {
        if(motDataNum[no] == 0) {
            break;
        }
        motId[no] = CharMotionCreate(charNo, motDataNum[no]);
        no++;
    } while(1);
    CharMotionDataClose(charNo);
    return modelId;
}

s32 CharModelTimingHookNoGet(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    if(workP->timingHookNo == 0) {
        return 0;
    } else if(workP->timingHookNo == 1) {
        return 1;
    } else {
        return 2;
    }
}

typedef struct HookDustWork_s {
    u16 cameraBit;
    HU3DMODELID modelId;
} HOOKDUSTWORK;

static void CreateHookDust(void);

static HUPROCESS *CreateHookDustProc(void)
{
    s16 i;
    for(i=0; i<CHAR_MOT_MAX; i++) {
        if(!hookDustProc[i]) {
            break;
        }
    }
    if(i == CHAR_MOT_MAX) {
        return NULL;
    }
    hookDustProc[i] = HuPrcCreate(CreateHookDust, 100, 8192, 0);
    return hookDustProc[i];
}

static void KillHookDustProc(HUPROCESS *process)
{
    s16 i;
    for(i=0; i<CHAR_MOT_MAX; i++) {
        if(hookDustProc[i] == process) {
            HuPrcKill(process);
            hookDustProc[i] = NULL;
        }
    }
}

void CharModelHookDustCreate(s16 charNo, char *objName)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    HSFOBJECT *objPtr = Hu3DModelObjPtrGet(workP->modelId, objName);
    HSFCONSTDATA *constData;
    HOOKDUSTWORK *hookDustWork;
    HU3DMODELID hookMdlId;
    HUPROCESS *process;
    Mtx hookMtx;
    HuVecF temp;
    
    Hu3DModelObjMtxGet(workP->modelId, objName, hookMtx);
    constData = objPtr->constData;
    hookMdlId = constData->hookMdlId;
    if(hookMdlId == HU3D_MODELID_NONE) {
        return;
    }
    Hu3DModelHookObjReset(workP->modelId, objName);
    process = CreateHookDustProc();
    if(!process) {
        Hu3DModelDispOff(hookMdlId);
        return;
    }
    process->property = hookDustWork = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, sizeof(HOOKDUSTWORK), HU_MEMNUM_OVL);
    modelP = &Hu3DData[hookMdlId];
    Hu3DMtxTransGet(hookMtx, &temp);
    Hu3DModelPosSetV(hookMdlId, &temp);
    Hu3DMtxRotGet(hookMtx, &temp);
    mtxRot(modelP->mtx, temp.x, temp.y, temp.z);
    Hu3DMtxScaleGet(hookMtx, &temp);
    Hu3DModelScaleSetV(hookMdlId, &temp);
    hookDustWork->modelId = hookMdlId;
    hookDustWork->cameraBit = modelP->cameraBit;
}

static void CreateHookDust(void)
{
    HOOKDUSTWORK *hookDustWork = HuPrcCurrentGet()->property;
    HU3DMODEL *modelP = &Hu3DData[hookDustWork->modelId];
    Mtx rootMtx;
    HuVecF pos;
    s16 i;
    Hu3DModelObjMtxGet(hookDustWork->modelId, "", rootMtx);
    pos.x = PGMinPos.x+((PGMaxPos.x-PGMinPos.x)/2);
    pos.y = PGMinPos.y+((PGMaxPos.y-PGMinPos.y)/2);
    pos.z = PGMinPos.z+((PGMaxPos.z-PGMinPos.z)/2);
    for(i=0; i<40; i++) {
        modelP->pos.y += 4.0f;
        mtxRotCat(modelP->mtx, 24, 0, 0);
        modelP->scale.x *= 0.95f;
        modelP->scale.y *= 0.95f;
        modelP->scale.z *= 0.95f;
        HuPrcVSleep();
    }
    Hu3DModelObjMtxGet(hookDustWork->modelId, "", rootMtx);
    pos.x = PGMinPos.x+((PGMaxPos.x-PGMinPos.x)/2);
    pos.y = PGMinPos.y+((PGMaxPos.y-PGMinPos.y)/2);
    pos.z = PGMinPos.z+((PGMaxPos.z-PGMinPos.z)/2);
    Hu3DModelDispOff(hookDustWork->modelId);
    dustEffParam.vel.x = 0;
    dustEffParam.vel.y = 0;
    dustEffParam.vel.z = 0;
    EffectDustCreate(hookDustWork->modelId, pos.x, pos.y, pos.z, 40.0f, &dustEffParam);
    for(i=0; i<8; i++) {
        dustEffParam.vel.x = frandmod(10)-5;
        dustEffParam.vel.y = frandmod(10)-5;
        dustEffParam.vel.z = frandmod(10)-5;
        EffectDustCreate(hookDustWork->modelId, pos.x, pos.y, pos.z, 20.0f, &dustEffParam);
    }
    MTXIdentity(modelP->mtx);
    KillHookDustProc(HuPrcCurrentGet());
    while(1) {
        HuPrcVSleep();
    }
}

static EFFECTPARAM modelSmokeEffParam = {
    0,
    { 255, 255, 255, 255 },
    { 64, 32, 0, 255 },
    { 0, 2, 1 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    1,
    -5,
    0.02f
};

void CharEffectSmokeCreate(s16 cameraBit, HuVecF *pos)
{
    HU3DPARTICLEDATA *particleDataP;
    s16 i;
    s16 effectNo;
    HU3DPARTICLE *particleP;
    HU3DMODEL *modelP;
    s16 bit;
    s16 cameraNo;
    for(bit=HU3D_CAM0, cameraNo=0; cameraNo<HU3D_CAM_MAX; cameraNo++, bit <<= 1) {
        for(i=0; i<8; i++) {
            effectNo = EffectCreate(EFFECT_SMOKE, bit, pos->x, pos->y, pos->z, 20, &modelSmokeEffParam);
            if(effectNo == -1) {
                break;
            }
            modelP = &Hu3DData[effModelId[EFFECT_SMOKE][cameraNo]];
            particleP = modelP->hookData;
            particleDataP = &particleP->data[effectNo];
            particleDataP->parManId = 2;
            particleDataP->vel.x = 30*HuSin(i*45);
            particleDataP->vel.y = 30*HuCos(i*45);
            particleDataP->vel.z = 0;
            particleDataP->accel = *pos;
            particleDataP->speedDecay = 1.15f;
            particleDataP->colorIdx = 0.1f*(frandmod(20)-10);
            particleDataP->color.a = 255-(frandmod(3)*16);
        }
        for(i=0; i<8; i++) {
            effectNo = EffectCreate(EFFECT_SMOKE, bit, pos->x, pos->y, pos->z, 10, &modelSmokeEffParam);
            if(effectNo == -1) {
                break;
            }
            modelP = &Hu3DData[effModelId[EFFECT_SMOKE][cameraNo]];
            particleP = modelP->hookData;
            particleDataP = &particleP->data[effectNo];
            particleDataP->parManId = 2;
            particleDataP->vel.x = frandmod(100)-50;
            particleDataP->vel.y = frandmod(100)-50;
            particleDataP->vel.z = frandmod(100)-50;
            particleDataP->accel = *pos;
            particleDataP->speedDecay = 1.15f;
            particleDataP->colorIdx = 0.1f*(frandmod(20)-10);
            particleDataP->color.a = 255-(frandmod(3)*16);
        }
        effectNo = EffectCreate(EFFECT_SMOKE, bit, pos->x, pos->y, pos->z, 10, &modelSmokeEffParam);
        if(effectNo == -1) {
            break;
        }
        modelP = &Hu3DData[effModelId[EFFECT_SMOKE][cameraNo]];
        particleP = modelP->hookData;
        particleDataP = &particleP->data[effectNo];
        particleDataP->parManId = 2;
        particleDataP->vel.x = 0;
        particleDataP->vel.y = 0;
        particleDataP->vel.z = 0;
        particleDataP->accel = *pos;
        particleDataP->colorIdx = 0;
        particleDataP->speedDecay = 1.15f;
        particleDataP->color.a = 255;
    }
}

static void UpdateModelEffect(HU3DPARTICLEDATA *particleDataP)
{
    float speed;
    float angle;
    angle = 20+(3.75f*particleDataP->time);
    if(angle > 90.0f) {
        angle = 90.0f;
    }
    speed = HuSin(angle);
    particleDataP->pos.x = particleDataP->accel.x+(particleDataP->vel.x*speed);
    particleDataP->pos.y = particleDataP->accel.y+(particleDataP->vel.y*speed);
    particleDataP->pos.z = particleDataP->accel.z+(particleDataP->vel.z*speed);
    particleDataP->scale *= particleDataP->speedDecay;
    particleDataP->speedDecay -= 0.01;
    if(particleDataP->speedDecay < 1.0f) {
        particleDataP->speedDecay = 1.0f;
    }
    if(particleDataP->time > 8) {
        s16 alpha = particleDataP->color.a;
        alpha -= 8;
        if(alpha < 0) {
            particleDataP->color.a = 0;
            particleDataP->scale = 0;
        } else {
            particleDataP->color.a = alpha;
        }
    }
    particleDataP->time++;
}

static EFFECTPARAM coinEffParam = {
    1,
    { 255, 255, 0, 255 },
    { 255, 255, 0, 255 },
    { 0, 2, 1 },
    { 0.95f, 1.0f, 0.95f },
    -0.1f,
    0,
    -0.2f,
    -8,
    0
};

void CharEffectCoinGlowCreate(s16 cameraBit, HuVecF *pos)
{
    s16 i;
    for(i=0; i<16; i++) {
        s16 angle = i*(360.0f/16.0f);
        s16 effectNo;
        coinEffParam.vel.x = 5*HuSin(angle);
        coinEffParam.vel.y = 0.1f*(frandmod(100)-50);
        coinEffParam.vel.z = 5*HuCos(angle);
        effectNo = EffectCreate(EFFECT_GLOW, cameraBit, pos->x, pos->y, pos->z, 30, &coinEffParam);
        if(effectNo == -1) {
            break;
        }
    }
}

static EFFECTPARAM modelHitEffParam = {
    0,
    { 255, 255, 0, 255 },
    { 255, 128, 0, 255 },
    { 0, 2, 1 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    -0.2f,
    -16,
    0.05f
};

static EFFECTPARAM hitGlowEffParam = {
    0,
    { 224, 32, 32, 255 },
    { 224, 32, 32, 255 },
    { 0, 2, 1 },
    { 1.0f, 1.0f, 1.0f },
    0,
    0,
    -0.2f,
    -12,
    0.05f
};

void CharModelHitCreate(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    s16 motNo;
    Mtx mtx;
    HuVecF pos;
    HuVecF radius;
    
    for(motNo=0; motNo<CHAR_MOT_MAX; motNo++) {
        if(workP->motId[motNo] == modelP->motId) {
            break;
        }
    }
    if(motNo == CHAR_MOT_MAX) {
        return;
    }
    Hu3DModelObjMtxGet(workP->modelId, "test11_tex_we-itemhook-r", mtx);
    pos.x = mtx[0][3];
    pos.y = mtx[1][3];
    pos.z = mtx[2][3];
    mtxRot(mtx, modelP->rot.x, modelP->rot.y, modelP->rot.z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2]; 
    VECScale(&radius, &radius, 20);
    VECAdd(&pos, &radius, &pos);
    CharEffectHitCreate(modelP->cameraBit, &pos, &modelP->rot);
}

void CharEffectHitCreate(s16 cameraBit, HuVecF *pos, HuVecF *rot)
{
    Mtx mtx;
    HuVecF radius;
    HuVecF dir;
    HuVecF vel;
    s16 i;
    s16 effectNo;
    mtxRot(mtx, rot->x, rot->y, rot->z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2]; 
    for(i=0; i<8; i++) {
        float angle = i*45;
        dir.x = ((radius.x*radius.y)*(1-HuCos(angle)))-(radius.z*HuSin(angle));
        dir.y = (radius.y*radius.y)+((1-(radius.y*radius.y))*HuCos(angle));
        dir.z = ((radius.y*radius.z)*(1-HuCos(angle)))+(radius.x*HuSin(angle));
        VECNormalize(&dir, &dir);
        VECScale(&dir, &modelHitEffParam.vel, 10);
        effectNo = EffectCreate(EFFECT_STAR, cameraBit, pos->x, pos->y, pos->z, 20, &modelHitEffParam);
        if(effectNo == -1) {
            break;
        }
        VECScale(&radius, &vel, -2-(0.1*frandmod(20)));
        VECScale(&dir, &dir, 2);
        VECAdd(&dir, &vel, &hitGlowEffParam.vel);
        effectNo = EffectCreate(EFFECT_HIT, cameraBit, pos->x, pos->y, pos->z, 20, &hitGlowEffParam);
        if(effectNo == -1) {
            break;
        }
    }
}

static EFFECTPARAM shoeHitEffParam = {
    0,
    { 32, 32, 255, 255 },
    { 128, 255, 32, 255 },
    { 0, 2, 1 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    -0.2f,
    -16,
    0.06f
};

static EFFECTPARAM shoeHitGlowEffParam = {
    0,
    { 32, 224, 32, 255 },
    { 32, 224, 32, 255 },
    { 0, 2, 1 },
    { 1.0f, 1.0f, 1.0f },
    0,
    0,
    -0.2f,
    -12,
    0.05f
};

void CharModelShoeHitCreate(s16 charNo)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    Mtx mtx;
    HuVecF pos;
    HuVecF radius;

    Hu3DModelObjMtxGet(workP->modelId, "test11_tex_we-ske_R_shoe1", mtx);
    pos.x = mtx[0][3];
    pos.y = mtx[1][3];
    pos.z = mtx[2][3];
    mtxRot(mtx, modelP->rot.x, modelP->rot.y, modelP->rot.z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2]; 
    VECScale(&radius, &radius, 30);
    VECAdd(&pos, &radius, &pos);
    CharEffectShoeHitCreate(modelP->cameraBit, &pos, &modelP->rot);
}

void CharEffectShoeHitCreate(s16 cameraBit, HuVecF *pos, HuVecF *rot)
{
    Mtx mtx;
    HuVecF radius;
    HuVecF dir;
    HuVecF vel;
    s16 i;
    s16 effectNo;
    mtxRot(mtx, rot->x, rot->y, rot->z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2]; 
    for(i=0; i<8; i++) {
        float angle = i*45;
        dir.x = ((radius.x*radius.y)*(1-HuCos(angle)))-(radius.z*HuSin(angle));
        dir.y = (radius.y*radius.y)+((1-(radius.y*radius.y))*HuCos(angle));
        dir.z = ((radius.y*radius.z)*(1-HuCos(angle)))+(radius.x*HuSin(angle));
        VECNormalize(&dir, &dir);
        VECScale(&dir, &shoeHitEffParam.vel, 10);
        effectNo = EffectCreate(EFFECT_STAR, cameraBit, pos->x, pos->y, pos->z, 20, &shoeHitEffParam);
        if(effectNo == -1) {
            break;
        }
        VECScale(&radius, &vel, -2-(0.1*frandmod(20)));
        VECScale(&dir, &dir, 2);
        VECAdd(&dir, &vel, &shoeHitGlowEffParam.vel);
        effectNo = EffectCreate(EFFECT_HIT, cameraBit, pos->x, pos->y, pos->z, 20, &shoeHitGlowEffParam);
        if(effectNo == -1) {
            break;
        }
    }
}

void CharEffectLayerSet(s16 layerNo)
{
    s16 i, j;
    for(i=0; i<EFFECT_MAX; i++) {
        for(j=0; j<HU3D_CAM_MAX; j++) {
            if(effModelId[i][j] != HU3D_MODELID_NONE) {
                Hu3DModelLayerSet(effModelId[i][j], layerNo);
            }
        }
    }
    effectLayer = layerNo;
}

void CharMotionVoiceOnSet(s16 charNo, s16 motNo, BOOL voiceOn)
{
    CHARWORK *workP = &charWork[charNo];
    s16 i;
    if(workP->modelId == HU3D_MODELID_NONE) {
        return;
    }
    motNo = CHAR_MOTNO(motNo);
    for(i=0; i<CHAR_MOT_MAX; i++) {
        if(workP->motNoTbl[i] == motNo) {
            break;
        }
    }
    if(i == CHAR_MOT_MAX) {
        return;
    }
    if(!voiceOn) {
        workP->voiceFlag[i] |= 0x1;
    } else {
        workP->voiceFlag[i] &= ~0x1;
    }
}

void CharModelVoicePanAutoSet(s16 charNo, BOOL voicePanAuto)
{
    CHARWORK *workP = &charWork[charNo];
    if(voicePanAuto) {
        workP->attr |= 0x8;
    } else {
        workP->attr &= ~0x8;
    }
}

void CharModelFxFlagSet(s16 charNo, BOOL fxFlag)
{
    CHARWORK *workP = &charWork[charNo];
    if(charNo >= CHARNO_MAX) {
        if(!fxFlag) {
            dustFlags[charNo] |= 0x10;
        } else {
            dustFlags[charNo] &= ~0x10;
        }
        return;
    } else {
        if(!fxFlag) {
            workP->attr |= 0x10;
        } else {
            workP->attr &= ~0x10;
        }
    }
}


void CharMotionUpdateSet(s16 charNo, unsigned int dataNum, BOOL updateF)
{
    CHARWORK *workP = &charWork[charNo];
    s16 i;
    if(charNo >= CHARNO_MAX) {
        return;
    }
    dataNum = FILENUM(dataNum);
    for(i=0; i<CHAR_MOT_MAX; i++) {
        if(workP->motNoTbl[i] == dataNum) {
            break;
        }
    }
    if(i == CHAR_MOT_MAX) {
        return;
    }
    if(updateF) {
        workP->voiceFlag[i] &= ~0x2;
    } else {
        workP->voiceFlag[i] |= 0x2;
    }
}

typedef struct NpcDustWork_s {
    HU3DMODELID modelId;
    HU3DMOTID motId;
    s16 type;
    s16 npcNo;
} NPCDUSTWORK;

static void UpdateNpcDust(void);

s32 CharNpcDustSet(HU3DMODELID modelId, HU3DMOTID motId, s16 type, s16 npcNo)
{
    HUPROCESS *parent = HuPrcCurrentGet();
    HUPROCESS *process = HuPrcChildCreate(UpdateNpcDust, 100, 8192, 0, parent);
    if(!process) {
        return;
    } else {
        NPCDUSTWORK *work = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, sizeof(NPCDUSTWORK), HU_MEMNUM_OVL);
        process->property = work;
        work->modelId = modelId;
        work->motId = motId;
        work->type = type;
        work->npcNo = npcNo;
        EffectInit();
    }
}

s32 CharNpcDustVoiceOffSet(HU3DMODELID modelId, HU3DMOTID motId, s16 type)
{
    s32 ret;
    CharNpcDustSet(modelId, motId, type, CHAR_NPC_NONE);
    return ret;
}

static s8 npcSeTimeTbl[12] = {
    7, 32, 1, 30,
    1, 30, 1, 20,
    19, 32, 2, 19
};

static u16 npcSeTbl[12] = {
    0, 0, 0, 0,
    0, 0, 288, 2590,
    286, 273, 59367, 59367
};

static u16 npcSeTbl2[7] = {};

static s8 npcSeTimeTbl2[8] = {
    3, 55, -25, -25,
    -25, -25, -25, -25
};

static u16 npcSeTbl3[4] = {};

static s8 npcSeTimeTbl3[8] = {
    4, 17, -25, -25,
    -25, -25, -25, -25
};

static u16 npcSeTbl4[4] = {};

static s8 npcSeTimeTbl4[8] = {
    1, 23, -25, -25,
    -25, -25, -25, -25
};

static u16 npcSeTbl5[4] = {};

static void UpdateNpcDust(void)
{
    NPCDUSTWORK *work = HuPrcCurrentGet()->property;
    HU3DMODELID modelId = work->modelId;
    s16 time = 0;
    HU3DMODEL *modelP = &Hu3DData[work->modelId];
    s16 npcNo = work->npcNo-CHAR_NPC_NO_BASE;
    Vec pos;
    s16 i;
    while(1) {
        HuPrcVSleep();
        if(Hu3DMotionIDGet(modelId) != work->motId) {
            continue;
        }
        if(modelP->attr & HU3D_ATTR_DISPOFF) {
            continue;
        }
        time = Hu3DMotionTimeGet(modelId);
        switch(work->type) {
            case 0:
                if((time & 0xF) == 0) {
                    if(!(dustFlags[npcNo] & 0x10)) {
                        dustEffParam.vel.x = -HuSin(modelP->rot.y)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -HuCos(modelP->rot.y)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
                    }
                }
                if(work->npcNo != CHAR_NPC_NONE) {
                    for(i=0; i<2; i++) {
                        if(npcSeTimeTbl[(npcNo*2)+i] == (time*2)) {
                            HuAudFXPlay(npcSeTbl[npcNo]);
                            break;
                        }
                    }
                }
                break;
            
            case 1:
                if((time & 0x3) == 0) {
                    if(!(dustFlags[npcNo] & 0x10)) {
                        dustEffParam.vel.x = -HuSin(modelP->rot.y)*4;
                        dustEffParam.vel.y = 2+(0.1*frandmod(10));
                        dustEffParam.vel.z = -HuCos(modelP->rot.y)*4;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
                    }
                }
                if(work->npcNo != CHAR_NPC_NONE) {
                    for(i=0; i<2; i++) {
                        if(npcSeTimeTbl[(npcNo*2)+i] == (time*2)) {
                            HuAudFXPlay(npcSeTbl2[npcNo]);
                            break;
                        }
                    }
                }
                break;
            
            case 2:
                if(npcNo == CHAR_NPC_NONE) {
                    continue;
                }
                if((time & 0x1F) == 0) {
                    if(!(dustFlags[npcNo] & 0x10)) {
                        dustEffParam.vel.x = -HuSin(modelP->rot.y)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -HuCos(modelP->rot.y)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
                    }
                }
                for(i=0; i<2; i++) {
                    if(npcSeTimeTbl2[(npcNo*2)+i] == (time*2)) {
                        HuAudFXPlay(npcSeTbl3[npcNo]);
                        break;
                    }
                }
                break;
             
            case 3:
                if(npcNo == CHAR_NPC_NONE) {
                    continue;
                }
                if((time & 0x3) == 0) {
                    if(!(dustFlags[npcNo] & 0x10)) {
                        dustEffParam.vel.x = -HuSin(modelP->rot.y)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -HuCos(modelP->rot.y)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
                    }
                }
                for(i=0; i<2; i++) {
                    if(npcSeTimeTbl3[(npcNo*2)+i] == (time*2)) {
                        HuAudFXPlay(npcSeTbl4[npcNo]);
                        break;
                    }
                }
                break;
             
            case 4:
                if(npcNo == CHAR_NPC_NONE) {
                    continue;
                }
                if((time & 0x7) == 0) {
                    if(!(dustFlags[npcNo] & 0x10)) {
                        dustEffParam.vel.x = -HuSin(modelP->rot.y)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -HuCos(modelP->rot.y)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
                    }
                }
                for(i=0; i<2; i++) {
                    if(npcSeTimeTbl4[(npcNo*2)+i] == (time*2)) {
                        HuAudFXPlay(npcSeTbl5[npcNo]);
                        break;
                    }
                }
                break;
             
            case 5:
                if(time != 0) {
                    continue;
                }
                if(!(dustFlags[npcNo] & 0x10)) {
                    for(i=0; i<8; i++){ 
                        npcHitEffParam.vel.x = 10*HuSin(45.0f*i)*modelP->scale.x;
                        npcHitEffParam.vel.y = 0;
                        npcHitEffParam.vel.z = 10*HuCos(45.0f*i)*modelP->scale.x;
                        EffectStarCreate(modelId, modelP->pos.x, modelP->pos.y+(10*modelP->scale.x), modelP->pos.z, 40, &npcHitEffParam);
                    }
                    for(i=0; i<8; i++){ 
                        dustEffParam.vel.x = 4*HuSin((45.0f*i)+22.5)*modelP->scale.x;
                        dustEffParam.vel.y = 0;
                        dustEffParam.vel.z = 4*HuCos((45.0f*i)+22.5)*modelP->scale.x;
                        EffectDustCreate(modelId, modelP->pos.x, modelP->pos.y+(10*modelP->scale.x), modelP->pos.z, 20, &dustEffParam);
                    }
                    if(npcNo != CHAR_NPC_NONE) {
                        HuAudFXPlay(MSM_SE_CMN_01);
                    }
                }
                break;
        }
    }
}

void CharModelStepFxSet(s16 charNo, s16 stepFx)
{
    CHARWORK *workP = &charWork[charNo];
    workP->stepFx = stepFx;
}

static s32 PlayStepVoice(s16 charNo, s16 seId, u8 voiceFlag)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    if(voiceFlag & 0x1) {
        return;
    }
    seId += workP->stepFx;
    if(workP->attr & 0x8) {
        HuAudCharFXPlayPos(charNo, seId, &modelP->pos);
    } else {
        PlayVoice(charNo, seId, voiceFlag);
    }
}

static EFFECTPARAM landEffParam = {
    0,
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 0, 2, 1 },
    { 0.85f, 0.85f, 0.85f },
    0.2f,
    0,
    1,
    -8,
    0.02f
};

static EFFECTPARAM hipDropLandEffParam = {
    0,
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 0, 2, 1 },
    { 0.9f, 0.9f, 0.9f },
    0.2f,
    0,
    1,
    -8,
    0.02f
};

static EFFECTPARAM hipDropStarEffParam = {
    0,
    { 255, 255, 0, 255 },
    { 255, 255, 0, 255 },
    { 0, 0, 0 },
    { 0.9f, 0.9f, 0.9f },
    -0.3f,
    0,
    -0.5f,
    -10,
    0
};

void CharModelLandDustCreate(s16 charNo, HuVecF *pos)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    s16 i;
    for(i=0; i<12; i++) {
        float speed = (6.0+frandmod(6))*modelP->scale.x;
        Vec effectPos;
        landEffParam.vel.x = speed*HuSin(30.0f*i);
        landEffParam.vel.y = 0;
        landEffParam.vel.z = speed*HuCos(30.0f*i);
        landEffParam.alphaBase = -(frandmod(4)+8);
        speed = (20.0+frandmod(20))*modelP->scale.x;
        effectPos.x = pos->x+(20*HuSin(30.0f*i)*modelP->scale.x);
        effectPos.y = pos->y+(10*modelP->scale.x);
        effectPos.z = pos->z+(20*HuCos(30.0f*i)*modelP->scale.x);
        landEffParam.gravity = frandmod(20)*0.01;
        EffectCreate(EFFECT_LANDDUST, modelP->cameraBit, effectPos.x, effectPos.y, effectPos.z, speed, &landEffParam);
    }
}

static void CreateHipDropEffect(s16 charNo, HuVecF *pos)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    s16 i;
    for(i=0; i<12; i++) {
        float speed = (6.0+frandmod(6))*modelP->scale.x;
        Vec effectPos;
        hipDropLandEffParam.vel.x = speed*HuSin(30.0f*i);
        hipDropLandEffParam.vel.y = 0;
        hipDropLandEffParam.vel.z = speed*HuCos(30.0f*i);
        hipDropLandEffParam.alphaBase = -(frandmod(4)+8);
        speed = (30.0+frandmod(20))*modelP->scale.x;
        effectPos.x = pos->x+(20*HuSin(30.0f*i)*modelP->scale.x);
        effectPos.y = pos->y+(15*modelP->scale.x);
        effectPos.z = pos->z+(20*HuCos(30.0f*i)*modelP->scale.x);
        hipDropLandEffParam.gravity = frandmod(20)*0.01;
        EffectCreate(EFFECT_LANDDUST, modelP->cameraBit, effectPos.x, effectPos.y, effectPos.z, speed, &hipDropLandEffParam);
    }
    for(i=0; i<8; i++) {
        hipDropStarEffParam.vel.x = (20*HuSin(45.0f*i)*modelP->scale.x);
        hipDropStarEffParam.vel.y = 5;
        hipDropStarEffParam.vel.z = (20*HuCos(45.0f*i)*modelP->scale.x);
        EffectStarCreate(workP->modelId, modelP->pos.x, modelP->pos.y+(10*modelP->scale.x), modelP->pos.z, 40, &hipDropStarEffParam);
    }
}

void CharEffectDustCreate(s16 cameraBit, float scale, HuVecF *pos)
{
    s16 i;
    for(i=0; i<8; i++) {
        dustEffParam.vel.x = scale*(4*HuSin(i*45.0f));
        dustEffParam.vel.y = 0;
        dustEffParam.vel.z = scale*(4*HuCos(i*45.0f));
        EffectCreate(EFFECT_DUST, cameraBit, pos->x, pos->y*scale, pos->z, 20*scale, &dustEffParam);
    }
    for(i=0; i<8; i++) {
        dustEffParam.vel.x = scale*(2*HuSin((i*45.0f)+22.5));
        dustEffParam.vel.y = 0;
        dustEffParam.vel.z = scale*(2*HuCos((i*45.0f)+22.5));
        EffectCreate(EFFECT_DUST, cameraBit, pos->x, pos->y*scale, pos->z, 20*scale, &dustEffParam);
    }
}

static EFFECTPARAM cryEffParam = {
    0,
    { 128, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 0, 2, 1 },
    { 0.85f, 0.85f, 0.85f },
    -0.5f,
    0,
    -0.1f,
    -16,
    0.02f
};

static char *cryHookTbl[CHARNO_MAX] = {
    "root_head",
    "root_head",
    "root_hair1",
    "ske_head",
    "eff_head",
    "hair1",
    "eff_head",
    "root_head",
    "root_head",
    "root_head",
    "root_head",
    "root_head",
    "root_head"
};

void CharEffectCryCreate(s16 cameraBit, HuVecF *pos, HuVecF *offset, float scale)
{
    float randZ;
    float randX;
    Mtx rotMtx;
    Vec effectPos;
    HuVecF dir;
    randX = frandmod(180)-90;
    randZ = frandmod(180)-90;
    VECNormalize(offset, &dir);
    mtxRot(rotMtx, frandmod(90)-45, frandmod(90)-45, 0);
    MTXMultVec(rotMtx, &dir, &dir);
    VECNormalize(&dir, &dir);
    cryEffParam.vel.x = 15*dir.x;
    cryEffParam.vel.y = 15*dir.y;
    cryEffParam.vel.z = 15*dir.z;
    effectPos.x = pos->x+(scale*(frandmod(30)-15));
    effectPos.y = pos->y+(scale*(frandmod(20)-10));
    effectPos.z = pos->z+(scale*(frandmod(30)-15));
    EffectCreate(EFFECT_CRY, cameraBit, effectPos.x, effectPos.y, effectPos.z, scale*8, &cryEffParam);
}

void CharModelCryCreate(s16 charNo, float yOfs, float ofsY)
{
    CHARWORK *workP = &charWork[charNo];
    HU3DMODEL *modelP = &Hu3DData[workP->modelId];
    Vec pos;
    Vec offset;
    Hu3DModelObjPosGet(workP->modelId, cryHookTbl[charNo], &pos);
    pos.y += yOfs;
    offset.x = HuSin(modelP->rot.y+180.0f);
    offset.y = ofsY;
    offset.z = HuCos(modelP->rot.y+180.0f);
    CharEffectCryCreate(modelP->cameraBit, &pos, &offset, modelP->scale.x);
}

unsigned int CharModelFileNumGet(s16 charNo, s16 model)
{
    if(model & CHAR_MODEL0) {
        return charDataDirTbl[charNo][0];
    } else if(model & CHAR_MODEL1) {
        return charDataDirTbl[charNo][1];
    } else if(model & CHAR_MODEL2) {
        return charDataDirTbl[charNo][2];
    } else {
        return charDataDirTbl[charNo][3];
    }
}

unsigned int CharMotionFileNumGet(s16 charNo, u16 motId)
{
    return charDataDirTbl[charNo][4]|motId;
}

void CharModelVoicePanSet(s16 charNo, s16 pan)
{
    CHARWORK *workP = &charWork[charNo];
    workP->pan = pan;
}

void CharModelVoiceVolSet(s16 charNo, s16 vol)
{
    CHARWORK *workP = &charWork[charNo];
    workP->vol = vol;
}


typedef struct WinLoseVoicePlay_s {
    s16 charNo;
    s16 seId;
    unsigned int motId;
} WINLOSEVOICEPLAY;

static void PlayWinLoseVoice(void);

void CharWinLoseVoicePlay(s16 charNo, unsigned int motId, s16 seId)
{
    HUPROCESS *parent = HuPrcCurrentGet();
    HUPROCESS *process = HuPrcChildCreate(PlayWinLoseVoice, 100, 6144, 0, parent);
    if(!process) {
        OSReport("Error: CharWinLoseVoicePlay Failure.\n");
        return;
    } else {
        WINLOSEVOICEPLAY *winLose = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, sizeof(WINLOSEVOICEPLAY), HU_MEMNUM_OVL);
        process->property = winLose;
        winLose->charNo = charNo;
        winLose->seId = seId;
        winLose->motId = motId;
    }
}

static void PlayWinLoseVoice(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    WINLOSEVOICEPLAY *winLose = process->property;
    CHARWORK *workP = &charWork[winLose->charNo];
    s16 i;
    s16 motNo;
    
    if(workP->modelId == HU3D_MODELID_NONE) {
        HuMemDirectFree(winLose);
        HuPrcEnd();
    }
    for(i=0; i<CHAR_MOT_MAX; i++) {
        if(workP->motId[i] == winLose->motId) {
            motNo = workP->motNoTbl[i];
            break;
        }
    }
    switch(motNo) {
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_306):
            if(omcurovl >= DLL_w01dll && omcurovl <= DLL_w20dll) {
                HuPrcSleep(boardWinAnimLen[winLose->charNo]);
            } else {
                HuPrcSleep(winAnimLen[winLose->charNo]);
            }
            break;
       
       case CHAR_MOTNO(CHARMOT_HSF_c000m1_356):
            HuPrcSleep(handUpAnimLen[winLose->charNo]);
            break;
       
       case CHAR_MOTNO(CHARMOT_HSF_c000m1_358):
            HuPrcSleep(okAnimLen[winLose->charNo]);
            break;
       
       case CHAR_MOTNO(CHARMOT_HSF_c000m1_359):
            HuPrcSleep(winAnimLen2[winLose->charNo]);
            break;
    }
    PlayVoice(winLose->charNo, winLose->seId, 0);
    HuMemDirectFree(process->property);
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

void CharLoseVoicePlay(s16 charNo1, s16 charNo2, s16 charNo3, s16 charNo4)
{
    if(charNo1 != CHARNO_NONE) {
        PlayVoice(charNo1, CHARVOICEID(10), 0);
    }
    if(charNo2 != CHARNO_NONE) {
        PlayVoice(charNo2, CHARVOICEID(10), 0);
    }
    if(charNo3 != CHARNO_NONE) {
        PlayVoice(charNo3, CHARVOICEID(10), 0);
    }
    if(charNo4 != CHARNO_NONE) {
        PlayVoice(charNo4, CHARVOICEID(10), 0);
    }
}

s16 CharMotionTotalTimeGet(s16 charNo, int motNo)
{
    switch(motNo) {
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_306):
            return winAnimLen[charNo];
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_356):
            return handUpAnimLen[charNo];
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_358):
            return okAnimLen[charNo];
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_359):
            return winAnimLen2[charNo];
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_380):
            return holdUpAnimLen[charNo];
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_385):
            return winJumpUpAnimLen[charNo];
        
        case CHAR_MOTNO(CHARMOT_HSF_c000m1_361):
            return loseAnimLen[charNo];
        
        default:
            return 0;
    }
}

s16 CharMotionExtraTimeGet(s16 charNo, int motNo)
{
    return 0;
}