#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"

typedef struct M507EffectParam_s {
    u32 flags;
    GXColor colorStart;
    GXColor colorEnd;
    HuVecF vel;
    HuVecF speedDecay;
    float accelY;
    float unk28;
    float scaleDecay;
    float alphaStep;
    float colorStep;
} M507EffectParam;

s16 fn_1_7AE8(s16 arg0, float arg2, float arg3, float arg4, float arg5, M507EffectParam *arg1);
void fn_1_7D8C(void);
void fn_1_8214(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx);

M507EffectParam lbl_1_data_3E8 = {
    0,
    { 0xA0, 0xA0, 0xA0, 0xFF },
    { 0x00, 0x00, 0x00, 0xFF },
    { 0.0f, 2.0f, 1.0f },
    { 0.95f, 0.99f, 0.95f },
    0.0f,
    0.0f,
    1.0f,
    -4.0f,
    0.0f,
};

s16 lbl_1_bss_133C[16];
M507EffectParam *lbl_1_bss_1338;

void fn_1_7720(void)
{
    s32 i;

    for (i = 0; i < 16; i++) {
        lbl_1_bss_133C[i] = -1;
    }
    lbl_1_bss_1338 = NULL;
}

void fn_1_7770(s16 arg0, HuVecF *arg1)
{
    s16 i;
    float scale;

    scale = 10.0f;
    for (i = 0; i < 8; i++) {
        lbl_1_data_3E8.vel.x = scale * (4.0 * HuSin(270.0f + (22.5f * i)));
        lbl_1_data_3E8.vel.y = 5.0f;
        lbl_1_data_3E8.vel.z = scale * (4.0 * HuCos(270.0f + (22.5f * i)));
        fn_1_7AE8(arg0, arg1->x, arg1->y * scale, arg1->z, 20.0f * scale, &lbl_1_data_3E8);
    }
    for (i = 0; i < 8; i++) {
        lbl_1_data_3E8.vel.x = scale * (2.0 * HuSin(270.0 + (22.5 + (22.5f * i))));
        lbl_1_data_3E8.vel.y = 5.0f;
        lbl_1_data_3E8.vel.z = scale * (2.0 * HuCos(270.0 + (22.5 + (22.5f * i))));
        fn_1_7AE8(arg0, arg1->x, arg1->y * scale, arg1->z, 20.0f * scale, &lbl_1_data_3E8);
    }
}

s16 fn_1_7AE8(s16 arg0, float arg2, float arg3, float arg4, float arg5, M507EffectParam *arg1)
{
    s16 cameraNo;
    s16 index;
    s16 cameraBit;
    s16 result;
    HU3DMODEL *modelP;
    M507EffectParam *paramP;
    s16 modelId;
    HU3DPARTICLE *particleP;
    HU3DPARTICLEDATA *particleDataP;

    fn_1_7D8C();
    result = -1;
    cameraNo = 0;
    cameraBit = 1;
    for (; cameraNo < 16; cameraNo++, cameraBit = (s16)(cameraBit << 1)) {
        if (((cameraBit & arg0) != 0) && (Hu3DCamera[cameraNo].fov != -1.0f)) {
            modelId = lbl_1_bss_133C[cameraNo];
            if (modelId == -1) {
                return -1;
            }
            modelP = &Hu3DData[modelId];
            particleP = modelP->hookData;
            paramP = particleP->work;
            particleDataP = &particleP->data[particleP->emitCnt];
            index = particleP->emitCnt;
            for (; index < particleP->maxCnt; index++, particleDataP++) {
                if (!particleDataP->scale) {
                    break;
                }
            }
            if (index >= particleP->maxCnt) {
                particleDataP = particleP->data;
                for (index = 0; index < particleP->maxCnt; index++, particleDataP++) {
                    if (!particleDataP->scale) {
                        break;
                    }
                }
            }
            if (index != particleP->maxCnt) {
                paramP[index] = *arg1;
                particleDataP->cameraBit = arg0;
                particleDataP->pos.x = arg2;
                particleDataP->pos.y = arg3;
                particleDataP->pos.z = arg4;
                particleDataP->vel = arg1->vel;
                particleDataP->color = arg1->colorStart;
                particleDataP->scaleBase = arg5;
                particleDataP->scale = arg5;
                particleDataP->time = 0;
                particleDataP->parManId = -1;
                particleP->emitCnt = index;
                result = index;
            }
        }
    }
    return result;
}

void fn_1_7D8C(void)
{
    s16 cameraNo;
    s16 i;
    void *data;
    ANIMDATA *anim;
    HU3DPARTICLE *particleP;
    HU3DPARTICLEDATA *particleDataP;

    anim = NULL;
    for (cameraNo = 0; cameraNo < 16; cameraNo++) {
        if (Hu3DCamera[cameraNo].fov != -1.0f) {
            if (lbl_1_bss_133C[cameraNo] != -1) {
                particleP = Hu3DData[lbl_1_bss_133C[cameraNo]].hookData;
                if (particleP->anim != 0) {
                    anim = particleP->anim;
                }
            }
            else {
                if (!anim) {
                    data = HuDataSelHeapReadNum(0x410016, HU_MEMNUM_OVL, HEAP_MODEL);
                    anim = HuSprAnimRead(data);
                }
                lbl_1_bss_133C[cameraNo] = Hu3DParticleCreate(anim, 0x96);
                Hu3DModelLayerSet(lbl_1_bss_133C[cameraNo], 4);
                Hu3DParticleHookSet(lbl_1_bss_133C[cameraNo], fn_1_8214);
                Hu3DModelCameraSet(lbl_1_bss_133C[cameraNo], 1 << cameraNo);
                if (!lbl_1_bss_1338) {
                    lbl_1_bss_1338 = HuMemDirectMallocNum(HEAP_HEAP, 0x20D0, HU_MEMNUM_OVL);
                }
                Hu3DParticleBlendModeSet(lbl_1_bss_133C[cameraNo], 0);
                particleP = Hu3DData[lbl_1_bss_133C[cameraNo]].hookData;
                particleP->emitCnt = 0;
                particleP->work = lbl_1_bss_1338;
                particleP->count = 1;
                for (particleDataP = particleP->data, i = 0; i < particleP->maxCnt; i++, particleDataP++) {
                    particleDataP->scale = 0.0f;
                }
            }
        }
    }
}

void fn_1_7FF0(void)
{
    s16 cameraNo;
    void *data;
    ANIMDATA *anim;
    s16 i;
    HU3DPARTICLE *particleP;
    HU3DPARTICLEDATA *particleDataP;

    anim = NULL;
    for (cameraNo = 0; cameraNo < 16; cameraNo++) {
        if ((Hu3DCamera[cameraNo].fov != -1.0f) && (lbl_1_bss_133C[cameraNo] == -1)) {
            if (!anim) {
                data = HuDataSelHeapReadNum(0x410016, HU_MEMNUM_OVL, HEAP_MODEL);
                anim = HuSprAnimRead(data);
            }
            lbl_1_bss_133C[cameraNo] = Hu3DParticleCreate(anim, 0x96);
            Hu3DModelLayerSet(lbl_1_bss_133C[cameraNo], 4);
            Hu3DParticleHookSet(lbl_1_bss_133C[cameraNo], fn_1_8214);
            Hu3DModelCameraSet(lbl_1_bss_133C[cameraNo], 1 << cameraNo);
            if (!lbl_1_bss_1338) {
                lbl_1_bss_1338 = HuMemDirectMallocNum(HEAP_HEAP, 0x20D0, HU_MEMNUM_OVL);
            }
            Hu3DParticleBlendModeSet(lbl_1_bss_133C[cameraNo], 0);
            particleP = Hu3DData[lbl_1_bss_133C[cameraNo]].hookData;
            particleP->emitCnt = 0;
            particleP->work = lbl_1_bss_1338;
            particleP->count = 1;
            for (particleDataP = particleP->data, i = 0; i < particleP->maxCnt; i++, particleDataP++) {
                particleDataP->scale = 0.0f;
            }
        }
    }
}

void fn_1_8214(HU3DMODEL *modelP, HU3DPARTICLE *particleP, Mtx mtx)
{
    HU3DPARTICLEDATA *particleDataP;
    M507EffectParam *paramP;
    s16 i;
    s16 color;
    double scale;

    paramP = particleP->work;
    if (particleP->count == 0) {
        for (particleDataP = particleP->data, i = 0; i < particleP->maxCnt; i++, particleDataP++) {
            particleDataP->scale = 0.0f;
        }
    }
    for (particleDataP = particleP->data, i = 0; i < particleP->maxCnt; i++, particleDataP++) {
        if (particleDataP->scale) {
            particleDataP->vel.x *= paramP[i].speedDecay.x;
            particleDataP->vel.y *= paramP[i].speedDecay.y;
            particleDataP->vel.z *= paramP[i].speedDecay.z;
            VECAdd(&particleDataP->vel, &particleDataP->pos, &particleDataP->pos);
            particleDataP->vel.y += paramP[i].accelY;

            color = particleDataP->color.r + (paramP[i].colorStep * (paramP[i].colorEnd.r - paramP[i].colorStart.r));
            if (color < 0) {
                color = 0;
            }
            else if (color > 255) {
                color = 255;
            }
            particleDataP->color.r = color;

            color = particleDataP->color.g + (paramP[i].colorStep * (paramP[i].colorEnd.g - paramP[i].colorStart.g));
            if (color < 0) {
                color = 0;
            }
            else if (color > 255) {
                color = 255;
            }
            particleDataP->color.g = color;

            color = particleDataP->color.b + (paramP[i].colorStep * (paramP[i].colorEnd.b - paramP[i].colorStart.b));
            if (color < 0) {
                color = 0;
            }
            else if (color > 255) {
                color = 255;
            }
            particleDataP->color.b = color;

            color = particleDataP->color.a + paramP[i].alphaStep;
            if (color < 1) {
                particleDataP->scale = 0.0f;
            }
            particleDataP->color.a = color;

            if (particleDataP->scale) {
                if (paramP[i].flags & 1) {
                    if ((particleDataP->time + i) & 1) {
                        scale = 1.0;
                    }
                    else {
                        scale = 0.5;
                    }
                    particleDataP->scale = particleDataP->scaleBase * scale;
                }
                else {
                    particleDataP->scale = particleDataP->scaleBase;
                }
                particleDataP->scaleBase += paramP[i].scaleDecay;
                if (particleDataP->scaleBase <= 0.01f) {
                    particleDataP->scale = 0.0f;
                }
            }
            particleDataP->time++;
        }
    }
    DCStoreRangeNoSync(particleP->data, particleP->maxCnt * sizeof(HU3DPARTICLEDATA));
}
