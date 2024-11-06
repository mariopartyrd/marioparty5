#ifndef _GAME_WIPE_H
#define _GAME_WIPE_H

#include "dolphin.h"
#include "game/animdata.h"
#include "game/process.h"

#define WIPE_TYPE_MASK 0x7F
#define WIPE_TYPE_FBKEEP 0x80
#define WIPE_TYPE_NORMAL 0
#define WIPE_TYPE_CROSS 1
#define WIPE_TYPE_CROSSCOPY (WIPE_TYPE_CROSS|WIPE_TYPE_FBKEEP)
#define WIPE_TYPE_DISSOLVEIN 2
#define WIPE_TYPE_DISSOLVEOUT 3
#define WIPE_TYPE_DISSOLVEINBLUR (WIPE_TYPE_DISSOLVEIN|WIPE_TYPE_FBKEEP)
#define WIPE_TYPE_DISSOLVEOUTBLUR (WIPE_TYPE_DISSOLVEIN|WIPE_TYPE_FBKEEP)
#define WIPE_TYPE_VIEWSHIFT 4
#define WIPE_TYPE_PREVTYPE 5
#define WIPE_TYPE_WHITEFADE 6
#define WIPE_TYPE_STAR 7
#define WIPE_TYPE_WAVE 8
#define WIPE_TYPE_KOOPA 9
#define WIPE_TYPE_MAX 10

#define WIPE_MODE_DUMMY 0
#define WIPE_MODE_IN 1
#define WIPE_MODE_OUT 2
#define WIPE_MODE_END 3

typedef struct WipeWork_s {
	void *image[2];
	void *unk08[8];
    float time;
    float maxTime;
    u32 unk30;
	GXColor color;
	u8 type;
    u8 mode;
	u8 stat;
} WIPEWORK;

void WipeInit(GXRenderModeObj *rmode);
void WipeExecAlways(void);
void WipeCreate(s16 mode, s16 type, s16 maxTime);
void WipeColorSet(u8 r, u8 g, u8 b);
u8 WipeTypeGet(void);
u8 WipeCheck(void);
u8 WipeCheckEnd(void);

static inline void WipeWait(void)
{
    while(WipeCheckEnd()) {
        HuPrcVSleep();
    }
}

extern WIPEWORK wipeData;
extern BOOL wipeFadeInF;

#endif