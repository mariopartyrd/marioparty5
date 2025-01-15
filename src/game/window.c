#include "game/window.h"
#include "game/gamework.h"
#include "game/process.h"
#include "game/data.h"
#include "game/armem.h"
#include "game/audio.h"
#include "game/hu3d.h"
#include "game/dvd.h"
#include "game/pad.h"

#include "game/disp.h"
#include "datanum/win.h"
#include "humath.h"

#include "stdarg.h"

#define WIN_COMKEY_MAX 256

typedef struct spcFontTbl_s {
    ANIMDATA **animP;
    s16 animBank;
    s16 w;
    s16 h;
    s16 centerX;
    s16 centerY;
} SPCFONTTBL;

typedef struct winComKey_s {
    s16 time;
    u32 key[GW_PLAYER_MAX];
} WINCOMKEY;

static u8 mesWInsert[HUWIN_INSERTMES_MAX];
static WINCOMKEY winComKeyBuf[WIN_COMKEY_MAX];
static u32 winKey[GW_PLAYER_MAX];
HUWIN winData[HUWIN_MAX];

static ANIMDATA *iconAnim;
static ANIMDATA *cursorAnim;
static ANIMDATA *cardAnimA;
static ANIMDATA *cardAnimB;
static HUPROCESS *winProc;
void *messDataPtr;
static s32 messDataNo;
static s16 winMaxWidth;
static s16 winMaxHeight;
static u8 winTabSize;
static u8 winInsertF;
static AMEM_PTR winAMemP;
static u8 *fontWidthP;
static s32 cancelCRF;
static s16 mesCharCnt;
static s16 comKeyIdxNow;
static s16 comKeyIdx;
static u8 LanguageNo;
static ANIMDATA *fontAnim;

s16 winPrio = 1000;
static BOOL newMessDataF = TRUE;

static SPCFONTTBL spcFontTbl[] = {
    {   &iconAnim,  0, 20, 24,  10, 12 },
    {   &iconAnim,  1, 20, 24,  10, 12 },
    {   &iconAnim,  2, 20, 24,  10, 12 },
    {   &iconAnim,  3, 20, 24,  10, 12 },
    {   &iconAnim,  4, 20, 24,  10, 12 },
    {   &iconAnim,  5, 20, 24,  10, 12 },
    {   &iconAnim,  6, 20, 24,  10, 12 },
    {   &iconAnim,  7, 20, 24,  10, 12 },
    {   &iconAnim,  8, 20, 24,  10, 12 },
    {   &iconAnim,  9, 20, 24,  10, 12 },
    {   &iconAnim, 10, 20, 24,  10, 12 },
    {   &iconAnim, 11, 30, 24,  15, 12 },
    {   &iconAnim, 12, 20, 24,  10, 12 },
    {   &iconAnim, 13, 20, 24,  10, 12 },
    {   &iconAnim, 14, 20, 24,  10, 12 },
    {   &iconAnim, 15, 20, 24,  10, 12 },
    {   &iconAnim, 16, 20, 24,  10, 12 },
    {   &iconAnim, 17, 20, 24,  10, 12 },
    {   &iconAnim, 18, 20, 24,  10, 12 },
    {   &iconAnim, 19, 24, 24,  12, 12 },
    { &cursorAnim,  0, 40, 32, -15, 18 },
    {  &cardAnimA,  0, 32, 32,  16, 16 },
    {  &cardAnimB,  0, 32, 32,  16, 16 }
};

u8 charWETbl[] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 20, 12, 12, 11, 14,
     8, 13, 12, 12, 12, 12, 12, 12, 12,  9, 11, 12, 11, 15, 12, 13,
    12, 13, 12, 12, 11, 12, 11, 15, 12, 13, 11, 12,  6,  8,  8, 12,
    20, 12, 11, 12, 11, 11,  9, 12, 11,  4,  8, 11,  4, 14, 11, 12,
    11, 12,  9, 11,  9, 11, 11, 15, 11, 11, 11,  4, 13,  8, 14, 12,
     9,  8,  8,  8, 20,  4, 12, 16, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 12, 12, 12, 12, 12, 12, 12, 12,  8,  8, 12, 12, 12, 12, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    12, 12,  7, 14, 17, 13, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 16, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

u8 charWJTbl[] = {
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,  0, 20,  0,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 80,  0, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
     0,  0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

static u8 ATTRIBUTE_ALIGN(32) charColPal[2][3][HUWIN_MESCOL_MAX] = {
    0, 0, 0,
    0, 0, 255,
    255, 0, 0,
    255, 0, 255,
    0, 255, 0,
    0, 255, 255,
    255, 160, 0,
    255, 255, 255,
    96, 96, 96,
    144, 144, 144,
    0, 0, 0,
    96, 176, 255,
    255, 64, 128,
    255, 0, 255,
    0, 255, 0,
    0, 255, 255,
    255, 255, 0,
    255, 255, 255,
    96, 96, 96,
    144, 144, 144
};

static unsigned int frameFileTbl[] = {
    WIN_ANM_frame1,
    WIN_ANM_frame2,
    WIN_ANM_frame3,
    HU_DATANUM_NONE,
    WIN_ANM_frame4,
    HU_DATANUM_NONE,
    WIN_ANM_frame1,
    HU_DATANUM_NONE
};

static char *mesDataTbl[] = {
    "mess/messdata.bin",
    "mess/board.dat",
    "mess/messdata_eng.bin",
    "mess/board.dat",
    "mess/messdata.bin",
    "mess/board.dat",
};

static int speakerSeIdTbl[21] = {};

static void MesDispFunc(HUSPRITE *sprP);
static u8 winBGMake(ANIMDATA *bgAnim, ANIMDATA *frameAnim);
static void HuWinProc(void);
static void HuWinDrawMes(HUWINID winId);
static BOOL HuWinCR(HUWIN *winP);
static void _HuWinHomeClear(HUWIN *winP);
static void HuWinKeyWait(HUWINID winId);
static s16 HuWinSpcFontEntry(HUWIN *winP, s16 entry, s16 x, s16 y);
static void HuWinSpcFontClear(HUWIN *winP);
static void HuWinChoice(HUWIN *winP);
static void GetMesMaxSizeSub(u32 messNum);
static s32 GetMesMaxSizeSub2(HUWIN *winP, void *data);

void HuWindowInit(void)
{
    s16 i;
    winAMemP = HuAR_DVDtoARAM(DATA_win);
    for(i=0; i<HUWIN_MAX; i++) {
        winData[i].grpId = HUSPR_GRP_NONE;
    }
    winProc = NULL;
    winPrio = 1000;
}

void HuWinInit(s32 _messDataNo)
{
    void *data;
    s16 i;
    if(winProc) {
        return;
    }
    HuAR_ARAMtoMRAM(winAMemP);
    winProc = HuPrcCreate(HuWinProc, 100, 4096, 0);
    HuPrcSetStat(winProc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    LanguageNo = GWLanguageGet();
    messDataNo = _messDataNo;
    fontWidthP = (LanguageNo == HUWIN_LANG_JAPAN) ? charWJTbl : charWETbl;
    HuWinMesRead();
    for(i=0; i<HUWIN_MAX; i++) {
        winData[i].grpId = HUSPR_GRP_NONE;
    }
    if(!fontAnim) {
        data = HuDataReadNum(WIN_ANM_font, HU_MEMNUM_OVL);
        fontAnim = HuSprAnimRead(data);
    }
    if(!iconAnim) {
        data = HuDataReadNum(WIN_ANM_icon, HU_MEMNUM_OVL);
        iconAnim = HuSprAnimRead(data);
        HuSprAnimLock(iconAnim);
    }
    if(!cursorAnim) {
        data = HuDataReadNum(WIN_ANM_cursor, HU_MEMNUM_OVL);
        cursorAnim = HuSprAnimRead(data);
        HuSprAnimLock(cursorAnim);
    }
    if(!cardAnimA) {
        data = HuDataReadNum(WIN_ANM_cardA, HU_MEMNUM_OVL);
        cardAnimA = HuSprAnimRead(data);
        HuSprAnimLock(cardAnimA);
    }
    if(!cardAnimB) {
        data = HuDataReadNum(WIN_ANM_cardB, HU_MEMNUM_OVL);
        cardAnimB = HuSprAnimRead(data);
        HuSprAnimLock(cardAnimB);
    }
    HuDataDirClose(DATA_win);
    HuWinComKeyReset();
    winPrio = 1000;
}

HUWINID HuWinCreate(float posX, float posY, s16 winW, s16 winH, s16 frame)
{
    GXColor bgColor = { 64, 16, 128, 255 };
    HUWINID winId;
    s16 i;
    HUWIN *winP;
    HUSPRGRPID grpId;
    HUSPRID sprId;
    ANIMDATA *bgAnim;
    void *data;
    for(winId=0; winId<HUWIN_MAX; winId++) {
        if(winData[winId].grpId == HUSPR_GRP_NONE) {
            break;
        }
    }
    if(winId == HUWIN_MAX) {
        return HUWIN_NONE;
    }
    winP = &winData[winId];
    winP->grpId = grpId = HuSprGrpCreate(HUWIN_SPR_MAX);
    if(frame < 0 || frame >= 4) {
        frame = 0;
    }
    winW = (winW+15)&0xFFF0;
    winH = (winH+15)&0xFFF0;
    if(posX == HUWIN_POS_CENTER) {
        winP->pos.x = (HU_DISP_WIDTH-winW)/2;
    } else {
        winP->pos.x = posX;
    }
    if(posY == HUWIN_POS_CENTER) {
        winP->pos.y = (HU_DISP_HEIGHT-winH)/2;
    } else {
        winP->pos.y = posY;
    }
    HuSprGrpCenterSet(grpId, winW/2, winH/2);
    HuSprGrpPosSet(grpId, winP->pos.x, winP->pos.y);
    data = HuAR_ARAMtoMRAMFileRead(frameFileTbl[(frame*2)+0], HU_MEMNUM_OVL, HEAP_MODEL);
    winP->animFrame[0] = HuSprAnimRead(data);
    sprId = winP->sprId[0] = HuSprCreate(winP->animFrame[0], winPrio, 0);
    HuSprGrpMemberSet(grpId, 0, sprId);
    HuSprTPLvlSet(grpId, 0, 0.9f);
    bgAnim = HuSprAnimMake(winW/16, winH/16, ANIM_BMP_IA4);
    HuSprBGSet(grpId, 0, bgAnim, 0);
    winP->bgPalNum = winBGMake(bgAnim, winP->animFrame[0]);
    if(frameFileTbl[(frame*2)+1] != HU_DATANUM_NONE) {
        data = HuAR_ARAMtoMRAMFileRead(frameFileTbl[(frame*2)+1], HU_MEMNUM_OVL, HEAP_MODEL);
        winP->animFrame[1] = HuSprAnimRead(data);
        sprId = winP->sprId[1] = HuSprCreate(winP->animFrame[1], winPrio, 0);
        HuSprGrpMemberSet(grpId, 1, sprId);
        HuSprBGSet(grpId, 1, bgAnim, 0);
        HuWinBGColSet(winId, &bgColor);
    } else {
        winP->animFrame[1] = NULL;
    }
    sprId = winP->sprId[2] = HuSprFuncCreate(MesDispFunc, winPrio);
    {
        HUSPRITE *sprP = &HuSprData[sprId];
        sprP->work[0] = winId;
    }
    HuSprGrpMemberSet(grpId, 2, sprId);
    winP->charEntryNum = 0;
    winP->charEntryMax = (winW/20)*(winH/24)*5;
    winP->charEntry = HuMemDirectMalloc(HEAP_HEAP, sizeof(WINCHARENTRY)*winP->charEntryMax);
    winP->attr = HUWIN_ATTR_NONE;
    winP->stat = HUWIN_STAT_NONE;
    winP->unk94 = 0;
    winP->mesTime = 0;
    winP->mesX = winP->mesY = 0;
    winP->mesRectX = 8;
    winP->mesRectY = 8;
    winP->mesRectW = winW-8;
    winP->mesRectH = winH-8;
    winP->mesSpeed = 1;
    winP->mesCol = HUWIN_MESCOL_WHITE;
    winP->mesColShadow = HUWIN_MESCOL_BLACK;
    winP->padMask = HUWIN_PLAYER_ALL;
    winP->disablePlayer = HUWIN_PLAYER_NONE;
    winP->messSp = 0;
    winP->messData = NULL;
    winP->choiceNum = 0;
    winP->prio = winPrio;
    winP->charPadX = 1;
    winP->charPadY = 2;
    winP->winW = winW;
    winP->winH = winH;
    winP->scissorX = winP->scissorY = 0;
    winP->scissorW = HU_FB_WIDTH;
    winP->scissorH = HU_FB_HEIGHT;
    winP->tabW = 24;
    winP->pushKey = PAD_BUTTON_A|PAD_BUTTON_B;
    winP->choiceEndSe = MSM_SE_CMN_03;
    winP->drawNo = 0;
    if(frame == 0 || frame == 2) {
        memcpy(&winP->mesPal[0][0], &charColPal[1][0][0], HUWIN_MESCOL_MAX*3);
    } else {
        memcpy(&winP->mesPal[0][0], &charColPal[0][0][0], HUWIN_MESCOL_MAX*3);
        winP->mesColShadow = HUWIN_MESCOL_LIGHTGRAY;
    }
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        winP->messDataInsert[i] = NULL;
    }
    for(i=HUWIN_SPR_BEGIN; i<HUWIN_SPR_MAX; i++) {
        winP->sprId[i] = HUSPR_NONE;
    }
    winPrio -= 3;
    if(winPrio < 500) {
        winPrio = 1000;
    }
    for(i=0; i<HUWIN_CHOICE_MAX; i++) {
        winP->choiceDisable[i] = FALSE;
        winP->choiceData[i].stat = 0;
    }
    return winId;
}

void HuWinKill(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    
    if(winP->grpId == HUSPR_GRP_NONE) {
        return;
    }
    HuMemDirectFree(winP->charEntry);
    for(i=HUWIN_SPR_BEGIN; i<HUWIN_SPR_MAX; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprGrpMemberKill(winP->grpId, i);
        }
    }
    HuSprGrpKill(winP->grpId);
    winP->grpId = HUSPR_GRP_NONE;
}

void HuWinAllKill(void)
{
    HUWINID i;
    for(i=0; i<HUWIN_MAX; i++) {
        if(winData[i].grpId != HUSPR_GRP_NONE) {
            HuWinKill(i);
        }
    }
    if(fontAnim) {
        HuSprAnimKill(fontAnim);
        fontAnim = NULL;
    }
    if(iconAnim) {
        HuSprAnimKill(iconAnim);
        iconAnim = NULL;
    }
    if(cursorAnim) {
        HuSprAnimKill(cursorAnim);
        cursorAnim = NULL;
    }
    if(cardAnimA) {
        HuSprAnimKill(cardAnimA);
        cardAnimA = NULL;
    }
    if(cardAnimB) {
        HuSprAnimKill(cardAnimB);
        cardAnimB = NULL;
    }
    if(winProc) {
        HuPrcKill(winProc);
        winProc = NULL;
    }
    if(messDataPtr) {
        HuMemDirectFree(messDataPtr);
        messDataPtr = NULL;
    }
    HuDataDirClose(DATA_win);
}

char *HuWinMesPtrGet(u32 messNum)
{
    if(!messDataPtr) {
        OSReport("Error: Not Read Mess Data.");
        return NULL;
    }
    if(newMessDataF) {
        return HuWinMesDataPtrGet(messDataPtr, messNum);
    } else {
        return MessData_MesPtrGet(messDataPtr, messNum);
    }
}

u32 HuWinMesMaxDirGet(void)
{
    u32 *dirNum = messDataPtr;
    return *dirNum;
}

u32 HuWinMesMaxNumGet(u32 dirNum)
{
    u32 dir = dirNum >> 16;
    u32 *messData = messDataPtr;
    if(dir >= *messData) {
        OSReport("Error: Message Dir Over\n");
    }
    messData++;
    messData += messData[dir]/sizeof(u32);
    return *messData;
}

void *HuWinMesDataPtrGet(void *data, u32 messNum)
{
    u32 dir = messNum >> 16;
    u32 num = messNum & 0xFFFF;
    u32 *messData = data;
    if(dir >= *messData) {
        OSReport("Error: Message Dir Over\n");
    }
    messData++;
    messData += messData[dir]/sizeof(u32);
    if(num >= *messData) {
        OSReport("Error: Message Number Over\n");
    }
    messData++;
    messData += messData[num]/sizeof(u32);
    messData++;
    return messData;
}

static void MesDispFunc(HUSPRITE *sprP)
{
    HUWIN *winP = &winData[sprP->work[0]];
    HUSPRGRP *sprGrpP;
    float uvMaxX;
    float uvMaxY;
    float uvMinX;
    float uvMinY;
    float charW;
    float charX;
    float charY;
    float charUvH;
    s16 i;
    u16 alpha;
    s16 color;
    Mtx scale;
    Mtx modelview;
    Mtx44 proj;

    if(winP->charEntryNum) {
        sprGrpP = &HuSprGrpData[winP->grpId];
        GXInvalidateTexAll();
        C_MTXOrtho(proj, 0.0f, HU_DISP_HEIGHT, 0.0f, HU_DISP_WIDTH, 0.0f, 10.0f);
        GXSetProjection(proj, GX_ORTHOGRAPHIC);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_RGBA6, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGBA8, 0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(1);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, 1, GX_DF_CLAMP, GX_AF_SPOT);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(0);
        MTXRotRad(modelview, 'z', MTXDegToRad(sprP->zRot));
        MTXScale(scale, sprP->scale.x, sprP->scale.y, 1.0f);
        MTXConcat(modelview, scale, modelview);
        mtxTransCat(modelview, sprP->pos.x, sprP->pos.y, 0.0f);
        MTXConcat(*sprP->groupMtx, modelview, modelview);
        GXLoadPosMtxImm(modelview, 0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        HuSprTexLoad(fontAnim, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
        GXBegin(GX_QUADS, GX_VTXFMT0, winP->charEntryNum*4);
        charUvH = (LanguageNo == HUWIN_LANG_JAPAN) ? (24.0f/408.0f) : (24.0f/312.0f);
        for(i=0; i<winP->charEntryNum; i++) {
            charW = fontWidthP[winP->charEntry[i].charNo+48];
            uvMinX = (20.0f/320.0f) * (winP->charEntry[i].charNo%16);
            uvMinY = charUvH * (winP->charEntry[i].charNo/16);
            charW--;
            uvMaxX = uvMinX + (charW/320.0);
            uvMaxY = uvMinY + charUvH;
            charX = winP->charEntry[i].x;
            charY = winP->charEntry[i].y;
            color = winP->charEntry[i].color;
            if(winP->mesSpeed != 0) {
                alpha = winP->charEntry[i].fade * 8;
            } else {
                alpha = 255;
            }
            alpha = 255;
            if(winP->charEntry[i].fade < 31) {
                winP->charEntry[i].fade++;
            }
            GXPosition3f32(charX + 1.0f, charY, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2], alpha);
            GXPosition2f32(uvMinX, uvMinY);
            GXPosition3f32(charX + charW, charY, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2], alpha);
            GXPosition2f32(uvMaxX, uvMinY);
            GXPosition3f32(charX + charW, charY + 24.0f, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2], alpha);
            GXPosition2f32(uvMaxX, uvMaxY);
            GXPosition3f32(charX + 1.0f, charY + 24.0f, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2], alpha);
            GXPosition2f32(uvMinX, uvMaxY);
        }
        GXEnd();
        mesCharCnt++;
    }
}

static u8 winBGMake(ANIMDATA *bgAnim, ANIMDATA *frameAnim)
{
    ANIMBMP *bmp;
    s16 blockH;
    s16 h;
    s16 blockW;
    s16 w;
    s16 i;
    s16 j;
    u8 *bmpData;

    w = bgAnim->bmp->sizeX;
    h = bgAnim->bmp->sizeY;
    blockW = (w+7) & 0xF8;
    blockH = (h+3) & 0xFC;
    bmpData = bgAnim->bmp->data = HuMemDirectMallocNum(HEAP_HEAP, blockW*blockH, HU_MEMNUM_OVL);
    for(i=0; i<h; i++) {
        if(i == 0) {
            for(j=0; j<w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x10;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x70;
                }
            }
        } else if(i == h-1) {
            for(j = 0; j < w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x20;
                } else if(j == w - 1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x30;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x60;
                }
            }
        } else {
            for(j = 0; j < w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x40;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x50;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0x80;
                }
            }
        }
    }
    DCStoreRangeNoSync(bgAnim->bmp->data, blockW*blockH);
    return w;
}

static void HuWinProc(void)
{
    HUWIN *winP;
    HUWINID winId;

    while(1) {
        HuPrcVSleep();
        winP = winData;
        for(winId=0; winId<HUWIN_MAX; winId++, winP++) {
            if(winP->grpId != HUSPR_GRP_NONE && !(winP->attr & HUWIN_ATTR_DISPOFF)) {
                switch(winP->stat) {
                    case HUWIN_STAT_NONE:
                        break;
                        
                    case HUWIN_STAT_DRAWMES:
                        HuWinDrawMes(winId);
                        break;
                        
                    case HUWIN_STAT_KEYWAIT:
                        HuWinComKeyGet(winId, winKey);
                        HuWinKeyWait(winId);
                        break;
                        
                    case HUWIN_STAT_CHOICE:
                        if(!(winP->attr & HUWIN_ATTR_CHOICEON) || HuWinActiveKeyGetX(winP) == 0) {
                            winP->attr &= ~HUWIN_ATTR_CHOICEON;
                            HuWinComKeyGet(winId, winKey);
                            HuWinChoice(winP);
                        }
                        break;
                }
            }
        }
    }
}

static void charEntry(s16 window, s16 x, s16 y, s16 charNo, s16 color)
{
    HUWIN *winP = &winData[window];
    WINCHARENTRY *winCharP = winP->charEntry;

    winCharP = &winP->charEntry[winP->charEntryNum];
    winCharP->x = x - winP->winW / 2;
    winCharP->y = y - winP->winH / 2;
    winCharP->charNo = charNo-48;
    winCharP->color = color;
    winCharP->fade = 0;
    winP->charEntryNum++;
    if(winP->charEntryNum >= winP->charEntryMax) {
        winP->charEntryNum = winP->charEntryMax-1;
    }
}

static void HuWinDrawMes(HUWINID winId) {
    HUWIN *winP = &winData[winId];
    HUSPRGRP *sprGrpP = &HuSprGrpData[winP->grpId];
    s16 c;
    s16 i;
    s16 charW;
    s16 tabW;
    s16 insertMesNo;
    s16 endF;
    s16 shadowColor;
    s16 color;
    s16 messW;

    winP->mesTime += 3;
    while(winP->mesTime >= winP->mesSpeed || (winP->attr & HUWIN_ATTR_CHOICE)) {
        winP->mesTime = 0;
        charW = winP->charPadX + fontWidthP[winP->messData[0]];
        endF = FALSE;
        if(winP->messData[0] != 0 && (winP->attr & HUWIN_ATTR_KEYWAIT_MESS)) {
            winP->attr &= ~HUWIN_ATTR_KEYWAIT_MESS;
            _HuWinHomeClear(winP);
            if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
                messW = GetMesMaxSizeSub2(winP, winP->messData);
                winP->mesX = (winP->mesRectW - messW) / 2;
            } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
                messW = GetMesMaxSizeSub2(winP, winP->messData);
                winP->mesX = winP->mesRectW - messW;
            }
        }
        while(winP->messData[0] < 48 || winP->messData[0] == 255) {
            switch(winP->messData[0]) {
                case 0:
                    if(winP->messSp == 0) {
                        for(i=0; i<HUWIN_CHOICE_MAX; i++) {
                            winP->choiceDisable[i] = FALSE;
                        }
                        winP->stat = 0;
                        winP->attr &= ~HUWIN_ATTR_SETCOLOR;
                        return;
                    }
                    winP->messSp--;
                    winP->messData = winP->messDataStack[winP->messSp];
                    break;
                    
                case 31:
                    winP->messData++;
                    insertMesNo = winP->messData[0]-1;
                    if(winP->messDataInsert[insertMesNo] != 0) {
                        winP->messDataStack[winP->messSp] = winP->messData;
                        winP->messData = winP->messDataInsert[insertMesNo];
                        winP->messSp++;
                        if(winP->messData[0] != 11) {
                            winP->messData--;
                        }
                    }
                    break;
                    
                case 11:
                    winP->attr &= ~(HUWIN_ATTR_WHITESPACE|HUWIN_ATTR_CHOICE);
                    _HuWinHomeClear(winP);
                    if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
                        messW = GetMesMaxSizeSub2(winP, winP->messData+1);
                        winP->mesX = (winP->mesRectW-messW)/2;
                    } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
                        messW = GetMesMaxSizeSub2(winP, winP->messData+1);
                        winP->mesX = winP->mesRectW-messW;
                    }
                    break;
                    
                case 30:
                    winP->messData++;
                    if(!(winP->attr & HUWIN_ATTR_SETCOLOR)) {
                        winP->mesCol = winP->messData[0]-1;
                    }
                    break;
                    
                case 29:
                    winP->attr ^= HUWIN_ATTR_OUTLINE;
                    break;
                    
                case 10:
                    winP->attr &= ~(HUWIN_ATTR_TEXTDISABLE|HUWIN_ATTR_CHOICE);
                    if(winP->attr & HUWIN_ATTR_WHITESPACE) {
                        if(!(winP->attr & HUWIN_ATTR_NOCR)) {
                            if(HuWinCR(winP) != 0) {
                                winP->messData++;
                                HuWinKeyWaitEntry(winId);
                                winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                                return;
                            }
                            if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
                                messW = GetMesMaxSizeSub2(winP, winP->messData+1);
                                winP->mesX = (winP->mesRectW-messW)/2;
                            } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
                                messW = GetMesMaxSizeSub2(winP, winP->messData+1);
                                winP->mesX = winP->mesRectW-messW;
                            }
                            break;
                        }
                        charW = fontWidthP[16]+winP->charPadX;
                        /* fallthrough */
                case 16:
                case 27:
                case 32:
                        winP->attr |= HUWIN_ATTR_WHITESPACE;
                        if(winP->mesX+charW > winP->mesRectW) {
                            if(HuWinCR(winP) != 0) {
                                winP->messData++;
                                HuWinKeyWaitEntry(winId);
                                winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                                return;
                            }
                            break;
                        }
                        winP->mesX += charW;
                    }
                    break;
                    
                case 14:
                    winP->attr |= HUWIN_ATTR_WHITESPACE;
                    winP->messData++;
                    tabW = winP->charPadX+spcFontTbl[winP->messData[0]-1].w;
                    if(winP->mesX+tabW > winP->mesRectW && HuWinCR(winP)) {
                        winP->messData--;
                        HuWinKeyWaitEntry(winId);
                        winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                        return;
                    }
                    HuWinSpcFontEntry(winP, winP->messData[0]-1, winP->mesRectX+winP->mesX, winP->mesRectY+winP->mesY);
                    winP->mesX += tabW;
                    endF = TRUE;
                    break;
                    
                case 28:
                    winP->messData++;
                    HuAudFXPlay(speakerSeIdTbl[winP->messData[0]-1]);
                    break;
                    
                case 255:
                    winP->messData++;
                    HuWinKeyWaitEntry(winId);
                    winP->attr |= HUWIN_ATTR_KEYWAIT_MESS;
                    winP->attr &= ~HUWIN_ATTR_WHITESPACE;
                    return;
                    
                case 13:
                    winP->choice = winP->choiceNum;

                case 15:
                    winP->attr |= HUWIN_ATTR_CHOICE;
                    if(winP->choiceDisable[winP->choiceNum]) {
                        winP->attr |= HUWIN_ATTR_TEXTDISABLE;
                        winP->choiceData[winP->choiceNum].stat |= 1;
                    } else {
                        winP->choiceData[winP->choiceNum].stat &= ~1;
                    }

                    winP->choiceData[winP->choiceNum].x = winP->mesX + winP->mesRectY;
                    winP->choiceData[winP->choiceNum].y = winP->mesY + winP->mesRectX;
                    winP->choiceNum++;
                    break;
                    
                case 12:
                    winP->attr |= HUWIN_ATTR_WHITESPACE;
                    tabW = winP->tabW*((winP->mesX+winP->tabW)/winP->tabW)-winP->mesX;
                    if(winP->mesX+tabW > winP->mesRectW) {
                        if(HuWinCR(winP)) {
                            winP->messData++;
                            HuWinKeyWaitEntry(winId);
                            winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                            return;
                        }
                    } else {
                        winP->mesX += tabW;
                    }
                    break;
            }
            winP->messData++;
            charW = winP->charPadX+fontWidthP[winP->messData[0]];
            if(endF) {
                break;
            }
        }
        if(endF == FALSE) {
            if(winP->mesX+charW > winP->mesRectW && HuWinCR(winP)) {
                HuWinKeyWaitEntry(winId);
                winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                return;
            }
            c = winP->messData[0];
            winP->attr |= HUWIN_ATTR_WHITESPACE;
            if(winP->messData[1] == 128) {
                if(c >= 150 && c <= 164) {
                    c = c+106;
                } else if(c >= 170 && c <= 174) {
                    c = c+101;
                } else if(c >= 214 && c <= 228) {
                    c = c+67;
                } else if(c >= 234 && c <= 238) {
                    c = c+62;
                }
                winP->messData++;
            } else if(winP->messData[1] == 129) {
                if(c >= 170 && c <= 174) {
                    c = c+106;
                } else if(c >= 234 && c <= 238) {
                    c = c+67;
                }
                winP->messData++;
            }
            color = (winP->attr & HUWIN_ATTR_TEXTDISABLE) ? HUWIN_MESCOL_DARKGRAY : winP->mesCol;
            if(winP->attr & HUWIN_ATTR_OUTLINE) {
                shadowColor = HUWIN_MESCOL_BLACK;
                charEntry(winId, winP->mesRectX+winP->mesX+2, winP->mesRectY+winP->mesY, c, shadowColor);
                charEntry(winId, winP->mesRectX+winP->mesX-2, winP->mesRectY+winP->mesY, c, shadowColor);
                charEntry(winId, winP->mesRectX+winP->mesX, winP->mesRectY+winP->mesY+2, c, shadowColor);
                charEntry(winId, winP->mesRectX+winP->mesX, winP->mesRectY+winP->mesY-2, c, shadowColor);
                charEntry(winId, winP->mesRectX+winP->mesX, winP->mesRectY+winP->mesY, c, color);
            } else {
                charEntry(winId, winP->mesRectX+winP->mesX+2, winP->mesRectY+winP->mesY+2, c, winP->mesColShadow);
                charEntry(winId, winP->mesRectX+winP->mesX, winP->mesRectY+winP->mesY, c, color);
            }
            winP->mesX += charW;
            winP->messData++;
        }
    }
}

static BOOL HuWinCR(HUWIN *winP)
{
    if(winP->mesY+48 > winP->mesRectH) {
        winP->mesY = 0;
        winP->mesX = 0;
        return TRUE;
    } else {
        winP->mesY += winP->charPadY+24;
        winP->mesX = 0;
        return FALSE;
    }
}

static void _HuWinHomeClear(HUWIN *winP)
{
    s16 i;

    winP->charEntryNum = 0;
    winP->mesY = winP->mesX = 0;
    winP->choiceNum = 0;
    HuWinSpcFontClear(winP);
    winP->attr &= ~(HUWIN_ATTR_TEXTDISABLE|HUWIN_ATTR_CHOICE);
    for(i=0; i<HUWIN_CHOICE_MAX; i++) {
        winP->choiceData[i].stat = 0;
    }
}

void HuWinHomeClear(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    _HuWinHomeClear(winP);
    for(i=0; i<HUWIN_CHOICE_MAX; i++) {
        winP->choiceDisable[i] = FALSE;
    }
    winP->stat = HUWIN_STAT_NONE;
    winP->attr &= ~(HUWIN_ATTR_SETCOLOR|HUWIN_ATTR_OUTLINE);
    winP->messSp = 0;
}

void HuWinKeyWaitEntry(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    if(winP->attr & HUWIN_ATTR_SKIP_KEYWAIT) {
        winP->stat = HUWIN_STAT_NONE;
    } else {
        winP->stat = HUWIN_STAT_KEYWAIT;
        winP->keyWaitSprNo = HuWinSpcFontEntry(winP, 19, winP->mesRectW-20, winP->mesRectH-24);
    }
}

static void HuWinKeyWait(HUWINID winId)
{
    HUWIN *winP = &winData[winId];

    if(winP->pushKey & HuWinActivePadGet(winP)) {
        winP->activePadKey = HuWinActivePadGet(winP);
        winP->stat = HUWIN_STAT_DRAWMES;
        HuAudFXPlay(MSM_SE_CMN_25);
        HuWinSprKill(winId, winP->keyWaitSprNo);
        if(winP->attr & HUWIN_ATTR_KEYWAIT_CLEAR) {
            _HuWinHomeClear(winP);
        }
        winP->attr &= ~HUWIN_ATTR_KEYWAIT_CLEAR;
    }
}

static s16 HuWinSpcFontEntry(HUWIN *winP, s16 entry, s16 x, s16 y)
{
    HUSPRGRP *sprGrpP = &HuSprGrpData[winP->grpId];
    HUSPRID sprId;
    s16 i;
    ANIMDATA *anim;

    for(i=HUWIN_SPCFONT_BEGIN; i<HUWIN_SPCFONT_END; i++) {
        if(winP->sprId[i] == HUSPR_NONE) {
            anim = *spcFontTbl[entry].animP;
            winP->sprId[i] = sprId = HuSprCreate(anim, winP->prio-1, spcFontTbl[entry].animBank);
            HuSprGrpMemberSet(winP->grpId, i, sprId);
            HuSprPosSet(winP->grpId, i, (x+spcFontTbl[entry].centerX)-(winP->winW/2), (y+spcFontTbl[entry].centerY)-(winP->winH/2));
            break;
        }
    }
    HuSprGrpDrawNoSet(winP->grpId, winP->drawNo);
    return i;
}

static void HuWinSpcFontPosSet(HUWIN *winP, s16 sprNo, s16 x, s16 y)
{
    HUSPRGRP *sprGrpP = &HuSprGrpData[winP->grpId];
    HuSprPosSet(winP->grpId, sprNo, x-(winP->winW/2), y-(winP->winH/2));
}

static void HuWinSpcFontClear(HUWIN *winP)
{
    s16 i;

    for(i=HUWIN_SPCFONT_BEGIN; i<HUWIN_SPCFONT_END; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprGrpMemberKill(winP->grpId, i);
            winP->sprId[i] = HUSPR_NONE;
        }
    }
}

#define WIN_CHOICEDIR_NONE -1
#define WIN_CHOICEDIR_LEFT 0
#define WIN_CHOICEDIR_UP 1
#define WIN_CHOICEDIR_RIGHT 2
#define WIN_CHOICEDIR_DOWN 3
#define WIN_CHOICE_DIST_INVALID 100000.0f

static void HuWinChoice(HUWIN *winP) {
    WINCHOICE *choiceP;
    float choiceDist;
    float minChoiceDist;
    s16 choiceDistY;
    s16 key;
    s16 choiceCurrX;
    s16 choiceCurrY;
    s16 choiceCurr;
    s16 choiceNum;
    s16 choiceDistX;
    s16 dir;
    s16 choiceY;
    s16 choice;
    s16 i;

    choiceCurr = winP->choice;
    choice = choiceCurr;
    dir = WIN_CHOICEDIR_NONE;
    key = HuWinActivePadGet(winP);
    if(key & PAD_BUTTON_LEFT) {
        dir = WIN_CHOICEDIR_LEFT;
    }
    if(key & PAD_BUTTON_RIGHT) {
        dir = WIN_CHOICEDIR_RIGHT;
    }
    if(key & PAD_BUTTON_UP) {
        dir = WIN_CHOICEDIR_UP;
    }
    if(key & PAD_BUTTON_DOWN) {
        dir = WIN_CHOICEDIR_DOWN;
    }
    choiceCurrX = winP->choiceData[choiceCurr].x;
    choiceCurrY = winP->choiceData[choiceCurr].y;
    minChoiceDist = WIN_CHOICE_DIST_INVALID;
    choiceDist = minChoiceDist;
    choiceNum = winP->choiceNum;
    switch(dir) {
        case WIN_CHOICEDIR_LEFT:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY && choiceP->x < choiceCurrX) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY && choiceP->x < choiceCurrX && choiceDist > choiceCurrX - choiceP->x) {
                        choiceDist = choiceCurrX - choiceP->x;
                        choice = i;
                    }
                }
            }
            break;
        case WIN_CHOICEDIR_UP:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y < choiceCurrY) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y < choiceCurrY) {
                        choiceDistY = choiceCurrY - choiceP->y;
                        if(choiceDistY <= choiceDist) {
                            if(choiceDistY < choiceDist) {
                                minChoiceDist = WIN_CHOICE_DIST_INVALID;
                            }
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                choiceDist = choiceDistY;
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            } else {
                choiceY = -1000;
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y > choiceY) {
                        choiceY = choiceP->y;
                    }
                }
                if(choiceY != choiceCurrY) {
                    minChoiceDist = WIN_CHOICE_DIST_INVALID;
                    for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                        if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceY) {
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            }
            break;
            
        case WIN_CHOICEDIR_RIGHT:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY && choiceP->x > choiceCurrX) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY && choiceP->x > choiceCurrX && choiceDist > choiceP->x - choiceCurrX) {
                        choiceDist = choiceP->x - choiceCurrX;
                        choice = i;
                    }
                }
            }
            break;
            
        case WIN_CHOICEDIR_DOWN:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y > choiceCurrY) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y > choiceCurrY) {
                        choiceDistY = choiceP->y - choiceCurrY;
                        if(choiceDistY <= choiceDist) {
                            if(choiceDistY < choiceDist) {
                                minChoiceDist = WIN_CHOICE_DIST_INVALID;
                            }
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                choiceDist = choiceDistY;
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            } else {
                choiceY = 1000;
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y < choiceY) {
                        choiceY = choiceP->y;
                    }
                }
                if(choiceY != choiceCurrY) {
                    minChoiceDist = WIN_CHOICE_DIST_INVALID;
                    for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                        if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceY) {
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            }
            break;
    }
    if(winP->choice != choice) {
        winP->choice = choice;
        HuAudFXPlay(0);
    } else if(key & PAD_BUTTON_A) {
        if(winP->choiceEndSe >= 0) {
            HuAudFXPlay(winP->choiceEndSe);
        }
        winP->activePadKey = key;
        winP->stat = 0;
    } else if((key & PAD_BUTTON_B) && !(winP->attr & HUWIN_ATTR_NOCANCEL)) {
        HuAudFXPlay(3);
        (void)key;
        winP->choice = -1;
        winP->stat = 0;
    }
    HuWinSpcFontPosSet(winP, winP->cursorSprNo, winP->choiceData[choice].x+spcFontTbl[20].centerX, winP->choiceData[choice].y+spcFontTbl[20].centerY);
}

#undef WIN_CHOICEDIR_NONE
#undef WIN_CHOICEDIR_LEFT
#undef WIN_CHOICEDIR_UP
#undef WIN_CHOICEDIR_RIGHT
#undef WIN_CHOICEDIR_DOWN
#undef WIN_CHOICE_DIST_INVALID 

u32 HuWinActivePadGet(HUWIN *winP)
{
    s32 key;
    u32 i;

    key = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(winP->padMask & (1 << i)) {
            key |= winKey[i];
        }
    }
    return key;
}

u32 HuWinActiveKeyGetX(HUWIN *winP)
{
    u32 btn;
    u32 i;
    u32 j;
    u8 padMask;

    btn = 0;
    padMask = (winP->padMask & ~winP->disablePlayer);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(padMask & (1 << i)) {
            for(j=0; j<GW_PLAYER_MAX; j++) {
                if(i == GwPlayerConf[j].padNo) {
                    break;
                }
            }
            if(j != GW_PLAYER_MAX && GwPlayerConf[j].type == GW_TYPE_MAN) {
                btn |= HuPadBtn[GwPlayerConf[j].padNo];
            }
        }
    }
    return btn;
}

void HuWinPosSet(HUWINID winId, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    if(posX == HUWIN_POS_CENTER) {
        winP->pos.x = (HU_DISP_WIDTH-winP->winW)/2;
    } else {
        winP->pos.x = posX;
    }
    if(posY == HUWIN_POS_CENTER) {
        winP->pos.y = (HU_DISP_HEIGHT-winP->winH)/2;
    } else {
        winP->pos.y = posY;
    }
    HuSprGrpPosSet(winP->grpId, winP->pos.x, winP->pos.y);
}

void HuWinScaleSet(HUWINID winId, float scaleX, float scaleY)
{
    HUWIN *winP = &winData[winId];
    winP->scale.x = scaleX;
    winP->scale.y = scaleY;
    HuSprGrpScaleSet(winP->grpId, scaleX, scaleY);
}

void HuWinZRotSet(HUWINID winId, float zRot)
{
    HUWIN *winP = &winData[winId];
    winP->zRot = zRot;
    HuSprGrpZRotSet(winP->grpId, zRot);
}

void HuWinCenterPosSet(HUWINID winId, float centerX, float centerY)
{
    HUWIN *winP = &winData[winId];
    HuSprGrpCenterSet(winP->grpId, (winP->winW/2.0f)-centerX, (winP->winH/2.0f)-centerY);
}

void HuWinDrawNoSet(HUWINID winId, s16 drawNo)
{
    HUWIN *winP = &winData[winId];
    winP->drawNo = drawNo;
    HuSprGrpDrawNoSet(winP->grpId, winP->drawNo);
}

void HuWinScissorSet(HUWINID winId, s16 x, s16 y, s16 w, s16 h)
{
    HUWIN *winP = &winData[winId];
    HuSprGrpScissorSet(winP->grpId, x, y, w, h);
}

void HuWinPriSet(HUWINID winId, s16 prio)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    HuSprPriSet(winP->grpId, 0, prio);
    HuSprPriSet(winP->grpId, 1, prio);
    HuSprPriSet(winP->grpId, 2, prio);
    for(i=HUWIN_SPR_BEGIN; i<HUWIN_SPR_MAX; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprPriSet(winP->grpId, i, prio-1);
        }
    }
    winP->prio = prio;
}

void HuWinAttrSet(HUWINID winId, u32 attr)
{
    HUWIN *winP = &winData[winId];
    winP->attr |= attr;
}

void HuWinAttrReset(HUWINID winId, u32 attr)
{
    HUWIN *winP = &winData[winId];
    winP->attr &= ~attr;
}

s16 HuWinStatGet(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    return winP->stat;
}

void HuWinMesColSet(HUWINID winId, u8 mesCol)
{
    HUWIN *winP = &winData[winId];
    winP->mesCol = mesCol;
    winP->attr |= HUWIN_ATTR_SETCOLOR;
}

void HuWinMesPalSet(HUWINID winId, u8 mesCol, u8 r, u8 g, u8 b)
{
    HUWIN *winP = &winData[winId];
    winP->mesPal[mesCol][0] = r;
    winP->mesPal[mesCol][1] = g;
    winP->mesPal[mesCol][2] = b;
}

void HuWinBGTPLvlSet(HUWINID winId, float tpLvl)
{
    HUWIN *winP = &winData[winId];
    HuSprTPLvlSet(winP->grpId, 0, tpLvl);
    HuSprTPLvlSet(winP->grpId, 1, tpLvl);
}

void HuWinBGColSet(HUWINID winId, GXColor *bgCol)
{
    HUWIN *winP = &winData[winId];
    HuSprColorSet(winP->grpId, 0, bgCol->r, bgCol->g, bgCol->b);
}

void HuWinMesSpeedSet(HUWINID winId, s16 mesSpeed)
{
    HUWIN *winP = &winData[winId];
    winP->mesSpeed = mesSpeed;
}

void HuWinMesRead(void)
{
    void *buf;
    char *path;
    u8 dataNo;
    
    dataNo = (newMessDataF) ? 0 : 1;
    if(messDataPtr) {
        HuMemDirectFree(messDataPtr);
    }
    if(LanguageNo == HUWIN_LANG_JAPAN) {
        path = mesDataTbl[dataNo];
    } else {
        path = mesDataTbl[dataNo+2];
    }
    buf = HuDvdDataRead(path);
    messDataPtr = HuMemDirectMalloc(HEAP_HEAP, DirDataSize);
    memcpy(messDataPtr, buf, DirDataSize);
    HuMemDirectFree(buf);
}

void HuWinMesSet(HUWINID winId, u32 messNum)
{
    HUWIN *winP = &winData[winId];
    s16 messW;
    winP->stat = HUWIN_STAT_DRAWMES;
    if(!(messNum & 0x80000000)) {
        if(!messDataPtr) {
            OSReport("Error: No Message\n");
            return;
        }
        winP->messData = HuWinMesPtrGet(messNum);
        if(!winP->messData) {
            OSReport("Error: No Message data\n");
        }
    } else {
        winP->messData = (void *)messNum;
    }
    _HuWinHomeClear(winP);
    if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
        messW = GetMesMaxSizeSub2(winP, winP->messData);
        winP->mesX = (winP->mesRectW - messW) / 2;
    } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
        messW = GetMesMaxSizeSub2(winP, winP->messData);
        winP->mesX = winP->mesRectW - messW;
    }
    if(!(winP->attr & HUWIN_ATTR_SETCOLOR)) {
        winP->mesCol = HUWIN_MESCOL_WHITE;
    }
}

void HuWinInsertMesSet(HUWINID winId, u32 messNum, s16 insertMesNo)
{
    HUWIN *winP = &winData[winId];
    s16 messW;
    if(!(messNum & 0x80000000)) {
        if(!messDataPtr) {
            OSReport("Error: No Message\n");
            return;
        }
        winP->messDataInsert[insertMesNo] = HuWinMesPtrGet(messNum);
        if(!winP->messDataInsert[insertMesNo]) {
            OSReport("Error: No Message data\n");
        }
    } else {
        winP->messDataInsert[insertMesNo] = (void *)messNum;
    }
}

s16 HuWinChoiceSet(HUWINID winId, s16 choiceNo)
{
    HUWIN *winP = &winData[winId];
    winP->attr |= HUWIN_ATTR_CHOICEON;
    while(winP->stat != HUWIN_STAT_NONE) {
        HuPrcVSleep();
    }
    if(choiceNo != -1) {
        while(choiceNo < winP->choiceNum) {
            if(!(winP->choiceData[choiceNo].stat & 0x1)) {
                break;
            }
            choiceNo++;
        }
        if(choiceNo == winP->choiceNum) {
            for(choiceNo=0; choiceNo<winP->choiceNum; choiceNo++) {
                if(!(winP->choiceData[choiceNo].stat & 0x1)) {
                    break;
                }
            }
            if(choiceNo == winP->choiceNum) {
                return -1;
            }
        }
        winP->choice = choiceNo;
    } else {
        if(winP->choice >= winP->choiceNum) {
            return -1;
        }
    }
    winP->cursorSprNo = HuWinSpcFontEntry(winP, 20, winP->choiceData[winP->choice].x, winP->choiceData[winP->choice].y);
    winP->stat = HUWIN_STAT_CHOICE;
    return 0;
}

s16 HuWinChoiceGet(HUWINID winId, s16 choiceNo)
{
    HUWIN *winP = &winData[winId];
    s16 result = HuWinChoiceSet(winId, choiceNo);
    if(result != 0) {
        return result;
    }
    while(winP->stat != HUWIN_STAT_NONE) {
        HuPrcVSleep();
    }
    return winP->choice;
}

s16 HuWinChoiceNumGet(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    return winP->choiceNum;
}

void HuWinChoiceDisable(HUWINID winId, s16 choiceNo)
{
    HUWIN *winP = &winData[winId];
    winP->choiceDisable[choiceNo] = TRUE;
}

s16 HuWinChoiceNowGet(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    if(winP->stat == HUWIN_STAT_CHOICE) {
        return winP->choice;
    } else {
        return -1;
    }
}

void HuWinMesWait(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    while(winP->stat != HUWIN_STAT_NONE) {
        HuPrcVSleep();
    }
}

s16 HuWinAnimSet(HUWINID winId, ANIMDATA *anim, s16 animBank, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    HUSPRID sprId = HuSprCreate(anim, winP->prio-1, animBank);
    return HuWinSprSet(winId, sprId, posX, posY);
}

s16 HuWinSprSet(HUWINID winId, HUSPRID sprId, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    HUSPRGRP *sprGrpP = &HuSprGrpData[winP->grpId];
    s16 i;
    for(i=HUWIN_SPR_BEGIN; i<=HUWIN_SPR_END; i++){ 
        if(winP->sprId[i] == HUSPR_NONE) {
            winP->sprId[i] = sprId;
            HuSprGrpMemberSet(winP->grpId, i, sprId);
            HuSprPosSet(winP->grpId, i, posX-sprGrpP->center.x, posY-sprGrpP->center.y);
            break;
        }
    }
    return i;
}

void HuWinSprPosSet(HUWINID winId, s16 sprNo, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    HUSPRGRP *sprGrpP = &HuSprGrpData[winP->grpId];
    HuSprPosSet(winP->grpId, sprNo, posX-sprGrpP->center.x, posY-sprGrpP->center.y);

}

void HuWinSprPriSet(HUWINID winId, s16 sprNo, s16 prio)
{
    HUWIN *winP = &winData[winId];
    HUSPRGRP *sprGrpP = &HuSprGrpData[winP->grpId];
    HuSprPriSet(winP->grpId, sprNo, prio);
}

HUSPRID HuWinSprIDGet(HUWINID winId, s16 sprNo)
{
    HUWIN *winP = &winData[winId];
    return winP->sprId[sprNo];
}

void HuWinSprKill(HUWINID winId, s16 sprNo)
{
    HUWIN *winP = &winData[winId];
    HuSprGrpMemberKill(winP->grpId, sprNo);
    winP->sprId[sprNo] = HUSPR_NONE;
}

void HuWinDispOff(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    for(i=0; i<HUWIN_SPR_MAX; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprDispOff(winP->grpId, i);
        }
    }
    winP->attr |= HUWIN_ATTR_DISPOFF;
}

void HuWinDispOn(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    for(i=0; i<HUWIN_SPR_MAX; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprDispOn(winP->grpId, i);
        }
    }
    winP->attr &= ~HUWIN_ATTR_DISPOFF;
}

void HuWinComKeyWait(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4, s16 time)
{
    _HuWinComKeySet((keyP1 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0, 
        (keyP2 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0,
        (keyP3 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0,
        (keyP4 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0,
        time);
    _HuWinComKeySet(keyP1, keyP2, keyP3, keyP4, 1);
}

void HuWinComKeySet(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4)
{
    _HuWinComKeySet(keyP1, keyP2, keyP3, keyP4, 1);
}

void _HuWinComKeySet(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4, s16 time)
{
    winComKeyBuf[comKeyIdx].key[0] = keyP1;
    winComKeyBuf[comKeyIdx].key[1] = keyP2;
    winComKeyBuf[comKeyIdx].key[2] = keyP3;
    winComKeyBuf[comKeyIdx].key[3] = keyP4;
    winComKeyBuf[comKeyIdx].time = time;
    comKeyIdx++;
    comKeyIdx &= 0xFF;
}

void HuWinComKeyGet(HUWINID winId, u32 *key)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    if(comKeyIdx == comKeyIdxNow) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(!(winP->disablePlayer & (1 << i))) {
                key[i] = HuPadDStkRep[i]|HuPadBtnDown[i];
            } else {
                key[i] = 0;
            }
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            key[i] = winComKeyBuf[comKeyIdxNow].key[i];
            if(key[i] == HUWIN_COMKEY_NONE) {
                if(!(winP->disablePlayer & (1 << i))) {
                    key[i] = HuPadDStkRep[i]|HuPadBtnDown[i];
                } else {
                    key[i] = 0;
                }
            }
        }
        winComKeyBuf[comKeyIdxNow].time--;
        if(winComKeyBuf[comKeyIdxNow].time <= 0) {
            comKeyIdxNow++;
            comKeyIdxNow &= 0xFF;
        }
    }
}


void HuWinComKeyReset(void)
{
    comKeyIdx = comKeyIdxNow = 0;
}

void HuWinMesMaxSizeGet(s16 messCnt, HuVec2F *maxSize, ...)
{
    s16 i;
    va_list vaList;
    winInsertF = FALSE;
    va_start(vaList, maxSize);
    winMaxWidth = winMaxHeight = 0;
    for(i=0; i<messCnt; i++) {
        u32 messNum = va_arg(vaList, u32);
        GetMesMaxSizeSub(messNum);
    }
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        mesWInsert[i] = 0;
    }
    winTabSize = 24;
    cancelCRF = FALSE;
    maxSize->x = (winMaxWidth+31)&0xFFF0;
    maxSize->y = winMaxHeight+16;
    va_end(vaList);
}

void HuWinInsertMesSizeGet(u32 messNum, s16 insertMesNo)
{
    winInsertF = TRUE;
    winMaxWidth = winMaxHeight = 0;
    GetMesMaxSizeSub(messNum);
    mesWInsert[insertMesNo] = winMaxWidth;
}

void HuWinMesSizeCancelCRSet(s32 cancelCR)
{
    cancelCRF = cancelCR;
}

void HuWinMesMaxSizeBetGet(HuVec2F *maxSize, u32 messStart, u32 messEnd)
{
    u32 i;
    if(messEnd < messStart) {
        maxSize->x = maxSize->y = 100;
        return;
    }
    winInsertF = FALSE;
    winMaxWidth = winMaxHeight = 0;
    for(i=messStart; i<=messEnd; i++) {
        GetMesMaxSizeSub(i);
    }
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        mesWInsert[i] = 0;
    }
    winTabSize = 24;
    cancelCRF = FALSE;
    maxSize->x = (winMaxWidth+31)&0xFFF0;
    maxSize->y = winMaxHeight+16;
}

static void GetMesMaxSizeSub(u32 messNum)
{
    s16 winHeight;
    s16 winWidth;
    s16 charW;
    s16 charH;
    char *messData;
    char *messDataOrig = NULL;
    BOOL crF = FALSE;
    s16 messDataF;
    
    if(messNum > 0x80000000) {
        messDataF = FALSE;
        messData = (char *)messNum;
    } else {
        messDataF = TRUE;
        messData = messDataOrig = HuWinMesPtrGet(messNum);
    }
    winHeight = 26;
    winWidth = 0;
    while(messData[0] != 0) {
        charH = 0;
        charW = fontWidthP[messData[0]]+1;
        switch(messData[0]) {
            case 16:
            case 27:
            case 32:
                break;

            case 30:
                messData++;
                charW = 0;
                break;
                
            case 10:
                if(crF) {
                    if(cancelCRF == FALSE) {
                        if(winMaxWidth < winWidth) {
                            winMaxWidth = winWidth;
                        }
                        winWidth = charW = 0;
                        if(messData[1] != 0) {
                            charH = 26;
                        }
                    } else {
                        charW = fontWidthP[16] + 1;
                    }
                }
                break;
            case 255:
                winWidth += charW;
                /* fallthrough */
            case 11:
                crF = 0;
                if(winInsertF == FALSE) {
                    if(winMaxWidth < winWidth) {
                        winMaxWidth = winWidth;
                    }
                    if(winMaxHeight < winHeight) {
                        winMaxHeight = winHeight;
                    }
                    winHeight = 26;
                    charH = 0;
                    winWidth = charW = 0;
                }
                break;
                
            case 28:
                messData++;
                /* fallthrough */
            case 15:
            case 29:
                charW = 0;
                break;
                
            case 12:
                charW = winTabSize*((winWidth+winTabSize)/winTabSize)-winWidth;
                break;
                
            case 14:
                messData++;
                charW = spcFontTbl[messData[0]-1].w+1;
                break;
                
            case 31:
                messData++;
                charW = mesWInsert[messData[0]-1];
                break;
        }
        if((messData[0] != 255 && messData[0] >= 32) || messData[0] == 16) {
            crF = 1;
        }
        winWidth += charW;
        winHeight += charH;
        messData++;
    }
    if(winMaxWidth < winWidth) {
        winMaxWidth = winWidth;
    }
    if(winMaxHeight < winHeight) {
        winMaxHeight = winHeight;
    }
}

static s32 GetMesMaxSizeSub2(HUWIN *winP, void *data)
{
    s16 i;
    char *messDataOrig = NULL;
    u8 messEnd;
    s16 winHeight;
    s16 winWidth;
    s16 charW;
    s16 charH;
    char *messData;
    BOOL crF = FALSE;
    
    
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        if(winP->messDataInsert[i]) {
            HuWinInsertMesSizeGet((u32)winP->messDataInsert[i], i);
        }
    }
    messData = data;
    winHeight = 26;
    winWidth = 0;
    messEnd = FALSE;
    while(messData[0] != 0 && !messEnd) {
        charH = 0;
        charW = fontWidthP[messData[0]]+1;
        switch(messData[0]) {
            case 16:
            case 32:
                break;
                
            case 10:
                if(crF && !(winP->attr & 0x100)) {
                    charW = 0;
                    messEnd = TRUE;
                }
                break;
                
            case 255:
                charW = 0;
                messEnd = TRUE;
                break;
                
            case 11:
                crF = 0;
                winHeight = 26;
                charH = 0;
                winWidth = charW = 0;
                break;
                
            case 28:
            case 30:
                messData++;
                /* fallthrough */
            case 15:
            case 29:
                charW = 0;
                break;
                
            case 12:
                charW = winTabSize*((winWidth+winTabSize)/winTabSize)-winWidth;
                break;
                
            case 14:
                messData++;
                charW = spcFontTbl[messData[0]-1].w+1;
                break;
                
            case 31:
                messData++;
                charW = mesWInsert[messData[0]-1];
                break;
        }
        if((messData[0] != 255 && messData[0] >= 32) || messData[0] == 16) {
            crF = 1;
        }
        winWidth += charW;
        winHeight += charH;
        messData++;
    }
    return winWidth;
}

s16 HuWinKeyWaitNumGet(u32 messNum)
{
    s16 waitNum;
    char *messData;
    if(messNum > 0x80000000) {
        messData = (char *)messNum;
    } else {
        messData = HuWinMesPtrGet(messNum);
    }
    for(waitNum=0; messData[0]; messData++) {
        if(*messData == 255) {
            waitNum++;
        }
    }
    return waitNum;
}

void HuWinPushKeySet(HUWINID winId, s16 pushKey)
{
    HUWIN *winP = &winData[winId];
    winP->pushKey = pushKey;
}

void HuWinDisablePlayerSet(HUWINID winId, u8 playerBit)
{
    HUWIN *winP = &winData[winId];
    winP->disablePlayer |= playerBit;
}

void HuWinDisablePlayerReset(HUWINID winId, u8 playerBit)
{
    HUWIN *winP = &winData[winId];
    winP->disablePlayer &= ~playerBit;
}

static unsigned int speakerFileTbl[] = {
    WIN_ANM_talkChorl,
    WIN_ANM_talkNerl,
    WIN_ANM_talkTerl,
    WIN_ANM_talkNirl,
    WIN_ANM_talkHarl,
    WIN_ANM_talkMarl,
    WIN_ANM_talkDarl,
    WIN_ANM_talkKoopa,
    WIN_ANM_talkKuribo,
    WIN_ANM_talkHeyho,
    WIN_ANM_talkNoko2,
    WIN_ANM_talkJugem,
    WIN_ANM_talkWan2,
    WIN_ANM_talkUkki,
    WIN_ANM_talkPata2,
    WIN_ANM_talkHanachan,
    WIN_ANM_talkHanachan2,
    WIN_ANM_talkKamekku,
    WIN_ANM_talkKokamekku,
    WIN_ANM_talkChoro,
    WIN_ANM_talkCapMachine,
    WIN_ANM_talkPuku2,
    WIN_ANM_talkBattan,
    WIN_ANM_talkDossun
};

HUWINID HuWinExCreate(float x, float y, s16 w, s16 h, s16 speakerNo)
{
    return HuWinExCreateFrame(x, y, w, h, speakerNo, 0);
}

HUWINID HuWinExCreateFrame(float x, float y, s16 w, s16 h, s16 speakerNo, s16 frame)
{
    
    HUWINID winId;
    HUWIN *winP;
    if(speakerNo >= 0) {
        h = (h < 84) ? 84 : h;
    }
    winId = HuWinCreate(x, y, w, h, frame);
    winP = &winData[winId];
    if(frame == 1) {
        winP->mesColShadow = HUWIN_MESCOL_LIGHTGRAY;
    }
    if(speakerNo >= 0) {
        void *data = HuAR_ARAMtoMRAMFileRead(speakerFileTbl[speakerNo], HU_MEMNUM_OVL, HEAP_MODEL);
        ANIMDATA *anim = HuSprAnimRead(data);
        HuWinAnimSet(winId, anim, 0, 48, 48);
    }
    HuWinDispOff(winId);
    return winId;
}

void HuWinExOpen(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    _HuWinHomeClear(winP);
    if(winP->sprId[HUWIN_SPR_BEGIN] == HUSPR_NONE) {
        HuSprScaleSet(winP->grpId, 0, 1.0f, 0.0f);
        HuSprScaleSet(winP->grpId, 1, 1.0f, 0.0f);
        HuWinDispOn(winId);
        for(i=0; i<=10; i++) {
            HuSprScaleSet(winP->grpId, 0, 1.0f, 1.0-(1.1*HuCos(9.0f*i)));
            HuSprScaleSet(winP->grpId, 1, 1.0f, 1.0-(1.1*HuCos(9.0f*i)));
            HuPrcVSleep();
        }
        HuSprScaleSet(winP->grpId, 0, 1.0f, 1.0f);
        HuSprScaleSet(winP->grpId, 1, 1.0f, 1.0f);
    } else {
        HuSprScaleSet(winP->grpId, 0, 1.0f, 0.0f);
        HuSprScaleSet(winP->grpId, 1, 1.0f, 0.0f);
        HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, 0.0f);
        HuWinDispOn(winId);
        for(i=0; i<=20; i++) {
            if(i <= 10) {
                HuSprScaleSet(winP->grpId, 0, 1.0f, 1.1*(1.0-HuCos(9.0f*i)));
                HuSprScaleSet(winP->grpId, 1, 1.0f, 1.1*(1.0-HuCos(9.0f*i)));
            } else {
                HuSprScaleSet(winP->grpId, 0, 1.0f, 1.0f);
                HuSprScaleSet(winP->grpId, 1, 1.0f, 1.0f);
            }
            if(i > 10) {
                s16 time = i-10;
                HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, 1.0-(1.1*HuCos(9.0f*time)));
            }
            HuPrcVSleep();
        }
        HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, 1.0f);
    }
}

void HuWinExClose(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;

    _HuWinHomeClear(winP);
    if(winP->sprId[HUWIN_SPR_BEGIN] == HUSPR_NONE) {
        for(i=0; i<=10; i++) {
            HuSprScaleSet(winP->grpId, 0, 1.0f, HuCos(9.0f*i));
            HuSprScaleSet(winP->grpId, 1, 1.0f, HuCos(9.0f*i));
            HuPrcVSleep();
        }
        HuWinDispOff(winId);
    } else {
        for(i=0; i<=15; i++) {
            if(i <= 10) {
                HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, HuCos(9.0f*i));
            }
            if(i > 5) {
                s16 time = i-5;
                HuSprScaleSet(winP->grpId, 0, 1.0f, HuCos(9.0f*time));
                HuSprScaleSet(winP->grpId, 1, 1.0f, HuCos(9.0f*time));
            }
            HuPrcVSleep();
        }
        HuWinDispOff(winId);
    }
}

void HuWinExKill(HUWINID winId)
{
    HuWinKill(winId);
}

void HuWinExSpeakerSet(HUWINID winId, s16 speakerNo)
{
    HUWIN *winP = &winData[winId];
    s16 i;

    for(i=0; i<=10; i++) {
        HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, HuCos(9.0f*i));
        HuPrcVSleep();
    }
    HuWinSprKill(winId, HUWIN_SPR_BEGIN);
    {
        void *data = HuAR_ARAMtoMRAMFileRead(speakerFileTbl[speakerNo], HU_MEMNUM_OVL, HEAP_MODEL);
        ANIMDATA *anim = HuSprAnimRead(data);
        HuWinAnimSet(winId, anim, 0, 48, 48);
    }
    for(i=0; i<=10; i++) {
        HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, HuSin(9.0f*i));
        HuPrcVSleep();
    }
    HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, 1.0f);
}