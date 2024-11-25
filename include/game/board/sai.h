#ifndef _BOARD_SAI_H
#define _BOARD_SAI_H

#include "game/board/main.h"

#define MB_SAI_NORMAL 0
#define MB_SAI_KINOKO 1
#define MB_SAI_SKINOKO 2
#define MB_SAI_NKINOKO 3
#define MB_SAI_KILLER 4
#define MB_SAI_HIDDEN 5
#define MB_SAI_KUPA 6
#define MB_SAI_PATAPATA_COIN 7
#define MB_SAI_CHANCE_TRADE 8
#define MB_SAI_CHANCE_CHAR 9
#define MB_SAI_BLOCK 10

#define MB_SAI_COLOR_GREEN 0
#define MB_SAI_COLOR_BLUE 1
#define MB_SAI_COLOR_RED 2

#define MB_SAI_VALUE_ANY -1

#define MB_SAI_RESULT_SCROLL -3
#define MB_SAI_RESULT_CAPSULESEL -5
#define MB_SAI_RESULT_CAPSULESKIP -6
#define MB_SAI_RESULT_DICE -7
#define MB_SAI_RESULT_NULL -8

void MBSaiKill(int playerNo);
void MBSaiObjKill(int playerNo);
BOOL MBSaiKillCheckAll(void);
int MBSaiResultGet(int playerNo);

void MBSaiKillSet(int playerNo);
int MBSaiExec(int playerNo, int saiNo, s8 *valueTbl, int tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int numColor);
int MBSaiTutorialExec(int playerNo, int saiNo, s8 *valueTbl, int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int numColor);
int MBSaiPlayerExec(int playerNo, int saiNo);

#endif