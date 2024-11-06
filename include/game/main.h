#ifndef _GAME_MAIN_H
#define _GAME_MAIN_H

#include "dolphin.h"

extern BOOL NintendoFirstDispF;
extern BOOL HuSRDisableF;
extern BOOL HuDvdErrWait;
extern u32 GlobalCounter;

void HuSysVWaitSet(s16 vWait);
s16 HuSysVWaitGet(s16 prev);

#endif