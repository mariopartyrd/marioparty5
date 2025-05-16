#include "game/board/telop.h"
#include "game/board/pad.h"
#include "game/board/status.h"
#include "game/board/player.h"
#include "game/board/pause.h"

#include "game/sprite.h"
#include "game/esprite.h"
#include "game/wipe.h"

typedef struct TelopObjWork_s {
    unsigned killF : 1;
    unsigned mode : 3;
    s8 playerNo;
    s8 no;
    s8 unk3;
    s8 comDelay;
    s16 time;
    s16 maxTime;
    u16 bubbleNum;
} TELOPOBJ_WORK;

typedef struct TelopBubble_s {
    HuVec2F pos;
    float size;
    HuVec2F vel;
    float radius;
    float angle;
    float angleSpeed;
    int row;
    int col;
    int depth;
} TELOPBUBBLE;

static const unsigned int telopFileTbl[TELOP_MAX] = {
    BOARD_ANM_telopMario,
    BOARD_ANM_telopLuigi,
    BOARD_ANM_telopPeach,
    BOARD_ANM_telopYoshi,
    BOARD_ANM_telopWario,
    BOARD_ANM_telopDaisy,
    BOARD_ANM_telopWaluigi,
    BOARD_ANM_telopKinopio,
    BOARD_ANM_telopTeresa,
    BOARD_ANM_telopMinikoopa,
    BOARD_ANM_telopMinikoopaR,
    BOARD_ANM_telopMinikoopaG,
    BOARD_ANM_telopMinikoopaB,
    BOARD_ANM_logoMap1,
    BOARD_ANM_logoMap2,
    BOARD_ANM_logoMap3,
    BOARD_ANM_logoMap4,
    BOARD_ANM_logoMap5,
    BOARD_ANM_logoMap6,
    BOARD_ANM_logoMap7
};

static const unsigned int bubbleFileTbl[3] = {
    BOARD_ANM_effBubble,
    BOARD_ANM_effBubbleBump,
    BOARD_ANM_effBubbleMask
};

static ANIMDATA *telopAnim[4];

static OMOBJ *telopObj;
static void *telopDlBuf;
static OMOBJ *telopLast5Obj;
static OMOBJ *telopComNumObj;
static OMOBJ *tauntObj;
static unsigned int telopLast5File;
static GXColor bubbleCol;
static GXColor bubbleBlendCol;
static u32 telopDlSize;
static u32 telopRetrace;
static HuVec2F *telopBubbleVtx;
static TELOPBUBBLE *telopBubbleData;

static void TelopObjCreate(OMOBJ *obj);

void MBTelopCreate(int playerNo, int telopNo, BOOL waitF)
{
    int i;
    TELOPOBJ_WORK *work;
    telopObj = MBAddObj(262, 1, 0, TelopObjCreate);
    work = omObjGetWork(telopObj, TELOPOBJ_WORK);
    work->killF = FALSE;
    work->playerNo = playerNo;
    work->mode = 0;
    work->no = telopNo;
    if(playerNo >= 0 && GwPlayer[playerNo].comF || playerNo < 0) {
        work->comDelay = 30;
    } else {
        work->comDelay = 0;
    }
    telopAnim[0] = HuSprAnimDataRead(telopFileTbl[telopNo]);
    for(i=1; i<4; i++) {
        telopAnim[i] = HuSprAnimDataRead(bubbleFileTbl[i-1]);
    }
    if(telopNo < TELOP_CHAR_MAX) {
        MBAudFXPlay(MSM_SE_BOARD_07);
    }
    if(!waitF) {
        return;
    }
    while(telopObj) {
        HuPrcVSleep();
    }
    for(i=0; i<4; i++) {
        if(telopAnim[i]) {
            HuSprAnimKill(telopAnim[i]);
            telopAnim[i] = NULL;
        }
    }
}

void MBTelopPlayerCreate(int playerNo)
{
    MBTelopCreate(playerNo, GwPlayer[playerNo].charNo, TRUE);
}

static void TelopObjExec(OMOBJ *obj);
static void TelopDraw(HU3DMODEL *modelP, Mtx *mtx);
static void TelopDLCreate(int num);
static void TelopBubbleCreate(ANIMDATA *animP);
static void TelopBubbleUpdate(void);

static void TelopObjCreate(OMOBJ *obj)
{
    TELOPOBJ_WORK *work = omObjGetWork(obj, TELOPOBJ_WORK);
    obj->mdlId[0] = Hu3DHookFuncCreate(TelopDraw);
    Hu3DModelLayerDraw(obj->mdlId[0], 4);
    Hu3DModelCameraSet(obj->mdlId[0], HU3D_CAM0);
    work->time = 0;
    work->maxTime = (work->no >= TELOP_CHAR_MAX) ? 60 : 15;
    telopRetrace = VIGetRetraceCount();
    TelopBubbleCreate(telopAnim[0]);
    telopBubbleVtx = HuMemDirectMallocNum(HEAP_HEAP, work->bubbleNum*sizeof(HuVec2F)*4, HU_MEMNUM_OVL);
    TelopDLCreate(work->bubbleNum);
    TelopBubbleUpdate();
    DCFlushRangeNoSync(telopBubbleVtx, work->bubbleNum*sizeof(HuVec2F));
    obj->objFunc = TelopObjExec;
}

static void TelopObjExec(OMOBJ *obj)
{
    TELOPOBJ_WORK *work = omObjGetWork(obj, TELOPOBJ_WORK);
    TELOPBUBBLE *bubbleP;
    int i;
    
    unsigned int tickNum;
    unsigned int tick;
    unsigned int retrace;
    
    if(work->killF || MBKillCheck()) {
        Hu3DModelKill(obj->mdlId[0]);
        telopObj = NULL;
        MBDelObj(obj);
        HuMemDirectFree(telopBubbleData);
        HuMemDirectFree(telopBubbleVtx);
        if(telopDlBuf != NULL) {
            HuMemDirectFree(telopDlBuf);
            telopDlBuf = NULL;
        }
        for(i=0; i<4; i++) {
            if(telopAnim[i]) {
                HuSprAnimKill(telopAnim[i]);
                telopAnim[i] = NULL;
            }
        }
        return;
    }
    retrace = VIGetRetraceCount();
    tickNum = retrace-telopRetrace;
    if(tickNum >= 0x80000000) {
        tickNum = ~tickNum+1;
    }
    telopRetrace = retrace;
    if(tickNum < 2) {
        tickNum = 1;
    } else if(tickNum > 2) {
        tickNum = 2;
    }
    switch(work->mode) {
        case 0:
            if(++work->time >= work->maxTime) {
                work->mode = 1;
            }
            break;
           
        case 1:
            if(work->comDelay) {
                work->comDelay--;
                break;
            }
            if(work->playerNo < 0) {
                work->mode = 2;
                work->time = 0;
                work->maxTime = (work->no >= TELOP_CHAR_MAX) ? 60 : 30;
                if(work->no < TELOP_CHAR_MAX) {
                    MBAudFXPlay(MSM_SE_BOARD_08);
                }
            } else {
                int padNo = GwPlayer[work->playerNo].padNo;
                if((MBPadBtnDown(padNo) & PAD_BUTTON_A) || GwPlayer[work->playerNo].comF) {
                    work->mode = 2;
                    work->time = 0;
                    work->maxTime = (work->no >= TELOP_CHAR_MAX) ? 60 : 30;
                    if(work->no < TELOP_CHAR_MAX) {
                        MBAudFXPlay(MSM_SE_BOARD_08);
                    }
                }
            }
            break;
       
        case 2:
            for(tick=0; tick<tickNum; tick++) {
                for(bubbleP = telopBubbleData, i=0; i<work->bubbleNum; i++, bubbleP++) {
                    bubbleP->pos.x += (bubbleP->radius*HuSin(bubbleP->angle))+bubbleP->vel.x;
                    bubbleP->pos.y += bubbleP->vel.y;
                    bubbleP->angle += bubbleP->angleSpeed;
                }
                TelopBubbleUpdate();
                if(++work->time >= work->maxTime) {
                    work->killF = TRUE;
                    break;
                }
            }
            DCFlushRangeNoSync(telopBubbleVtx, work->bubbleNum*sizeof(HuVec2F));
            break;
            
    }
}

BOOL MBTelopCheck(void)
{
    return telopObj == NULL;
}

static void TelopDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    TELOPBUBBLE *bubbleP = telopBubbleData;
    ANIMBMP *bmp = telopAnim[0]->bmp;
    TELOPOBJ_WORK *objWork = omObjGetWork(telopObj, TELOPOBJ_WORK);
    GXColor matColor = { 255, 0, 0, 255 };
    GXColor ambColor = { 0, 255, 0, 255 };
    
    static float scaleTexMtx[2][4] = {
        1, 0, 0, 0,
        0, 1, 0, 0
    };
    
    static float indTexMtx[2][3] = {};
    
    Mtx44 proj;
    Mtx modelview;
    float time;
    float scale;
    
    MTXOrtho(proj, 0, 480, 0, 640, 0, 100);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXSetViewport(0, 0, 640, 480, 0, 1);
    GXSetScissor(0, 0, 640, 480);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, ambColor);
    GXSetChanMatColor(GX_COLOR0A0, matColor);
    switch(objWork->mode) {
        case 0:
        case 1:
            time = (float)objWork->time/objWork->maxTime;
            if(time > 1) {
                time = 1;
            }
            bubbleBlendCol.a = 255*HuSin(time*90);
            GXSetTevColor(GX_TEVREG0, bubbleBlendCol);
            MTXTrans(modelview, 320, 240, 0);
            scale = 0.2f+(0.8f*time);
            mtxScaleCat(modelview, scale, scale, scale);
            GXLoadPosMtxImm(modelview, GX_PNMTX0);
            HuSprTexLoad(telopAnim[0], 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_NEAR);
            GXSetNumTexGens(1);
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
            GXSetNumTevStages(1);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
            GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition2f32(-bmp->sizeX*0.5f, -bmp->sizeY*0.5f);
            GXTexCoord2f32(0, 0);
            GXPosition2f32(bmp->sizeX*0.5f, -bmp->sizeY*0.5f);
            GXTexCoord2f32(1, 0);
            GXPosition2f32(bmp->sizeX*0.5f, bmp->sizeY*0.5f);
            GXTexCoord2f32(1, 1);
            GXPosition2f32(-bmp->sizeX*0.5f, bmp->sizeY*0.5f);
            GXTexCoord2f32(0, 1);
            GXEnd();
            break;

            
        case 2:
            time = (float)objWork->time*2/objWork->maxTime;
            if(time < 1) {
                bubbleBlendCol.a = 255*time;
                indTexMtx[0][2] = indTexMtx[1][1] = time;
                bubbleCol.a = 255;
            } else {
                bubbleCol.a = (1-(time-1))*255;
            }
            GXSetTevColor(GX_TEVREG0, bubbleBlendCol);
            GXSetTevColor(GX_TEVREG1, bubbleCol);
            GXSetNumTexGens(2);
            scaleTexMtx[0][0] = 1.0f/bmp->sizeX;
            scaleTexMtx[1][1] = 1.0f/bmp->sizeY;
            GXLoadTexMtxImm(scaleTexMtx, GX_TEXMTX0, GX_MTX2x4);
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0);
            GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
            GXSetNumIndStages(1);
            GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP2);
            GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
            GXSetIndTexMtx(GX_ITM_0, indTexMtx, 0);
            GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, TRUE, FALSE, GX_ITM_0);
            MTXTrans(modelview, 320-(0.5*bmp->sizeX), 240-(0.5f*bmp->sizeY), 0);
            GXLoadPosMtxImm(modelview, GX_PNMTX0);
            HuSprTexLoad(telopAnim[0], 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_NEAR);
            HuSprTexLoad(telopAnim[1], 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
            HuSprTexLoad(telopAnim[2], 0, GX_TEXMAP2, GX_CLAMP, GX_CLAMP, GX_NEAR);
            HuSprTexLoad(telopAnim[3], 0, GX_TEXMAP3, GX_CLAMP, GX_CLAMP, GX_NEAR);
            GXSetNumTevStages(4);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR_NULL);
            GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
            GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
            GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
            GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_A0, GX_CC_TEXC, GX_CC_CPREV);
            GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_APREV, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
            GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
            GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
            GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_APREV, GX_CA_A1, GX_CA_ZERO);
            GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
            GXSetArray(GX_VA_POS, telopBubbleVtx, sizeof(HuVec2F));
            GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
            GXCallDisplayList(telopDlBuf, telopDlSize);
            break;
    }
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
}


static void TelopDLCreate(int bubbleNum)
{
    void *buf = HuMemDirectMallocNum(HEAP_HEAP, 16384, HU_MEMNUM_OVL);
    int vtxNum, i;
    DCInvalidateRange(buf, 16384);
    GXBeginDisplayLsit(buf, 16384);
    for(vtxNum=0, i=0; i<bubbleNum; i++, vtxNum += 4) {
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition1x16(vtxNum);
        GXTexCoord2f32(0, 0);
        GXPosition1x16(vtxNum+1);
        GXTexCoord2f32(1, 0);
        GXPosition1x16(vtxNum+2);
        GXTexCoord2f32(1, 1);
        GXPosition1x16(vtxNum+3);
        GXTexCoord2f32(0, 1);
    }
    telopDlSize = GXEndDisplayList();
    telopDlBuf = HuMemDirectMallocNum(HEAP_HEAP, telopDlSize, HU_MEMNUM_OVL);
    memcpy(telopDlBuf, buf, telopDlSize);
    DCFlushRangeNoSync(telopDlBuf, telopDlSize);
    HuMemDirectFree(buf);
}

static void TelopBubbleUpdate(void)
{
    TELOPOBJ_WORK *objWork = omObjGetWork(telopObj, TELOPOBJ_WORK);
    TELOPBUBBLE *bubbleP = telopBubbleData;
    HuVec2F *vtx = telopBubbleVtx;
    int i;
    for(i=0; i<objWork->bubbleNum; i++, bubbleP++) {
        vtx->x = bubbleP->pos.x-bubbleP->size;
        vtx->y = bubbleP->pos.y-bubbleP->size;
        vtx++;
        vtx->x = bubbleP->pos.x+bubbleP->size;
        vtx->y = bubbleP->pos.y-bubbleP->size;
        vtx++;
        vtx->x = bubbleP->pos.x+bubbleP->size;
        vtx->y = bubbleP->pos.y+bubbleP->size;
        vtx++;
        vtx->x = bubbleP->pos.x-bubbleP->size;
        vtx->y = bubbleP->pos.y+bubbleP->size;
        vtx++;
    }
}

static void TelopBubbleCreate(ANIMDATA *animP)
{
    ANIMBMP *bmp = animP->bmp;
    TELOPOBJ_WORK *objWork = omObjGetWork(telopObj, TELOPOBJ_WORK);
    
    TELOPBUBBLE *bubbleP;
    int i;
    int j;
    s16 numX;
    u8 *alphaTbl;
    s16 numY;
    u16 *ofsTbl;
    int depthNum;
    int ofs;
    
    objWork->bubbleNum = 0;
    numX = (bmp->sizeX+19)/20;
    numY = (bmp->sizeY+19)/20;
    bubbleP = HuMemDirectMallocNum(HEAP_HEAP, numX*sizeof(TELOPBUBBLE)*numY, HU_MEMNUM_OVL);
    telopBubbleData = bubbleP;
    alphaTbl = HuMemDirectMallocNum(HEAP_HEAP, numX*numY*sizeof(u8), HU_MEMNUM_OVL);
    memset(alphaTbl, 0, numX*numY*sizeof(u8));
    ofsTbl = HuMemDirectMallocNum(HEAP_HEAP, numX*numY*sizeof(u16), HU_MEMNUM_OVL);
    while(1) {
        j = 0;
        for(i=0; i<numX*numY; i++) {
            if(!alphaTbl[i]) {
                ofsTbl[j++] = i;
            }
        }
        if(j == 0) {
            break;
        }
        ofs = ofsTbl[frandmod(j)];
        bubbleP->row = ofs%numX;
        bubbleP->col = ofs/numX;
        bubbleP->pos.x = (bubbleP->row*20)+10.0f;
        bubbleP->pos.y = (bubbleP->col*20)+10.0f;
        depthNum = 0;
        for(i=1; i<4; i++) {
            for(j=-i; j<=i; j++) {
                if(bubbleP->row+j < 0) {
                    continue;
                }
                if(bubbleP->row+j >= numX) {
                    continue;
                }
                if((bubbleP->col-i >= 0 && *(bubbleP->row+(numX*(bubbleP->col-i))+j+alphaTbl))
                    || (bubbleP->col+i < numY && *(bubbleP->row+(numX*(bubbleP->col+i))+j+alphaTbl))) {
                        break;
                    }
            }
            if(j <= i) {
                break;
            }
            for(j=-i; j<=i; j++) {
                if(bubbleP->col+j < 0) {
                    continue;
                }
                if(bubbleP->col+j >= numY) {
                    continue;
                }
                if((bubbleP->row-i >= 0) && alphaTbl[(bubbleP->row+(numX*(bubbleP->col+j))-i)]
                    || (bubbleP->row+i < numX && alphaTbl[(bubbleP->row+(numX*(bubbleP->col+j))+i)])) {
                    break;
                }
            }
            if(j <= i) {
                break;
            }
            depthNum = i;
        }
        if(depthNum) {
            bubbleP->depth = frandmod(depthNum);
        } else {
            bubbleP->depth = 0;
        }
        bubbleP->size = ((0.5f*((bubbleP->depth+1)*20))/HuSin(45))+2;
        for(i=-bubbleP->depth; i<=bubbleP->depth; i++) {
            if(bubbleP->col+i >= 0 && bubbleP->col+i < numY) {
                for(j=-bubbleP->depth; j<=bubbleP->depth; j++) {
                    if(bubbleP->row+j >= 0 && bubbleP->row+j < numX) {
                        *(bubbleP->row+(numX*(bubbleP->col+i))+j+alphaTbl) = 255;
                    }
                }
            }
        }
        bubbleP->vel.x = (4*frandf())-2;
        bubbleP->vel.y = (2*-frandf())-2;
        bubbleP->radius = frandf()*4;
        bubbleP->angle = frandf()*360;
        bubbleP->angleSpeed = (frandf()*10)-5;
        bubbleP++;
        objWork->bubbleNum++;
    }
    HuMemDirectFree(ofsTbl);
    HuMemDirectFree(alphaTbl);
}

static void TelopLast5Exec(OMOBJ *obj);
static void TelopLast5PauseHook(BOOL dispF);

typedef struct TelopLast5Work_s {
    unsigned killF : 1;
    unsigned mode : 3;
    unsigned lastTurn : 1;
    u8 delay;
    s16 angle;
    HUSPRGRPID grpId;
    HUSPRID sprId[3];
} TELOPLAST5_WORK;

void MBTelopLast5Create(void)
{
    TELOPLAST5_WORK *work;
    int i;
    OMOBJ *obj;
    int turnLeft;
    int type;
    int prio;
    HUSPRITE *sp;
    static unsigned int fileTbl[3] = {
        BOARD_ANM_telopLast,
        BOARD_ANM_telopTurn,
        BOARD_ANM_telopTurnMulti
    };
    static HuVec2F posTbl[2][3] = {
        { { -80, 0 }, { 0, 0 }, { 80, 0 } },
        { { -52, 0 }, { 0, 0 }, { 52, 0 } }
    };
    turnLeft = GwSystem.turnMax-GwSystem.turnNo;
    if(turnLeft > 4 || turnLeft < 0) {
        return;
    }
    turnLeft = 4-turnLeft;
    telopLast5Obj = obj = MBAddObj(0, 0, 0, TelopLast5Exec);
    work = omObjGetWork(obj, TELOPLAST5_WORK);
    work->killF = FALSE;
    work->delay = 0;
    work->angle = 0;
    work->grpId = HuSprGrpCreate(3);
    if(GwSystem.turnMax-GwSystem.turnNo == 0) {
        work->lastTurn = TRUE;
        type = 1;
    } else {
        work->lastTurn = FALSE;
        type = 0;
    }
    for(i=0; i<3; i++) {
        
        if(i == 1) {
            prio = 1000;
        } else {
            prio = 1400;
        }
        telopLast5File = fileTbl[i];
        if(work->lastTurn && i == 2) {
            telopLast5File = BOARD_ANM_telopTurn;
        }
        MBSpriteCreate(telopLast5File, prio, NULL, &work->sprId[i]);
        HuSprGrpMemberSet(work->grpId, i, work->sprId[i]);
        HuSprAttrSet(work->grpId, i, HUSPR_ATTR_LINEAR);
        HuSprPosSet(work->grpId, i, posTbl[type][i].x, posTbl[type][i].y);
    }
    if(!work->lastTurn) {
        sp = &HuSprData[HuSprGrpData[work->grpId].sprId[1]];
        HuSprBankSet(work->grpId, 1, 0);
        sp->animNo = turnLeft;
    } else {
        HuSprDispOff(work->grpId, 1);
    }
    HuSprAttrSet(work->grpId, 1, HUSPR_ATTR_NOANIM);
    obj->trans.x = 0;
    HuSprGrpTPLvlSet(work->grpId, obj->trans.x);
    HuSprGrpPosSet(work->grpId, 288, 72);
    MBAudFXPlay(MSM_SE_BOARD_128);
    MBPauseHookPush(TelopLast5PauseHook);
}

static void TelopLast5Exec(OMOBJ *obj)
{
    TELOPLAST5_WORK *work = omObjGetWork(obj, TELOPLAST5_WORK);
    if(work->killF || MBKillCheck()) {
        HuSprGrpKill(work->grpId);
        telopLast5Obj = NULL;
        MBDelObj(obj);
        MBPauseHookPop(TelopLast5PauseHook);
        return;
    }
    if(work->delay) {
        work->delay--;
        return;
    }
    switch(work->mode) {
        case 0:
            obj->trans.x += 0.033333335f;
            if(obj->trans.x > 1) {
                obj->trans.x = 1;
                work->mode = 1;
            }
            HuSprGrpTPLvlSet(work->grpId, obj->trans.x);
            break;
       
        case 1:
            if(work->angle >= 720) {
                work->mode = 2;
                work->delay = 90;
                if(work->lastTurn) {
                    HuSprGrpScaleSet(work->grpId, 1, 1); 
                } else {
                    HuSprScaleSet(work->grpId, 1, 1, 1); 
                }
            } else {
                s16 angle = work->angle%180;
                obj->trans.y = angle;
                obj->trans.y = 0.5+HuSin(obj->trans.y);
                if(work->lastTurn) {
                    HuSprGrpScaleSet(work->grpId, obj->trans.y, obj->trans.y); 
                } else {
                    HuSprScaleSet(work->grpId, 1, obj->trans.y, obj->trans.y); 
                }
                work->angle += 9;
            }
            break;
       
        case 2:
            obj->trans.x -= 0.033333335f;
            if(obj->trans.x < 0) {
                obj->trans.x = 0;
                work->killF = TRUE;
            }
            HuSprGrpTPLvlSet(work->grpId, obj->trans.x);
            break;
    }
}

static void TelopLast5PauseHook(BOOL dispF)
{
    if(telopLast5Obj) {
        TELOPLAST5_WORK *work = omObjGetWork(telopLast5Obj, TELOPLAST5_WORK);
        int i;
        for(i=0; i<3; i++) {
            if(dispF) {
                HuSprDispOn(work->grpId, i);
            } else {
                HuSprDispOff(work->grpId, i);
            }
            if(work->lastTurn) {
                HuSprDispOff(work->grpId, 1);
            }
        }
    }
}

typedef struct TelopComNumWork_s {
    unsigned killF : 1;
    unsigned mode : 3;
    s16 time;
    s16 maxTime;
    HUSPRGRPID grpId;
    s16 sprId[3];
} TELOPCOMNUM_WORK;

static void TelopComNumExec(OMOBJ *obj);
static void TelopComNumPauseHook(BOOL dispF);


void MBTelopComNumCreate(void)
{
    TELOPCOMNUM_WORK *work;
    int i;
    OMOBJ *obj;
    int prio;
    int num;
    HUSPRITE *sp;
    
    static unsigned int fileTbl[3] = {
        BOARD_ANM_telopComNumRemain,
        BOARD_ANM_telopComNumMinikoopa,
        BOARD_ANM_telopComNum
    };
    static HuVec2F posTbl[3] = {
        { 288, 176 }, { 224, 240 }, { 448, 240 }
    };
    
    num = MBPlayerAlivePlayerGet(NULL);
    telopComNumObj = obj = MBAddObj(0, 0, 0, TelopComNumExec);
    work = omObjGetWork(obj, TELOPCOMNUM_WORK);
    work->killF = FALSE;
    work->time = 0;
    work->maxTime = 30;
    obj->trans.x = 0;
    for(i=0; i<3; i++) {
        if(i == 2) {
            prio = 1000;
        } else {
            prio = 1400;
        }
        work->sprId[i] = espEntry(fileTbl[i], prio, 0);
        espPosSet(work->sprId[i], posTbl[i].x, posTbl[i].y);
        espTPLvlSet(work->sprId[i], obj->trans.x);
    }
    sp = &HuSprData[HuSprGrpData[work->grpId].sprId[2]];
    espBankSet(work->sprId[2], 4-num);
    MBPauseHookPush(TelopComNumPauseHook);
    MBAudFXPlay(MSM_SE_BOARD_137);
    while(telopComNumObj) {
        HuPrcVSleep();
    }
}


static void TelopComNumExec(OMOBJ *obj)
{
    TELOPCOMNUM_WORK *work = omObjGetWork(obj, TELOPCOMNUM_WORK);
    int i;
    float tpLvl;
    float scale;
    if(work->killF || MBKillCheck()) {
        for(i=0; i<3; i++) {
            espKill(work->sprId[i]);
        }
        telopComNumObj = NULL;
        MBDelObj(obj);
        MBPauseHookPop(TelopComNumPauseHook);
        return;
    }
    switch(work->mode) {
        case 0:
            if(work->time > work->maxTime) {
                work->mode = 1;
                work->time = 0;
                work->maxTime = 30;
            } else {
                tpLvl = (float)(work->time++)/work->maxTime;
                for(i=0; i<2; i++) {
                    espTPLvlSet(work->sprId[i], tpLvl);
                }
            }
            break;
          
          
        case 1:
            if(work->time > work->maxTime) {
                work->mode = 2;
                work->time = 0;
                work->maxTime = 60;
            } else {
                tpLvl = (float)(work->time++)/work->maxTime;
                scale = (5*HuCos(tpLvl*90))+1;
                espScaleSet(work->sprId[2], scale, scale);
                espTPLvlSet(work->sprId[2], tpLvl);
            }
            break;
       
        case 2:
            if(work->time > work->maxTime) {
                work->mode = 3;
                work->time = 0;
                work->maxTime = 30;
            }
            tpLvl = (float)(work->time++)/work->maxTime;
            scale = fabs(HuSin(tpLvl*540))+1;
            espScaleSet(work->sprId[2], scale, scale);
            break;
       
        case 3:
            if(work->time > work->maxTime) {
                work->killF = TRUE;
            } else {
                tpLvl = (float)(work->time++)/work->maxTime;
                for(i=0; i<3; i++) {
                    espTPLvlSet(work->sprId[i], 1-tpLvl);
                }
            }
            break;
    }
}

static void TelopComNumPauseHook(BOOL dispF)
{
    if(telopComNumObj) {
        TELOPCOMNUM_WORK *work = omObjGetWork(telopComNumObj, TELOPCOMNUM_WORK);
        int i;
        for(i=0; i<3; i++) {
            if(dispF) {
                espDispOn(work->sprId[i]);
            } else {
                espDispOff(work->sprId[i]);
            }
        }
    }
}

s8 MBPadStkXGet(int playerNo)
{
    s8 stkX = HuPadStkX[playerNo];
    s8 subStkX = HuPadSubStkX[playerNo];
    if(abs(stkX) > abs(subStkX)) {
        if(abs(stkX) < 8) {
            return 0;
        } else {
            return stkX;
        }
    } else {
        if(abs(subStkX) < 8) {
            return 0;
        } else {
            return subStkX;
        }
    }
}

s8 MBPadStkYGet(int playerNo)
{
    s8 stkY = HuPadStkX[playerNo];
    s8 subStkY = HuPadSubStkX[playerNo];
    if(abs(stkY) > abs(subStkY)) {
        if(abs(stkY) < 8) {
            return 0;
        } else {
            return stkY;
        }
    } else {
        if(abs(subStkY) < 8) {
            return 0;
        } else {
            return subStkY;
        }
    }
}

static int tauntSeNo[GW_PLAYER_MAX] = { MSM_SENO_NONE, MSM_SENO_NONE, MSM_SENO_NONE, MSM_SENO_NONE };

static void TauntExec(OMOBJ *obj);

typedef struct TauntWork_s {
    unsigned killF : 1;
} TAUNTWORK;

void MBTauntInit(void)
{
    int i;
    tauntObj = MBAddObj(32256, 0, 0, TauntExec);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        tauntSeNo[i] = MSM_SENO_NONE;
    }
    _SetFlag(FLAG_BOARD_WALKDONE);
}

void MBTauntKill(void)
{
    if(tauntObj) {
        TAUNTWORK *work = omObjGetWork(tauntObj, TAUNTWORK);
        work->killF = TRUE;
        _SetFlag(FLAG_BOARD_WALKDONE);
    }
}

static const int tauntSeIdTbl[CHARNO_MAX] = {
    MSM_SE_VOICE_MARIO+13,
    MSM_SE_VOICE_LUIGI+13,
    MSM_SE_VOICE_PEACH+13,
    MSM_SE_VOICE_YOSHI+13,
    MSM_SE_VOICE_WARIO+13,
    MSM_SE_VOICE_DAISY+13,
    MSM_SE_VOICE_WALUIGI+13,
    MSM_SE_VOICE_KINOPIO+13,
    MSM_SE_VOICE_TERESA+13,
    MSM_SE_VOICE_MINIKOOPAR+13,
    MSM_SE_VOICE_MINIKOOPA+13,
    MSM_SE_VOICE_MINIKOOPAG+13,
    MSM_SE_VOICE_MINIKOOPAB+13
};

static void TauntExec(OMOBJ *obj)
{
    int padNo;
    int charNo;
    TAUNTWORK *work;
    int i;
    work = omObjGetWork(obj, TAUNTWORK);
    if(work->killF || MBKillCheck()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(tauntSeNo[i] >= 0) {
                MBAudFXStop(tauntSeNo[i]);
                tauntSeNo[i] = MSM_SENO_NONE;
            }
        }
        tauntObj = NULL;
        MBDelObj(obj);
        return;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(tauntSeNo[i] >= 0 && HuAudFXStatusGet(tauntSeNo[i]) == MSM_SE_DONE) {
            tauntSeNo[i] = MSM_SENO_NONE;
        }
    }
    if(MBPauseProcCheck() || _CheckFlag(FLAG_BOARD_WALKDONE) || WipeCheckEnd() || GwSystem.turnPlayerNo == -1 || GWPartyFGet() == FALSE || _CheckFlag(FLAG_BOARD_TUTORIAL) || _CheckFlag(FLAG_MG_CIRCUIT)) {
        return;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        
        if(i == GwSystem.turnPlayerNo) {
            continue;
        }
        if(GwPlayer[i].comF) {
            continue;
        }
        padNo = GwPlayer[i].padNo;
        charNo = GwPlayer[i].charNo;
        if(tauntSeNo[padNo] < 0 && (HuPadBtnDown[padNo] & PAD_TRIGGER_L)) {
            tauntSeNo[padNo] = MBAudFXPlay(tauntSeIdTbl[charNo]);
        }
    }
}