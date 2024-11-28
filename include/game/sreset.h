#ifndef _GAME_SRESET_H
#define _GAME_SRESET_H

#include "dolphin.h"

BOOL HuSoftResetButtonCheck(void);
void HuDvdErrDispInit(GXRenderModeObj *rmode, void *xfb1, void *xfb2);
void HuRestartSystem(void);
BOOL HuSoftResetCountCheck(void);
BOOL HuSoftResetCheck(void);

#endif
