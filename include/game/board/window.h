#ifndef _BOARD_WINDOW_H
#define _BOARD_WINDOW_H

#include "game/window.h"
#include "game/process.h"
#include "game/pad.h"

#define MBWIN_MAX 16

#define MBWIN_TYPE_RESULT 0
#define MBWIN_TYPE_TALKCHOICE 1
#define MBWIN_TYPE_TALKEVENT 2
#define MBWIN_TYPE_EVENT 3
#define MBWIN_TYPE_CHOICE 4
#define MBWIN_TYPE_GUIDE 5
#define MBWIN_TYPE_TUTORIAL 6
#define MBWIN_TYPE_GUIDETOP 7
#define MBWIN_TYPE_CAPSULE 8
#define MBWIN_TYPE_HELP 9
#define MBWIN_TYPE_PAUSEGUIDE 10

#define MBWIN_TYPE_MAX 11

#define MBWIN_MES_NONE -1

#define MBWIN_NONE -1

typedef void (*MBWINCOMKEYHOOK)(void);

typedef struct MBWin_s {
    HUPROCESS *proc;
    int winId;
    u32 mess;
    u8 no;
    s8 mesSpeed;
    u32 attr;
    int type;
    s8 speakerNo;
    s16 prio;
    int playerNo;
    HuVec2F pos;
    HuVec2F size;
    HuVec2F scale;
    BOOL pauseF;
    BOOL choiceF;
    int choiceNo;
    BOOL choiceDisable[HUWIN_CHOICE_MAX];
    u32 insertMes[HUWIN_INSERTMES_MAX];
    u8 disablePlayer;
    MBWINCOMKEYHOOK comKeyHook;
} MBWIN;

void MBWinInit(void);
void MBWinClose(void);
int MBWinCreate(int type, u32 mess, int speakerNo);
int MBWinCreateChoice(int type, u32 mess, int speakerNo, int choiceNo);
int MBWinCreateHelp(u32 mess);
MBWIN *MBWinGet(int winNo);
void MBWinKill(s16 winNo);
void MBTopWinKill(void);
void MBWinKillAll(void);

void MBWinPosSet(s16 winNo, s16 posX, s16 posY);
void MBTopWinPosSet(s16 posX, s16 posY);
void MBWinPosGet(s16 winNo, HuVec2F *pos);
void MBTopWinPosGet(HuVec2F *pos);
void MBWinSizeSet(s16 winNo, s16 sizeX, s16 sizeY);
void MBTopWinSizeSet(s16 sizeX, s16 sizeY);
void MBWinMesMaxSizeGet(s16 winNo, HuVec2F *size);
void MBTopWinMesMaxSizeGet(HuVec2F *size);
void MBWinScaleSet(s16 winNo, float scaleX, float scaleY);
void MBTopWinScaleSet(float scaleX, float scaleY);
void MBWinScaleGet(s16 winNo, HuVec2F *scale);
void MBTopWinScaleGet(HuVec2F *scale);
int MBWinLastChoiceGet(void);
int MBWinChoiceGet(s16 winNo);
void MBWinPause(s16 winNo);
void MBTopWinPause(void);
void MBWinInsertMesSet(s16 winNo, u32 insertMes, int insertMesNo);
void MBTopWinInsertMesSet(u32 insertMes, int insertMesNo);
BOOL MBWinDoneCheck(s16 winNo);
BOOL MBTopWinDoneCheck(void);
void MBWinWait(s16 winNo);
void MBTopWinWait(void);
void MBWinAttrSet(s16 winNo, u32 attr);
void MBTopWinAttrSet(u32 attr);
void MBWinAttrReset(s16 winNo, u32 attr);
void MBTopWinAttrReset(u32 attr);
void MBWinChoiceDisable(s16 winNo, int choiceNo);
void MBTopWinChoiceDisable(int choiceNo);
void MBWinMesSpeedSet(s16 winNo, int speed);
void MBTopWinMesSpeedSet(int speed);
void MBWinMesColSet(s16 winNo, int mesCol);
void MBTopWinMesColSet(int mesCol);
s16 MBWinChoiceNowGet(s16 winNo);
s16 MBTopWinChoiceNowGet(void);
void MBWinPriSet(s16 winNo, s16 prio);
void MBTopWinPriSet(s16 prio);
s16 MBWinChoiceNowGet2(s16 winNo);
s16 MBTopWinChoiceNowGet2(void);
void MBWinPlayerDisable(s16 winNo, int playerNo);
void MBTopWinPlayerDisable(int playerNo);
void MBWinComKeyHookSet(s16 winNo, MBWINCOMKEYHOOK comKeyHook);
void MBTopWinComKeyHookSet(MBWINCOMKEYHOOK comKeyHook);
void MBWinDispSet(s16 winNo, BOOL dispF);
void MBTopWinDispSet(BOOL dispF);
void MBWinPauseHook(BOOL dispF);
HUWINID MBWinIDGet(s16 winNo);
HUWINID MBTopWinIDGet(void);
void MBWinCenterGet(s16 winNo, HuVec2F *pos);

#endif