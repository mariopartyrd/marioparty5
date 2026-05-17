#include "game/board/story.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/window.h"
#include "game/board/player.h"
#include "game/board/masu.h"
#include "game/board/opening.h"
#include "game/board/tutorial.h"
#include "game/board/coin.h"

#include "game/wipe.h"
#include "game/esprite.h"
#include "game/sprite.h"
#include "game/printfunc.h"

#include "game/board/telop.h"
#include "game/board/capsule.h"
#include "game/board/pad.h"

#include "datanum/bdemo.h"
#include "datanum/effect.h"

#include "messnum/board_story.h"
#include "messnum/board_name.h"

static const u32 openingMessTbl[5] = {
    MESS_BOARD_STORY_SCENE0,
    MESS_BOARD_STORY_SCENE1,
    MESS_BOARD_STORY_SCENE2,
    MESS_BOARD_STORY_SCENE3,
    MESS_BOARD_STORY_SCENE4
};

static const int openingMiniKoopaFXTbl[3] = {
    MSM_SE_VOICE_MINIKOOPA+5,
    MSM_SE_VOICE_MINIKOOPAG+5,
    MSM_SE_VOICE_MINIKOOPAB+5
};

static const u32 sceneMessTbl[5] = {
    MESS_BOARD_STORY_SCENE0+8,
    MESS_BOARD_STORY_SCENE1+8,
    MESS_BOARD_STORY_SCENE2+8,
    MESS_BOARD_STORY_SCENE3+8,
    MESS_BOARD_STORY_SCENE4+8
};

static const int sceneMiniKoopaFXTbl[3] = {
    MSM_SE_VOICE_MINIKOOPA+5,
    MSM_SE_VOICE_MINIKOOPAG+5,
    MSM_SE_VOICE_MINIKOOPAB+5
};

static const int scene4MiniKoopaFXTbl[3] = {
    MSM_SE_VOICE_MINIKOOPA+5,
    MSM_SE_VOICE_MINIKOOPAG+5,
    MSM_SE_VOICE_MINIKOOPAB+5
};

static const HuVecF winCameraOfs = { 0, 150, 0 };

static const int miniKoopaGroup1MotTbl[] = {
    BDEMO_HSF_minikoopaLogoStartRed,
    BDEMO_HSF_minikoopaLogoStartGreen,
    BDEMO_HSF_minikoopaLogoStartBlue,
    BDEMO_HSF_minikoopaArmUp,
    BDEMO_HSF_minikoopaSalute,
    BDEMO_HSF_minikoopaWalkShake,
    HU_DATANUM_NONE
};

static const int miniKoopaGroup2MotTbl[] = {
    BDEMO_HSF_minikoopaLogoStartRed,
    BDEMO_HSF_minikoopaLogoStartGreen,
    BDEMO_HSF_minikoopaLogoStartBlue,
    BDEMO_HSF_minikoopaArmUp,
    BDEMO_HSF_minikoopaSalute,
    BDEMO_HSF_minikoopaStackIdle,
    BDEMO_HSF_minikoopaShuffleArmIn,
    BDEMO_HSF_minikoopaShuffleArmOut,
    BDEMO_HSF_minikoopaShuffleJump,
    HU_DATANUM_NONE
};

static const int miniKoopaGroup3MotTbl[] = {
    BDEMO_HSF_minikoopaSplitStart,
    BDEMO_HSF_minikoopaSplitMissing,
    BDEMO_HSF_minikoopaSplitStun,
    BDEMO_HSF_minikoopaArmUp,
    BDEMO_HSF_minikoopaSalute,
    BDEMO_HSF_minikoopaSplitPunchR,
    BDEMO_HSF_minikoopaSplitPunchL,
    BDEMO_HSF_minikoopaSplitHit,
    HU_DATANUM_NONE
};

static const int miniKoopaLogoFileTbl[3] = {
    BDEMO_HSF_minikoopaLogoR,
    BDEMO_HSF_minikoopaLogoG,
    BDEMO_HSF_minikoopaLogoB
};

static HuVecF storyCamRot = { -10, 0, 0 };
static HuVecF storyCamOfs = { 0, 100, 0 };

static int boardSndGrp[7] = {
    MSM_GRP_BD1,
    MSM_GRP_BD2,
    MSM_GRP_BD3,
    MSM_GRP_BD4,
    MSM_GRP_BD5,
    MSM_GRP_BD6,
    MSM_GRP_BD7
};

static void StoryOpeningScene0Exec(void);
static void StoryOpeningScene1Exec(void);
static void StoryOpeningScene2Exec(void);
static void StoryOpeningScene3Exec(void);
static void StoryOpeningScene4Exec(void);

static void StoryWinScene0Exec(int no);
static void StoryWinScene1Exec(int no);
static void StoryWinScene2Exec(int no);
static void StoryWinScene3Exec(int no);
static void StoryWinScene4Exec(int no);

typedef void (*STORY_OPENING_FUNC)(void);
typedef void (*STORY_WIN_FUNC)(int no);

static STORY_OPENING_FUNC openingSceneTbl[5] = {
    StoryOpeningScene0Exec,
    StoryOpeningScene1Exec,
    StoryOpeningScene2Exec,
    StoryOpeningScene3Exec,
    StoryOpeningScene4Exec
};

static STORY_WIN_FUNC winSceneTbl[5] = {
    StoryWinScene0Exec,
    StoryWinScene1Exec,
    StoryWinScene2Exec,
    StoryWinScene3Exec,
    StoryWinScene4Exec
};

static OMOBJ *storyKoopaObj;
static OMOBJ *storyMiniKoopaObj;
static OMOBJ *storyPlayerObj;
static int miniKoopaGrpNum;
static HUPROCESS *storyProc;
static OMOBJ *storyMiniKoopaGroupObj[3];

static void StoryOpeningExec(void);
static void StoryOpeningDestroy(void);
static void StoryKoopaExitExec(void);

void MBStoryOpeningExec(void)
{
    MBDataClose();
    GwSystem.turnPlayerNo = -1;
    HuAudSndGrpSet(MSM_GRP_STORY);
    storyProc = MBPrcCreate(StoryOpeningExec, 8199, 16384);
    MBPrcDestructorSet(storyProc, StoryOpeningDestroy);
    while(storyProc != NULL) {
        HuPrcVSleep();
    }
    HuAudSndGrpSet(boardSndGrp[MBBoardNoGet()]);
    
}

static int StorySceneSelExec(int startScene);
static void StoryPlayerCreate(void);
static void StoryKoopaCreate(void);
static void StoryMiniKoopaCreate(void);
static int StoryObjMotCreate(OMOBJ *obj, int dataNum, u32 attr, BOOL lockDisable);

static void StoryObjPosSet(OMOBJ *obj, float x, float y, float z);
static void StoryObjPosGet(OMOBJ *obj, HuVecF *pos);

static void StoryObjDispSet(OMOBJ *obj, BOOL dispF);
static void StoryObjRotStartWalk(OMOBJ *obj, float rotY, s16 speed);
static void StoryObjRotStart(OMOBJ *obj, float rotY, s16 speed, u32 forceIdle);

static void StoryObjMotShiftSet(OMOBJ *obj, int no, u32 attr, BOOL lockDisable);
static void StoryObjMotNoSet(OMOBJ *obj, int no, u32 attr, BOOL lockDisable);

static void StoryObjMotWait(OMOBJ *obj);

static void StoryKoopaSePlay(int seId);
static void StoryMiniKoopaSePlay(int seId);
static void StoryMiniKoopaGroupSePlay(int no, int seId);

static void StoryPlayerFallStart(float posY);

static void StoryMiniKoopaArmDownStart(void);
static void StoryKoopaAngryStart(void);
static void StoryMesPlayerNameInsert(int playerNo);
static void StoryMiniKoopaWarnStart(void);
static void StoryKoopaFlexStart(void);
static void StoryMiniKoopaGroupMotWait(void);

static void StoryObjKill(OMOBJ *obj);

static void StoryPlayerLookUpReset(void);

static void StoryKoopaStareStart(void);
static void StoryMiniKoopaGroupSaluteSet(void);
static void StoryPlayerLookUpStart(void);
static void StoryKoopaJumpStart(void);

static void StoryOpeningExec(void)
{
    int i; //r31
    u32 messBase; //r30
    int sceneNo; //r29
    int playerNo; //r27
    int asyncStat; //r26
    
    HuVecF pos; //sp+0x38
    HuVecF focusPos; //sp+0x2C
    HuVecF viewRot; //sp+0x20
    HuVecF viewPos; //sp+0x14
    HuVecF masuPos; //sp+0x8
    
    sceneNo = 0;
    playerNo = MBPlayerStoryPlayerGet();
    if(GwCommon.storyNo < 0) {
        sceneNo = StorySceneSelExec(sceneNo);
    } else {
        sceneNo = GwCommon.storyNo;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i)) {
            MBModelDispSet(MBPlayerModelGet(i), FALSE);
        }
        
    }
    StoryPlayerCreate();
    StoryKoopaCreate();
    StoryMiniKoopaCreate();
    messBase = openingMessTbl[sceneNo];
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &pos);
    MBCameraZoomSet(MBOpeningViewZoomGet());
    MBOpeningViewRotGet(&viewRot);
    MBCameraRotSetV(&viewRot);
    MBOpeningViewFocusGet(&focusPos);
    MBCameraPosTargetSetV(&focusPos);
    StoryObjPosSet(storyPlayerObj, pos.x-250, pos.y+500, pos.z);
    StoryObjDispSet(storyPlayerObj, FALSE);
    StoryObjRotStartWalk(storyKoopaObj, -80, 0);
    StoryObjPosSet(storyKoopaObj, pos.x+250, pos.y, pos.z-100);
    StoryObjRotStartWalk(storyMiniKoopaObj, -80, 0);
    StoryObjPosSet(storyMiniKoopaObj, pos.x+50, pos.y, pos.z+150);
    HuDataDirRead(DATA_effect);
    asyncStat = MBDataDirReadAsync(DATA_capsule);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    MBDataAsyncWait(asyncStat);
    MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_BOARD_STORY_OP, 127, 0);
    MBTelopLogoCreate(MBBoardNoGet(), FALSE);
    HuPrcSleep(72);
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &masuPos);
    MBCameraFocusSet(MB_MODEL_NONE);
    MBCameraSkipSet(TRUE);
    viewPos.x = pos.x+150;
    viewPos.y = pos.y;
    viewPos.z = pos.z;
    MBCameraPosViewSet(&viewPos, &storyCamRot, &storyCamOfs, 1400, -1, 180);
    MBCameraMotionWait();
    HuPrcSleep(60);
    StoryObjMotShiftSet(storyKoopaObj, 10, HU3D_MOTATTR_LOOP, TRUE);
    if(sceneNo < 2) {
        StoryKoopaSePlay(MSM_SE_GUIDE_12);
    } else {
        StoryKoopaSePlay(MSM_SE_GUIDE_14);
    }
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+0, HUWIN_SPEAKER_NONE);
    MBTopWinInsertMesSet(MESS_BOARD_NAME_W01+MBBoardNoGet(), 0);
    MBTopWinWait();
    StoryObjMotShiftSet(storyMiniKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+1, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBCameraPosViewSet(&pos, &storyCamRot, &storyCamOfs, 1600, -1, 30);
    MBCameraMotionWait();
    StoryObjDispSet(storyPlayerObj, TRUE);
    StoryPlayerFallStart(pos.y);
    StoryObjMotWait(storyPlayerObj);
    StoryObjMotShiftSet(storyKoopaObj, 1, HU3D_MOTATTR_LOOP, FALSE);
    StoryMiniKoopaArmDownStart();
    HuPrcSleep(60);
    StoryKoopaAngryStart();
    if(sceneNo < 2) {
        StoryKoopaSePlay(MSM_SE_GUIDE_14);
    }
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+2, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+3, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 1, HU3D_MOTATTR_LOOP, FALSE);
    StoryObjMotCreate(storyPlayerObj, CHARMOT_HSF_c000m1_484, HU3D_MOTATTR_LOOP, TRUE);
    StoryObjRotStart(storyPlayerObj, 80, 60, FALSE);
    StoryObjMotWait(storyPlayerObj);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaWarnStart();
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+4, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+5, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryKoopaFlexStart();
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+6, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 11, HU3D_MOTATTR_LOOP, TRUE);
    StoryObjRotStart(storyKoopaObj, 0, 30, FALSE);
    StoryObjRotStartWalk(storyMiniKoopaObj, 0, 30);
    StoryObjMotWait(storyKoopaObj);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+7, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    openingSceneTbl[sceneNo]();
    StoryKoopaExitExec();
    HuDataDirClose(DATA_bdemo);
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    StoryObjRotStartWalk(storyPlayerObj, 0, 1);
    for(i=0; i<3; i++) {
        StoryObjRotStartWalk(storyMiniKoopaGroupObj[i], 0, 1);
    }
    StoryObjMotWait(storyPlayerObj);
    StoryMiniKoopaGroupMotWait();
    StoryObjKill(storyPlayerObj);
    StoryObjKill(storyKoopaObj);
    StoryObjKill(storyMiniKoopaObj);
    for(i=0; i<3; i++) {
        StoryObjKill(storyMiniKoopaGroupObj[i]);
    }
    StoryPlayerLookUpReset();
    HuDataDirClose(DATA_effect);
    HuPrcEnd();
}

static void StoryOpeningDestroy(void)
{
    storyProc = NULL;
}

static void StoryKoopaExitExec(void)
{
    int i;
    StoryObjMotShiftSet(storyKoopaObj, 1, HU3D_MOTATTR_LOOP, FALSE);
    StoryKoopaStareStart();
    for(i=0; i<3; i++) {
        StoryObjRotStart(storyMiniKoopaGroupObj[i], 180, 30, FALSE);
    }
    StoryObjMotShiftSet(storyPlayerObj, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_EXIT_PLAYER, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaGroupSaluteSet();
    StoryPlayerLookUpStart();
    StoryKoopaJumpStart();
    for(i=0; i<3; i++) {
        StoryMiniKoopaGroupSePlay(i, openingMiniKoopaFXTbl[i]);
    }
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_EXIT_GRUMBLE, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    HuPrcSleep(60);
}

static void StoryEndExec(void);
static void StoryEndDestroy(void);

void MBStoryEndProcExec(void)
{
    MBDataClose();
    GwSystem.turnPlayerNo = -1;
    storyProc = MBPrcCreate(StoryEndExec, 8199, 16384);
    MBPrcDestructorSet(storyProc, StoryEndDestroy);
    while(storyProc != NULL) {
        HuPrcVSleep();
    }
    MBPauseWatchProcStop();
    HuPrcSleep(-1);
}

static void StoryWinExec(void);
static void StoryLoseExec(void);
static void StoryTieExec(void);
static BOOL StoryObjMotCheck(OMOBJ *obj);

static void StoryEndExec(void)
{
    _SetFlag(FLAG_BOARD_TURN_NOSTART);
    if(GwSystem.turnNo > GwSystem.turnMax) {
        StoryTieExec();
    } else if(MBPlayerCoinGet(MBPlayerStoryPlayerGet()) != 0) {
        StoryWinExec();
    } else {
        StoryLoseExec();
    }
    HuPrcEnd();
}

static void StoryEndDestroy(void)
{
    storyProc = NULL;
}

static void StoryWinExec(void)
{
    int sceneNo; //r31
    s16 espId; //r30
    int playerNo; //r28
    
    HuVecF masuPos; //sp+0x14
    HuVecF pos; //sp+0x8
    
    sceneNo = 0;
    playerNo = MBPlayerStoryPlayerGet();
    if(GwCommon.storyNo < 0) {
        sceneNo = StorySceneSelExec(sceneNo);
    } else {
        sceneNo = GwCommon.storyNo;
    }
    StoryPlayerCreate();
    StoryKoopaCreate();
    StoryMiniKoopaCreate();
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &masuPos);
    MBCameraFocusSet(MB_MODEL_NONE);
    MBCameraSkipSet(FALSE);
    pos.x = masuPos.x+150;
    pos.y = masuPos.y;
    pos.z = masuPos.z;
    MBCameraPosViewSet(&masuPos, &storyCamRot, &storyCamOfs, 1600, -1, 30);
    MBCameraMotionWait();
    StoryObjPosSet(storyPlayerObj, masuPos.x-250, masuPos.y, masuPos.z);
    StoryObjRotStartWalk(storyPlayerObj, 80, 0);
    StoryObjRotStartWalk(storyKoopaObj, -30, 0);
    StoryObjPosSet(storyKoopaObj, masuPos.x+250, masuPos.y, masuPos.z-100);
    StoryObjRotStart(storyMiniKoopaObj, 150, 0, FALSE);
    StoryObjPosSet(storyMiniKoopaObj, masuPos.x+50, masuPos.y, masuPos.z+150);
    StoryObjMotNoSet(storyMiniKoopaObj, 10, HU3D_MOTATTR_LOOP, FALSE);
    espId = espEntry(BOARD_ANM_telopStoryClear, 100, 0);
    espPosSet(espId, 288, 64);
    espDispOff(espId);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_STORY_WIN, 127, 0);
    HuPrcSleep(60);
    winSceneTbl[sceneNo](sceneNo);
    HuPrcSleep(30);
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    MBMusPlayJingle(MSM_STREAM_JNGL_STORY_WIN);
    StoryObjRotStartWalk(storyPlayerObj, 0, 15);
    StoryObjMotWait(storyPlayerObj);
    MBPlayerWinLoseVoicePlay(playerNo, MB_PLAYER_MOT_WIN, CHARVOICEID(4));
    StoryObjMotShiftSet(storyPlayerObj, MB_PLAYER_MOT_WIN, HU3D_MOTATTR_NONE, FALSE);
    MBCameraSkipSet(TRUE);
    MBCameraPlayerViewSet(playerNo, NULL, &winCameraOfs, 900, -1, 60);
    MBCameraMotionWait();
    while(!StoryObjMotCheck(storyPlayerObj)) {
        HuPrcVSleep();
    }
    espDispOn(espId);
    HuPrcSleep(120);
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    StoryObjRotStartWalk(storyPlayerObj, 0, 1);
    StoryObjMotWait(storyPlayerObj);
    StoryObjKill(storyPlayerObj);
    StoryObjKill(storyKoopaObj);
    StoryObjKill(storyMiniKoopaObj);
    
}

static void StoryMiniKoopaScareStart(void);

static void StoryWinScene0Exec(int no)
{
    static u32 winMess[5] = {
        MESS_BOARD_STORY_SCENE0+13,
        MESS_BOARD_STORY_SCENE1+13,
        MESS_BOARD_STORY_SCENE2+13,
        MESS_BOARD_STORY_SCENE3+13,
        MESS_BOARD_STORY_SCENE4+13
    };
    int playerNo = MBPlayerStoryPlayerGet();
    u32 mess = winMess[no];
    MBWinCreate(MBWIN_TYPE_EVENT, mess+0, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+1, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaArmDownStart();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+6);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+2, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+3, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+4, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
    StoryObjMotShiftSet(storyKoopaObj, 7, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+5, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjRotStart(storyMiniKoopaObj, 150, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 8, HU3D_MOTATTR_NONE, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+6, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -30, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 13, HU3D_MOTATTR_NONE, FALSE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+7, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyMiniKoopaObj, 150, 12, FALSE);
    StoryMiniKoopaScareStart();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+9);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+8, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+9, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+10, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+11, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaScareStart();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+12, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
}

static void StoryWinScene1Exec(int no)
{
    static u32 winMess[5] = {
        MESS_BOARD_STORY_SCENE0+13,
        MESS_BOARD_STORY_SCENE1+13,
        MESS_BOARD_STORY_SCENE2+13,
        MESS_BOARD_STORY_SCENE3+13,
        MESS_BOARD_STORY_SCENE4+13
    };
    int playerNo = MBPlayerStoryPlayerGet();
    u32 mess = winMess[no];
    MBWinCreate(MBWIN_TYPE_EVENT, mess+0, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+1, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaArmDownStart();
    MBWinCreate(MBWIN_TYPE_EVENT, mess+2, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+3, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+4, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
    StoryObjMotShiftSet(storyKoopaObj, 7, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+5, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjRotStart(storyMiniKoopaObj, 150, 12, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+6, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -30, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 14, HU3D_MOTATTR_NONE, FALSE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+7, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyMiniKoopaObj, 150, 12, FALSE);
    StoryMiniKoopaScareStart();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+9);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+8, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+9, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+10, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
}

static void StoryWinScene2Exec(int no)
{
    static u32 winMess[3] = {
        MESS_BOARD_STORY_SCENE0+13,
        MESS_BOARD_STORY_SCENE1+13,
        MESS_BOARD_STORY_SCENE2+13
    };
    int playerNo = MBPlayerStoryPlayerGet();
    u32 mess = winMess[no];
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyMiniKoopaObj, 1, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 7, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjRotStart(storyMiniKoopaObj, 150, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 8, HU3D_MOTATTR_NONE, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+2);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 14, HU3D_MOTATTR_NONE, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyMiniKoopaObj, 1, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyMiniKoopaObj, 9, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+4);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -30, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 14, HU3D_MOTATTR_NONE, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyMiniKoopaObj, 12, HU3D_MOTATTR_NONE, FALSE);
    StoryObjRotStart(storyMiniKoopaObj, 150, 12, FALSE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+6);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
}

static void StoryWinScene3Exec(int no)
{
    static u32 winMess[5] = {
        MESS_BOARD_STORY_SCENE0+13,
        MESS_BOARD_STORY_SCENE1+13,
        MESS_BOARD_STORY_SCENE2+13,
        MESS_BOARD_STORY_SCENE3+13,
        MESS_BOARD_STORY_SCENE4+13
    };
    int playerNo = MBPlayerStoryPlayerGet();
    u32 mess = winMess[no];
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 13, HU3D_MOTATTR_NONE, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaScareStart();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+9);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaArmDownStart();
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
    StoryObjMotShiftSet(storyKoopaObj, 7, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -30, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 14, HU3D_MOTATTR_NONE, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryMiniKoopaScareStart();
    StoryObjRotStart(storyMiniKoopaObj, 150, 12, FALSE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess++, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
}

static void StoryWinScene4Exec(int no)
{
    static u32 winMess[5] = {
        MESS_BOARD_STORY_SCENE0+13,
        MESS_BOARD_STORY_SCENE1+13,
        MESS_BOARD_STORY_SCENE2+13,
        MESS_BOARD_STORY_SCENE3+13,
        MESS_BOARD_STORY_SCENE4+13
    };
    int playerNo = MBPlayerStoryPlayerGet();
    u32 mess = winMess[no];
    MBWinCreate(MBWIN_TYPE_EVENT, mess+0, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+1, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaArmDownStart();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+6);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+2, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+3, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_EVENT, mess+4, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+5, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyKoopaObj);
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+6, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotWait(storyMiniKoopaObj);
    StoryObjMotShiftSet(storyKoopaObj, 7, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+7, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_EVENT, mess+8, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(playerNo);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+9, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryMiniKoopaScareStart();
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+5);
    MBWinCreate(MBWIN_TYPE_EVENT, mess+10, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
}

static void StoryLoseExec(void)
{
    int i;
    HuVecF pos;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBModelDispSet(MBPlayerModelGet(i), FALSE);
    }
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &pos);
    MBCameraFocusSet(MB_MODEL_NONE);
    MBCameraSkipSet(FALSE);
    MBCameraPosViewSet(&pos, &storyCamRot, &storyCamOfs, 1500, -1, 1);
    MBCameraMotionWait();
    StoryKoopaCreate();
    
    StoryObjPosSet(storyKoopaObj, pos.x+150, pos.y, pos.z);
    StoryObjMotCreate(storyKoopaObj, BDEMO_HSF_koopaWin, HU3D_MOTATTR_LOOP, FALSE);
    StoryMiniKoopaCreate();
    StoryObjPosSet(storyMiniKoopaObj, pos.x-150, pos.y, pos.z);
    StoryObjMotCreate(storyMiniKoopaObj, BDEMO_HSF_minikoopaWin, HU3D_MOTATTR_LOOP, FALSE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_STORY_LOSE, 127, 0);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    StoryMiniKoopaSePlay(MSM_SE_VOICE_MINIKOOPAR+0);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_LOSE, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    HuPrcSleep(60);
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    WipeColorSet(255, 255, 255);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    }
    WipeWait();
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    StoryObjKill(storyKoopaObj);
    StoryObjKill(storyMiniKoopaObj);
}

static void StoryCoinArcExec(MBCOINEFF *effP);

static void StoryTieExec(void)
{
    int i; //r31
    s16 effId; //r30
    MBCOINEFF *effP; //r29
    int playerNo; //r28
    
    HuVecF masuPos; //sp+0x14
    HuVecF pos; //sp+0x8
    
    playerNo = MBPlayerStoryPlayerGet();
    StoryPlayerCreate();
    StoryKoopaCreate();
    StoryMiniKoopaCreate();
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &masuPos);
    MBCameraFocusSet(MB_MODEL_NONE);
    MBCameraSkipSet(FALSE);
    pos.x = masuPos.x+150;
    pos.y = masuPos.y;
    pos.z = masuPos.z;
    MBCameraPosViewSet(&masuPos, &storyCamRot, &storyCamOfs, 1600, -1, 30);
    MBCameraMotionWait();
    StoryObjPosSet(storyPlayerObj, masuPos.x-250, masuPos.y, masuPos.z);
    StoryObjRotStartWalk(storyPlayerObj, 30, 0);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryObjRotStart(storyKoopaObj, -30, 0, FALSE);
    StoryObjPosSet(storyKoopaObj, masuPos.x+250, masuPos.y, masuPos.z-100);
    StoryObjRotStartWalk(storyMiniKoopaObj, -30, 0);
    StoryObjPosSet(storyMiniKoopaObj, masuPos.x+50, masuPos.y, masuPos.z+150);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i)) {
            MBModelDispSet(MBPlayerModelGet(i), FALSE);
        }
        GwPlayer[i].dispLightF = FALSE;
    }
    effId = MBCoinEffCreate(20, StoryCoinArcExec);
    effP = MBCoinEffGet(effId);
    effP->work[3] = MBPlayerCoinGet(playerNo);
    MBCoinEffAttrSet(effId, MBCOIN_ATTR_IDLE);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    HuPrcSleep(60);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_GAME_OVER, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_TIE, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjRotStart(storyMiniKoopaObj, -80, 12, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 6, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_TIE_USELESS, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 7, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_TIE_COIN, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    StoryObjMotShiftSet(storyKoopaObj, 4, HU3D_MOTATTR_NONE, FALSE);
    while(!StoryObjMotCheck(storyKoopaObj)) {
        HuPrcVSleep();
    }
    MBCoinEffAttrReset(effId, MBCOIN_ATTR_IDLE);
    while(!MBCoinEffCheck(effId)) {
        HuPrcVSleep();
    }
    StoryObjMotCreate(storyPlayerObj, CHARMOT_HSF_c000m1_485, HU3D_MOTATTR_NONE, FALSE);
    while(!StoryObjMotCheck(storyPlayerObj)) {
        HuPrcVSleep();
    }
    StoryObjRotStart(storyKoopaObj, -30, 12, FALSE);
    StoryObjRotStart(storyMiniKoopaObj, -30, 12, FALSE);
    StoryObjMotShiftSet(storyKoopaObj, 12, HU3D_MOTATTR_LOOP, FALSE);
    StoryObjMotShiftSet(storyMiniKoopaObj, 9, HU3D_MOTATTR_LOOP, FALSE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_TIE_DREAM, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    HuPrcSleep(90);
    WipeColorSet(0, 0, 0);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    StoryObjRotStartWalk(storyPlayerObj, 0, 1);
    StoryObjMotWait(storyPlayerObj);
    StoryObjKill(storyPlayerObj);
    StoryObjKill(storyKoopaObj);
    StoryObjKill(storyMiniKoopaObj);
}

static void StoryCoinArcExec(MBCOINEFF *effP)
{
    MBCOINEFFDATA *effDataP; //r30
    int i; //r29
    int delay; //r28
    HuVecF playerPos; //sp+0x20
    HuVecF koopaPos; //sp+0x14
    HuVecF dir; //sp+0x8

    static int delayTbl[2][2] = {
        { 20, 8 },
        { -1, -1 }
    };
    
    if(effP->work[0] == FALSE) {
        for(i=0; delayTbl[i][0] >= 0; i++) {
            if(delayTbl[i][0] > effP->count) {
                break;
            }
        }
        if(delayTbl[i][0] < 0) {
            delay = 4;
        } else {
            delay = delayTbl[i][1];
        }
        for(effDataP=effP->data, i=0; i<effP->count; i++, effDataP++) {
            effDataP->dispF = FALSE;
            effDataP->rot.y = frandmod(360);
            effDataP->scale.x = effDataP->scale.y = effDataP->scale.z = 0;
            effDataP->timeHit = 0;
        }
        effP->work[0] = TRUE;
        effP->work[1] = effP->work[2] = delay;
    }
    if(effP->work[3] != 0 && effP->work[1]++ >= effP->work[2]) {
        for(effDataP=effP->data, i=0; i<effP->count; i++, effDataP++) {
            if(effDataP->dispF == FALSE) {
                MBCoinEffDispOn(effP->effNo, i);
                break;
            }
        }
        if(i < effP->count) {
            effP->work[3]--;
            MBPlayerCoinAdd(MBPlayerStoryPlayerGet(), -1);
        }
        effP->work[1] = 0;
    }
    effDataP = effP->data;
    StoryObjPosGet(storyPlayerObj, &playerPos);
    StoryObjPosGet(storyKoopaObj, &koopaPos);
    playerPos.y += 150;
    koopaPos.y += 200;
    VECSubtract(&koopaPos, &playerPos, &dir);
    for(i=0; i<effP->count; i++, effDataP++) {
        if(effDataP->dispF != FALSE) {
            if(effDataP->time > 45u) {
                effDataP->hitF = TRUE;
            } else {
                float t = (effDataP->time++)/45.0f;
                effDataP->pos.x = playerPos.x+(t*dir.x);
                effDataP->pos.y = playerPos.y+(t*dir.y)+((2*HuSin(t*180.0f))*100);
                effDataP->pos.z = playerPos.z+(t*dir.z);
                effDataP->rot.y += 5;
            }
        }
    }
}

typedef struct StoryObjWork_s {
    u8 killF : 1;
    u8 doneF : 1;
    u8 jumpF : 1;
    u8 forceIdle : 1;
    s8 no;
    s8 playerNo; //0x02
    float rotYStart; //0x04
    float rotYDist; //0x08
    s8 mode; //0x0C
    HuVecF basePos; //0x10
    HuVecF rot; //0x1C
    HuVecF vel; //0x28
    s16 motNum; //0x34
    float fallY; //0x38
    s16 time; //0x3C
    s16 maxTime; //0x3E
    int unk40;
    int splitPrev; //0x44
    int splitNum; //0x48
    int rotDelay; //0x4C
    int unk50;
    OMOBJFUNC extModeExec; //0x54
} STORY_OBJ_WORK;

static void StoryObjExec(OMOBJ *obj)
{
    STORY_OBJ_WORK *work; //r30
    int i; //r29
    
    float t; //f31
    float s; //f30
    float rotY; //f29
    
    work = obj->data;
    if(work->killF || MBKillCheck()) {
        MBCapsuleEffHookStoryCall(0, 2, MB_MODEL_NONE);
        MBCapsuleEffHookStoryCall(1, 2, MB_MODEL_NONE);
        if(work->playerNo < 0) {
            for(i=0; i<obj->mdlcnt; i++) {
                if(obj->mdlId[0] > 0) {
                    MBModelKill(obj->mdlId[i]);
                }
                
            }
        } else if(obj->mtncnt) {
            for(i=0; i<obj->mtncnt; i++) {
                if(obj->mtnId[i] >= 0) {
                    MBMotionKill(obj->mdlId[0], obj->mtnId[i]);
                    obj->mtnId[i] = MB_MOT_NONE;
                }
            }
        }
        obj->mdlId[0] = MB_MODEL_NONE;
        omDelObj(HuPrcCurrentGet(), obj);
        return;
    }
    switch(work->mode) {
        case 0:
            break;
        
        case 1:
            if(MBMotionShiftIDGet(obj->mdlId[0]) < 0 && MBMotionEndCheck(obj->mdlId[0])) {
                work->mode = 0;
                work->doneF = TRUE;
                if(work->forceIdle) {
                    MBMotionShiftSet(obj->mdlId[0], 1, 0, 8, HU3D_MOTATTR_LOOP);
                }
            }
            break;
       
        case 2:
            if(work->time > work->maxTime) {
                work->mode = 0;
                work->doneF = TRUE;
                if(work->forceIdle) {
                    MBMotionShiftSet(obj->mdlId[0], 1, 0, 8, HU3D_MOTATTR_LOOP);
                }
            } else {
                t = (float)(work->time++)/work->maxTime;
                s = HuSin(t*90);
                rotY = work->rotYStart+(s*work->rotYDist);
                MBModelRotYSet(obj->mdlId[0], rotY);
            }
            break;
        
        default:
            if(work->extModeExec) {
                work->extModeExec(obj);
            }
            break;
    }
}

static void StoryObjPosSet(OMOBJ *obj, float x, float y, float z)
{
    MBModelPosSet(obj->mdlId[0], x, y, z);
}

static void StoryObjPosSetV(OMOBJ *obj, HuVecF *pos)
{
    MBModelPosSetV(obj->mdlId[0], pos);
}

static void StoryObjPosGet(OMOBJ *obj, HuVecF *pos)
{
    MBModelPosGet(obj->mdlId[0], pos);
}

static void StoryObjDispSet(OMOBJ *obj, BOOL dispF)
{
    MBModelDispSet(obj->mdlId[0], dispF);
}

static void StoryObjMotNoSet(OMOBJ *obj, int no, u32 attr, BOOL lockDisable)
{
    MBMODEL *modelP = MBModelGet(obj->mdlId[0]);
    MBMotionNoSet(obj->mdlId[0], no, attr);
    if(lockDisable) {
        Hu3DMotionAttrSet(modelP->motId[no], HU3D_ATTR_MOT_RESET_LOCK);
    } else {
        Hu3DMotionAttrReset(modelP->motId[no], HU3D_ATTR_MOT_RESET_LOCK);
    }
}

static void StoryObjMotShiftSet(OMOBJ *obj, int no, u32 attr, BOOL lockDisable)
{
    MBMODEL *modelP = MBModelGet(obj->mdlId[0]);
    MBMotionShiftSet(obj->mdlId[0], no, 0, 8, attr);
    if(lockDisable) {
        Hu3DMotionAttrSet(modelP->motId[no], HU3D_ATTR_MOT_RESET_LOCK);
    } else {
        Hu3DMotionAttrReset(modelP->motId[no], HU3D_ATTR_MOT_RESET_LOCK);
    }
}

static int StoryObjMotCreate(OMOBJ *obj, int dataNum, u32 attr, BOOL lockDisable)
{
    STORY_OBJ_WORK *work;
    int no;
    MBMODEL *modelP;
    
    modelP = MBModelGet(obj->mdlId[0]);
    work = obj->data;
    obj->mtnId[work->motNum++] = no = MBMotionCreate(obj->mdlId[0], dataNum);
    MBMotionShiftSet(obj->mdlId[0], no, 0, 8, attr);
    if(lockDisable) {
        Hu3DMotionAttrSet(modelP->motId[no], HU3D_ATTR_MOT_RESET_LOCK);
    } else {
        Hu3DMotionAttrReset(modelP->motId[no], HU3D_ATTR_MOT_RESET_LOCK);
    }
    return no;
}

static BOOL StoryObjMotCheck(OMOBJ *obj)
{
    if(MBMotionShiftIDGet(obj->mdlId[0]) < 0 && MBMotionEndCheck(obj->mdlId[0])) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void StoryObjRotStart(OMOBJ *obj, float rotY, s16 speed, u32 walkF)
{
    STORY_OBJ_WORK *work = obj->data;
    float deltaAngle;
    
    if(speed <= 0) {
        work->rotYStart = MBModelRotYSet(obj->mdlId[0], rotY);
        if(walkF) {
            StoryObjMotNoSet(obj, 1, HU3D_MOTATTR_LOOP, FALSE);
        }
        return;
    }
    work->rotYStart = MBModelRotYGet(obj->mdlId[0]);
    deltaAngle = fmod(rotY-work->rotYStart, 360);
    if(deltaAngle < 0) {
        deltaAngle += 360;
    }
    if(deltaAngle > 180) {
        deltaAngle -= 360;
    }
    work->doneF = FALSE;
    work->time = 0;
    work->maxTime = speed;
    work->rotYDist = deltaAngle;
    work->mode = 2;
    work->forceIdle = walkF;
    if(walkF) {
        MBMotionShiftSet(obj->mdlId[0], 2, 0, 8, HU3D_MOTATTR_LOOP);
    }
}

static void StoryObjRotStartWalk(OMOBJ *obj, float rotY, s16 speed)
{
    StoryObjRotStart(obj, rotY, speed, TRUE);
}

static BOOL StoryObjDoneCheck(OMOBJ *obj)
{
    STORY_OBJ_WORK *work = obj->data;
    if(!work->doneF) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void StoryObjMotWait(OMOBJ *obj)
{
    while(!StoryObjDoneCheck(obj)) {
        HuPrcVSleep();
    }
}

static void StoryObjKill(OMOBJ *obj)
{
    STORY_OBJ_WORK *work = obj->data;
    work->killF = TRUE;
}

static void StoryPlayerExec(OMOBJ *obj);

static void StoryPlayerCreate(void)
{
    OMOBJ *obj;
    STORY_OBJ_WORK *work;
    int i;
    storyPlayerObj = obj = MBAddObj(256, 1, 10, StoryObjExec);
    for(i=0; i<obj->mtncnt; i++) {
        obj->mtnId[i] = -1;
    }
    obj->data = work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(STORY_OBJ_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(STORY_OBJ_WORK));
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    work->playerNo = MBPlayerStoryPlayerGet();
    obj->mdlId[0] = MBPlayerModelGet(MBPlayerStoryPlayerGet());
    work->extModeExec = StoryPlayerExec;
}

static void StoryPlayerExec(OMOBJ *obj)
{
    STORY_OBJ_WORK *work; //r31
    
    float angle; //f31
    float angleZ; //f30
    
    Mtx rotMtx; //sp+0x68
    Mtx rotInv; //sp+0x38
    HuVecF pos; //sp+0x2C
    HuVecF koopaPos; //sp+0x20
    HuVecF dir; //sp+0x14
    HuVecF rot; //sp+0x68
    
    static s8 lockHeadF[CHARNO_MAX] = {
        FALSE,
        FALSE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        FALSE,
        FALSE,
        FALSE,
        FALSE,
        FALSE,
        FALSE,
    };
    
    work = obj->data;
    switch(work->mode) {
        case 3:
            MBModelPosGet(obj->mdlId[0], &pos);
            pos.y += work->vel.y;
            work->vel.y -= 1.0f;
            if(!work->jumpF && pos.y < work->fallY+100) {
                StoryObjMotShiftSet(storyPlayerObj, 5, HU3D_MOTATTR_NONE, FALSE);
                work->jumpF = TRUE;
            }
            if(pos.y <= work->fallY) {
                pos.y = work->fallY;
                work->doneF = TRUE;
                work->mode = 4;
            }
            MBModelPosSetV(obj->mdlId[0], &pos);
            break;
        
        case 4:
            if(StoryObjMotCheck(obj)) {
                work->mode = 0;
                StoryObjMotShiftSet(storyPlayerObj, 1, HU3D_MOTATTR_LOOP, FALSE);
            }
            break;
        
        case 5:
            MBModelPosGet(storyKoopaObj->mdlId[0], &koopaPos);
            koopaPos.y += 150;
            MBModelPosGet(obj->mdlId[0], &pos);
            VECSubtract(&koopaPos, &pos, &dir);
            MBModelRotGet(obj->mdlId[0], &rot);
            mtxRot(rotMtx, rot.x, rot.y, rot.z);
            MTXInverse(rotMtx, rotInv);
            MTXMultVec(rotInv, &dir, &dir);
            if(!lockHeadF[GwPlayer[work->playerNo].charNo]) {
                angle = -HuAtan(dir.x, dir.z);
                angleZ = HuAtan(dir.y, -dir.z);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ske_head", HU3D_CONST_FORCE_ROTY, angle);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ske_head", HU3D_CONST_FORCE_ROTZ, angleZ);
            } else {
                angle = HuAtan(dir.x, dir.z);
                angleZ = HuAtan(dir.z, dir.y);
                if((GwPlayer[work->playerNo].charNo == CHARNO_PEACH) || GwPlayer[work->playerNo].charNo == CHARNO_DAISY) {
                    if(angleZ < 80) {
                        angleZ = 80;
                    }
                }
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ske_head", HU3D_CONST_FORCE_ROTX, angle);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ske_head", HU3D_CONST_FORCE_ROTZ, angleZ);
            }
            break;
    }
}

static void StoryPlayerFallStart(float posY)
{
    STORY_OBJ_WORK *work = storyPlayerObj->data;
    StoryObjMotShiftSet(storyPlayerObj, 4, HU3D_MOTATTR_NONE, FALSE);
    work->vel.y = 0;
    work->doneF = FALSE;
    work->mode = 3;
    work->fallY = posY;
}

static void StoryPlayerLookUpStart(void)
{
    STORY_OBJ_WORK *work = storyPlayerObj->data;
    work->mode = 5;
}

static void StoryPlayerLookUpReset(void)
{
    Hu3DMotionNoMotReset(MBModelIdGet(storyPlayerObj->mdlId[0]), "ske_head", HU3D_CONST_FORCE_ROTX);
    Hu3DMotionNoMotReset(MBModelIdGet(storyPlayerObj->mdlId[0]), "ske_head", HU3D_CONST_FORCE_ROTY);
    Hu3DMotionNoMotReset(MBModelIdGet(storyPlayerObj->mdlId[0]), "ske_head", HU3D_CONST_FORCE_ROTZ);
}

static void StoryKoopaExec(OMOBJ *obj);

static void StoryKoopaCreate(void)
{
    OMOBJ *obj;
    STORY_OBJ_WORK *work;
    
    static int motTbl[] = {
        BDEMO_HSF_koopaIdle,
        BDEMO_HSF_koopaWalk,
        BDEMO_HSF_koopaLaugh,
        BDEMO_HSF_koopaJumpStart,
        BDEMO_HSF_koopaJump,
        BDEMO_HSF_koopaFingerPoint,
        BDEMO_HSF_koopaTalk,
        BDEMO_HSF_koopaAngry,
        BDEMO_HSF_koopaAngryEnd,
        BDEMO_HSF_koopaWarn,
        BDEMO_HSF_koopaHandUp,
        BDEMO_HSF_koopaWin,
        BDEMO_HSF_koopaFireStart,
        BDEMO_HSF_koopaFlex,
        HU_DATANUM_NONE
    };
    
    storyKoopaObj = obj = MBAddObj(256, 1, 10, StoryObjExec);
    obj->data = work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(STORY_OBJ_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(STORY_OBJ_WORK));
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = MBModelCreate(BDEMO_HSF_koopa, motTbl, FALSE);
    work->extModeExec = StoryKoopaExec;
    work->playerNo = -1;
    MBCapsuleEffHookStoryCall(0, 4, obj->mdlId[0]);
}

static void StoryKoopaExec(OMOBJ *obj)
{
    STORY_OBJ_WORK *work = obj->data;
    float t;
    HuVecF pos;
    switch(work->mode) {
        case 3:
            if(StoryObjMotCheck(obj)) {
                StoryObjMotShiftSet(obj, 9, HU3D_MOTATTR_LOOP, FALSE);
                work->mode = 0;
                work->doneF = TRUE;
            }
            break;
        
        case 4:
            if(StoryObjMotCheck(obj)) {
                StoryObjMotShiftSet(obj, 1, HU3D_MOTATTR_LOOP, FALSE);
                work->mode = 0;
                work->doneF = TRUE;
            }
            break;
        
        case 5:
            if(work->time > work->maxTime) {
                work->doneF = TRUE;
                work->mode = 0;
            } else {
                t = (float)(work->time++)/work->maxTime;
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTX, -8*t);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTY, 10*t);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTZ, 20*t);
            }
            break;
            
        case 6:
            if(work->time < 30) {
                t = 1-(work->time/30.0f);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTX, -8*t);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTY, 10*t);
                Hu3DMotionForceSet(MBModelIdGet(obj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTZ, 20*t);
                if(++work->time == 30) {
                    Hu3DMotionNoMotReset(MBModelIdGet(storyKoopaObj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTX);
                    Hu3DMotionNoMotReset(MBModelIdGet(storyKoopaObj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTY);
                    Hu3DMotionNoMotReset(MBModelIdGet(storyKoopaObj->mdlId[0]), "ns_hed", HU3D_CONST_FORCE_ROTZ);
                }
                return;
            } else {
                MBModelPosGet(obj->mdlId[0], &pos);
                pos.y += work->vel.y;
                work->vel.y -= 1.0f;
                if(work->vel.y < 0) {
                    work->doneF = TRUE;
                    work->mode = 0;
                }
                MBModelPosSetV(obj->mdlId[0], &pos);
            }
            break;
    }
}

static void StoryKoopaAngryStart(void)
{
    STORY_OBJ_WORK *work = storyKoopaObj->data;
    work->mode = 3;
    work->doneF = FALSE;
    StoryObjMotShiftSet(storyKoopaObj, 8, HU3D_MOTATTR_NONE, FALSE);
}

static void StoryKoopaFlexStart(void)
{
    STORY_OBJ_WORK *work = storyKoopaObj->data;
    work->mode = 4;
    work->doneF = FALSE;
    StoryObjMotShiftSet(storyKoopaObj, 14, HU3D_MOTATTR_NONE, FALSE);
}

static void StoryKoopaStareStart(void)
{
    STORY_OBJ_WORK *work = storyKoopaObj->data;
    work->mode = 5;
    work->doneF = FALSE;
    work->time = 0;
    work->maxTime = 15; 
}

static void StoryKoopaJumpStart(void)
{
    STORY_OBJ_WORK *work = storyKoopaObj->data;
    work->mode = 6;
    work->doneF = FALSE;
    work->time = 0;
    work->vel.y = 50;
    StoryObjMotShiftSet(storyKoopaObj, 5, HU3D_MOTATTR_NONE, FALSE);
}

static void StoryMiniKoopaExec(OMOBJ *obj);

static void StoryMiniKoopaCreate(void)
{
    OMOBJ *obj;
    STORY_OBJ_WORK *work;
    
    static int motTbl[] = {
        BDEMO_HSF_minikoopaIdle,
        BDEMO_HSF_minikoopaRun,
        BDEMO_HSF_minikoopaFistUp,
        BDEMO_HSF_minikoopaArmDown,
        BDEMO_HSF_minikoopaWarn,
        BDEMO_HSF_minikoopaPoint,
        BDEMO_HSF_minikoopaShake,
        BDEMO_HSF_minikoopaSalute,
        BDEMO_HSF_minikoopaWin,
        BDEMO_HSF_minikoopaSad,
        BDEMO_HSF_minikoopaLeanL,
        BDEMO_HSF_minikoopaScare,
        BDEMO_HSF_minikoopaIdlePunch,
        HU_DATANUM_NONE
    };
    
    storyMiniKoopaObj = obj = MBAddObj(256, 1, 10, StoryObjExec);
    obj->data = work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(STORY_OBJ_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(STORY_OBJ_WORK));
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = MBModelCreate(DATA_minikoopa, motTbl, FALSE);
    HuDataDirClose(DATA_minikoopa);
    work->extModeExec = StoryMiniKoopaExec;
    work->playerNo = -1;
    MBCapsuleEffHookStoryCall(1, 4, obj->mdlId[0]);
}

static void StoryMiniKoopaExec(OMOBJ *obj)
{
    STORY_OBJ_WORK *work = obj->data;
}

static void StoryMiniKoopaArmDownStart(void)
{
    STORY_OBJ_WORK *work = storyMiniKoopaObj->data;
    work->mode = 1;
    work->forceIdle = TRUE;
    work->doneF = FALSE;
    StoryObjMotShiftSet(storyMiniKoopaObj, 4, HU3D_MOTATTR_NONE, FALSE);
}

static void StoryMiniKoopaWarnStart(void)
{
    STORY_OBJ_WORK *work = storyMiniKoopaObj->data;
    work->mode = 1;
    work->forceIdle = TRUE;
    work->doneF = FALSE;
    StoryObjMotShiftSet(storyMiniKoopaObj, 5, HU3D_MOTATTR_NONE, FALSE);
}

static void StoryMiniKoopaScareStart(void)
{
    STORY_OBJ_WORK *work = storyMiniKoopaObj->data;
    work->mode = 1;
    work->forceIdle = TRUE;
    work->doneF = FALSE;
    StoryObjMotShiftSet(storyMiniKoopaObj, 12, HU3D_MOTATTR_NONE, FALSE);
}

static void StoryMiniKoopaGroupCreate(int no);
static void StoryMiniKoopaGroupScene0Add(void);
static void StoryMiniKoopaGroupScene1Add(void);
static void StoryMiniKoopaGroupScene2Add(void);
static void StoryMiniKoopaGroupScene3Add(void);
static void StoryMiniKoopaGroupScene4Add(int no);
static void StoryMiniKoopaGroupInSet(int no);
static void StoryMiniKoopaGroupArmUpSet(void);

static void StoryOpeningSeqExec(int sceneNo)
{
    int i;
    u32 messBase;
    messBase = sceneMessTbl[sceneNo];
    for(i=0; i<3; i++) {
        StoryMiniKoopaGroupInSet(i);
        MBAudFXPlay(sceneMiniKoopaFXTbl[i]);
        MBWinCreate(MBWIN_TYPE_EVENT, messBase+i, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        StoryMiniKoopaGroupMotWait();
    }
    StoryMiniKoopaGroupArmUpSet();
    for(i=0; i<3; i++) {
        StoryMiniKoopaGroupSePlay(i, sceneMiniKoopaFXTbl[i]);
    }
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+3, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
}

static void StoryOpeningScene0Exec(void)
{
    int i;
    MBAudFXPlay(MSM_SE_STORY_11);
    StoryObjMotShiftSet(storyMiniKoopaObj, 7, HU3D_MOTATTR_LOOP, TRUE);
    StoryObjMotCreate(storyPlayerObj, CHARMOT_HSF_c000m1_423, HU3D_MOTATTR_NONE, FALSE);
    HuPrcSleep(120);
    StoryObjMotShiftSet(storyPlayerObj, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, FALSE);
    StoryMiniKoopaGroupCreate(0);
    StoryObjDispSet(storyMiniKoopaObj, FALSE);
    StoryMiniKoopaGroupScene0Add();
    StoryMiniKoopaGroupMotWait();
    HuPrcSleep(30);
    StoryOpeningSeqExec(0);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_SCENE0+12, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    for(i=0; i<3; i++) {
        StoryObjMotShiftSet(storyMiniKoopaGroupObj[i], 1, HU3D_MOTATTR_NONE, FALSE);
    }
}

static HU3DPARMANID StoryMiniKoopaShatterCreate(void);

static void StoryOpeningScene1Exec(void)
{
    int i; //r29
    HU3DPARMANID parManId; //r18
    HuVecF pos;
    u32 messBase;
    StoryObjMotShiftSet(storyMiniKoopaObj, 11, HU3D_MOTATTR_NONE, FALSE);
    MBAudFXPlay(MSM_SE_STORY_13);
    parManId = StoryMiniKoopaShatterCreate();
    StoryObjPosGet(storyMiniKoopaObj, &pos);
    Hu3DParManPosSet(parManId, pos.x, 50+pos.y, pos.z);
    HuPrcSleep(12);
    StoryObjDispSet(storyMiniKoopaObj, FALSE);
    StoryObjMotCreate(storyPlayerObj, CHARMOT_HSF_c000m1_423, HU3D_MOTATTR_NONE, FALSE);
    HuPrcSleep(60);
    StoryObjMotShiftSet(storyPlayerObj, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, FALSE);
    Hu3DParManKill(parManId);
    StoryMiniKoopaGroupCreate(0);
    StoryMiniKoopaGroupScene1Add();
    StoryMiniKoopaGroupMotWait();
    HuPrcSleep(30);
    StoryOpeningSeqExec(1);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_SCENE1+12, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(MBPlayerStoryPlayerGet());
    MBTopWinWait();
    for(i=0; i<3; i++) {
        StoryObjMotShiftSet(storyMiniKoopaGroupObj[i], 1, HU3D_MOTATTR_NONE, FALSE);
    }
}

static void StoryOpeningScene2Exec(void)
{
    int i;
    MBAudFXPlay(MSM_SE_STORY_11);
    StoryObjMotShiftSet(storyMiniKoopaObj, 7, HU3D_MOTATTR_LOOP, TRUE);
    StoryObjMotCreate(storyPlayerObj, CHARMOT_HSF_c000m1_423, HU3D_MOTATTR_NONE, FALSE);
    HuPrcSleep(120);
    StoryObjMotShiftSet(storyPlayerObj, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, FALSE);
    StoryMiniKoopaGroupCreate(1);
    StoryObjDispSet(storyMiniKoopaObj, FALSE);
    StoryMiniKoopaGroupScene2Add();
    StoryMiniKoopaGroupMotWait();
    HuPrcSleep(30);
    StoryOpeningSeqExec(2);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_SCENE2+12, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(MBPlayerStoryPlayerGet());
    MBTopWinWait();
    for(i=0; i<3; i++) {
        StoryObjMotShiftSet(storyMiniKoopaGroupObj[i], 1, HU3D_MOTATTR_NONE, FALSE);
    }
}

static void StoryOpeningScene3Exec(void)
{
    int i;
    MBAudFXPlay(MSM_SE_STORY_11);
    StoryObjMotShiftSet(storyMiniKoopaObj, 7, HU3D_MOTATTR_LOOP, TRUE);
    StoryObjMotCreate(storyPlayerObj, CHARMOT_HSF_c000m1_423, HU3D_MOTATTR_NONE, FALSE);
    HuPrcSleep(120);
    StoryObjMotShiftSet(storyPlayerObj, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, FALSE);
    StoryMiniKoopaGroupCreate(1);
    StoryObjDispSet(storyMiniKoopaObj, FALSE);
    StoryMiniKoopaGroupScene3Add();
    StoryMiniKoopaGroupMotWait();
    HuPrcSleep(30);
    StoryOpeningSeqExec(3);
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_SCENE3+12, HUWIN_SPEAKER_NONE);
    StoryMesPlayerNameInsert(MBPlayerStoryPlayerGet());
    MBTopWinWait();
    for(i=0; i<3; i++) {
        StoryObjMotShiftSet(storyMiniKoopaGroupObj[i], 1, HU3D_MOTATTR_NONE, FALSE);
    }
}

static void StoryOpeningSeq4Exec(void)
{
    int i;
    u32 messBase;
    messBase = MESS_BOARD_STORY_SCENE4+8;
    for(i=0; i<3; i++) {
        StoryMiniKoopaGroupScene4Add(i);
        switch(i) {
            case 0:
                StoryMiniKoopaGroupSePlay(0, MSM_SE_VOICE_MINIKOOPA+5);
                break;
            
            case 1:
                StoryMiniKoopaGroupSePlay(1, MSM_SE_VOICE_MINIKOOPAG+5);
                break;
        }
        MBWinCreate(MBWIN_TYPE_EVENT, messBase+i, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        StoryMiniKoopaGroupMotWait();
    }
    StoryMiniKoopaGroupArmUpSet();
    for(i=0; i<3; i++) {
        StoryMiniKoopaGroupSePlay(i, scene4MiniKoopaFXTbl[i]);
    }
    MBWinCreate(MBWIN_TYPE_EVENT, messBase+3, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
}


static void StoryOpeningScene4Exec(void)
{
    int i;
    HuVecF pos;
    MBAudFXPlay(MSM_SE_STORY_11);
    StoryObjMotShiftSet(storyMiniKoopaObj, 7, HU3D_MOTATTR_LOOP, TRUE);
    StoryObjMotCreate(storyPlayerObj, CHARMOT_HSF_c000m1_423, HU3D_MOTATTR_NONE, FALSE);
    HuPrcSleep(120);
    StoryObjMotShiftSet(storyPlayerObj, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, FALSE);
    StoryMiniKoopaGroupCreate(2);
    StoryObjDispSet(storyMiniKoopaObj, FALSE);
    for(i=0; i<3; i++) {
        StoryObjPosGet(storyMiniKoopaObj, &pos);
        StoryObjPosSetV(storyMiniKoopaGroupObj[i], &pos);
    }
    StoryOpeningSeq4Exec();
    StoryKoopaSePlay(MSM_SE_GUIDE_12);
    StoryObjMotShiftSet(storyKoopaObj, 3, HU3D_MOTATTR_LOOP, TRUE);
    MBWinCreate(MBWIN_TYPE_EVENT, MESS_BOARD_STORY_SCENE4+12, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    for(i=0; i<3; i++) {
        StoryObjMotShiftSet(storyMiniKoopaGroupObj[i], 1, HU3D_MOTATTR_NONE, FALSE);
    }
}

static void StoryMiniKoopaGroupExec(OMOBJ *obj);

static void StoryMiniKoopaGroupCreate(int no)
{
    OMOBJ *obj; //r31
    int j; //r30
    STORY_OBJ_WORK *work; //r29
    int i; //r28
    int num; //r27
    const int *motTbl; //r26
    
    static const int *motTblList[3] = {
        miniKoopaGroup1MotTbl,
        miniKoopaGroup2MotTbl,
        miniKoopaGroup3MotTbl
    };
    motTbl = motTblList[no];
    for(num=0, i=0; i<GW_PLAYER_MAX; i++) {
        if(i != MBPlayerStoryPlayerGet()) {
            storyMiniKoopaGroupObj[num] = obj = MBAddObj(256, 10, 20, StoryObjExec);
            for(j=0; j<obj->mtncnt; j++) {
                obj->mtnId[j] = MB_MOT_NONE;
            }
            obj->data = work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(STORY_OBJ_WORK), HU_MEMNUM_OVL);
            memset(work, 0, sizeof(STORY_OBJ_WORK));
            work->doneF = TRUE;
            omSetStatBit(obj, OM_STAT_MODELPAUSE);
            obj->mdlId[0] = MBPlayerModelGet(i);
            MBModelDispSet(obj->mdlId[0], TRUE);
            obj->mdlId[1] = MBModelCreate(miniKoopaLogoFileTbl[num], NULL, FALSE);
            MBModelDispSet(obj->mdlId[1], FALSE);
            MBMotionSpeedSet(obj->mdlId[1], 0);
            obj->mdlId[2] = MBModelCreate(BDEMO_HSF_minikoopaInEff, NULL, FALSE);
            MBModelDispSet(obj->mdlId[2], FALSE);
            MBMotionSpeedSet(obj->mdlId[2], 0);
            for(j=0; motTbl[j]>=0;) {
                obj->mtnId[j++] = MBMotionCreate(obj->mdlId[0], motTbl[j]);
            }
            work->extModeExec = StoryMiniKoopaGroupExec;
            work->no = num++;
            work->playerNo = i;
        }
    }
}

static void StoryMiniKoopaMotionSet(OMOBJ *obj, int no, u32 attr, BOOL lockDisable)
{
    StoryObjMotNoSet(obj, obj->mtnId[no], attr, lockDisable);
}

static void StoryMiniKoopaMotionShiftSet(OMOBJ *obj, int no, u32 attr, BOOL lockDisable)
{
    StoryObjMotShiftSet(obj, obj->mtnId[no], attr, lockDisable);
}

static void StoryMiniKoopaGroupExec(OMOBJ *obj)
{
    STORY_OBJ_WORK *work; //r30
    float t; //f31
    
    Mtx rotMtx; //sp+0x3C
    HuVecF pos; //sp+0x30
    HuVecF rot; //sp+0x24
    BOOL splitEnd; //sp+0x20
    
    static float narrowGrpOfs[3] = { -80, 0, 80 };
    static float grpOfs[3] = { -180, 0, 180 };
    
    work = obj->data;
    switch(work->mode) {
        case 3:
            if(work->time > work->maxTime) {
                work->doneF = TRUE;
                work->mode = 0;
                StoryObjMotShiftSet(obj, 1, HU3D_MOTATTR_LOOP, FALSE);
            } else {
                t = (float)(work->time++)/work->maxTime;
                pos.x = obj->trans.x+(t*(obj->rot.x-obj->trans.x));
                MBModelPosSet(obj->mdlId[0], pos.x, obj->trans.y, obj->trans.z);
            }
            break;
        
        case 4:
            if(work->time == 0) {
                MBModelDispSet(obj->mdlId[0], TRUE);
                work->mode = 5;
                StoryObjMotNoSet(obj, 4, HU3D_MOTATTR_NONE, FALSE);
                work->vel.y  = 30.000002f;
                work->time = 0;
                work->maxTime = 45;
                MBModelSePlay(obj->mdlId[0], MSM_SE_STORY_14);
                MBModelDispSet(obj->mdlId[2], TRUE);
                MBMotionSpeedSet(obj->mdlId[2], 1);
                MBModelPosGet(obj->mdlId[0], &pos);
                MBModelPosSet(obj->mdlId[2], pos.x, pos.y, pos.z);
            } else {
                work->time--;
            }
            break;
        
        case 5:
            if(work->time > work->maxTime) {
                work->doneF = TRUE;
                work->mode = 0;
                StoryObjMotShiftSet(obj, 1, HU3D_MOTATTR_LOOP, FALSE);
            } else {
                t = (float)(work->time++)/work->maxTime;
                MBModelPosGet(obj->mdlId[0], &pos);
                pos.y = work->fallY+(100*(4*HuSin(t*180)));
                MBModelPosSetV(obj->mdlId[0], &pos);
                MBModelRotYSet(obj->mdlId[0], HuSin(t*90)*720);
            }
            break;
            
        case 6:
            if(work->splitPrev == 0) {
                work->mode = 7;
                miniKoopaGrpNum++;
            } else {
                if(work->time > work->maxTime) {
                    work->splitPrev--;
                    work->time = 0;
                    work->basePos.y += 100;
                    if(work->splitPrev == 1) {
                        MBAudFXPlay(MSM_SE_STORY_16);
                    }
                }
                t = (float)(work->time++)/work->maxTime;
                MBModelPosGet(obj->mdlId[0], &pos);
                pos.y = work->basePos.y+(100*HuSin(t*90));
                MBModelPosSetV(obj->mdlId[0], &pos);
            }
            break;
        
        case 7:
            if(miniKoopaGrpNum == 3) {
                if(work->no != 1) {
                    work->mode = 9;
                    work->jumpF = FALSE;
                    work->time = 0;
                    work->maxTime = 36;
                    StoryMiniKoopaMotionShiftSet(obj, 6, HU3D_MOTATTR_NONE, FALSE);
                    MBAudFXPlay(MSM_SE_STORY_17);
                    obj->rot.x = work->basePos.x+grpOfs[work->no];
                    obj->rot.y = (work->basePos.x > obj->rot.x) ? -80.0f : 80.0f;
                } else {
                    work->doneF = TRUE;
                    work->mode = 0;
                    StoryObjMotShiftSet(obj, 1, HU3D_MOTATTR_LOOP, FALSE);
                }
            }
            break;
        
        case 9:
            if(work->time > work->maxTime) {
                StoryObjRotStartWalk(obj, 0, 12);
            } else {
                t = (float)(work->time++)/work->maxTime;
                if(!work->jumpF && t >= 0.8f) {
                    StoryMiniKoopaMotionShiftSet(obj, 8, HU3D_MOTATTR_NONE, FALSE);
                    work->jumpF = TRUE;
                }
                pos.x = work->basePos.x+(t*(obj->rot.x-work->basePos.x));
                pos.y = work->basePos.y+(t*(work->fallY-work->basePos.y))+(100*(2*HuSin(t*180)));
                MBModelRotGet(obj->mdlId[0], &rot);
                rot.x = 360*t;
                rot.y = work->rot.y+(t*(obj->rot.y-work->rot.y));
                MBModelRotSet(obj->mdlId[0], rot.x, rot.y, rot.z);
                MTXTrans(rotMtx, 0, -80, 0);
                mtxRotCat(rotMtx, rot.x, rot.y, rot.z);
                mtxTransCat(rotMtx, 0, 80, 0);
                MBModelPosSet(obj->mdlId[0], pos.x+rotMtx[0][3], pos.y+rotMtx[1][3], work->basePos.z+rotMtx[2][3]);
                
            }
            break;
        
        case 10:
            if(work->time > work->maxTime || work->time < 0) {
                splitEnd = FALSE;
                if(work->time >= 0) {
                    if(--work->rotDelay == 0) {
                        StoryObjRotStartWalk(obj, 0, 12);
                        break;
                    } else {
                        work->splitPrev = work->splitNum;
                        if(--work->splitNum < 0) {
                            work->splitNum = 2;
                            splitEnd = TRUE;
                        }
                    }
                } else {
                    obj->trans.x = obj->rot.x = work->basePos.x;
                    obj->trans.y = work->basePos.y;
                }
                if(work->splitPrev != work->splitNum) {
                    MBAudFXPlay(MSM_SE_STORY_17);
                    StoryMiniKoopaMotionShiftSet(obj, 7, HU3D_MOTATTR_LOOP, FALSE);
                    if(splitEnd) {
                        obj->scale.y = 400;
                    } else {
                        obj->scale.y = 200;
                    }
                    MBModelRotGet(obj->mdlId[0], &work->rot);
                    obj->trans.x = obj->rot.x;
                    obj->rot.x = work->basePos.x+grpOfs[work->splitNum];
                    obj->rot.y = (obj->trans.x > obj->rot.x) ? -80.0f : 80.0f;
                }
                work->time = 0;
                work->jumpF = FALSE;
                MBAudFXPlay(MSM_SE_STORY_18);
            }
            t = (float)(work->time++)/work->maxTime;
            if(work->splitPrev != work->splitNum) {
                if(!work->jumpF && t >= 0.8f) {
                    StoryMiniKoopaMotionShiftSet(obj, 8, HU3D_MOTATTR_NONE, FALSE);
                    work->jumpF = TRUE;
                }
                pos.x = obj->trans.x+(t*(obj->rot.x-obj->trans.x));
                pos.y = obj->trans.y+(obj->scale.y*HuSin(t*180));
                MBModelRotGet(obj->mdlId[0], &rot);
                rot.x = 720*t;
                rot.y = work->rot.y+(t*(obj->rot.y-work->rot.y));
                MBModelRotSet(obj->mdlId[0], rot.x, rot.y, rot.z);
                MTXTrans(rotMtx, 0, -80, 0);
                mtxRotCat(rotMtx, rot.x, rot.y, rot.z);
                mtxTransCat(rotMtx, 0, 80, 0);
                MBModelPosSet(obj->mdlId[0], pos.x+rotMtx[0][3], pos.y+rotMtx[1][3], work->basePos.z+rotMtx[2][3]);
            }
            break;
        
        case 12:
            if(work->time > work->maxTime) {
                work->doneF = TRUE;
                work->mode = 0;
            } else {
                t = (float)(work->time++)/work->maxTime;
                pos.x = obj->trans.x+(t*(obj->rot.x-obj->trans.x));
                pos.y = obj->trans.y+(100*HuSin(t*180));
                MBModelPosSet(obj->mdlId[0], pos.x, pos.y, obj->trans.z);
            }
            break;
        
        case 13:
            if(--work->time == 0) {
                work->doneF = TRUE;
                work->mode = 0;
                StoryMiniKoopaGroupScene4Add(3);
            }
            break;
        
        case 14:
            if(--work->time == 0) {
                work->mode = 1;
                work->forceIdle = FALSE;
                work->doneF = FALSE;
                StoryMiniKoopaMotionShiftSet(obj, 7, HU3D_MOTATTR_NONE, FALSE);
                MBAudFXPlay(MSM_SE_STORY_21);
            }
            break;
            
        case 15:
            if(work->splitPrev == 0) {
                if(--work->time == 0) {
                    MBModelPosGet(obj->mdlId[0], &pos);
                    MBModelPosSet(obj->mdlId[1], pos.x, pos.y+90, pos.z+100);
                    MBModelScaleSet(obj->mdlId[1], 1.5f, 1.5f, 5.0f);
                    MBModelRotSet(obj->mdlId[1], -15, 0, 0);
                    MBModelSePlay(obj->mdlId[0], MSM_SE_STORY_22);
                    MBModelDispSet(obj->mdlId[1], TRUE);
                    MBMotionSpeedSet(obj->mdlId[1], 1);
                    work->time = 30;
                    work->splitPrev = 1;
                }
            } else {
                if(--work->time == 0) {
                    work->mode = 0;
                    work->doneF = TRUE;
                }
            }
            break;
        
        case 16:
            if(StoryObjMotCheck(obj)) {
                StoryObjMotShiftSet(obj, 1, HU3D_MOTATTR_LOOP, FALSE);
                work->mode = 0;
                work->doneF = TRUE;
            }
            break;
    }
}

static void StoryMiniKoopaGroupScene3Add(void)
{
    STORY_OBJ_WORK *work;
    OMOBJ *obj;
    int i;
    static float grpOfs[3] = { -180, 0, 180 };
    for(i=0; i<3; i++) {
        obj = storyMiniKoopaGroupObj[i];
        work = obj->data;
        work->mode = 10;
        work->doneF = FALSE;
        work->time = -1;
        work->maxTime = 30;
        work->splitPrev = 1;
        work->splitNum = work->no;
        work->rotDelay = 4;
        StoryObjPosGet(storyMiniKoopaObj, &work->basePos);
        obj->rot.x = work->basePos.x+grpOfs[i];
        MBModelPosSetV(obj->mdlId[0], &work->basePos);
        work->fallY = work->basePos.y;
    }
    miniKoopaGrpNum = 0;
}

static void StoryMiniKoopaGroupScene4Add(int no)
{
    OMOBJ *obj; //r31
    STORY_OBJ_WORK *work; //r30
    
    switch(no) {
        case 0:
            obj = storyMiniKoopaGroupObj[0];
            work = obj->data;
            work->mode = 12;
            work->doneF = FALSE;
            work->time = 0;
            work->maxTime = 21;
            StoryObjPosGet(storyMiniKoopaObj, &obj->trans);
            obj->rot.x = obj->trans.x-150;
            MBModelPosSetV(obj->mdlId[0], &obj->trans);
            MBAudFXPlay(MSM_SE_STORY_19);
            StoryMiniKoopaMotionShiftSet(obj, 0, HU3D_MOTATTR_LOOP, TRUE);
            break;
        
        case 1:
            obj = storyMiniKoopaGroupObj[2];
            work = obj->data;
            work->mode = 12;
            work->doneF = FALSE;
            work->time = 0;
            work->maxTime = 21;
            StoryObjPosGet(storyMiniKoopaObj, &obj->trans);
            obj->rot.x = obj->trans.x+150;
            MBModelPosSetV(obj->mdlId[0], &obj->trans);
            MBAudFXPlay(MSM_SE_STORY_19);
            StoryMiniKoopaMotionShiftSet(obj, 2, HU3D_MOTATTR_LOOP, TRUE);
            break;
        
        case 2:
            obj = storyMiniKoopaGroupObj[1];
            work = obj->data;
            work->mode = 13;
            work->doneF = FALSE;
            work->time = 120;
            MBAudFXPlay(MSM_SE_STORY_20);
            StoryMiniKoopaMotionShiftSet(obj, 1, HU3D_MOTATTR_LOOP, TRUE);
            break;
        
        case 3:
            obj = storyMiniKoopaGroupObj[0];
            work = obj->data;
            StoryMiniKoopaMotionShiftSet(obj, 5, HU3D_MOTATTR_NONE, FALSE);
            work->mode = 1;
            work->forceIdle = FALSE;
            work->doneF = FALSE;
            obj = storyMiniKoopaGroupObj[1];
            work = obj->data;
            work->mode = 14;
            work->time = 30;
            work->doneF = FALSE;
            obj = storyMiniKoopaGroupObj[2];
            work = obj->data;
            StoryMiniKoopaMotionShiftSet(obj, 6, HU3D_MOTATTR_NONE, FALSE);
            work->mode = 1;
            work->forceIdle = FALSE;
            work->doneF = FALSE;
            break;
    }
}

static void StoryMiniKoopaGroupScene2Add(void)
{
    STORY_OBJ_WORK *work;
    OMOBJ *obj;
    int i;
    
    HuVecF ofs[3] = {
        { -180, 200, 0 },
        { 0, 0, 0 },
        { 180, 100, 0 },
    };
    
    static s16 splitNo[3] = { 2, 0, 1 };
    
    for(i=0; i<3; i++) {
        obj = storyMiniKoopaGroupObj[i];
        work = obj->data;
        work->mode = 6;
        work->doneF = FALSE;
        work->time = 0;
        work->maxTime = 30;
        work->splitPrev = splitNo[work->no];
        StoryObjPosGet(storyMiniKoopaObj, &work->basePos);
        obj->rot.x = work->basePos.x+ofs[i].x;
        obj->rot.y = work->basePos.y+ofs[i].y;
        MBModelPosSetV(obj->mdlId[0], &work->basePos);
        work->fallY = work->basePos.y;
        StoryMiniKoopaMotionSet(obj, 5, HU3D_MOTATTR_LOOP, FALSE);
    }
    MBAudFXPlay(MSM_SE_STORY_15);
    miniKoopaGrpNum = 0;
}

static void StoryMiniKoopaGroupScene0Add(void)
{
    STORY_OBJ_WORK *work;
    OMOBJ *obj;
    int i;
    static float grpOfs[3] = { -180, 0, 180 };
    for(i=0; i<3; i++) {
        obj = storyMiniKoopaGroupObj[i];
        work = obj->data;
        work->mode = 3;
        work->doneF = FALSE;
        work->time = 0;
        work->maxTime = 60;
        StoryObjPosGet(storyMiniKoopaObj, &obj->trans);
        obj->rot.x = obj->trans.x+grpOfs[i];
        MBModelPosSetV(obj->mdlId[0], &obj->trans);
        StoryMiniKoopaMotionSet(obj, 5, HU3D_MOTATTR_LOOP, FALSE);
    }
    MBAudFXPlay(MSM_SE_STORY_12);
}

static void StoryMiniKoopaGroupScene1Add(void)
{
    OMOBJ *obj;
    STORY_OBJ_WORK *work;
    int i;
    HuVecF pos;
    static float grpOfs[3] = { -180, 0, 180 };
    
    StoryObjPosGet(storyMiniKoopaObj, &pos); 
    for(i=0; i<3; i++) {
        obj = storyMiniKoopaGroupObj[i];
        work = obj->data;
        work->mode = 4;
        work->doneF = FALSE;
        work->time = i*30;
        work->fallY = pos.y;
        StoryObjPosSet(obj, pos.x+grpOfs[i], pos.y, pos.z);
        StoryObjDispSet(obj, FALSE);
        
        StoryMiniKoopaMotionSet(obj, 5, HU3D_MOTATTR_LOOP, FALSE);
    }
}

static void StoryMiniKoopaGroupInSet(int no)
{
    OMOBJ *obj = storyMiniKoopaGroupObj[no];
    STORY_OBJ_WORK *work = obj->data;
    work->mode = 15;
    work->doneF = FALSE;
    work->time = 12;
    work->splitPrev = 0;
    StoryMiniKoopaMotionShiftSet(storyMiniKoopaGroupObj[no], no, HU3D_MOTATTR_LOOP, FALSE);
}

static void StoryMiniKoopaGroupArmUpSet(void)
{
    int i;
    STORY_OBJ_WORK *work;
    for(i=0; i<3; i++) {
        work = storyMiniKoopaGroupObj[i]->data;
        StoryMiniKoopaMotionShiftSet(storyMiniKoopaGroupObj[i], 3, HU3D_MOTATTR_NONE, FALSE);
        work->mode = 16;
        work->doneF = FALSE;
    }
}

static void StoryMiniKoopaGroupSaluteSet(void)
{
    int i;
    STORY_OBJ_WORK *work;
    for(i=0; i<3; i++) {
        work = storyMiniKoopaGroupObj[i]->data;
        StoryMiniKoopaMotionShiftSet(storyMiniKoopaGroupObj[i], 4, HU3D_MOTATTR_NONE, TRUE);

    }
}

static void StoryMiniKoopaGroupMotWait(void)
{
    int i;
    int num;
    do {
        num = 0;
        for(i=0; i<3; i++) {
            if(!StoryObjDoneCheck(storyMiniKoopaGroupObj[i])) {
                num++;
            }
        }
        HuPrcVSleep();
    } while(num != 0);
}

static HU3DPARMANID StoryMiniKoopaShatterCreate(void)
{
    int parManId; //r31
    int mdlId; //r30
    ANIMDATA *animP; //r29
    
    static HU3DPARMANPARAM param = {
        120,
        264,
        50,
        0,
        65,
        { 0, 0, 0 },
        15,
        0.9f,
        60,
        0.95f,
        1,
        {
            { 255, 255, 255, 255 }
        },
        {
            { 255, 255, 255, 0 }
        },
    };
    animP = HuSprAnimDataRead(EFFECT_ANM_smoke);
    parManId = Hu3DParManCreate(animP, 200, &param);
    Hu3DParManTimeLimitSet(parManId, 120);
    mdlId = Hu3DParManModelIDGet(parManId);
    Hu3DModelLayerSet(mdlId, 3);
    Hu3DModelCameraSet(mdlId, HU3D_CAM0);
    return parManId;
    
}

static int StorySceneSelExec(int sceneNo)
{
    int x, y, w, h;
    u16 dstk;
    u16 btnDown;
    static GXColor winCol = { 0, 0, 144, 192 };
    if(MBPlayerAllComCheck()) {
        return sceneNo;
    }
    w = 188;
    h = 28;
    x = 320-(w/2);
    y = 240-(h/2);
    while(1) {
        
        dstk = MBPadDStkRepGetAll();
        btnDown = MBPadBtnDownGetAll();
        printWin(x, y, w, h, &winCol);
        print8(x+6, y+6, 2, "sceneNo = %d", sceneNo);
        if((dstk & PAD_BUTTON_LEFT) && sceneNo > 0) {
            sceneNo--;
        }
        if((dstk & PAD_BUTTON_RIGHT) && sceneNo < 4) {
            sceneNo++;
        }
        
        if(btnDown & PAD_BUTTON_A) {
            break;
        }
        HuPrcVSleep();
    }
    return sceneNo;
}

static void StoryMesPlayerNameInsert(int playerNo)
{
    char baseMes[3] = {};
    
    static char mesBuf[256];
    baseMes[0] = 0x1E;
    baseMes[1] = 0x05;
    strcpy(mesBuf, baseMes);
    strcat(mesBuf, HuWinMesPtrGet(MBPlayerNameMesGet(playerNo)));
    baseMes[1] = 0x08;
    strcat(mesBuf, baseMes);
    MBTopWinInsertMesSet(MESSNUM_PTR(mesBuf), 0);
}

static void StoryKoopaSePlay(int seId)
{
    MBModelSePlay(storyKoopaObj->mdlId[0], seId);
}

static void StoryMiniKoopaSePlay(int seId)
{
    MBModelSePlay(storyMiniKoopaObj->mdlId[0], seId);
}

static void StoryMiniKoopaGroupSePlay(int no, int seId)
{
    MBModelSePlay(storyMiniKoopaGroupObj[no]->mdlId[0], seId);
}