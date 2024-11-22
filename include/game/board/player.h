#ifndef _BOARD_PLAYER_H
#define _BOARD_PLAYER_H

#include "game/board/model.h"
#include "game/gamework.h"

void MBPlayerInit(BOOL noEventF);
void MBPlayerClose(void);
BOOL MBPlayerAllComCheck(void);
void MBPlayerCapsuleAdd(s32 playerNo, s32 capsuleNo);
void MBPlayerCoinSet(s32 playerNo, s32 coinNum);
s32 MBPlayerFirstGet(void);
MBMODELID MBPlayerModelGet(s32 playerNo);
void MBPlayerPosGet(s32 playerNo, HuVecF *pos);
s32 MBPlayerAliveComGet(s32 *playerNo);
BOOL MBPlayerStoryComCheck(s32 playerNo);
BOOL MBPlayerAliveCheck(s32 playerNo);

#endif