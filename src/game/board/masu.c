#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/tutorial.h"
#include "game/board/status.h"
#include "game/board/audio.h"
#include "game/board/pause.h"

#include "game/sprite.h"

#define MASU_LIGHT_MAX 64

typedef struct MasuLight_s {
    int modelId;
    HuVecF pos;
    Mtx matrix;
    int anmNo;
    int animId;
} MASULIGHT;

//Functions in headers that don't exist
void MBPathFlagInit(void);
u32 MBPathFlagGet(void);
void MBGateCreate(void);
void MBStarNumSet(int starNum);
void MBMasuStarExec(int playerNo, int masuId);
s16 MBCoinDispCreate(HuVecF *pos, int coinNum, BOOL playSe);
s32 MBCoinAddExec(s32 playerNo, int counNum);
BOOL MBCoinDispCheck(s16 no);

static u8 masuPathVisitF[MASU_MAX];
static s16 masuDonkeyTbl[MASU_DONKEY_MAX];
static s16 masuStarTbl[MASU_STAR_MAX];
static MASULIGHT masuLight[MASU_LIGHT_MAX];
static MASU masuData[MASU_LAYER_MAX][MASU_MAX];

static BOOL masuDispF;
static s16 masuNum[MASU_LAYER_MAX];
static u32 masuDispMask[MASU_LAYER_MAX];
static int masuDispCnt;
static s16 masuDonkeyNum;
static u32 masuDlBufLen;
static int masuPathCnt;
static int masuPathLen;
static int masuPathId;
static MASUPATHCHECKHOOK masuPathCheckHook;
static MASUPLAYERHOOK masuHatenaHook;
static MASUPLAYERHOOK masuWalkPostHook;
static MASUPLAYERHOOK masuWalkPreHook;
static void *masuDlBuf;
static ANIMDATA *masuCapsuleAnim;
static ANIMDATA *masuLightAnim;
static ANIMDATA *masuAnim;
static int masuLightNo;

static HU3DMODELID masuDrawMdl = HU3D_MODELID_NONE;

static const GXColor masuColor = { 255, 255, 255, 255 };


static void MasuLightHiliteHook(s16 layerNo);
static void MasuDraw(HU3DMODEL *modelP, Mtx *mtx);
static void MakeMasuDisplayList(void);

void MBMasuCreate(int dataNum)
{
    int i;
    int boardNo;
    int koopaId[MASU_MAX];
    BOOL dispF;
    int koopaNum;
    int donkeyNum;
    
    
    MBPathFlagInit();
    memset(&masuData[0][0], 0, sizeof(masuData));
    for(i=0; i<MASU_LAYER_MAX; i++) {
        masuDispMask[i] = MASU_FLAG_NONE;
    }
    masuWalkPreHook = NULL;
    masuWalkPostHook = NULL;
    masuHatenaHook = NULL;
    masuPathCheckHook = NULL;
    masuDispF = FALSE;
    boardNo = MBBoardNoGet();
    masuAnim = HuSprAnimDataRead(MBDATANUM(BOARD_ANM_masu));
    masuLightAnim = HuSprAnimDataRead(MBDATANUM(BOARD_ANM_masuLight));
    masuCapsuleAnim = HuSprAnimDataRead(MBDATANUM(BOARD_ANM_masuCapsule));
    MakeMasuDisplayList();
    MBMasuDataRead(MASU_LAYER_DEFAULT, dataNum);
    Hu3DLayerHookSet(0, MasuLightHiliteHook);
    masuDrawMdl = Hu3DHookFuncCreate(MasuDraw);
    Hu3DModelCameraSet(masuDrawMdl, HU3D_CAM0);
    if(MBEventMgCheck() == FALSE && !_CheckFlag(FLAG_BOARD_OPENING)) {
        GwSystem.starTotal = 0;
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) && !_CheckFlag(FLAG_MG_CIRCUIT)) {
            MBMasuSaiHiddenSet();
            if(masuDonkeyNum != 0) {
                GwSystem.mgEvent = frandmod(masuDonkeyNum);
            } else {
                GwSystem.mgEvent = 255;
            }
            MBCapsuleBowserAdd();
        }
    }
    MBGateCreate();
    MBCapMachineCreateAll();
    donkeyNum = MBMasuCapsuleFind(MASU_LAYER_DEFAULT, CAPSULE_DONKEY, NULL);
    if(donkeyNum >= 0) {
        MBMasuTypeSet(MASU_LAYER_DEFAULT, donkeyNum, MASU_TYPE_DONKEY);
    }
    koopaNum = MBMasuCapsuleFind(MASU_LAYER_DEFAULT, CAPSULE_KOOPA, koopaId);
    if(koopaNum >= 0) {
        for(i=0; i<koopaNum; i++) {
            MBMasuTypeSet(MASU_LAYER_DEFAULT, koopaId[i], MASU_TYPE_KUPA);
        }
    }
    masuLightNo = 0;
    for(i=0; i<MASU_LIGHT_MAX; i++) {
        masuLight[i].modelId = MBModelCreate(MBDATANUM(BOARD_HSF_masuLightNext), NULL, FALSE);
        MBModelDispSet(masuLight[i].modelId, FALSE);
        masuLight[i].animId = Hu3DAnimCreate(masuLightAnim, MBModelIdGet(masuLight[i].modelId), "i8tex");
        Hu3DAnimAttrSet(masuLight[i].animId, HU3D_ANIM_ATTR_ANIMON);
    }
    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
        dispF = TRUE;
    } else {
        dispF = FALSE;
    }
    masuDispF = dispF;
}

void MBMasuKill(void)
{
    int i;
    Hu3DLayerHookSet(0, NULL);
    if(masuDrawMdl >= 0) {
        Hu3DModelKill(masuDrawMdl);
        masuDrawMdl = HU3D_MODELID_NONE;
    }
    if(masuAnim != NULL) {
        HuSprAnimKill(masuAnim);
        masuAnim = NULL;
    }
    if(masuCapsuleAnim != NULL) {
        HuSprAnimKill(masuCapsuleAnim);
        masuCapsuleAnim = NULL;
    }
    for(i=0; i<MASU_LIGHT_MAX; i++) {
        Hu3DAnimKill(masuLight[i].animId);
        MBModelKill(masuLight[i].modelId);
    }
    if(masuDlBuf != NULL) {
        HuMemDirectFree(masuDlBuf);
        masuDlBuf = NULL;
    }
}

#define DATA_READ16(ptr, dst) do { \
    (dst) = (*((u16 *)(ptr))); \
    ptr = ((u8 *)ptr)+sizeof(u16); \
} while (0)

#define DATA_READLINK(ptr, dst) do { \
    (dst) = (*((u16 *)(ptr)))+1; \
    ptr = ((u8 *)ptr)+sizeof(u16); \
} while (0)
    
#define DATA_READ32(ptr, dst) do { \
    (dst) = (*((u32 *)(ptr))); \
    ptr = ((u8 *)ptr)+sizeof(u32); \
} while (0)

#define DATA_READCOUNT(dataPtr, ptr, dst) do { \
    (dst) = (*((s32 *)(dataPtr))); \
    ptr = ((u8 *)dataPtr)+sizeof(s32); \
} while (0)

#define DATA_READVEC(ptr, dst) do { \
    memcpy(&(dst), ptr, sizeof(HuVecF)); \
    ptr = ((u8 *)ptr)+sizeof(HuVecF); \
} while (0)
    
BOOL MBMasuDataRead(int layer, int dataNum)
{
    MASU *masuP;
    void *ptr;
    int i, j;
    int starNum;
    int *dataPtr;
    
    starNum = 0;
    dataPtr = HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HUHEAPTYPE_MODEL);
    DATA_READCOUNT(dataPtr, ptr, masuNum[layer]);
    masuDonkeyNum = 0;
    for(masuP = &masuData[layer][0], i=0; i<masuNum[layer]; i++, masuP++) {
        DATA_READVEC(ptr, masuP->pos);
        DATA_READVEC(ptr, masuP->rot);
        DATA_READVEC(ptr, masuP->scale);
        DATA_READ32(ptr, masuP->flag);
        DATA_READ16(ptr, masuP->type);
        masuP->capsuleNo = GwSystem.masuCapsule[i];
        DATA_READ16(ptr, masuP->linkNum);
        for(j=0; j<masuP->linkNum; j++) {
            DATA_READLINK(ptr, masuP->linkTbl[j]);
        }
        if(masuP->type == MASU_TYPE_STAR) {
            masuP->type = MASU_TYPE_BLUE;
            masuStarTbl[starNum++] = i+1;
            masuP->hasStarF = TRUE;
        }
        if(masuP->type == MASU_TYPE_DONKEY) {
            masuP->type = MASU_TYPE_BLUE;
            masuDonkeyTbl[masuDonkeyNum++] = i+1;
        }
    }
    HuDataClose(dataPtr);
    MBStarNumSet(starNum);
    return TRUE;
}

#undef DATA_READ16
#undef DATA_READ32
#undef DATA_READCOUNT
#undef DATA_READVEC
#undef DATA_READLINK

#define DLBUF_SIZE 4096

static void MakeMasuDisplayList(void)
{
    void *dlBuf = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, DLBUF_SIZE, HU_MEMNUM_OVL);
    DCInvalidateRange(dlBuf, DLBUF_SIZE);
    GXBeginDisplayList(dlBuf, DLBUF_SIZE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-100, 0, -100);
    GXTexCoord2f32(0, 0);
    GXPosition3f32(100, 0, -100);
    GXTexCoord2f32(1, 0);
    GXPosition3f32(100, 0, 100);
    GXTexCoord2f32(1, 1);
    GXPosition3f32(-100, 0, 100);
    GXTexCoord2f32(0, 1);
    GXEnd();
    masuDlBufLen = GXEndDisplayList();
    masuDlBuf = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, masuDlBufLen, HU_MEMNUM_OVL);
    memcpy(masuDlBuf, dlBuf, masuDlBufLen);
    DCFlushRangeNoSync(masuDlBuf, masuDlBufLen);
    HuMemDirectFree(dlBuf);
}

#undef DLBUF_SIZE

static void MasuLightDraw(void)
{
    MASU *masuP;
    int i, j;
    int posNo;
    HuVecF posPlayer[GW_PLAYER_MAX];
    
    static int walkAnmNo[] = {
        -1, 0, 1, 1,
        2, 2, 2, 2,
        0, -1, -1, -1
    };
    static int capsuleAnmNo[] = {
        -1, 4, 5, -1,
        -1, -1, -1, -1,
        -1, -1, -1, -1
    };
    static int capsuleSelAnmNo[] = {
        -1, 6, 7, -1,
        -1, -1, -1, -1,
        -1, -1, -1, -1
    };
    static HuVecF upHalf = { 0, 0.5f, 0 };
    posNo = 0;
    if(GWPartyFGet() != FALSE || (GwSystem.turnPlayerNo >= 0 && MBPlayerStoryComCheck(GwSystem.turnPlayerNo) == FALSE)) {
        int playerNo = GwSystem.turnPlayerNo;
        if(GwPlayer[playerNo].dispLightF && playerNo >= 0) {
            MBPlayerPosGet(playerNo, &posPlayer[posNo++]);
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i) && GwPlayer[i].dispLightF) {
                MBPlayerPosGet(i, &posPlayer[posNo++]);
            }
        }
    }
    masuP = &masuData[MASU_LAYER_DEFAULT][0];
    for(i=0; i<masuNum[MASU_LAYER_DEFAULT]; i++, masuP++) {
        Mtx trans;
        Mtx rot;
        Mtx model;
        HuVecF pos;
        BOOL playerMasuF;
        int anmNo;
        if(masuP->type == MASU_TYPE_NONE) {
            continue;
        }
        if(masuP->flag & masuDispMask[MASU_LAYER_DEFAULT]) {
            continue;
        }
        MBMasuPosGet(MASU_LAYER_DEFAULT, (masuP-&masuData[MASU_LAYER_DEFAULT][0])+1, &pos);
        if(MBCameraCullCheck(&pos, 200) == FALSE) {
            continue;
        }
        playerMasuF = FALSE;
        for(j=0; j<posNo; j++) {
            if(fabs(pos.x-posPlayer[j].x) < 100 && fabs(pos.z-posPlayer[j].z) < 100 && fabs(pos.y-posPlayer[j].y) < 300) {
                playerMasuF = TRUE;
            }
        }
        if(masuP->useMtxF == FALSE) {
            mtxRot(rot, masuP->rot.x, masuP->rot.y, masuP->rot.z);
            MTXTrans(trans, masuP->pos.x, masuP->pos.y+3, masuP->pos.z);
            MTXConcat(trans, rot, model);
        } else {
            MTXCopy(masuP->matrix, model);
        }
        switch(masuP->effNo) {
            case MASU_EFF_WALK:
                anmNo = walkAnmNo[masuP->type];
                break;
            
            case MASU_EFF_CAPSULE:
                anmNo = capsuleAnmNo[masuP->type];
                break;
            
            case MASU_EFF_CAPSULESEL:
                anmNo = capsuleSelAnmNo[masuP->type];
                break;
            
            default:
                anmNo = -1;
                break;
        }
        if(playerMasuF) {
            anmNo = walkAnmNo[masuP->type];
        }
        if(anmNo >= 0) {
            int no;
            MTXMultVec(model, &upHalf, &pos);
            model[0][3] = model[1][3] = model[2][3] = 0;
            no = masuLightNo++;
            masuLight[no].pos.x = pos.x;
            masuLight[no].pos.y = pos.y;
            masuLight[no].pos.z = pos.z;
            MTXCopy(model, masuLight[no].matrix);
            masuLight[no].anmNo = anmNo;
        }
    }
}

static void MasuLightHiliteHook(s16 layerNo)
{
    int i;
    int lightNum;
    if(Hu3DCameraNo == 1) {
        Hu3DZClear();
        return;
    }
    if(masuDispF == FALSE) {
        return;
    }
    lightNum = masuLightNo;
    masuLightNo = 0;
    MasuLightDraw();
    for(i=0; i<masuLightNo; i++) {
        MBModelDispSet(masuLight[i].modelId, TRUE);
        Hu3DModelDispOn(MBModelIdGet(masuLight[i].modelId));
        MBModelPosSetV(masuLight[i].modelId, &masuLight[i].pos);
        MBModelMtxSet(masuLight[i].modelId, &masuLight[i].matrix);
        Hu3DAnmNoSet(masuLight[i].animId, masuLight[i].anmNo);
    }
    if(lightNum > masuLightNo) {
        for(; i<lightNum; i++) {
            MBModelDispSet(masuLight[i].modelId, FALSE);
            Hu3DModelDispOff(MBModelIdGet(masuLight[i].modelId));
        }
    }
}

static void MasuDrawMain(Mtx lookAt);

static void MasuGXInit(void)
{
    GXColor color = masuColor;
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXInvalidateTexAll();
    HuSprTexLoad(masuAnim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_TRUE);
    HuSprTexLoad(masuCapsuleAnim, 0, GX_TEXMAP1, GX_CLAMP, GX_CLAMP, GX_TRUE);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTexGens(3);
    GXSetTexCoordGen(GX_TEVSTAGE0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
    GXSetTexCoordGen(GX_TEVSTAGE1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1);
    GXSetTexCoordGen(GX_TEVSTAGE2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanAmbColor(GX_COLOR0A0, color);
    GXSetChanMatColor(GX_COLOR0A0, color);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_BACK);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

static void MasuDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    Mtx lookAt;
    Mtx44 proj;
    HuVecF target;
    HuVecF pos;
    MBCAMERA *cameraP;
    GXColor color;
    if(masuDispF == FALSE) {
        return;
    }
    cameraP = MBCameraGet();
    MBCameraPosGet(&pos);
    MBCameraPosTargetGet(&target);
    MTXPerspective(proj, cameraP->fov, cameraP->aspect, cameraP->near, cameraP->far);
    GXSetProjection(proj, GX_PERSPECTIVE);
    MTXLookAt(lookAt, &pos, &cameraP->up, &target);
    GXSetViewport(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH, cameraP->viewportNear, cameraP->viewportFar);
    GXSetScissor(cameraP->viewportX, cameraP->viewportY, cameraP->viewportW, cameraP->viewportH);
    MasuGXInit();
    MasuDrawMain(lookAt);
}

static void MasuDrawMain(Mtx lookAt)
{
    MASU *masuP = &masuData[MASU_LAYER_DEFAULT][0];
    static int patNoTbl[MASU_TYPE_MAX] = {
        -1, 0, 1, 3,
        2, 5, -1, 6,
        4
    };
    
    int i;
    masuDispCnt = 0;
    for(i=0; i<masuNum[MASU_LAYER_DEFAULT]; i++, masuP++) {
        float uvX, uvY;
        int tevStage;
        int patNo;
        Mtx modelview;
        Mtx trans;
        Mtx rot;
        Mtx model;
        HuVecF pos;
        Mtx texMtx;
        if(masuP->type == MASU_TYPE_NONE) {
            continue;
        }
        if(masuP->type == MASU_TYPE_BLANKHATENA) {
            continue;
        }
        if(masuP->flag & masuDispMask[MASU_LAYER_DEFAULT]) {
            continue;
        }
        MBMasuPosGet(MASU_LAYER_DEFAULT, (masuP-&masuData[MASU_LAYER_DEFAULT][0])+1, &pos);
        if(MBCameraCullCheck(&pos, 200) == FALSE) {
            continue;
        }
        patNo = patNoTbl[masuP->type];
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_LO_NOOP, GX_LO_NOOP);
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        tevStage = GX_TEVSTAGE0;
        GXSetTevOrder(tevStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        tevStage++;
        uvX = (patNo%4)/4.0f;
        uvY = (patNo/4)/2.0f;
        MTXScale(texMtx, 1.0f/4.0f, 1.0f/2.0f, 0);
        mtxTransCat(texMtx, uvX, uvY, 0);
        GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
        if(masuP->capsuleNo >= 0) {
            patNo = MBCapsuleMasuPatNoGet(masuP->capsuleNo);
            uvX = (patNo%4)/4.0f;
            uvY = (patNo/4)/3.0f;
            MTXScale(texMtx, 1.0f/4.0f, 1.0f/3.0f, 0);
            mtxTransCat(texMtx, uvX, uvY, 0);
            GXLoadTexMtxImm(texMtx, GX_TEXMTX2, GX_MTX2x4);
            GXSetTevOrder(tevStage, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR0A0);
            GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
            GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
            GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            tevStage++;
        }
        GXSetNumTevStages(tevStage);
        if(masuP->useMtxF == FALSE) {
            mtxRot(rot, masuP->rot.x, masuP->rot.y, masuP->rot.z);
            MTXTrans(trans, masuP->pos.x, masuP->pos.y+3, masuP->pos.z);
            MTXConcat(trans, rot, model);
        } else {
            MTXCopy(masuP->matrix, model);
        }
        MTXConcat(lookAt, model, modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXCallDisplayList(masuDlBuf, masuDlBufLen);
        masuDispCnt++;
    }
}

static BOOL CheckMasuCapMachine(MASU *masuP);

BOOL MBMasuWalkExec(int playerNo, int masuId)
{
    MASU *masuP;
    MBPauseDisableSet(TRUE);
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    if(MBMasuStarCheck(masuP)) {
        if(GWPartyFGet() != FALSE) {
            MBMasuStarExec(playerNo, masuId);
        }
        return TRUE;
    }
    MBCapsuleExec(playerNo, masuP->capsuleNo, FALSE, TRUE);
    if(CheckMasuCapMachine(masuP)) {
        MBCapMachineExec(playerNo);
        return TRUE;
    } else if(masuP->type == MASU_TYPE_DONKEY) {
        MBPlayerVoicePanPlay(playerNo, MSM_SE_GUIDE_31);
    }
    MBPauseDisableSet(FALSE);
    return FALSE;
}

BOOL MBMasuWalkEndExec(int playerNo, int masuId)
{
    BOOL result = TRUE;
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        MBTutorialExec(TUTORIAL_INST_MASU_CAPSULE);
    }
    switch(masuP->type) {
        case MASU_TYPE_BLUE:
            GwPlayer[playerNo].blueMasuNum++;
            if(GwPlayer[playerNo].blueMasuNum > 99) {
                GwPlayer[playerNo].blueMasuNum = 99;
            }
            break;
        
        case MASU_TYPE_RED:
            GwPlayer[playerNo].redMasuNum++;
            if(GwPlayer[playerNo].redMasuNum > 99) {
                GwPlayer[playerNo].redMasuNum = 99;
            }
            break;
        
        case MASU_TYPE_KUPA:
            GwPlayer[playerNo].kupaMasuNum++;
            if(GwPlayer[playerNo].kupaMasuNum > 99) {
                GwPlayer[playerNo].kupaMasuNum = 99;
            }
            break;
        
        case MASU_TYPE_HATENA:
        case MASU_TYPE_BLANKHATENA:
            GwPlayer[playerNo].hatenaMasuNum++;
            if(GwPlayer[playerNo].hatenaMasuNum > 99) {
                GwPlayer[playerNo].hatenaMasuNum = 99;
            }
            break;
        
        case MASU_TYPE_DONKEY:  
            GwPlayer[playerNo].donkeyMasuNum++;
            if(GwPlayer[playerNo].donkeyMasuNum > 99) {
                GwPlayer[playerNo].donkeyMasuNum = 99;
            }
            break;
    }
    if(masuId == GwSystem.saiMasuId) {
        MBSaiHiddenExec(playerNo);
        MBMasuSaiHiddenSet();
    } else {
        if(masuP->type != MASU_TYPE_KUPA && masuP->type != MASU_TYPE_DONKEY) {
            result = MBCapsuleExec(playerNo, masuP->capsuleNo, FALSE, FALSE);
            MB3MiracleGetExec(playerNo);
        }
    }
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        MBTutorialExec(TUTORIAL_INST_WALKEND);
    }
    return result;
}

static void MasuBlueMain(int playerNo);
static void MasuRedMain(int playerNo);
static int MasuHatenaMain(int playerNo, int masuId);

void MBMasuWalkEndEventExec(int playerNo, int masuId)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        MBTutorialExec(TUTORIAL_INST_WALKEND_EVENT);
    }
    switch(masuP->type) {
        case MASU_TYPE_BLUE:
            MasuBlueMain(playerNo);
            break;
        
        case MASU_TYPE_RED:
            MasuRedMain(playerNo);
            break;
        
        case MASU_TYPE_KUPA:
            if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                MBKupaExec(playerNo);
            }
            break;
        
        case MASU_TYPE_HATENA:
        case MASU_TYPE_BLANKHATENA:
            MasuHatenaMain(playerNo, masuId);
            break;
        
        case MASU_TYPE_STAR:
            MBMasuStarExec(playerNo, masuId);
            break;
        
        case MASU_TYPE_DONKEY:
            if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                MBDonkeyExec(playerNo);
            }
            break;
        
        case MASU_TYPE_VS:
            MBVsMgExec(playerNo);
            break;
    }
}

static void MasuCoinMain(int playerNo, int coinNum)
{
    HuVecF pos;
    BOOL doneF;
    s8 dispId;
    if(coinNum < 0) {
        omVibrate(playerNo, 12, 6, 6);
    }
    MBPlayerRotateStart(playerNo, 0, 15);
    while(MBPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    if(GwSystem.last5Effect == MB_LAST5_EFF_COINMUL) {
        coinNum *= 3;
    }
    MBPlayerPosGet(playerNo, &pos);
    pos.y += 250;
    if(coinNum >= 0) {
        MBAudFXPlay(MSM_SE_BOARD_97);
    } else {
        MBAudFXPlay(MSM_SE_BOARD_98);
    }
    dispId = MBCoinDispCreate(&pos, coinNum, FALSE);
    MBCameraMotionWait();
    if(coinNum >= 0) {
        MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINWIN, 0, 4, HU3D_ATTR_NONE);
    } else {
        MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
        MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_COINLOSE, 0, 4, HU3D_ATTR_NONE);
    }
    MBCoinAddExec(playerNo, coinNum);
    for(doneF=FALSE; MBCoinDispCheck(dispId) == FALSE || doneF == FALSE;) {
        if(MBPlayerMotionEndCheck(playerNo) && !doneF) {
            MBPlayerMotIdleSet(playerNo);
            doneF = TRUE;
        }
        HuPrcVSleep();
    }
}

static void MasuBlueMain(int playerNo)
{
    MBCameraSkipSet(TRUE);
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MBCameraMotionWait();
    MasuCoinMain(playerNo, 3);
}

void MBMasuColorSet(int playerNo)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId);
    int colorTbl[][2] = {
        MASU_TYPE_BLUE, STATUS_COLOR_BLUE,
        MASU_TYPE_RED, STATUS_COLOR_RED,
        MASU_TYPE_KUPA, STATUS_COLOR_RED,
        MASU_TYPE_HATENA, STATUS_COLOR_GREEN,
        MASU_TYPE_BLANKHATENA, STATUS_COLOR_GREEN,
        MASU_TYPE_DONKEY, STATUS_COLOR_BLUE,
        MASU_TYPE_VS, STATUS_COLOR_GREEN,
        MASU_TYPE_NULL, STATUS_COLOR_NULL
    };
    int color;
    for(color=0; colorTbl[color][0] >= 0; color++) {
        if(masuP->type == colorTbl[color][0]) {
            break;
        }
    }
    if(colorTbl[color][0] < 0) {
        MBStatusColorSet(playerNo, STATUS_COLOR_GRAY);
    } else {
        MBStatusColorSet(playerNo, colorTbl[color][1]);
    }
}

static int MasuHatenaMain(int playerNo, int masuId)
{
    if(masuHatenaHook) {
        MBAudFXPlay(MSM_SE_BOARD_100);
        omVibrate(playerNo, 12, 4, 2);
        masuHatenaHook(playerNo);
    }
    return 0;
}

static void MasuRedMain(int playerNo)
{
    MBCameraSkipSet(TRUE);
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MBCameraMotionWait();
    MasuCoinMain(playerNo, -3);
}


int MBMasuNumGet(int layer)
{
    return masuNum[layer];
}

MASU *MBMasuGet(int layer, int id)
{
    if(id <= 0 || id > masuNum[layer]) {
        return NULL;
    }
    return &masuData[layer][id-1];
}

u32 MBMasuFlagGet(int layer, int id)
{
    if(id <= 0 || id > masuNum[layer]) {
        return MASU_FLAG_NONE;
    }
    return masuData[layer][id-1].flag;
}

int MBMasuTypeGet(int layer, int id)
{
    if(id <= 0 || id > masuNum[layer]) {
        return MASU_TYPE_NONE;
    }
    return masuData[layer][id-1].type;
}

void MBMasuTypeSet(int layer, int id, int type)
{
    if(id <= 0 || id > masuNum[layer]) {
        return;
    }
    masuData[layer][id-1].type = type;
}

int MBMasuCapsuleGet(int layer, int id)
{
    if(id <= 0 || id > masuNum[layer]) {
        return CAPSULE_NULL;
    }
    return masuData[layer][id-1].capsuleNo;
}

void MBMasuCapsuleSet(int layer, int id, int capsuleNo)
{
    if(id <= 0 || id > masuNum[layer]) {
        return;
    }
    if(id == GwSystem.saiMasuId) {
        MBMasuSaiHiddenSet();
    }
    GwSystem.masuCapsule[id-1] = capsuleNo;
    masuData[layer][id-1].capsuleNo = capsuleNo;
}

void MBMasuDispMaskSet(int layer, u32 bit)
{
    masuDispMask[layer] |= bit;
}

void MBMasuDispMaskReset(int layer, u32 bit)
{
    masuDispMask[layer] &= ~bit;
}

BOOL MBMasuPosGet(int layer, int id, HuVecF *pos)
{
    MASU *masuP = MBMasuGet(layer, id);
    if(masuP == NULL) {
        return FALSE;
    }
    if(masuP->useMtxF == FALSE) {
        *pos = masuP->pos;
    } else {
        pos->x = masuP->matrix[0][3];
        pos->y = masuP->matrix[1][3];
        pos->z = masuP->matrix[2][3];
    }
    return TRUE;
}

#define CORNER_MAX 8
typedef struct MasuCorner_s {
    int no;
    int order;
} MASUCORNER;

void MBMasuCornerPosGet(int id, int cornerNo, HuVecF *pos)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, id);
    MASUCORNER cornerTbl[CORNER_MAX];
    s16 linkTbl[MASU_LINK_MAX*2];
    int cornerNum[CORNER_MAX];
    HuVecF posMasu;
    HuVecF posLink;
    HuVecF linkDir;
    
    int i, j;
    int linkNum;
    static int cornerNoTbl[CORNER_MAX] = {
        3, 1,
        7, 5,
        4, 2,
        0, 6
    };
    for(linkNum=0, i=0; i<masuP->linkNum; i++) {
        s32 link = masuP->linkTbl[i];
        linkTbl[linkNum++] = link;
    }
    linkNum += MBMasuLinkTblGet(MASU_LAYER_DEFAULT, id, &linkTbl[linkNum]);
    for(i=0; i<CORNER_MAX; i++) {
        cornerNum[i] = 0;
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, id, &posMasu);
    for(i=0; i<linkNum; i++) {
        float linkAngle;
        s32 cornerIdx;
        MBMasuPosGet(MASU_LAYER_DEFAULT, linkTbl[i], &posLink);
        VECSubtract(&posLink, &posMasu, &linkDir);
        linkAngle = HuAtan(-linkDir.z, linkDir.x);
        if(linkAngle < 0) {
            linkAngle += 360;
        }
        cornerIdx = (s16)((linkAngle+22.5f)/45);
        cornerNum[cornerIdx]++;
    }
    for(i=0; i<CORNER_MAX; i++) {
        cornerTbl[i].no = cornerNoTbl[i];
        cornerTbl[i].order = cornerNum[cornerTbl[i].no];
    }
    for(i=0; i<CORNER_MAX-1; i++) {
        for(j=i; j<CORNER_MAX; j++) {
            if(cornerTbl[j].order < cornerTbl[i].order) {
                MASUCORNER temp = cornerTbl[i];
                cornerTbl[i] = cornerTbl[j];
                cornerTbl[j] = temp;
            }
        }
    }
    pos->x = HuCos((cornerTbl[cornerNo].no*360.0f)/8.0f)*100;
    pos->y = 0;
    pos->z = -HuSin((cornerTbl[cornerNo].no*360.0f)/8.0f)*100;
}

#undef CORNER_MAX

void MBMasuCornerRotPosGet(int id, int cornerNo, HuVecF *pos)
{
    HuVecF posMasu;
    HuVecF posCorner;
    HuVecF rot;
    MBMasuCornerPosGet(id, cornerNo, &posCorner);
    MBMasuPosGet(MASU_LAYER_DEFAULT, id, &posMasu);
    MBMasuRotGet(MASU_LAYER_DEFAULT, id, &rot);
    pos->x = ((posCorner.x*HuCos(rot.z))+posMasu.x+(posCorner.y*HuSin(rot.z)));
    pos->y = (posCorner.z*HuSin(-rot.x))+(posMasu.y+(posCorner.x*HuSin(rot.z))+(posCorner.y*(HuCos(rot.x)*HuCos(rot.z))));
    pos->z = ((posCorner.y*HuSin(rot.x))+posMasu.z+(posCorner.z*HuCos(rot.x)));
}

BOOL MBMasuRotGet(int layer, int id, HuVecF *rot)
{
    MASU *masuP = MBMasuGet(layer, id);
    if(!masuP) {
        return FALSE;
    }
    if(masuP->useMtxF) {
        Hu3DMtxRotGet(masuP->matrix, rot);
    } else {
        *rot = masuP->rot;
        
    }
    return TRUE;
}

int MBMasuFlagFind(int layer, u32 flag, u32 mask)
{
    int i;
    for(i=0; i<masuNum[layer]; i++) {
        MASU *masuP = &masuData[layer][i];
        if(flag == (masuP->flag & mask)) {
            return (masuP-&masuData[layer][0])+1;
        }
    }
    return MASU_NULL;
}

int MBMasuFlagMatchFind(int layer, u32 flag)
{
    return MBMasuFlagFind(layer, flag, flag);
}

int MBMasuFlagPosGet(int layer, u32 flag, HuVecF *pos)
{
    int masuId = MBMasuFlagMatchFind(layer, flag);
    if(masuId == MASU_NULL) {
        return MASU_NULL;
    }
    if(pos) {
        MBMasuPosGet(layer, masuId, pos);
    }
    return masuId;
}

int MBMasuFlagFindLink(int layer, int masuId, u32 flag, u32 mask)
{
    MASU *masuP = MBMasuGet(layer, masuId);
    int i;
    if(!masuP) {
        return MASU_NULL;
    }
    for(i=0; i<masuP->linkNum; i++) {
        MASU *linkMasuP = MBMasuGet(layer, masuP->linkTbl[i]);
        if(flag == (linkMasuP->flag & mask)) {
            return (linkMasuP-&masuData[layer][0])+1;
        }
    }
    return MASU_NULL;
}

int MBMasuFlagMatchFindLink(int layer, int masuId, u32 flag)
{
    return MBMasuFlagFindLink(layer, masuId, flag, flag);
}


int MBMasuLinkTblGet(int layer, int id, s16 *linkTbl)
{
    int i, j;
    int linkNum = 0;
    MASU *masuP = &masuData[layer][0];
    for(i=0; i<masuNum[layer]; i++, masuP++) {
        for(j=0; j<masuP->linkNum; j++) {
            if(masuP->linkTbl[j] == id && linkNum < MASU_LINK_MAX) {
                linkTbl[linkNum++] = (masuP-&masuData[layer][0])+1;
            }
        }
    }
    return linkNum;
}

int MBMasuTypeLinkGet(int layer, int id, u16 type)
{
    MASU *masuP = MBMasuGet(layer, id);
    int i;
    if(!masuP) {
        return MASU_NULL;
    }
    for(i=0; i<masuP->linkNum; i++) {
        MASU *linkMasuP = MBMasuGet(layer, masuP->linkTbl[i]);
        if(linkMasuP->type == type) {
            return masuP->linkTbl[i];
        }
    }
    return MASU_NULL;
}

int MBMasuCapsuleFind(int layer, int capsuleNo, int *list)
{
    int i;
    int listNum;
    listNum = 0;
    for(i=0; i<masuNum[layer]; i++) {
        MASU *masuP = &masuData[layer][i];
        if(masuP->capsuleNo >= 0 && masuP->capsuleNo == capsuleNo) {
            int no = (masuP-&masuData[layer][0])+1;
            if(list != NULL) {
                list[listNum++] = no;
            } else {
                return no;
            }
        }
    }
    if(listNum == 0) {
        return MASU_NULL;
    } else {
        return listNum;
    }
}

void MBMasuWalkPostHookSet(MASUPLAYERHOOK hook)
{
    masuWalkPostHook = hook;
}

void MBMasuWalkPreHookSet(MASUPLAYERHOOK hook)
{
    masuWalkPreHook = hook;
}

int MBMasuWalkPostExec(int playerNo)
{
    int result = FALSE;
    if(masuWalkPostHook) {
        MBPauseDisableSet(TRUE);
        result = masuWalkPostHook(playerNo);
        MBPauseDisableSet(FALSE);
    }
    return result;
}

int MBMasuWalkPreExec(int playerNo)
{
    int result = -1;
    if(masuWalkPreHook) {
        MBPauseDisableSet(TRUE);
        result = masuWalkPreHook(playerNo);
        MBPauseDisableSet(FALSE);
    }
    return result;
}

void MBMasuHatenaHookSet(MASUPLAYERHOOK hook)
{
    masuHatenaHook = hook;
}

static BOOL CheckMasuCapMachine(MASU *masuP)
{
    if(masuP->flag & MASU_FLAG_CAPMACHINE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void MBMasuTypeChange(u16 oldType, u16 newType)
{
    int i;
    for(i=0; i<masuNum[0]; i++) {
        MASU *masuP = &masuData[0][i];
        if(masuP->type == oldType) {
            masuP->type = newType;
            masuP->capsuleNo = CAPSULE_NULL;
        }
    }
}

void MBMasuDispSet(BOOL dispF)
{
    if(dispF) {
        Hu3DModelDispOn(masuDrawMdl);
    } else {
        Hu3DModelDispOff(masuDrawMdl);
    }
}

void MBMasuCameraSet(u16 cameraBit)
{
    Hu3DModelCameraSet(masuDrawMdl, cameraBit);
}

void MBMasuSaiHiddenSet(void)
{
    int no;
    int masuNum;
    int *saiMasuIdTbl;
    int i;
    int saiMasuNum;
    
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }
    if(GWPartyFGet() == FALSE || !GWBonusStarFGet()) {
        GwSystem.saiMasuId = MASU_NULL;
        return;
    }
    masuNum = MBMasuNumGet(MASU_LAYER_DEFAULT);
    saiMasuIdTbl = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, sizeof(int)*masuNum, HU_MEMNUM_OVL);
    saiMasuNum = 0;
    for(i=0; i<masuNum; i++) {
        MASU *masuP;
        no = i+1;
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, no);
        if(no != GwSystem.saiMasuId && masuP->type == MASU_TYPE_BLUE && masuP->capsuleNo < 0) {
            saiMasuIdTbl[saiMasuNum++] = no;
        }
    }
    if(saiMasuNum == 0) {
        GwSystem.saiMasuId = MASU_NULL;
    } else {
        GwSystem.saiMasuId = saiMasuIdTbl[frandmod(saiMasuNum)];
    }
    HuMemDirectFree(saiMasuIdTbl);
}

void MBMasuEffSet(int masuId, int effNo)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    masuP->effNo = effNo;
}

void MBMasuEffClear(void)
{
    MASU *masuP = &masuData[MASU_LAYER_DEFAULT][0];
    int i;
    for(i=0; i<masuNum[MASU_LAYER_DEFAULT]; i++, masuP++) {
        masuP->effNo = MASU_EFF_NONE;
    }
}

void MBMasuPlayerLightSet(int playerNo, BOOL dispLightF)
{
    GwPlayer[playerNo].dispLightF = dispLightF;
}

void MBMasuMtxSet(int masuId, Mtx matrix)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    if(matrix != NULL) {
        MTXCopy(matrix, masuP->matrix);
        masuP->useMtxF = TRUE;
        MBPlayerPosFixSet(GwSystem.turnPlayerNo, masuId);
    } else {
        masuP->useMtxF = FALSE;
    }
}

void MBMasuMtxGet(int masuId, Mtx matrix)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    if(masuP->useMtxF) {
        MTXCopy(masuP->matrix, matrix);
    } else {
        Mtx rot, trans;
        mtxRot(rot, masuP->rot.x, masuP->rot.y, masuP->rot.z);
        MTXTrans(trans, masuP->pos.x, masuP->pos.y, masuP->pos.z);
        MTXConcat(trans, rot, matrix);
    }
}

int MBMasuStarGet(int starNo)
{
    return masuStarTbl[starNo % MASU_STAR_MAX];
}

BOOL MBMasuStarCheck(MASU *masuP)
{
    return masuP->type == MASU_TYPE_STAR;
}

BOOL MBMasuIdStarCheck(int masuId)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    return MBMasuStarCheck(masuP);
}

void MBMasuStub(void)
{
    
}

void MBMasuCapsuleClear(void)
{
    int i;
    for(i=0; i<MASU_MAX; i++) {
        GwSystem.masuCapsule[i] = CAPSULE_NULL;
    }
}

void MBMasuPathCheckHookSet(MASUPATHCHECKHOOK hook)
{
    masuPathCheckHook = hook;
}

int MBMasuPathCheck(s16 masuId, s16 *linkTbl, BOOL endF)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    MASU *masuLinkP;
    int i;
    int linkNum;
    
    if(masuPathCheckHook != NULL) {
        if((linkNum = masuPathCheckHook(masuId, masuP->flag, linkTbl, endF)) >= 0) {
            return linkNum;
        }
    }
    linkNum = 0;
    for(i=0; i<masuP->linkNum; i++) {
        masuLinkP = MBMasuGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]);
        if((masuLinkP->flag & MASU_FLAG_BIT(25)) == 0 && (masuLinkP->flag & MASU_FLAG_PATHBLOCK) == 0&& (masuLinkP->flag & MBPathFlagGet()) == 0) {
            linkTbl[linkNum++] = masuP->linkTbl[i];
        }
    }
    return linkNum;
}

int MBMasuPathNoLoopCheck(s16 masuId, s16 *linkTbl)
{
    return MBMasuPathCheck(masuId, linkTbl, FALSE);
}

static void MasuPathCheckType(s16 masuId, s16 type, BOOL endF)
{
    s16 linkTbl[MASU_LINK_MAX];
    int i;
    int linkNum;
    if(type == MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId)) {
        if(masuPathCnt < masuPathLen) {
            masuPathLen = masuPathCnt;
            masuPathId = masuId;
        }
        return;
    }
    masuPathCnt++;
    masuPathVisitF[masuId] = TRUE;
    linkNum = MBMasuPathCheck(masuId, linkTbl, endF);
    for(i=0; i<linkNum; i++) {
        if(masuPathVisitF[linkTbl[i]]) {
            continue;
        }
        MasuPathCheckType(linkTbl[i], type, endF);
    }
    masuPathCnt--;
    masuPathVisitF[masuId] = FALSE;
}

int MBMasuPathTypeWrapLenGet(s16 masuId, s16 type, BOOL endF)
{
    masuPathCnt = 0;
    masuPathLen = 9999;
    masuPathId = MASU_NULL;
    memset(masuPathVisitF, 0, MASU_MAX);
    MasuPathCheckType(masuId, type, endF);
    return masuPathLen;
}

int MBMasuPathTypeLenGet(s16 masuId, s16 type)
{
    return MBMasuPathTypeWrapLenGet(masuId, type, FALSE);
}

static void MasuPathCheckFlag(s16 masuId, u32 flag, u32 mask)
{
    s16 linkTbl[MASU_LINK_MAX];
    int i;
    int linkNum;
    if(flag == (mask & MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId))) {
        if(masuPathCnt < masuPathLen) {
            masuPathLen = masuPathCnt;
            masuPathId = masuId;
        }
        return;
    }
    masuPathCnt++;
    masuPathVisitF[masuId] = TRUE;
    linkNum = MBMasuPathNoLoopCheck(masuId, linkTbl);
    for(i=0; i<linkNum; i++) {
        if(masuPathVisitF[linkTbl[i]]) {
            continue;
        }
        MasuPathCheckFlag(linkTbl[i], flag, mask);
    }
    masuPathCnt--;
    masuPathVisitF[masuId] = FALSE;
}

int MBMasuPathFlagLenGet(s16 masuId, u32 flag, u32 mask)
{
    masuPathCnt = 0;
    masuPathLen = 9999;
    masuPathId = MASU_NULL;
    memset(masuPathVisitF, 0, MASU_MAX);
    MasuPathCheckFlag(masuId, flag, mask);
    return masuPathLen;
}

int MBMasuPathFlagMatchLenGet(s16 masuId, u32 mask)
{
    return MBMasuPathFlagLenGet(masuId, mask, mask);
}

s16 MBMasuPathMasuGet(s16 masuId, s16 type)
{
    MBMasuPathTypeLenGet(masuId, type);
    return masuPathId;
}

s16 MBMasuPathMasuWrapGet(s16 masuId, s16 type, BOOL endF)
{
    MBMasuPathTypeWrapLenGet(masuId, type, endF);
    return masuPathId;
}

static void MasuPathCheck(s16 masuId, s16 masuIdEnd, BOOL endF)
{
    s16 linkTbl[MASU_LINK_MAX];
    int i;
    int linkNum;
    if(masuId == masuIdEnd) {
        if(masuPathCnt < masuPathLen) {
            masuPathLen = masuPathCnt;
            masuPathId = masuId;
        }
        return;
    }
    masuPathCnt++;
    masuPathVisitF[masuId] = TRUE;
    linkNum = MBMasuPathCheck(masuId, linkTbl, endF);
    for(i=0; i<linkNum; i++) {
        if(masuPathVisitF[linkTbl[i]]) {
            continue;
        }
        MasuPathCheck(linkTbl[i], masuIdEnd, endF);
    }
    masuPathCnt--;
    masuPathVisitF[masuId] = FALSE;
}

int MBMasuPathWrapLenGet(s16 masuId, s16 masuIdEnd, BOOL endF)
{
    masuPathCnt = 0;
    masuPathLen = 9999;
    masuPathId = MASU_NULL;
    memset(masuPathVisitF, 0, MASU_MAX);
    MasuPathCheck(masuId, masuIdEnd, endF);
    return masuPathLen;
}

int MBMasuPathLenGet(s16 masuId, s16 masuIdEnd)
{
    return MBMasuPathWrapLenGet(masuId, masuIdEnd, FALSE);
}

u32 MBMasuEventFlagGet(u32 flag, u32 mask)
{
    int i;
    u32 result, bitSet, bit;
    flag &= mask;
    for(result=0, bitSet=bit=MASU_FLAG_BIT(0), i=0; i<32; bit <<= 1, i++) {
        if(mask & bit) {
            if(flag & bit) {
                result |= bitSet;
            }
            bitSet <<= 1;
        }
    }
    return result;
}

u32 MBMasuDispMaskGet(void)
{
    return masuDispMask[MASU_LAYER_DEFAULT];
}

BOOL MBMasuDispGet(void)
{
    return masuDispF;
}

s16 MBMasuCarTargetGet(s16 masuId, s16 playerNo)
{
    int i, j;
    for(i=0; i<playerNo; i++) {
        MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
        for(j=0; j<masuP->linkNum; j++) {
            MASU *linkMasuP = MBMasuGet(MASU_LAYER_DEFAULT, masuP->linkTbl[j]);
            if((linkMasuP->flag & MASU_FLAG_JUMPFROM) == 0 && (linkMasuP->flag & MASU_FLAG_PATHBLOCK) == 0) {
                masuId = masuP->linkTbl[j];
                break;
            }
        }
    }
    return masuId;
}

s16 MBMasuTargetGet(s16 masuId)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
        s16 linkTbl[MASU_LINK_MAX];
        int linkNum;
        if(masuP->flag & MASU_FLAG_JUMPFROM) {
            return masuId;
        }
        linkNum = MBMasuLinkTblGet(MASU_LAYER_DEFAULT, masuId, linkTbl);
        masuId = linkTbl[0];
    }
    return MASU_NULL;
}

s16 MBMasuCarNextGet(s16 masuId)
{
    s16 masuTarget = MBMasuTargetGet(masuId);
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuTarget);
    s16 masuNext = MASU_NULL;
    int i;
    for(i=0; i<masuP->linkNum; i++) {
        MASU *linkMasuP = MBMasuGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]);
        if(linkMasuP->flag & MASU_FLAG_JUMPFROM) {
            masuNext = masuP->linkTbl[i];
            break;
        }
    }
    return masuNext;
}

s16 MBMasuCarPrevGet(s16 masuId)
{
    s16 masuTarget = MBMasuTargetGet(masuId);
    MASU *masuP = &masuData[MASU_LAYER_DEFAULT][0];
    int i, j;
    for(i=0; i<masuNum[MASU_LAYER_DEFAULT]; i++, masuP++) {
        for(j=0; j<masuP->linkNum; j++) {
            if(masuP->linkTbl[j] == masuTarget && (masuP->flag & MASU_FLAG_JUMPFROM)) {
                return (masuP-&masuData[MASU_LAYER_DEFAULT][0])+1;
            }
        }
    }
    return MASU_NULL;
}

s16 MBMasuDonkeySet(s16 no)
{
    s16 masuId;
    if(no < 0) {
        return;
    }
    masuId = masuDonkeyTbl[no];
    MBMasuTypeSet(MASU_LAYER_DEFAULT, masuId, MASU_TYPE_DONKEY);
    MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, CAPSULE_DONKEY);
    return masuId;
}

s16 MBMasuDonkeyGet(s16 no)
{
    if(no >= masuDonkeyNum) {
        return MASU_NULL;
    }
    return masuDonkeyTbl[no];
}

s16 MBMasuDonkeyNumGet(void)
{
    return masuDonkeyNum;
}