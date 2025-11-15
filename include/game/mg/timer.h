#ifndef _MG_TIMER_H
#define _MG_TIMER_H

#include "dolphin/types.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/gamemes.h"

#include "humath.h"

#define MGTIMER_MODE_OFF 0
#define MGTIMER_MODE_ON 1
#define MGTIMER_MODE_MAX 2

#define MGTIMER_TYPE_NORMAL 0
#define MGTIMER_TYPE_RECORD 1
#define MGTIMER_TYPE_SCORE 2
#define MGTIMER_TYPE_WIDESCORE 3
#define MGTIMER_TYPE_MAX 4

#define MGTIMER_FLASH_COLOR (1 << 0)
#define MGTIMER_FLASH_RECORD (1 << 1)
#define MGTIMER_FLASH_FADEOUT (1 << 2)

#define MGTIMER_OFFTYPE_NORMAL 0
#define MGTIMER_OFFTYPE_FADEOUT 1
#define MGTIMER_OFFTYPE_FLASH 2


typedef struct MgTimer_s {
    s32 mode;
    s32 prevMode;
    s32 type;
    s32 offType;
    BOOL dispOnF;
    BOOL stopF;
    s32 scaleDir;
    BOOL winDispF;
    u8 flashFlag;
    s32 maxTime;
    s32 endTime;
    s32 speed;
    s32 time;
    s32 recordTime;
    s32 fadeOutTime;
    HuVec2F pos;
    s16 timeUnit;
    GAMEMESID gameMesId;
    HUSPRGRPID winGrpId;
    HUSPRID espId[15];
    float digitScale;
    HUPROCESS *proc;
    u8 digitR;
    u8 digitG;
    u8 digitB;
} MGTIMER;

MGTIMER* MgTimerCreate(int type);
void MgTimerKill(MGTIMER* timer);
int MgTimerModeGet(MGTIMER* timer);
void MgTimerParamSet(MGTIMER* timer, int maxTime, int endTime, int recordTime);
int MgTimerValueGet(MGTIMER* timer);
void MgTimerPosSet(MGTIMER* timer, float posX, float posY);
void MgTimerPosGet(MGTIMER* timer, float *posX, float *posY);
void MgTimerModeOnSet(MGTIMER* timer, int offType);
void MgTimerModeOffSet(MGTIMER* timer);
BOOL MgTimerDoneCheck(MGTIMER* timer);
void MgTimerRecordSet(MGTIMER* timer, int recordTime);
void MgTimerDigitColorSet(MGTIMER* timer, u8 r, u8 g, u8 b);
void MgTimerWinColorSet(MGTIMER* timer, u8 r, u8 g, u8 b);
void MgTimerDispOn(MGTIMER* timer);
void MgTimerDispOff(MGTIMER* timer);
void MgTimerRecordDispOn(MGTIMER* timer);
void MgTimerRecordDispOff(MGTIMER* timer);


#endif