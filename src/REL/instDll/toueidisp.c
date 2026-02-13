#include "REL/instDll.h"


#define TOUEI_FB_W 80
#define TOUEI_FB_H 80

static void *ToueiFbCopyBuf[INST_CHARMDL_MAX];
static HuVecF ToueiFbCopyPos[INST_CHARMDL_MAX*2];
static s16 ToueiNo;

static char *ToueiCenterObj[15] = {
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

static float ToueiCenterOfsY[13] = {
    15,
    18,
    12,
    20,
    15,
    6,
    -5,
    20,
    70,
    15,
    15,
    15,
    15
};

static void ToueiFBCopy(s16 layerNo);
static void ToueiZClear(void);
static void ToueiMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);


void ToueiDispInit(char *movie)
{
    s16 i;
    HuVecF pos3D;
    HuVecF pos2D;
    s16 posX, posY;
    for(i=0; i<INST_CHARMDL_MAX; i++) {
        if(CharMdlId[i] != HU3D_MODELID_NONE) {
            ToueiFbCopyBuf[i] = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(TOUEI_FB_W, TOUEI_FB_H, GX_TF_RGB565, GX_FALSE, 0), HU_MEMNUM_OVL);
            if(i < GW_PLAYER_MAX) {
                Hu3DModelObjPosGet(CharMdlId[i], ToueiCenterObj[CharNo[i]], &pos3D);
                pos3D.y += ToueiCenterOfsY[CharNo[i]];
            } else {
                pos3D = Hu3DData[CharMdlId[i]].pos;
                pos3D.y = 125;
            }
            Hu3D3Dto2D(&pos3D, HU3D_CAM0, &pos2D);
            posX = pos2D.x*(640.0/576.0);
            posY = pos2D.y;
            posX &= 0xFFFE;
            posY &= 0xFFFE;
            ToueiFbCopyPos[i].x = posX;
            ToueiFbCopyPos[i].y = posY;
            Hu3DModelMatHookSet(StageMdlId[STAGE_MODEL_TOUEI1+i], ToueiMatHook);
        }
    }
    CharMdlId[INST_CHARMDL_NPC] != HU3D_MODELID_NONE;
    Hu3DLayerHookSet(0, ToueiFBCopy);
    HuTHPSprCreateVol(movie, TRUE, 100, 0);
    Hu3DModelMatHookSet(StageMdlId[STAGE_MODEL_MGMOVIE], HuTHPMatHook);
    
}

static void ToueiFBCopy(s16 layerNo)
{
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    GXColor fogColor = {};
    if(Hu3DCameraNo == 0) {
        GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, fogColor);
    } else {
        s16 i;
        s16 num;
        GXSetFog(GX_FOG_PERSP_EXP, 2700, 4000, cameraP->near, cameraP->far, ToueiFogColor);
        
        for(i=num=0; i<INST_CHARMDL_MAX; i++) {
            if(CharMdlId[i] != HU3D_MODELID_NONE && !(Hu3DData[StageMdlId[STAGE_MODEL_TOUEI1+i]].attr &HU3D_ATTR_DISPOFF)) {
                Hu3DFbCopyExec(ToueiFbCopyPos[i].x-(TOUEI_FB_W/2), ToueiFbCopyPos[i].y-(TOUEI_FB_H/2), TOUEI_FB_W, TOUEI_FB_H, GX_TF_RGB565, GX_FALSE, ToueiFbCopyBuf[num++]);
            }
            
        }
        ToueiZClear();
        ToueiNo = 0;
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
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 0, GX_AOP_AND, GX_GEQUAL, 0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);

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
    GXInitTexObj(&texObj, ToueiFbCopyBuf[ToueiNo], TOUEI_FB_W, TOUEI_FB_H, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    
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
    ToueiNo++;
}