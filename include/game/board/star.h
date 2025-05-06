#ifndef _BOARD_STAR_H
#define _BOARD_STAR_H

#include "game/board/main.h"

void MBStarNoInit(void);
void MBStarInit(void);
void MBStarClose(void);
void MBStarNoReset(int no);
void MBStarCreate(int no);
void MBMasuStarExec(int playerNo, int masuId);
void MBStarGetProcExec(int playerNo, HuVecF *pos, int starNum, BOOL waitF);
void MBStarGetExec(int playerNo, HuVecF *pos);
void MBStarMapViewProcExec(int playerNo, HuVecF *pos, int no);
void MBStarMapViewExec(int playerNo, int no);
void MBStarNumSet(int num);
int MBStarNumGet(void);

void MBStarFlagInit(void);
void MBStarFlagSet(int no);
void MBStarFlagReset(int no);
u8 MBStarFlagGet(void);
void MBStarNextNoSet(s8 nextNo);
u8 MBStarNextNoGet(void);
void MBStarNoSet(int no);

int MBStarNoRandGet(void);
void MBStarObjDispSet(int objNo, BOOL dispF);
void MBStarDispSet(int no, BOOL dispF);
void MBStarMasuDispSet(int masuId, BOOL dispF);
void MBStarDispSetAll(BOOL dispF);
void MBStarTutorialExec(void);

#endif