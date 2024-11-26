#include "game/board/effect.h"
#include "game/board/main.h"
#include "game/board/model.h"
#include "game/frand.h"
#include "game/main.h"
#include "game/init.h"

#include "game/disp.h"

static OMOBJ *confettiEffObj;
static OMOBJ *fadeEffObj;

typedef struct fadeEffWork_s {
    u8 killF : 1;
    u8 pauseF : 1;
    u8 alpha;
    s16 time;
    s16 maxTime;
    HU3DMODELID modelId;
    GXColor color;
    float speed;
} FADEEFFWORK;

static void FadeExec(OMOBJ *obj);
static void FadeDraw(HU3DMODEL *modelP, Mtx *mtx);

void MBEffFadeOutSet(s16 maxTime)
{
    FADEEFFWORK *work;
    float maxTimeF;
    if(!fadeEffObj) {
        return;
    }
    if(maxTime <= 0) {
        maxTime = 1;
    }
    work = omObjGetWork(fadeEffObj, FADEEFFWORK);
    work->maxTime = maxTime;
    OSs16tof32(&maxTime, &maxTimeF);
    work->speed = -work->color.a/maxTimeF;
    work->pauseF = FALSE;
    work->time = work->maxTime;
}

void MBEffFadeCreate(s16 maxTime, u8 alpha)
{
    FADEEFFWORK *work;
    if(fadeEffObj) {
        work = omObjGetWork(fadeEffObj, FADEEFFWORK);
        work->killF = TRUE;
        while(fadeEffObj) {
            HuPrcVSleep();
        }
    }
    fadeEffObj = MBAddObj(32000, 0, 0, FadeExec);
    omSetStatBit(fadeEffObj, OM_STAT_NOPAUSE|OM_STAT_SPRPAUSE);
    if(maxTime <= 0) {
        maxTime = 1;
    }
    work = omObjGetWork(fadeEffObj, FADEEFFWORK);
    work->killF = FALSE;
    work->pauseF = FALSE;
    work->color.r = 0;
    work->color.g = 0;
    work->color.b = 0;
    work->color.a = 0;
    work->alpha = alpha;
    work->speed = (float)(alpha-work->color.a)/maxTime;
    work->time = maxTime;
    work->maxTime = maxTime;
    work->modelId = Hu3DHookFuncCreate(FadeDraw);
    Hu3DModelCameraSet(work->modelId, HU3D_CAM0);
    Hu3DModelLayerSet(work->modelId, 5);
}

BOOL MBEffFadeDoneCheck(void)
{
    FADEEFFWORK *work;
    if(!fadeEffObj) {
        return TRUE;
    }
    work = omObjGetWork(fadeEffObj, FADEEFFWORK);
    return (work->pauseF) ? TRUE : FALSE;
}

void MBEffFadeCameraSet(u16 bit)
{
    if(fadeEffObj) {
        FADEEFFWORK *work = omObjGetWork(fadeEffObj, FADEEFFWORK);
        Hu3DModelCameraSet(work->modelId, bit);
    }
}

BOOL MBEffFadeCheck(void)
{
    return (fadeEffObj != NULL) ? FALSE : TRUE;
}

static void FadeExec(OMOBJ *obj)
{
    FADEEFFWORK *work = omObjGetWork(obj, FADEEFFWORK);
    float alpha;
    if(work->killF || MBKillCheck()) {
        if(work->modelId != HU3D_MODELID_NONE) {
            Hu3DModelKill(work->modelId);
        }
        fadeEffObj = NULL;
        MBDelObj(obj);
        return;
    }
    if(work->pauseF) {
        return;
    }
    OSu8tof32(&work->color.a, &alpha);
    alpha += work->speed;
    OSf32tou8(&alpha, &work->color.a);
    if(work->time > 0) {
        work->time--;
        return;
    }
    if(work->speed > 0) {
        work->pauseF = TRUE;
        work->color.a = work->alpha;
    } else {
        work->killF = TRUE;
    }
}

static void FadeDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    static GXColor colorN = { 0xFF, 0xFF, 0xFF, 0xFF };
	Mtx44 proj;
	Mtx modelview;
	
	float x1, x2, y1, y2;
	FADEEFFWORK *work;
	if(!fadeEffObj) {
		return;
	}
	work = omObjGetWork(fadeEffObj, FADEEFFWORK);
	x1 = 0.0f;
	x2 = HU_FB_WIDTH;
	y1 = 0.0f;
	y2 = HU_FB_HEIGHT;
	MTXOrtho(proj, y1, y2, x1, x2, 0, 10);
	GXSetProjection(proj, GX_ORTHOGRAPHIC);
	MTXIdentity(modelview);
	GXLoadPosMtxImm(modelview, GX_PNMTX0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetViewport(0, 0, x2, 1.0f+y2, 0, 1);
	GXSetScissor(0, 0, x2, 1.0f+y2);
	GXClearVtxDesc();
	GXSetChanMatColor(GX_COLOR0A0, work->color);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXSetNumTexGens(0);
	GXSetNumTevStages(1);
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U16, 0);
	GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);
	GXSetAlphaUpdate(GX_FALSE);
	GXSetColorUpdate(GX_TRUE);
	GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	GXPosition2u16(x1, y1);
	GXPosition2u16(x2, y1);
	GXPosition2u16(x2, y2);
	GXPosition2u16(x1, y2);
	GXEnd();
	GXSetChanMatColor(GX_COLOR0A0, colorN);
}

typedef struct confettiEffData_s {
    s16 time;
    u8 alpha;
    u8 ambNo;
    HuVecF pos;
    HuVecF rot;
    HuVecF vel;
    HuVecF rotVel;
} CONFETTIEFFDATA;

typedef struct confettiEffWork_s {
    u8 killF : 1;
    u8 pauseF : 1;
    s8 addNum;
    s8 time;
    s8 delay;
    s16 maxCnt;
    MBMODELID modelId;
    HU3DMODELID hookMdlId;
    CONFETTIEFFDATA *data;
} CONFETTIEFFWORK;

static void ConfettiExec(OMOBJ *obj);
static void ConfettiEffAdd(OMOBJ *obj);
static void ConfettiEffUpdate(OMOBJ *obj);
static void ConfettiDraw(HU3DMODEL *modelP, Mtx *mtx);

void MBEffConfettiCreate(HuVecF *pos, s16 maxCnt, float width)
{
    CONFETTIEFFWORK *work;
    OMOBJ *obj;
    CONFETTIEFFDATA *data;
    int i;
    if(confettiEffObj) {
        MBEffConfettiReset();
        HuPrcSleep(17);
    }
    confettiEffObj = obj = MBAddObj(257, 0, 0, ConfettiExec);
    work = omObjGetWork(obj, CONFETTIEFFWORK);
    work->killF = FALSE;
    work->pauseF = FALSE;
    work->maxCnt = maxCnt;
    work->addNum = 1;
    work->time = 0;
    work->delay = 10;
    work->hookMdlId = Hu3DHookFuncCreate(ConfettiDraw);
    Hu3DModelCameraSet(work->hookMdlId, HU3D_CAM0);
    work->data = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, work->maxCnt*sizeof(CONFETTIEFFDATA), HU_MEMNUM_OVL);
    obj->trans.x = pos->x;
    obj->trans.y = pos->y;
    obj->trans.z = pos->z;
    obj->rot.x = width;
    work->modelId = MBModelCreate(BOARD_HSF_confetti, NULL, FALSE);
    MBModelLayerSet(work->modelId, 2);
    MBModelDispSet(work->modelId, FALSE);
    for(data=work->data, i=0; i<work->maxCnt; i++, data++) {
        data->time = -1;
    }
}

void MBEffConfettiKill(void)
{
    if(confettiEffObj) {
        omObjGetWork(confettiEffObj, CONFETTIEFFWORK)->killF = TRUE;
    }
}

void MBEffConfettiReset(void)
{
    CONFETTIEFFWORK *work;
    CONFETTIEFFDATA *data;
    int i;
    if(!confettiEffObj) {
        return;
    }
    work = omObjGetWork(confettiEffObj, CONFETTIEFFWORK);
    work->pauseF = TRUE;
    for(data=work->data, i=0; i<work->maxCnt; i++, data++) {
        if(data->time != -1 && data->time > 16) {
            data->time = 16;
        }
    }
}

static void ConfettiExec(OMOBJ *obj)
{
    CONFETTIEFFWORK *work = omObjGetWork(obj, CONFETTIEFFWORK);
    if(work->killF || MBKillCheck()) {
        MBModelKill(work->modelId);
        Hu3DModelKill(work->hookMdlId);
        HuMemDirectFree(work->data);
        confettiEffObj = NULL;
        MBDelObj(obj);
        return;
    }
    ConfettiEffAdd(obj);
    ConfettiEffUpdate(obj);
}

static void ConfettiEffAdd(OMOBJ *obj)
{
    CONFETTIEFFWORK *work = omObjGetWork(obj, CONFETTIEFFWORK);
    int j, i;
    CONFETTIEFFDATA *data;
    if(work->pauseF) {
        return;
    }
    if(work->addNum < 5) {
        if(work->time++ > work->delay) {
            work->time = 0;
            work->addNum++;
        }
    }
    for(j=0; j<work->addNum; j++) {
        float angle;
        for(data=work->data, i=0; i<work->maxCnt; i++, data++) {
            if(data->time == -1) {
                break;
            }
        }
        if(i == work->maxCnt) {
            break;
        }
        data->time = frandmod(60)+120;
        angle = frandf()*360;
        data->pos.x = (obj->rot.x*HuSin(angle))+obj->trans.x;
        data->pos.y = obj->trans.y;
        data->pos.z = (obj->rot.x*HuCos(angle))+obj->trans.z;
        data->vel.x = (frandf()-0.5f)*2;
        data->vel.y = -6.533334f*frandf();
        data->vel.z = (frandf()-0.5f)*2;
        data->rotVel.x = 8.0f+((frandf()-0.5f)*20.0f);
        data->rotVel.y = 8.0f+((frandf()-0.5f)*20.0f);
        data->rotVel.z = 8.0f+((frandf()-0.5f)*20.0f);
        data->rot.x = 0;
        data->rot.y = 0;
        data->rot.z = 0;
        data->alpha = 255;
        data->ambNo = frandmod(6);
    }
}

static void ConfettiEffUpdate(OMOBJ *obj)
{
    CONFETTIEFFDATA *data;
    int i;
    CONFETTIEFFWORK *work = omObjGetWork(obj, CONFETTIEFFWORK);
    BOOL noKillF = FALSE;
    
    for(data=work->data, i=0; i<work->maxCnt; i++, data++) {
        if(data->time == -1) {
            continue;
        }
        if(data->time <= 0) {
            data->time = -1;
            continue;
        }
        data->time--;
        data->pos.x += data->vel.x;
        data->pos.y += data->vel.y;
        data->pos.z += data->vel.z;
        data->rot.x += data->rotVel.x;
        data->rot.y += data->rotVel.y;
        data->rot.z += data->rotVel.z;
        if(data->time < 16) {
            if(data->alpha >= 15) {
                data->alpha -= 15;
            } else {
                data->alpha = 0;
            }
        }
        if(!noKillF) {
            noKillF = TRUE;
        }
    }
    if(noKillF == FALSE && work->pauseF) {
        work->killF = TRUE;
    }
}

static void ConfettiDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    static HuVecF ambTbl[6] = {
        { 0.1, 0.4, 1 },
        { 0.2, 1, 0.1 },
        { 0.3, 1, 1 },
        { 1, 0.2, 0.1 },
        { 1, 0.2, 0.8 },
        { 1, 8, 0.3 }
    };
    if(!confettiEffObj || MBKillCheck()) {
        return;
    } else {
        CONFETTIEFFWORK *work = omObjGetWork(confettiEffObj, CONFETTIEFFWORK);
        HU3DMODEL *modelP = &Hu3DData[MBModelIdGet(work->modelId)];
        CONFETTIEFFDATA *data;
        int i;
        if(!modelP->hsf) {
            return;
        }
        for(data=work->data, i=0; i<work->maxCnt; i++, data++) {
            Mtx final, temp;
            float ambR, ambG, ambB, tpLvl;
            if(data->time == -1) {
                continue;
            }
            MTXRotDeg(temp, 'z', data->rot.z);
			MTXRotDeg(final, 'x', data->rot.x);
			MTXConcat(temp, final, final);
			MTXRotDeg(temp, 'y', data->rot.y);
			MTXConcat(temp, final, final);
			MTXTrans(temp, data->pos.x, data->pos.y, data->pos.z);
			MTXConcat(temp, final, final);
			MTXConcat(*mtx, final, final);
            ambR = ambTbl[data->ambNo].x;
            ambG = ambTbl[data->ambNo].y;
            ambB = ambTbl[data->ambNo].z;
            OSu8tof32(&data->alpha, &tpLvl);
            tpLvl = (1.0f/255.0f)*tpLvl;
            Hu3DModelTPLvlSet(MBModelIdGet(work->modelId), tpLvl);
            Hu3DModelAmbSet(MBModelIdGet(work->modelId), ambR, ambG, ambB);
            Hu3DModelObjDraw(MBModelIdGet(work->modelId), "grid2", final);
        }
    }
}

static void EffectDraw(HU3DMODEL *modelP, Mtx *mtx);

HU3DMODELID MBEffCreate(ANIMDATA *anim, s16 maxCnt)
{
    HU3DMODELID modelId = Hu3DHookFuncCreate(EffectDraw);
    MBEFFECTDATA *effData;
    s16 i;
    MBEFFECT *effP;
    HU3DMODEL *modelP;
    HuVecF *vertex;
    HuVec2F *st;
    void *dl;
    
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    modelP = &Hu3DData[modelId];
    modelP->hookData = effP = HuMemDirectMallocNum(HUHEAPTYPE_MODEL, sizeof(MBEFFECT), modelP->mallocNo);
    memset(effP, 0, sizeof(MBEFFECT));
    effP->anim = anim;
    anim->useNum++;
    effP->num = maxCnt;
    effP->blendMode = MB_EFFECT_BLEND_NORMAL;
    effP->prevCounter = -1;
    effP->modelId = modelId;
    effP->data = effData = HuMemDirectMallocNum(HUHEAPTYPE_MODEL, maxCnt*sizeof(MBEFFECTDATA), modelP->mallocNo);
    memset(effData, 0, maxCnt*sizeof(MBEFFECTDATA));
    for(i=0; i<maxCnt; i++, effData++) {
        effData->cameraBit = HU3D_CAM_ALL;
        effData->rot.x = effData->rot.y = effData->rot.z = 0;
        effData->pos.x = ((frand()%128)-64)*20;
        effData->pos.y = ((frand()%128)-64)*30;
        effData->pos.z = ((frand()%128)-64)*20;
        effData->color.r = effData->color.g = effData->color.b = effData->color.a = 255;
        effData->animTime = 0;
        effData->animSpeed = 0;
        effData->dispF = TRUE;
    }
    effP->vertex = vertex = HuMemDirectMallocNum(HUHEAPTYPE_MODEL, maxCnt*sizeof(HuVecF)*4, modelP->mallocNo);
    for(i=0; i<maxCnt*4; i++, vertex++) {
        vertex->x = vertex->y = vertex->z = 0;
    }
    effP->st = st = HuMemDirectMallocNum(HUHEAPTYPE_MODEL, maxCnt*sizeof(HuVec2F)*4, modelP->mallocNo);
    for(i=0; i<maxCnt*4; i++, st++) {
        st->x = st->y = 0;
    }
    effP->dl = dl = HuMemDirectMallocNum(HUHEAPTYPE_MODEL, (maxCnt*96)+128, modelP->mallocNo);
    DCInvalidateRange(dl, (maxCnt*96)+128);
    GXBeginDisplayList(dl, 0x20000);
    GXBegin(GX_QUADS, GX_VTXFMT0, maxCnt*4);
    for(i=0; i<maxCnt; i++) {
        GXPosition1x16(i*4);
        GXColor1x16(i);
        GXTexCoord1x16(i*4);
        GXPosition1x16((i*4)+1);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+1);
        GXPosition1x16((i*4)+2);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+2);
        GXPosition1x16((i*4)+3);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+3);
    }
    effP->dlSize = GXEndDisplayList();
    return modelId;
}

void MBEffKill(HU3DMODELID modelId)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    MBEFFECT *effP = modelP->hookData;
    HuMemDirectFreeNum(HUHEAPTYPE_MODEL, modelP->mallocNo);
    HuSprAnimKill(effP->anim);
    modelP->hsf = NULL;
}

static Vec basePos[] = {
    { -0.5f,  0.5f, 0.0f },
    {  0.5f,  0.5f, 0.0f },
    {  0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f }
};

static void EffectDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    MBEFFECTDATA *effData;
    HuVecF *vertex;
    MBEFFECT *effP;
    ANIMLAYER *animLayer;
    HuVec2F *st;
    ANIMFRAME *animFrame;
    ANIMBMP *animBmp;
    ANIMBANK *animBank;
    ANIMDATA *anim;
    int j, i;
    s16 bmpFmt;
    ANIMPAT *animPat;
    Mtx mtxInv;
    Mtx mtxPos;
    Mtx mtxRotZ;
    Vec scaleVtx[4];
    Vec finalVtx[4];
    Vec initVtx[4];
    ROMtx basePosMtx;
    MBEFFHOOK hook;
    BOOL dispF;
    float scale;
    effP = modelP->hookData;
    anim = effP->anim;
    if(HmfInverseMtxF3X3(*mtx, mtxInv) == FALSE) {
        PSMTXIdentity(mtxInv);
    }
    PSMTXReorder(mtxInv, basePosMtx);
    if((Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE))) {
        if(effP->hook && effP->prevCounter != GlobalCounter) {
            hook = effP->hook;
            hook(modelP, effP, *mtx);
        }
    } else if(effP->prevCounter == -1) {
        return;
    }
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, effP->vertex, sizeof(HuVecF));
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetArray(GX_VA_CLR0, &effP->data->color, sizeof(MBEFFECTDATA));
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, effP->st, 8);
    GXSetCullMode(GX_CULL_NONE);
    GXLoadPosMtxImm(*mtx, 0);
    if(shadowModelDrawF != 0) {
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetZMode(0, GX_LEQUAL, 0);
    } else {
        bmpFmt = (effP->anim->bmp->dataFmt & 0xF);
        if(bmpFmt == ANIM_BMP_I8 || bmpFmt == ANIM_BMP_I4) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_ZERO);
        } else {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        }
        if(modelP->attr & HU3D_ATTR_ZWRITE_OFF) {
            GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        } else {
            GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        }
    }
    switch(effP->blendMode) {
        case MB_EFFECT_BLEND_NORMAL:
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            break;
        case MB_EFFECT_BLEND_ADDCOL:
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
            break;
        case MB_EFFECT_BLEND_INVCOL:
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
            break;
    }
    effData = effP->data;
    vertex = effP->vertex;
    st = effP->st;
    PSMTXROMultVecArray(basePosMtx, &basePos[0], initVtx, 4);
    for(i=0; i<effP->num; i++, effData++) {
        dispF = FALSE;
        if(effData->scale && (effData->cameraBit & Hu3DCameraBit) && effData->dispF) {
            if(effData->rot.x == 0.0f && effData->rot.y == 0.0f && effData->rot.z == 0.0f) {
                scale = effData->scale;
                if(!(effP->attr & MB_EFFECT_ATTR_3D)) {
                    vertex->x = initVtx[0].x * scale + effData->pos.x;
                    vertex->y = initVtx[0].y * scale + effData->pos.y;
                    vertex->z = initVtx[0].z * scale + effData->pos.z;
                    vertex++;
                    vertex->x = initVtx[1].x * scale + effData->pos.x;
                    vertex->y = initVtx[1].y * scale + effData->pos.y;
                    vertex->z = initVtx[1].z * scale + effData->pos.z;
                    vertex++;
                    vertex->x = initVtx[2].x * scale + effData->pos.x;
                    vertex->y = initVtx[2].y * scale + effData->pos.y;
                    vertex->z = initVtx[2].z * scale + effData->pos.z;
                    vertex++;
                    vertex->x = initVtx[3].x * scale + effData->pos.x;
                    vertex->y = initVtx[3].y * scale + effData->pos.y;
                    vertex->z = initVtx[3].z * scale + effData->pos.z;
                    vertex++;
                } else {
                    vertex->x = basePos[0].x * scale + effData->pos.x;
                    vertex->y = basePos[0].y * scale + effData->pos.y;
                    vertex->z = basePos[0].z * scale + effData->pos.z;
                    vertex++;
                    vertex->x = basePos[1].x * scale + effData->pos.x;
                    vertex->y = basePos[1].y * scale + effData->pos.y;
                    vertex->z = basePos[1].z * scale + effData->pos.z;
                    vertex++;
                    vertex->x = basePos[2].x * scale + effData->pos.x;
                    vertex->y = basePos[2].y * scale + effData->pos.y;
                    vertex->z = basePos[2].z * scale + effData->pos.z;
                    vertex++;
                    vertex->x = basePos[3].x * scale + effData->pos.x;
                    vertex->y = basePos[3].y * scale + effData->pos.y;
                    vertex->z = basePos[3].z * scale + effData->pos.z;
                    vertex++;
                }
            } else {
                VECScale(&basePos[0], &scaleVtx[0], effData->scale);
                VECScale(&basePos[1], &scaleVtx[1], effData->scale);
                VECScale(&basePos[2], &scaleVtx[2], effData->scale);
                VECScale(&basePos[3], &scaleVtx[3], effData->scale);
                if(!(effP->attr & MB_EFFECT_ATTR_3D)) {
                    MTXRotDeg(mtxRotZ, 'Z', effData->rot.z);
                    PSMTXConcat(mtxInv, mtxRotZ, mtxPos);
                } else {
                    mtxRot(mtxPos, effData->rot.x, effData->rot.y, effData->rot.z);
                }
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effData->pos, vertex++);
                VECAdd(&finalVtx[1], &effData->pos, vertex++);
                VECAdd(&finalVtx[2], &effData->pos, vertex++);
                VECAdd(&finalVtx[3], &effData->pos, vertex++);
            }
            dispF = TRUE;
        } else {
            vertex->x = vertex->y = vertex->z = 0.0f;
            vertex++;
            vertex->x = vertex->y = vertex->z = 0.0f;
            vertex++;
            vertex->x = vertex->y = vertex->z = 0.0f;
            vertex++;
            vertex->x = vertex->y = vertex->z = 0.0f;
            vertex++;
        }
        animBank = &anim->bank[effData->animBank];
        animFrame = &animBank->frame[effData->animNo];
        animPat = &anim->pat[animFrame->pat];
        if((effP->attr & MB_EFFECT_ATTR_UPAUSE) && !effData->pauseF) {
            if(Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE)) {
                for(j=0; j<(s32)effData->animSpeed*minimumVcount; j++) {
                    effData->animTime += 1.0f;
                    if(effData->animTime >= animFrame->time) {
                        effData->animNo++;
                        effData->animTime -= animFrame->time;
                        animFrame = &animBank->frame[effData->animNo];
                        if(effData->animNo >= animBank->timeNum || animFrame->time == -1) {
                            if(!(effP->attr & MB_EFFECT_ATTR_LOOP)) {
                                effData->dispF = FALSE;
                                effData->pauseF = TRUE;
                            }
                            effData->animNo = 0;
                        }
                    }
                }
                effData->animTime += effData->animSpeed * minimumVcount - j;
                if(effData->animTime >= animFrame->time) {
                    effData->animNo++;
                    effData->animTime -= animFrame->time;
                    animFrame = &animBank->frame[effData->animNo];
                    if(effData->animNo >= animBank->timeNum || animFrame->time == -1) {
                        if(!(effP->attr & MB_EFFECT_ATTR_LOOP)) {
                            effData->dispF = FALSE;
                            effData->pauseF = TRUE;
                        }
                        effData->animNo = 0;
                    }
                }
            }
        }
        animLayer = animPat->layer;
        animBmp = &anim->bmp[animLayer->bmpNo];
        st->x = (float)(animLayer->startX+animLayer->vtx[0]) / animBmp->sizeX;
        st->y = (float)(animLayer->startY+animLayer->vtx[1]) / animBmp->sizeY;
        st++;
        st->x = (float)(animLayer->startX+animLayer->vtx[2]) / animBmp->sizeX;
        st->y = (float)(animLayer->startY+animLayer->vtx[3]) / animBmp->sizeY;
        st++;
        st->x = (float)(animLayer->startX+animLayer->vtx[4]) / animBmp->sizeX;
        st->y = (float)(animLayer->startY+animLayer->vtx[5]) / animBmp->sizeY;
        st++;
        st->x = (float)(animLayer->startX+animLayer->vtx[6]) / animBmp->sizeX;
        st->y = (float)(animLayer->startY+animLayer->vtx[7]) / animBmp->sizeY;
        st++;
        DCFlushRangeNoSync(&effData->color, sizeof(GXColor));
    }
    HuSprTexLoad(effP->anim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    DCFlushRangeNoSync(effP->vertex, effP->num*sizeof(HuVecF)*4);
    DCFlushRangeNoSync(effP->st, effP->num*sizeof(HuVec2F)*4);
    GXCallDisplayList(effP->dl, effP->dlSize);
    totalPolyCnt += effP->num;
    if(shadowModelDrawF == FALSE) {
        if(!(effP->attr & MB_EFFECT_ATTR_STOPCNT) && Hu3DPauseF == FALSE) {
            effP->count++;
        }
        if(effP->prevCount != 0 && effP->prevCount <= effP->count) {
            if(effP->attr & MB_EFFECT_ATTR_LOOP) {
                effP->count = 0;
            } else {
                effP->count = effP->prevCount;
            }
            
        }
        effP->prevCounter = GlobalCounter;
    }
}

void MBEffHookSet(HU3DMODELID modelId, MBEFFHOOK hook)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    MBEFFECT *effP = modelP->hookData;
    effP->hook = hook;
}

void MBEffAttrSet(HU3DMODELID modelId, u8 attr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    MBEFFECT *effP = modelP->hookData;
    effP->attr |= attr;
}

void MBEffAttrReset(HU3DMODELID modelId, u8 attr)
{
    HU3DMODEL *modelP = &Hu3DData[modelId];
    MBEFFECT *effP = modelP->hookData;
    effP->attr &= ~attr;
}

MBEFFECTDATA *MBEffDataCreate(MBEFFECT *effP)
{
    MBEFFECTDATA *effData;
    int i;
    for(effData=effP->data, i=0; i<effP->num; i++, effData++) {
        if(effData->pauseF) {
            effData->pauseF = FALSE;
            effData->dispF = TRUE;
            effData->animTime = 0;
            return effData;
        }
    }
    return NULL;
}

typedef struct unkTotalData2_s {
    u8 unk0[2];
    s16 num;
    u8 unk4[8];
} UNKTOTALDATA2;

typedef struct unkTotalData_s {
    s16 num;
    UNKTOTALDATA2 *data;
} UNKTOTALDATA;

typedef struct unkTotal_s {
    u8 unk0[8];
    UNKTOTALDATA *data;
} UNKTOTAL;

int MBEffUnkTotalGet(void *ptr, int no)
{
    int i;
    int num = 0;
    UNKTOTALDATA *data = &((UNKTOTAL *)ptr)->data[no];

    for(i=0; i<data->num; i++) {
        UNKTOTALDATA2 *data2 = &data->data[i];
        num += data2->num;
    }
    return num;
}