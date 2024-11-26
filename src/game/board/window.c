#include "game/board/window.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/pause.h"

#include "game/disp.h"

#include "game/flag.h"
#include "game/gamework.h"

static u8 winStack[MBWIN_MAX];
static s8 winChoice[MBWIN_MAX];
static BOOL winOnF[MBWIN_MAX];
static MBWIN mbWinData[MBWIN_MAX];

static HuVec2F winSizeTbl[MBWIN_TYPE_MAX] = {
    384, 64,
    432, 192,
    504, 96,
    432, 96,
    288, 192,
    432, 128,
    432, 96,
    432, 96,
    504, 96,
    384, 64,
    384, 64
};

static HuVec2F winPosTbl[MBWIN_TYPE_MAX] = {
    96, 328,
    72, 128,
    36, 344,
    72, 328,
    144, 144,
    128, 312,
    128, 264,
    128, 75,
    36, 352,
    96, 416,
    96, 328
};

static u8 winSp;
static int mbWinNum;
static int mbWinNo;
static s8 winChoiceLast;

static const u8 winSpeedTbl[4] = { 0, 1, 4, 0 };

static void KeyWaitInit(MBWIN *winP);
static void KeyWaitSet(MBWIN *winP, int keyWaitNum);

void MBWinInit(void)
{
    MBWIN *winP = &mbWinData[0];
    int i;
    HuWinInit(1);
    memset(&mbWinData[0], 0, sizeof(mbWinData));
    for(i=0; i<MBWIN_MAX; i++, winP++) {
        winP->winId = HUWIN_NONE;
    }
    memset(&winOnF[0], 0, sizeof(winOnF));
    mbWinNum = 0;
    mbWinNo = 0;
    winSp = 0;
    MBPauseHookPush(MBWinPauseHook);
}

void MBWinClose(void)
{
    MBWinKillAll();
    HuWinAllKill();
}

static void MBWinProc(void)
{
    HUPROCESS *proc = HuPrcCurrentGet();
    MBWIN *winP = proc->property;
    HuVec2F size;
    int i;
    HUWIN *huWinP;
    HuWinMesMaxSizeGet(1, &size, winP->mess);
    if(winP->size.x < size.x) {
        winP->pos.x -= (size.x-winP->size.x)*0.5f;
        winP->size.x = size.x;
    }
    if(winP->size.y < size.y) {
        winP->pos.y -= (size.y-winP->size.y)*0.5f;
        winP->size.y = size.y;
    }
    switch(winP->type) {
        case MBWIN_TYPE_HELP:
            winP->winId = HuWinCreate(winP->pos.x, winP->pos.y, winP->size.x, winP->size.y, 0);
            if(winP->winId == HUWIN_NONE) {
                HuPrcEnd();
            }
            HuWinBGTPLvlSet(winP->winId, 0);
            HuWinMesSpeedSet(winP->winId, 0);
            break;
        
        case MBWIN_TYPE_CAPSULE:
            winP->winId = HuWinCreate(winP->pos.x, winP->pos.y, winP->size.x, winP->size.y, 0);
            if(winP->winId == HUWIN_NONE) {
                HuPrcEnd();
            }
            HuWinMesSpeedSet(winP->winId, 0);
            break;
        
        case MBWIN_TYPE_PAUSEGUIDE:
            winP->winId = HuWinCreate(winP->pos.x, winP->pos.y, winP->size.x, winP->size.y, 0);
            if(winP->winId == HUWIN_NONE) {
                HuPrcEnd();
            }
            HuWinMesSpeedSet(winP->winId, 0);
            break;
       
        default:
            if(winP->speakerNo == HUWIN_SPEAKER_NONE) {
                winP->winId = HuWinExCreate(winP->pos.x, winP->pos.y, winP->size.x, winP->size.y, HUWIN_SPEAKER_NONE);
            } else {
                winP->winId = HuWinExCreate(winP->pos.x, winP->pos.y, winP->size.x, winP->size.y, winP->speakerNo);
            }
            if(winP->winId == HUWIN_NONE) {
                HuPrcEnd();
            }
            if(winP->speakerNo == HUWIN_SPEAKER_NONE) {
                HuWinExOpen(winP->winId);
            } else {
                HuWinExOpen(winP->winId);
            }
            HuWinMesSpeedSet(winP->winId, winP->mesSpeed);
            break;
    }
    HuWinDisablePlayerSet(winP->winId, winP->disablePlayer);
    if(winP->prio != -1) {
        HuWinPriSet(winP->winId, winP->prio);
    }
    HuWinMesSet(winP->winId, winP->mess);
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        if(winP->insertMes[i] != MBWIN_MES_NONE) {
            HuWinInsertMesSet(winP->winId, winP->insertMes[i], i);
        }
    }
    HuWinAttrSet(winP->winId, winP->attr);
    for(i=0; i<HUWIN_CHOICE_MAX; i++) {
        if(winP->choiceDisable[i]) {
            HuWinChoiceDisable(winP->winId, i);
        }
    }
    if(winP->choiceF) {
        KeyWaitInit(winP);
        if(winP->comKeyHook) {
            winP->comKeyHook();
        }
    } else {
        KeyWaitInit(winP);
    }
    huWinP = &winData[winP->winId];
    while(huWinP->stat != HUWIN_STAT_NONE) {
        if((huWinP->attr & HUWIN_ATTR_KEYWAIT_CLEAR) && !(winP->attr & HUWIN_ATTR_KEYWAIT_CLEAR)) {
            KeyWaitSet(winP, 1);
            winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
        }
        HuPrcVSleep();
    }
    if(winP->pauseF) {
        HuPrcSleep(-1);
    }
    if(winP->choiceNo != HUWIN_CHOICE_NONE) {
        winChoice[winP->no] = winChoiceLast = HuWinChoiceGet(winP->winId, winP->choiceNo);
    }
    if(winP->speakerNo != HUWIN_SPEAKER_NONE) {
        HuWinExClose(winP->winId);
    }
    HuPrcEnd();
}

static void DestroyWin(void)
{
    HUPROCESS *proc = HuPrcCurrentGet();
    MBWIN *winP = proc->property;
    int i;
    if(winP->winId >= 0) {
        if(winP->speakerNo == HUWIN_SPEAKER_NONE) {
            HuWinExKill(winP->winId);
        } else {
            HuWinExKill(winP->winId);
            winP->speakerNo = HUWIN_SPEAKER_NONE;
        }
        winP->winId = HUWIN_NONE;
    }
    winP->mess = 0;
    winP->proc = NULL;
    if(winOnF[winP->no]) {
        winOnF[winP->no] = FALSE;
        for(i=0; i<winSp; i++) {
            if(winStack[i] == winP->no) {
                break;
            }
        }
        for(; i<winSp-1; i++) {
            winStack[i] = winStack[i+1];
        }
        winSp--;
    }
    mbWinNum--;
}

int MBWinCreate(int type, u32 mess, int speakerNo)
{
    MBWIN *winP;
    int i;
    int no;
    HuVec2F size;
    
    for(i=0; i<MBWIN_MAX; i++) {
        winP = &mbWinData[mbWinNo];
        if(winP->proc == NULL) {
            break;
        }
        mbWinNo = (mbWinNo+1)%MBWIN_MAX;
    }
    if(i >= MBWIN_MAX) {
        return MBWIN_NONE;
    }
    no = mbWinNo;
    winP->proc = MBPrcCreate(MBWinProc, 8206, 16384);
    MBPrcDestructorSet(winP->proc, DestroyWin);
    winP->winId = HUWIN_NONE;
    winP->mess = mess;
    winP->no = no;
    winP->attr = HUWIN_ATTR_NONE;
    winP->mesSpeed = 0;
    winP->prio = -1;
    winP->type = type;
    winP->speakerNo = speakerNo;
    winP->pauseF = winP->choiceF = FALSE;
    winP->choiceNo = -1;
    winP->scale.x = winP->scale.y = 1;
    winP->comKeyHook = NULL;
    HuWinMesMaxSizeGet(1, &size, winP->mess);
    if(winP->type != MBWIN_TYPE_HELP) {
        winP->pos = winPosTbl[winP->type];
        winP->size = winSizeTbl[winP->type];
    } else {
        winP->pos.x = HU_DISP_CENTERX-((size.x/2)-16);
        winP->pos.y = HU_DISP_HEIGHT-176;
        winP->size = size;
    }
    winChoice[winP->no] = 0;
    for(i=0; i<HUWIN_CHOICE_MAX; i++) {
        winP->choiceDisable[i] = FALSE;
    }
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        winP->insertMes[i] = MBWIN_MES_NONE;
    }
    winP->disablePlayer = HUWIN_PLAYER_NONE;
    winP->proc->property = winP;
    winStack[winSp++] = no;
    MBWinMesSpeedSet(no, GWMessSpeedGet());
    winP->playerNo = (_CheckFlag(FLAG_BOARD_TUTORIAL) == FALSE) ? GwSystem.turnPlayerNo : -1;
    MBWinPlayerDisable(no, winP->playerNo);
    winOnF[winP->no] = TRUE;
    mbWinNum++;
    mbWinNo = (mbWinNo+1)%MBWIN_MAX;
    return no;
}

int MBWinCreateChoice(int type, u32 mess, int speakerNo, int choiceNo)
{
    int winNo = MBWinCreate(type, mess, speakerNo);
    if(winNo >= 0) {
        MBWIN *winP = &mbWinData[winNo];
        winP->choiceF = TRUE;
        winP->choiceNo = choiceNo;
        winP->attr |= HUWIN_ATTR_NOCANCEL;
    }
    return winNo;
}

int MBWinCreateHelp(u32 mess)
{
    int winNo = MBWinCreate(MBWIN_TYPE_HELP, mess, HUWIN_SPEAKER_NONE);
    if(winNo >= 0) {
        MBWIN *winP = &mbWinData[winNo];
        winP->pauseF = TRUE;
    }
    return winNo;
}

MBWIN *MBWinGet(int winNo)
{
    return &mbWinData[winNo];
}

void MBWinKill(s16 winNo)
{
    MBWIN *winP = &mbWinData[winNo];
    int i;
    if(winP == NULL) {
        return;
    }
    if(winOnF[winNo] == FALSE) {
        return;
    }
    if(winP->proc) {
        HuPrcKill(winP->proc);
    }
    winOnF[winP->no] = FALSE;
    for(i=0; i<winSp; i++) {
        if(winStack[i] == winP->no) {
            break;
        }
    }
    for(; i<winSp-1; i++) {
        winStack[i] = winStack[i+1];
    }
    winSp--;
}

void MBTopWinKill(void)
{
    if(winSp == 0) {
        return;
    }
    MBWinKill(winStack[winSp-1]);
}

void MBWinKillAll(void)
{
    int i;
    for(i=0; i<winSp; i++) {
        MBWinKill(winStack[i]);
    }
}

void MBWinPosSet(s16 winNo, s16 posX, s16 posY)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->pos.x = posX;
    winP->pos.y = posY;
    if(winP->winId >= 0) {
        HuWinPosSet(winP->winId, posX, posY);
    }
}

void MBTopWinPosSet(s16 posX, s16 posY)
{
    if(winSp == 0) {
        return;
    }
    MBWinPosSet(winStack[winSp-1], posX, posY);
}

void MBWinPosGet(s16 winNo, HuVec2F *pos)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    pos->x = winP->pos.x;
    pos->y = winP->pos.y;
}

void MBTopWinPosGet(HuVec2F *pos)
{
    if(winSp == 0) {
        return;
    }
    MBWinPosGet(winStack[winSp-1], pos);
}

void MBWinSizeSet(s16 winNo, s16 sizeX, s16 sizeY)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->size.x = sizeX;
    winP->size.y = sizeY;
}

void MBTopWinSizeSet(s16 sizeX, s16 sizeY)
{
    if(winSp == 0) {
        return;
    }
    MBWinSizeSet(winStack[winSp-1], sizeX, sizeY);
}

void MBWinMesMaxSizeGet(s16 winNo, HuVec2F *size)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    HuWinMesMaxSizeGet(1, size, winP->mess);
}

void MBTopWinMesMaxSizeGet(HuVec2F *size)
{
    if(winSp == 0) {
        return;
    }
    MBWinMesMaxSizeGet(winStack[winSp-1], size);
}

void MBWinScaleSet(s16 winNo, float scaleX, float scaleY)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->scale.x = scaleX;
    winP->scale.y = scaleY;
    if(winP->winId >= 0) {
        HuWinScaleSet(winP->winId, scaleX, scaleY);
    }
}

void MBTopWinScaleSet(float scaleX, float scaleY)
{
    if(winSp == 0) {
        return;
    }
    MBWinScaleSet(winStack[winSp-1], scaleX, scaleY);
}

void MBWinScaleGet(s16 winNo, HuVec2F *scale)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    scale->x = winP->scale.x;
    scale->y = winP->scale.y;
}

void MBTopWinScaleGet(HuVec2F *scale)
{
    if(winSp == 0) {
        return;
    }
    MBWinScaleGet(winStack[winSp-1], scale);
}

int MBWinLastChoiceGet(void)
{
    return winChoiceLast;
}

int MBWinChoiceGet(s16 winNo)
{
    return winChoice[winNo];
}

void MBWinPause(s16 winNo)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->pauseF = TRUE;
}

void MBTopWinPause(void)
{
    if(winSp == 0) {
        return;
    }
    MBWinPause(winStack[winSp-1]);
}

void MBWinInsertMesSet(s16 winNo, u32 insertMes, int insertMesNo)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->insertMes[insertMesNo] = insertMes;
}

void MBTopWinInsertMesSet(u32 insertMes, int insertMesNo)
{
    if(winSp == 0) {
        return;
    }
    MBWinInsertMesSet(winStack[winSp-1], insertMes, insertMesNo);
}

BOOL MBWinDoneCheck(s16 winNo)
{
    return winOnF[winNo] == FALSE;
}

BOOL MBTopWinDoneCheck(void)
{
    if(winSp == 0) {
        return TRUE;
    }
    return MBWinDoneCheck(winStack[winSp-1]);
}

void MBWinWait(s16 winNo)
{
    while(winOnF[winNo]) {
        HuPrcVSleep();
    }
}

void MBTopWinWait(void)
{
    if(winSp == 0) {
        return;
    }
    MBWinWait(winStack[winSp-1]);
}

void MBWinAttrSet(s16 winNo, u32 attr)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->attr |= attr;
    if(winP->winId >= 0) {
        HuWinAttrSet(winP->winId, winP->attr);
    }
}

void MBTopWinAttrSet(u32 attr)
{
    if(winSp == 0) {
        return;
    }
    MBWinAttrSet(winStack[winSp-1], attr);
}

void MBWinAttrReset(s16 winNo, u32 attr)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->attr &= ~attr;
    if(winP->winId >= 0) {
        HuWinAttrSet(winP->winId, winP->attr);
    }
}

void MBTopWinAttrReset(u32 attr)
{
    if(winSp == 0) {
        return;
    }
    MBWinAttrReset(winStack[winSp-1], attr);
}

void MBWinChoiceDisable(s16 winNo, int choiceNo)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->choiceDisable[choiceNo] = TRUE;
}

void MBTopWinChoiceDisable(int choiceNo)
{
    if(winSp == 0) {
        return;
    }
    MBWinChoiceDisable(winStack[winSp-1], choiceNo);
}

void MBWinMesSpeedSet(s16 winNo, int speed)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->mesSpeed = winSpeedTbl[speed];
    if(winP->winId >= 0) {
        HuWinMesSpeedSet(winP->winId, winP->mesSpeed);
    }
}

void MBTopWinMesSpeedSet(int speed)
{
    if(winSp == 0) {
        return;
    }
    MBWinMesSpeedSet(winStack[winSp-1], speed);
}

void MBWinMesColSet(s16 winNo, int mesCol)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    if(winP->winId >= 0) {
        HuWinMesColSet(winP->winId, mesCol);
    }
}

void MBTopWinMesColSet(int mesCol)
{
    if(winSp == 0) {
        return;
    }
    MBWinMesColSet(winStack[winSp-1], mesCol);
}

s16 MBWinChoiceNowGet(s16 winNo)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP != NULL && winP->winId >= 0) {
        return HuWinChoiceNowGet(winP->winId);
    } else {
        return -1;
    }
}

s16 MBTopWinChoiceNowGet(void)
{
    if(winSp == 0) {
        return 0;
    }
    return MBWinChoiceNowGet(winStack[winSp-1]);
}

void MBWinPriSet(s16 winNo, s16 prio)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->prio = prio;
    if(winP->winId >= 0) {
        HuWinPriSet(winP->winId, winP->prio);
    }
}

void MBTopWinPriSet(s16 prio)
{
    if(winSp == 0) {
        return;
    }
    MBWinPriSet(winStack[winSp-1], prio);
}

s16 MBWinChoiceNowGet2(s16 winNo)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP != NULL && winP->winId >= 0) {
        return HuWinChoiceNowGet(winP->winId);
    } else {
        return -1;
    }
}

s16 MBTopWinChoiceNowGet2(void)
{
    if(winSp == 0) {
        return 0;
    }
    return MBWinChoiceNowGet(winStack[winSp-1]);
}


void MBWinPlayerDisable(s16 winNo, int playerNo)
{
    MBWIN *winP = &mbWinData[winNo];
    int i;
    if(winP == NULL) {
        return;
    }
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        winP->disablePlayer = ~HUWIN_PLAYER_1;
        winP->playerNo = 0;
        return;
    } else if(playerNo == -1) {
        winP->disablePlayer = HUWIN_PLAYER_ALL;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(GwPlayer[i].comF == FALSE) {
                winP->disablePlayer &= ~(1 << GwPlayer[i].padNo);
            }
        }
    } else {
        if(GwPlayer[playerNo].comF) {
            winP->disablePlayer = HUWIN_PLAYER_ALL;
        } else {
            winP->disablePlayer = ~(1 << GwPlayer[playerNo].padNo);
        }
    }
    winP->playerNo = playerNo;
}

void MBTopWinPlayerDisable(int playerNo)
{
    if(winSp == 0) {
        return;
    }
    MBWinPlayerDisable(winStack[winSp-1], playerNo);
}

static void KeyWaitInit(MBWIN *winP)
{
    int waitNum;
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
       return; 
    }
    waitNum = HuWinKeyWaitNumGet(winP->mess);
    if(waitNum) {
        HuWinComKeyReset();
        KeyWaitSet(winP, waitNum);
    }
}

static void KeyWaitSet(MBWIN *winP, int keyWaitNum)
{
    s32 key[GW_PLAYER_MAX] = {};
    int i;
    int delay;
    if(keyWaitNum == 0) {
        return;
    }
    if(MBPlayerAllComCheck()) {
        key[0] = key[1] = key[2] = key[3] = PAD_BUTTON_A;
    } else {
        if(winP->playerNo == -1) {
            return;
        }
        if(!GwPlayer[winP->playerNo].comF) {
            return;
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            int padNo = GwPlayer[i].padNo;
            if(winP->playerNo == i) {
                key[padNo] |= PAD_BUTTON_A;
            }
        }
    }
    delay = GWComKeyDelayGet()*1.5f;
    for(i=0; i<keyWaitNum; i++) {
        HuWinComKeyWait(key[0], key[1], key[2], key[3], delay);
    }
}

void MBWinComKeyHookSet(s16 winNo, MBWINCOMKEYHOOK comKeyHook)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP == NULL) {
        return;
    }
    winP->comKeyHook = comKeyHook;
}

void MBTopWinComKeyHookSet(MBWINCOMKEYHOOK comKeyHook)
{
    if(winSp == 0) {
        return;
    }
    MBWinComKeyHookSet(winStack[winSp-1], comKeyHook);
}

void MBWinDispSet(s16 winNo, BOOL dispF)
{
    MBWIN *winP = &mbWinData[winNo];
    if(winP->proc == NULL || winP->winId < 0) {
        return;
    }
    if(dispF) {
        HuWinDispOn(winP->winId);
    } else {
        HuWinDispOff(winP->winId);
    }
}

void MBTopWinDispSet(BOOL dispF)
{
    if(winSp == 0) {
        return;
    }
    MBWinDispSet(winStack[winSp-1], dispF);
}

void MBWinPauseHook(BOOL dispF)
{
    int i;
    for(i=0; i<MBWIN_MAX; i++) {
        MBWinDispSet(i, dispF);
    }
}

HUWINID MBWinIDGet(s16 winNo)
{
    MBWIN *winP = &mbWinData[winNo];
    return winP->winId;
}

HUWINID MBTopWinIDGet(void)
{
    if(winSp == 0) {
        return HUWIN_NONE;
    }
    return MBWinIDGet(winStack[winSp-1]);
}

void MBWinCenterGet(s16 winNo, HuVec2F *pos)
{
    MBWIN *winP = &mbWinData[winNo];
    HuVec2F size;
    MBWinMesMaxSizeGet(winNo, &size);
    if(winP->size.x > size.x) {
        size.x = winP->size.x;
    }
    if(winP->size.y > size.y) {
        size.y = winP->size.y;
    }
    pos->x = HU_DISP_CENTERX-(size.x/2);
    pos->y = HU_DISP_CENTERY-(size.y/2);
}