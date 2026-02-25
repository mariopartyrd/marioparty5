#ifndef _BOARD_SCROLL_H
#define _BOARD_SCROLL_H

#include "game/board/main.h"

typedef void (*MAPVIEWHOOK)(BOOL enterF);

void MBScrollInit(unsigned int dataNum, float zoom, float posZ);
void MBScrollClose(void);
//void MBScrollExec(int playerNo);

void MBMapViewExec(int playerNo);

void MBStarScrollStart(void);
BOOL MBStarScrollDoneCheck(void);
void MBStarScrollKill(void);

void MBMapViewCameraSet(HuVecF *rot, HuVecF *pos, float zoom);
void MBMapViewHookSet(MAPVIEWHOOK hook);

#endif