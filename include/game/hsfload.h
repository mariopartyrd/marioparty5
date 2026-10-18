#ifndef _GAME_HSFLOAD_H
#define _GAME_HSFLOAD_H

#include "game/hsfformat.h"

HSFDATA *LoadHSF(void *data);
void ClusterAdjustObject(HSFDATA *model, HSFDATA *srcModel);
char *SetName(u32 *strOfs);
char *MakeObjectName(char *name);
s32 CmpObjectName(const char *name1, const char *name2);

#endif