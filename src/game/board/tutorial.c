#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/board/audio.h"
#include "game/board/guide.h"
#include "game/board/player.h"
#include "game/board/capsule.h"
#include "game/board/star.h"
#include "game/board/mgcall.h"

#include "game/wipe.h"
#include "game/esprite.h"

#include "messnum/board_tutorial.h"

static int tutorialExecNum[TUTORIAL_INST_MAX];
static s16 tutorialSprId[18];

static s16 tutorialSprNum;
static HUPROCESS *tutorialWatchProc;
static BOOL tutorialExitF;

typedef int (*TUTORIALFUNC)(int execNum);

static void TutorialWatchProcExec(void);

static int TutorialExecTurn(int execNum);
static int TutorialExecSai(int execNum);
static int TutorialExecWalkStart(int execNum);
static int TutorialExecCapsule(int execNum);
static int TutorialExecMasuCapsule(int execNum);
static int TutorialExecWalkEnd(int execNum);
static int TutorialExecWalkEndEvent(int execNum);
static int TutorialExecPathChoice(int execNum);
static int TutorialExecTurnEnd(int execNum);
static int TutorialExecSaiHidden(int execNum);
static int TutorialExecCapsuleSet(int execNum);
static int TutorialExecCapsuleUse(int execNum);

static TUTORIALFUNC tutorialExecTbl[TUTORIAL_INST_MAX] = {
    TutorialExecTurn,
    NULL,
    TutorialExecSai,
    TutorialExecWalkStart,
    TutorialExecCapsule,
    TutorialExecMasuCapsule,
    TutorialExecWalkEnd,
    TutorialExecWalkEndEvent,
    TutorialExecPathChoice,
    TutorialExecTurnEnd,
    TutorialExecSaiHidden,
    TutorialExecCapsuleSet,
    TutorialExecCapsuleUse
};

void MBTutorialWatchProcCreate(void)
{
    memset(&tutorialExecNum[0], 0, sizeof(tutorialExecNum));
    tutorialSprNum = 0;
    tutorialExitF = FALSE;
    tutorialWatchProc = MBPrcCreate(TutorialWatchProcExec, 8208, 8192);
}

static void KillTutorialSpr(void);

static void TutorialWatchProcExec(void)
{
    s16 winNo;
    HuVec2F size;
    HuPrcSleep(60);
    winNo = MBWinCreateHelp(MESS_BOARD_TUTORIAL_EXIT);
    MBWinMesMaxSizeGet(winNo, &size);
    MBWinPosSet(winNo, 288.0f-((s16)size.x/2), 400);
    while(!tutorialExitF) {
        u16 btn = HuPadBtnDown[0];
        if(btn & PAD_BUTTON_START) {
            tutorialExitF = TRUE;
            break;
        }
        HuPrcVSleep();
    }
    MBPauseFlagSet();
    while(HuARDMACheck()) {
        HuPrcVSleep();
    }
    WipeWait();
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    WipeWait();
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    MBWinKill(winNo);
    MBGuideKill();
    KillTutorialSpr();
    MBPauseWatchProcStop();
    HuPrcSleep(-1);
}


BOOL MBTutorialExec(int instNo)
{
    if(!tutorialExecTbl[instNo]) {
        return FALSE;
    }
    tutorialExecTbl[instNo](tutorialExecNum[instNo]++);
}

static int TutorialExecTurn(int execNum)
{
    switch(execNum) {
        case 6:
            MBPlayerCapsuleAdd(GwSystem.turnPlayerNo, CAPSULE_KINOKO);
            break;
       
        case 7:
            MBPlayerCapsuleAdd(GwSystem.turnPlayerNo, CAPSULE_HAMMERBRO);
            break;
        
        case 8:
            MBCapMachineTutorialExec();
            MBMgCallTutorialExec();
            MBStarTutorialExec();
            tutorialExitF = TRUE;
            HuPrcSleep(-1);
            break;
    }
}

static int TutorialExecSai(int execNum)
{
    static const int resultTbl[8] = {
        0,
        2,
        3,
        4,
        3,
        2,
        -1,
        3
    };
    if(execNum == 0) {
        MBAudFXPlay(MSM_SE_GUIDE_49);
        MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_SAI, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
    }
    return resultTbl[execNum];
}

static int TutorialExecWalkStart(int execNum)
{
    if(execNum == 0) {
        MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_PLAYER1, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
    }
    return 0;
}

static int TutorialExecCapsule(int execNum)
{
    switch(execNum) {
        case 0:
            MBAudFXPlay(MSM_SE_GUIDE_50);
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULESYS, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            break;
       
        case 1:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE_MASU, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE_MASU_COIN, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            break;
    }
}

static int TutorialExecMasuCapsule(int execNum)
{
    if(execNum == 7) {
        MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE_MASU_EVENT, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
    }
    return 0;
}

static int TutorialExecWalkEnd(int execNum)
{
    switch(execNum) {
        case 5:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_SAIHIDDEN_COIN, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            break;
    }
}

static int TutorialExecWalkEndEvent(int execNum)
{
    s16 sprId;
    switch(execNum) {
        case 0:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MASU, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            sprId = MBTutorialSprDispOn(W10_ANM_masuBlue);
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MASUBLUE, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTutorialSprDispOff(sprId);
            break;
       
        case 1:
            sprId = MBTutorialSprDispOn(W10_ANM_masuRed);
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MASURED, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTutorialSprDispOff(sprId);
            break;
       
        case 2:
            sprId = MBTutorialSprDispOn(W10_ANM_masuHatena);
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MASUHATENA, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTutorialSprDispOff(sprId);
            break;
       
        case 3:
            sprId = MBTutorialSprDispOn(W10_ANM_masuKoopa);
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MASUKOOPA, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_TURNEND, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTutorialSprDispOff(sprId);
            break;
       
        case 4:
            sprId = MBTutorialSprDispOn(W10_ANM_masuDonkey);
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_MASUDONKEY, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTutorialSprDispOff(sprId);
            break;
        
        case 6:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE_SELFUSE_EFFECT, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            break;
        
        case 7:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE_MASU_SELF, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            break;
    }
    return 0;
}

static int TutorialExecSaiHidden(int execNum)
{
    if(execNum == 0) {
        MBAudFXPlay(MSM_SE_GUIDE_49);
        MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_SAIHIDDEN, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
    }
    return 0;
}

static int TutorialExecPathChoice(int execNum)
{
    static const int resultTbl[3] = {
        1,
        0,
        1
    };
    if(execNum < 3) {
        return resultTbl[execNum];
    } else {
        return 0;
    }
}

static int TutorialExecTurnEnd(int execNum)
{
    switch(execNum) {
        case 0:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_TURNEND, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            return 0;
    }
}

static int TutorialExecCapsuleSet(int execNum)
{
    static const int resultTbl[3] = {
        0,
        1,
        0
    };
    if(execNum < 3) {
        return resultTbl[execNum];
    } else {
        return 0;
    }
}

static int TutorialExecCapsuleUse(int execNum)
{
    switch(execNum) {
        case 0:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE_SELFUSE, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            break;
        
        case 1:
            MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_CAPSULE_MASU_MAXDIST, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            break;
    }
}


s16 MBTutorialSprDispOn(unsigned int dataNum)
{
    s16 sprId = tutorialSprId[tutorialSprNum++] = espEntry(dataNum, 1500, 0);
    int i;
    espPosSet(sprId, 288, 176);
    
    for(i=0; i<=12u; i++) {
        float scale = i/12.0f;
        espScaleSet(sprId, scale, scale);
        HuPrcVSleep();
    }
    return sprId;
}

void MBTutorialSprDispOff(s16 sprId)
{
    int i;
    for(i=0; i<=12u; i++) {
        float scale = 1-(i/12.0f);
        espScaleSet(sprId, scale, scale);
        HuPrcVSleep();
    }
}

static void KillTutorialSpr(void)
{
    int i;
    for(i=0; i<tutorialSprNum; i++) {
        espKill(tutorialSprId[i]);
    }
}

BOOL MBTutorialExitFlagGet(void)
{
    return tutorialExitF;
}