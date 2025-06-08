#include "dolphin/PPCArch.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "game/process.h"
#include "game/main.h"
#include "game/mg/timer.h"
#include "datanum/mgconst.h"


const static s16 timeUnitTbl[4] = { 
    0, 60, 100, 60 
};

const static s16 scoreWinSize[MGTIMER_TYPE_MAX][2] = { 
    { 32, 32 }, 
    { 144, 48 }, 
    { 96, 32 }, 
    { 136, 32 }
};

typedef void (*TIMERFUNC)(MGTIMER *timer);

static void TimerExec();
static void TimerExecOn(MGTIMER* timer);
static void TimerExecOff(MGTIMER* timer);
static void CreateScoreSpr(MGTIMER* timer);
static void KillScoreSpr(MGTIMER* timer);
static void UpdateTimerSpr(MGTIMER* timer);


static const HuVec2F scoreOfsTbl[MGTIMER_TYPE_MAX][15] = {
    {
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f},
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f},
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f},
        { 0.0f,  0.0f}, { 0.0f,  0.0f}, { 0.0f,  0.0f}
    },
    {
        {-46.0f,  0.0f}, {-30.0f,  0.0f}, {-14.0f,  0.0f}, {  2.0f,  0.0f},
        { 18.0f,  0.0f}, { 34.0f,  0.0f}, { 50.0f,  0.0f}, {-46.0f, -16.0f},
        {-30.0f, -16.0f}, {-14.0f, -16.0f}, {  2.0f, -16.0f}, { 18.0f, -16.0f},
        { 34.0f, -16.0f}, { 50.0f, -16.0f}, {-66.0f, -16.0f},
    },
    {
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-40.0f, -8.0f}, {-24.0f, -8.0f},
        {-8.0f, -8.0f}, {  8.0f, -8.0f}, { 24.0f, -8.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}
    },
    {
        {-56.0f, -8.0f}, {-40.0f, -8.0f}, {-24.0f, -8.0f}, {-8.0f, -8.0f},
        {  8.0f, -8.0f}, { 24.0f, -8.0f}, { 40.0f, -8.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}
    },
};

static TIMERFUNC modeTbl[MGTIMER_MODE_MAX] = { TimerExecOff, TimerExecOn};

MGTIMER* MgTimerCreate(int type) {
    MGTIMER* timer;
    int i;
    timer = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MGTIMER), HU_MEMNUM_OVL);
    if (timer == NULL) {
        return NULL;
    }
    timer->type = type;
    timer->mode = MGTIMER_MODE_OFF;
    timer->prevMode = MGTIMER_MODE_OFF;
    timer->dispOnF = FALSE;
    timer->winDispF = TRUE;
    timer->stopF = FALSE;
    timer->scaleDir = 0;
    timer->flashFlag = 0;
    timer->time = 0;
    timer->maxTime = 0;
    timer->recordTime = 0;
    timer->endTime = 0;
    timer->speed = 0;
    timer->pos.x = 288.0f;
    timer->pos.y = 64.0f;
    timer->gMesId = GMES_ID_NONE;
    timer->winGrpId = HUSPR_GRP_NONE;
    timer->digitR = 255;
    timer->digitG = 255;
    timer->digitB = 255;
    timer->digitScale = 1.0f;
    timer->timeUnit = timeUnitTbl[type];
    timer->fadeOutTime = 0;
    for (i = 0; i < 15; i++) {
        timer->espId[i] = -1 ;
    }
    CreateScoreSpr(timer);
    switch (type) {
        case 0:
            break;
            
        case 1:
        case 2:
        case 3:
            timer->winGrpId = MgScoreWinCreate(scoreWinSize[timer->type][0], scoreWinSize[timer->type][1]);
            MgScoreWinColorSet(timer->winGrpId, 0, 0, 0);
            MgScoreWinTPLvlSet(timer->winGrpId, 0.7f);
            MgScoreWinDispSet(timer->winGrpId, 0);
            break;

    }
    MgTimerDigitColorSet(timer, 255, 255, 255);
    UpdateTimerSpr(timer);
    timer->proc = HuPrcChildCreate(TimerExec, 0x1000, 0x1200, 0, HuPrcCurrentGet());
    timer->proc->property = timer;
    return timer;
}

void MgTimerKill(MGTIMER* timer) {
    HuPrcKill(timer->proc);
    KillScoreSpr(timer);
    if (timer->gMesId != GMES_ID_NONE) {
        GMesKill(timer->gMesId);
    }
    HuMemDirectFree(timer);
}

int MgTimerModeGet(MGTIMER* timer) {
    return timer->mode;
}

void MgTimerParamSet(MGTIMER* timer, int maxTime, int endTime, int recordTime) {
    timer->maxTime = maxTime;
    timer->time = timer->maxTime;
    timer->endTime = endTime;
    timer->recordTime = recordTime;
    timer->speed = (maxTime == endTime) ? 0 : ((maxTime > endTime) ? -1 : 1);
    UpdateTimerSpr(timer);
}

int MgTimerValueGet(MGTIMER* timer) {
    return timer->time;
}

void MgTimerPosSet(MGTIMER *timer, float posX, float posY) {
    timer->pos.x = posX;
    timer->pos.y = posY;
    UpdateTimerSpr(timer);
}

void MgTimerPosGet(MGTIMER* timer, float *posX, float *posY) {
    *posX = timer->pos.x;
    *posY = timer->pos.y;
}

void MgTimerModeOnSet(MGTIMER* timer, int offType) {
    timer->offType = offType;
    timer->stopF = FALSE;
    timer->mode = MGTIMER_MODE_ON;
}

void MgTimerModeOffSet(MGTIMER* timer) {
    timer->mode = MGTIMER_MODE_OFF;
}

BOOL MgTimerDoneCheck(MGTIMER* timer) {
    return timer->endTime == timer->time;
}

void MgTimerRecordSet(MGTIMER* timer, int recordTime) {
    if (recordTime == -1) {
        if (((timer->speed > 0) && (timer->time < timer->recordTime)) || ((timer->speed < 0) && (timer->time > timer->recordTime))) {
            timer->recordTime = timer->time;
            timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_RECORD;
        }
    } else {
        timer->recordTime = recordTime;
        timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_RECORD;
    }
}

void MgTimerDigitColorSet(MGTIMER* timer, u8 r, u8 g, u8 b) {
    int i;
    timer->digitR = r;
    timer->digitG = g;
    timer->digitB = b;
    for (i = 0; i <= 6; i++) {
        if (timer->espId[i] != -1) {
            espColorSet(timer->espId[i], r, g, b);
        }
    }
}

void MgTimerWinColorSet(MGTIMER* timer, u8 r, u8 g, u8 b) {
    if (timer->winGrpId != HUSPR_GRP_NONE) {
        MgScoreWinColorSet(timer->winGrpId, r, g, b);
    }
}

static void TimerExec(void) {
    MGTIMER* timer;

    timer = HuPrcCurrentGet()->property;
    while (1) {
        modeTbl[timer->mode](timer);
    }
}

static void TimerExecOff(MGTIMER* timer) {
    int i;
    s16 mode;
    int maxCheck;
    s16 prevMode;

    while (1) {
        UpdateTimerSpr(timer);
        for(maxCheck=1, i=0; i<maxCheck; i++) {
            HuPrcVSleep();
            mode = ((MGTIMER *)HuPrcCurrentGet()->property)->mode;
            prevMode = ((MGTIMER *)HuPrcCurrentGet()->property)->prevMode;
            ((MGTIMER *)HuPrcCurrentGet()->property)->prevMode = mode;
            if (mode != prevMode) {
                return;
            }
        }
    }
}

static void TimerExecOn(MGTIMER* timer) {
    int i;
    s16 mode;
    int maxCheck;
    s16 prevMode;
    
    if (timer->dispOnF == 0) {
        MgTimerDispOn(timer);
    }
    while ((timer->time != timer->endTime) && (timer->stopF == 0)) {
        timer->time = timer->time + timer->speed;
        UpdateTimerSpr(timer);
        for(maxCheck=1, i=0; i<maxCheck; i++) {
            HuPrcVSleep();
            mode = ((MGTIMER*)HuPrcCurrentGet()->property)->mode;
            prevMode = ((MGTIMER*)HuPrcCurrentGet()->property)->prevMode;
            ((MGTIMER*)HuPrcCurrentGet()->property)->prevMode = mode;
            if (mode != prevMode) {
                return;
            }
        }
    }
    timer->stopF = TRUE;
    UpdateTimerSpr(timer);
    timer->fadeOutTime = 0;
    switch (timer->offType) {               
        case MGTIMER_OFFTYPE_NORMAL:
            timer->mode = MGTIMER_MODE_OFF;
            return;
            
        case MGTIMER_OFFTYPE_FADEOUT:
            if (timer->type == MGTIMER_TYPE_NORMAL) {
                MgTimerDispOff(timer);
            } else {
                timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_FADEOUT;
            }
            timer->mode = MGTIMER_MODE_OFF;
            return;
            
        case MGTIMER_OFFTYPE_FLASH:
            timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_COLOR;
            timer->mode = MGTIMER_MODE_OFF;
            break;
    }
}

static void CreateScoreSpr(MGTIMER* timer) {
    int i;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            break;
            
        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (i = 0; i < 7; i++) {
                timer->espId[i] = espEntry(MGCONST_ANM_scoreSmall, 0, 0);
                espColorSet(timer->espId[i], 255, 255, 255);
            }
            for (i = 0; i < 7; i++) {
                timer->espId[i + 7] = espEntry(MGCONST_ANM_scoreSmall, 0, 0);
                espColorSet(timer->espId[i + 7], 66, 255, 122);
            }
            timer->espId[14] = espEntry(MGCONST_ANM_crown, 0, 0);
            espBankSet(timer->espId[1], 10);
            espBankSet(timer->espId[4], 11);
            espBankSet(timer->espId[8], 10);
            espBankSet(timer->espId[11], 11);
            for (i = 0; i < 15; i++) {
                if (timer->espId[i] != -1) {
                    espDispOff(timer->espId[i]);
                    espPriSet(timer->espId[i], 9); 
                }
            }
            break;
    }
}

static void KillScoreSpr(MGTIMER* timer) {
    int i;
    for (i = 0; i < 15; i++) {
        if (timer->espId[i] != -1) {
            espKill(timer->espId[i]);
            timer->espId[i] = -1;
        }
    }
}

static void UpdateTimerSpr(MGTIMER* timer) {
    int i;
    s32 second;
    s32 centisecs;
    s32 minute;
    u8 r;
    u8 g;
    u8 b;
    float tpLvl;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if ((timer->time % 60) == 0) {
                GMesTimerValueSet(timer->gMesId, timer->time / 60);
            }
            GMesPosSet(timer->gMesId, timer->pos.x, timer->pos.y);
            break;
        
        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (i = 0; i < 15; i++) {
                if (timer->espId[i] != -1) {
                    espPosSet(timer->espId[i],  timer->pos.x + scoreOfsTbl[timer->type][i].x, timer->pos.y + scoreOfsTbl[timer->type][i].y);
                } 
            }
            if (timer->winGrpId != -1) {
                MgScoreWinPosSet(timer->winGrpId, timer->pos.x - 8.0f, timer->pos.y - 8.0f);
            }
            minute = (timer->time) / (timer->timeUnit * 60);
            second = (timer->time % (timer->timeUnit * 60)) / 60;
            centisecs =  1.6666666f * ((timer->time % 3600) % 60);
            espBankSet(timer->espId[0], minute % 10);
            espBankSet(timer->espId[2], second / 10);
            espBankSet(timer->espId[3], second % 10);
            espBankSet(timer->espId[5], centisecs / 10);
            espBankSet(timer->espId[6], centisecs % 10);

            minute = timer->recordTime / (timer->timeUnit * 60);
            second = (timer->recordTime % (timer->timeUnit * 60)) / 60;
            centisecs = 1.6666666f * ((timer->recordTime % 3600) % 60);
            espBankSet(timer->espId[7], minute % 10);
            espBankSet(timer->espId[9], second / 10);
            espBankSet(timer->espId[10], second % 10);
            espBankSet(timer->espId[12], centisecs / 10);
            espBankSet(timer->espId[13],  centisecs - (centisecs / 10 * 10));
            if (timer->flashFlag & MGTIMER_FLASH_COLOR) {
                if (((GlobalCounter / 10) & 1) == 0) {
                    r = timer->digitR;
                    g = timer->digitG;
                    b = timer->digitB;
                } else {
                    r = 255;
                    g = 216;
                    b = 0;
                }
                for (i = 0; i < 7; i++) {
                    if (timer->espId[i] != -1) {
                        espColorSet(timer->espId[i], r, g, b);
                    } 
                }
            }
            if (timer->flashFlag & MGTIMER_FLASH_RECORD) {
                if (timer->scaleDir) {
                    if ((timer->digitScale += 0.02f) >= 1.2f) {
                        timer->digitScale = 1.2f;
                        timer->scaleDir = 0;
                    }
                } else {
                    if ((timer->digitScale -= 0.04f) <= 1.0f) {
                        timer->digitScale = 1.0f;
                        timer->scaleDir = 1;
                    }
                }
                for (i = 0; i < 8; i++) {
                    if (timer->espId[i+7] != -1) {
                        espScaleSet(timer->espId[i+7], timer->digitScale, timer->digitScale);
                    }  
                }
            }
            if (timer->flashFlag & MGTIMER_FLASH_FADEOUT) {
                tpLvl = 1.0f - (0.016666668f * timer->fadeOutTime);
                for (i = 0; i < 15; i++) {
                    if (timer->espId[i] != -1) {
                        espTPLvlSet(timer->espId[i], tpLvl);
                    }
                    if (timer->winGrpId != -1) {
                        MgScoreWinTPLvlSet(timer->winGrpId, tpLvl);
                    } 
                }
                if (timer->fadeOutTime++ >= 60) {
                    timer->flashFlag = timer->flashFlag & ~MGTIMER_FLASH_FADEOUT;
                    MgTimerDispOff(timer);
                }
            }
    }
    (void)second;
}

void MgTimerDispOn(MGTIMER* timer) {
    int i;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if (timer->gMesId == GMES_ID_NONE) {
                timer->gMesId = GMesTimerCreate(timer->time / 60);
                GMesPosSet(timer->gMesId, timer->pos.x, timer->pos.y);
            }
            break;
        
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_WIDESCORE:
            for (i = 0; i < 15; i++) {
                if (timer->espId[i] != -1) {
                    espDispOn(timer->espId[i]);
                }
            }
            if (timer->winGrpId != -1) {
                MgScoreWinDispSet(timer->winGrpId, timer->winDispF);
            }
            break;
            
    }
    timer->dispOnF = TRUE;
}

void MgTimerDispOff(MGTIMER* timer) {
    int i;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if (timer->gMesId != GMES_ID_NONE) {
                GMesTimerEnd(timer->gMesId);
                timer->gMesId = GMES_ID_NONE;
            }
            break;
        
        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (i = 0; i < 15; i++) {
                if (timer->espId[i] != -1) {
                    espDispOff(timer->espId[i]);
                }
            }
            if (timer->winGrpId != -1) {
                MgScoreWinDispSet(timer->winGrpId, FALSE);
            }
            break;
    }
    timer->dispOnF = FALSE;
}

void MgTimerRecordDispOn(MGTIMER* timer) {
    int i;
    
    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if (timer->gMesId == GMES_ID_NONE) {
                timer->gMesId = GMesTimerCreate(timer->time / 60);
                GMesPosSet(timer->gMesId, timer->pos.x, timer->pos.y);
            }   
            break;
        
        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (i = 0; i < 15; i++) {
                if (timer->espId[i] != -1) {
                    espDispOn(timer->espId[i]);
                }
            }
            if (timer->winGrpId != HUSPR_GRP_NONE) {
                MgScoreWinDispSet(timer->winGrpId, timer->winDispF);
            }
            break;
    }
    timer->dispOnF = TRUE;
}

void MgTimerRecordDispOff(MGTIMER* timer) {
    int i;
    
    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if(timer->gMesId != GMES_ID_NONE) {
                GMesTimerEnd(timer->gMesId);
                timer->gMesId = GMES_ID_NONE;
            }
            break;
            
        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for(i = 0; i < 15; i++) {
                if (timer->espId[i] != -1) {
                    espDispOff(timer->espId[i]);
                }
            }
            if(timer->winGrpId != -1) {
                MgScoreWinDispSet(timer->winGrpId, FALSE);
            }
    }
    
    timer->dispOnF = FALSE;
}