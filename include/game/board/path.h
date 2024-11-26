#ifndef _BOARD_PATH_H
#define _BOARD_PATH_H

#include "game/board/main.h"
#include "game/board/masu.h"

void MBPathInit(void);
BOOL MBPathExec(int playerNo, s16 *masuId);
BOOL MBPathExecDebug(int playerNo, s16 *masuId);
void MBPathFlagSet(u32 flag);
void MBPathFlagReset(u32 flag);
void MBPathFlagInit(void);
u32 MBPathFlagGet(void);

#endif