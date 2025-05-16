#ifndef _BOARD_TELOP_H
#define _BOARD_TELOP_H

#include "game/charman.h"
#include "game/board/main.h"

#define TELOP_CHAR_MAX CHARNO_MAX
#define TELOP_LOGO_BASE TELOP_CHAR_MAX
#define TELOP_LOGO_MAX 7
#define TELOP_MAX (TELOP_CHAR_MAX+TELOP_LOGO_MAX)

#define MBTelopCharCreate(playerNo, no, waitF) MBTelopCreate(playerNo, no, waitF)
#define MBTelopLogoCreate(no, waitF) MBTelopCreate(-1, (no)+TELOP_LOGO_BASE, waitF)

void MBTelopPlayerCreate(int playerNo);
void MBTelopCreate(int playerNo, int telopNo, BOOL waitF);
BOOL MBTelopCheck(void);
void MBTelopLast5Create(void);
void MBTelopComNumCreate(void);
void MBTauntInit(void);
void MBTauntKill(void);

#endif