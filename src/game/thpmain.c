#include "game/thpmain.h"
#include "game/hu3d.h"
#include "game/init.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/THPSimple.h"

#include "string.h"

static void THPTestProc(void);
static void THPViewFunc(HU3DMODEL *arg0, Mtx *arg1);
static void THPViewSprFunc(HUSPRITE *arg0);
static void THPDecodeFunc(void *param);

static char THPFileName[64];

static UnkThpStruct04 videoInfo;
static s32 decodeRate;
static u8 *decodeStackP;
static OSThread *decodeThread;
static s16 THPLoopF;
static s16 THPStat;
static s32 THPFrame;
static s16 THPStart;
HUPROCESS *THPProc;

HUSPRID HuTHPSprCreateVol(char *path, s16 loop, s16 prio, float volume)
{
    HUSPRID sprId;

    if(THPProc) {
        return HUSPR_NONE;
    }
    THPStart = 0;
    THPProc = HuPrcCreate(THPTestProc, 0x64, 0x3000, 0);
    if(!THPProc) {
        return HUSPR_NONE;
    }
    sprId = HuSprFuncCreate(THPViewSprFunc, prio);
    if(sprId == HUSPR_NONE) {
        return HUSPR_NONE;
    }
    THPSimpleInit(2);
    SimpleControl.unk19C = 0;
    SimpleControl.unk19E = sprId;
    SimpleControl.unkBC = volume;
    strcpy(THPFileName, path);
    THPLoopF = loop;
    THPStat = 0;
    THPFrame = 0;
    return sprId;
}

HUSPRID HuTHPSprCreate(char *path, s16 loop, s16 prio)
{
    return HuTHPSprCreateVol(path, loop, prio, 110.0f);
}

HU3DMODELID HuTHP3DCreateVol(char *path, s16 loop, float volume)
{
    HU3DMODELID modelId;

    if(THPProc) {
        return HU3D_MODELID_NONE;
    }
    THPStart = 0;
    THPProc = HuPrcCreate(THPTestProc, 0x64, 0x3000, 0);
    if(!THPProc) {
        return HU3D_MODELID_NONE;
    }
    THPSimpleInit(2);
    SimpleControl.unk19C = 1;
    modelId = Hu3DHookFuncCreate(THPViewFunc);
    SimpleControl.unk19E = modelId;
    SimpleControl.unkBC = volume;
    strcpy(THPFileName, path);
    THPLoopF = loop;
    THPStat = 0;
    THPFrame = 0;
    Hu3DModelAttrSet(SimpleControl.unk19E, 1);
    return modelId;
}

HU3DMODELID HuTHP3DCreate(char *path, s16 loop) {
    return HuTHP3DCreateVol(path, loop, 110.0f);
}

void HuTHPStop(void) {
    THPStat = 1;
}

void HuTHPClose(void) {
    THPStat = 2;
}

void HuTHPRestart(void) {
    THPStat = 3;
}

BOOL HuTHPEndCheck(void) {
    s32 temp_r31;
    
    if(!THPProc) {
        return TRUE;
    }
    if(!THPStart) {
        return FALSE;
    }
    temp_r31 = THPSimpleGetTotalFrame() - 1;
    if(temp_r31 <= 0) {
        return FALSE;
    }
    return (temp_r31 <= THPFrame);
}

s32 HuTHPFrameGet(void) {
    return THPFrame;
}

s32 HuTHPTotalFrameGet(void) {
    return THPSimpleGetTotalFrame();
}

void HuTHPSetVolume(s32 left, s32 right) {
    THPSimpleSetVolume(left, right);
}

BOOL HuTHPProcCheck(void)
{
    if(THPProc) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL HuTHPStartCheck(void)
{
    return THPStart;
}

static void THPTestProc(void) {
    s32 temp_r29;
    u32 var_r28;
    void *temp_r30;

    while(THPSimpleOpen(THPFileName) == 0) {
        OSReport("THPSimpleOpen fail");
        HuPrcVSleep();
    }
    THPSimpleGetVideoInfo(&videoInfo);
    temp_r29 = THPSimpleCalcNeedMemory();
    OSReport("Size %x\n", temp_r29);
    temp_r30 = HuMemDirectMalloc(HEAP_MODEL, temp_r29);
    memset(temp_r30, 0, temp_r29);
    DCFlushRange(temp_r30, temp_r29);
    if(temp_r30 == 0) {
        OSReport("Can't allocate the memory\n");
    }
    THPSimpleSetBuffer(temp_r30);
    while(THPSimplePreLoad(THPLoopF) == 0) {
        OSReport("THPSimplePreLoad fail");
        HuPrcVSleep();
    }
    HuPrcVSleep();
    if(VIGetNextField() == 1) {
        HuPrcVSleep();
    }
    decodeStackP = HuMemDirectMalloc(HEAP_HEAP, 0x2000);
    decodeThread = OSSetIdleFunction(THPDecodeFunc, NULL, decodeStackP + 0x2000, 0x2000);
    decodeRate = 0;
    THPStart = 1;
    if(SimpleControl.unk19C == 1) {
        Hu3DModelAttrReset(SimpleControl.unk19E, 1);
    } else {
        HUSPRITE *sp = &HuSprData[SimpleControl.unk19E];
        sp->attr &= ~HUSPR_ATTR_DISPOFF;
    }
    while(1) {
        if(THPStat == 2) {
            break;
        }
        switch (THPStat) {
            case 0:
                var_r28 = 1;
                THPStat = 4;
                break;
            case 1:
                THPSimpleAudioStop();
                THPSimpleLoadStop();
                break;
            case 3:
                THPSimpleAudioStop();
                THPSimpleLoadStop();
                while(THPSimplePreLoad(THPLoopF) == 0) {
                    OSReport("THPSimplePreLoad fail");
                    HuPrcVSleep();
                }
                THPStat = 0;
                var_r28 = 1;
                break;
        }
        if(var_r28 != 0) {
            THPSimpleAudioStart();
            var_r28 = 0;
        }
        HuPrcVSleep();
    }
    if(SimpleControl.unk19C == 1) {
        Hu3DModelKill(SimpleControl.unk19E);
    } else {
        HuSprKill(SimpleControl.unk19E);
    }
    OSCancelThread(decodeThread);
    HuMemDirectFree(decodeStackP);
    THPSimpleAudioStop();
    THPSimpleLoadStop();
    THPSimpleClose();
    HuMemDirectFree(temp_r30);
    THPSimpleQuit();
    THPProc = NULL;
    HuPrcKill(HuPrcCurrentGet());
    while(1) {
        HuPrcVSleep();
    }
}

static void THPViewFunc(HU3DMODEL *arg0, Mtx *arg1) {
    GXColor spC = { 0xFF, 0xFF, 0xFF, 0xFF };

    if(THPStart != 0) {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GXSetCullMode(GX_CULL_NONE);
        THPFrame = THPSimpleDrawCurrentFrame(RenderMode, &spC, *arg1, -videoInfo.unk00 / 2, videoInfo.unk04 / 2, videoInfo.unk00, -videoInfo.unk04);
        THPSimpleAudioStart();
    }
}

static void THPViewSprFunc(HUSPRITE *arg0) {
    Vec spC = { 0.0f, 0.0f, 1.0f };
    GXColor sp8;
    s32 temp_r30;
    s32 temp_r29;
    Mtx sp48;
    Mtx sp18;

    if(THPStart != 0) {
        sp8.r = arg0->r;
        sp8.g = arg0->g;
        sp8.b = arg0->b;
        sp8.a = arg0->a;
        if(arg0->zRot != 0.0f) {
            PSMTXRotAxisRad(sp18, &spC, MTXDegToRad(arg0->zRot));
            PSMTXScale(sp48, arg0->scale.x, arg0->scale.y, 1.0f);
            PSMTXConcat(sp18, sp48, sp48);
        } else {
            PSMTXScale(sp48, arg0->scale.x, arg0->scale.y, 1.0f);
        }
        mtxTransCat(sp48, arg0->pos.x, arg0->pos.y, 0.0f);
        PSMTXConcat(*arg0->groupMtx, sp48, sp48);
        temp_r30 = -((s32) videoInfo.unk00 / 2);
        temp_r29 = -((s32) videoInfo.unk04 / 2);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        THPFrame = THPSimpleDrawCurrentFrame(RenderMode, &sp8, sp48, temp_r30, temp_r29, videoInfo.unk00, videoInfo.unk04);
    }
}

static void THPDecodeFunc(void *param)
{
    while(1) {
        if(THPStat == 2) {
            break;
        }
        if(THPSimpleDecode() == 1) {
            OSReport("Fail to decode video data");
        }
        VIWaitForRetrace();
    }
}

void HuTHPMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    if(!THPStart || SimpleControl.unk144[0].unk0C < 0) {
        if(material->attrNum == 0) {
            Hu3DTevStageNoTexSet(drawObj, material);
        } else {
            Hu3DTevStageTexSet(drawObj, material);
        }
        return;
    } else {
        u16 thpW = SimpleControl.unk80.unk00;
        u16 thpH = SimpleControl.unk80.unk04;
        GXTexObj texY;
        GXTexObj texCb;
        GXTexObj texCr;
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaUpdate(GX_FALSE);
        GXSetNumTexGens(2);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(4);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_APREV, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
        GXSetTevColorS10(GX_TEVREG0, (GXColorS10) { 0xFFA6, 0, 0xFF8E, 0x0087 });
        GXSetTevKColor(GX_KCOLOR0, (GXColor) { 0, 0, 0xE2, 0x58 });
        GXSetTevKColor(GX_KCOLOR1, (GXColor) { 0xB3, 0, 0, 0xB6 });
        GXSetTevKColor(GX_KCOLOR2, (GXColor) { 0xFF, 0, 0xFF, 0x80 });
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
        GXInitTexObj(&texY, SimpleControl.unk144[bufIdx].unk00, (s32)thpW, (s32)thpH, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texY, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texY, GX_TEXMAP0);
        GXInitTexObj(&texCb, SimpleControl.unk144[bufIdx].unk04, thpW >> 1, thpH >> 1, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texCb, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texCb, GX_TEXMAP1);
        GXInitTexObj(&texCr, SimpleControl.unk144[bufIdx].unk08, thpW >> 1, thpH >> 1, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texCr, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texCr, GX_TEXMAP2);
        THPFrame = SimpleControl.unk144[0].unk0C;
    }
}