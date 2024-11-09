#ifndef _GAME_HSFMAN_H
#define _GAME_HSFMAN_H

#include "game/hsfformat.h"
#include "game/memory.h"
#include "game/data.h"

//Max for model properties
#define HU3D_CLUSTER_MAX 4
#define HU3D_MODEL_LLIGHT_MAX 8

//Max for layer properties
#define HU3D_LAYER_HOOK_MAX 16
#define HU3D_LAYER_MAX 8
#define HU3D_LAYER_HOOK_POST 8

//Max for global buffers
#define HU3D_MODEL_MAX 512
#define HU3D_GLIGHT_MAX 8
#define HU3D_LLIGHT_MAX 48
#define HU3D_PROJ_MAX 4
#define HU3D_MOTION_MAX 384
#define HU3D_TEXANIM_MAX 256
#define HU3D_TEXSCROLL_MAX 16
#define HU3D_PARMAN_MAX 64
#define HU3D_WAVE_MAX 32

//Motion attributes
#define HU3D_MOTATTR 0x40000000
#define HU3D_MOTATTR_NONE 0
#define HU3D_MOTATTR_LOOP 0x40000001
#define HU3D_MOTATTR_PAUSE 0x40000002
#define HU3D_MOTATTR_REV 0x40000004
#define HU3D_MOTATTR_SHIFT_LOOP 0x40000008
#define HU3D_MOTATTR_SHIFT_PAUSE 0x40000010
#define HU3D_MOTATTR_SHIFT_REV 0x40000020
#define HU3D_MOTATTR_SHAPE_LOOP 0x40000040
#define HU3D_MOTATTR_SHAPE_PAUSE 0x40000080
#define HU3D_MOTATTR_SHAPE_REV 0x40000100
#define HU3D_MOTATTR_OVL_LOOP 0x40000200
#define HU3D_MOTATTR_OVL_PAUSE 0x40000400
#define HU3D_MOTATTR_OVL_REV 0x40000800
#define HU3D_MOTATTR_EXECSHIFT 0x40001000

//Cluster Attributes
#define HU3D_CLUSTER_ATTR ((s32)0xC0000000)
#define HU3D_CLUSTER_ATTR_LOOP ((s32)0xC0000001)
#define HU3D_CLUSTER_ATTR_PAUSE ((s32)0xC0000002)
#define HU3D_CLUSTER_ATTR_REV ((s32)0xC0000004)

//Model attributes
#define HU3D_ATTR_NONE 0
#define HU3D_ATTR_DISPOFF (1 << 0)
#define HU3D_ATTR_ZWRITE_OFF (1 << 1)
#define HU3D_ATTR_SHADOW (1 << 2)
#define HU3D_ATTR_MOTION_OFF (1 << 3)
#define HU3D_ATTR_HOOKFUNC (1 << 4)
#define HU3D_ATTR_PARTICLE (1 << 5)
#define HU3D_ATTR_TEX_NEAR (1 << 6)
#define HU3D_ATTR_ENVELOPE_OFF (1 << 7)
#define HU3D_ATTR_MOT_SLOW (1 << 8)
#define HU3D_ATTR_TOON_MAP (1 << 9)
#define HU3D_ATTR_CLUSTER_ON (1 << 10)
#define HU3D_ATTR_MOT_EXEC (1 << 11)
#define HU3D_ATTR_LLIGHT (1 << 12)
#define HU3D_ATTR_CAMERA_MOTON (1 << 13)
#define HU3D_ATTR_NOCULL (1 << 14)
#define HU3D_ATTR_HOOK (1 << 15)
#define HU3D_ATTR_CAMERA (1 << 16)
#define HU3D_ATTR_HILITE (1 << 17)
#define HU3D_ATTR_TPLVL_SET (1 << 18)
#define HU3D_ATTR_CURVE_MOTOFF (1 << 19)
#define HU3D_ATTR_LINK (1 << 20)
#define HU3D_ATTR_NOPAUSE (1 << 21)
#define HU3D_ATTR_ZCMP_OFF (1 << 22)
#define HU3D_ATTR_CULL_FRONT (1 << 23)
#define HU3D_ATTR_REFLECT_MODEL (1 << 24)
#define HU3D_ATTR_MOTION_MODEL (1 << 25)
#define HU3D_ATTR_COLOR_NOUPDATE (1 << 26)

//Texture Animation Attributes
#define HU3D_ANIM_ATTR_NONE 0
#define HU3D_ANIM_ATTR_ANIMON (1 << 0)
#define HU3D_ANIM_ATTR_LOOP (1 << 1)
#define HU3D_ANIM_ATTR_NOUSE (1 << 2)
#define HU3D_ANIM_ATTR_PAUSE (1 << 5)

//Attribute Animation Attributes
#define HU3D_ATTRANIM_ATTR_NONE 0
#define HU3D_ATTRANIM_ATTR_ANIM2D (1 << 0)
#define HU3D_ATTRANIM_ATTR_TEXMTX (1 << 1)
#define HU3D_ATTRANIM_ATTR_ANIM3D (1 << 2)
#define HU3D_ATTRANIM_ATTR_BMPANIM (1 << 3)

//Texture Scroll Attributes
#define HU3D_TEXSCR_ATTR_NONE 0
#define HU3D_TEXSCR_ATTR_POSMOVE (1 << 0)
#define HU3D_TEXSCR_ATTR_ROTMOVE (1 << 1)
#define HU3D_TEXSCR_ATTR_SCALEMOVE (1 << 2)
#define HU3D_TEXSCR_ATTR_PAUSEDISABLE (1 << 3)

//Particle Attributes
#define HU3D_PARTICLE_ATTR_NONE 0
#define HU3D_PARTICLE_ATTR_RESETCNT (1 << 0)
#define HU3D_PARTICLE_ATTR_STOPCNT (1 << 1)
#define HU3D_PARTICLE_ATTR_ANIMON (1 << 3)

//Particle Manager Attributes
#define HU3D_PARMAN_ATTR_NONE 0
#define HU3D_PARMAN_ATTR_TIMEUP (1 << 0)
#define HU3D_PARMAN_ATTR_RANDSPEED90 (1 << 1)
#define HU3D_PARMAN_ATTR_RANDSPEED70 (1 << 2)
#define HU3D_PARMAN_ATTR_RANDSPEED100 (1 << 3)
#define HU3D_PARMAN_ATTR_RANDSCALE90 (1 << 4)
#define HU3D_PARMAN_ATTR_RANDSCALE70 (1 << 5)
#define HU3D_PARMAN_ATTR_SCALEJITTER (1 << 6)
#define HU3D_PARMAN_ATTR_PAUSE (1 << 7)

#define HU3D_PARMAN_ATTR_RANDANGLE (1 << 8)
#define HU3D_PARMAN_ATTR_VACUUM (1 << 9)
#define HU3D_PARMAN_ATTR_RANDTIME90 (1 << 10)
#define HU3D_PARMAN_ATTR_RANDTIME70 (1 << 11)
#define HU3D_PARMAN_ATTR_RANDCOLOR (1 << 11)
#define HU3D_PARMAN_ATTR_SETCOLOR (1 << 12)

#define HU3D_PARTICLE_ATTR_RESETCNT (1 << 0)
#define HU3D_PARTICLE_ATTR_STOPCNT (1 << 1)
#define HU3D_PARTICLE_ATTR_ANIMON (1 << 3)

//Camera attributes
#define HU3D_CAM0 (1 << 0)
#define HU3D_CAM1 (1 << 1)
#define HU3D_CAM2 (1 << 2)
#define HU3D_CAM3 (1 << 3)
#define HU3D_CAM4 (1 << 4)
#define HU3D_CAM5 (1 << 5)
#define HU3D_CAM6 (1 << 6)
#define HU3D_CAM7 (1 << 7)
#define HU3D_CAM8 (1 << 8)
#define HU3D_CAM9 (1 << 9)
#define HU3D_CAM10 (1 << 10)
#define HU3D_CAM11 (1 << 11)
#define HU3D_CAM12 (1 << 12)
#define HU3D_CAM13 (1 << 13)
#define HU3D_CAM14 (1 << 14)
#define HU3D_CAM15 (1 << 15)
#define HU3D_CAM_ALL 0xFFFF
#define HU3D_CAM_NONE 0
#define HU3D_CAM_MAX 16

//Reflection types
#define HU3D_REFLECT_TYPE_NONE -1
#define HU3D_REFLECT_TYPE_METAL 0
#define HU3D_REFLECT_TYPE_GROUND 1
#define HU3D_REFLECT_TYPE_ROOM 2
#define HU3D_REFLECT_TYPE_OCEAN 3
#define HU3D_REFLECT_TYPE_LAND 4

//Particle Blend Modes
#define HU3D_PARTICLE_BLENDMODE_NORMAL 0
#define HU3D_PARTICLE_BLENDMODE_ADDCOL 1
#define HU3D_PARTICLE_BLENDMODE_INVCOL 2

//Special IDs
#define HU3D_MODELID_NONE -1
#define HU3D_MOTID_NONE -1
#define HU3D_LIGHTID_NONE -1
#define HU3D_PROJID_NONE -1
#define HU3D_ANIMID_NONE -1
#define HU3D_TEXSCRID_NONE -1
#define HU3D_CLUSTER_NONE -1
#define HU3D_PARMANID_NONE -1

//HSFCONSTDATA attribute values
#define HU3D_CONST_NONE 0
#define HU3D_CONST_XLU (1 << 0)
#define HU3D_CONST_KCOLOR_XLU (1 << 1)
#define HU3D_CONST_REFLECT (1 << 2)
#define HU3D_CONST_SHADOW_MAP (1 << 3)
#define HU3D_CONST_FORCE_POSX (1 << 4)
#define HU3D_CONST_FORCE_POSY (1 << 5)
#define HU3D_CONST_FORCE_POSZ (1 << 6)
#define HU3D_CONST_FORCE_ROTX (1 << 7)
#define HU3D_CONST_FORCE_ROTY (1 << 8)
#define HU3D_CONST_FORCE_ROTZ (1 << 9)
#define HU3D_CONST_SHADOW (1 << 10)
#define HU3D_CONST_ALTBLEND (1 << 11)
#define HU3D_CONST_LOCK (1 << 12)
#define HU3D_CONST_DISPOFF (1 << 13)
#define HU3D_CONST_XLUVERTEX (1 << 14)
#define HU3D_CONST_HILITE (1 << 15)
#define HU3D_CONST_NEAR (1 << 16)
#define HU3D_CONST_MATHOOK (1 << 17)
#define HU3D_CONST_REFLECTMODEL (1 << 18)

#define HU3D_WATER_ANIM_NONE ((ANIMDATA *)1)

#define Hu3DModelDispOff(modelId) Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF)
#define Hu3DModelDispOn(modelId) Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF)

#define Hu3DModelZWriteOff(modelId) Hu3DModelAttrSet(modelId, HU3D_ATTR_ZWRITE_OFF)
#define Hu3DModelZWriteOn(modelId) Hu3DModelAttrReset(modelId, HU3D_ATTR_ZWRITE_OFF)

#define Hu3DModelCreateData(dataNum) (Hu3DModelCreate(HuDataSelHeapReadNum((dataNum), HU_MEMNUM_OVL, HUHEAPTYPE_MODEL)))
#define Hu3DJointMotionData(model, dataNum) (Hu3DJointMotion((model), HuDataSelHeapReadNum((dataNum), HU_MEMNUM_OVL, HUHEAPTYPE_MODEL)))

//Typedefs for various IDs
typedef s16 HU3DMODELID;
typedef s16 HU3DMOTID;
typedef s16 HU3DPROJID;
typedef s16 HU3DLIGHTID;
typedef s16 HU3DLLIGHTID;
typedef s16 HU3DPARMANID;
typedef s16 HU3DANIMID;
typedef s16 HU3DTEXSCRID;

//Forward declarations
typedef struct Hu3DModel_s HU3DMODEL;
typedef struct Hu3DDrawObj_s HU3DDRAWOBJ;
typedef struct Hu3DParticle_s HU3DPARTICLE;

//Function pointer declarations
typedef void (*HU3DLAYERHOOK)(s16 layerNo);
typedef void (*HU3DMODELHOOK)(HU3DMODEL *modelP, Mtx *mtx);
typedef void (*HU3DTIMINGHOOK)(HU3DMODELID modelId, HU3DMOTID motId, BOOL lagF);
typedef void (*HU3DMATHOOK)(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);
typedef void (*HU3DPARTICLEHOOK)(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);

struct Hu3DDrawObj_s {
    /* 0x00 */ HU3DMODEL *model;
    /* 0x04 */ HSFOBJECT *object;
    /* 0x08 */ float z;
    /* 0x0C */ Mtx matrix;
    /* 0x3C */ HuVecF scale;
};

typedef struct Hu3DAttrAnim_s {
    u16 attr;
    HU3DANIMID animId;
    HU3DTEXSCRID texScrId;
    HuVecF trans3D;
    HuVecF rot;
    HuVecF scale3D;
    HuVec2F scale;
    HuVec2F trans;
    HSFBITMAP *bitMapPtr;
    u32 unk40;
} HU3DATTRANIM;

typedef struct HsfDrawData_s {
    s32 dlOfs;
    s32 dlSize;
    u16 polyCnt;
    u32 flags;
} HSFDRAWDATA;

typedef struct HsfConstData_s {
    u32 attr;
    HU3DMODELID hookMdlId;
    HSFDRAWDATA *drawData;
    void *dlBuf;
    Mtx matrix;
    ANIMDATA *hiliteMap;
    u32 triCnt;
} HSFCONSTDATA;

typedef struct Hu3DMotWork_s {
    float time;
    float speed;
    float start;
    float end;
} HU3DMOTWORK;

struct Hu3DModel_s {
    u8 tick;
    u8 camInfoBit;
    u8 projBit;
    u8 hiliteIdx;
    s8 reflectType;
    s16 layerNo;
    HU3DMOTID motId;
    HU3DMOTID motIdOvl;
    HU3DMOTID motIdShift;
    HU3DMOTID motIdShape;
    HU3DMOTID motIdCluster[HU3D_CLUSTER_MAX];
    s16 clusterAttr[HU3D_CLUSTER_MAX];
    HU3DMOTID motIdSrc;
    u16 cameraBit;
    HU3DMODELID linkMdlId;
    u16 lightNum;
    u16 lightId[HU3D_GLIGHT_MAX];
    HU3DLIGHTID lLightId[HU3D_MODEL_LLIGHT_MAX];
    u32 mallocNo;
    u32 mallocNoLink;
    u32 attr;
    u32 motAttr;
    float ambR;
    float ambB;
    float ambG;
    HU3DMOTWORK motWork;
    HU3DMOTWORK motOvlWork;
    HU3DMOTWORK motShiftWork;
    HU3DMOTWORK motShapeWork;
    float clusterTime[HU3D_CLUSTER_MAX];
    float clusterSpeed[HU3D_CLUSTER_MAX];
    union {
        HSFDATA *hsf;
        HU3DMODELHOOK hookFunc;
    };
    HSFDATA *hsfLink;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    Mtx mtx;
    void *hookData;
    HU3DTIMINGHOOK timingHook;
    HSFOBJECT *timingHookObj;
    HU3DMATHOOK matHook;
    u32 endCounter;
};

typedef struct Hu3DCamera_s {
    float fov;
    float near;
    float far;
    float aspect;
    float upRot;
    Vec pos;
    Vec up;
    Vec target;
    s16 scissorX;
    s16 scissorY;
    s16 scissorW;
    s16 scissorH;
    float viewportX;
    float viewportY;
    float viewportW;
    float viewportH;
    float viewportNear;
    float viewportFar;
} HU3DCAMERA;

typedef struct Hu3DProjection_s {
    u8 alpha;
    ANIMDATA *anim;
    float fov;
    float near;
    float far;
    HuVecF camPos;
    HuVecF camTarget;
    HuVecF camUp;
    Mtx lookAtMtx;
    Mtx projMtx;
} HU3DPROJECTION;

typedef struct Hu3DShadow_s {
    GXColor color;
    u16 size;
    void *buf;
    float fov;
    float near;
    float far;
    HuVecF camPos;
    HuVecF camTarget;
    HuVecF camUp;
    Mtx lookAtMtx;
    Mtx projMtx;
} HU3DSHADOW;

typedef struct Hu3DLight_s {
    s16 type;
    s16 func;
    float cutoff;
    float brightness;
    u8 unkC[16];
    HuVecF pos;
    HuVecF dir;
    HuVecF offset;
    GXColor color;
} HU3DLIGHT;

typedef struct Hu3DMotion_s {
    u16 attr;
    HU3DMODELID modelId;
    HSFDATA *hsf;
} HU3DMOTION;

typedef struct Hu3DParticleData_s {
    s16 time;
    HU3DPARMANID parManId;
    s16 unk04;
    s16 cameraBit;
    HuVecF vel;
    HuVecF accel;
    float speedDecay;
    float colorIdx;
    float scaleBase;
    float scale;
    float zRot;
    HuVecF pos;
    GXColor color;
} HU3DPARTICLEDATA;

typedef struct Hu3DParticle_s {
    s16 dataCnt;
    s16 emitCnt;
    HuVecF pos;
    HuVecF unk_10;
    void *work;
    s16 animBank;
    s16 animNo;
    float animSpeed;
    float animTime;
    u8 blendMode;
    u8 attr;
    s16 unk_2E;
    s16 maxCnt;
    u32 count;
    u32 prevCounter;
    u32 prevCount;
    u32 dlSize;
    ANIMDATA *anim;
    HU3DPARTICLEDATA *data;
    HuVecF *vtxBuf;
    void *dlBuf;
    HU3DPARTICLEHOOK hook;
} HU3DPARTICLE; 

typedef struct Hu3DParmanParam_s {
    s16 maxTime;
    float accelRange;
    float scaleRange;
    float angleRange;
    Vec gravity;
    float speedBase;
    float speedDecay;
    float scaleBase;
    float scaleDecay;
    s16 colorNum;
    GXColor colorStart[4];
    GXColor colorEnd[4];
} HU3DPARMANPARAM;

typedef struct Hu3DTexAnim_s {
    u16 attr;
    s16 bank;
    s16 anmNo;
    HU3DMODELID modelId;
    float time;
    float speed;
    ANIMDATA *anim;
} HU3DTEXANIM; // Size 0x14

typedef struct Hu3DTexScroll_s {
    u16 attr;
    HU3DMODELID modelId;
    HuVecF pos;
    HuVecF scale;
    HuVecF posMove;
    HuVecF scaleMove;
    float rot;
    float rotMove;
    Mtx texMtx;
} HU3DTEXSCROLL;

typedef struct Hu3DParMan_s {
    HU3DMODELID modelId;
    s16 attr;
    s16 timeLimit;
    HU3DPARMANID parManId;
    s16 color;
    Vec pos;
    Vec vec;
    Vec vacuum;
    float vacuumSpeed;
    float accel;
    s32 jitterNo;
    HU3DPARMANPARAM *param;
} HU3DPARMAN;

typedef struct Hu3DWaterWave_s {
    HuVecF pos;
    float radiusMax;
    float radius;
    float texMtx[2][3];
    s16 time;
} HU3DWATERWAVE;

typedef struct Hu3DWater_s {
    s16 layerNo;
    u16 cameraBit;
    float texMtx[2][3];
    s16 glowMode;
    s16 fbWaterW;
    s16 fbWaterH;
    void *fbWater;
    ANIMDATA *animBump;
    ANIMDATA *animSurface;
    ANIMDATA *animSky;
    ANIMDATA *animWave;
    void *fbDisp;
    HuVecF texPos;
    HuVecF texScale;
    HuVecF posMin;
    HuVecF posMax;
    float padY;
    GXColor glowCol;
    GXColor hiliteCol;
    BOOL mipMapF;
    s16 unk7C;
    HU3DWATERWAVE wave[HU3D_WAVE_MAX];
    float maxTime;
} HU3DWATER;

void Hu3DDrawPreInit(void);
void Hu3DDraw(HU3DMODEL *model, Mtx matrix, HuVecF *scale);
BOOL ObjCullCheck(HSFDATA *hsf, HSFOBJECT *object, Mtx mtx);
void Hu3DTevStageNoTexSet(HU3DDRAWOBJ *drawObj, HSFMATERIAL *matP);
void Hu3DTevStageTexSet(HU3DDRAWOBJ *drawObj, HSFMATERIAL *matP);
void Hu3DMatLightSet(HU3DMODEL *modelP, u32 flags, float hilitePower);
void Hu3DDrawPost(void);
void MakeDisplayList(HU3DMODELID modelId, u32 mallocno);
HSFCONSTDATA *ObjConstantMake(HSFOBJECT *object, u32 no);
void mtxTransCat(Mtx dst, float x, float y, float z);
void mtxRotCat(Mtx dst, float x, float y, float z);
void mtxRot(Mtx dst, float x, float y, float z);
void mtxScaleCat(Mtx dst, float x, float y, float z);
s16 HmfInverseMtxF3X3(Mtx dst, Mtx src);
void SetDefLight(HuVecF *pos, HuVecF *dir, u8 colorR, u8 colorG, u8 colorB, u8 ambR, u8 ambG, u8 ambB, u8 matR, u8 matG, u8 matB);
void Hu3DModelObjPosGet(HU3DMODELID modelId, char *objName, HuVecF *pos);
void Hu3DModelObjMtxGet(HU3DMODELID modelId, char *objName, Mtx mtx);
void PGObjCall(HU3DMODEL *model, HSFOBJECT *object);
void PGObjCalc(HU3DMODEL *model, HSFOBJECT *object);
void PGObjReplica(HU3DMODEL *model, HSFOBJECT *object);
HSFOBJECT *Hu3DObjDuplicate(HSFDATA *hsf, u32 mallocNo);
HSFATTRIBUTE *Hu3DAttrDuplicate(HSFDATA *hsf, u32 mallocNo);
HSFMATERIAL *Hu3DMatDuplicate(HSFDATA *hsf, u32 mallocNo);
void Hu3DModelObjDrawInit(void);
void Hu3DModelObjDraw(HU3DMODELID modelId, char *objName, Mtx mtx);
void Hu3DModelObjPtrDraw(HU3DMODELID modelId, HSFOBJECT *objPtr, Mtx mtx);

void Hu3DInit(void);
void Hu3DPreProc(void);
void Hu3DExec(void);
void Hu3DAllKill(void);
void Hu3DBGColorSet(u8 r, u8 g, u8 b);
void Hu3DLayerHookSet(s16 layerNo, HU3DLAYERHOOK hookFunc);
void Hu3DPauseSet(BOOL pauseF);
void Hu3DNoSyncSet(BOOL noSync);
HU3DMODELID Hu3DModelCreate(void *data);
HU3DMODELID Hu3DModelLink(HU3DMODELID linkMdlId);
HU3DMODELID Hu3DHookFuncCreate(HU3DMODELHOOK hookFunc);
void Hu3DModelKill(HU3DMODELID modelId);
void Hu3DModelAllKill(void);
void Hu3DModelPosSet(HU3DMODELID modelId, float posX, float posY, float posZ);
void Hu3DModelPosSetV(HU3DMODELID modelId, HuVecF *pos);
void Hu3DModelRotSet(HU3DMODELID modelId, float rotX, float rotY, float rotZ);
void Hu3DModelRotSetV(HU3DMODELID modelId, HuVecF *rot);
void Hu3DModelScaleSet(HU3DMODELID modelId, float scaleX, float scaleY, float scaleZ);
void Hu3DModelScaleSetV(HU3DMODELID modelId, HuVecF *scale);
void Hu3DModelAttrSet(HU3DMODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3DMODELID modelId, u32 attr);
u32 Hu3DModelAttrGet(HU3DMODELID modelId);
u32 Hu3DModelMotionAttrGet(HU3DMODELID modelId);
void Hu3DModelClusterAttrSet(HU3DMODELID modelId, s16 clusterNo, s32 clusterAttr);
void Hu3DModelClusterAttrReset(HU3DMODELID modelId, s16 clusterNo, s32 clusterAttr);
void Hu3DModelCameraSet(HU3DMODELID modelId, u16 cameraBit);
void Hu3DModelLayerSet(HU3DMODELID modelId, s16 layerNo);
HSFOBJECT *Hu3DModelObjPtrGet(HU3DMODELID modelId, char *objName);
void Hu3DModelTPLvlSet(HU3DMODELID modelId, float tpLvl);
void Hu3DModelHiliteMapSet(HU3DMODELID modelId, ANIMDATA *anim);
void Hu3DModelShadowSet(HU3DMODELID modelId);
void Hu3DModelShadowReset(HU3DMODELID modelId);
void Hu3DModelShadowDispOn(HU3DMODELID modelId);
void Hu3DModelShadowDispOff(HU3DMODELID modelId);
void Hu3DModelShadowMapSet(HU3DMODELID modelId);
void Hu3DModelShadowMapObjSet(HU3DMODELID modelId, char *objName);
void Hu3DModelShadowMapObjReset(HU3DMODELID modelId);
void Hu3DModelAmbSet(HU3DMODELID modelId, float ambR, float ambG, float ambB);
void Hu3DModelHookSet(HU3DMODELID modelId, char *objName, HU3DMODELID hookMdlId);
void Hu3DModelHookReset(HU3DMODELID modelId);
void Hu3DModelHookObjReset(HU3DMODELID modelId, char *objName);
void Hu3DModelProjectionSet(HU3DMODELID modelId, HU3DPROJID projId);
void Hu3DModelProjectionReset(HU3DMODELID modelId, HU3DPROJID projId);
void Hu3DModelHiliteTypeSet(HU3DMODELID modelId, s16 hiliteType);
void Hu3DModelReflectTypeSet(HU3DMODELID modelId, s16 reflectType);
void Hu3DReflectModelSet(HU3DMODELID modelId);
void Hu3DModelMatHookSet(HU3DMODELID modelId, HU3DMATHOOK matHook);
void Hu3DCameraCreate(int cameraBit);
void Hu3DCameraPerspectiveSet(int cameraBit, float fov, float near, float far, float aspect);
void Hu3DCameraViewportSet(int cameraBit, float vpX, float vpY, float vpW, float vpH, float vpNearZ, float vpFarZ);
void Hu3DCameraScissorSet(int cameraBit, unsigned int scissorX, unsigned int scissorY, unsigned int scissorW, unsigned int scissorH);
void Hu3DCameraPosSet(int cameraBit, float posX, float posY, float posZ, float upX, float upY, float upZ, float targetX, float targetY, float targetZ);
void Hu3DCameraPosSetV(int cameraBit, Vec *pos, Vec *up, Vec *target);
void Hu3DCameraPosGet(int cameraBit, Vec *pos, Vec *up, Vec *target);
void Hu3DCameraPerspectiveGet(int cameraBit, float *fov, float *near, float *far);
void Hu3DCameraKill(int cameraBit);
void Hu3DCameraAllKill(void);
void Hu3DCameraSet(s32 cameraNo, Mtx modelView);
BOOL Hu3DModelCameraInfoSet(HU3DMODELID modelId, u16 cameraBit);
s16 Hu3DModelCameraCreate(HU3DMOTID motId, u16 cameraBit);
void Hu3DCameraMotionOn(HU3DMODELID modelId, u16 cameraBit);
void Hu3DCameraMotionStart(HU3DMODELID modelId, u16 cameraBit);
void Hu3DCameraMotionOff(HU3DMODELID modelId);
void Hu3DLighInit(void);
HU3DLIGHTID Hu3DGLightCreate(float posX, float posY, float posZ, float dirX, float dirY, float dirZ, u8 colorR, u8 colorG, u8 colorB);
HU3DLIGHTID Hu3DGLightCreateV(HuVecF *pos, HuVecF *dir, GXColor *color);
HU3DLLIGHTID Hu3DLLightCreate(HU3DMODELID modelId, float posX, float posY, float posZ, float dirX, float dirY, float dirZ, u8 colorR, u8 colorG, u8 colorB);
HU3DLLIGHTID Hu3DLLightCreateV(HU3DMODELID modelId, HuVecF *pos, HuVecF *dir, GXColor *color);
void Hu3DGLightSpotSet(HU3DLIGHTID lightId, GXSpotFn spotFunc, float cutoff);
void Hu3DLLightSpotSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, GXSpotFn spotFunc, float cutoff);
void Hu3DGLightInfinitytSet(HU3DLIGHTID lightId);
void Hu3DLLightInfinitytSet(HU3DMODELID modelId, HU3DLLIGHTID lightId);
void Hu3DGLightPointSet(HU3DLIGHTID lightId, float refDistance, float refBrightness, GXDistAttnFn distFunc);
void Hu3DLLightPointSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float refDistance, float refBrightness, GXDistAttnFn distFunc);
void Hu3DGLightKill(HU3DLIGHTID lightId);
void Hu3DLLightKill(HU3DMODELID modelId, HU3DLLIGHTID lightId);
void Hu3DLightAllKill(void);
void Hu3DGLightColorSet(HU3DLIGHTID lightId, u8 r, u8 g, u8 b, u8 a);
void Hu3DLLightColorSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, u8 r, u8 g, u8 b, u8 a);
void Hu3DGLightPosSetV(HU3DLIGHTID lightId, HuVecF *pos, HuVecF *dir);
void Hu3DLLightPosSetV(HU3DMODELID modelId, HU3DLLIGHTID lightId, HuVecF *pos, HuVecF *dir);
void Hu3DGLightPosSet(HU3DLIGHTID lightId, float posX, float posY, float posZ, float dirX, float dirY, float dirZ);
void Hu3DLLightPosSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float posX, float posY, float posZ, float dirX, float dirY, float dirZ);
void Hu3DGLightPosAngleSet(HU3DLIGHTID lightId, float posX, float posY, float posZ, float angleX, float angleY);
void Hu3DLLightPosAngleSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float posX, float posY, float posZ, float angleX, float angleY);
void Hu3DGLightPosAimSetV(HU3DLIGHTID lightId, HuVecF *pos, HuVecF *aim);
void Hu3DLLightPosAimSetV(HU3DMODELID modelId, HU3DLLIGHTID lightId, HuVecF *pos, HuVecF *aim);
void Hu3DGLightPosAimSet(HU3DLIGHTID lightId, float posX, float posY, float posZ, float aimX, float aimY, float aimZ);
void Hu3DLLightPosAimSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, float posX, float posY, float posZ, float aimX, float aimY, float aimZ);
void Hu3DGLightStaticSet(HU3DLIGHTID lightId, BOOL staticF);
void Hu3DLLightStaticSet(HU3DMODELID modelId, HU3DLLIGHTID lightId, BOOL staticF);
s16 Hu3DLLightParamGet(HU3DLIGHTID lLightId, Vec *pos, Vec *dir, GXColor *color);
s16 Hu3DGLightParamGet(HU3DLIGHTID gLightId, Vec *pos, Vec *dir, GXColor *color);
s32 Hu3DModelLightInfoSet(HU3DMODELID modelId, s16 staticF);
HU3DLIGHTID Hu3DModelLightIdGet(HU3DMODELID modelId, char *objName);
s16 Hu3DLightSet(HU3DMODEL *modelP, Mtx cameraMtx, Mtx cameraMtxXPose, float hilitePower);
void Hu3DReflectMapSet(ANIMDATA *anim);
void Hu3DReflectNoSet(s16 no);
void Hu3DFogSet(float start, float end, u8 r, u8 g, u8 b);
void Hu3DFogClear(void);
void Hu3DAmbColorSet(float r, float g, float b);
void Hu3DShadowMultiCreate(float fov, float near, float far, s16 cameraBit);
void Hu3DShadowCreate(float fov, float near, float far);
void Hu3DShadowMultiPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget, s16 cameraBit);
void Hu3DShadowPosSet(HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget);
void Hu3DShadowMultiTPLvlSet(float tpLvl, s16 cameraBit);
void Hu3DShadowTPLvlSet(float tpLvl);
void Hu3DShadowMultiSizeSet(u16 size, s16 cameraBit);
void Hu3DShadowSizeSet(u16 size);
void Hu3DShadowMultiColSet(u8 r, u8 g, u8 b, s16 cameraBit);
void Hu3DShadowColSet(u8 r, u8 g, u8 b);
HU3DPROJID Hu3DProjectionCreate(ANIMDATA *anim, float fov, float near, float far);
void Hu3DProjectionKill(HU3DPROJID projId);
void Hu3DProjectionPosSet(HU3DPROJID projId, HuVecF *camPos, HuVecF *camUp, HuVecF *camTarget);
void Hu3DProjectionTPLvlSet(HU3DPROJID projId, float tpLvl);
void Hu3DMipMapSet(void *animData, HU3DMODELID modelId, char *bmpName);
void Hu3DFbCopyExec(s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF, void *buf);
void Hu3DFbCopyLayerSet(s16 layerNo, s16 x, s16 y, s16 w, s16 h, GXTexFmt texFmt, s16 mipmapF, void *buf);
void Hu3DZClear(void);
void Hu3DZClearLayerSet(s16 layerNo);

void Hu3DMotionInit(void);
HU3DMOTID Hu3DMotionCreate(void *data);
HU3DMOTID Hu3DMotionModelCreate(HU3DMODELID modelId);
BOOL Hu3DMotionKill(HU3DMOTID motId);
void Hu3DMotionAllKill(void);
void Hu3DMotionSet(HU3DMODELID modelId, HU3DMOTID motId);
void Hu3DMotionOverlaySet(HU3DMODELID modelId, HU3DMOTID motId);
void Hu3DMotionOverlayReset(HU3DMODELID modelId);
float Hu3DMotionOverlayTimeGet(HU3DMODELID modelId);
float Hu3DMotionOverlayMaxTimeGet(HU3DMODELID modelId);
BOOL Hu3DMotionOverlayEndCheck(HU3DMODELID modelId);
void Hu3DMotionOverlayTimeSet(HU3DMODELID modelId, float time);
void Hu3DMotionOverlaySpeedSet(HU3DMODELID modelId, float speed);
void Hu3DMotionShiftSet(HU3DMODELID modelId, HU3DMOTID motId, float start, float end, u32 attr);
void Hu3DMotionShapeSet(HU3DMODELID modelId, HU3DMOTID motId);
HU3DMOTID Hu3DMotionShapeIDGet(HU3DMODELID modelId);
void Hu3DMotionShapeSpeedSet(HU3DMODELID modelId, float speed);
void Hu3DMotionShapeTimeSet(HU3DMODELID modelId, float time);
float Hu3DMotionShapeMaxTimeGet(HU3DMODELID modelId);
void Hu3DMotionShapeStartEndSet(HU3DMODELID modelId, float start, float end);
s16 Hu3DMotionClusterSet(HU3DMODELID modelId, HU3DMOTID motId);
s16 Hu3DMotionClusterNoSet(HU3DMODELID modelId, HU3DMOTID motId, s16 clusterNo);
void Hu3DMotionShapeReset(HU3DMODELID modelId);
void Hu3DMotionClusterReset(HU3DMODELID modelId, s16 clusterNo);
void Hu3DMotionClusterSpeedSet(HU3DMODELID modelId, s16 clusterNo, float speed);
void Hu3DMotionClusterTimeSet(HU3DMODELID modelId, s16 clusterNo, float time);
float Hu3DMotionClusterMaxTimeGet(HU3DMODELID modelId, s16 clusterNo);
HU3DMOTID Hu3DMotionIDGet(HU3DMODELID modelId);
HU3DMOTID Hu3DMotionShiftIDGet(HU3DMODELID modelId);
void Hu3DMotionTimeSet(HU3DMODELID modelId, float time);
float Hu3DMotionTimeGet(HU3DMODELID modelId);
float Hu3DMotionShiftTimeGet(HU3DMODELID modelId);
float Hu3DMotionMaxTimeGet(HU3DMODELID modelId);
float Hu3DMotionShiftMaxTimeGet(HU3DMODELID modelId);
void Hu3DMotionShiftStartEndSet(HU3DMODELID modelId, float start, float end);
float Hu3DMotionMotionMaxTimeGet(HU3DMOTID motId);
void Hu3DMotionStartEndSet(HU3DMODELID modelId, float start, float end);
BOOL Hu3DMotionEndCheck(HU3DMODELID modelId);
void Hu3DMotionSpeedSet(HU3DMODELID modelId, float speed);
void Hu3DMotionShiftSpeedSet(HU3DMODELID modelId, float speed);
void Hu3DMotionNoMotSet(HU3DMODELID modelId, char *objName, u32 forceAttr);
void Hu3DMotionNoMotReset(HU3DMODELID modelId, char *objName, u32 forceAttr);
void Hu3DMotionForceSet(HU3DMODELID modelId, char *objName, u32 forceAttr, float value);
void Hu3DMotionTimingHookSet(HU3DMODELID modelId, HU3DTIMINGHOOK timingHook);
void Hu3DMotionAttrSet(HU3DMOTID motId, u16 attr);
void Hu3DMotionAttrReset(HU3DMOTID motId, u16 attr);
void Hu3DMotionNext(HU3DMODELID modelId);
void Hu3DMotionExec(HU3DMODELID modelId, HU3DMOTID motId, float time, BOOL copyXFormF);
void Hu3DCameraMotionExec(HU3DMODELID modelId);
void Hu3DSubMotionExec(HU3DMODELID modelId);
float *GetObjTRXPtr(HSFOBJECT *objPtr, u16 channel);
void SetObjMatMotion(HU3DMODELID modelId, HSFTRACK *trackP, float value);
void SetObjAttrMotion(HU3DMODELID modelId, HSFTRACK *trackP, float value);
void SetObjCameraMotion(HU3DMODELID modelId, HSFTRACK *trackP, float value);
void SetObjLightMotion(HU3DMODELID modelId, HSFTRACK *trackP, float value);
float GetCurve(HSFTRACK *trackP, float time);
float GetConstant(s32 keyNum, HSFCONSTANTKEY *key, float time);
float GetLinear(s32 keyNum, HSFLINEARKEY *key, float time);
float GetBezier(s32 keyNum, HSFTRACK *trackP, float time);
HSFBITMAP *GetBitMap(s32 keyNum, HSFBITMAPKEY *key, float time);
HU3DMOTID Hu3DJointMotion(HU3DMODELID modelId, void *data);
void JointModel_Motion(HU3DMODELID modelId, HU3DMOTID motId);
void Hu3DMotionCalc(HU3DMODELID modelId);
void Hu3DSubMotionSet(HU3DMODELID modelId, HU3DMOTID motId, float time);
void Hu3DSubMotionReset(HU3DMODELID modelId);
void Hu3DSubMotionTimeSet(HU3DMODELID modelId, float time);
void Hu3DMotionShiftTimeSet(HU3DMODELID modelId, float time);

void Hu3DAnimInit(void);
HU3DANIMID Hu3DAnimCreate(void *dataP, HU3DMODELID modelId, char *bmpName);
HU3DANIMID Hu3DAnimLink(HU3DANIMID linkAnimId, HU3DMODELID modelId, char *bmpName);
void Hu3DAnimKill(HU3DANIMID animId);
void Hu3DAnimModelKill(HU3DMODELID modelId);
void Hu3DAnimAllKill(void);
void Hu3DAnimAttrSet(HU3DANIMID animId, u16 attr);
void Hu3DAnimAttrReset(HU3DANIMID animId, u16 attr);
void Hu3DAnimSpeedSet(HU3DANIMID animId, float speed);
void Hu3DAnimBankSet(HU3DANIMID animId, s32 bank);
void Hu3DAnmNoSet(HU3DANIMID animId, u16 anmNo);
s32 Hu3DAnimSet(HU3DMODEL *modelP, HSFATTRIBUTE *attrP, s16 texSlotNo);
void Hu3DAnimExec(void);
HU3DTEXSCRID Hu3DTexScrollCreate(HU3DMODELID modelId, char *bmpName);
void Hu3DTexScrollKill(HU3DTEXSCRID texScrId);
void Hu3DTexScrollAllKill(void);
void Hu3DTexScrollPosSet(HU3DTEXSCRID texScrId, float posX, float posY, float posZ);
void Hu3DTexScrollPosMoveSet(HU3DTEXSCRID texScrId, float posX, float posY, float posZ);
void Hu3DTexScrollRotSet(HU3DTEXSCRID texScrId, float rot);
void Hu3DTexScrollRotMoveSet(HU3DTEXSCRID texScrId, float rot);
void Hu3DTexScrollPauseDisableSet(HU3DTEXSCRID texScrId, BOOL pauseDiableF);
HU3DMODELID Hu3DParticleCreate(ANIMDATA *anim, s16 maxCnt);
void Hu3DParticleScaleSet(HU3DMODELID modelId, float scale);
void Hu3DParticleZRotSet(HU3DMODELID modelId, float zRot);
void Hu3DParticleColSet(HU3DMODELID modelId, u8 r, u8 g, u8 b);
void Hu3DParticleTPLvlSet(HU3DMODELID modelId, float tpLvl);
void Hu3DParticleBlendModeSet(HU3DMODELID modelId, u8 blendMode);
void Hu3DParticleHookSet(HU3DMODELID modelId, HU3DPARTICLEHOOK hook);
void Hu3DParticleAttrSet(HU3DMODELID modelId, u8 attr);
void Hu3DParticleAttrReset(HU3DMODELID modelId, u8 attr);
void Hu3DParticleCntSet(HU3DMODELID modelId, s16 count);
void Hu3DParticleAnimModeSet(HU3DMODELID modelId, s16 animBank);
void Hu3DParManInit(void);
HU3DPARMANID Hu3DParManCreate(ANIMDATA *anim, s16 maxCnt, HU3DPARMANPARAM *param);
HU3DPARMANID Hu3DParManLink(HU3DPARMANID linkParManId, HU3DPARMANPARAM *param);
void Hu3DParManKill(HU3DPARMANID parManId);
void Hu3DParManAllKill(void);
HU3DPARMAN *Hu3DParManPtrGet(HU3DPARMANID parManId);
void Hu3DParManPosSet(HU3DPARMANID parManId, float posX, float posY, float posZ);
void Hu3DParManVecSet(HU3DPARMANID parManId, float x, float y, float z);
void Hu3DParManRotSet(HU3DPARMANID parManId, float rotX, float rotY, float rotZ);
void Hu3DParManAttrSet(HU3DPARMANID parManId, s32 attr);
void Hu3DParManAttrReset(HU3DPARMANID parManId, s32 attr);
HU3DMODELID Hu3DParManModelIDGet(HU3DPARMANID parManId);
void Hu3DParManTimeLimitSet(HU3DPARMANID parManId, s32 timeLimit);
void Hu3DParManVacumeSet(HU3DPARMANID parManId, float x, float y, float z, float speed);
void Hu3DParManColorSet(HU3DPARMANID parManId, s16 color);
void Hu3DParManLayerSet(HU3DPARMANID parManId, s16 layer);

void Hu3DWaterCreate(s16 layerNo, void *animBump, void *animSurface, void *animSky, BOOL mipMapF, HuVecF *posMin, HuVecF *posMax);
void Hu3DWaterTexPosSet(float posX, float posY, float posZ);
void Hu3DWaterTexScaleSet(float scaleX, float scaleY, float scaleZ);
void Hu3DWaterPadYSet(float padY);
void Hu3DWaterGlowSet(s16 glowMode, GXColor *glowCol);
void Hu3DWaterIndTexMtxSet(float texMtx[2][3]);
void Hu3DWaterHiliteSet(float level);
void Hu3DWaterHiliteTPLvlSet(float tpLvl);
void Hu3DWaterCameraSet(u16 cameraBit);
void Hu3DWaterWaveCreate(HuVecF *pos, float radius, float radiusMax);
void Hu3DTexLoad(void *buf, s16 w, s16 h, u32 format, GXTexWrapMode wrapS, GXTexWrapMode wrapT, BOOL filterF, GXTexMapID texMapId);

extern HuVecF PGMaxPos;
extern HuVecF PGMinPos;
extern u32 totalPolyCnt;
extern u32 totalPolyCnted;
extern u32 totalMatCnt;
extern u32 totalMatCnted;
extern u32 totalTexCnt;
extern u32 totalTexCnted;
extern u32 totalTexCacheCnt;
extern u32 totalTexCacheCnted;

extern HU3DLIGHT Hu3DLocalLight[HU3D_LLIGHT_MAX];
extern HU3DLIGHT Hu3DGlobalLight[HU3D_GLIGHT_MAX];
extern Mtx Hu3DCameraMtxXPose;
extern Mtx Hu3DCameraMtx;
extern HSFSCENE FogData;
extern HU3DSHADOW Hu3DShadowBuf[HU3D_CAM_MAX];
extern HU3DPROJECTION Hu3DProjection[HU3D_PROJ_MAX];
extern ANIMDATA *hiliteAnim[4];
extern ANIMDATA *reflectAnim[5];
extern HU3DCAMERA Hu3DCamera[HU3D_CAM_MAX];
extern HU3DMODEL Hu3DData[HU3D_MODEL_MAX];


extern GXColor BGColor;
extern s16 reflectMapNo;
extern ANIMDATA *toonAnim;
extern s16 shadowNum;
extern s16 Hu3DShadowCamBit;
extern BOOL Hu3DShadowF;
extern BOOL shadowModelDrawF;
extern HU3DSHADOW *Hu3DShadow;
extern s16 Hu3DProjectionNum;
extern s16 Hu3DCameraNo;
extern s16 Hu3DCameraBit;
extern u32 Hu3DMallocNo;
extern s16 Hu3DPauseF;
extern u16 Hu3DCameraExistF;
extern HU3DMODELID Hu3DReflectModelId;
extern ANIMDATA *Hu3DReflectModelAnim;
extern float Hu3DAmbColR;
extern float Hu3DAmbColG;
extern float Hu3DAmbColB;
extern HuVecF PGMaxPos;
extern HuVecF PGMinPos;

extern HU3DMOTION Hu3DMotion[HU3D_MOTION_MAX];

extern HU3DTEXANIM Hu3DTexAnimData[HU3D_TEXANIM_MAX];
extern HU3DTEXSCROLL Hu3DTexScrData[HU3D_TEXSCROLL_MAX];

extern HU3DWATER Hu3DWaterData;

#endif
