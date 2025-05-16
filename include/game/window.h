#ifndef _GAME_WINDOW_H
#define _GAME_WINDOW_H

#include "game/animdata.h"
#include "game/sprite.h"

#include "dolphin.h"

#define MESSNUM_PTR(ptr) ((u32)ptr)

#define HUWIN_NONE -1
#define HUWIN_MAX 32

#define HUWIN_POS_CENTER -10000.0f

#define HUWIN_SPR_BEGIN 3
#define HUWIN_SPR_END 10
#define HUWIN_SPCFONT_BEGIN 11
#define HUWIN_SPCFONT_END 31

#define HUWIN_SPR_MAX HUWIN_SPCFONT_END

#define HUWIN_INSERTMES_MAX 16
#define HUWIN_CHOICE_MAX 16

#define HUWIN_CHOICE_NONE -1

#define HUWIN_COMKEY_NONE -1

#define HUWIN_PLAYER_NONE 0
#define HUWIN_PLAYER_1 (1 << 0)
#define HUWIN_PLAYER_2 (1 << 1)
#define HUWIN_PLAYER_3 (1 << 2)
#define HUWIN_PLAYER_4 (1 << 3)
#define HUWIN_PLAYER_ALL 0xF

#define HUWIN_ATTR_NONE 0
#define HUWIN_ATTR_OUTLINE (1 << 0)
#define HUWIN_ATTR_KEYWAIT_CLEAR (1 << 1)
#define HUWIN_ATTR_KEYWAIT_MESS (1 << 2)
#define HUWIN_ATTR_DISPOFF (1 << 3)
#define HUWIN_ATTR_NOCANCEL (1 << 4)
#define HUWIN_ATTR_TEXTDISABLE (1 << 5)
#define HUWIN_ATTR_CHOICEON (1 << 6)
#define HUWIN_ATTR_SETCOLOR (1 << 7)
#define HUWIN_ATTR_NOCR (1 << 8)
#define HUWIN_ATTR_WHITESPACE (1 << 9)
#define HUWIN_ATTR_SKIP_KEYWAIT (1 << 10)
#define HUWIN_ATTR_ALIGN_CENTER (1 << 11)
#define HUWIN_ATTR_ALIGN_RIGHT (1 << 12)
#define HUWIN_ATTR_CHOICE (1 << 13)

#define HUWIN_MESCOL_BLACK 0
#define HUWIN_MESCOL_BLUE 1
#define HUWIN_MESCOL_RED 2
#define HUWIN_MESCOL_MAGENTA 3
#define HUWIN_MESCOL_GREEN 4
#define HUWIN_MESCOL_CYAN 5
#define HUWIN_MESCOL_ORANGE 6
#define HUWIN_MESCOL_WHITE 7
#define HUWIN_MESCOL_DARKGRAY 8
#define HUWIN_MESCOL_LIGHTGRAY 9
#define HUWIN_MESCOL_MAX 10

#define HUWIN_STAT_NONE 0
#define HUWIN_STAT_DRAWMES 1
#define HUWIN_STAT_KEYWAIT 2
#define HUWIN_STAT_CHOICE 3

#define HUWIN_SPEAKER_CHORL 0
#define HUWIN_SPEAKER_NERL 1
#define HUWIN_SPEAKER_TERL 2
#define HUWIN_SPEAKER_NIRL 3
#define HUWIN_SPEAKER_HARL 4
#define HUWIN_SPEAKER_MARL 5
#define HUWIN_SPEAKER_DARL 6
#define HUWIN_SPEAKER_KOOPA 7
#define HUWIN_SPEAKER_KURIBO 8
#define HUWIN_SPEAKER_HEYHO 9
#define HUWIN_SPEAKER_NOKO2 10
#define HUWIN_SPEAKER_JUGEM 11
#define HUWIN_SPEAKER_WAN2 12
#define HUWIN_SPEAKER_UKKI 13
#define HUWIN_SPEAKER_PATA2 14
#define HUWIN_SPEAKER_HANACHAN 15
#define HUWIN_SPEAKER_HANACHAN2 16
#define HUWIN_SPEAKER_KAMEKKU 17
#define HUWIN_SPEAKER_KOKAMEKKU 18
#define HUWIN_SPEAKER_CHORO 19
#define HUWIN_SPEAKER_CAPMACHINE 20
#define HUWIN_SPEAKER_PUKU2 21
#define HUWIN_SPEAKER_BATTAN 22
#define HUWIN_SPEAKER_DOSSUN 23
#define HUWIN_SPEAKER_NONE -1


typedef s16 HUWINID;

typedef struct WinCharEntry_s {
    u8 color;
    u8 fade;
    s16 x;
    s16 y;
    s16 charNo;
} WINCHARENTRY;

typedef struct WinChoice_s {
    u8 stat;
    s16 x;
    s16 y;
} WINCHOICE;

typedef struct Win_s {
    u8 stat;
    u8 padMask;
    u8 disablePlayer;
    u8 bgPalNum;
    HUSPRGRPID grpId;
    HUSPRID sprId[HUWIN_SPR_MAX];
    s16 mesSpeed;
    s16 mesTime;
    s16 keyWaitSprNo;
    s16 prio;
    s16 drawNo;
    u32 attr;
    ANIMDATA *animFrame[2];
    s16 mesRectX;
    s16 mesRectW;
    s16 mesRectY;
    s16 mesRectH;
    s16 mesX;
    s16 mesY;
    s16 mesCol;
    s16 mesColShadow;
    s16 charPadX;
    s16 charPadY;
    s16 winW;
    s16 winH;
    HuVec2F pos;
    HuVec2F scale;
    float zRot;
    s16 charEntryNum;
    s16 charEntryMax;
    WINCHARENTRY *charEntry;
    s16 messSp;
    s32 unk94;
    char *messData;
    char *messDataStack[HUWIN_INSERTMES_MAX];
    char *messDataInsert[HUWIN_INSERTMES_MAX];
    s16 choiceNum;
    s16 choice;
    s16 cursorSprNo;
    u8 choiceDisable[HUWIN_CHOICE_MAX];
    WINCHOICE choiceData[HUWIN_CHOICE_MAX];
    s16 scissorX;
    s16 scissorY;
    s16 scissorW;
    s16 scissorH;
    s16 tabW;
    s16 pushKey;
    s16 activePadKey;
    s16 choiceEndSe;
    u8 ATTRIBUTE_ALIGN(32) mesPal[HUWIN_MESCOL_MAX][3];
} HUWIN;

extern HUWIN winData[HUWIN_MAX];
extern void *messDataPtr;

void HuWindowInit(void);
void HuWinInit(s32 _messDataNo);
HUWINID HuWinCreate(float posX, float posY, s16 winW, s16 winH, s16 frame);
void HuWinKill(HUWINID winId);
void HuWinAllKill(void);
char *HuWinMesPtrGet(u32 messNum);
u32 HuWinMesMaxDirGet(void);
u32 HuWinMesMaxNumGet(u32 dirNum);
void *HuWinMesDataPtrGet(void *messData, u32 messNum);

void HuWinHomeClear(HUWINID winId);
void HuWinKeyWaitEntry(HUWINID winId);
void HuWinPosSet(HUWINID winId, float posX, float posY);
void HuWinScaleSet(HUWINID winId, float scaleX, float scaleY);
void HuWinZRotSet(HUWINID winId, float zRot);
void HuWinCenterPosSet(HUWINID winId, float centerX, float centerY);
void HuWinDrawNoSet(HUWINID winId, s16 drawNo);
void HuWinScissorSet(HUWINID winId, s16 x, s16 y, s16 w, s16 h);
void HuWinPriSet(HUWINID winId, s16 prio);
void HuWinAttrSet(HUWINID winId, u32 attr);
void HuWinAttrReset(HUWINID winId, u32 attr);
s16 HuWinStatGet(HUWINID winId);
void HuWinMesColSet(HUWINID winId, u8 mesCol);
void HuWinMesPalSet(HUWINID winId, u8 mesCol, u8 r, u8 g, u8 b);
void HuWinBGTPLvlSet(HUWINID winId, float tpLvl);
void HuWinBGColSet(HUWINID winId, GXColor *bgCol);
void HuWinMesSpeedSet(HUWINID winId, s16 mesSpeed);
void HuWinMesRead(void);
void HuWinMesSet(HUWINID winId, u32 messNum);
void HuWinInsertMesSet(HUWINID winId, u32 messNum, s16 insertMesNo);
s16 HuWinChoiceSet(HUWINID winId, s16 choiceNo);
s16 HuWinChoiceGet(HUWINID winId, s16 choiceNo);
s16 HuWinChoiceNumGet(HUWINID winId);
void HuWinChoiceDisable(HUWINID winId, s16 choiceNo);
s16 HuWinChoiceNowGet(HUWINID winId);
void HuWinMesWait(HUWINID winId);
s16 HuWinAnimSet(HUWINID winId, ANIMDATA *anim, s16 animBank, float posX, float posY);
s16 HuWinSprSet(HUWINID winId, HUSPRID sprId, float posX, float posY);
void HuWinSprPosSet(HUWINID winId, s16 sprNo, float posX, float posY);
void HuWinSprPriSet(HUWINID winId, s16 sprNo, s16 prio);
HUSPRID HuWinSprIDGet(HUWINID winId, s16 sprNo);
void HuWinSprKill(HUWINID winId, s16 sprNo);
void HuWinDispOff(HUWINID winId);
void HuWinDispOn(HUWINID winId);
void HuWinComKeyWait(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4, s16 time);
void HuWinComKeySet(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4);
void _HuWinComKeySet(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4, s16 time);
void HuWinComKeyGet(HUWINID winId, u32 *key);
void HuWinComKeyReset(void);
void HuWinMesMaxSizeGet(s16 messCnt, HuVec2F *maxSize, ...);
void HuWinInsertMesSizeGet(u32 messNum, s16 insertMesNo);
void HuWinMesSizeCancelCRSet(s32 cancelCR);
void HuWinMesMaxSizeBetGet(HuVec2F *maxSize, u32 messStart, u32 messEnd);
s16 HuWinKeyWaitNumGet(u32 messNum);
void HuWinPushKeySet(HUWINID winId, s16 pushKey);
void HuWinDisablePlayerSet(HUWINID winId, u8 playerBit);
void HuWinDisablePlayerReset(HUWINID winId, u8 playerBit);
HUWINID HuWinExCreate(float x, float y, s16 w, s16 h, s16 speakerNo);
HUWINID HuWinExCreateFrame(float x, float y, s16 w, s16 h, s16 speakerNo, s16 frame);
void HuWinExOpen(HUWINID winId);
void HuWinExClose(HUWINID winId);
void HuWinExKill(HUWINID winId);
void HuWinExSpeakerSet(HUWINID winId, s16 speakerNo);
void *MessData_MesPtrGet(void *data, u32 messNum);

#endif
