#include "game/sprite.h"
#include "game/init.h"
#include "game/hu3d.h"
#include "game/disp.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "dolphin/vi.h"

void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter);;

typedef struct HuSprLayer_s {
    s16 drawNo;
    s16 layer;
    s16 camera;
} HUSPRLAYER;

static void *bmpNoCC[8];
static HUSPRLAYER HuSprLayer[HU3D_LAYER_HOOK_MAX];

static s16 bmpCCIdx;

void mtxTransCat(Mtx matrix, float x, float y, float z);

static void HuSprLayerHook(s16 layer);

void HuSprDispInit(void)
{
    Mtx44 proj;
    s16 i;
    for(i=0; i<8; i++) {
        bmpNoCC[i] = NULL;
    }
    bmpCCIdx = 0;
    GXInvalidateTexAll();
    MTXOrtho(proj, 0, HU_DISP_HEIGHT, 0, HU_DISP_WIDTH, 0, 10);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    if(RenderMode->field_rendering) {
        GXSetViewportJitter(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1, VIGetNextField());
    } else {
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
    }
    GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
}

void HuSprDisp(HUSPRITE *sp)
{
    s16 i;
    ANIMDATA *anim = sp->data;
    ANIMPAT *pat = sp->patP;
    Vec axis = {0, 0, 1};
    Mtx modelview, rot;
    s16 chanSum;
    
    GXSetScissor(sp->scissorX, sp->scissorY, sp->scissorW, sp->scissorH);
    if(sp->attr & HUSPR_ATTR_FUNC) {
        if(sp->func) {
            HUSPRFUNC func = sp->func;
            func(sp);
            HuSprDispInit();
        }
        
    } else {
        ANIMLAYER *layer;
        ANIMBMP *bgBmp;
        GXColor color;
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        color.r = color.g = color.b = color.a = 255;
        GXSetChanAmbColor(GX_COLOR0A0, color);
        GXSetChanMatColor(GX_COLOR0A0, color);
        color.r = sp->r;
        color.g = sp->g;
        color.b = sp->b;
        color.a = sp->a;
        chanSum = color.r+color.g+color.b+color.a;
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_A0, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
        if(sp->attr & HUSPR_ATTR_ADDCOL) {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
        } else if(sp->attr & HUSPR_ATTR_INVCOL) {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
        } else {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        }
        if(sp->bg) {
            ANIMPAT *bgPat;
            ANIMFRAME *bgFrame;
            bgFrame = sp->bg->bank[sp->bgBank].frame;
            bgPat = &sp->bg->pat[bgFrame->pat];
            layer = bgPat->layer;
            bgBmp = &sp->bg->bmp[layer->bmpNo];
            HuSprTexLoad(sp->bg, layer->bmpNo, 1, GX_CLAMP, GX_CLAMP, GX_NEAR);
            GXSetNumIndStages(1);
            GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, bgBmp->sizeX*16, bgBmp->sizeY*16);
            GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
            GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_16, GX_ITS_16);
            GXSetTevIndTile(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 16, 16, 16, 16, GX_ITF_4, GX_ITM_0, GX_ITB_NONE, GX_ITBA_OFF);
        }
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(GX_FALSE);
        if(0 != sp->zRot) {
            MTXRotAxisDeg(rot, &axis, sp->zRot);
            MTXScale(modelview, sp->scale.x, sp->scale.y, 1.0f);
            MTXConcat(rot, modelview, modelview);
        } else {
            MTXScale(modelview, sp->scale.x, sp->scale.y, 1.0f);
        }
        mtxTransCat(modelview, sp->pos.x, sp->pos.y, 0);
        MTXConcat(*sp->groupMtx, modelview, modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        for(i=pat->layerNum-1; i>=0; i--) {
            float pos[4][2];
            float uvX1, uvY1, uvX2, uvY2;
            ANIMBMP *bmp;
            layer = &pat->layer[i];
            bmp = &anim->bmp[layer->bmpNo];
            if(!bmp) {
                continue;
            }
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
            HuSprTexLoad(anim, layer->bmpNo, 0, sp->wrapS, sp->wrapT, (sp->attr & HUSPR_ATTR_LINEAR) ? GX_LINEAR : GX_NEAR);
            if(layer->alpha != 255 || chanSum != 255*4) {
                color.a = (u16)(sp->a*layer->alpha) >> 8;
                GXSetTevColor(GX_TEVREG0, color);
                GXSetNumTevStages(2);
            } else {
                GXSetNumTevStages(1);
            }
            if(!sp->bg) {
                pos[0][0] = layer->vtx[0]-pat->centerX;
                pos[0][1] = layer->vtx[1]-pat->centerY;
                pos[1][0] = layer->vtx[2]-pat->centerX;
                pos[1][1] = layer->vtx[3]-pat->centerY;
                pos[2][0] = layer->vtx[4]-pat->centerX;
                pos[2][1] = layer->vtx[5]-pat->centerY;
                pos[3][0] = layer->vtx[6]-pat->centerX;
                pos[3][1] = layer->vtx[7]-pat->centerY;
                if(layer->flip & ANIM_LAYER_FLIPX) {
                    uvX2 = layer->startX/(float)bmp->sizeX;
                    uvX1 = (layer->startX+layer->sizeX)/(float)bmp->sizeX;
                } else {
                    uvX1 = layer->startX/(float)bmp->sizeX;
                    uvX2 = (layer->startX+layer->sizeX)/(float)bmp->sizeX;
                }
                if(layer->flip & ANIM_LAYER_FLIPY) {
                    uvY2 = layer->startY/(float)bmp->sizeY;
                    uvY1 = (layer->startY+layer->sizeY)/(float)bmp->sizeY;
                } else {
                    uvY1 = layer->startY/(float)bmp->sizeY;
                    uvY2 = (layer->startY+layer->sizeY)/(float)bmp->sizeY;
                }
            } else {
                pos[0][0] = pos[3][0] = -(bgBmp->sizeX*16)/2;
                pos[0][1] = pos[1][1] = -(bgBmp->sizeY*16)/2;
                pos[2][0] = pos[1][0] = pos[0][0]+(bgBmp->sizeX*16);
                pos[2][1] = pos[3][1] = pos[0][1]+(bgBmp->sizeY*16);
                uvX1 = uvY1 =  1.0/(bgBmp->sizeX*16);
                uvX2 = uvY2 = 1.0f;
            }
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(pos[0][0], pos[0][1], 0);
            GXTexCoord2f32(uvX1*sp->uvScaleX, uvY1*sp->uvScaleY);
            GXPosition3f32(pos[1][0], pos[1][1], 0);
            GXTexCoord2f32(uvX2*sp->uvScaleX, uvY1*sp->uvScaleY);
            GXPosition3f32(pos[2][0], pos[2][1], 0);
            GXTexCoord2f32(uvX2*sp->uvScaleX, uvY2*sp->uvScaleY);
            GXPosition3f32(pos[3][0], pos[3][1], 0);
            GXTexCoord2f32(uvX1*sp->uvScaleX, uvY2*sp->uvScaleY);
            GXEnd();
        }
        if(sp->bg) {
            GXSetNumIndStages(0);
            GXSetTevDirect(GX_TEVSTAGE0);
            GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
        }
    }
}

void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter)
{
    GXTexObj texObj;
    GXTlutObj tlutObj;
    ANIMBMP *bmp = &anim->bmp[bmpNo];
    s16 sizeX = bmp->sizeX;
    s16 sizeY = bmp->sizeY;
    switch(bmp->dataFmt & ANIM_BMP_FMTMASK) {
        case ANIM_BMP_RGBA8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_RGBA8, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_RGB565:
        case ANIM_BMP_RGB5A3:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_RGB5A3, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_C8:
            GXInitTlutObj(&tlutObj, bmp->palData, GX_TL_RGB5A3, bmp->palNum);
            GXLoadTlut(&tlutObj, texMapId);
            GXInitTexObjCI(&texObj,bmp->data, sizeX, sizeY, GX_TF_C8, wrapS, wrapT, GX_FALSE, texMapId);
            break;
            
        case ANIM_BMP_C4:
            GXInitTlutObj(&tlutObj, bmp->palData, GX_TL_RGB5A3, bmp->palNum);
            GXLoadTlut(&tlutObj, texMapId);
            GXInitTexObjCI(&texObj,bmp->data, sizeX, sizeY, GX_TF_C4, wrapS, wrapT, GX_FALSE, texMapId);
            break;
            
        case ANIM_BMP_IA8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_IA8, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_IA4:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_IA4, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_I8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_I8, wrapS, wrapT, GX_FALSE);
            break;
        
        case ANIM_BMP_I4:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_I4, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_A8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_CTF_A8, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_CMPR:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_CMPR, wrapS, wrapT, GX_FALSE);
            break;
            
        default:
            break;
    }
    GXInitTexObjLOD(&texObj, filter, filter, 0, 0, 0, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, texMapId);
}

void HuSprExecLayerInit(void)
{
    s16 i;
    for(i=0; i<HU3D_LAYER_HOOK_MAX; i++) {
        HuSprLayer[i].layer = -1;
    }
}

void HuSprExecLayerCameraSet(s16 drawNo, s16 camera, s16 layer)
{
    s16 i;
    
    for(i=0; i<HU3D_LAYER_HOOK_MAX; i++) {
        if(-1 == HuSprLayer[i].layer) {
            break;
        }
    }
    if(i == HU3D_LAYER_HOOK_MAX) {
        return;
    }
    HuSprLayer[i].layer = layer;
    HuSprLayer[i].camera = camera;
    HuSprLayer[i].drawNo = drawNo;
    Hu3DLayerHookSet(layer, HuSprLayerHook);
}

void HuSprExecLayerSet(s16 drawNo, s16 layer)
{
    s16 i;
    
    for(i=0; i<HU3D_LAYER_HOOK_MAX; i++) {
        if(-1 == HuSprLayer[i].layer) {
            break;
        }
    }
    if(i == HU3D_LAYER_HOOK_MAX) {
        return;
    }
    HuSprLayer[i].layer = layer;
    HuSprLayer[i].camera = -1;
    HuSprLayer[i].drawNo = drawNo;
    Hu3DLayerHookSet(layer, HuSprLayerHook);
}

static void HuSprLayerHook(short layer)
{
    s16 i;
    for(i=0; i<HU3D_LAYER_HOOK_MAX; i++) {
        if(layer == HuSprLayer[i].layer) {
            break;
        }
    }
    if(i == HU3D_LAYER_HOOK_MAX) {
        return;
    }
    if((Hu3DCameraBit & HuSprLayer[i].camera) == 0) {
        return;
    }
    HuSprDispInit();
    HuSprExec(HuSprLayer[i].drawNo);
}