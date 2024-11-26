#ifndef _BOARD_EFFECT_H
#define _BOARD_EFFECT_H

#include "game/hu3d.h"
#include "game/sprite.h"

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
    u8 hideF : 1;
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
    HuVecF *st;
    void *dl;
    MBEFFHOOK hook;
};

void MBEffKill(HU3DMODELID modelId);
HU3DMODELID MBEffCreate(ANIMDATA *anim, s16 maxCnt);
void MBEffHookSet(HU3DMODELID modelId, MBEFFHOOK hook);

#endif