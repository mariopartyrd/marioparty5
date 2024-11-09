#ifndef _GAME_THPMAIN_H
#define _GAME_THPMAIN_H

#include "dolphin/types.h"
#include "game/process.h"
#include "game/hu3d.h"
#include "game/sprite.h"


HUSPRID HuTHPSprCreateVol(char *path, s16 loop, s16 prio, float volume);
HUSPRID HuTHPSprCreate(char *path, s16 loop, s16 prio);
HU3DMODELID HuTHP3DCreateVol(char *path, s16 loop, float volume);
HU3DMODELID HuTHP3DCreate(char *path, s16 loop);
void HuTHPStop(void);
void HuTHPClose(void);
void HuTHPRestart(void);
BOOL HuTHPEndCheck(void);
s32 HuTHPFrameGet(void);
s32 HuTHPTotalFrameGet(void);
void HuTHPSetVolume(s32 left, s32 right);
BOOL HuTHPProcCheck(void);
BOOL HuTHPStartCheck(void);
void HuTHPMatHook(HU3DDRAWOBJ *drawObj, HSFMATERIAL *material);

extern HUPROCESS *THPProc;


#endif