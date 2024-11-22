#ifndef _BOARD_MASU_H
#define _BOARD_MASU_H

#include "game/hu3d.h"

#define MASU_TYPE_NONE 0
#define MASU_TYPE_BLUE 1
#define MASU_TYPE_RED 2
#define MASU_TYPE_KUPA 3
#define MASU_TYPE_HATENA 4
#define MASU_TYPE_STAR 5
#define MASU_TYPE_VS 7
#define MASU_TYPE_DONKEY 8

#define MASU_LAYER_DEFAULT 0

#define MASU_NONE -1

void MBMasuKill(void);
void MBMasuCapsuleClear(void);
void MBMasuPosGet(int layer, int id, HuVecF *pos);

s16 MBMasuDonkeySet(s16 eventNo);

void MBMasuTypeChange(s32 oldType, s32 newType);

#endif