#include "game/board/player.h"
#include "game/board/masu.h"
#include "game/board/status.h"
#include "game/board/sai.h"
#include "game/board/tutorial.h"
#include "game/board/camera.h"
#include "game/board/telop.h"
#include "game/board/pause.h"
#include "game/board/window.h"
#include "game/board/branch.h"

#include "game/flag.h"
#include "game/wipe.h"

#include "datanum/w20.h"
#include "messnum/charaname.h"

void MBStarDispSetAll(BOOL dispF);
void MBStarMasuDispSet(int masuId, BOOL dispF);
void MBScrollExec(int playerNo);

s16 MBGateMasuExec(int playerNo, s16 masuId, s16 masuNext);

static MBPLAYERWORK playerWork[GW_PLAYER_MAX];
static int startPlayerNo;
static BOOL blackoutF;
static BOOL posFixAnimF;

#define CHAR_MDLFILE(name) DATANUM(DATA_##name##mdl1, 0)
#define CHAR_MOTDIR(name) DATA_##name##mot
#define W20_MCCAR(charName) W20_HSF_mcCar##charName

static const int charMdlFileTbl[CHARNO_MAX] = {
    CHAR_MDLFILE(mario),
    CHAR_MDLFILE(luigi),
    CHAR_MDLFILE(peach),
    CHAR_MDLFILE(yoshi),
    CHAR_MDLFILE(wario),
    CHAR_MDLFILE(daisy),
    CHAR_MDLFILE(waluigi),
    CHAR_MDLFILE(kinopio),
    CHAR_MDLFILE(teresa),
    CHAR_MDLFILE(minikoopa),
    CHAR_MDLFILE(minikoopa),
    CHAR_MDLFILE(minikoopa),
    CHAR_MDLFILE(minikoopa),
};

static const int charMotDirTbl[CHARNO_MAX] = {
    CHAR_MOTDIR(mario),
    CHAR_MOTDIR(luigi),
    CHAR_MOTDIR(peach),
    CHAR_MOTDIR(yoshi),
    CHAR_MOTDIR(wario),
    CHAR_MOTDIR(daisy),
    CHAR_MOTDIR(waluigi),
    CHAR_MOTDIR(kinopio),
    CHAR_MOTDIR(teresa),
    CHAR_MOTDIR(minikoopa),
    CHAR_MOTDIR(minikoopa),
    CHAR_MOTDIR(minikoopa),
    CHAR_MOTDIR(minikoopa),
};

static const u16 charMotNoTbl[MB_PLAYER_MOT_COUNT] = {
    CHAR_MOTNO(CHARMOT_HSF_c000m1_300),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_301),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_302),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_303),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_304),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_323),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_306),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_307),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_326),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_368),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_311),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_356),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_357),
    CHAR_MOTNO(CHARMOT_HSF_c000m1_444),
};

static const int tutorialCharNoTbl[GW_PLAYER_MAX] = {
    CHARNO_YOSHI,
    CHARNO_MARIO,
    CHARNO_PEACH,
    CHARNO_WARIO
};

static const int mcCarFileTbl[CHARNO_MINIKOOPA+1] = {
    W20_MCCAR(Mario),
    W20_MCCAR(Luigi),
    W20_MCCAR(Peach),
    W20_MCCAR(Yoshi),
    W20_MCCAR(Wario),
    W20_MCCAR(Daisy),
    W20_MCCAR(Waluigi),
    W20_MCCAR(Kinopio),
    W20_MCCAR(Teresa),
    W20_MCCAR(Minikoopa),
};

#undef CHAR_MDLFILE
#undef CHAR_MOTDIR
#undef W20_MCCAR

void MBPlayerInit(BOOL noEventF)
{
    MBPLAYERWORK *workP = &playerWork[0];
    int i, j;
    s16 masuRedTbl[MASU_MAX];
    int motDataNum[MB_PLAYER_MOT_COUNT+2];
    s8 charF[CHARNO_MAX];
    s8 charTbl[CHARNO_MAX];
    int num;
    int masuId;
    memset(playerWork, 0, sizeof(playerWork));
    if(_CheckFlag(FLAG_BOARD_MG)) {
        GwSystem.turnPlayerNo = 0;
    }
    if(noEventF) {
        int startMasu = MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, NULL);
        int grp;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(GWPartyFGet() == FALSE) {
                if(MBPlayerStoryComCheck(i)) {
                    GwPlayerConf[i].charNo = i+CHARNO_MINIKOOPAR-1;
                    GwPlayerConf[i].type = GW_TYPE_COM;
                } else {
                    if(GwPlayerConf[i].charNo >= CHARNO_KINOPIO) {
                        GwPlayerConf[i].charNo = CHARNO_MARIO;
                    }
                }
            } else {
                if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
                    GwPlayerConf[i].charNo = tutorialCharNoTbl[i];
                }
                if(_CheckFlag(FLAG_MG_CIRCUIT) && GwPlayerConf[i].charNo >= CHARNO_MINIKOOPA+1) {
                    memset(charF, 0, CHARNO_MAX);
                    for(j=0; j<GW_PLAYER_MAX; j++) {
                        charF[GwPlayerConf[j].charNo] = TRUE;
                    }
                    num = 0;
                    for(j=0; j<CHARNO_MAX; j++) {
                        if(!charF[j]) {
                            charTbl[num++] = j;
                        }
                    }
                    GwPlayerConf[i].charNo = charTbl[frandmod(num)];
                }
            }
            GwPlayer[i].charNo = GwPlayerConf[i].charNo;
            GwPlayerConf[i].charNo = GwPlayerConf[i].charNo;
            GwPlayer[i].padNo = GwPlayerConf[i].padNo;
            GwPlayerConf[i].padNo = GwPlayerConf[i].padNo;
            GwPlayer[i].comF = GwPlayerConf[i].type;
            GwPlayerConf[i].type = GwPlayerConf[i].type;
            GwPlayer[i].dif = GwPlayerConf[i].dif;
            GwPlayerConf[i].dif = GwPlayerConf[i].dif;
            GwPlayer[i].masuId = startMasu;
            GwPlayer[i].color = STATUS_COLOR_GRAY;
            GwPlayer[i].saiMode = MB_SAIMODE_NORMAL;
            MBPlayerStoryAliveReset(i);
            for(j=0; j<3; j++) {
                GwPlayer[i].capsule[j] = CAPSULE_NULL;
            }
            
            GwPlayer[i].grp = grp = GwPlayerConf[i].grp;
            GwPlayerConf[i].grp = grp;
            if(_CheckFlag(FLAG_MG_CIRCUIT)) {
                masuId = MBMasuCarTargetGet(startMasu, i);
                GwPlayer[i].masuId = masuId;
                GwPlayer[i].masuIdPrev = masuId;
            }
            GwPlayer[i].orderNo = i;
        }
        for(i=0; i<CHARNO_MAX; i++) {
            if(CharMotionAMemPGet(i)) {
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(i == GwPlayer[j].charNo) {
                         break;
                    }
                }
                if(j >= GW_PLAYER_MAX) {
                    CharDataClose(i);
                }
            }
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(CharMotionAMemPGet(GwPlayer[i].charNo) == 0) {
                CharMotionInit(GwPlayer[i].charNo);
            }
        }
        if(GWPartyFGet() == FALSE) {
            int masuNum;
            int masuNo1;
            int masuNo2;
            MASU *masuP;
            
            masuNum = MBMasuNumGet(MASU_LAYER_DEFAULT);
            num = 0;
            for(i=0; i<masuNum; i++) {
                masuId = i+1;
                masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
                if(masuP->type == MASU_TYPE_RED) {
                    masuRedTbl[num++] = masuId;
                }
            }
            for(i=0; i<100; i++) {
                masuNo1 = frandmod(num);
                masuNo2 = frandmod(num);
                masuId = masuRedTbl[masuNo1];
                masuRedTbl[masuNo1] = masuRedTbl[masuNo2];
                masuRedTbl[masuNo2] = masuId;
            }
            for(j=0,i=0; i<GW_PLAYER_MAX; i++){ 
                if(MBPlayerStoryComCheck(i)) {
                    GwPlayer[i].masuId = masuRedTbl[j++];
                }
            }
        }
        GwSystem.playerMode = MB_PLAYER_MODE_DEFAULT;
    }
    for(i=0; i<GW_PLAYER_MAX; i++, workP++) {
        GWPLAYER *playerP;
        int charNo;
        MBMODELID modelId;
        workP->preTurnHook = workP->postTurnHook = NULL;
        workP->rotateObj = workP->walkObj = workP->posFixObj = NULL;
        playerP = GWPlayerGet(i);
        playerP->playerNo = workP->playerNo = i;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grp = MBPlayerGrpGet(i);
        GwPlayerConf[i].dif = GwPlayer[i].dif;
        charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].charNo = charNo;
        if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
            for(j=0; j<MB_PLAYER_MOT_COUNT; j++) {
                motDataNum[j] = DATANUM(charMotDirTbl[charNo], charMotNoTbl[j]);
            }
            motDataNum[j] = HU_DATANUM_NONE;
            modelId = workP->modelId = MBModelCreateChar(charNo, charMdlFileTbl[charNo], motDataNum, FALSE);
        } else {
            for(j=0; j<MB_PLAYER_CAR_MOT_NUM; j++) {
                motDataNum[j] = mcCarFileTbl[charNo]+j+1;
            }
            motDataNum[j] = HU_DATANUM_NONE;
            modelId = workP->modelId = MBModelCreate(mcCarFileTbl[charNo], motDataNum, FALSE);
            MBModelScaleSet(modelId, 0.9f, 0.9f, 0.9f);
        }
        MBPlayerMatClone(i);
        workP->motNo = MB_PLAYER_MOT_IDLE;
        MBMotionNoSet(modelId, workP->motNo, HU3D_MOTATTR_LOOP);
        GwPlayer[i].dispLightF = TRUE;
        MBModelStubVal2Set(modelId, TRUE);
        MBModelStubValSet(modelId, TRUE);
        MBModelUnkSpeedSet(modelId, -1);
        GwPlayer[i].masuIdPrev = MASU_NULL;
        CharModelDataClose(charNo);
        if(!MBPlayerAliveCheck(i)) {
            MBModelDispSet(MBPlayerModelGet(i), FALSE);
        }
        if(_CheckFlag(FLAG_MG_CIRCUIT)) {
            MBPlayerPosReset(i);
            MBPlayerRotYSet(i, -90);
        }
    }
    MBPlayerPosFixFlagSet(FALSE);
    CharEffectLayerSet(2);
}

void MBPlayerClose(void)
{
    MBPLAYERWORK *workP = &playerWork[0];
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++, workP++) {
        GWPLAYER *playerP = GWPlayerGet(i);
        if(workP->modelId != MB_MODEL_NONE) {
            MBModelKill(workP->modelId);
            workP->modelId = MB_MODEL_NONE;
        }
        if(workP->matCopy) {
            HuMemDirectFree(workP->matCopy);
            workP->matCopy = NULL;
        }
        MBSaiNumKill(i);
    }
}

MBPLAYERWORK *MBPlayerWorkGet(int playerNo)
{
    return &playerWork[playerNo];
}

void MBPlayerPreTurnHookSet(int playerNo, MBPLAYERTURNHOOK hook)
{
    playerWork[playerNo].preTurnHook = hook;
}

void MBPlayerPostTurnHookSet(int playerNo, MBPLAYERTURNHOOK hook)
{
    playerWork[playerNo].postTurnHook = hook;
}

static void PlayerMoveExec(int playerNo);
static void StoryPlayerMoveExec(void);

void MBTurnExecParty(int playerNo)
{
    int i, j;
    startPlayerNo = playerNo;
    blackoutF = FALSE;
    for(i=GwSystem.turnPlayerNo; i<GW_PLAYER_MAX; i++) {
        int no;
        GwSystem.turnPlayerNo = i;
        no = 1;
        GwPlayer[i].orderNo = 0;
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(i != j) {
                GwPlayer[j].orderNo = no++;
            }
        }
        if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            MBTutorialExec(TUTORIAL_INST_TURN);
        }
        PlayerMoveExec(i);
        startPlayerNo = 0;
         if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            MBTutorialExec(TUTORIAL_INST_TURNEND);
        }
    }
}

void MBTurnExecStory(int playerNo)
{
    int i, j;
    startPlayerNo = playerNo;
    blackoutF = FALSE;
    if(playerNo != 0) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBPlayerStoryLoseExec();
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
        }
    }
    for(i=(GwSystem.turnPlayerNo == MBPlayerStoryPlayerGet()) ? 0 : 1; i<2; i++) {
        if(i == 0) {
            PlayerMoveExec(MBPlayerStoryPlayerGet());
            startPlayerNo = 0;
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
            GwSystem.turnPlayerNo = 1;
        } else {
            StoryPlayerMoveExec();
        }
        if(i == 0) {
            GwPlayer[0].orderNo = GW_PLAYER_MAX-1;
            for(j=0; j<GW_PLAYER_MAX-1; j++) {
                GwPlayer[j+1].orderNo = j;
            }
        } else {
            GwPlayer[0].orderNo = 0;
            for(j=0; j<GW_PLAYER_MAX-1; j++) {
                GwPlayer[j+1].orderNo = j+1;
            }
        }
    }
}

static void ExecPreTurnHook(int playerNo);
static void ExecPostTurnHook(int playerNo);

static int SaiExec(int playerNo);
static void PlayerWalkMain(int playerNo);

static void PlayerMoveExec(int playerNo)
{
    BOOL saiResult;
    BOOL eventResult;
    GwSystem.turnPlayerNo = playerNo;
    MBPlayerPosReset(playerNo);
    MBPlayerPosFixPlayer(playerNo, GwPlayer[playerNo].masuId, TRUE);
    MBStarDispSetAll(TRUE);
    MBStarMasuDispSet(GwPlayer[playerNo].masuId, FALSE);
    if(startPlayerNo == 0) {
        MBCameraSkipSet(FALSE);
        MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
        MBCameraMotionWait();
        MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
        MBPlayerRotYSet(playerNo, 0);
        MBMasuPlayerLightSet(playerNo, TRUE);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        }
        WipeWait();
        GwPlayer[playerNo].capsuleUse = CAPSULE_NULL;
        ExecPreTurnHook(playerNo);
        omVibrate(playerNo, 12, 12, 0);
        MBTelopPlayerCreate(playerNo);
        GwPlayer[playerNo].walkNum = -1;
        GwPlayer[playerNo].storySkipEventF = 0;
        GwSystem.playerMode = MB_PLAYER_MODE_DEFAULT;
    }
    repeatPlayer:
    switch(GwSystem.playerMode) {
        case MB_PLAYER_MODE_DEFAULT:
        case MB_PLAYER_MODE_SAIEXEC:
            _ClearFlag(FLAG_BOARD_WALKDONE);
            if(startPlayerNo != 0 || blackoutF) {
                MBCameraSkipSet(blackoutF ^ 1);
                MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
                MBCameraMotionWait();
                if(blackoutF) {
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                    blackoutF = FALSE;
                }
                startPlayerNo = 0;
            }
            saiResult = SaiExec(playerNo);
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
            if(GwPlayer[playerNo].walkNum != 0) {
                GwSystem.playerMode = MB_PLAYER_MODE_WALK;
            }
            if(saiResult) {
                MBKillerExec(playerNo);
                GwSystem.playerMode = MB_PLAYER_MODE_KETTOU_CHECK;
                goto repeatPlayer;
            }
            
        case MB_PLAYER_MODE_WALK:
            if(GwPlayer[playerNo].walkNum != 0) {
                _ClearFlag(FLAG_BOARD_WALKDONE);
                MBCameraSkipSet(blackoutF ^ 1);
                MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_WALK);
                MBCameraMotionWait();
                if(startPlayerNo != 0 && GwPlayer[playerNo].walkNum) {
                    MBWalkNumCreate(playerNo, TRUE);
                }
                if(blackoutF) {
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                    blackoutF = FALSE;
                }
                startPlayerNo = 0;
                MBCameraSkipSet(TRUE);
                if(GwPlayer[playerNo].walkNum != 0) {
                    PlayerWalkMain(playerNo);
                }
            }
            GwSystem.playerMode = MB_PLAYER_MODE_KETTOU_CHECK;
        
        case MB_PLAYER_MODE_KETTOU_CHECK:
            GwSystem.playerMode = MB_PLAYER_MODE_WALKEND;
            if(GWPartyFGet() != FALSE && _CheckFlag(FLAG_BOARD_LAST5) && MBPlayerKettouCheck(playerNo, GwPlayer[playerNo].masuId)) {
                MBKettouExec(playerNo);
            }
            
        case MB_PLAYER_MODE_WALKEND:
            if(startPlayerNo != 0 || blackoutF) {
                MBCameraSkipSet(blackoutF ^ 1);
                MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
                MBCameraMotionWait();
                if(blackoutF) {
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                    blackoutF = FALSE;
                }
                startPlayerNo = 0;
            }
            GwSystem.playerMode = MB_PLAYER_MODE_WALKEND_EVENT;
            MBMasuColorSet(playerNo);
            eventResult = MBMasuWalkEndExec(playerNo, GwPlayer[playerNo].masuId);
            MBPlayerStoryLoseExec();
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
            if(eventResult == FALSE) {
                GwSystem.playerMode = MB_PLAYER_MODE_ROT_PLAYER;
                goto repeatPlayer;
            }
            
        case MB_PLAYER_MODE_WALKEND_EVENT:
            if(startPlayerNo != 0 || blackoutF) {
                MBCameraSkipSet(blackoutF ^ 1);
                MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
                MBCameraMotionWait();
                if(blackoutF) {
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                    blackoutF = FALSE;
                }
                startPlayerNo = 0;
            }
            GwSystem.playerMode = MB_PLAYER_MODE_ROT_PLAYER;
            MBMasuWalkEndEventExec(playerNo, GwPlayer[playerNo].masuId);
            MBPlayerStoryLoseExec();
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
        
        case MB_PLAYER_MODE_ROT_PLAYER:
            if(startPlayerNo != 0 || blackoutF) {
                MBCameraSkipSet(blackoutF ^ 1);
                MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
                MBCameraMotionWait();
                if(blackoutF) {
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                    blackoutF = FALSE;
                }
                startPlayerNo = 0;
            }
            MBPlayerRotateStart(playerNo, 0, 15);
            HuPrcSleep(15);
            MBPlayerMotIdleSet(playerNo);
        
        default:
            break;
    }
    ExecPostTurnHook(playerNo);
    if(playerNo != GW_PLAYER_MAX-1 || _CheckFlag(FLAG_BOARD_TUTORIAL)) {
        WipeColorSet(255, 255, 255);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        }
        WipeWait();
    } else {
        WipeColorSet(0, 0, 0);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
        }
        WipeWait();
    }
}

static void StoryComSaiExec(void);
static void ExecStoryComWalk(void);
static void SetMasuView(int playerNo);

static void StoryPlayerMoveExec(void)
{
    int masuIdFix[GW_PLAYER_MAX] = { MASU_NULL, MASU_NULL, MASU_NULL, MASU_NULL };
    int alivePlayer[GW_PLAYER_MAX];
    
    int i;
    int aliveNum;
    BOOL eventResult;
    MBPlayerPosFix(masuIdFix, TRUE);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
            MBPLAYERWORK *workP = &playerWork[i];
            workP->moveF = TRUE;
            workP->moveEndF = FALSE;
            workP->masuNext = MASU_NULL;
            workP->playerNo = i;
        }
    }
    if(startPlayerNo == 0) {
        MBCameraSkipSet(FALSE);
        MBCameraFocusPlayerAddAll();
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                MBPlayerMotionNoSet(i, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
                MBPlayerRotYSet(i, 0);
                MBMasuPlayerLightSet(i, TRUE);
                GwPlayer[i].capsuleUse = CAPSULE_NULL;
                GwPlayer[i].walkNum = -1;
                GwPlayer[i].storySkipEventF = 0;
                
            }
        }
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        }
        WipeWait();
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                ExecPreTurnHook(i);
            }
        }
        MBTelopCharCreate(-1, CHARNO_MINIKOOPA, TRUE);
        GwSystem.playerMode = MB_PLAYER_MODE_DEFAULT;
    }
    switch(GwSystem.playerMode) {
        case MB_PLAYER_MODE_DEFAULT:
        case MB_PLAYER_MODE_ADDCAPSULE:
        case MB_PLAYER_MODE_SAIEXEC:
            if(startPlayerNo != 0 || blackoutF) {
                if(blackoutF) {
                    MBCameraSkipSet(FALSE);
                    MBCameraFocusPlayerAddAll();
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                    MBCameraSkipSet(TRUE);
                    blackoutF = FALSE;
                } else {
                    MBCameraSkipSet(TRUE);
                    MBCameraPointFocusSet(NULL, NULL, -1, 30);
                    MBCameraMotionWait();
                }
                startPlayerNo = 0;
            }
            StoryComSaiExec();
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
            GwSystem.playerMode = MB_PLAYER_MODE_WALK;
       
        case MB_PLAYER_MODE_WALK:
            aliveNum = MBPlayerAlivePlayerGet(alivePlayer);
            for(i=0; i<aliveNum; i++) {
                if(GwPlayer[alivePlayer[i]].walkNum != 0) {
                    break;
                }
            }
            if(i < aliveNum) {
                if(startPlayerNo != 0 || blackoutF) {
                    if(!blackoutF) {
                        MBStatusDispSetAll(FALSE);
                        while(!MBStatusOffCheckAll()) {
                            HuPrcVSleep();
                        }
                    } else {
                        MBStatusDispForceSetAll(FALSE);
                    }
                    if(blackoutF) {
                        MBCameraSkipSet(FALSE);
                        MBCameraFocusPlayerAddAll();
                        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                        WipeWait();
                        MBCameraSkipSet(TRUE);
                        blackoutF = FALSE;
                    } else {
                        MBCameraSkipSet(TRUE);
                        MBCameraPointFocusSet(NULL, NULL, -1, 30);
                        MBCameraMotionWait();
                    }
                    
                    startPlayerNo = 0;
                }
                for(i=0; i<aliveNum; i++) {
                    if(GwPlayer[alivePlayer[i]].walkNum != 0) {
                        MBWalkNumCreateColor(alivePlayer[i], FALSE, SAI_COLOR_RED);
                    }
                }
                MBCameraSkipSet(TRUE);
                ExecStoryComWalk();
            }
            MBStatusDispSetAll(TRUE);
            while(!MBStatusOffCheckAll()) {
                HuPrcVSleep();
            }
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                    MBMasuColorSet(i);
                }
            }
            GwSystem.playerMode = MB_PLAYER_MODE_WALKEND;
            GwSystem.turnPlayerNo = 1;
            
        case MB_PLAYER_MODE_WALKEND:
        case MB_PLAYER_MODE_WALKEND_EVENT:
        case MB_PLAYER_MODE_ROT_PLAYER:
            aliveNum = MBPlayerAlivePlayerGet(NULL);
            if(startPlayerNo != 0 && blackoutF && !MBPlayerAliveCheck(GwSystem.turnPlayerNo)) {
                GwSystem.playerMode = MB_PLAYER_MODE_WALKEND;
            }
            for(i=GwSystem.turnPlayerNo; i<GW_PLAYER_MAX; i++) {
                if(!MBPlayerStoryComCheck(i) || !MBPlayerAliveCheck(i)) {
                    continue;
                }
                if(aliveNum > 1) {
                    if(!blackoutF) {
                        WipeColorSet(255, 255, 255);
                        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
                        }
                        WipeWait();
                        SetMasuView(i);
                        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
                        }
                        WipeWait();
                    } else {
                        SetMasuView(i);
                        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
                        }
                        WipeWait();
                        blackoutF = FALSE;
                    }
                } else {
                    MBCameraSkipSet(TRUE);
                    MBCameraViewNoSet(i, MB_CAMERA_VIEW_ZOOMIN);
                    MBCameraMotionWait();
                    if(blackoutF) {
                        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
                        }
                        WipeWait();
                        blackoutF = FALSE;
                    }
                }
                startPlayerNo = 0;
                GwSystem.turnPlayerNo = i;
                repeatPlayer:
                switch(GwSystem.playerMode) {
                    case MB_PLAYER_MODE_WALKEND:
                        if(GwPlayer[i].storySkipEventF) {
                            GwSystem.playerMode = MB_PLAYER_MODE_ROT_PLAYER;
                            goto repeatPlayer;
                        } else {
                            GwSystem.playerMode = MB_PLAYER_MODE_WALKEND_EVENT;
                        }
                        eventResult = MBMasuWalkEndExec(i, GwPlayer[i].masuId);
                        MBPlayerStoryLoseExec();
                        if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                            return;
                        }
                        if(blackoutF) {
                            if(MBPlayerAliveCheck(i)) {
                                SetMasuView(i);
                                if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
                                }
                                WipeWait();
                                blackoutF = FALSE;
                            } else {
                                break;
                            }
                        }
                        if(eventResult == FALSE) {
                            GwSystem.playerMode = MB_PLAYER_MODE_ROT_PLAYER;
                            goto repeatPlayer;
                        }
                    
                    case MB_PLAYER_MODE_WALKEND_EVENT:
                        GwSystem.playerMode = MB_PLAYER_MODE_ROT_PLAYER;
                        MBMasuWalkEndEventExec(i, GwPlayer[i].masuId);
                        MBPlayerStoryLoseExec();
                        if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                            return;
                        }
                        if(blackoutF) {
                            if(MBPlayerAliveCheck(i)) {
                                SetMasuView(i);
                                if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
                                }
                                WipeWait();
                                blackoutF = FALSE;
                            } else {
                                break;
                            }
                        }
                    
                    case MB_PLAYER_MODE_ROT_PLAYER:
                        MBPlayerRotateStart(i, 0, 15);
                        HuPrcSleep(15);
                        MBPlayerMotIdleSet(i);
                        break;
                }
                if(i != GW_PLAYER_MAX-1) {
                    GwSystem.playerMode = MB_PLAYER_MODE_WALKEND;
                }
            }
       
        default:
            break;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
            ExecPostTurnHook(i);
        }
    }
    if(GwSystem.turnNo < GwSystem.turnMax) {
        if(!blackoutF) {
            WipeColorSet(255, 255, 255);
            if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
            }
            WipeWait();
        }
    } else {
        WipeColorSet(0, 0, 0);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
        }
        WipeWait();
    }
}

int MBPlayerSaiTypeGet(int saiMode)
{
    int saiTbl[5][2] = {
        MB_SAIMODE_NORMAL, SAITYPE_NORMAL,
        MB_SAIMODE_KINOKO, SAITYPE_KINOKO,
        MB_SAIMODE_SKINOKO, SAITYPE_SKINOKO,
        MB_SAIMODE_NKINOKO, SAITYPE_NKINOKO,
        MB_SAIMODE_KILLER, SAITYPE_KILLER,
    };
    int i;
    for(i=0; i<5; i++) {
        if(saiTbl[i][0] == saiMode) {
            return saiTbl[i][1];
        }
    }
    return SAITYPE_NORMAL;
}

static int SaiExec(int playerNo)
{
    BOOL killerF = FALSE;
    int tutorialVal[2] = { 2, 4 };
    BOOL capsuleSkip = FALSE;
    int result;
    GwPlayer[playerNo].saiNum = 1;
    repeatFunc:
    if(GwPlayer[playerNo].capsuleUse == CAPSULE_NULL && capsuleSkip == FALSE && MBPlayerCapsuleNumGet(playerNo) != 0) {
        GwSystem.playerMode = MB_PLAYER_MODE_DEFAULT;
        if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            MBTutorialExec(TUTORIAL_INST_CAPSULE);
        }
        result = MBCapsuleSelExec();
        if(GwPlayer[playerNo].capsuleUse != CAPSULE_NULL && _CheckFlag(FLAG_BOARD_STORYEND)) {
            return -1;
        }
    } else {
        MBStatusDispSetAll(TRUE);
        while(!MBStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        MBPauseDisableSet(FALSE);
        GwSystem.playerMode = MB_PLAYER_MODE_SAIEXEC;
        if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            int saiVal = MBTutorialExec(TUTORIAL_INST_SAI);
            if(saiVal >= 0) {
                result = MBSaiExec(playerNo, MBPlayerSaiTypeGet(GwPlayer[playerNo].saiMode), NULL, saiVal, FALSE, TRUE, NULL, SAI_COLOR_GREEN);
            } else {
                result = MBSaiTutorialExec(playerNo, MBPlayerSaiTypeGet(GwPlayer[playerNo].saiMode), NULL, tutorialVal, FALSE, TRUE, NULL, SAI_COLOR_GREEN);
            }
            MBTutorialExec(TUTORIAL_INST_WALK_START);
        } else {
            result = MBSaiPlayerExec(playerNo, MBPlayerSaiTypeGet(GwPlayer[playerNo].saiMode));
        }
    }
    
    switch(result) {
        case SAI_RESULT_SCROLL:
            MBSaiKill(playerNo);
            MBScrollExec(playerNo);
            break;
        
        case SAI_RESULT_CAPSULESEL:
            capsuleSkip = FALSE;
            MBSaiKill(playerNo);
            break;
       
        case SAI_RESULT_CAPSULESKIP:
            capsuleSkip = TRUE;
            MBSaiKill(playerNo);
            break;
    }
    if(result <= 0) {
        goto repeatFunc;
    }
    if(GwPlayer[playerNo].saiMode == MB_SAIMODE_KILLER) {
        killerF = TRUE;
    }
    GwPlayer[playerNo].saiMode = MB_SAIMODE_NORMAL;
    GwPlayer[playerNo].walkNum = result;
    MBWalkNumCreate(playerNo, TRUE);
    MBSaiNumKill(playerNo);
    MBPauseDisableSet(TRUE);
    return killerF;
}

static void StoryComSaiExec(void)
{
    BOOL tempF = TRUE;
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
            GwPlayer[i].saiNum = 1;
        }
    }
    switch(GwSystem.playerMode) {
        case MB_PLAYER_MODE_DEFAULT:
            MBCapsuleSelStoryExec();
            GwSystem.turnPlayerNo = 1;
            GwSystem.playerMode = MB_PLAYER_MODE_ADDCAPSULE;
        
        case MB_PLAYER_MODE_ADDCAPSULE:
            MBCapsuleAddStory();
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
            GwSystem.playerMode = MB_PLAYER_MODE_SAIEXEC;
            MBStatusDispSetAll(FALSE);
            while(!MBStatusOffCheckAll()) {
                HuPrcVSleep();
            }
       
        case MB_PLAYER_MODE_SAIEXEC:
            MBPauseDisableSet(FALSE);
            GwSystem.playerMode = MB_PLAYER_MODE_SAIEXEC;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                    MBSaiExec(i, MBPlayerSaiTypeGet(GwPlayer[i].saiMode), NULL, SAI_VALUE_ANY, FALSE, FALSE, NULL, SAI_COLOR_RED);
                }
                
            }
            while(MBSaiKillCheckAll() == FALSE) {
                HuPrcVSleep();
            }
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                    GwPlayer[i].saiMode = MB_SAIMODE_NORMAL;
                    GwPlayer[i].walkNum = MBSaiResultGet(i);
                    MBSaiNumKill(i);
                }
            }
        default:
            break;
    }
    MBPauseDisableSet(TRUE);
}

static void PlayerWalkProc(void);
static void DestroyWalkProc(void);

static void PlayerWalkMain(int playerNo)
{
    MBPLAYERWORK *workP = &playerWork[playerNo];
    MBPauseDisableSet(FALSE);
    MBPlayerPosFixFlagSet(TRUE);
    workP->walkProc = MBPrcCreate(PlayerWalkProc, 8205, 24576);
    workP->walkProc->property = workP;
    MBPrcDestructorSet(workP->walkProc, DestroyWalkProc);
    while(workP->walkProc) {
        HuPrcVSleep();
    }
    _SetFlag(FLAG_BOARD_WALKDONE);
    MBPlayerPosFixFlagSet(FALSE);
    MBPauseDisableSet(TRUE);
}

static void ExecStoryComWalk(void)
{
    int walkPlayerTbl[GW_PLAYER_MAX];
    int playerNoTbl[GW_PLAYER_MAX];
    int masuIdFix[GW_PLAYER_MAX] = { MASU_NULL, MASU_NULL, MASU_NULL, MASU_NULL };
    int i, j;
    MBPLAYERWORK *workP;
    int movingNum;
    int walkPlayerNum;
    MBPauseDisableSet(FALSE);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i) && GwPlayer[i].walkNum != 0) {
            workP = &playerWork[i];
            workP->walkProc = MBPrcCreate(PlayerWalkProc, 8205, 24576);
            workP->walkProc->property = workP;
            MBPrcDestructorSet(workP->walkProc, DestroyWalkProc);
        }
    }
    while(1) {
        MBPlayerPosFixFlagSet(TRUE);
        for(i=0; i<GW_PLAYER_MAX; i++) {
            masuIdFix[i] = MASU_NULL;
        }
        do {
            walkPlayerNum = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                    workP = &playerWork[i];
                    if(workP->walkProc) {
                        walkPlayerTbl[walkPlayerNum++] = i;
                    }
                }
            }
            movingNum = 0;
            for(i=0; i<walkPlayerNum; i++) {
                workP = &playerWork[walkPlayerTbl[i]];
                if(workP->masuNext >= 0) {
                    movingNum++;
                }
            }
            if(movingNum != walkPlayerNum) {
                for(i=0; i<walkPlayerNum; i++) {
                    workP = &playerWork[walkPlayerTbl[i]];
                    if(workP->masuNext >= 0) {
                        MBPlayerMotionNoShiftSet(walkPlayerTbl[i], MB_PLAYER_MOT_IDLE, 0, 4, HU3D_MOTATTR_LOOP);
                    }
                }
            }
            HuPrcVSleep();
            if(walkPlayerNum == 0) {
                break;
            }
        } while(movingNum != walkPlayerNum);
        if(walkPlayerNum) {
            int playerNum;
            
            while(!MBPlayerPosFixCheck()) {
                HuPrcVSleep();
            }
            playerNum = 0;
            for(i=0; i<walkPlayerNum; i++) {
                workP = &playerWork[walkPlayerTbl[i]];
                movingNum = 0;
                for(j=i-1; j>=0; j--) {
                    MBPLAYERWORK *work2P = &playerWork[walkPlayerTbl[j]];
                    if(workP->masuNext == work2P->masuNext) {
                        movingNum++;
                    }
                }
                if(movingNum == 0) {
                    playerNoTbl[playerNum++] = walkPlayerTbl[i];
                    HuPrcWakeup(workP->walkProc);
                    workP->walkProc->exec = HUPRC_EXEC_NORMAL;
                    masuIdFix[walkPlayerTbl[i]] = workP->masuNext;
                }
            }
            MBPlayerPosFix(masuIdFix, FALSE);
            for(i=0; i<playerNum; i++) {
                workP = &playerWork[playerNoTbl[i]];
                workP->masuNext = MASU_NULL;
            }
            do {
                movingNum = 0;
                for(i=0; i<playerNum; i++) {
                    workP = &playerWork[playerNoTbl[i]];
                    if(!workP->moveEndF) {
                        movingNum++;
                    }
                }
                HuPrcVSleep();
            } while(movingNum);
            for(i=0; i<playerNum; i++) {
                workP = &playerWork[playerNoTbl[i]];
                workP->moveEndF = FALSE;
                HuPrcWakeup(workP->walkProc);
                workP->walkProc->exec = HUPRC_EXEC_NORMAL;
            }
        } else {
            break;
        }
    }
    while(!MBPlayerPosFixCheck()) {
        HuPrcVSleep();
    }
    MBPlayerPosFixFlagSet(FALSE);
    MBPauseDisableSet(TRUE);
}

static void PlayerPosFixKill(int playerNo);

static void PlayerWalkProc(void)
{
    MBPLAYERWORK *workP = HuPrcCurrentGet()->property;
    int playerNo = workP->playerNo;
    s16 masuNext;
    do {
        GwPlayer[playerNo].masuIdPrev = GwPlayer[playerNo].masuId;
        if(!_CheckFlag(FLAG_BOARD_DEBUG) || (GWPartyFGet() == FALSE && workP->moveF) || _CheckFlag(FLAG_BOARD_TUTORIAL)) {
            if(MBBranchExec(playerNo, &masuNext)) {
                break;
            }
        } else if(MBBranchExecDebug(playerNo, &masuNext)) {
            break;
        } else if(masuNext < 0) {
            break;
        }
        masuNext = MBGateMasuExec(playerNo, GwPlayer[playerNo].masuId, masuNext);
        GwPlayer[playerNo].masuIdNext = masuNext;
        workP->masuNext = masuNext;
        if(workP->moveF) {
            HuPrcSleep(-1);
        } else {
            MBPlayerPosFixPlayer(workP->playerNo, masuNext, FALSE);
        }
        PlayerPosFixKill(playerNo);
        MBMasuWalkPreExec(masuNext);
        MBPlayerMasuWalk(playerNo, TRUE);
        GwPlayer[playerNo].masuId = masuNext;
        if(MBMasuWalkPostExec(playerNo) == FALSE) {
            masuNext = GwPlayer[playerNo].masuId;
            if(MBMasuWalkExec(playerNo, masuNext) == FALSE) {
                int type = MBMasuTypeGet(MASU_LAYER_DEFAULT, masuNext);
                u32 flag = MBMasuFlagGet(MASU_LAYER_DEFAULT, masuNext);
                if(type != MASU_TYPE_NONE && type != MASU_TYPE_STAR) {
                    MBAudFXPlay(MSM_SE_BOARD_02);
                    GwPlayer[playerNo].walkNum--;
                    if(GwPlayer[playerNo].walkNum == 0) {
                        MBWalkNumKill(playerNo);
                    }
                }
            }
        }
        if(workP->moveF) {
            workP->moveEndF = TRUE;
            HuPrcSleep(-1);
        }
        if(GWPartyFGet() == FALSE && MBPlayerKettouStoryCheck(playerNo) && GwPlayer[playerNo].masuIdPrev >= 0) {
            MBWalkNumKill(playerNo);
            GwSystem.turnPlayerNo = playerNo;
            MBPauseDisableSet(TRUE);
            MBPlayerPosFixOrderSet();
            MBKettouExec(playerNo);
            MBPauseDisableSet(FALSE);
        }
    } while(GwPlayer[playerNo].walkNum);
    MBWalkNumKill(playerNo);
    if(workP->moveF) {
        MBPlayerRotateStart(playerNo, 0, 15);
        while(MBPlayerRotateCheck(playerNo) == FALSE) {
            HuPrcVSleep();
        }
        MBPlayerMotIdleSet(playerNo);
        MBPlayerPosFixOrderSet();
    } else {
        MBPlayerMotIdleSet(playerNo);
    }
    HuPrcEnd();
}

static void DestroyWalkProc(void)
{
    MBPLAYERWORK *workP = HuPrcCurrentGet()->property;
    workP->walkProc = NULL;
}

static void ExecPreTurnHook(int playerNo)
{
    if(playerWork[playerNo].preTurnHook) {
        if(playerWork[playerNo].preTurnHook()) {
            playerWork[playerNo].preTurnHook = NULL;
        }
    }
}

static void ExecPostTurnHook(int playerNo)
{
    if(playerWork[playerNo].postTurnHook) {
        if(playerWork[playerNo].postTurnHook()) {
            playerWork[playerNo].postTurnHook = NULL;
        }
    }
}

void MBPlayerMasuWalkTo(int playerNo, int masuId, BOOL waitF)
{
    GwPlayer[playerNo].masuIdNext = masuId;
    MBPlayerMasuWalk(playerNo, waitF);
}

void MBPlayerMasuWalk(int playerNo, BOOL waitF)
{
    MBPlayerWalkExec(playerNo, NULL, NULL, MBPlayerWalkSpeedGet(), NULL, waitF);
}

void MBPlayerMasuWalkPos(int playerNo, HuVecF *pos, BOOL waitF)
{
    MBPlayerWalkExec(playerNo, NULL, pos, MBPlayerWalkSpeedGet(), NULL, waitF);
}

void MBPlayerMasuWalkSpeed(int playerNo, int masuId, s16 maxTime, BOOL waitF)
{
    HuVecF pos;
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
    MBPlayerWalkExec(playerNo, NULL, &pos, maxTime, NULL, waitF);
}

void MBPlayerWalkExec(int playerNo, HuVecF *srcPos, HuVecF *dstPos, s16 maxTime, HuVecF *rot, BOOL waitF)
{
    MBPlayerWalkMain(playerNo, srcPos, dstPos, MB_MOT_DEFAULT, 1.0f, HU3D_MOTATTR_LOOP, maxTime, rot, waitF);
}

#define PLAYER_WALK_MODE_RUN 0
#define PLAYER_WALK_MODE_JUMP 1
#define PLAYER_WALK_MODE_CLIMB 2

typedef struct PlayerWalkWork_s {
    unsigned killF : 1;
    unsigned mode : 2;
    unsigned playerNo : 2;
    s16 time;
    s16 maxTime;
} PLAYERWALKWORK;

static void UpdatePlayerWalk(OMOBJ *obj);

void MBPlayerWalkMain(int playerNo, HuVecF *srcPos, HuVecF *dstPos, u32 motId, float motSpeed, u32 motAttr, s16 maxTime, HuVecF *rot, BOOL waitF)
{
    BOOL setAngle = FALSE;
    OMOBJ *obj = playerWork[playerNo].walkObj = MBAddObj(256, 0, 0, UpdatePlayerWalk);
    PLAYERWALKWORK *work = omObjGetWork(obj, PLAYERWALKWORK);
    int mode;
    HuVecF walkDir;
    HuVecF playerRot;
    if(srcPos != NULL) {
        obj->trans.x = srcPos->x;
        obj->trans.y = srcPos->y;
        obj->trans.z = srcPos->z;
    } else {
        MBPlayerPosGet(playerNo, &obj->trans);
    }
    if(dstPos != NULL) {
        obj->rot.x = dstPos->x;
        obj->rot.y = dstPos->y;
        obj->rot.z = dstPos->z;
        work->mode = PLAYER_WALK_MODE_RUN;
    } else {
        MASU *masuPrev;
        MASU *masuNext;
        s16 masuIdPrev;
        int masuIdNext;
        masuIdNext = GwPlayer[playerNo].masuIdNext;
        masuNext = MBMasuGet(MASU_LAYER_DEFAULT, masuIdNext);
        masuIdPrev = GwPlayer[playerNo].masuIdPrev;
        if(masuIdPrev < 0) {
            mode = PLAYER_WALK_MODE_RUN;
        } else {
            masuPrev = MBMasuGet(MASU_LAYER_DEFAULT, masuIdPrev);
            if((masuPrev->flag & MASU_FLAG_JUMPFROM) && (masuNext->flag & MASU_FLAG_JUMPTO)) {
                mode = PLAYER_WALK_MODE_JUMP;
            } else if((masuPrev->flag & MASU_FLAG_CLIMBFROM) && (masuNext->flag & MASU_FLAG_CLIMBTO)) {
                mode = PLAYER_WALK_MODE_CLIMB;
            } else {
                mode = PLAYER_WALK_MODE_RUN;
            }
        }
        work->mode = mode;
        MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuIdNext, &obj->rot);
    }
    if(motId == MB_MOT_DEFAULT) {
        
        switch(work->mode) {
            case PLAYER_WALK_MODE_RUN:
                MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_RUN, 0, 4, HU3D_MOTATTR_LOOP);
                break;
            
            case PLAYER_WALK_MODE_JUMP:
                MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_JUMP, 0, 4, HU3D_MOTATTR_NONE);
                break;
            
            case PLAYER_WALK_MODE_CLIMB:
                MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_CLIMB, 0, 4, HU3D_MOTATTR_LOOP);
                maxTime = 100;
                motSpeed = 2;
                VECSubtract(&obj->rot, &obj->trans, &walkDir);
                maxTime = VECMag(&walkDir)/10.0f;
                if(obj->trans.y >= obj->rot.y) {
                    walkDir.x = -walkDir.x;
                    walkDir.y = -walkDir.y;
                    walkDir.z = -walkDir.z;
                    motSpeed = -motSpeed;
                }
                playerRot.x = playerRot.z = 0;
                playerRot.y = HuAtan(walkDir.x, walkDir.z);
                MBPlayerRotSetV(playerNo, &playerRot);
                setAngle = TRUE;
                break;
        }
    } else {
        MBPlayerMotionNoShiftSet(playerNo, motId, 0, 4, motAttr);
    }
    MBPlayerMotionSpeedSet(playerNo, motSpeed);
    if(setAngle == FALSE) {
        if(rot == NULL) {
            float rotY;
            VECSubtract(&obj->rot, &obj->trans, &playerRot);
            rotY = 90-HuAtan(playerRot.z, playerRot.x);
            MBPlayerRotYSet(playerNo, rotY);
        } else {
            MBPlayerRotSetV(playerNo, rot);
        }
    }
    MBPlayerPosSetV(playerNo, srcPos);
    obj->scale.x = obj->trans.x;
    obj->scale.y = obj->trans.y;
    obj->scale.z = obj->trans.z;
    work->playerNo = playerNo;
    work->time = 0;
    work->maxTime = maxTime;
    GwPlayer[playerNo].moveF = TRUE;
    if(!waitF) {
        return;
    }
    while(GwPlayer[playerNo].moveF) {
        HuPrcVSleep();
    }
}

static void UpdatePlayerWalk(OMOBJ *obj)
{
    PLAYERWALKWORK *work = omObjGetWork(obj, PLAYERWALKWORK);
    float weight;
    if(MBKillCheck() || work->killF) {
        GwPlayer[work->playerNo].moveF = FALSE;
        MBDelObj(obj);
        playerWork[work->playerNo].walkObj = NULL;
        return;
    }
    work->time++;
    weight = (float)work->time/work->maxTime;
    obj->trans.x = obj->scale.x+(weight*(obj->rot.x-obj->scale.x));
    obj->trans.y = obj->scale.y+(weight*(obj->rot.y-obj->scale.y));
    obj->trans.z = obj->scale.z+(weight*(obj->rot.z-obj->scale.z));
    if(work->time >= work->maxTime) {
        GwPlayer[work->playerNo].moveF = FALSE;
        MBDelObj(obj);
        MBPlayerPosSet(work->playerNo, obj->rot.x, obj->rot.y, obj->rot.z);
    } else {
        if(work->mode != PLAYER_WALK_MODE_JUMP) {
            MBPlayerPosSet(work->playerNo, obj->trans.x, obj->trans.y, obj->trans.z);
        } else {
            MBPlayerPosSet(work->playerNo, obj->trans.x, obj->trans.y+(100*(1.5*HuSin((work->time*180.0f)/work->maxTime))), obj->trans.z);
            if(work->time == work->maxTime-4) {
                MBPlayerMotionNoShiftSet(work->playerNo, MB_PLAYER_MOT_JUMPEND, 0, 4, HU3D_MOTATTR_NONE);
            }
        }
    }
}

#undef PLAYER_WALK_MODE_RUN
#undef PLAYER_WALK_MODE_JUMP
#undef PLAYER_WALK_MODE_CLIMB


typedef struct PlayerRotateWork_s {
    unsigned killF : 1;
    s8 playerNo;
    s16 maxTime;
    s16 time;
} PLAYERROTATEWORK;

static void UpdatePlayerRotate(OMOBJ *obj);

void MBPlayerRotateStart(int playerNo, s16 endAngle, s16 maxTime)
{
    OMOBJ *obj;
    PLAYERROTATEWORK *work;
    float angle;
    if(maxTime <= 0) {
        return;
    }
    if(playerWork[playerNo].rotateObj) {
        obj = playerWork[playerNo].rotateObj;
    } else {
        playerWork[playerNo].rotateObj = obj = MBAddObj(256, 0, 0, UpdatePlayerRotate);
    }
    work = omObjGetWork(obj, PLAYERROTATEWORK);
    work->killF = FALSE;
    work->maxTime = maxTime;
    work->playerNo = playerNo;
    work->time = 0;
    obj->rot.y = MBPlayerRotYGet(playerNo);
    obj->scale.z = endAngle;
    angle = fmod(endAngle-obj->rot.y, 360);
    if((s16)angle == 0) {
        work->killF = TRUE;
    } else {
        if(angle < 0) {
            angle += 360;
        }
        if(angle > 180) {
            angle -= 360;
        }
        obj->scale.y = angle;
        if(fabs(angle) > 5) {
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_WALK, 0, 5, HU3D_MOTATTR_LOOP);
        } else {
            MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
        }
    }
}

static void UpdatePlayerRotate(OMOBJ *obj)
{
    PLAYERROTATEWORK *work = omObjGetWork(obj, PLAYERROTATEWORK);
    float rotY;
    float angle;
    float weight;
    if(work->killF || MBKillCheck()) {
        MBPlayerRotYSet(work->playerNo, obj->scale.z);
        playerWork[work->playerNo].rotateObj = NULL;
        MBDelObj(obj);
        return;
    }
    
    angle = (float)(work->time++)/work->maxTime;
    weight = HuSin(angle*90.0f);
    rotY = obj->rot.y+(weight*obj->scale.y);
    MBPlayerRotYSet(work->playerNo, rotY);
    if(work->time >= work->maxTime) {
        work->killF = TRUE;
        MBPlayerMotionNoSet(work->playerNo, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
        return;
    }
}

BOOL MBPlayerRotateCheck(int playerNo)
{
    return playerWork[playerNo].rotateObj == NULL;
}

BOOL MBPlayerRotateCheckAll(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(playerWork[i].rotateObj != NULL) {
            return FALSE;
        }
    }
    return TRUE;
}

void MBPlayerSaiMotExec(int playerNo)
{
    int time;
    MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_JUMPSAI, HU3D_MOTATTR_NONE);
    time = 0;
    do {
        if(time++ == 27) {
            MBSaiObjHit(playerNo);
        }
        HuPrcVSleep();
    } while(MBPlayerMotionEndCheck(playerNo) == FALSE);
    MBPlayerMotIdleSet(playerNo);
}

typedef struct WalkNumWork_s {
    unsigned killF : 1;
    unsigned dispF : 1;
    unsigned playerNo : 2;
    unsigned carF : 1;
} WALKNUMWORK;

static void UpdateWalkNum(OMOBJ *obj);

static const int numberFileTbl[] = {
    BOARD_HSF_num0,
    BOARD_HSF_num1,
    BOARD_HSF_num2,
    BOARD_HSF_num3,
    BOARD_HSF_num4,
    BOARD_HSF_num5,
    BOARD_HSF_num6,
    BOARD_HSF_num7,
    BOARD_HSF_num8,
    BOARD_HSF_num9
};

void MBWalkNumCreateColor(int playerNo, BOOL carF, int color)
{
    int modelId;
    HU3DCAMERA *cameraP;
    OMOBJ *obj;
    WALKNUMWORK *work;
    int i;
    cameraP = &Hu3DCamera[0];
    if(playerWork[playerNo].walkNumObj) {
        return;
    }
    playerWork[playerNo].walkNumObj = obj = MBAddObj(32258, 20, 0, UpdateWalkNum);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    work = omObjGetWork(obj, WALKNUMWORK);
    work->dispF = TRUE;
    work->killF = FALSE;
    work->playerNo = playerNo;
    work->carF = carF;
    for(i=0; i<20; i++) {
        HU3DMODELID rawMdlId;
        modelId = MBModelCreate(MBDATANUM(numberFileTbl[i%10]), NULL, FALSE);
        MBModelDispSet(modelId, FALSE);
        MBMotionNoSet(modelId, MB_MOT_DEFAULT, HU3D_MOTATTR_NONE);
        MBMotionSpeedSet(modelId, 0.0f);
        MBMotionTimeSet(modelId, color+0.5f);
        MBModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
        rawMdlId = MBModelIdGet(modelId);
        Hu3DModelAttrSet(rawMdlId, HU3D_ATTR_ZCMP_OFF);
        Hu3DModelLayerSet(rawMdlId, 3);
        obj->mdlId[i] = modelId;
    }
    if(carF) {
        Mtx lookAt;
        HuVecF pos;
        HuVecF posCamera;
        float tanFov;
        MBPlayerPosGet(playerNo, &pos);
        pos.y += 300;
        MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
        MTXMultVec(lookAt, &pos, &posCamera);
        tanFov = HuSin(cameraP->fov*0.5f)/HuCos(cameraP->fov*0.5f);
        obj->rot.y = posCamera.y/(tanFov*posCamera.z);
        obj->rot.z = -posCamera.z;
        obj->rot.y = -0.447408f;
        obj->rot.z = 1691.0725f;
    }
}

void MBWalkNumCreate(int playerNo, BOOL carF)
{
    MBWalkNumCreateColor(playerNo, carF, SAI_COLOR_GREEN);
}

static void UpdateWalkNum(OMOBJ *obj)
{
    int digitNum = 0;
    WALKNUMWORK *work = omObjGetWork(obj, WALKNUMWORK);
    HU3DCAMERA *cameraP = &Hu3DCamera[0];
    int i;
    Mtx lookAt;
    HuVecF pos;
    HuVecF posCamera;
    float posX, posY, posZ;
    float tanFov;
    
    if(work->killF || MBKillCheck()) {
        for(i=0; i<20; i++) {
            if(obj->mdlId[i] != MB_MODEL_NONE) {
                MBModelKill(obj->mdlId[i]);
            }
        }
        MBDelObj(obj);
        playerWork[work->playerNo].walkNumObj = NULL;
        return;
    }
    if(MBPauseFlagCheck()) {
        return;
    }
    MBPlayerPosGet(work->playerNo, &pos);
    pos.y += 300;
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXMultVec(lookAt, &pos, &posCamera);
    tanFov = HuSin(cameraP->fov*0.5f)/HuCos(cameraP->fov*0.5f);
    posX = posCamera.x/(tanFov*posCamera.z);
    if(work->carF) {
        posY = obj->rot.y;
        posZ = obj->rot.z;
    } else {
        posY = posCamera.y/(tanFov*posCamera.z);
        posZ = 2000;
    }
    pos.x = -posZ*(posX*tanFov);
    pos.y = -posZ*(posY*tanFov);
    pos.z = -posZ;
    MTXInverse(lookAt, lookAt);
    MTXMultVec(lookAt, &pos, &pos);
    for(i=0; i<20; i++) {
        MBModelDispSet(obj->mdlId[i], FALSE);
    }
    if(work->dispF) {
        int modelNo = GwPlayer[work->playerNo].walkNum/10;
        if(modelNo != 0) {
            MBModelDispSet(obj->mdlId[modelNo], TRUE);
            MBModelPosSet(obj->mdlId[modelNo], pos.x-50, pos.y, pos.z);
            digitNum++;
        }
        modelNo = (GwPlayer[work->playerNo].walkNum%10)+10;
        if(modelNo != 0) {
            MBModelDispSet(obj->mdlId[modelNo], TRUE);
            if(digitNum == 0) {
                MBModelPosSet(obj->mdlId[modelNo], pos.x, pos.y, pos.z);
            } else {
                MBModelPosSet(obj->mdlId[modelNo], pos.x+50, pos.y, pos.z);
            }
        }
    }
}

void MBWalkNumKill(int playerNo)
{
    if(playerWork[playerNo].walkNumObj) {
        WALKNUMWORK *work = omObjGetWork(playerWork[playerNo].walkNumObj, WALKNUMWORK);
        work->killF = TRUE;
    }
}

void MBWalkNumDispSet(int playerNo, BOOL dispF)
{
    if(playerWork[playerNo].walkNumObj) {
        WALKNUMWORK *work = omObjGetWork(playerWork[playerNo].walkNumObj, WALKNUMWORK);
        work->dispF = dispF;
    }
}

#define POS_FIX_MODE_MOVE 0
#define POS_FIX_MODE_ROTATE 1

typedef struct PosFixWork_s {
    unsigned motStartF : 1;
    unsigned killF : 1;
    unsigned playerNo : 2;
    unsigned mode : 2;
    unsigned cornerNo : 2;
    u8 masuId;
    s8 time;
    s8 maxTime;
    float rotYStart;
} POSFIXWORK;

static void PlayerPosFixUpdate(OMOBJ *obj);
static void PlayerPosFixInit(int playerNo, int masuId, int cornerNo);

void MBPlayerPosFix(int *masuIdFix, BOOL snapF)
{
    int i, j;
    int cornerNo;
    int masuId;
    s8 orderNo;
    HuVecF pos;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(masuIdFix[i] >= 0) {
            MBPlayerMasuCornerSet(i, 0);
            continue;
        }
        masuId = GwPlayer[i].masuId;
        orderNo = GwPlayer[i].orderNo;
        cornerNo = 0;
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(i != j && MBPlayerAliveCheck(j) && masuId == masuIdFix[j]) {
                cornerNo++;
            }
        }
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(i != j && MBPlayerAliveCheck(j) && masuIdFix[j] < 0 && masuId == GwPlayer[j].masuId && orderNo > GwPlayer[j].orderNo) {
                cornerNo++;
            }
        }
        if(cornerNo == 0) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
        } else {
            MBMasuCornerRotPosGet(masuId, cornerNo-1, &pos);
        }
        if(snapF) {
            MBPlayerPosSetV(i, &pos);
        } else if(cornerNo != MBPlayerMasuCornerGet(i)) {
            if(playerWork[i].posFixObj == NULL) {
                playerWork[i].posFixObj = MBAddObj(256, 0, 0, PlayerPosFixUpdate);
            }
            PlayerPosFixInit(i, masuId, cornerNo);
        }
        MBPlayerMasuCornerSet(i, cornerNo);
    }
}

void MBPlayerPosFixWarp(int playerNo, int masuId, BOOL snapF)
{
    int i, j;
    int cornerNo;
    s8 orderNo;
    HuVecF pos;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(masuId != GwPlayer[i].masuId) {
            continue;
        }
        if(i == playerNo) {
            continue;
        }
        orderNo = GwPlayer[i].orderNo;
        cornerNo = 1;
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(i != j && MBPlayerAliveCheck(j) && masuId == GwPlayer[j].masuId && orderNo > GwPlayer[j].orderNo) {
                cornerNo++;
            }
        }
        MBMasuCornerRotPosGet(masuId, cornerNo-1, &pos);
        if(snapF) {
            MBPlayerPosSetV(i, &pos);
        } else if(cornerNo != MBPlayerMasuCornerGet(i)) {
            if(playerWork[i].posFixObj == NULL) {
                playerWork[i].posFixObj = MBAddObj(256, 0, 0, PlayerPosFixUpdate);
            }
            PlayerPosFixInit(i, masuId, cornerNo);
        }
        MBPlayerMasuCornerSet(i, cornerNo);
    }
    MBPlayerMasuCornerSet(playerNo, 0);
}

void MBPlayerPosFixPlayer(int playerNo, int masuId, BOOL snapF)
{
    int masuIdFix[GW_PLAYER_MAX];
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        masuIdFix[i] = MASU_NULL;
    }
    masuIdFix[playerNo] = masuId;
    MBPlayerPosFix(masuIdFix, snapF);
}

static void PlayerPosFixUpdate(OMOBJ *obj)
{
    POSFIXWORK *work = omObjGetWork(obj, POSFIXWORK);
    int playerNo = work->playerNo;
    Mtx matrix;
    HuVecF pos;
    HuVecF posDiff;
    float weight;
    float rotY;
    
    if(work->killF || MBKillCheck()) {
        MBDelObj(obj);
        playerWork[playerNo].posFixObj = NULL;
        return;
    }
    switch(work->mode) {
        case POS_FIX_MODE_MOVE:
            if(!work->motStartF) {
                MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_RUN, HU3D_MOTATTR_LOOP);
                work->motStartF = TRUE;
                work->time = 0;
                work->maxTime = 12;
                obj->rot.x = obj->trans.x;
                obj->rot.y = obj->trans.y;
                obj->rot.z = obj->trans.z;
            }
            if(work->cornerNo == 0) {
                pos.x = pos.y = pos.z = 0;
            } else {
                MBMasuCornerPosGet(work->masuId, work->cornerNo-1, &pos);
            }
            if(work->time > work->maxTime) {
                work->mode = POS_FIX_MODE_ROTATE;
                work->time = 0;
                work->maxTime = 8;
                obj->trans.x = pos.x;
                obj->trans.y = pos.y;
                obj->trans.z = pos.z;
                MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
            } else {
                weight = (float)(work->time++)/work->maxTime;
                VECSubtract(&pos, &obj->rot, &posDiff);
                obj->trans.x = obj->rot.x+(weight*(pos.x-obj->rot.x));
                obj->trans.y = obj->rot.y+(weight*(pos.y-obj->rot.y));
                obj->trans.z = obj->rot.z+(weight*(pos.z-obj->rot.z));
            }
            break;
       
        case POS_FIX_MODE_ROTATE:
            if(work->time > work->maxTime) {
                MBPlayerRotYSet(playerNo, 0);
                playerWork[playerNo].posFixObj = NULL;
                MBDelObj(obj);
                GwPlayer[playerNo].moveF = FALSE;
                return;
            } else {
                weight = (float)(work->time++)/work->maxTime;
                rotY = MBAngleLerpSin(work->rotYStart, 0, weight);
                MBPlayerRotYSet(playerNo, rotY);
            }
            break;
    }
    MBMasuMtxGet(work->masuId, matrix);
    MTXMultVec(matrix, &obj->trans, &pos);
    MBPlayerPosSetV(playerNo, &pos);
}

static void PlayerPosFixInit(int playerNo, int masuId, int cornerNo)
{
    POSFIXWORK *work = omObjGetWork(playerWork[playerNo].posFixObj, POSFIXWORK);
    Mtx masuMtx, masuMtxInv;
    HuVecF posPlayer, posMasu, posDiff;
    float rotY;
    work->motStartF = FALSE;
    work->killF = FALSE;
    work->mode = POS_FIX_MODE_MOVE;
    work->playerNo = playerNo;
    work->masuId = masuId;
    work->cornerNo = cornerNo;
    MBMasuMtxGet(work->masuId, masuMtx);
    MTXInverse(masuMtx, masuMtxInv);
    MBPlayerPosGet(playerNo, &posPlayer);
    MTXMultVec(masuMtxInv, &posPlayer, &playerWork[playerNo].posFixObj->trans);
    GwPlayer[playerNo].moveF = TRUE;
    if(work->cornerNo == 0) {
        MBMasuPosGet(MASU_LAYER_DEFAULT, work->masuId, &posMasu);
    } else {
        MBMasuCornerRotPosGet(work->masuId, work->cornerNo-1, &posMasu);
    }
    VECSubtract(&posMasu, &posPlayer, &posDiff);
    rotY = 90-HuAtan(posDiff.z, posDiff.x);
    MBPlayerRotYSet(playerNo, rotY);
    work->rotYStart = rotY;
}

BOOL MBPlayerPosFixCheck(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(playerWork[i].posFixObj) {
            return FALSE;
        }
    }
    return TRUE;
}

static void PlayerPosFixKill(int playerNo)
{
    if(playerWork[playerNo].posFixObj) {
        POSFIXWORK *work = omObjGetWork(playerWork[playerNo].posFixObj, POSFIXWORK);
        work->killF = TRUE;
    }
}

void MBPlayerPosFixEvent(int playerNo, int masuId, BOOL snapF)
{
    int masuIdFix[GW_PLAYER_MAX];
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        masuIdFix[i] = MASU_NULL;
    }
    masuIdFix[playerNo] = masuId;
    MBPlayerPosFix(masuIdFix, snapF);
}

void MBPlayerPosFixSet(int playerNo, int masuId)
{
    int i;
    int cornerNo;
    HuVecF pos;
    if(!posFixAnimF) {
        return;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(GwPlayer[i].moveF) {
            continue;
        }
        if(masuId != GwPlayer[i].masuId) {
            continue;
        }
        cornerNo = MBPlayerMasuCornerGet(i);
        if(cornerNo == 0) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
        } else {
            MBMasuCornerRotPosGet(masuId, cornerNo-1, &pos);
        }
        MBPlayerPosSetV(i, &pos);
    }
}

void MBPlayerPosFixFlagSet(BOOL fixF)
{
    posFixAnimF = fixF;
}

void MBPlayerPosFixCurr(int masuId, BOOL snapF)
{
    MBPlayerPosFixPlayer(GwSystem.turnPlayerNo, masuId, snapF);
}

void MBPlayerPosFixOrderSet(void)
{
    s8 playerNo[GW_PLAYER_MAX];
    s8 orderNo[GW_PLAYER_MAX];
    s8 fixF[GW_PLAYER_MAX];
    int i, j, k;
    int num;
    s16 masuId;
    
    memset(fixF, 0, GW_PLAYER_MAX);
    
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(fixF[i]) {
            continue;
        }
        masuId = GwPlayer[i].masuId;
        playerNo[0] = i;
        orderNo[0] = GwPlayer[i].orderNo;
        num = 1;
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(i != j && MBPlayerAliveCheck(j) && masuId == GwPlayer[j].masuId) {
                playerNo[num] = j;
                orderNo[num] = GwPlayer[j].orderNo;
                num++;
            }
        }
        if(num > 1) {
            for(j=0; j<num-1; j++) {
                for(k=j+1; k<num; k++) {
                    if(orderNo[j] > orderNo[k]) {
                        s8 temp = orderNo[j];
                        orderNo[j] = orderNo[k];
                        orderNo[k] = temp;
                    }
                }
            }
        }
        for(j=0; j<num; j++) {
            GwPlayer[playerNo[j]].orderNo = orderNo[MBPlayerMasuCornerGet(playerNo[j])];
            fixF[playerNo[j]] = TRUE;
        }
    }
}

#undef POS_FIX_MODE_MOVE
#undef POS_FIX_MODE_ROTATE

static char *eyeMatNameTbl[CHARNO_MAX][2] = {
    "eye1", "eye2",
    "eye1", "eye2",
    "mat14", "mat16",
    "eye1", "eye2",
    "Clswario_eye_l1_AUTO14", "Clswario_eye_l1_AUTO15",
    "m_donkey_eye4", "m_donkey_eye5",
    "mat65", "mat66",
    "Clswaluigi_eye_l1_AUTO1", "Clswaluigi_eye_l1_AUTO2"
};

void MBPlayerEyeMatDarkSet(int playerNo, BOOL darkF)
{
    BOOL validF;
    HU3DMODELID modelId = MBModelIdGet(MBPlayerModelGet(playerNo));
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFMATERIAL *matP = hsf->material;
    HSFMATERIAL *matCopy = playerWork[playerNo].matCopy;
    if(darkF) {
        char **name = &eyeMatNameTbl[GwPlayer[playerNo].charNo][0];
        int i, j;
        for(i=0; i<hsf->materialNum; i++, matP++, matCopy++) {
            validF = TRUE;
            for(j=0; j<matP->attrNum; j++) {
                HSFATTRIBUTE *attrP = &hsf->attribute[matP->attr[j]];
                if(strcmp(name[0], attrP->bitmap->name) == 0 || strcmp(name[1], attrP->bitmap->name) == 0) {
                    validF = FALSE;
                }
            }
            if(validF) {
                if(darkF) {
                    matP->color[0] *= 0.0f;
                    matP->color[1] *= 0.0f;
                    matP->color[2] *= 0.0f;
                } else {
                    matP->color[0] = matCopy->color[0];
                    matP->color[1] = matCopy->color[1];
                    matP->color[2] = matCopy->color[2];
                }
            }
        }
    } else {
        memcpy(hsf->material, matCopy, hsf->materialNum*sizeof(HSFMATERIAL));
    }
    DCStoreRange(hsf->material, hsf->materialNum*sizeof(HSFMATERIAL));
}

void MBPlayerMatClone(int playerNo)
{
    HU3DMODELID modelId = MBModelIdGet(MBPlayerModelGet(playerNo));
    HU3DMODEL *modelP = &Hu3DData[modelId];
    HSFDATA *hsf = modelP->hsf;
    HSFMATERIAL *matP = HuMemDirectMallocNum(HUHEAPTYPE_HEAP, hsf->materialNum*sizeof(HSFMATERIAL), HU_MEMNUM_OVL);
    memcpy(matP, hsf->material, hsf->materialNum*sizeof(HSFMATERIAL));
    playerWork[playerNo].matCopy = matP;
}

u32 MBPlayerNameMesGet(int playerNo)
{
    u32 nameTbl[CHARNO_MAX] = {
        CHARANAME_MARIO,
        CHARANAME_LUIGI,
        CHARANAME_PEACH,
        CHARANAME_YOSHI,
        CHARANAME_WARIO,
        CHARANAME_DAISY,
        CHARANAME_WALUIGI,
        CHARANAME_KINOPIO,
        CHARANAME_TERESA,
        CHARANAME_MINIKOOPA,
        CHARANAME_MINIKOOPAR,
        CHARANAME_MINIKOOPAG,
        CHARANAME_MINIKOOPAB
    };
    return nameTbl[GwPlayer[playerNo].charNo];
}

char *MBPlayerNameGet(int playerNo)
{
    char *nameTbl[CHARNO_MAX] = {
        "Mario",
        "Luigi",
        "Peach",
        "Yoshi",
        "Wario",
        "Daisy",
        "Waluigi",
        "Kinopio",
        "Teresa",
        "Mini Koopa",
        "Mini KoopaR",
        "Mini KoopaG",
        "Mini KoopaB"
    };
    return nameTbl[GwPlayer[playerNo].charNo];
}

void MBPlayerAmbSet(int playerNo, float ambR, float ambG, float ambB)
{
    MBModelAmbSet(MBPlayerModelGet(playerNo), ambR, ambG, ambB);
}

MBMODELID MBPlayerModelGet(int playerNo)
{
    return playerWork[playerNo].modelId;
}

BOOL MBPlayerAllComCheck(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!GwPlayer[i].comF) {
            return FALSE;
        }
    }
    return TRUE;
}

int MBPlayerGrpFind(int playerNo)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(i == playerNo) {
            continue;
        }
        if(MBPlayerGrpGet(playerNo) == MBPlayerGrpGet(i)) {
            break;
        }
    }
    return i;
}

int MBPlayerGrpFindOther(int playerNo)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(i == playerNo) {
            continue;
        }
        if(MBPlayerGrpGet(playerNo) != MBPlayerGrpGet(i)) {
            break;
        }
    }
    return i;
}

int MBPlayerGrpFindPlayer(int grpNo, int memberNo)
{
    int i;
    int no;
    no = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(grpNo != MBPlayerGrpGet(i)) {
            continue;
        }
        if(no == memberNo) {
            return i;
        }
        no++;
    }
    return -1;
}

float MBPlayerWalkSpeedGet(void)
{
    return 25;
}

void MBPlayerLayerSet(int playerNo, int layer)
{
    MBModelLayerSet(MBPlayerModelGet(playerNo), layer);
}

void MBPlayerCameraSet(int playerNo, u16 bit)
{
    MBModelCameraSet(MBPlayerModelGet(playerNo), bit);
}

void MBPlayerUnkSpeedSet(int playerNo, float speed)
{
    MBModelUnkSpeedSet(MBPlayerModelGet(playerNo), speed);
}

void MBPlayerStubValSet(int playerNo, BOOL value)
{
    if(value == FALSE) {
        MBModelStubValSet(MBPlayerModelGet(playerNo), FALSE);
    } else {
        MBModelStubValSet(MBPlayerModelGet(playerNo), TRUE);
    }
}

void MBPlayerPosReset(int playerNo)
{
    HuVecF pos;
    MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId, &pos);
    MBPlayerPosSetV(playerNo, &pos);
}

void MBPlayerPosRestoreAll(void)
{
    int i, j;
    int cornerNo;
    s16 masuId;
    s8 orderNo;
    HuVecF pos;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        orderNo = GwPlayer[i].orderNo;
        masuId = GwPlayer[i].masuId;
        cornerNo = 0;
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(i != j && MBPlayerAliveCheck(j) && masuId == GwPlayer[j].masuId && orderNo > GwPlayer[j].orderNo) {
                cornerNo++;
            }
        }
        MBPlayerMasuCornerSet(i, cornerNo);
        if(cornerNo == 0) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
        } else {
            MBMasuCornerRotPosGet(masuId, cornerNo-1, &pos);
        }
        MBPlayerPosSetV(i, &pos);
    }
}

void MBPlayerMtxSet(int playerNo, Mtx *matrix)
{
    MBModelMtxSet(MBPlayerModelGet(playerNo), matrix);
}

void MBPlayerPosSetV(int playerNo, HuVecF *pos)
{
    MBModelPosSetV(MBPlayerModelGet(playerNo), pos);
}

void MBPlayerPosSet(int playerNo, float posX, float posY, float posZ)
{
    MBModelPosSet(MBPlayerModelGet(playerNo), posX, posY, posZ);
}

void MBPlayerPosGet(int playerNo, HuVecF *pos)
{
    MBModelPosGet(MBPlayerModelGet(playerNo), pos);
}

void MBPlayerRotSetV(int playerNo, HuVecF *rot)
{
    MBModelRotSetV(MBPlayerModelGet(playerNo), rot);
}

void MBPlayerRotSet(int playerNo, float rotX, float rotY, float rotZ)
{
    MBModelRotSet(MBPlayerModelGet(playerNo), rotX, rotY, rotZ);
}

void MBPlayerRotGet(int playerNo, HuVecF *rot)
{
    MBModelRotGet(MBPlayerModelGet(playerNo), rot);
}

void MBPlayerRotYSet(int playerNo, float rotY)
{
    rotY = fmod(rotY, 360);
    if(rotY < 0) {
        rotY += 360;
    }
    MBModelRotYSet(MBPlayerModelGet(playerNo), rotY);
}

float MBPlayerRotYGet(int playerNo)
{
    return MBModelRotYGet(MBPlayerModelGet(playerNo));
}

void MBPlayerScaleSetV(int playerNo, HuVecF *scale)
{
    MBModelScaleSetV(MBPlayerModelGet(playerNo), scale);
}

void MBPlayerScaleSet(int playerNo, float scaleX, float scaleY, float scaleZ)
{
    MBModelScaleSet(MBPlayerModelGet(playerNo), scaleX, scaleY, scaleZ);
}

void MBPlayerScaleGet(int playerNo, HuVecF *scale)
{
    MBModelScaleGet(MBPlayerModelGet(playerNo), scale);
}

void MBPlayerMotionNoSet(int playerNo, int motNo, u32 attr)
{
    GWPLAYER *playerP = GWPlayerGet(playerNo);
    (void)(motNo == MB_MOT_DEFAULT);
    if(motNo == playerWork[playerNo].motNo) {
        return;
    }
    if(MBMotionNoSet(MBPlayerModelGet(playerNo), motNo, attr)) {
        playerWork[playerNo].motNo = motNo;
    }
    if(motNo == MB_PLAYER_MOT_FLATTEN) {
        MBModelOffsetSet(MBPlayerModelGet(playerNo), 0, 4, 0);
    } else {
        MBModelOffsetSet(MBPlayerModelGet(playerNo), 0, 0, 0);
    }
}

int MBPlayerMotionNoGet(int playerNo)
{
    return playerWork[playerNo].motNo;
}

void MBPlayerMotionNoShiftSet(int playerNo, int motNo, float start, float end, u32 attr)
{
    GWPLAYER *playerP = GWPlayerGet(playerNo);
    (void)(motNo == MB_MOT_DEFAULT);
    if(motNo == playerWork[playerNo].motNo) {
        return;
    }
    if(MBMotionShiftSet(MBPlayerModelGet(playerNo), motNo, start, end, attr)) {
        playerWork[playerNo].motNo = motNo;
    }
    if(motNo == MB_PLAYER_MOT_FLATTEN) {
        MBModelOffsetSet(MBPlayerModelGet(playerNo), 0, 4, 0);
    } else {
        MBModelOffsetSet(MBPlayerModelGet(playerNo), 0, 0, 0);
    }
}

int MBPlayerMotionCreate(int playerNo, int dataNum)
{
    return MBMotionCreate(MBPlayerModelGet(playerNo), dataNum);
}

void MBPlayerMotionKill(int playerNo, int motNo)
{
    MBMotionKill(MBPlayerModelGet(playerNo), motNo);
}

void MBPlayerMotionSpeedSet(int playerNo, float speed)
{
    MBMotionSpeedSet(MBPlayerModelGet(playerNo), speed);
}

void MBPlayerMotionTimeSet(int playerNo, float time)
{
    MBMotionTimeSet(MBPlayerModelGet(playerNo), time);
}

float MBPlayerMotionTimeGet(int playerNo)
{
    return MBMotionTimeGet(MBPlayerModelGet(playerNo));
}

float MBPlayerMotionMaxTimeGet(int playerNo)
{
    return MBMotionMaxTimeGet(MBPlayerModelGet(playerNo));
}

void MBPlayerMotionStartEndSet(int playerNo, float start, float end)
{
    MBMotionStartEndSet(MBPlayerModelGet(playerNo), start, end);
}

void MBPlayerModelAttrSet(int playerNo, u32 modelAttr)
{
    MBModelAttrSet(MBPlayerModelGet(playerNo), modelAttr);
}

void MBPlayerModelAttrReset(int playerNo, u32 modelAttr)
{
    MBModelAttrReset(MBPlayerModelGet(playerNo), modelAttr);
}

void MBPlayerMotionVoiceOnSet(int playerNo, int motNo, BOOL voiceOn)
{
    MBMotionVoiceOnSet(MBPlayerModelGet(playerNo), motNo, voiceOn);
}

BOOL MBPlayerMotionEndCheck(int playerNo)
{
    int modelId;
    BOOL endF;
    modelId = MBPlayerModelGet(playerNo);

    return MBMotionShiftIDGet(modelId) < 0 && MBMotionEndCheck(modelId);
}

BOOL MBPlayerMotionEndCheckAll(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerMotionEndCheck(i)) {
            return FALSE;
        }
    }
    return TRUE;
}

void MBPlayerMotionEndWait(int playerNo)
{
    while(!MBPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
}

void MBPlayerMotIdleSet(int playerNo)
{
    MBPlayerMotionNoShiftSet(playerNo, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
}

void MBPlayerCoinSet(int playerNo, int coinNum)
{
    if(!GWTeamFGet()) {
        GWPlayerCoinSet(playerNo, coinNum);
    } else {
        GWPlayerCoinSet(MBPlayerGrpFindPlayer(MBPlayerGrpGet(playerNo), 0), coinNum);
    }
}

int MBPlayerCoinGet(int playerNo)
{
    if(!GWTeamFGet()) {
        return GWPlayerCoinGet(playerNo);
    } else {
        return GWPlayerCoinGet(MBPlayerGrpFindPlayer(MBPlayerGrpGet(playerNo), 0));
    }
}

void MBPlayerCoinAdd(int playerNo, int coinNum)
{
    GWPLAYER *playerP;
    if(GWTeamFGet()) {
        playerNo = MBPlayerGrpFindPlayer(MBPlayerGrpGet(playerNo), 0);
    }
    playerP = GWPlayerGet(playerNo);
    if(coinNum > 0 && playerP->coinTotal < 999) {
        playerP->coinTotal += coinNum;
        if(playerP->coinTotal > 999) {
            playerP->coinTotal = 999;
        }
    }
    
    MBPlayerCoinSet(playerNo, coinNum+MBPlayerCoinGet(playerNo));
}

void MBPlayerGrpCoinSet(int grp, int coinNum)
{
    GWPlayerCoinSet(MBPlayerGrpFindPlayer(grp, 0), coinNum);
}

s16 MBPlayerGrpCoinGet(int grp)
{
    return GWPlayerCoinGet(MBPlayerGrpFindPlayer(grp, 0));
}

int MBPlayerMaxCoinGet(int grp)
{
    int maxCoin = 0;
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerCoinGet(i) >= maxCoin) {
            maxCoin = MBPlayerCoinGet(i);
        }
    }
    return maxCoin;
}

void MBPlayerStarSet(int playerNo, int starNum)
{
    if(!GWTeamFGet()) {
        GWPlayerStarSet(playerNo, starNum);
    } else {
        GWPlayerStarSet(MBPlayerGrpFindPlayer(MBPlayerGrpGet(playerNo), 0), starNum);
    }
}

int MBPlayerStarGet(int playerNo)
{
    if(!GWTeamFGet()) {
        return GWPlayerStarGet(playerNo);
    } else {
        return GWPlayerStarGet(MBPlayerGrpFindPlayer(MBPlayerGrpGet(playerNo), 0));
    }
}

void MBPlayerStarAdd(int playerNo, int starNum)
{
    MBAudFXPlay(MSM_SE_CMN_09);
    MBPlayerStarSet(playerNo, MBPlayerStarGet(playerNo)+starNum);
}

void MBPlayerGrpStarSet(int grp, int starNum)
{
    GWPlayerStarSet(MBPlayerGrpFindPlayer(grp, 0), starNum);
}

s16 MBPlayerGrpStarGet(int grp)
{
    return GWPlayerStarGet(MBPlayerGrpFindPlayer(grp, 0));
}

int MBPlayerBestPathGet(void)
{
    int i;
    int bestPlayer;
    int len;
    int minLen;
    minLen = 9999;
    bestPlayer = -1;
    for(i=0; i<GW_PLAYER_MAX; i++){
        len = MBMasuPathTypeLenGet(GwPlayer[i].masuId, MASU_TYPE_STAR);
        if(len < minLen) {
            minLen = len;
            bestPlayer = i;
        }
    }
    if(bestPlayer < 0) {
        return frandmod(4);
    } else {
        return bestPlayer;
    }
}

int MBPlayerRankGet(int playerNo)
{
    int score[GW_PLAYER_MAX];
    int i;
    int rank;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        score[i] = MBPlayerCoinGet(i)|(MBPlayerStarGet(i)*1024);
    }
    rank = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(i != playerNo && score[playerNo] < score[i]) {
            rank++;
        }
    }
    return rank;
}

s16 MBPlayerGrpRankGet(int grp)
{
    int score[2];
    int i;
    int rank;
    for(i=0; i<2; i++) {
        score[i] = MBPlayerGrpCoinGet(i)|(MBPlayerGrpStarGet(i)*2048);
    }
    rank = 0;
    for(i=0; i<2; i++) {
        if(i != grp && score[grp] < score[i]) {
            rank++;
        }
    }
    return rank;
}

void MBPlayerCapsuleUseSet(int capsuleNo)
{
    GwPlayer[GwSystem.turnPlayerNo].capsuleUse = capsuleNo;
}

int MBPlayerCapsuleUseGet(void)
{
    return GwPlayer[GwSystem.turnPlayerNo].capsuleUse;
}

int MBPlayerCapsuleMaxGet(void)
{
    return (GWTeamFGet() == FALSE) ? 3 : 5;
}

static s8 *PlayerCapsulePtrGet(int playerNo, int index)
{
    if(!GWTeamFGet()) {
        return &GwPlayer[playerNo].capsule[index];
    } else {
        int memberNo = (index < 3) ? 0 : 1;
        int grp = MBPlayerGrpGet(playerNo);
        int no = -1;
        int j;
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(grp == MBPlayerGrpGet(j)) {
                no++;
                if(no == memberNo) {
                    break;
                }
            }
        }
        if(j < GW_PLAYER_MAX) {
            return  &GwPlayer[j].capsule[index-(memberNo*3)];
        }
    }
}

int MBPlayerCapsuleAdd(int playerNo, int capsuleNo)
{
    GWPLAYER *playerP = GWPlayerGet(playerNo);
    int max = MBPlayerCapsuleMaxGet();
    int i;
    for(i=0; i<max; i++) {
        s8 *capsuleP;
        if(MBPlayerCapsuleGet(playerNo, i) != CAPSULE_NULL) {
            continue;
        }
        *PlayerCapsulePtrGet(playerNo, i) = capsuleNo;
        return i;
    }
    return -1;
}

int MBPlayerCapsuleRemove(int playerNo, int index)
{
    int capsuleNo = MBPlayerCapsuleGet(playerNo, index);
    GWPLAYER *playerP = GWPlayerGet(playerNo);
    int max;
    int i;
    if(capsuleNo == CAPSULE_NULL) {
        return capsuleNo;
    }
    
    max = MBPlayerCapsuleMaxGet();
    for(i=index; i<max-1; i++) {
        *PlayerCapsulePtrGet(playerNo, i) = *PlayerCapsulePtrGet(playerNo, i+1);
    }
    for(; i<max; i++) {
        *PlayerCapsulePtrGet(playerNo, i) = CAPSULE_NULL;
    }
    return capsuleNo;
}

int MBPlayerCapsuleFind(int playerNo, int capsuleNo)
{
    int max = MBPlayerCapsuleMaxGet();
    int i;
    for(i=0; i<max; i++){ 
        if(capsuleNo == MBPlayerCapsuleGet(playerNo, i)) {
            return i;
        }
    }
    return -1;
}

s8 MBPlayerCapsuleGet(int playerNo, int index)
{
    return *PlayerCapsulePtrGet(playerNo, index);
}

s8 MBPlayerGrpCapsuleGet(int grp, int index)
{
    return MBPlayerCapsuleGet(MBPlayerGrpFindPlayer(grp, 0), index);
}

static int PlayerCountCapsules(int playerNo)
{
    int i;
    for(i=0; i<3; i++) {
        if(GwPlayer[playerNo].capsule[i] == CAPSULE_NULL) {
            break;
        }
    }
    return i;
}

int MBPlayerCapsuleNumGet(int playerNo)
{
    int num = PlayerCountCapsules(playerNo);
    if(GWTeamFGet()) {
        int otherPlayer = MBPlayerGrpFind(playerNo);
        num += PlayerCountCapsules(otherPlayer);
        if(num > 5) {
            num = 5;
        }
    }
    return num;
}

int MBPlayerGrpCapsuleNumGet(int grp)
{
    return MBPlayerCapsuleNumGet(MBPlayerGrpFindPlayer(grp, 0));
}

BOOL MBPlayerAliveCheck(int playerNo)
{
    if(!GwPlayer[playerNo].deadF) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MBPlayerDeadCheck(int playerNo)
{
    return MBPlayerAliveCheck(playerNo) == FALSE;
}

int MBPlayerAliveGet(int *playerNo)
{
    int num = 0;
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            if(playerNo) {
                playerNo[num] = i;
            }
            num++;
        }
    }
    return num;
}

int MBPlayerAlivePlayerGet(int *playerNo)
{
    int num = 0;
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerStoryComCheck(i)) {
            continue;
        }
        if(MBPlayerAliveCheck(i)) {
            if(playerNo) {
                playerNo[num] = i;
            }
            num++;
        }
    }
    return num;
}

int MBPlayerDeadGet(int *playerNo)
{
    int num = 0;
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            if(playerNo) {
                playerNo[num] = i;
            }
            num++;
        }
    }
    return num;
}

int MBPlayerDeadComGet(int *playerNo)
{
    int num = 0;
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerStoryComCheck(i)) {
            continue;
        }
        if(!MBPlayerAliveCheck(i)) {
            if(playerNo) {
                playerNo[num] = i;
            }
            num++;
        }
    }
    return num;
}

BOOL MBPlayerMoveCheck(int playerNo)
{
    if(playerWork[playerNo].moveF) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MBPlayerStoryLoseCheck(int playerNo)
{
    if(GWPartyFGet() == FALSE && MBPlayerAliveCheck(playerNo) != FALSE && MBPlayerCoinGet(playerNo) == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void MBPlayerStoryAliveReset(int playerNo)
{
    GwPlayer[playerNo].deadF = FALSE;
}

static void PlayerLoseExec(int playerNo)
{
    HuVecF rot = { -33, 0, 0 };
    BOOL tempF = FALSE;
    int masuIdFix[GW_PLAYER_MAX] = { MASU_NULL, MASU_NULL, MASU_NULL, MASU_NULL };
    int i;
    int motTbl[10];
    BOOL dispLightF[GW_PLAYER_MAX];
    HuVecF pos;
    for(i=0; i<10; i++) {
        motTbl[i] = MB_MOT_NONE;
    }
    if(!blackoutF) {
        WipeColorSet(255, 255, 255);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        }
        WipeWait();
    }
    
    MBPlayerPosFixPlayer(playerNo, GwPlayer[playerNo].masuId, TRUE);
    MBPlayerPosReset(playerNo);
    MBCameraSkipSet(FALSE);
    MBCameraPlayerViewSet(playerNo, &rot, NULL, 1200, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        dispLightF[i] = GwPlayer[i].dispLightF;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(i != playerNo && MBPlayerAliveCheck(i)) {
            MBPlayerDispSet(i, FALSE);
            GwPlayer[i].dispLightF = FALSE;
            
        }
    }
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || MBTutorialExitFlagGet() == FALSE) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    }
    WipeWait();
    if(!MBPlayerStoryComCheck(playerNo)) {
        motTbl[0] = MBPlayerMotionCreate(playerNo, CHAR_HSF_c000m1_322);
        motTbl[1] = MBPlayerMotionCreate(playerNo, CHAR_HSF_c000m1_485);
        MBPlayerWinLoseVoicePlay(playerNo, motTbl[0], CHARVOICEID(1));
        MBPlayerMotionNoShiftSet(playerNo, motTbl[0], 0, 8, HU3D_MOTATTR_LOOP);
        HuPrcSleep(120);
        MBPlayerMotionNoShiftSet(playerNo, motTbl[1], 0, 8, HU3D_MOTATTR_NONE);
        while(!MBPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBStatusDispSet(playerNo, FALSE);
        while(!MBStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        HuPrcSleep(60);
        _SetFlag(FLAG_BOARD_STORYEND);
        MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
        WipeColorSet(0, 0, 0);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        WipeWait();
        while(MBMusCheck(MB_MUS_CHAN_BG)) {
            HuPrcVSleep();
        }
        GwPlayer[playerNo].deadF = TRUE;
    } else {
        int aliveNum;
        motTbl[0] = MBPlayerMotionCreate(playerNo, DATANUM(DATA_bdemo, 35));
        
        MBPlayerMotionNoShiftSet(playerNo, motTbl[0], 0, 8, HU3D_MOTATTR_NONE);
        MBPlayerWinLoseVoicePlay(playerNo, motTbl[0], CHARVOICEID(10));
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESSNUM(MESS_BOARD_STORY, 0), HUWIN_SPEAKER_NONE);
        MBTopWinPause();
        while(!MBPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        MBStatusDispSet(playerNo, FALSE);
        while(!MBStatusOffCheckAll()) {
            HuPrcVSleep();
        }
        MBTopWinKill();
        HuPrcSleep(60);
        aliveNum = MBPlayerAlivePlayerGet(NULL);
        if(aliveNum > 1) {
            MBTelopComNumCreate();
        }
        WipeColorSet(0, 0, 0);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        if(aliveNum == 1) {
            _SetFlag(FLAG_BOARD_STORYEND);
            MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_BG)) {
                HuPrcVSleep();
            }
        }
        WipeWait();
        GwPlayer[playerNo].deadF = TRUE;
        HuDataDirClose(DATA_bdemo);
        MBModelDispSet(MBPlayerModelGet(playerNo), FALSE);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayer[i].dispLightF = dispLightF[i];
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerDispSet(i, TRUE);
    }
    for(i=0; motTbl[i] >= 0; i++) {
        MBPlayerMotionKill(playerNo, motTbl[i]);
    }
    MBPlayerPosFix(masuIdFix, TRUE);
}

void MBPlayerStoryLoseExec(void)
{
    int i;
    if(MBPlayerStoryLoseCheck(MBPlayerStoryPlayerGet())) {
        PlayerLoseExec(MBPlayerStoryPlayerGet());
        blackoutF = TRUE;
        if(_CheckFlag(FLAG_BOARD_STORYEND)) {
            return;
        }
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerStoryComCheck(i)) {
            continue;
        }
        if(MBPlayerAliveCheck(i) && MBPlayerStoryLoseCheck(i)) {
            PlayerLoseExec(i);
            blackoutF = TRUE;
            if(_CheckFlag(FLAG_BOARD_STORYEND)) {
                return;
            }
        }
    }
}

BOOL MBPlayerKettouCheck(int playerNo, s16 masuId)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(playerNo != i && masuId == GwPlayer[i].masuId) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL MBPlayerKettouStoryCheck(int playerNo)
{
    int storyPlayer = MBPlayerStoryPlayerGet();
    int i;
    if(playerNo == storyPlayer) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(i == storyPlayer) {
                continue;
            }
            if(MBPlayerAliveCheck(i) && GwPlayer[storyPlayer].masuId == GwPlayer[i].masuId) {
                return TRUE;
            }
        }
    } else {
        if(!MBPlayerAliveCheck(storyPlayer)) {
            return FALSE;
        }
        if(GwPlayer[playerNo].masuId == GwPlayer[storyPlayer].masuId) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL MBPlayerStoryPlayerCheck(int playerNo)
{
    return playerNo == 0;
}

BOOL MBPlayerStoryComCheck(int playerNo)
{
    return playerNo != 0;
}

int MBPlayerStoryPlayerGet(void)
{
    return 0;
}

void MBPlayerWinLoseVoicePlay(int playerNo, int motNo, int seId)
{
    MBMODEL *modelP = MBModelGet(MBPlayerModelGet(playerNo));
    u8 charNo = GwPlayer[playerNo].charNo;
    CharWinLoseVoicePlay(charNo, modelP->motId[motNo], seId);
}

int MBPlayerVoicePanPlay(int playerNo, s16 seId)
{
    return MBModelSePlay(MBPlayerModelGet(playerNo), seId);
}

int MBPlayerVoicePlay(int playerNo, s16 seId)
{
    HuVecF pos;
    u8 pan;
    MBPlayerPosGet(playerNo, &pos);
    pan = MBAudFXPosPanGet(&pos);
    return HuAudCharFXPlayVolPan(GwPlayer[playerNo].charNo, seId, MSM_VOL_MAX, pan);
}

void MBPlayerDispSet(int playerNo, BOOL dispF)
{
    if(MBPlayerAliveCheck(playerNo)) {
        MBModelDispSet(MBPlayerModelGet(playerNo), dispF);
    }
}

GXColor MBPlayerColorGet(int playerNo)
{
    GXColor color[CHARNO_MAX] = {
        { 227, 67, 67, 255 },
        { 68, 67, 227, 255 },
        { 241, 158, 220, 255 },
        { 67, 228, 68, 255 },
        { 138, 60, 180, 255 },
        { 227, 228, 68, 255 },
        { 192, 192, 192, 255 },
        { 227, 227, 227, 255 },
        { 40, 227, 227, 255 },
        { 227, 139, 40, 255 },
        { 180, 40, 40, 255 },
        { 40, 180, 40, 255 },
        { 40, 40, 180, 255 }
    };
    return color[GwPlayer[playerNo].charNo];
}

void MBPlayerBlackoutSet(BOOL value)
{
    blackoutF = value;
}

BOOL MBPlayerBlackoutGet(void)
{
    return blackoutF;
}

static void SetMasuView(int playerNo)
{
    int masuIdFix[GW_PLAYER_MAX] = { MASU_NULL, MASU_NULL, MASU_NULL, MASU_NULL };
    HuVecF pos;
    masuIdFix[playerNo] = GwPlayer[playerNo].masuId;
    MBPlayerPosFix(masuIdFix, TRUE);
    MBPlayerPosReset(playerNo);
    MBPlayerPosFixOrderSet();
    MBCameraSkipSet(FALSE);
    MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
    MBCameraMotionWait();
}

void MBPlayerMasuCornerSet(int playerNo, s8 cornerNo)
{
    playerWork[playerNo].masuCorner = cornerNo;
    
}

s8 MBPlayerMasuCornerGet(int playerNo)
{
    return playerWork[playerNo].masuCorner;
}