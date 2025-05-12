#include "game/board/opening.h"
#include "game/board/story.h"
#include "game/board/camera.h"
#include "game/board/player.h"
#include "game/board/coin.h"
#include "game/board/tutorial.h"
#include "game/board/audio.h"
#include "game/board/telop.h"
#include "game/board/masu.h"
#include "game/board/guide.h"
#include "game/board/window.h"
#include "game/board/sai.h"
#include "game/board/status.h"
#include "game/board/capsule.h"
#include "game/board/com.h"

#include "game/wipe.h"

#include "messnum/board_opening.h"
#include "messnum/boardope.h"
#include "messnum/board_tutorial.h"
#include "messnum/board_mgcircuit.h"

#include "datanum/effect.h"
#include "datanum/w20.h"

#define FUSENOBJ_MAX 20

static OMOBJ *fusenObj[FUSENOBJ_MAX];
static int saiDelay[GW_PLAYER_MAX];
static HuVecF openingViewTarget;
static HuVecF openingViewRot;

static float openingViewZoom;
static OMOBJ *hikosenObj;
static HUPROCESS *openingProc;

static u32 openingMessTbl[7] = {
    MESS_BOARD_OPENING_BOARD1,
    MESS_BOARD_OPENING_BOARD2,
    MESS_BOARD_OPENING_BOARD3,
    MESS_BOARD_OPENING_BOARD4,
    MESS_BOARD_OPENING_BOARD5,
    MESS_BOARD_OPENING_BOARD6,
    MESS_BOARD_OPENING_BOARD7
};

void MBOpeningProcExec(void)
{
    if(GWPartyFGet() == FALSE) {
        MBStoryOpeningExec();
    } else {
        MBOpeningExec();
    }
}

static void OpeningExec(void);
static void OpeningProcDestroy(void);

static void MgCircuitOpeningExec(void);
static void TutorialOpeningExec(void);

void MBOpeningExec(void)
{
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
            openingProc = MBPrcCreate(OpeningExec, 8204, 16384);
        } else {
            openingProc = MBPrcCreate(MgCircuitOpeningExec, 8204, 16384);
        }
    } else {
        openingProc = MBPrcCreate(TutorialOpeningExec, 8204, 16384);
    }
    MBPrcDestructorSet(openingProc, OpeningProcDestroy);
    while(openingProc != NULL) {
        HuPrcVSleep();
    }
}

static void PlayerInProc(HuVecF *start, HuVecF *end);
static u16 SaiPadBtnHook(int playerNo);
static void PlayerSort(int *order);
static void CoinEffHook(MBCOINEFF *coinEff);

static void OpeningExec(void)
{
    int orderTbl[10];
    int coinEff[GW_PLAYER_MAX];
    HuVecF playerPos;
    HuVecF cameraOfs = { 0, 250, 0 };
    HuVecF inPos;
    HuVecF masuPos;
    HuVecF guidePos;
    HuVecF playerRotDir;
    HuVecF cameraRot = { -10, 0, 0 };
    int playerOrder[GW_PLAYER_MAX];
    float cameraZoom = 20000;
    
    static const float zoomTbl[7] = {
        20000,
        20000,
        20000,
        16000,
        15000,
        20000,
        20000
    };
    int i;
    int num;
    MBMODELID inMdlId;
    int order1;
    int order2;
    GWPLAYER *playerP;
    MBCOINEFF *coinEffP;
    MBMODELID guideMdlId;
    s16 playerRotAngle;
    MBPLAYERWORK *playerWorkP;
    int statId;
    float scale;
    
    MBCameraNearFarSet(10, 30000);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerDispSet2(i, FALSE);
    }
    MBCameraZoomSet(openingViewZoom);
    MBCameraRotSetV(&openingViewRot);
    MBCameraPosTargetSetV(&openingViewTarget);
    HuDataDirRead(DATA_guide);
    statId = MBDataDirReadAsync(DATA_capsule);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    MBDataAsyncWait(statId);
    MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_BOARD_OP, 127, 0);
    MBTelopLogoCreate(MBBoardNoGet(), FALSE);
    HuPrcSleep(72);
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &masuPos);
    MBCameraPosViewSet(&masuPos, &cameraRot, &cameraOfs, 1800, -1, 180);
    MBCameraMotionWait();
    inPos.x = masuPos.x;
    inPos.y = 200+masuPos.y;
    inPos.z = masuPos.z-200;
    inMdlId = MBModelCreate(BOARD_HSF_mapCapsule, NULL, FALSE);
    MBModelPosSet(inMdlId, inPos.x, inPos.y, inPos.z);
    MBMotionNoSet(inMdlId, MB_MOT_DEFAULT, HU3D_MOTATTR_LOOP);
    for(i=0; i<60u; i++) {
        scale = i/60.0f;
        MBModelScaleSet(inMdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    guidePos.x = inPos.x;
    guidePos.y = inPos.y+250;
    guidePos.z = inPos.z-100;
    guideMdlId = MBGuideCreateIn(&guidePos, FALSE, TRUE, FALSE);
    playerPos.x = inPos.x;
    playerPos.y = masuPos.y;
    playerPos.z = masuPos.z;
    PlayerInProc(&inPos, &playerPos);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerPosGet(i, &playerPos);
        VECSubtract(&guidePos, &playerPos, &playerRotDir);
        playerRotAngle = 90-HuAtan(playerRotDir.z, playerRotDir.x);
        MBPlayerRotateStart(i, playerRotAngle, 30);
    }
    for(i=0; i<30u; i++) {
        scale = 1-(i/30.0f);
        MBModelScaleSet(inMdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    MBModelKill(inMdlId);
    while(!MBPlayerRotateCheckAll()) {
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_GUIDE_49);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, openingMessTbl[MBBoardNoGet()], MBGuideSpeakerNoGet());
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_OPENING_TURNORDER, MBGuideSpeakerNoGet());
    MBTopWinWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerRotateStart(i, 0, 30);
    }
    while(!MBPlayerRotateCheckAll()) {
        HuPrcVSleep();
    }
    for(i=0; i<10; i++) {
        orderTbl[i] = i;
    }
    for(i=0; i<100; i++) {
        order1 = frandmod(10);
        order2 = frandmod(10);
        num = orderTbl[order1];
        orderTbl[order1] = orderTbl[order2];
        orderTbl[order2] = num;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBSaiExec(i, SAITYPE_EVENT, NULL, orderTbl[i], FALSE, FALSE, NULL, SAI_COLOR_GREEN);
        if(GwPlayer[i].comF) {
            saiDelay[i] = frandmod(30)+30;
            MBSaiPadBtnHookSet(i, SaiPadBtnHook);
        }
    }
    MBWinCreateHelp(MESS_BOARDOPE_PAD_SAI);
    MBTopWinPosSet(228, 416);
    while(!MBSaiKillCheckAll(i)) {
        HuPrcVSleep();
    }
    MBTopWinKill();
    memcpy(playerOrder, orderTbl, GW_PLAYER_MAX*sizeof(int));
    PlayerSort(playerOrder);
    MBStatusReinit();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_OPENING_TURNORDER1+i, HUWIN_SPEAKER_NONE);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(i), 0);
        if(GwPlayer[i].comF || HuPadStatGet(GwPlayer[i].padNo) != PAD_ERR_NONE) {
            MBTopWinPlayerDisable(-1);
        } else {
            MBTopWinPlayerDisable(i);
        }
        MBTopWinWait();
        playerP = GWPlayerGet(i);
        playerWorkP = MBPlayerWorkGet(i);
        MBSaiNumShrinkSet(playerP->playerNo);
        playerP->playerNo = playerWorkP->playerNo = i;
        if(!GWTeamFGet()) {
            MBStatusDispSet(i, TRUE);
        }
        MBPlayerWinLoseVoicePlay(i, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_COINWIN, 0, 8, HU3D_MOTATTR_NONE);
        omVibrate(i, 12, 12, 0);
        HuPrcSleep(8);
        while(!MBPlayerMotionEndCheck(i)) {
            HuPrcVSleep();
        }
    }
    HuPrcSleep(30);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerMotIdleSet(i);
    }
    if(GWTeamFGet()) {
        MBStatusDispSetAll(TRUE);
    }
    HuPrcSleep(30);
    MBAudFXPlay(MSM_SE_GUIDE_50);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_OPENING_10COINS, MBGuideSpeakerNoGet());
    MBTopWinWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        coinEff[i] = MBCoinEffCreate(10, CoinEffHook);
        coinEffP = MBCoinEffGet(coinEff[i]);
        coinEffP->work[1] = i;
    }
    while(1) {
        for(num=0, i=0; i<GW_PLAYER_MAX; i++) {
            if(!MBCoinEffCheck(coinEff[i])) {
                num++;
            }
        }
        HuPrcVSleep();
        if(num == 0) {
            break;
        }
    }
    MBDonkeyOpeningExec();
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_OPENING_EVENT, MBGuideSpeakerNoGet());
    MBTopWinWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerWinLoseVoicePlay(i, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_WIN, 0, 8, HU3D_MOTATTR_NONE);
    }
    while(!MBPlayerMotionEndCheckAll()) {
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
    while(MBMusCheck(MB_MUS_CHAN_BG) || WipeCheckEnd()) {
        HuPrcVSleep();
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerMotionNoSet(i, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
    }
    MBCameraNearFarSet(100, 20000);
    MBGuideEnd(guideMdlId);
    MBStarMapViewExec(-1, MBStarNoRandGet());
    HuPrcEnd();
}

static u16 SaiPadBtnHook(int playerNo)
{
    if(saiDelay[playerNo]) {
        if(--saiDelay[playerNo] == 0) {
            return PAD_BUTTON_A;
        }
    }
    return 0;
}

static void OpeningProcDestroy(void)
{
    openingProc = NULL;
}

void MBOpeningViewSet(HuVecF *rot, HuVecF *pos, float zoom)
{
    if(rot) {
        openingViewRot = *rot;
    }
    if(pos) {
        openingViewTarget = *pos;
    }
    if(zoom >= 0) {
        openingViewZoom = zoom;
    }
}

static void CoinEffHook(MBCOINEFF *coinEff)
{
    MBCOINEFFDATA *coinEffData;
    int i;
    HuVecF pos;
    if(coinEff->work[0] == FALSE) {
        MBPlayerPosGet(coinEff->work[1], &pos);
        for(coinEffData=coinEff->data, i=0; i<coinEff->count; i++, coinEffData++) {
            coinEffData->pos.x = (pos.x+((0.2f*frandf())*100))-10;
            coinEffData->pos.y = 800+pos.y;
            coinEffData->pos.z = pos.z;
            pos.y += 75;
            coinEffData->vel.y = -2;
            coinEffData->rot.y = frandmod(360);
        }
        coinEff->work[0] = TRUE;
    }
    MBPlayerPosGet(coinEff->work[1], &pos);
    for(coinEffData=coinEff->data, i=0; i<coinEff->count; i++, coinEffData++) {
        if(coinEffData->dispF && !coinEffData->timeEndF) {
            VECAdd(&coinEffData->pos, &coinEffData->vel, &coinEffData->pos);
            coinEffData->vel.y -= 0.2f;
            coinEffData->rot.y += 5;
            if(coinEffData->pos.y < 100+pos.y) {
                coinEffData->hitF = TRUE;
                if(GWTeamFGet()) {
                    MBPlayerCoinAdd(coinEff->work[1], 1);
                } else {
                    MBPlayerCoinAdd(coinEff->work[1], 1);
                }
            }
        }
    }
}

typedef struct PlayerInData_s {
    int delay;
    int time;
    HuVecF endPos;
    HU3DPARMANID parManId;
} PLAYERINDATA;

static HU3DPARMANID PlayerInEffCreate(ANIMDATA *animP, s16 layer, s16 cameraBit);
static void PlayerInEffUpdate(HU3DPARMANID parManId, HuVecF *pos);
static void PlayerInEffKill(HU3DPARMANID parManId);

static void PlayerInProc(HuVecF *start, HuVecF *end)
{
    PLAYERINDATA inData[GW_PLAYER_MAX];
    HuVecF pos;
    
    PLAYERINDATA *inDataP;
    int i;
    int playerNum;
    ANIMDATA *animP;
    animP = HuSprAnimDataRead(EFFECT_ANM_glow);
    HuDataDirClose(DATA_effect);
    for(inDataP=&inData[0], i=0; i<GW_PLAYER_MAX; i++, inDataP++) {
        inDataP->delay = i*30;
        inDataP->time = 0;
        inDataP->endPos.x = (end->x-250)+(166.66667f*i);
        inDataP->endPos.y = end->y;
        inDataP->endPos.z = end->z;
        inDataP->parManId = PlayerInEffCreate(animP, 2, HU3D_CAM0);
        MBPlayerPosSetV(i, start);
    }
    while(1) {
        playerNum = 0;
        for(inDataP=&inData[0], i=0; i<GW_PLAYER_MAX; i++, inDataP++) {
            if(inDataP->delay == 0) {
                if(inDataP->time == 0) {
                    MBPlayerDispSet2(i, TRUE);
                    MBPlayerMotionNoSet(i, MB_PLAYER_MOT_JUMP, HU3D_MOTATTR_NONE);
                    MBPlayerMotionTimeSet(i, MBPlayerMotionMaxTimeGet(i));
                    MBPlayerVoicePlay(i, CHARSEID(16));
                }
                if(inDataP->time++ < 30u) {
                    float weight = inDataP->time/30.0f;
                    pos.x = start->x+(weight*(inDataP->endPos.x-start->x));
                    pos.y = (start->y+(weight*(inDataP->endPos.y-start->y)))+(100*(2*HuSin(weight*180.0f)));
                    pos.z = start->z+(weight*(inDataP->endPos.z-start->z));
                    MBPlayerPosSetV(i, &pos);
                    PlayerInEffUpdate(inDataP->parManId, &pos);
                    playerNum++;
                } else {
                    MBPlayerMotIdleSet(i);
                }
            } else {
                inDataP->delay--;
                playerNum++;
            }
        }
        HuPrcVSleep();
        if(playerNum == 0) {
            break;
        }
    }
    HuPrcSleep(30);
    for(inDataP=&inData[0], i=0; i<GW_PLAYER_MAX; i++, inDataP++) {
        PlayerInEffKill(inDataP->parManId);
    }
}

static void PlayerSort(int *order)
{
    int i, j;
    int playerGrp;
    int temp;
    
    GWPLAYER tempPlayer;
    MBPLAYERWORK tempWork;
    GWPLAYERCONF tempPlayerConf;
    int teamCoin[GW_PLAYER_MAX/2];
    int teamStar[GW_PLAYER_MAX/2];
    
    for(i=0; i<GW_PLAYER_MAX-1; i++) {
        for(j=i+1; j<GW_PLAYER_MAX; j++) {
            if(order[i] < order[j]) {
                temp = order[i];
                order[i] = order[j];
                order[j] = temp;
                tempPlayerConf = GwPlayerConf[i];
                GwPlayerConf[i] = GwPlayerConf[j];
                GwPlayerConf[j] = tempPlayerConf;
                tempPlayer = GwPlayer[i];
                GwPlayer[i] = GwPlayer[j];
                GwPlayer[j] = tempPlayer;
                memcpy(&tempWork, MBPlayerWorkGet(i), sizeof(MBPLAYERWORK));
                memcpy(MBPlayerWorkGet(i), MBPlayerWorkGet(j), sizeof(MBPLAYERWORK));
                memcpy(MBPlayerWorkGet(j), &tempWork, sizeof(MBPLAYERWORK));
            }
        }
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayer[i].padNo = GwPlayerConf[i].padNo;
        GwPlayerConf[i].padNo = GwPlayerConf[i].padNo;
    }
    if(GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX/2; i++) {
            teamCoin[i] = 0;
            teamStar[i] = 0;
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            playerGrp = MBPlayerGrpGet(i);
            teamCoin[playerGrp] += GwPlayer[i].coin;
            teamStar[playerGrp] += GwPlayer[i].star;
            GwPlayer[i].coin = 0;
            GwPlayer[i].star = 0;
        }
        for(i=0; i<GW_PLAYER_MAX/2; i++) {
            MBPlayerGrpCoinSet(i, teamCoin[i]);
            MBPlayerGrpStarSet(i, teamStar[i]);
        }
    }
}

static HU3DPARMANID PlayerInEffCreate(ANIMDATA *animP, s16 layer, s16 cameraBit)
{
    static HU3DPARMANPARAM effParam = {
        50, 0,
        2.0f,
        30.0f,
        180.0f,
        { 0, -0.2f, 0 },
        1,
        0.8f,
        30,
        0.95f,
        3,
        {
            { 128, 192, 255, 255 },
            { 255, 255, 64, 255 },
            { 255, 96, 128, 255 },
        },
        {
            { 128, 192, 255, 0 },
            { 255, 64, 32, 0 },
            { 128, 16, 16, 0 },
        },
    };
    HU3DPARMANID parManId = Hu3DParManCreate(animP, 100, &effParam);
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_SCALEJITTER|HU3D_PARMAN_ATTR_RANDSPEED70|HU3D_PARMAN_ATTR_RANDSCALE70|HU3D_PARMAN_ATTR_TIMEUP);
    
    Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(parManId), HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DParManRotSet(parManId, 90, 0, 0);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(parManId), layer);
    Hu3DModelCameraSet(Hu3DParManModelIDGet(parManId), cameraBit);
    return parManId;
}

static void PlayerInEffUpdate(HU3DPARMANID parManId, HuVecF *pos)
{
    Hu3DParManPosSet(parManId, pos->x, pos->y, pos->z);
    Hu3DParManTimeLimitSet(parManId, 2);
    Hu3DParManAttrReset(parManId, HU3D_PARMAN_ATTR_TIMEUP);
}

static void PlayerInEffKill(HU3DPARMANID parManId)
{
    Hu3DParManKill(parManId);
}

static void TutorialOpeningExec(void)
{
    static const HuVecF cameraOfs = { 0, 120.00001f, -100.0f };
    static const HuVecF cameraRot = { -25, 0, 0 };
    static const int saiNumTbl[GW_PLAYER_MAX] = {
        4,
        9,
        6,
        1
    };
    int i;
    MBMODELID inMdlId;
    int starNo;
    GWPLAYER *playerP;
    MBPLAYERWORK *playerWorkP;
    int statId;
    
    HuVecF masuPos;
    HuVecF inPos;
    HuVecF guidePos;
    int playerOrder[GW_PLAYER_MAX];
    MBMODELID guideMdlId;
    
    float scale;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayer[i].comF = TRUE;
        GwPlayerConf[i].type = GW_TYPE_COM;
        MBPlayerDispSet2(i, FALSE);
        
    }
    starNo = MBStarNoRandGet();
    if(starNo >= 0) {
        MBStarNoSet(starNo);
    }
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &masuPos);
    masuPos.z += 100;
    MBCameraPosViewSet(&masuPos, (HuVecF *)&cameraRot, (HuVecF *)&cameraOfs, 1800, -1, 1);
    MBCameraMotionWait();
    HuDataDirRead(DATA_guide);
    statId = MBDataDirReadAsync(DATA_capsule);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    MBDataAsyncWait(statId);
    MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_W10, 127, 0);
    inPos.x = masuPos.x;
    inPos.y = 200+masuPos.y;
    inPos.z = masuPos.z-200;
    inMdlId = MBModelCreate(BOARD_HSF_mapCapsule, NULL, FALSE);
    MBModelPosSet(inMdlId, inPos.x, inPos.y, inPos.z);
    MBMotionNoSet(inMdlId, MB_MOT_DEFAULT, HU3D_MOTATTR_LOOP);
    for(i=0; i<60u; i++) {
        scale = i/60.0f;
        MBModelScaleSet(inMdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    guidePos.x = 68;
    guidePos.y = 308;
    guidePos.z = 1000;
    guideMdlId = MBGuideCreateIn(&guidePos, TRUE, TRUE, FALSE);
    guidePos.x = inPos.x;
    guidePos.y = masuPos.y;
    guidePos.z = masuPos.z;
    PlayerInProc(&inPos, &guidePos);
    for(i=0; i<30u; i++) {
        scale = 1-(i/30.0f);
        MBModelScaleSet(inMdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    MBModelKill(inMdlId);
    while(!MBPlayerRotateCheckAll()) {
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_GUIDE_50);
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_START, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerRotateStart(i, 0, 30);
    }
    while(!MBPlayerRotateCheckAll()) {
        HuPrcVSleep();
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBSaiExec(i, SAITYPE_EVENT, NULL, saiNumTbl[i]-1, FALSE, FALSE, NULL, SAI_COLOR_GREEN);
        if(GwPlayer[i].comF) {
            saiDelay[i] = 0;
            MBSaiPadBtnHookSet(i, SaiPadBtnHook);
        }
    }
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_OPENING1, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        saiDelay[i] = frandmod(30)+30;
    }
    while(!MBSaiKillCheckAll(i)) {
        HuPrcVSleep();
    }
    memcpy(playerOrder, saiNumTbl, GW_PLAYER_MAX*sizeof(int));
    PlayerSort(playerOrder);
    MBStatusReinit();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_OPENING_TURNORDER1+i, HUWIN_SPEAKER_NONE);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(i), 0);
        MBTopWinWait();
        playerP = GWPlayerGet(i);
        playerWorkP = MBPlayerWorkGet(i);
        MBSaiNumShrinkSet(playerP->playerNo);
        playerP->playerNo = playerWorkP->playerNo = i;
        MBStatusDispSet(i, TRUE);
        MBPlayerWinLoseVoicePlay(i, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_COINWIN, 0, 8, HU3D_MOTATTR_NONE);
        HuPrcSleep(8);
        while(!MBPlayerMotionEndCheck(i)) {
            HuPrcVSleep();
        }
    }
    HuPrcSleep(30);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerMotIdleSet(i);
    }
    HuPrcSleep(30);
    MBWinCreate(MBWIN_TYPE_TUTORIAL, MESS_BOARD_TUTORIAL_OPENING2, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    WipeColorSet(255, 255, 255);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    }
    WipeWait();
    HuPrcEnd();
}

static void CreateFusenObj(HuVecF *pos);
static void CreateHikosenObj(HuVecF *pos);
static void KillFusenObj(void);

static void MgCircuitOpeningExec(void)
{
    static const HuVecF cameraRot = { -10, 0, 0 };
    static const HuVecF cameraRotLeft = { -10, -90, 0 };
    static const HuVecF guideRot = { 0, -90, 0 };
    static const HuVecF guidePos = { 0, 220, 200 };
    static const int capsuleTbl[3] = {
        CAPSULE_KINOKO,
        CAPSULE_KINOKO,
        CAPSULE_SKINOKO
    };
    
    char playerNumMes[256];
    
    HuVecF masuPos;
    HuVecF pos;
    MBMODELID capsuleMdlId[GW_PLAYER_MAX];
    
    int i;
    int j;
    int capsuleNo;
    MBMODELID guideMdlId;
    int statId;
    
    float scale, time;
    
    MBCameraZoomSet(3000);
    MBCameraRotSet(0, 0, 0);
    MBCameraPosTargetSet(0, 3100, -858);
    
    MBMasuFlagPosGet(MASU_LAYER_DEFAULT, MASU_FLAG_START, &masuPos);
    masuPos.z += 100;
    
    HuDataDirRead(DATA_guide);
    statId = MBDataDirReadAsync(DATA_capsule);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    MBDataAsyncWait(statId);
    MBMusPlay(MB_MUS_CHAN_BG, MSM_STREAM_BOARD_OP, 127, 0);
    pos.x = 0;
    pos.y = 2000;
    pos.z = -900;
    CreateFusenObj(&pos);
    pos.x = 0;
    pos.y = 3100;
    pos.z = -12000;
    CreateHikosenObj(&pos);
    HuPrcSleep(180);
    MBCameraSkipSet(TRUE);
    MBCameraPosViewSet(NULL, NULL, NULL, 1000, -1, 150);
    MBCameraMotionWait();
    HuPrcSleep(150);
    MBCameraPosViewSet(&masuPos, (HuVecF *)&cameraRot, NULL, 1800, -1, 180);
    HuPrcSleep(120);
    WipeColorSet(255, 255, 255);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    }
    WipeWait();
    KillFusenObj();
    MBCameraPosViewSet((HuVecF *)&guidePos, (HuVecF *)&cameraRotLeft, NULL, 2350, -1, 1);
    MBCameraMotionWait();
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    }
    WipeWait();
    pos.x = guidePos.x;
    pos.y = 400;
    pos.z = guidePos.z;
    guideMdlId = MBGuideCreate(MB_GUIDE_NIRL, &pos, (HuVecF *)&guideRot, MB_GUIDE_ATTR_EFFOUT|MB_GUIDE_ATTR_EFFIN);
    MBAudFXPlay(MSM_SE_GUIDE_56);
    MBWinCreateChoice(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_RULECHOICE, HUWIN_SPEAKER_NIRL, 1);
    MBTopWinAttrReset(HUWIN_ATTR_NOCANCEL);
    if(MBPlayerAllComCheck()) {
        MBComChoiceSetRight();
    }
    MBTopWinWait();
    if(MBWinLastChoiceGet() == 0) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_RULE1, HUWIN_SPEAKER_NIRL);
        MBTopWinWait();
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_RULE2, HUWIN_SPEAKER_NIRL);
        MBTopWinWait();
    }
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_PLAYER, HUWIN_SPEAKER_NIRL);
    MBTopWinWait();
    MBGuideEnd(guideMdlId);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBCameraPlayerViewSet(i, NULL, NULL, 1000, -1, 60);
        MBCameraMotionWait();
        MBStatusDispSet(i, TRUE);
        while(!MBStatusMoveCheck(i)) {
            HuPrcVSleep();
        }
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_PLAYERNUM, HUWIN_SPEAKER_NIRL);
        sprintf(playerNumMes, "%d", i+1);
        MBTopWinInsertMesSet(MESSNUM_PTR(playerNumMes), 0);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(i), 1);
        if(GwPlayer[i].comF || HuPadStatGet(GwPlayer[i].padNo) != PAD_ERR_NONE) {
            MBTopWinPlayerDisable(-1);
        } else {
            MBTopWinPlayerDisable(i);
        }
        MBTopWinWait();
        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_JUMP, 0, 8, HU3D_MOTATTR_NONE);
        MBPlayerVoicePlay(i, CHARVOICEID(2));
        omVibrate(i, 12, 12, 0);
        MBPlayerMotionEndWait(i);
    }
    MBCameraFocusSet(-1);
    MBCameraPosViewSet((HuVecF *)&guidePos, NULL, NULL, 2350, -1, 60);
    MBCameraMotionWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    }
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_CAPSULEBEGIN, HUWIN_SPEAKER_NIRL);
    MBTopWinWait();
    for(capsuleNo=0; capsuleNo<3; capsuleNo++) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            capsuleMdlId[i] = MBModelCreate(MBCapsuleMdlGet(capsuleTbl[capsuleNo]), NULL, 0);
            MBPlayerPosGet(i, &pos);
            pos.y += 300;
            MBModelPosSetV(capsuleMdlId[i], &pos);
        }
        MBAudFXPlay(MSM_SE_BOARD_110);
        for(i=0; i<30u; i++) {
            time = i/30.0f;
            scale = HuSin(time*90);
            for(j=0; j<GW_PLAYER_MAX; j++) {
                MBModelScaleSet(capsuleMdlId[j], scale, scale, scale);
            }
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        for(i=0; i<30u; i++) {
            float revTime;
            time = i/30.0f;
            scale = HuSin(time*90);
            revTime = 1-time;
            for(j=0; j<GW_PLAYER_MAX; j++) {
                MBPlayerPosGet(j, &pos);
                MBModelPosSet(capsuleMdlId[j], pos.x, pos.y+(100*(3*(1-scale))), pos.z);
                MBModelScaleSet(capsuleMdlId[j], revTime, revTime, revTime);
            }
            HuPrcVSleep();
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            MBPlayerCapsuleAdd(i, capsuleTbl[capsuleNo]);
            MBModelKill(capsuleMdlId[i]);
        }
    }
    HuPrcSleep(30);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_CAPSULEEND, HUWIN_SPEAKER_NIRL);
    MBTopWinWait();
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_BOARD_MGCIRCUIT_START, HUWIN_SPEAKER_NIRL);
    MBTopWinWait();
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    while(MBMusCheck(MB_MUS_CHAN_BG) || WipeCheckEnd()) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

static void ExecFusenObj(OMOBJ *obj);
static void ExecHikosenObj(OMOBJ *obj);

static BOOL FusenObjColCheck(void);

static void CreateFusenObj(HuVecF *pos)
{
    static const unsigned int fileTbl[5] = {
        W20_HSF_fusen1,
        W20_HSF_fusen2,
        W20_HSF_fusen3,
        W20_HSF_fusen4,
        W20_HSF_fusen5
    };
    
    int i;
    for(i=0; i<FUSENOBJ_MAX; i++) {
        OMOBJ *obj = fusenObj[i] = MBAddObj(256, 1, 0, ExecFusenObj);
        obj->mdlId[0] = MBModelCreate(fileTbl[i/5], NULL, TRUE);
        obj->trans.x = pos->x+(100.0f*(10.0f*(frandf()-0.5f)));
        obj->trans.y = pos->y;
        obj->trans.z = pos->z+(100.0f*(10.0f*(frandf()-0.5f)));
        obj->rot.x = 100.0f*(0.1f*(frandf()-0.5f));
        obj->rot.y = 2.5f+(100.0f*(frandf()*0.075f));
        omSetStatBit(obj, OM_STAT_MODELPAUSE);
    }
}

static void ExecFusenObj(OMOBJ *obj)
{
    if(MBKillCheck() || obj->work[0]) {
        MBModelKill(obj->mdlId[0]);
        MBDelObj(obj);
        return;
    }
    obj->trans.x += obj->rot.x;
    obj->trans.y += obj->rot.y;
    MBModelPosSetV(obj->mdlId[0], &obj->trans);
}

static void KillFusenObj(void)
{
    int i;
    for(i=0; i<FUSENOBJ_MAX; i++) {
        fusenObj[i]->work[0] = TRUE;
    }
    hikosenObj->work[0] = TRUE;
}

static BOOL FusenObjColCheck(void)
{
    int i;
    int j;
    OMOBJ *obj1;
    int num;
    OMOBJ *obj2;
    HuVecF outVec[FUSENOBJ_MAX];
    HuVecF dir;
    float mag;
    float scale;
    
    num = 0;
    for(i=0; i<FUSENOBJ_MAX; i++) {
        outVec[i].x = outVec[i].y = outVec[i].z = 0;
    }
    for(i=0; i<FUSENOBJ_MAX; i++) {
        obj1 = fusenObj[i];
        for(j=0; j<FUSENOBJ_MAX; j++) {
            obj2 = fusenObj[j];
            if(i == j) {
                continue;
            }
            VECSubtract(&obj2->trans, &obj1->trans, &dir);
            mag = VECMag(&dir);
            if(mag < 90) {
                if(0.0f == mag) {
                    dir.y = 1;
                } else {
                    VECNormalize(&dir, &dir);
                }
                scale = 0.0001f+(0.5f*(90.0f-mag));
                VECScale(&dir, &dir, scale);
                VECSubtract(&outVec[i], &dir, &outVec[i]);
                VECAdd(&outVec[j], &dir, &outVec[j]);
                num++;
            }
        }
    }
    for(i=0; i<FUSENOBJ_MAX; i++) {
        obj1 = fusenObj[i];
        VECAdd(&obj1->trans, &outVec[i], &obj1->trans);
    }
}

static void CreateHikosenObj(HuVecF *pos)
{
    OMOBJ *obj = hikosenObj = MBAddObj(256, 1, 0, ExecHikosenObj);
    obj->mdlId[0] = MBModelCreate(W20_HSF_hikosen, NULL,  FALSE);
    MBMotionNoSet(obj->mdlId[0], MB_MOT_DEFAULT, HU3D_MOTATTR_LOOP);
    obj->trans.x = pos->x;
    obj->trans.y = pos->y;
    obj->trans.z = pos->z;
    obj->work[1] = 0;
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
}

static void ExecHikosenObj(OMOBJ *obj)
{
    float angle;
    float time;
    HuVecF pos;
    int i;
    if(MBKillCheck() || obj->work[0]) {
        MBModelKill(obj->mdlId[0]);
        MBDelObj(obj);
        return;
    }
    time = obj->work[1]++/720.0f;
    angle = 15+(-30*time);
    pos.x = obj->trans.x+(100*(100*HuSin(angle)));
    pos.y = obj->trans.y;
    pos.z = obj->trans.z+(100*(100*HuCos(angle)));
    MBModelPosSetV(obj->mdlId[0], &pos);
    MBModelRotSet(obj->mdlId[0], 0, angle-90, 0);
    for(i=0; i<50; i++) {
        if(!FusenObjColCheck()) {
            break;
        }
    }
}

float MBOpeningViewZoomGet(void)
{
    return openingViewZoom;
}

void MBOpeningViewRotGet(HuVecF *rot)
{
    *rot = openingViewRot;
}

void MBOpeningViewFocusGet(HuVecF *focus)
{
    *focus = openingViewTarget;
}
