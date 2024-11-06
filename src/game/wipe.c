#include "game/wipe.h"
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/disp.h"
#include "datanum/win.h"

typedef BOOL (*FADEFUNC)(void);

WIPEWORK wipeData;
BOOL wipeFadeInF;

static BOOL WipeNormalFade(void);
static BOOL WipeCrossFade(void);
static BOOL WipeDissolve(void);
static BOOL WipeViewShift(void);
static BOOL WipeStar(void);
static BOOL WipeWave(void);
static BOOL WipeKoopa(void);

static FADEFUNC fadeInFunc[WIPE_TYPE_MAX] = {
    WipeNormalFade,
    WipeCrossFade,
    WipeDissolve,
    WipeDissolve,
    WipeViewShift,
    WipeNormalFade,
    WipeNormalFade,
    WipeStar,
    WipeWave,
    WipeKoopa
};

static FADEFUNC fadeOutFunc[WIPE_TYPE_MAX] = {
    WipeNormalFade,
    WipeCrossFade,
    WipeDissolve,
    WipeDissolve,
    WipeViewShift,
    WipeNormalFade,
    WipeNormalFade,
    WipeStar,
    WipeWave,
    WipeKoopa
};

void WipeInit(GXRenderModeObj *rmode)
{
    wipeData.color.r = wipeData.color.g = wipeData.color.b = 0;
    wipeData.color.a = 255;
    wipeData.type = WIPE_TYPE_NORMAL;
    wipeData.mode = WIPE_MODE_END;
    wipeData.maxTime = wipeData.time = 100;
    wipeData.image[0] = NULL;
}

void WipeExecAlways(void)
{
    switch(wipeData.mode) {
        case WIPE_MODE_IN:
            wipeData.time++;
            if(!fadeInFunc[wipeData.type & WIPE_TYPE_MASK]()) {
                wipeData.mode = WIPE_MODE_DUMMY;
                WipeColorSet(0, 0, 0);
            }
            break;
        
        case WIPE_MODE_OUT:
            wipeData.time++;
            if(!fadeInFunc[wipeData.type & WIPE_TYPE_MASK]()) {
                wipeData.mode = WIPE_MODE_END;
            }
            break;
        
        case WIPE_MODE_END:
            fadeOutFunc[wipeData.type & WIPE_TYPE_MASK]();
            break;
    }
}

void WipeCreate(s16 mode, s16 type, s16 maxTime)
{
    if(omSysExitReq && (wipeData.mode == WIPE_MODE_OUT || wipeData.mode == WIPE_MODE_END)) {
        return;
    }
    if(omSysExitReq && wipeData.mode == WIPE_MODE_IN && mode == WIPE_MODE_OUT) {
        wipeData.mode = WIPE_MODE_DUMMY;
    }
    if(wipeData.mode == WIPE_MODE_OUT || wipeData.mode == WIPE_MODE_IN) {
        return;
    }
    if(type == WIPE_TYPE_PREVTYPE) {
        type = wipeData.type;
    }
    if(type == WIPE_TYPE_WHITEFADE) {
        WipeColorSet(255, 255, 255);
        type = WIPE_TYPE_NORMAL;
    }
    if(mode == WIPE_MODE_IN) {
        wipeFadeInF = TRUE;
        if(!(type & WIPE_TYPE_FBKEEP)) {
            if(wipeData.image[0]) {
                HuMemDirectFree(wipeData.image[0]);
            }
            wipeData.image[0] = NULL;
            if(wipeData.image[1]) {
                HuMemDirectFree(wipeData.image[1]);
            }
            wipeData.image[1] = NULL;
        }
    } else if(mode == WIPE_MODE_OUT) {
        
    }
    wipeData.maxTime = maxTime;
    wipeData.type = type;
    wipeData.mode = mode;
    wipeData.time = 0;
}

void WipeColorSet(u8 r, u8 g, u8 b)
{
    wipeData.color.r = r;
    wipeData.color.g = g;
    wipeData.color.b = b;
}

u8 WipeTypeGet(void)
{
    return wipeData.type;
}

u8 WipeCheckEnd(void)
{
    if(wipeData.mode == WIPE_MODE_END || wipeData.mode == WIPE_MODE_DUMMY) {
        return FALSE;
    } else {
        return TRUE;
    }
}

u8 WipeCheck(void)
{
    if(wipeData.mode == WIPE_MODE_DUMMY) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void WipeGXInit(void);

static BOOL WipeNormalFade(void)
{
    GXColor color = wipeData.color;
    color.a = 255*(wipeData.time/wipeData.maxTime);
    if(wipeData.mode == WIPE_MODE_IN) {
        color.a = 255-color.a;
    }
    WipeGXInit();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXEnd();
    if(wipeData.mode == WIPE_MODE_END) {
        return TRUE;
    }
    if(wipeData.maxTime <= wipeData.time) {
        wipeData.time = wipeData.maxTime;
        return FALSE;
    } else {
        return TRUE;
    }
}

static BOOL WipeCrossFade(void)
{
    GXColor color;
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, GXGetTexBufferSize(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_FALSE, 0));
            DCFlushRange(wipeData.image[0], GXGetTexBufferSize(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_FALSE, 0));
        }
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        wipeData.time = wipeData.maxTime;
        return FALSE;
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(!wipeData.image[0]) {
            return WipeNormalFade();
        } else {
            if(wipeData.maxTime <= wipeData.time) {
                wipeData.time = wipeData.maxTime;
                if(wipeData.image[0]) {
                    HuMemDirectFree(wipeData.image[0]);
                }
                wipeData.image[0] = NULL;
                return FALSE;
            }
        }
    }
    color.a = 255*(wipeData.time/wipeData.maxTime);
    if(wipeData.mode == WIPE_MODE_IN) {
        color.a = 255-color.a;
    }
    WipeGXInit();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE, GX_TEXMAP0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(0, 0);
    GXTexCoord2f32(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXTexCoord2f32(1, 0);
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXTexCoord2f32(1, 1);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXTexCoord2f32(0, 1);
    GXEnd();
    return TRUE;
}

static BOOL WipeDissolve(void)
{
    if(wipeData.mode == WIPE_MODE_IN) {
        if(wipeData.image[0]) {
            HuMemDirectFree(wipeData.image[0]);
        }
        wipeData.image[0] = NULL;
        return WipeNormalFade();
    }
    if(wipeData.mode == WIPE_MODE_END) {
        WipeNormalFade();
        return TRUE;
    } else {
        GXColor color;
        Mtx trans;
        Mtx rot;
        Mtx modelview;
        if(!wipeData.image[0]) {
            wipeData.image[0] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, GXGetTexBufferSize(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_FALSE, 0));
        }
        if(wipeData.time == 1.0) {
            Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        }
        
        color.r = color.g = color.b = 255*(wipeData.time/wipeData.maxTime);
        color.a = 224;
        WipeGXInit();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);
        Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE, GX_TEXMAP0);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColor(GX_COLOR1, color);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        MTXTrans(trans, -(HU_FB_WIDTH/2), -(HU_FB_HEIGHT/2), 0);
        MTXRotDeg(rot, 'Z', wipeData.time/10.0);
        MTXConcat(rot, trans, modelview);
        mtxTransCat(modelview, (HU_FB_WIDTH/2), (HU_FB_HEIGHT/2), 0);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        if(wipeData.type == WIPE_TYPE_DISSOLVEINBLUR) {
            GXPosition2u16(0, 0);
            GXTexCoord2f32(0.03f, 0.03f);
            GXPosition2u16(HU_FB_WIDTH, 0);
            GXTexCoord2f32(0.97f, 0.03f);
            GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
            GXTexCoord2f32(0.97f, 0.97f);
            GXPosition2u16(0, HU_FB_HEIGHT);
            GXTexCoord2f32(0.03f, 0.97f);
        } else {
            GXPosition2u16(10, 10);
            GXTexCoord2f32(0, 0);
            GXPosition2u16(HU_FB_WIDTH-10, 10);
            GXTexCoord2f32(1, 0);
            GXPosition2u16(HU_FB_WIDTH-10, HU_FB_HEIGHT-10);
            GXTexCoord2f32(1, 1);
            GXPosition2u16(10, HU_FB_HEIGHT-10);
            GXTexCoord2f32(0, 1);
        }
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        WipeNormalFade();
        if(wipeData.maxTime <= wipeData.time) {
            wipeData.time = wipeData.maxTime;
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

static void TransformPoint(float fov, float vpW, float vpH, Mtx modelview, Vec *point, Vec *pointNew);

static BOOL WipeViewShift(void)
{
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, GXGetTexBufferSize(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_FALSE, 0));
        }
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
        wipeData.time = wipeData.maxTime;
        return FALSE;
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(!wipeData.image[0]) {
            return WipeNormalFade();
        } else {
            if(wipeData.maxTime <= wipeData.time) {
                wipeData.time = wipeData.maxTime;
                if(wipeData.image[0]) {
                    HuMemDirectFree(wipeData.image[0]);
                }
                wipeData.image[0] = NULL;
                return FALSE;
            }
        }
    }
    if(wipeData.mode == WIPE_MODE_END) {
        return WipeCrossFade();
    } else {
        Mtx44 proj;
        Mtx modelview;
        Vec pos;
        Vec target;
        Vec up;
        Vec point1, point2;
        Vec posMin, posMax;
        s16 i, j;
        s16 size;
        
        MTXPerspective(proj, 20.0f, (float)HU_FB_WIDTH/HU_FB_HEIGHT, 100, 3000);
        GXSetProjection(proj, GX_PERSPECTIVE);
        pos.x = pos.y = 0;
        pos.z = 1000;
        up.x = up.z = 0;
        up.y = 1;
        target.x = target.y = target.z = 0;
        MTXLookAt(modelview, &pos, &up, &target);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
        GXSetScissor(0, 0, 640, 480);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GXSetAlphaUpdate(GX_FALSE);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
        GXSetCullMode(GX_CULL_NONE);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);
        Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE, GX_TEXMAP0);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        pos.x = pos.y = 0;
        pos.z = 1000;
        TransformPoint(20.0f, HU_FB_WIDTH/6.0, HU_FB_HEIGHT/6, modelview, &pos, &point1);
        pos.x = HU_FB_WIDTH/6.0;
        pos.y = HU_FB_HEIGHT/6;
        pos.z = 1000;
        TransformPoint(20.0f, HU_FB_WIDTH/6.0, HU_FB_HEIGHT/6, modelview, &pos, &point2);
        size = (point2.x-point1.x)/2;
        for(j=5; j>=0; j--) {
            for(i=0; i<6; i++) {
                float angle = (i+(wipeData.time-(j*6)))/(wipeData.maxTime-36);
                if(angle < 0.0f) {
                    angle = 0.0f;
                }
                if(angle > 1.0) {
                    angle = 1.0f;
                }
                GXSetViewport((HU_FB_WIDTH/6.0)*i, (HU_FB_HEIGHT/6.0)*j, HU_FB_WIDTH/6.0, HU_FB_HEIGHT/6.0, 0, 1);
                posMin.x = size+((size*HuCos(270.0f*angle))+point1.x);
                posMin.y = point1.y;
                posMin.z = (size*HuSin(270.0f*angle))+point1.z;
                posMax.x = size+((size*HuCos((270.0f*angle)+180.0f))+point1.x);
                posMax.y = point2.y;
                posMax.z = (size*HuSin((270.0f*angle)+180.0f))+point1.z;
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32(posMin.x, posMin.y, posMin.z);
                GXTexCoord2f32((1/6.0)+((1/6.0)*i), (1/6.0)*j);
                GXPosition3f32(posMax.x, posMin.y, posMax.z);
                GXTexCoord2f32(((1/6.0)*i), (1/6.0)*j);
                GXPosition3f32(posMax.x, posMax.y, posMax.z);
                GXTexCoord2f32(((1/6.0)*i), (1/6.0)+((1/6.0)*j));
                GXPosition3f32(posMin.x, posMax.y, posMin.z);
                GXTexCoord2f32((1/6.0)+((1/6.0)*i), (1/6.0)+((1/6.0)*j));
            }
        }
        return TRUE;
    }
    
}

static BOOL WipeImage(int dataNum);

static BOOL WipeStar(void)
{
    return WipeImage(WIN_ANM_wipeStar);
}

static BOOL WipeKoopa(void)
{
    return WipeImage(WIN_ANM_wipeKoopa);
}

static BOOL WipeImage(int dataNum)
{
    GXColor color;
    Mtx modelview;
    float time;
    if(wipeData.mode == WIPE_MODE_END) {
        return WipeNormalFade();
    }
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] = HuSprAnimRead(HuDataRead(dataNum));
        }
        time = HuSin(90.0*(wipeData.time/wipeData.maxTime));
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(wipeData.time == 1.0) {
            if(wipeData.image[0]) {
                HuMemDirectFree(wipeData.image[0]);
            }
            wipeData.image[0] = HuSprAnimRead(HuDataRead(dataNum));
        }
        if(!wipeData.image[0]) {
            wipeData.image[0] = HuSprAnimRead(HuDataRead(dataNum));
        }
        if(wipeData.maxTime <= wipeData.time) {
            wipeData.time = wipeData.maxTime;
            if(wipeData.image[0]) {
                HuMemDirectFree(wipeData.image[0]);
            }
            wipeData.image[0] = NULL;
            return FALSE;
        } else {
            time = HuCos(90.0*(wipeData.time/wipeData.maxTime));
        }
    }
    color = wipeData.color;
    color.a = 255.0f*time;
    WipeGXInit();
    MTXTrans(modelview, -HU_FB_WIDTH, -HU_FB_HEIGHT, 0);
    mtxScaleCat(modelview, 3.0f, 3.0f, 3.0f);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    HuSprTexLoad(wipeData.image[0], 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(time*(HU_FB_WIDTH/2), time*(HU_FB_HEIGHT/2));
    GXTexCoord2f32(0, 0);
    GXPosition2u16(HU_FB_WIDTH-(time*(HU_FB_WIDTH/2)), time*(HU_FB_HEIGHT/2));
    GXTexCoord2f32(1, 0);
    GXPosition2u16(HU_FB_WIDTH-(time*(HU_FB_WIDTH/2)), HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXTexCoord2f32(1, 1);
    GXPosition2u16(time*(HU_FB_WIDTH/2), HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXTexCoord2f32(0, 1);
    GXEnd();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColor(GX_COLOR1, color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 16);
    GXPosition2u16(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXPosition2u16(HU_FB_WIDTH, time*(HU_FB_HEIGHT/2));
    GXPosition2u16(0, time*(HU_FB_HEIGHT/2));
    GXPosition2u16(0, HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXPosition2u16(0, time*(HU_FB_HEIGHT/2));
    GXPosition2u16(time*(HU_FB_WIDTH/2), time*(HU_FB_HEIGHT/2));
    GXPosition2u16(time*(HU_FB_WIDTH/2), HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXPosition2u16(0, HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH-(time*(HU_FB_WIDTH/2)), time*(HU_FB_HEIGHT/2));
    GXPosition2u16(HU_FB_WIDTH, time*(HU_FB_HEIGHT/2));
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXPosition2u16(HU_FB_WIDTH-(time*(HU_FB_WIDTH/2)), HU_FB_HEIGHT-(time*(HU_FB_HEIGHT/2)));
    GXEnd();
    if(wipeData.maxTime <= wipeData.time) {
        wipeData.time = wipeData.maxTime;
        return FALSE;
    } else {
        return TRUE;
    }
}

static s16 waveSprIdx;
static float waveTexMtx[2][3] = {
    0.02f, 0, 0,
    0, 0.02f, 0
};

static BOOL WipeWave(void)
{
    Mtx texTrans;
    Mtx texScale;
    Mtx texMtx;
    GXColor color;
    float time;
    if(wipeData.mode == WIPE_MODE_END) {
        return WipeNormalFade();
    }
    if(wipeData.mode == WIPE_MODE_OUT) {
        if(!wipeData.image[0]) {
            wipeData.image[0] = HuMemDirectMalloc(HUHEAPTYPE_HEAP, GXGetTexBufferSize(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_FALSE, 0));
            DCFlushRange(wipeData.image[0], GXGetTexBufferSize(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_FALSE, 0));
            wipeData.image[1] = HuSprAnimRead(HuDataRead(WIN_ANM_wipeWave+waveSprIdx));
            waveSprIdx++;
            if(waveSprIdx >= 9) {
                waveSprIdx = 0;
            }
        }
        time = 1.0-(wipeData.time/wipeData.maxTime);
    }
    if(wipeData.mode == WIPE_MODE_IN) {
        if(!wipeData.image[0]) {
            return WipeNormalFade();
        } else {
            if(wipeData.maxTime <= wipeData.time) {
                wipeData.time = wipeData.maxTime;
                if(wipeData.image[0]) {
                    HuMemDirectFree(wipeData.image[0]);
                }
                if(wipeData.image[1]) {
                    HuMemDirectFree(wipeData.image[1]);
                }
                wipeData.image[0] = NULL;
                wipeData.image[1] = NULL;
                return FALSE;
            } else {
                time = wipeData.time/wipeData.maxTime;
            }
        }
    }
    Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, TRUE, wipeData.image[0]);
    color.a = 255*time;
    waveTexMtx[0][0] = 1.0-time;
    waveTexMtx[1][1] = 1.0-time;
    WipeGXInit();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    Hu3DTexLoad(wipeData.image[0], HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE, GX_TEXMAP0);
    HuSprTexLoad(wipeData.image[1], 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetNumTexGens(2);
    GXSetNumTevStages(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    MTXTrans(texTrans, 0, 0, 0);
    MTXScale(texScale, 1, 1, 1);
    MTXConcat(texScale, texTrans, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTevColor(GX_COLOR1, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, waveTexMtx, -1);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(0, 0);
    GXTexCoord2f32(0, 0);
    GXPosition2u16(HU_FB_WIDTH, 0);
    GXTexCoord2f32(1, 0);
    GXPosition2u16(HU_FB_WIDTH, HU_FB_HEIGHT);
    GXTexCoord2f32(1, 1);
    GXPosition2u16(0, HU_FB_HEIGHT);
    GXTexCoord2f32(0, 1);
    GXEnd();
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTevDirect(GX_TEVSTAGE0);
    if(wipeData.maxTime <= wipeData.time) {
        wipeData.time = wipeData.maxTime;
        return FALSE;
    } else {
        return TRUE;
    }
}

static void WipeGXInit(void)
{
    Mtx44 proj;
    Mtx modelview;
    MTXOrtho(proj, 0, HU_FB_HEIGHT, 0, HU_FB_WIDTH, 0, 10);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    MTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
    GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_SPEC);
    GXSetCullMode(GX_CULL_NONE);
}

static void TransformPoint(float fov, float vpW, float vpH, Mtx modelview, Vec *point, Vec *pointNew)
{
    float angleTan = HuSin(fov/2)/HuCos(fov/2);
    float height = 2.0f*(angleTan*point->z);
    float width = height*(vpW/vpH);
    float scaleX = point->x/vpW;
    float scaleY = point->y/vpH;
    Mtx invModelview;
    pointNew->x = (scaleX-0.5)*width;
    pointNew->y = -(scaleY-0.5)*height;
    pointNew->z = -point->z;
    MTXInverse(modelview, invModelview);
    MTXMultVec(invModelview, pointNew, pointNew);
}