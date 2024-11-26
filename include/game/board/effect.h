#ifndef _BOARD_EFFECT_H
#define _BOARD_EFFECT_H

#include "game/hu3d.h"
#include "game/sprite.h"

//Particle Blend Modes
#define MB_EFFECT_BLEND_NORMAL 0
#define MB_EFFECT_BLEND_ADDCOL 1
#define MB_EFFECT_BLEND_INVCOL 2

#define MB_EFFECT_ATTR_LOOP (1 << 0)
#define MB_EFFECT_ATTR_STOPCNT (1 << 1)
#define MB_EFFECT_ATTR_UPAUSE (1 << 3)
#define MB_EFFECT_ATTR_3D (1 << 4)

typedef struct MBEffect_s MBEFFECT;
typedef void (*MBEFFHOOK)(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix);

typedef struct MBEffectData_s {
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
    HuVecF rot;
    HuVecF pos;
    GXColor color;
    s16 animBank;
    s16 animNo;
    float animSpeed;
    float animTime;
    u8 dispF : 1;
    u8 pauseF : 1;
} MBEFFECTDATA;

struct MBEffect_s {
    s16 mode;
    s16 time;
    HuVecF vel;
    s16 work[8];
    u8 blendMode;
    u8 attr;
    HU3DMODELID modelId;
    s16 num;
    u32 count;
    u32 prevCounter;
    u32 prevCount;
    u32 dlSize;
    ANIMDATA *anim;
    MBEFFECTDATA *data;
    HuVecF *vertex;
    HuVec2F *st;
    void *dl;
    MBEFFHOOK hook;
};

void MBEffFadeOutSet(s16 maxTime);
void MBEffFadeCreate(s16 maxTime, u8 alpha);
BOOL MBEffFadeDoneCheck(void);
void MBEffFadeCameraSet(u16 bit);
void MBEffConfettiCreate(HuVecF *pos, s16 maxCnt, float width);
void MBEffConfettiKill(void);
void MBEffConfettiReset(void);
HU3DMODELID MBEffCreate(ANIMDATA *anim, s16 maxCnt);
void MBEffKill(HU3DMODELID modelId);
void MBEffHookSet(HU3DMODELID modelId, MBEFFHOOK hook);
void MBEffAttrSet(HU3DMODELID modelId, u8 attr);
void MBEffAttrReset(HU3DMODELID modelId, u8 attr);
MBEFFECTDATA *MBEffDataCreate(MBEFFECT *effP);
int MBEffUnkTotalGet(void *ptr, int no);

#endif