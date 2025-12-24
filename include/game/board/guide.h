#ifndef _BOARD_GUIDE_H
#define _BOARD_GUIDE_H

#include "game/board/model.h"
#include "game/board/main.h"
#include "game/window.h"

#define MB_GUIDE_CHORL 0
#define MB_GUIDE_NERL 1
#define MB_GUIDE_TERL 2
#define MB_GUIDE_NIRL 3
#define MB_GUIDE_MAX 4

#define MB_GUIDE_ATTR_NONE 0
#define MB_GUIDE_ATTR_SCREEN (1 << 0)
#define MB_GUIDE_ATTR_EFFIN (1 << 1)
#define MB_GUIDE_ATTR_EFFOUT (1 << 2)
#define MB_GUIDE_ATTR_MODELOFF (1 << 3)
#define MB_GUIDE_ATTR_ALTMTX (1 << 4)

void MBGuideInit(void);
MBMODELID MBGuideCreate(int guideNo, const HuVecF *pos, const HuVecF *rot, u32 attr);
MBMODELID MBGuideCreateFlag(int guideNo, const HuVecF *pos, BOOL screenF, BOOL effInF, BOOL effOutF, BOOL modelOffF);
MBMODELID MBGuideCreateIn(const HuVecF *pos, BOOL screenF, BOOL effInF, BOOL modelOffF);
MBMODELID MBGuideCreateDirect(void);
void MBGuideEnd(MBMODELID modelId);
void MBGuideKill(void);
void MBGuideMdlPosSet(MBMODELID modelId, float posX, float posY, float posZ);
void MBGuideMdlPosSetV(MBMODELID modelId, const HuVecF *pos);
void MBGuideMdlPosGet(MBMODELID modelId, HuVecF *pos);
void MBGuideScreenPosSet(HuVecF *pos);
void MBGuideScreenPosGet(HuVecF *pos);
int MBGuideNoGet(void);
int MBGuideSpeakerNoGet(void);

#endif