#ifndef _BOARD_GATE_H
#define _BOARD_GATE_H

#include "game/board/main.h"

void MBGateInit(void);
void MBGateClose(void);
void MBGateCreate(void);

void MBGateBattanCreate(int masuId);
void MBGateBattanKill(int no);
int MBGateBattanSearch(s16 masuId);
int MBGateBattanExec(int playerNo, int masuId);

s16 MBGateMasuExec(int playerNo, s16 masuId, s16 masuNext);

void MBGateDossunCreate(int masuId);
void MBGateDossunKill(int no);
int MBGateDossunSearch(s16 masuId);
int MBGateDossunExec(int playerNo, int masuId);

#endif