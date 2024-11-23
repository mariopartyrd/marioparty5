#ifndef _BOARD_PLAYER_H
#define _BOARD_PLAYER_H

#include "game/board/model.h"
#include "game/gamework.h"
#include "game/charman.h"

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
void MBPlayerRotateStart(s32 playerNo, s16 endAngle, s16 maxTime);
BOOL MBPlayerRotateCheck(s32 playerNo);
int MBPlayerVoicePanPlay(s32 playerNo, int seId);
void MBPlayerWinLoseVoicePlay(s32 playerNo, s32 motId, int seId);
void MBPlayerMotionNoShiftSet(s32 playerNo, s32 motId, float start, float end, u32 attr);
BOOL MBPlayerMotionEndCheck(s32 playerNo);
void MBPlayerMotIdleSet(s32 playerNo);
void MBPlayerPosFixSet(s32 playerNo, int masuId);

#endif