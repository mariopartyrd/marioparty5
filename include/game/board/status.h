#ifndef _BOARD_STATUS_H
#define _BOARD_STATUS_H

#include "dolphin/types.h"
#include "game/sprite.h"

#define STATUS_COLOR_GRAY 0
#define STATUS_COLOR_BLUE 1
#define STATUS_COLOR_RED 2
#define STATUS_COLOR_GREEN 3
#define STATUS_COLOR_MAX 4
#define STATUS_COLOR_NULL -1

#define STATUS_MOVE_LINEAR 0
#define STATUS_MOVE_SIN 1
#define STATUS_MOVE_REVCOS 2

#define STATUS_LAYOUT_TOP 0
#define STATUS_LAYOUT_BOTTOM 1

#define STATUS_KAO_NORMAL 0
#define STATUS_KAO_WIN 1
#define STATUS_KAO_LOSE 2

void MBStatusInit(void);
void MBStatusClose(void);
void MBStatusReinit(void);

void MBStatusMoveTo(int playerNo, HuVecF *posBegin, HuVecF *posEnd);
void MBStatusMoveSet(int playerNo, HuVecF *posBegin, HuVecF *posEnd, int type, int time);
void MBStatusNoMoveSet(int statusNo, HuVecF *posBegin, HuVecF *posEnd, int type, int time);
void MBStatusPosOnGet(int statusNo, HuVecF *pos);
void MBStatusPosOffGet(int statusNo, HuVecF *pos);
BOOL MBStatusMoveCheck(int playerNo);
BOOL MBStatusOffCheckAll(void);
void MBStatusDispSet(int playerNo, BOOL dispF);
void MBStatusDispBackup(void);
void MBStatusDispRestore(void);
BOOL MBStatusDispGet(int playerNo);
int MBStatusNoGet(int playerNo);
void MBStatusDispSetAll(BOOL dispF);
void MBStatusDispForceSet(int playerNo, BOOL dispF);
void MBStatusDispForceSetAll(BOOL dispF);
void MBStatusCapsuleDispSet(int statusNo, BOOL dispF);
void MBStatusLayoutSet(int statusNo, int layoutNo);
void MBStatusCapsuleFocusSet(int statusNo, int index);
void MBStatusPosGet(int playerNo, HuVecF *pos);
void MBStatusNoPosGet(int statusNo, HuVecF *pos);
void MBStatusKaoNoSet(int statusNo, int kaoNo);
void MBSpriteCreate(int dataNum, int prio, ANIMDATA **animP, HUSPRID *sprIdP);
void MBSpriteNumSet(HUSPRGRPID gid, s16 memberNo, int num);
void MBStatusRainbowSet(int statusNo, float time, int colorEnd);
void MBStatusColorSet(int playerNo, int color);
int MBStatusColorGet(int playerNo);
void MBStatusColorAllSet(int color);

#endif