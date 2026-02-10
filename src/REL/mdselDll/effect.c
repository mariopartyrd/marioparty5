#ifndef _MDSEL_EFFECT_H
#define _MDSEL_EFFECT_H

#include "game/hu3d.h"

typedef struct MdSelEffect_s {
    int parManId[3];
} MDSEL_EFFECT;

static MDSEL_EFFECT *ParticleDotCreate(ANIMDATA *animP, s16 layer, s16 cameraBit)
{
    static HU3DPARMANPARAM param1 = {
        30,
        0,
        50,
        10,
        180,
        { 0, -0.05f, 0 },
        20,
        0.8f,
        10,
        1,
        2,
        {
            { 0xFF, 0xFF, 0xFF, 0xFF },
            { 0xFF, 0xFF, 0x40, 0xFF },
        },
        {
            { 0xFF, 0x80, 0x80, 0x00 },
            { 0xFF, 0x40, 0x20, 0x00 },
        }
    };
    static HU3DPARMANPARAM param2 = {
        40,
        0,
        50,
        0,
        90,
        { 0, -0.05f, 0 },
        35,
        0.88f,
        25,
        0.99f,
        1,
        {
            { 0xFF, 0xFF, 0xFF, 0xFF },
        },
        {
            { 0xFF, 0x80, 0x80, 0x00 },
        }
    };
    static HU3DPARMANPARAM param3 = {
        30,
        0,
        30,
        60,
        180,
        { 0, 0, 0 },
        0.2f,
        1.0f,
        100,
        1.0f,
        2,
        {
            { 0xFF, 0xFF, 0xFF, 0x20 },
        },
        {
            { 0x40, 0x20, 0x20, 0x00 },
        }
    };
    HU3DPARMANID parManId;
    MDSEL_EFFECT *effectP;
    effectP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDSEL_EFFECT), HU_MEMNUM_OVL);
    parManId = Hu3DParManCreate(animP, 100, &param1);
    effectP->parManId[0] = parManId;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_SCALEJITTER|HU3D_PARMAN_ATTR_RANDSCALE70|HU3D_PARMAN_ATTR_RANDSPEED70|HU3D_PARMAN_ATTR_TIMEUP);
    Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(parManId), HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DParManRotSet(parManId, 90, 0, 0);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(parManId), layer);
    Hu3DModelCameraSet(Hu3DParManModelIDGet(parManId), cameraBit);
    parManId = Hu3DParManCreate(animP, 100, &param2);
    effectP->parManId[1] = parManId;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_RANDANGLE|HU3D_PARMAN_ATTR_SCALEJITTER|HU3D_PARMAN_ATTR_RANDSCALE70|HU3D_PARMAN_ATTR_RANDSPEED90|HU3D_PARMAN_ATTR_TIMEUP);
    Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(parManId), HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DParManRotSet(parManId, frandmod(40)+70, 0, frandmod(80)-40);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(parManId), layer);
    Hu3DModelCameraSet(Hu3DParManModelIDGet(parManId), cameraBit);
    parManId = Hu3DParManCreate(animP, 100, &param3);
    effectP->parManId[2] = parManId;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_RANDSCALE70|HU3D_PARMAN_ATTR_TIMEUP);
    Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(parManId), HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DParManRotSet(parManId, 0, 0, 0);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(parManId), layer);
    Hu3DModelCameraSet(Hu3DParManModelIDGet(parManId), cameraBit);
    return effectP;
}

static void ParticleDotPosSet(MDSEL_EFFECT *effectP, HuVecF *pos)
{
    Hu3DParManPosSet(effectP->parManId[0], pos->x, pos->y, pos->z);
    Hu3DParManTimeLimitSet(effectP->parManId[0], 2);
    Hu3DParManAttrReset(effectP->parManId[0], HU3D_PARMAN_ATTR_TIMEUP);
    Hu3DParManPosSet(effectP->parManId[1], pos->x, pos->y, pos->z);
    Hu3DParManTimeLimitSet(effectP->parManId[1], 2);
    Hu3DParManAttrReset(effectP->parManId[1], HU3D_PARMAN_ATTR_TIMEUP);
    Hu3DParManPosSet(effectP->parManId[2], pos->x, pos->y, pos->z);
    Hu3DParManTimeLimitSet(effectP->parManId[2], 5);
    Hu3DParManAttrReset(effectP->parManId[2], HU3D_PARMAN_ATTR_TIMEUP);
}

static void ParticleDotKill(MDSEL_EFFECT *effectP)
{
    Hu3DParManKill(effectP->parManId[0]);
    Hu3DParManKill(effectP->parManId[1]);
    Hu3DParManKill(effectP->parManId[2]);
}

static MDSEL_EFFECT *ParticleGlowCreate(ANIMDATA *animP, s16 layer, s16 cameraBit)
{
    static HU3DPARMANPARAM param = {
        50,
        0,
        2,
        30,
        180,
        { 0, -0.2f, 0 },
        1,
        0.8f,
        30,
        0.95f,
        3,
        {
            { 128, 192, 255, 255 },
            { 255, 255, 64, 255 },
            { 255, 96, 128, 255 },
        },
        {
            { 128, 192, 255, 0 },
            { 255, 64, 32, 0 },
            { 128, 16, 16, 0 },
        }
    };
    HU3DPARMANID parManId;
    MDSEL_EFFECT *effectP;
    effectP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDSEL_EFFECT), HU_MEMNUM_OVL);
    parManId = Hu3DParManCreate(animP, 100, &param);
    effectP->parManId[0] = parManId;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_SCALEJITTER|HU3D_PARMAN_ATTR_RANDSCALE70|HU3D_PARMAN_ATTR_RANDSPEED70|HU3D_PARMAN_ATTR_TIMEUP);
    Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(parManId), HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DParManRotSet(parManId, 90, 0, 0);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(parManId), layer);
    Hu3DModelCameraSet(Hu3DParManModelIDGet(parManId), cameraBit);
    return effectP;
}

static void ParticleGlowPosSet(MDSEL_EFFECT *effectP, HuVecF *pos)
{
    Hu3DParManPosSet(effectP->parManId[0], pos->x, pos->y, pos->z);
    Hu3DParManTimeLimitSet(effectP->parManId[0], 2);
    Hu3DParManAttrReset(effectP->parManId[0], HU3D_PARMAN_ATTR_TIMEUP);
}

static void ParticleGlowKill(MDSEL_EFFECT *effectP)
{
    Hu3DParManKill(effectP->parManId[0]);
}

#endif