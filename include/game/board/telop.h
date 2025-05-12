#ifndef _BOARD_TELOP_H
#define _BOARD_TELOP_H

#include "game/charman.h"

#define MBTelopCharCreate(playerNo, no, waitF) MBTelopCreate(playerNo, no, waitF)
#define MBTelopLogoCreate(no, waitF) MBTelopCreate(-1, (no)+CHARNO_MAX, waitF)

void MBTelopPlayerCreate(int playerNo);
void MBTelopCreate(int playerNo, int telopNo, BOOL waitF);
void MBTelopComNumCreate(void);

#endif