#ifndef _BOARD_SAI_H
#define _BOARD_SAI_H

#include "game/board/main.h"

typedef void (*SAIHITHOOK)(int result);
typedef u16 (*SAIPADBTNHOOK)(int playerNo);
typedef void (*SAIMOTHOOK)(int playerNo);

#define SAITYPE_NORMAL 0
#define SAITYPE_KINOKO 1
#define SAITYPE_SKINOKO 2
#define SAITYPE_NKINOKO 3
#define SAITYPE_KILLER 4
#define SAITYPE_HIDDEN 5
#define SAITYPE_EVENT 6
#define SAITYPE_PATAPATA_COIN 7
#define SAITYPE_CHANCE_TRADE 8
#define SAITYPE_CHANCE_CHAR 9
#define SAITYPE_BLOCK 10
#define SAITYPE_MAX 11

#define SAI_COLOR_GREEN 0
#define SAI_COLOR_BLUE 1
#define SAI_COLOR_RED 2

#define SAI_VALUE_ANY -1
#define SAI_VALUENUM_MAX 11

#define SAI_RESULT_FAIL1 -1
#define SAI_RESULT_FAIL2 -2
#define SAI_RESULT_SCROLL -3
#define SAI_RESULT_FAIL4 -4
#define SAI_RESULT_CAPSULESEL -5
#define SAI_RESULT_CAPSULESKIP -6
#define SAI_RESULT_SAIKORO -7
#define SAI_RESULT_NULL -8

#define SAI_PLAYER_NULL -1

void MBSaiInit(void);
int MBSaiTutorialExec(int playerNo, int saiType, s8 *valueTbl, int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
int MBSaiExec(int playerNo, int saiType, s8 *valueTbl, int tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
int MBSaiPlayerExec(int playerNo, int saiType);
int MBSaiChanceTradeExec(int playerNo);
int MBSaiChanceCharExec(int playerNo, int *playerNoTbl);
void MBSaiKill(int playerNo);
void MBSaiClose(void);
BOOL MBSaiKillCheck(int playerNo);
BOOL MBSaiKillCheckAll();
int MBSaiResultGet(int playerNo);
void MBSaiHitHookSet(int playerNo, SAIHITHOOK hook);
void MBSaiObjHit(int playerNo);
void MBSaiPadBtnHookSet(int playerNo, SAIPADBTNHOOK hook);
void MBSaiMotHookSet(int playerNo, SAIMOTHOOK hook);
OMOBJ *MBSaiNumObjCreate(int playerNo, HuVecF *pos1, HuVecF *pos2, int value, int color, BOOL topF);
void MBSaiNumObjKill(OMOBJ *obj);
BOOL MBSaiNumMoveCheck(int playerNo);
void MBSaiNumKill(int playerNo);
void MBSaiNumShrinkSet(int playerNo);
BOOL MBSaiNumStopCheck(int playerNo);

#endif