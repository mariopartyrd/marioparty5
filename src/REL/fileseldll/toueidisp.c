#include "REL/fileseldll.h"

#define TOUEI_FB_W 64
#define TOUEI_FB_H 64

#define TOUEI_MAX 9


static void *toueiFbCopyBuf[TOUEI_MAX];
static HuVecF toueiFbCopyPos[TOUEI_MAX];
static s16 toueiNo;

static char *toueiCenterObj[15] = {
    "root_head",
    "root_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
    "ske_head",
};

static float toueiCenterOfsY[13] = {
    15,
    18,
    12,
    20,
    15,
    6,
    -5,
    20,
    45,
    15,
    15,
    15,
    15
};

static void ToueiFBCopy(s16 layerNo);
static void ToueiZClear(void);
static void ToueiMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);


void ToueiDispInit(void)
{
    s16 i;
    HuVecF pos3D;
    HuVecF pos2D;
    s16 posX, posY;
    for(i=0; i<8; i++) {
        toueiFbCopyBuf[i] = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(TOUEI_FB_W, TOUEI_FB_H, GX_TF_RGB565, GX_FALSE, 0), HU_MEMNUM_OVL);
        if(i != 7) {
            Hu3DModelObjPosGet(charToueiMdlId[i], toueiCenterObj[i], &pos3D);
            pos3D.y += toueiCenterOfsY[i];
        } else {
            pos3D = Hu3DData[charToueiMdlId[i]].pos;
        }
        Hu3D3Dto2D(&pos3D, HU3D_CAM0, &pos2D);
        posX = pos2D.x*(640.0/576.0);
        posY = pos2D.y;
        posX &= 0xFFFE;
        posY &= 0xFFFE;
        toueiFbCopyPos[i].x = posX;
        toueiFbCopyPos[i].y = posY;
        Hu3DModelMatHookSet(toueiMdlId[i], ToueiMatHook);
    }
    toueiFbCopyBuf[i] = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(TOUEI_FB_W, TOUEI_FB_H, GX_TF_RGB565, GX_FALSE, 0), HU_MEMNUM_OVL);
    toueiFbCopyPos[i].x = posX+TOUEI_FB_W;
    toueiFbCopyPos[i].y = posY+TOUEI_FB_H;
    Hu3DLayerHookSet(0, ToueiFBCopy);
}


static void ToueiFBCopy(s16 layerNo)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    GXColor fogColor = {};
    if(Hu3DCameraNo == 0) {
        GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, fogColor);
    } else {
        s16 i;
        s16 temp;
        for(temp=i=0; i<TOUEI_MAX; i++) {
            Hu3DFbCopyExec(toueiFbCopyPos[i].x-(TOUEI_FB_W/2), toueiFbCopyPos[i].y-(TOUEI_FB_H/2), TOUEI_FB_W, TOUEI_FB_H, GX_TF_RGB565, GX_FALSE, toueiFbCopyBuf[i]);
        }
        ToueiZClear();
        toueiNo = 0;
    }
}

static void ToueiZClear(void)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    Mtx44 proj;
    Mtx modelview;
    Vec pos;
    Vec target;
    Vec up;
    GXColor color;
    float z;
    
    MTXPerspective(proj, cameraP->fov, cameraP->aspect, cameraP->near, cameraP->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    GXSetViewport(0, 0, 640, 480, 0, 1);
    GXSetScissor(0, 0, 640, 480);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    color.r = color.g = color.b = 0;
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 0, GX_AOP_AND, GX_GEQUAL, 0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);

    pos.x = pos.y = pos.z = 0;
    target.x = target.y = 0;
    target.z = -100;
    up.x = up.z = 0;
    up.y = 1;
    MTXLookAt(modelview, &pos, &up, &target);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    pos.x = (10000*-sin(cameraP->fov/2))*1.2f;
    pos.y = 10000*-sin(cameraP->fov/2)*(1/1.2f);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-pos.x, -pos.y, -10000);
    GXPosition3f32(pos.x, -pos.y, -10000);
    GXPosition3f32(pos.x, pos.y, -10000);
    GXPosition3f32(-pos.x, pos.y, -10000);
}

static void ToueiMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material)
{
    GXTexObj texObj;
    GXColor color = { 32, 32, 32, 128 };
    HU3DCAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    int i;
    for(i=0; i<SAVE_BOX_MAX; i++) {
        if(drawObj->model == &Hu3DData[fileselBox[i].toueiMdlId]) {
            break;
        }
    }
    GXInitTexObj(&texObj, toueiFbCopyBuf[fileselBox[i].charNo], TOUEI_FB_W, TOUEI_FB_H, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, GX_TEXMAP0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}