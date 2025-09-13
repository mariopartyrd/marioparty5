#include "game/esprite.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/main.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/com.h"
#include "game/board/masu.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/pad.h"
#include "game/board/pause.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/board/audio.h"
#include "game/board/coin.h"
#include "game/board/effect.h"
#include "game/board/roulette.h"
#include "game/board/guide.h"
#include "game/board/star.h"

#include "datanum/capsule.h"
#include "datanum/effect.h"

#include "datanum/capsulechar0.h"
#include "datanum/capsulechar1.h"
#include "datanum/capsulechar2.h"
#include "datanum/capsulechar3.h"
#include "datanum/capsulechar4.h"

#include "messnum/capsule_ex01.h"
#include "messnum/capsule_ex02.h"
#include "messnum/capsule_ex03.h"
#include "messnum/capsule_ex04.h"
#include "messnum/capsule_ex98.h"
#include "messnum/last5koopa.h"
#include "messnum/koopamasu.h"
#include "messnum/vsmasu.h"
#include "messnum/circuitevent.h"
#include "messnum/storyevent.h"
#include "messnum/board_star.h"

//Fake Board Prototypes
extern BOOL MBCircuitGoalCheck(int playerNo);

typedef void (*CAPSULE_FUNC)(void);

typedef struct CapsuleData_s {
    CAPSULE_FUNC main;
    CAPSULE_FUNC destroy;
    BOOL masuF;
    int statusMode;
    BOOL focusF;
    int dirNum;
} CAPSULE_DATA;

#define CAPSULE_WORK_MAX 64

typedef struct CapsuleStar_s {
    int mdlId;
    int parManId;
    ANIMDATA *animP;
    OMOBJ *obj;
    HuVecF pos;
    int angle;
} CAPSULE_STAR;

typedef struct CapsuleWork_s {
    CAPSULE_STAR star[CAPSULE_WORK_MAX];
    int motId[CAPSULE_WORK_MAX][GW_PLAYER_MAX];
    int mdlId[CAPSULE_WORK_MAX];
    int sprId[CAPSULE_WORK_MAX];
    void *mem[CAPSULE_WORK_MAX];
} CAPSULE_WORK;

typedef struct CapsuleFlag_s {
    unsigned saiHiddenF : 1;
    unsigned kettouF : 1;
    unsigned circuitKettouF : 1;
    unsigned capsuleKoopaGetF : 1;
    unsigned capsuleMiracleGetF : 1;
    unsigned kettouMgEndF : 1;
    unsigned circuitKettouMgEndF : 1;
    unsigned donkeyOpeningF : 1;
    
    unsigned donkeyF : 1;
    unsigned koopaF : 1;
    unsigned circuitKoopaF : 1;
    unsigned last5KoopaF : 1;
    unsigned gameEndF : 1;
    unsigned vsMgF : 1;
    unsigned donkeyMgEndF : 1;
    unsigned koopaMgEndF : 1;
    unsigned circuitKoopaMgEndF : 1;
    unsigned vsMgEndF : 1;
    
    unsigned killerEventF : 1;
    unsigned killerMultiF : 1;
    unsigned moveF : 1;
} CAPSULE_FLAG;


static HUPROCESS *eventKillerProc[GW_PLAYER_MAX] = {};
static OMOBJ *explodeEffObj[GW_PLAYER_MAX] = {};

static CAPSULE_DATA capsuleData[CAPSULE_MAX] = {
    {
        MBCapsuleKinokoExec,
        MBCapsuleKinokoDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleSKinokoExec,
        MBCapsuleSKinokoDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleNKinokoExec,
        MBCapsuleNKinokoDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleDokanExec,
        MBCapsuleDokanDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleJangoExec,
        MBCapsuleJangoDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar2
    },
    {
        MBCapsuleBobleExec,
        MBCapsuleBobleDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsuleHanachanExec,
        MBCapsuleHanachanDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar1
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleHammerBroExec,
        MBCapsuleHammerBroDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsuleCoinBlockExec,
        MBCapsuleCoinBlockDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleTogezoExec,
        MBCapsuleTogezoDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsulePatapataExec,
        MBCapsulePatapataDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar2
    },
    {
        MBCapsuleKillerExec,
        MBCapsuleKillerDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsuleKuriboExec,
        MBCapsuleKuriboDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar2
    },
    {
        MBCapsuleBomheiExec,
        MBCapsuleBomheiDestroy,
        TRUE,
        1,
        FALSE,
        DATA_capsulechar0
    },
    {
        MBCapsuleBankExec,
        MBCapsuleBankDestroy,
        TRUE,
        0,
        TRUE,
        DATA_capsulechar0
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleKamekkuExec,
        MBCapsuleKamekkuDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsuleThrowmanExec,
        MBCapsuleThrowmanDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsulePakkunExec,
        MBCapsulePakkunDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar0
    },
    {
        MBCapsuleKokamekkuExec,
        MBCapsuleKokamekkuDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsuleUkkiExec,
        MBCapsuleUkkiDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar2
    },
    {
        MBCapsuleJugemExec,
        MBCapsuleJugemDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar0
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleTumujikunExec,
        MBCapsuleTumujikunDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar1
    },
    {
        MBCapsuleKettouExec,
        MBCapsuleKettouDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar0
    },
    {
        MBCapsuleWanwanExec,
        MBCapsuleWanwanDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar0
    },
    {
        MBCapsuleHoneExec,
        MBCapsuleHoneDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleKoopaExec,
        MBCapsuleKoopaDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar4
    },
    {
        MBCapsuleChanceExec,
        MBCapsuleChanceDestroy,
        FALSE,
        2,
        FALSE,
        DATA_capsulechar4
    },
    {
        MBCapsuleMiracleExec,
        MBCapsuleMiracleDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleDonkeyExec,
        MBCapsuleDonkeyDestroy,
        FALSE,
        0,
        TRUE,
        DATA_capsulechar3
    },
    {
        MBCapsuleVsExec,
        MBCapsuleVsDestroy,
        FALSE,
        1,
        TRUE,
        DATA_capsulechar4
    },
    {
        NULL,
        NULL,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleDebugCamExec,
        MBCapsuleDebugCamDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleDebugWarpExec,
        MBCapsuleDebugWarpDestroy,
        FALSE,
        0,
        TRUE
    },
    {
        MBCapsuleDebugPosSetExec,
        MBCapsuleDebugPosSetDestroy,
        FALSE,
        0,
        TRUE
    },
};

static int charMdlFileTbl[CHARNO_MAX] = {
    DATA_mariomdl1,
    DATA_luigimdl1,
    DATA_peachmdl1,
    DATA_yoshimdl1,
    DATA_wariomdl1,
    DATA_daisymdl1,
    DATA_waluigimdl1,
    DATA_kinopiomdl1,
    DATA_teresamdl1,
    DATA_minikoopamdl1,
    DATA_minikoopamdl1,
    DATA_minikoopamdl1,
    DATA_minikoopamdl1,
};

static int charMotIdleFileTbl[CHARNO_MAX] = {
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_300, minikoopa),
};

static int charMotWinFileTbl[CHARNO_MAX] = {
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, kinopio),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, teresa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_306, minikoopa),
};

static int numberFileTbl[] = {
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

static HuVecF lbl_801CB84C[] = {
    { 0, -10, -20 },
    { 0, -10, -20 },
    { 0, -30.000002, -20 },
    { 0, -10, -20 },
    { 0, -10, -20 },
    { 0, 10, -10 },
    { 0, -30.000002, -20 },
    { 0, -10, -20 },
    { 0, -10, -20 },
    { 0, -10, -20 }
};

static GXColor kinokoGlowColorTbl[] = {
    { 255, 127, 127, 255 },
    { 255, 127, 64, 255 },
    { 255, 255, 127, 255 },
    { 127, 255, 127, 255 },
    { 127, 127, 255, 255 },
    { 64, 64, 255, 255 },
    { 255, 127, 255, 255 },
};

static HuVecF capsuleViewOfs = { 0, 100, 0 };

static CAPSULE_FUNC killerEventTbl[GW_PLAYER_MAX] = {
    MBCapsuleKillerMoveP1Exec,
    MBCapsuleKillerMoveP2Exec,
    MBCapsuleKillerMoveP3Exec,
    MBCapsuleKillerMoveP4Exec
};

static CAPSULE_FUNC killerEventDestroyTbl[GW_PLAYER_MAX] = {
    MBCapsuleKillerMoveP1Destroy,
    MBCapsuleKillerMoveP2Destroy,
    MBCapsuleKillerMoveP3Destroy,
    MBCapsuleKillerMoveP4Destroy
};

static int capsulePlayer = -1;
static int capsuleCurNo = -1;
static int kettouCoinLose = 10;
static int kettouOppCoinLose = 5;

static CAPSULE_FLAG capsuleFlags;
static int capsuleChoice;
static int saiHookMdlId;
static OMOBJ *capsuleObj;
static BOOL saiHookF;
static int playerCapsuleSeTimer;
static CAPSULE_EFF_HOOK capsuleEffHook;
static CAPSULE_WORK *capsuleWork;
static OMOBJ *chanceSprObj;
static OMOBJ *loseEffObj;
static OMOBJ *starManObj;
static OMOBJ *coinManObj;
static OMOBJ *coinEffObj;
static OMOBJ *hanachanRingObj;
static OMOBJ *rayEffObj;
static OMOBJ *ringEffObj;
static OMOBJ *glowEffObj;
static OMOBJ *snowEffObj;
static OMOBJ *NKinokoEffObj;
static BOOL capsuleMasuF;
static BOOL capsuleCircuitF;
static HUPROCESS *capsuleProc;

static BOOL killerMoveF[GW_PLAYER_MAX];

static void CapsuleEventCall(void);
static void CapsuleDataInit(void);
static void CapsuleDataClose(void);

BOOL MBCapsuleExec(int playerNo, int capsuleNo, BOOL circuitF, BOOL masuF)
{
    BOOL resetDispF = FALSE;
    int masuId, viewNo;
    capsuleNo &= 0xFF;
    if(playerNo < 0 || playerNo >= GW_PLAYER_MAX || capsuleNo < 0 || capsuleNo >= CAPSULE_MAX) {
        return TRUE;
    }
    if(masuF && !capsuleData[capsuleNo].masuF) {
        return TRUE;
    }
    if(masuF && capsuleData[capsuleNo].masuF) {
        if(GwPlayer[playerNo].walkNum <= 1 && !_CheckFlag(FLAG_BOARD_DEBUG)) {
            return TRUE;
        } else {
            MBWalkNumDispSet(playerNo, FALSE);
            resetDispF = TRUE;
        }
    }
    capsulePlayer = playerNo;
    capsuleCurNo = capsuleNo;
    capsuleCircuitF = circuitF;
    capsuleMasuF = masuF;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    masuId = GwPlayer[capsulePlayer].masuId;
    viewNo = MBCameraViewNoGet();
    MBAudFXPlay(MSM_SE_BOARD_31);
    if(!capsuleCircuitF && !capsuleMasuF) {
        int player = capsulePlayer;
        GwPlayer[player].capsuleMasuNum++;
        if(GwPlayer[player].capsuleMasuNum > 99) {
            GwPlayer[player].capsuleMasuNum = 99;
        }
    }
    MBPlayerPosFixFlagSet(FALSE);
    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
        MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        MBPlayerRotateStart(capsulePlayer, 0, 15);
        while(!MBPlayerRotateCheck(capsulePlayer)) {
            HuPrcVSleep();
        }
    }
    if(capsuleData[capsuleCurNo].focusF) {
        MBCameraSkipSet(TRUE);
        MBCapsuleCameraViewPlayerSet(capsulePlayer);
        MBCameraMotionWait();
    }
    switch(capsuleData[capsuleCurNo].statusMode) {
        case 0:
            if(!MBCapsuleStatusDispCheck(capsulePlayer)) {
                MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                MBStatusDispFocusSet(capsulePlayer, TRUE);
                while(!MBStatusMoveCheck(capsulePlayer)) {
                    HuPrcVSleep();
                }
            }
            break;
       
        case 1:
            MBCapsuleStatusPosMoveWait(TRUE, TRUE);
            break;
      
        case 2:
            MBCapsuleStatusPosMoveWait(FALSE, TRUE);
            break;
    }
    CapsuleEventCall();
    if(resetDispF) {
        MBWalkNumDispSet(capsulePlayer, TRUE);
    }
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    MBCapsuleCameraViewSet(capsulePlayer, viewNo, masuF);
    if(masuId != GwPlayer[capsulePlayer].masuId || capsuleFlags.moveF) {
        return FALSE; 
    } else {
        return TRUE;
    }
}

void MBSaiHiddenExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_COINBLOCK;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.saiHiddenF = TRUE;
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    capsuleFlags.saiHiddenF = FALSE;
}

void MBKettouExec(int playerNo)
{
    if(GWTeamFGet()) {
        int i;
        int num;
        for(i=0, num=0; i<GW_PLAYER_MAX; i++) {
            if(i != playerNo && !MBCapsuleTeamCheck(i, playerNo) && GwPlayer[i].masuId == GwPlayer[playerNo].masuId) {
                num++;
            }
        }
        if(num <= 0) {
            return;
        }
    }
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_DUEL;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.kettouF = TRUE;
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    capsuleFlags.kettouF = FALSE;
}

void MBCircuitKettouExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_DUEL;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.circuitKettouF = TRUE;
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    MBDataAsyncWait(MBDataDirReadAsync(DATA_capsule));
    CapsuleEventCall();
    capsuleFlags.circuitKettouF = FALSE;
}

void MBCapsuleKupaGetExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_KOOPA;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.capsuleKoopaGetF = TRUE;
    MBWalkNumDispSet(playerNo, FALSE);
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    MBWalkNumDispSet(playerNo, TRUE);
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    capsuleFlags.capsuleKoopaGetF = FALSE;
}

void MB3MiracleGetExec(int playerNo)
{
    int i;
    int num;
    int max;
    max = MBPlayerCapsuleNumGet(playerNo);
    for(i=0, num=0; i<max; i++) {
        if(MBPlayerCapsuleGet(playerNo, i) == CAPSULE_MIRACLE) {
            num++;
        }
    }
    if(num < 3) {
        return;
    }
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_MIRACLE;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.capsuleMiracleGetF = TRUE;
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.capsuleMiracleGetF = FALSE;
}

void MBDonkeyOpeningExec(void)
{
    capsulePlayer = 0;
    capsuleCurNo = CAPSULE_DONKEY;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.donkeyOpeningF = TRUE;
    CapsuleEventCall();
    capsuleFlags.donkeyOpeningF = FALSE;
}

void MBDonkeyExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_DONKEY;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    MBAudFXPlay(MSM_SE_BOARD_104);
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.donkeyF = TRUE;
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.donkeyF = FALSE;
}

void MBKupaExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_KOOPA;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.koopaF = TRUE;
    MBAudFXPlay(MSM_SE_BOARD_99);
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.koopaF = FALSE;
}

void MBCircuitKupaExec(int playerNo)
{
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        int i;
        int num;
        for(i=0, num=0; i<GW_PLAYER_MAX; i++) {
            if(i != playerNo && MBCircuitGoalCheck(i)) {
                num++;
            }
        }
        if(num > 0) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KOOPA_MG_CANCEL, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTopWinKill();
            return;
        }
    }
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_KOOPA;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.circuitKoopaF = TRUE;
    MBAudFXPlay(MSM_SE_BOARD_99);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    MBDataAsyncWait(MBDataDirReadAsync(DATA_capsule));
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.circuitKoopaF = FALSE;
}

void MBLast5Exec(void)
{
    _SetFlag(FLAG_BOARD_TURN_NOSTART);
    capsulePlayer = 0;
    capsuleCurNo = CAPSULE_KOOPA;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.last5KoopaF = TRUE;
    GwSystem.turnPlayerNo = -1;
    CapsuleEventCall();
    GwSystem.turnPlayerNo = 0;
    capsuleFlags.last5KoopaF = FALSE;
}

void MBGameEndExec(void)
{
    capsulePlayer = 0;
    capsuleCurNo = CAPSULE_KOOPA;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.gameEndF = TRUE;
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    capsuleFlags.gameEndF = FALSE;
}

void MBVsMgExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_VS;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.vsMgF = TRUE;
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    capsuleFlags.vsMgF = FALSE;
}

void MBKettouMgEndExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_DUEL;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.kettouMgEndF = TRUE;
    MBCapsuleStatusPosMoveWait(TRUE, FALSE);
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    capsuleFlags.kettouMgEndF = FALSE;
}

BOOL MBCircuitKettouMgEndExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_DUEL;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.circuitKettouMgEndF = TRUE;
    MBCapsuleStatusPosMoveWait(TRUE, FALSE);
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    capsuleFlags.circuitKettouMgEndF = FALSE;
    return FALSE;
}

void MBDonkeyMgEndExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_DONKEY;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.donkeyMgEndF = TRUE;
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.donkeyMgEndF = FALSE;
}

void MBKupaMgEndExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_KOOPA;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.koopaMgEndF = TRUE;
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.koopaMgEndF = FALSE;
}

void MBCircuitKupaMgEndExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_KOOPA;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.circuitKoopaMgEndF = TRUE;
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.circuitKoopaMgEndF = FALSE;
}

void MBVsMgEndExec(int playerNo)
{
    capsulePlayer = playerNo;
    capsuleCurNo = CAPSULE_VS;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    memset(&capsuleFlags, 0, sizeof(capsuleFlags));
    capsuleFlags.vsMgEndF = TRUE;
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    CapsuleEventCall();
    MBCapsuleCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_NULL);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    capsuleFlags.vsMgEndF = FALSE;
}

void MBKillerExec(int playerNo)
{
    int i;
    capsuleFlags.killerMultiF = FALSE;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        eventKillerProc[i] = NULL;
    }
    MBKillerEventExec(playerNo);
    while(1) {
        HuPrcVSleep();
        if(MBKillerEventCheck(playerNo)) {
            break;
        }
    }
    MBCapsuleCameraViewNoSet(playerNo, MB_CAMERA_VIEW_NULL);
    capsuleFlags.killerEventF = FALSE;
}

void MBKillerMultiExec(int *playerNo)
{
    int i;
    capsuleFlags.killerMultiF = TRUE;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        eventKillerProc[i] = NULL;
    }
    for(i=0; i<GW_PLAYER_MAX && playerNo[i] != -1; i++) {
        if(playerNo[i] > 0) {
            MBKillerEventExec(playerNo[i]);
        }
    }
    while(1) {
        HuPrcVSleep();
        if(MBKillerEventCheckAll()) {
            break;
        }
    }
    capsuleFlags.killerEventF = FALSE;
    capsuleFlags.killerMultiF = FALSE;
}

void MBKillerEventExec(int playerNo)
{
    int i;
    while(1) {
        HuPrcVSleep();
        if(MBKillerEventCheck(playerNo)) {
            break;
        }
    }
    capsulePlayer = playerNo;
    capsuleCurNo= CAPSULE_KILLER;
    capsuleCircuitF = FALSE;
    capsuleMasuF = FALSE;
    capsuleFlags.killerEventF = TRUE;
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBCapsuleDataDirRead(CAPSULE_KILLER);
    if(MBKillerEventCheckAll()) {
        CapsuleDataInit();
        _SetFlag(FLAG_BOARD_TURN_NOSTART);
        MBPlayerMoveInit();
        MBPlayerMoveObjCreate();
        for(i=0; i<GW_PLAYER_MAX; i++) {
            killerMoveF[i] = FALSE;
        }
    }
    eventKillerProc[playerNo] = MBPrcCreate(killerEventTbl[playerNo], 8196, 24576);
    MBPrcDestructorSet(eventKillerProc[playerNo], killerEventDestroyTbl[playerNo]);
}

BOOL MBKillerEventCheck(int playerNo)
{
    if(eventKillerProc[playerNo] == NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MBKillerEventCheckAll(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBKillerEventCheck(i)) {
            break;
        }
    }
    if(i < GW_PLAYER_MAX) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static BOOL CapsuleEventCheck(void);

int MBCapsuleNoGet(void)
{
    if(CapsuleEventCheck()) {
        return CAPSULE_NULL;
    } else{
        return capsuleCurNo;
    }
}

int MBBankCoinGet(void)
{
    return GwSystem.bankCoin;
}

int MBKettouCoinLoseGet(void)
{
    return kettouCoinLose;
}

int MBKettouOppCoinLoseGet(void)
{
    return kettouOppCoinLose;
}

int MBKettouCoinLoseGet2(void)
{
    return kettouCoinLose;
}

int MBKettouOppCoinLoseGet2(void)
{
    return kettouOppCoinLose;
}

void MBCapsuleOpeningAdd(int capsuleNum)
{
    int i;
    int *capsuleList;
    int *masuTbl;
    int *capsuleListOut;
    int masuNum;
    int capsuleListOutNum;
    int masuId;
    int srcIdx;
    int dstIdx;
    int no;
    int temp;
    int capsuleListNum;
    int masuOfs;
    
    masuTbl = HuMemDirectMallocNum(HEAP_HEAP, sizeof(int)*MASU_MAX, HU_MEMNUM_OVL);
    memset(masuTbl, 0, sizeof(int)*MASU_MAX);
    for(i=1, masuNum=0; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        if(MBMasuTypeGet(MASU_LAYER_DEFAULT,  i) == MASU_TYPE_BLUE || MBMasuTypeGet(MASU_LAYER_DEFAULT,  i) == MASU_TYPE_RED) {
            masuTbl[masuNum] = i;
            masuNum++;
        }
    }
    for(i=0; i<masuNum*5; i++) {
        srcIdx = i%masuNum;
        dstIdx = (frand() & 0x7FFF)%masuNum;
        temp = masuTbl[srcIdx];
        masuTbl[srcIdx] = masuTbl[dstIdx];
        masuTbl[dstIdx] = temp;
    }
    capsuleList = HuMemDirectMallocNum(HEAP_HEAP, (CAPSULE_MAX-4)*sizeof(int), HU_MEMNUM_OVL);
    memset(capsuleList, 0, (CAPSULE_MAX-4)*sizeof(int));
    capsuleListNum = MBCapsuleListGet(capsuleList);
    for(i=0; i<capsuleListNum; i++) {
        if(GWPartyFGet() == FALSE && capsuleList[i] == CAPSULE_PATAPATA) {
            capsuleList[i] = CAPSULE_NULL;
        }
        if(GWPartyFGet() == TRUE && capsuleList[i] == CAPSULE_BANK) {
            capsuleList[i] = CAPSULE_NULL;
        }
        if(capsuleList[i] == CAPSULE_BOMHEI ||
            capsuleList[i] == CAPSULE_HONE ||
            capsuleList[i] == CAPSULE_KOOPA ||
            capsuleList[i] == CAPSULE_MIRACLE ||
            MBCapsuleCodeGet(capsuleList[i]) == 'C' ||
            MBCapsuleCodeGet(capsuleList[i]) == 'E') {
                capsuleList[i] = CAPSULE_NULL;
            }
    }
    capsuleListOut = HuMemDirectMallocNum(HEAP_HEAP, (CAPSULE_MAX-4)*sizeof(int), HU_MEMNUM_OVL);
    memset(capsuleListOut, 0, (CAPSULE_MAX-4)*sizeof(int));
    for(i=0, capsuleListOutNum=0; i<capsuleListNum; i++) {
        if(capsuleList[i] != CAPSULE_NULL) {
            capsuleListOut[capsuleListOutNum] = capsuleList[i];
            capsuleListOutNum++;
        }
    }
    for(i=0; i<capsuleListOutNum*5; i++) {
        srcIdx = i%capsuleListOutNum;
        dstIdx = (frand() & 0x7FFF)%capsuleListOutNum;
        temp = capsuleListOut[srcIdx];
        capsuleListOut[srcIdx] = capsuleListOut[dstIdx];
        capsuleListOut[dstIdx] = temp;
    }
    for(no=0; no<capsuleNum; no++) {
        masuOfs = (frand() & 0x7FFF)%masuNum;
        for(i=0; i<masuNum; i++) {
            masuId = masuOfs+i;
            if(masuId >= masuNum) {
                masuId %= masuNum;
            }
            if(MBMasuCapsuleGet(MASU_LAYER_DEFAULT, masuTbl[masuId]) == CAPSULE_NULL) {
                masuId = masuTbl[masuId];
                break;
            }
        }
        if(i < masuNum) {
            MBCapsuleMasuSet(masuId, capsuleListOut[no%capsuleListOutNum]);
        }
    }
    HuMemDirectFree(masuTbl);
    HuMemDirectFree(capsuleList);
    HuMemDirectFree(capsuleListOut);
}

void MBCapsuleBowserAdd(void)
{
    int *masuTbl;
    int i;
    int masuNum;
    int srcIdx;
    int dstIdx;
    int temp;
    masuTbl = HuMemDirectMallocNum(HEAP_HEAP, sizeof(int)*MASU_MAX, HU_MEMNUM_OVL);
    memset(masuTbl, 0, sizeof(int)*MASU_MAX);
    
    if(GWPartyFGet() == TRUE) {
        for(i=1, masuNum=0; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
            if(MBMasuTypeGet(MASU_LAYER_DEFAULT,  i) == MASU_TYPE_RED) {
                masuTbl[masuNum] = i;
                masuNum++;
            }
        }
        for(i=0; i<masuNum*5; i++) {
            srcIdx = i%masuNum;
            dstIdx = (frand() & 0x7FFF)%masuNum;
            temp = masuTbl[srcIdx];
            masuTbl[srcIdx] = masuTbl[dstIdx];
            masuTbl[dstIdx] = temp;
        }
        MBMasuTypeSet(MASU_LAYER_DEFAULT, masuTbl[0], MASU_TYPE_KUPA);
        MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuTbl[0], CAPSULE_KOOPA);
    }
    HuMemDirectFree(masuTbl);
}

void MBCapsuleEffHookSet(CAPSULE_EFF_HOOK effHook)
{
    capsuleEffHook = effHook;
}

void MBCapsuleEffHookCall(int type, int mdlId, BOOL flag1, BOOL flag2, BOOL flag3)
{
    if(capsuleEffHook != NULL) {
        capsuleEffHook(capsuleCurNo, type, mdlId, flag1, flag2, flag3);
    }
}

void MBCapsuleEffHookStoryCall(int evtType, int type, int mdlId)
{
    if(capsuleEffHook != NULL) {
        switch(evtType) {
            case 0:
                capsuleEffHook(CAPSULE_KOOPA, type, mdlId, FALSE, FALSE, FALSE);
                break;
           
            case 1:
                capsuleEffHook(CAPSULE_INVALID, type, mdlId, FALSE, FALSE, FALSE);
                break;
        }
    }
}

void MBBankCoinReset(void)
{
    GwSystem.bankCoin = 0;
    capsuleEffHook = NULL;
}

void MBMasuBomheiSet(int masuId)
{
    MBMasuBomheiNumSet(masuId, 3);
}

void MBMasuBomheiNumSet(int masuId, int num)
{
    int capsuleNo = MBMasuCapsuleGet(MASU_LAYER_DEFAULT, masuId) & 0xFF;
    capsuleNo |= (num << 8);
    MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, capsuleNo);
}

static void CapsuleEventDestroy(void)
{
    MBCapsuleEffHookCall(2, MB_MODEL_NONE, FALSE, FALSE, FALSE);
    if(capsuleCurNo != CAPSULE_NULL && capsuleData[capsuleCurNo].destroy != NULL) {
        CAPSULE_FUNC func = capsuleData[capsuleCurNo].destroy;
        func();
    }
    HuDataDirClose(DATA_blast5);
    HuDataDirClose(DATA_guide);
    HuDataDirClose(DATA_effect);
    HuDataDirClose(DATA_capsulechar0);
    HuDataDirClose(DATA_capsulechar1);
    HuDataDirClose(DATA_capsulechar2);
    HuDataDirClose(DATA_capsulechar3);
    HuDataDirClose(DATA_capsulechar4);
    capsuleObj = NULL;
    CapsuleDataClose();
    capsuleProc = NULL;
}

static BOOL CapsuleEventCheck(void)
{
    if(capsuleProc != NULL) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void CapsuleEventCall(void)
{
    if(!capsuleFlags.donkeyOpeningF) {
        _SetFlag(FLAG_BOARD_TURN_NOSTART);
    }
    MBCapsuleEffHookCall(0, MB_MODEL_NONE, FALSE, FALSE, FALSE);
    MBCapsuleDataDirRead(capsuleCurNo);
    CapsuleDataInit();
    capsuleProc = MBPrcCreate(capsuleData[capsuleCurNo].main, 8196, 24576);
    MBPrcDestructorSet(capsuleProc, CapsuleEventDestroy);
    while(1) {
        HuPrcVSleep();
        if(CapsuleEventCheck()) {
            break;
        }
    }
    if(!capsuleFlags.donkeyOpeningF) {
        _ClearFlag(FLAG_BOARD_TURN_NOSTART);
    }
}

void MBCapsuleDataDirRead(int capsuleNo)
{
    int statId;
    if(capsuleData[capsuleCurNo].dirNum) {
        statId = MBDataDirReadAsync(capsuleData[capsuleCurNo].dirNum);
        if(statId != HU_DATA_STAT_NONE) {
            MBDataAsyncWait(statId);
        }
    }
}

static void CapsuleDataInit(void)
{
    CAPSULE_WORK *work;
    int i;
    int j;
    work = capsuleWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULE_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(CAPSULE_WORK));
    for(i=0; i<CAPSULE_WORK_MAX; i++) {
        work->star[i].mdlId = MB_MODEL_NONE;
        for(j=0; j<GW_PLAYER_MAX; j++) {
            work->motId[i][j] = MB_MOT_NONE;
        }
        work->mdlId[i] = MB_MODEL_NONE;
        work->sprId[i] = -1;
        work->mem[i] = NULL;
    }
}

static void CapsuleDataClose(void)
{
    CAPSULE_WORK *work = capsuleWork;
    int i;
    int j;
    if(capsuleWork == NULL) {
        return;
    }
    for(i=0; i<CAPSULE_WORK_MAX; i++) {
        if(work->star[i].mdlId != MB_MODEL_NONE) {
            MBCapsuleStarKill(work->star[i].mdlId);
        }
        if(!MBKillCheck()) {
            for(j=0; j<GW_PLAYER_MAX; j++) {
                if(work->motId[i][j] != MB_MOT_NONE) { 
                    MBPlayerMotionKill(j, work->motId[i][j]);
                }
            }
        }
        if(work->mdlId[i] != MB_MODEL_NONE) {
            MBModelKill(work->mdlId[i]);
        }
        if(work->sprId[i] != -1) {
            espKill(work->sprId[i]);
        }
        if(work->mem[i] != NULL) {
            HuMemDirectFree(work->mem[i]);
        }
    }
    HuMemDirectFree(capsuleWork);
    capsuleWork = NULL;
}

static HU3DPARMANID CapsuleStarEffCreate(ANIMDATA *animP);

int MBCapsuleStarCreate(void)
{
    CAPSULE_WORK *work = capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->star[no].mdlId == MB_MODEL_NONE) {
            break;
        }
    }
    if(no >= CAPSULE_WORK_MAX) {
        return MB_MODEL_NONE;
    }
    work->star[no].mdlId = MBModelCreate(BOARD_HSF_star, NULL, TRUE);
    work->star[no].animP = HuSprAnimDataRead(EFFECT_ANM_glow);
    work->star[no].parManId = CapsuleStarEffCreate(work->star[no].animP);
    HuDataDirClose(DATA_effect);
    work->star[no].obj = NULL;
    return work->star[no].mdlId;
}

void MBCapsuleStarKill(int mdlId)
{
    CAPSULE_WORK *work = capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    if(mdlId == MB_MODEL_NONE) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->star[no].mdlId == mdlId) {
            MBModelKill(mdlId);
            work->star[no].mdlId = MB_MODEL_NONE;
            if(work->star[no].parManId != HU3D_PARMANID_NONE) {
                Hu3DParManKill(work->star[no].parManId);
            }
            work->star[no].parManId = HU3D_PARMANID_NONE;
            work->star[no].animP = NULL;
            if(!MBKillCheck() && work->star[no].obj != NULL) {
                work->star[no].obj = NULL;
            }
        }
    }
}

void MBCapsuleStarPosSet(int mdlId, float x, float y, float z)
{
    CAPSULE_WORK *work = capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    if(mdlId == MB_MODEL_NONE) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->star[no].mdlId == mdlId) {
            MBModelPosSet(work->star[no].mdlId, x, y, z);
            Hu3DParManPosSet(work->star[no].parManId, x, y-50, z);
            work->star[no].pos.x = x;
            work->star[no].pos.y = y;
            work->star[no].pos.z = z;
        }
    }
}

void MBCapsuleStarRotSet(int mdlId, float x, float y, float z)
{
    CAPSULE_WORK *work = capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    if(mdlId == MB_MODEL_NONE) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->star[no].mdlId == mdlId) {
            MBModelRotSet(work->star[no].mdlId, x, y, z);
        }
    }
}

void MBCapsuleStarScaleSet(int mdlId, float x, float y, float z)
{
    CAPSULE_WORK *work = capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    if(mdlId == MB_MODEL_NONE) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->star[no].mdlId == mdlId) {
            MBModelScaleSet(work->star[no].mdlId, x, y, z);
        }
    }
}

void MBCapsuleStarObjCreate(int mdlId)
{
    CAPSULE_WORK *work = capsuleWork;
    int no;
    CAPSULE_STAR **objWork;
    OMOBJ *obj;
    if(capsuleWork == NULL) {
        return;
    }
    if(mdlId == MB_MODEL_NONE) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->star[no].mdlId == mdlId) {
            if(work->star[no].obj) {
                work->star[no].obj = NULL;
            }
            obj = work->star[no].obj = MBAddObj(32768, 0, 0, MBCapsuleStarObjExec);
            obj->data = objWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULE_STAR *), HU_MEMNUM_OVL);
            *objWork = &work->star[no];
            MBModelPosGet(work->star[no].mdlId, &work->star[no].pos);
            work->star[no].angle = 0;
        }
    }
}

void MBCapsuleStarObjExec(OMOBJ *obj)
{
    CAPSULE_STAR *star;
    CAPSULE_STAR **work;
    if(MBKillCheck() || capsuleWork == NULL || obj->data == NULL) {
        obj->data = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    work = obj->data;
    star = *work;
    if(star->obj == NULL) {
        obj->data = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    star->angle += 2;
    {
        float x = star->pos.x;
        float y = star->pos.y+((100*HuSin(star->angle))*0.2f);
        float z = star->pos.z;
        MBModelPosSet(star->mdlId, x, y, z);
        Hu3DParManPosSet(star->parManId, x, y, z);
    }
}

s16 MBCapsulePlayerMotionCreate(int playerNo, int fileNum)
{
    CAPSULE_WORK *work =  capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return MB_MOT_NONE;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->motId[no][playerNo] == MB_MOT_NONE) {
            break;
        }
    }
    if(no >= CAPSULE_WORK_MAX) {
        return MB_MOT_NONE;
    }
    work->motId[no][playerNo] = MBPlayerMotionCreate(playerNo, fileNum);
    HuPrcVSleep();
    return work->motId[no][playerNo];
}

int MBCapsuleModelCreate(int fileNum, int *motTbl, BOOL linkF, int delay, BOOL closeDir)
{
    CAPSULE_WORK *work =  capsuleWork;
    int no;
    int i;
    if(capsuleWork == NULL) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->mdlId[no] == MB_MODEL_NONE) {
            break;
        }
    }
    if(no >= CAPSULE_WORK_MAX) {
        return MB_MODEL_NONE;
    }
    work->mdlId[no] = MBModelCreate(fileNum, NULL, linkF);
    if(closeDir) {
        HuDataDirClose(fileNum);
    }
    MBModelDispSet(work->mdlId[no], FALSE);
    if(delay > 0) {
        HuPrcSleep(delay);
    }
    if(work->mdlId[no] != MB_MODEL_NONE && motTbl != NULL) {
        for(i=0; motTbl[i] != HU_DATANUM_NONE; i++) {
            MBMotionCreate(work->mdlId[no], motTbl[i]);
            if(delay > 0) {
                HuPrcVSleep();
            }
        }
    }
    MBModelDispSet(work->mdlId[no], TRUE);
    return work->mdlId[no];
}

void MBCapsuleModelKill(int mdlId)
{
    CAPSULE_WORK *work =  capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    if(mdlId == MB_MODEL_NONE) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->mdlId[no] == mdlId) {
            MBModelKill(mdlId);
            work->mdlId[no] = MB_MODEL_NONE;
        }
    }
}

s16 MBCapsuleSprCreate(unsigned int dataNum, s16 prio, s16 bank)
{
    CAPSULE_WORK *work =  capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->sprId[no] == -1) {
            break;
        }
    }
    if(no >= CAPSULE_WORK_MAX) {
        return -1;
    }
    work->sprId[no] = espEntry(dataNum, prio, bank);
    return work->sprId[no];
}

void MBCapsuleSprClose(s16 sprId)
{
    CAPSULE_WORK *work =  capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    if(sprId == -1) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->sprId[no] == sprId) {
            espKill(sprId);
            work->sprId[no] = -1;
        }
    }
}

void *MBCapsuleMalloc(int size)
{
    CAPSULE_WORK *work =  capsuleWork;
    int no;
    void *mem;
    if(capsuleWork == NULL) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->mem[no] == NULL) {
            break;
        }
    }
    if(no >= CAPSULE_WORK_MAX) {
        return NULL;
    }
    mem = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    work->mem[no] = mem;
    return work->mem[no];
}

void MBCapsuleFree(void *ptr)
{
    CAPSULE_WORK *work =  capsuleWork;
    int no;
    if(capsuleWork == NULL) {
        return;
    }
    if(ptr == NULL) {
        return;
    }
    for(no=0; no<CAPSULE_WORK_MAX; no++) {
        if(work->mem[no] == ptr) {
            HuMemDirectFree(ptr);
            work->mem[no] = NULL;
        }
    }
}

void MBCapsuleKinokoExec(void)
{
    int i;
    int mdlId;
    float t;
    float power;
    float effAngle;
    float scale;
    float dirRadius;
    float playerRadius;
    float yOfs;
    
    HuVecF effectPos;
    HuVecF dir;
    HuVecF playerPos;
    GXColor color;
    
    MBCapsuleExplodeEffCreate(0);
    MBCapsuleGlowEffCreate();
    
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerPos.y += 300;
    mdlId = MBCapsuleModelCreate(CAPSULE_HSF_kinoko, NULL, FALSE, 10, FALSE);
    MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
    MBModelScaleSet(mdlId, 1, 1, 1);
    MBAudFXPlay(MSM_SE_BOARD_35);
    for(i=0; i<32; i++) {
        effAngle = i*(360.0f/32.0f);
        power = (MBCapsuleEffRandF()*100)*0.33f;
        effectPos.x = playerPos.x+(HuCos(effAngle)*power);
        effectPos.y = playerPos.y+(HuSin(effAngle)*power);
        effectPos.z = playerPos.z+50;
        dirRadius = ((MBCapsuleEffRandF()*0.04f)+0.005f)*100;
        dir.x = HuCos(effAngle)*dirRadius;
        dir.y = HuSin(effAngle)*dirRadius;
        dir.z = 0;
        power = MBCapsuleEffRandF();
        color.r = 192+(power*63);
        color.g = 192+(power*63);
        color.b = 192+(power*63);
        color.a = 192+(MBCapsuleEffRandF()*63);
        MBCapsuleExplodeEffLineAdd(0, effectPos, dir,
            ((MBCapsuleEffRandF()*0.5f)+1.0f)*100,
            2.5f,
            ((MBCapsuleEffRandF()*0.5f)+0.5f)*100,
            (MBCapsuleEffRandF()*0.66f)+0.33f,
            color);
    }
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        scale = HuSin(t*180)+1;
        MBModelScaleSet(mdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    for(i=0; i<90.0f; i++) {
        t = i/90.0f;
        MBPlayerPosGet(capsulePlayer, &playerPos);
        playerPos.y += ((100*HuSin((360*t)*1.5f))*0.1f)+300;
        MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
        if((i & 1) == 0) {
            effectPos.x = playerPos.x+(150*(MBCapsuleEffRandF()+-0.5f));
            effectPos.y = playerPos.y+(150*(MBCapsuleEffRandF()+-0.5f));
            effectPos.z = playerPos.z+50;
            dir.x = 0;
            dir.z = 0;
            dir.y = (MBCapsuleEffRandF()*0.2f)+1.0f;
            power = MBCapsuleEffRandF();
            color.r = 192+(power*63);
            color.g = 192+(power*63);
            color.b = 192+(power*63);
            color.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleGlowEffAdd(effectPos, dir, ((MBCapsuleEffRandF()*0.1f)+0.2f)*100, 0.016666668f, 0.1f, 0, color);
        }
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_BOARD_36);
    for(i=0; i<120.0f; i++) {
        t = 1-(i/120.0f);
        yOfs = 50+(t*t*250);
        playerRadius = HuSin(t*t*180);
        MBPlayerPosGet(capsulePlayer, &playerPos);
        playerPos.x += ((playerRadius*HuCos(3*(t*360)))*100)*1.5;
        playerPos.z += ((playerRadius*HuSin(3*(t*360)))*100)*1.5;
        playerPos.y += yOfs;
        MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
        MBModelScaleSet(mdlId, t, t, t);
        effectPos.x = playerPos.x+(100*(t*(MBCapsuleEffRandF()+-0.5f)));
        effectPos.y = playerPos.y+(100*(t*(MBCapsuleEffRandF()+-0.5f)));
        effectPos.z = playerPos.z+50; 
        dir.x = 0;
        dir.z = 0;
        dir.y = (MBCapsuleEffRandF()*0.2f)+1.0f;
        power = MBCapsuleEffRandF();
        color.r = 192+(power*63);
        color.g = 192+(power*63);
        color.b = 192+(power*63);
        color.a = 192+(MBCapsuleEffRandF()*63);
        MBCapsuleGlowEffAdd(effectPos, dir, ((MBCapsuleEffRandF()*0.1f)+0.2f)*(100*t), 0.016666668f, 0.1f, 0, color);
        HuPrcVSleep();
    }
    GwPlayer[capsulePlayer].saiMode = MB_SAIMODE_KINOKO;
    omVibrate(capsulePlayer, 12, 6, 6);
    for(i=0; i<6.0f; i++) {
        HuPrcVSleep();
    }
    while(MBCapsuleGlowEffCheck() > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void MBCapsuleKinokoDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleGlowEffKill();
}

void MBCapsuleSKinokoExec(void)
{
    int i;
    int mdlId;
    float power;
    float t;
    
    float effAngle;
    float scale;
    float dirRadius;
    float playerRadius;
    float yOfs;
    
    HuVecF effectPos;
    HuVecF dir;
    HuVecF playerPos;
    GXColor color;
    
    MBCapsuleExplodeEffCreate(0);
    MBCapsuleGlowEffCreate();
    
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerPos.y += 300;
    mdlId = MBCapsuleModelCreate(CAPSULE_HSF_skinoko, NULL, FALSE, 10, FALSE);
    MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
    MBModelScaleSet(mdlId, 1, 1, 1);
    MBAudFXPlay(MSM_SE_BOARD_35);
    for(i=0; i<32; i++) {
        effAngle = i*(360.0f/32.0f);
        power = (MBCapsuleEffRandF()*100)*0.33f;
        effectPos.x = playerPos.x+(HuCos(effAngle)*power);
        effectPos.y = playerPos.y+(HuSin(effAngle)*power);
        effectPos.z = playerPos.z+50;
        dirRadius = ((MBCapsuleEffRandF()*0.04f)+0.005f)*100;
        dir.x = HuCos(effAngle)*dirRadius;
        dir.y = HuSin(effAngle)*dirRadius;
        dir.z = 0;
        power = MBCapsuleEffRandF();
        color.r = 192+(power*63);
        color.g = 192+(power*63);
        color.b = 192+(power*63);
        color.a = 192+(MBCapsuleEffRandF()*63);
        MBCapsuleExplodeEffLineAdd(0, effectPos, dir,
            ((MBCapsuleEffRandF()*0.5f)+1.0f)*100,
            2.5f,
            ((MBCapsuleEffRandF()*0.5f)+0.5f)*100,
            (MBCapsuleEffRandF()*0.66f)+0.33f,
            color);
    }
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        scale = HuSin(t*180)+1;
        MBModelScaleSet(mdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    for(i=0; i<90.0f; i++) {
        t = i/90.0f;
        MBPlayerPosGet(capsulePlayer, &playerPos);
        playerPos.y += ((100*HuSin((360*t)*1.5f))*0.1f)+300;
        MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
        if((i & 1) == 0) {
            effectPos.x = playerPos.x+(150*(MBCapsuleEffRandF()+-0.5f));
            effectPos.y = playerPos.y+(150*(MBCapsuleEffRandF()+-0.5f));
            effectPos.z = playerPos.z+50;
            dir.x = 0;
            dir.z = 0;
            dir.y = (MBCapsuleEffRandF()*0.2f)+1.0f;
            power = MBCapsuleEffRandF();
            color.r = 192+(power*63);
            color.g = 192+(power*63);
            color.b = 192+(power*63);
            color.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleGlowEffAdd(effectPos, dir, ((MBCapsuleEffRandF()*0.1f)+0.2f)*100, 0.016666668f, 0.1f, 0, color);
        }
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_BOARD_36);
    for(i=0; i<120.0f; i++) {
        t = 1-(i/120.0f);
        yOfs = 50+(t*t*250);
        playerRadius = HuSin(t*t*180);
        MBPlayerPosGet(capsulePlayer, &playerPos);
        playerPos.x += ((playerRadius*HuCos(3*(t*360)))*100)*1.5;
        playerPos.z += ((playerRadius*HuSin(3*(t*360)))*100)*1.5;
        playerPos.y += yOfs;
        MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
        MBModelScaleSet(mdlId, t, t, t);
        effectPos.x = playerPos.x+(100*(1*(MBCapsuleEffRandF()+-0.5f)));
        effectPos.y = playerPos.y+(100*(1*(MBCapsuleEffRandF()+-0.5f)));
        effectPos.z = playerPos.z+50; 
        dir.x = 0;
        dir.z = 0;
        dir.y = (MBCapsuleEffRandF()*0.2f)+1.0f;
        power = MBCapsuleEffRandF();
        color.r = 192+(power*63);
        color.g = 192+(power*63);
        color.b = 192+(power*63);
        color.a = 192+(MBCapsuleEffRandF()*63);
        MBCapsuleGlowEffAdd(effectPos, dir, ((MBCapsuleEffRandF()*0.1f)+0.2f)*(100*t), 0.016666668f, 0.1f, 0, color);
        HuPrcVSleep();
    }
    GwPlayer[capsulePlayer].saiMode = MB_SAIMODE_SKINOKO;
    omVibrate(capsulePlayer, 12, 4, 2);
    for(i=0; i<6.0f; i++) {
        HuPrcVSleep();
    }
    while(MBCapsuleGlowEffCheck() > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void MBCapsuleSKinokoDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleGlowEffKill();
}

void MBCapsuleNKinokoExec(void)
{
    int i;
    int mdlId;
    float power;
    float t;
    
    float effAngle;
    float scale;
    float dirRadius;
    float playerRadius;
    float yOfs;
    
    HuVecF effectPos;
    HuVecF dir;
    HuVecF playerPos;
    GXColor color;
    
    MBCapsuleExplodeEffCreate(0);
    MBCapsuleGlowEffCreate();
    MBCapsuleGlowEffBlendModeSet(HU3D_PARTICLE_BLEND_INVCOL);
    MBCapsuleNKinokoEffCreate();
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerPos.y += 300;
    mdlId = MBCapsuleModelCreate(CAPSULE_HSF_nkinoko, NULL, FALSE, 10, FALSE);
    MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
    MBModelScaleSet(mdlId, 1, 1, 1);
    MBAudFXPlay(MSM_SE_BOARD_35);
    for(i=0; i<32; i++) {
        effAngle = i*(360.0f/32.0f);
        power = (MBCapsuleEffRandF()*100)*0.33f;
        effectPos.x = playerPos.x+(HuCos(effAngle)*power);
        effectPos.y = playerPos.y+(HuSin(effAngle)*power);
        effectPos.z = playerPos.z+50;
        dirRadius = ((MBCapsuleEffRandF()*0.04f)+0.005f)*100;
        dir.x = HuCos(effAngle)*dirRadius;
        dir.y = HuSin(effAngle)*dirRadius;
        dir.z = 0;
        power = MBCapsuleEffRandF();
        color.r = 127+(power*32);
        color.g = 64+(power*32);
        color.b = 127+(power*32);
        color.a = 192+(MBCapsuleEffRandF()*63);
        MBCapsuleExplodeEffLineAdd(0, effectPos, dir,
            ((MBCapsuleEffRandF()*0.5f)+1.0f)*100,
            2.5f,
            ((MBCapsuleEffRandF()*0.5f)+0.5f)*100,
            (MBCapsuleEffRandF()*0.66f)+0.33f,
            color);
    }
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        scale = HuSin(t*180)+1;
        MBModelScaleSet(mdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    for(i=0; i<90.0f; i++) {
        t = i/90.0f;
        MBPlayerPosGet(capsulePlayer, &playerPos);
        playerPos.y += ((100*HuSin((360*t)*1.5f))*0.1f)+300;
        MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
        if((i & 1) == 0) {
            effectPos.x = playerPos.x+(150*(MBCapsuleEffRandF()+-0.5f));
            effectPos.y = playerPos.y+(150*(MBCapsuleEffRandF()+-0.5f));
            effectPos.z = playerPos.z+50;
            dir.x = 0;
            dir.z = 0;
            dir.y = (MBCapsuleEffRandF()*0.2f)+1.0f;
            power = MBCapsuleEffRandF();
            color.r = 192+(power*63);
            color.g = 64+(power*63);
            color.b = 192+(power*63);
            color.a = 127+(MBCapsuleEffRandF()*63);
            MBCapsuleNKinokoEffAdd(effectPos, dir, ((MBCapsuleEffRandF()*0.1f)+0.2f)*100, 0.1f, 60, color);
        }
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_BOARD_37);
    for(i=0; i<120.0f; i++) {
        t = 1-(i/120.0f);
        yOfs = 50+(t*t*250);
        playerRadius = HuSin(t*t*180);
        MBPlayerPosGet(capsulePlayer, &playerPos);
        playerPos.x += ((playerRadius*HuCos(3*(-t*360)))*100)*1.5;
        playerPos.z += ((playerRadius*HuSin(3*(-t*360)))*100)*1.5;
        playerPos.y += yOfs;
        MBModelPosSet(mdlId, playerPos.x, playerPos.y, playerPos.z);
        MBModelScaleSet(mdlId, t, t, t);
        if((i & 0x1) == 0) {
            effectPos.x = playerPos.x+(150*(1*(MBCapsuleEffRandF()+-0.5f)));
            effectPos.y = playerPos.y+(150*(1*(MBCapsuleEffRandF()+-0.5f)));
            effectPos.z = playerPos.z+50; 
            dir.x = 0;
            dir.z = 0;
            dir.y = (MBCapsuleEffRandF()*0.2f)+1.0f;
            power = MBCapsuleEffRandF();
            color.r = 192+(power*63);
            color.g = 64+(power*63);
            color.b = 192+(power*63);
            color.a = 127+(MBCapsuleEffRandF()*63);
            MBCapsuleNKinokoEffAdd(effectPos, dir, t*(100*((MBCapsuleEffRandF()*0.1f)+0.2f)), 0.1f, 60, color);
        }
        HuPrcVSleep();
    }
    GwPlayer[capsulePlayer].saiMode = MB_SAIMODE_NKINOKO;
    omVibrate(capsulePlayer, 12, 6, 6);
    for(i=0; i<6.0f; i++) {
        HuPrcVSleep();
    }
    while(MBCapsuleGlowEffCheck() > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void MBCapsuleNKinokoDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleGlowEffKill();
    MBCapsuleNKinokoEffKill();
}

void MBCapsuleDokanExec(void)
{
    int j;
    int i;
    int swapPlayer;
    int masuIdSwap;
    int masuId;
    float t;
    float scale;
    
    HuVecF mdlPos[2];
    HuVecF playerPos;
    HuVecF playerPosSwap;
    
    int mdlId[2];
    int playerId[2];
    int playerMotId[2];
    
    if(MBPlayerAliveGet(NULL) >= 3) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_DOKAN_SWAP_ROULETTE, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        if(MBRouletteCreate(capsulePlayer, 3)) {
            MBRouletteWait();
        }
        swapPlayer = MBRouletteResultGet();
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_DOKAN_SWAP, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        swapPlayer = MBCapsulePlayerAliveFind(capsulePlayer);
    }
    masuId = GwPlayer[capsulePlayer].masuId;
    masuIdSwap = GwPlayer[swapPlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    MBPlayerPosGet(swapPlayer, &playerPosSwap);
    playerId[0] = capsulePlayer;
    playerId[1] = swapPlayer;
    playerMotId[0] = MBCapsulePlayerMotionCreate(playerId[0], CHARMOT_HSF_c000m1_434);
    playerMotId[1] = MBCapsulePlayerMotionCreate(playerId[1], CHARMOT_HSF_c000m1_434);
    if(masuId == masuIdSwap) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_DOKAN_NULL, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        HuPrcEnd();
    }
    for(i=0; i<2; i++) {
        mdlId[i] = MBCapsuleModelCreate(CAPSULE_HSF_dokan, NULL, TRUE, 10, FALSE);
        MBPlayerPosGet(playerId[i], &mdlPos[i]);
        MBModelDispSet(mdlId[i], FALSE);
    }
    MBAudFXPlay(MSM_SE_BOARD_39);
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        for(j=0; j<2; j++) {
            MBModelPosSet(mdlId[j], mdlPos[j].x, mdlPos[j].y, mdlPos[j].z);
            MBModelScaleSet(mdlId[j], HuSin(t*180)+(t*1.2f), HuSin(t*180)+t, HuSin(t*180)+(t*1.2f));
            MBModelDispSet(mdlId[j], TRUE);
            MBPlayerPosSet(playerId[j], mdlPos[j].x, mdlPos[j].y+((HuSin(t*180)+t)*100), mdlPos[j].z);
        }
        HuPrcVSleep();
    }
    for(i=0; i<2; i++) {
        if(playerMotId[i] != MB_MOT_NONE) {
            MBPlayerMotionNoShiftSet(playerId[i], playerMotId[i], 0, 8, HU3D_MOTATTR_NONE);
        }
        omVibrate(playerId[i], 12, 6, 6);
    }
    MBAudFXPlay(MSM_SE_BOARD_40);
    for(i=0; i<30.0f; i++) {
        t = 1-(i/30.0f);
        for(j=0; j<2; j++) {
            MBPlayerPosSet(playerId[j], mdlPos[j].x, mdlPos[j].y+(t*100), mdlPos[j].z);
            scale = (t*0.5f)+0.5f;
            MBPlayerScaleSet(playerId[j], scale, scale, scale);
        }
        HuPrcVSleep();
    }
    scale = 0;
    MBPlayerScaleSet(playerId[0], scale, scale, scale);
    MBPlayerScaleSet(playerId[1], scale, scale, scale);
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        for(j=0; j<2; j++) {
            MBModelPosSet(mdlId[j], mdlPos[j].x, mdlPos[j].y, mdlPos[j].z);
            MBModelScaleSet(mdlId[j], 1.2f, 1-t, 1.2f);
        }
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId[0], FALSE);
    MBModelDispSet(mdlId[1], FALSE);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBPlayerPosSetV(playerId[0], &playerPosSwap);
    GwPlayer[playerId[0]].masuId = masuIdSwap;
    MBPlayerPosSetV(playerId[1], &playerPos);
    GwPlayer[playerId[1]].masuId = masuId;
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(masuIdSwap, NULL, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBStarMasuDispSet(masuIdSwap, FALSE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    WipeWait();
    MBAudFXPlay(MSM_SE_BOARD_39);
    MBModelDispSet(mdlId[0], TRUE);
    MBModelDispSet(mdlId[1], TRUE);
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        for(j=0; j<2; j++) {
            MBModelScaleSet(mdlId[j], HuSin(t*180)+(t*1.2f), HuSin(t*180)+t, HuSin(t*180)+(t*1.2f));
        }
        HuPrcVSleep();
    }
    for(i=0; i<2; i++) {
        if(playerMotId[i] != MB_MOT_NONE) {
            MBPlayerMotionNoSet(playerId[i], playerMotId[i], HU3D_MOTATTR_NONE);
            MBPlayerMotionSpeedSet(playerId[i], -0.75f);
            MBPlayerMotionTimeSet(playerId[i], MBPlayerMotionMaxTimeGet(playerId[i]));
        }
    }
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        for(j=0; j<2; j++) {
            MBPlayerPosSet(playerId[j], mdlPos[j^1].x, mdlPos[j^1].y+(t*100), mdlPos[j^1].z);
            MBPlayerRotSet(playerId[j], 0, 0, 0);
            scale = (t*0.5f)+0.5f;
            MBPlayerScaleSet(playerId[j], scale, scale, scale);
        }
        HuPrcVSleep();
    }
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        for(j=0; j<2; j++) {
            MBModelPosSet(mdlId[j], mdlPos[j].x, mdlPos[j].y, mdlPos[j].z);
            MBModelScaleSet(mdlId[j], 1.2f, 1-t, 1.2f);
            MBPlayerPosSet(playerId[j], mdlPos[j^1].x, mdlPos[j^1].y+((1-t)*100), mdlPos[j^1].z);
        }
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId[0], FALSE);
    MBModelDispSet(mdlId[1], FALSE);
    for(j=0; j<2; j++) {
        MBCapsulePlayerMotSet(playerId[j], MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    }
    HuPrcEnd();
}

void MBCapsuleDokanDestroy(void)
{
    
}

void MBCapsuleJangoExec(void)
{
    int i; //r31
    int mdlId; //r30
    int masuIdSwap; //r26
    BOOL colF; //r24
    MBCAMERA *cameraP; //r23
    int masuId; //r20
    int swapPlayer; //r18
    
    float t; //f31
    float yOfs; //f28
    float baseYOfs; //f27
    
    
    
    Mtx hookMtx; //sp+0xD4
    HuVecF playerPos; //sp+0xC8
    HuVecF masuPos; //sp+0xBC
    HuVecF startPos; //sp+0xB0
    HuVecF ctrlPos; //sp+0xA4
    HuVecF mdlPos; //sp+0x98
    HuVecF mdlRotEnd; //sp+0x8C
    HuVecF mdlRot; //sp+0x80
    HuVecF hookPos; //sp+0x74
    HuVecF ofs; //sp+0x68
    HuVecF norm; //sp+0x5C
    HuVecF bezierA; //sp+0x50
    HuVecF colPosStart; //sp+0x44
    HuVecF colPosOut; //sp+0x38
    HuVecF bezierB; //sp+0x2C
    HuVecF colPosEnd; //sp+0x20
    int jangoMotFile[3]; //sp+0x14
    int playerMot[1]; //sp+0x10
    
    colF = FALSE;
    playerMot[0] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_422);
    jangoMotFile[0] = CAPSULECHAR2_HSF_jango_fly;
    jangoMotFile[1] = CAPSULECHAR2_HSF_jango_fly_up;
    jangoMotFile[2] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR2_HSF_jango, jangoMotFile, TRUE, 10, FALSE); 
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelScaleSet(mdlId, 0.5f, 0.5f, 0.5f);
    MBModelDispSet(mdlId, FALSE);
    HuPrcVSleep();
    baseYOfs = 150;
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    ofs.x = 0;
    ofs.y = 100;
    ofs.z = 0;
    MBCameraMasuViewSet(masuId, NULL, &ofs, -1, -1, 21);
    MBModelDispSet(mdlId, TRUE);
    cameraP = MBCameraGet();
    startPos.x = playerPos.x;
    startPos.y = cameraP->pos.y+500;
    startPos.z = cameraP->pos.z-500;
    ctrlPos.x = playerPos.x;
    ctrlPos.y = playerPos.y+150;
    ctrlPos.z = playerPos.z+300;
    ofs.x = playerPos.x;
    ofs.y = playerPos.y+150;
    ofs.z = playerPos.z;
    mdlRot.x = 0;
    mdlRot.y = 180;
    mdlRot.z = 0;
    MBAudFXPlay(MSM_SE_BOARD_41);
    capsuleObj = NULL;
    Hu3DMotionTimingHookSet(MBModelIdGet(mdlId), MBCapsuleTimingHookCreate);
    bezierA = ofs;
    bezierB.x = startPos.x;
    bezierB.y = playerPos.y;
    bezierB.z = startPos.z+300;
    colPosStart = ofs;
    colF = FALSE;
    for(i=0; i<120.0f; i++) {
        t = i/120.0f;
        MBCapsuleBezierGetV(t, (float *)&startPos, (float *)&ctrlPos, (float *)&ofs, (float *)&mdlPos);
        MBCapsuleBezierNormGetV(t, (float *)&startPos, (float *)&ctrlPos, (float *)&ofs, (float *)&norm);
        if(t < 0.9f) {
            mdlRotEnd.x = HuAtan(norm.y, HuMagXZVecF(&norm));
            mdlRotEnd.y = HuAtan(norm.x, norm.z);
        } else {
            mdlRotEnd.x = 0;
            mdlRotEnd.y = 180;
        }
        mdlRot.x = MBCapsuleAngleLerp(mdlRotEnd.x, mdlRot.x, 2.5f);
        mdlRot.y = MBCapsuleAngleLerp(mdlRotEnd.y, mdlRot.y, 2.5f);
        mdlRot.z = 0;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, -mdlRot.x, mdlRot.y, mdlRot.z);
        if(!colF && i < 90.0f) {
            t = i/90.0f;
            MBCapsuleBezierGetV(t, (float *)&bezierA, (float *)&bezierB, (float *)&startPos, (float *)&colPosEnd);
            if(MBCapsuleColCheck(&colPosStart, &colPosEnd, &colPosOut) || MBCapsulePlayerMasuCheck(capsulePlayer, &colPosStart, &colPosEnd, &colPosOut)) {
                colF = TRUE;
            }
            colPosStart = colPosEnd;
        }
        HuPrcVSleep();
    }
    mdlPos = ofs;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    for(i=0; i<45.0f; i++) {
        mdlRot.x = MBCapsuleAngleLerp(0, mdlRot.x, 2.5f);
        mdlRot.y = MBCapsuleAngleLerp(0, mdlRot.y, 5.0f);
        mdlRot.z = MBCapsuleAngleLerp(0, mdlRot.z, 2.5f);
        MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
        HuPrcVSleep();
    }
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    if(MBPlayerAliveGet(NULL) >= 3) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_JANGO_SWAP_ROULETTE, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        if(MBRouletteCreate(capsulePlayer, 3)) {
            MBRouletteWait();
        }
        swapPlayer = MBRouletteResultGet();
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_JANGO_SWAP, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        swapPlayer = MBCapsulePlayerAliveFind(capsulePlayer);
    }
    masuIdSwap = GwPlayer[swapPlayer].masuId;
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdSwap, &masuPos);
    if(masuIdSwap == masuId) {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_JANGO_NULL, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            yOfs = baseYOfs+(t*(750-baseYOfs));
            MBModelPosSet(mdlId, playerPos.x, playerPos.y+yOfs, playerPos.z);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
    } else {
        bezierA = mdlPos;
        ctrlPos.x = startPos.x;
        ctrlPos.y = playerPos.y;
        ctrlPos.z = startPos.z+300;
        MBMotionShiftSet(mdlId, 2, 0, 30, HU3D_MOTATTR_LOOP);
        omVibrate(capsulePlayer, 12, 6, 6);
        MBPlayerMotionNoShiftSet(capsulePlayer, playerMot[0], 0, 8, HU3D_MOTATTR_LOOP);
        MBPlayerPosGet(capsulePlayer, &hookPos);
        for(i=0; i<30.0f; i++) {
            t = i/30.0f;
            Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_oya", hookMtx);
            hookMtx[1][3] -= 120;
            ofs.x = hookPos.x+(t*(hookMtx[0][3]-hookPos.x));
            ofs.y = hookPos.y+(t*(hookMtx[1][3]-hookPos.y));
            ofs.z = hookPos.z+(t*(hookMtx[2][3]-hookPos.z));
            MBPlayerPosSetV(capsulePlayer, &ofs);
            HuPrcVSleep();
        }
        if(!colF) {
            for(i=0; i<90.0f; i++) {
                t = i/90.0f;
                MBCapsuleBezierGetV(t, (float *)&bezierA, (float *)&ctrlPos, (float *)&startPos, (float *)&mdlPos);
                MBCapsuleBezierNormGetV(t, (float *)&bezierA, (float *)&ctrlPos, (float *)&startPos, (float *)&norm);
                if(t < 0.9f) {
                    mdlRotEnd.x = HuAtan(norm.y, HuMagXZVecF(&norm));
                    mdlRotEnd.y = HuAtan(norm.x, norm.z);
                } else {
                    mdlRotEnd.x = 0;
                    mdlRotEnd.y = 180;
                }
                mdlRot.x = MBCapsuleAngleLerp(mdlRotEnd.x, mdlRot.x, 2.5f);
                mdlRot.y = MBCapsuleAngleLerp(mdlRotEnd.y, mdlRot.y, 2.5f);
                mdlRot.z = 0;
                MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                MBModelRotSet(mdlId, -mdlRot.x, mdlRot.y, mdlRot.z);
                Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_oya", hookMtx);
                hookMtx[1][3] -= 120;
                hookPos.x = hookMtx[0][3];
                hookPos.y = hookMtx[1][3];
                hookPos.z = hookMtx[2][3];
                MBPlayerPosSetV(capsulePlayer, &hookPos);
                HuPrcVSleep();
            }
        } else {
            for(i=0; i<90.0f; i++) {
                t = i/90.0f;
                mdlPos.y += 20.0f;
                MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_oya", hookMtx);
                hookMtx[1][3] -= 120;
                hookPos.x = hookMtx[0][3];
                hookPos.y = hookMtx[1][3];
                hookPos.z = hookMtx[2][3];
                MBPlayerPosSetV(capsulePlayer, &hookPos);
                HuPrcVSleep();
            }
        }
        MBModelDispSet(mdlId, FALSE);
        MBPlayerDispSet(capsulePlayer, FALSE);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBPlayerPosFixEvent(capsulePlayer, masuIdSwap, TRUE);
        GwPlayer[capsulePlayer].masuId = masuIdSwap;
        yOfs = 750-baseYOfs;
        MBPlayerPosSet(capsulePlayer, masuPos.x, masuPos.y+yOfs, masuPos.z);
        MBPlayerRotSet(capsulePlayer, 0, 0, 0);
        MBStarMasuDispSet(masuIdSwap, FALSE);
        MBCameraSkipSet(FALSE);
        ofs.x = 0;
        ofs.y = 100;
        ofs.z = 0;
        MBCameraMasuViewSet(masuIdSwap, NULL, &ofs, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
        WipeWait();
        MBModelDispSet(mdlId, TRUE);
        MBPlayerDispSet(capsulePlayer, TRUE);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            yOfs = baseYOfs+((1-t)*(750-baseYOfs));
            MBModelPosSet(mdlId, masuPos.x, masuPos.y+yOfs, masuPos.z);
            MBModelRotSet(mdlId, 0, 0, 0);
            Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_oya", hookMtx);
            hookMtx[1][3] -= 120;
            hookPos.x = hookMtx[0][3];
            hookPos.y = hookMtx[1][3];
            hookPos.z = hookMtx[2][3];
            MBPlayerPosSetV(capsulePlayer, &hookPos);
            HuPrcVSleep();
        }
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        hookPos = masuPos;
        for(i=0; i<8; i++) {
            t = (1-(i/7.0f));
            Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_oya", hookMtx);
            hookMtx[1][3] -= 120;
            ofs.x = hookPos.x+(t*(hookMtx[0][3]-hookPos.x));
            ofs.y = hookPos.y+(t*(hookMtx[1][3]-hookPos.y));
            ofs.z = hookPos.z+(t*(hookMtx[2][3]-hookPos.z));
            MBPlayerPosSetV(capsulePlayer, &ofs);
            HuPrcVSleep();
        }
        while(1) {
            HuPrcVSleep();
            if(MBMotionShiftIDGet(mdlId) == HU3D_MOTID_NONE) {
                break;
            }
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            yOfs = baseYOfs+(t*(750-baseYOfs));
            MBModelPosSet(mdlId, masuPos.x, masuPos.y+yOfs, masuPos.z);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
    }
    HuPrcEnd();
}

void MBCapsuleJangoDestroy(void)
{
    
}

void MBCapsuleBobleExec(void)
{
    int i; //r31
    int j; //r30
    int masuId; //r25
    int startMasuId; //r22
    int playerMdlId; //r19
    int capsuleMdlId; //r18
    int fallMotId2; //r17
    
    float t; //f31
    float angle; //f30
    float power; //f29
    float loseAngleX; //f28
    float baseAngle; //f27
    float bounceHeight; //f26
    float bounceSpeed; //f25
    float loseAngleY; //f24
    float moveSpeed; //f23
    
    Mtx hookMtx; //sp+0x114
    HuVecF effPos; //sp+0x108
    HuVecF effVel; //sp+0xFC
    HuVecF masuPos; //sp+0xF0
    HuVecF rotEnd; //sp+0xE4
    HuVecF rot; //sp+0xD8
    HuVecF pos; //sp+0xCC
    HuVecF nextPos; //sp+0xC0
    HuVecF vel; //sp+0xB4
    int mdlId[3]; //sp+0xA8
    HU3DMODEL *modelP; //sp+0x74
    int dist; //sp+0x70
    int playerMdl; //sp+0x6C
    int charNo; //sp+0x68
    int stepNum; //sp+0x64
    int masuType; //sp+0x60
    u32 masuFlag; //sp+0x5C
    int fallMotId; //sp+0x58
    int capsuleNo; //sp+0x54
    int masuNum; //sp+0x50
    int temp; //sp+0x4C
    BOOL removeCapsuleF; //sp+0x48
    GXColor color; //sp+0x44
    
    temp = 0;
    removeCapsuleF = FALSE;
    MBCapsuleExplodeEffCreate(0);
    HuPrcVSleep();
    MBCapsuleLoseEffCreate();
    HuPrcVSleep();
    MBPlayerPosGet(capsulePlayer, &effPos);
    startMasuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerMoveInit();
    HuPrcVSleep();
    fallMotId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_321);
    fallMotId2 = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_321);
    capsuleMdlId = MBCapsuleModelCreate(CAPSULE_HSF_capsuleGreen, NULL, FALSE, 10, FALSE);
    MBModelPosSetV(capsuleMdlId, &effPos);
    MBModelDispSet(capsuleMdlId, FALSE);
    pos.x = 0;
    pos.y = 100;
    pos.z = 0;
    MBCameraSkipSet(TRUE);
    MBCameraModelViewSet(capsuleMdlId, NULL, &pos, -1, -1, 21);
    for(i=0; i<3; i++) {
        angle = i*120.0f;
        pos.x = effPos.x+(HuCos(angle)*100);
        pos.y = effPos.y+200;
        pos.z = effPos.z+(HuSin(angle)*100);
        mdlId[i] = MBCapsuleModelCreate(CAPSULECHAR1_HSF_bubble, NULL, FALSE, 10, FALSE);
        MBModelPosSet(mdlId[i], pos.x,  pos.y, pos.z);
        MBModelScaleSet(mdlId[i], 0, 0, 0);
        MBModelAttrSet(mdlId[i], HU3D_MOTATTR_LOOP);
        MBModelLayerSet(mdlId[i], 2);
        MBModelDispSet(mdlId[i], FALSE);
        HuPrcVSleep();
    }
    for(j=0; j<3; j++) {
        MBAudFXPlay(MSM_SE_BOARD_43);
        for(i=0; i<30.0f; i++) {
            t = i/30.0f;
            MBModelScaleSet(mdlId[j], t, t, t);
            MBModelDispSet(mdlId[j], TRUE);
            HuPrcVSleep();
        }
    }
    MBAudFXPlay(MSM_SE_BOARD_44);
    for(j=0, baseAngle=0; j<120.0f; j++) {
        t = j/180.0f;
        for(i=0; i<3; i++) {
            baseAngle += HuSin(t*180.0f)*5;
            angle = baseAngle+(i*120.0f);
            pos.x = effPos.x+(HuCos(angle)*100.0);
            pos.y = effPos.y+200;
            pos.z = effPos.z+(HuSin(angle)*100.0);
            MBModelPosSet(mdlId[i], pos.x, pos.y, pos.z);
        }
        HuPrcVSleep();
    }
    for(j=0; j<15.0f; j++) {
        t = j/15.0f;
        for(i=0; i<3; i++) {
            angle = baseAngle+(i*120.0f);
            pos.x = effPos.x+(HuCos(angle)*100.0);
            pos.y = effPos.y+200;
            pos.z = effPos.z+(HuSin(angle)*100.0);
            nextPos.x = effPos.x;
            nextPos.y = effPos.y+66;
            nextPos.z = effPos.z;
            VECSubtract(&nextPos, &pos, &vel);
            VECScale(&vel, &vel, t);
            VECAdd(&pos, &vel, &pos);
            MBModelPosSet(mdlId[i], pos.x, pos.y, pos.z);
        }
        HuPrcVSleep();
    }
    for(i=0; i<3; i++) {
        MBModelDispSet(mdlId[i], FALSE);
    }
    MBAudFXPlay(MSM_SE_BOARD_20);
    CharFXPlay(GwPlayer[capsulePlayer].charNo, CHARVOICEID(3));
    omVibrate(capsulePlayer, 12, 4, 2);
    dist = 10;
    masuNum = 0;
    do {
        startMasuId = GwPlayer[capsulePlayer].masuId;
        masuId = MBCapsuleMasuNextRegularGet(startMasuId, NULL);
        MBPlayerPosGet(capsulePlayer, &masuPos);
        if(masuId <= 0) {
            break;
        }
        if(MBCapsuleMasuNextGet(masuId, NULL) <= 0) {
            break;
        }
        MBPlayerPosFixEvent(capsulePlayer, masuId, FALSE);
        MBPlayerPosGet(capsulePlayer, &effPos);
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
        VECSubtract(&masuPos, &effPos, &pos);
        moveSpeed = VECMag(&pos);
        stepNum = moveSpeed/22.5f;
        rotEnd.x = 0;
        rotEnd.y = HuAtan(pos.x, pos.z);
        rotEnd.z = 0;
        bounceHeight = 0;
        bounceSpeed = 0;
        if((MBMasuFlagGet(MASU_LAYER_DEFAULT, startMasuId) & MASU_FLAG_JUMPFROM) && (MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId) & MASU_FLAG_JUMPTO)) {
            bounceHeight = 200;
            MBPlayerMotionNoShiftSet(capsulePlayer, fallMotId2, 0, 8, HU3D_MOTATTR_LOOP);
        } else if((MBMasuFlagGet(MASU_LAYER_DEFAULT, startMasuId) & MASU_FLAG_CLIMBFROM) && (MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId) & MASU_FLAG_CLIMBTO)) {
            if(masuPos.y > effPos.y) {
                bounceSpeed = 50;
            }
            MBPlayerMotionNoShiftSet(capsulePlayer, fallMotId2, 0, 8, HU3D_MOTATTR_LOOP);
        } else {
            bounceHeight = 200;
            MBPlayerMotionNoShiftSet(capsulePlayer, fallMotId2, 0, 8, HU3D_MOTATTR_LOOP);
        }
        for(j=0; j<stepNum; j++) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, startMasuId, &effPos);
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
            t = (float)j/(float)stepNum;
            VECSubtract(&masuPos, &effPos, &pos);
            VECScale(&pos, &pos, t);
            VECAdd(&effPos, &pos, &pos);
            MBModelPosSetV(capsuleMdlId, &pos);
            pos.x += bounceSpeed*HuSin(t*720)*HuSin(t*180);
            pos.z += bounceSpeed*HuCos(t*720)*HuSin(t*180);
            pos.y += bounceHeight*HuSin(t*180);
            MBPlayerPosSetV(capsulePlayer, &pos);
            MBPlayerRotGet(capsulePlayer, &rot);
            rot.x = MBCapsuleAngleLerp(rotEnd.x, rot.x, 5.0f);
            if(bounceSpeed <= 0) {
                rot.y = rotEnd.y;
            } else {
                rot.y = rotEnd.y+(t*720);
            }
            MBPlayerRotSetV(capsulePlayer, &rot);
            playerMdl = MBPlayerModelGet(capsulePlayer);
            playerMdlId = MBModelIdGet(playerMdl);
            modelP = &Hu3DData[playerMdlId];
            charNo = GwPlayerConf[capsulePlayer].charNo;
            Hu3DModelObjMtxGet(playerMdlId, CharModelItemHookGet(charNo, CHAR_MODEL1, 4), hookMtx);
            effPos.x = hookMtx[0][3];
            effPos.y = hookMtx[1][3];
            effPos.z = hookMtx[2][3];
            effVel.x = 0;
            effVel.y = 2;
            effVel.z = 0;
            power = MBCapsuleEffRandF();
            color.r = 32+(power*32);
            color.g = 32+(power*32);
            color.b = 32+(power*32);
            color.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleExplodeEffAdd(0, effPos, effVel, ((MBCapsuleEffRandF()*0.5f)+1)*100, 2.5f, (MBCapsuleEffRandF()*0.66f)+0.33f, color);
            HuPrcVSleep();
        }
        if(bounceHeight > 0) {
            MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_JUMPEND, 0, 2,HU3D_MOTATTR_NONE);
            HuPrcVSleep();
        }
        GwPlayer[capsulePlayer].masuIdPrev = startMasuId;
        GwPlayer[capsulePlayer].masuId = masuId;
        masuType = MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId);
        masuFlag = MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId);
        if(masuType != MASU_TYPE_NONE && masuType != MASU_TYPE_STAR && !(masuFlag & (MASU_FLAG_START|MASU_FLAG_BLOCKR))) {
            dist--;
        }
        masuNum++;
        if(MBPlayerCapsuleNumGet(capsulePlayer) > 0) {
            capsuleNo = MBPlayerCapsuleGet(capsulePlayer, 0);
            if(capsuleNo != CAPSULE_NULL) {
                t = ((MBCapsuleEffRandF()*0.1f)+1.0f)*65;
                loseAngleY = (MBCapsuleEffRandF()*15)+75;
                loseAngleX = MBCapsuleEffRandF()*360;
                MBPlayerPosGet(capsulePlayer, &pos);
                pos.x += 100*HuSin(loseAngleX);
                pos.z += 100*HuCos(loseAngleX);
                effVel.x = HuSin(loseAngleX)*HuCos(loseAngleY)*t;
                effVel.z = HuCos(loseAngleX)*HuCos(loseAngleY)*t;
                effVel.y = HuSin(loseAngleY)*t;
                MBCapsuleLoseEffAdd(&effPos, &effVel, 1, 60, capsuleNo);
                MBPlayerCapsuleRemove(capsulePlayer, 0);
                removeCapsuleF = TRUE;
            }
        }
    } while(dist > 0);
    if(MBPlayerCapsuleNumGet(capsulePlayer) > 0) {
        MBCapsuleLoseEffAddMulti(capsulePlayer, MBPlayerCapsuleMaxGet(), 50);
        for(j=0; j<MBPlayerCapsuleMaxGet(); j++){ 
            MBPlayerCapsuleRemove(capsulePlayer, 0);
        }
        removeCapsuleF = TRUE;
    }
    if(masuNum  == 0) {
        MBPlayerPosGet(capsulePlayer, &masuPos);
        MBPlayerMotionNoShiftSet(capsulePlayer, fallMotId2, 0, 8, HU3D_MOTATTR_LOOP);
        for(j=0; j<36.0f; j++) {
            pos = masuPos;
            pos.y += (HuSin((j/36.0f)*180.0f)*100)*3;
            MBPlayerPosSetV(capsulePlayer, &pos);
            playerMdl = MBPlayerModelGet(capsulePlayer);
            playerMdlId = MBModelIdGet(playerMdl);
            modelP = &Hu3DData[playerMdlId];
            charNo = GwPlayerConf[capsulePlayer].charNo;
            Hu3DModelObjMtxGet(playerMdlId, CharModelItemHookGet(charNo, CHAR_MODEL1, 4), hookMtx);
            effPos.x = hookMtx[0][3];
            effPos.y = hookMtx[1][3];
            effPos.z = hookMtx[2][3];
            effVel.x = 0;
            effVel.y = 2;
            effVel.z = 0;
            power = MBCapsuleEffRandF();
            color.r = 32+(power*32);
            color.g = 32+(power*32);
            color.b = 32+(power*32);
            color.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleExplodeEffAdd(0, effPos, effVel, ((MBCapsuleEffRandF()*0.5f)+1)*100, 2.5f, (MBCapsuleEffRandF()*0.66f)+0.33f, color);
            HuPrcVSleep();
        }
    }
    MBCameraMotionWait();
    pos.x = 0;
    pos.y = 100;
    pos.z = 0;
    MBCameraModelViewSet(MBPlayerModelGet(capsulePlayer), NULL, &pos, -1, -1, 21);
    MBPlayerPosSetV(capsulePlayer, &masuPos);
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_DIZZY, HU3D_MOTATTR_LOOP, TRUE);
    HuPrcSleep(60);
    while(1) {
        HuPrcVSleep();
        if(MBCapsuleLoseEffNumGet() == 0) {
            break;
        }
    }
    if(removeCapsuleF) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_BOBLE_END, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBCameraMotionWait();
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    MBPlayerMoveObjKill(capsulePlayer);
    HuPrcEnd();
}

void MBCapsuleBobleDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleLoseEffKill();
}

static void CapsuleHanachanEffExec(OMOBJ *obj);

void MBCapsuleHanachanExec(void)
{
    int i; //r31
    int mdlId; //r30
    int flowerMdlId; //r28
    OMOBJ *obj; //r19
    
    float t; //f31
    float angle; //f30
    float effAngle; //f29
    float power; //f28
    float radius; //f27
    float effRadius; //f26
    float hookYOfs; //f25
    float rotX; //f24
    float glowSpeed; //f23
    float hanachanDist; //f22
    
    Mtx hookMtx; //sp+0x1F8
    int motFile[8]; //sp+0x1D8
    HuVecF playerPos; //sp+0x1CC
    HuVecF masuPos; //sp+0x1C0
    HuVecF mdlPos; //sp+0x1B4
    HuVecF endPos; //sp+0x1A8
    HuVecF ctrlPos; //sp+0x19C
    HuVecF startPos; //sp+0x190
    HuVecF tempVec; //sp+0x184
    HuVecF ringDir; //sp+0x178
    HuVecF explodeRot; //sp+0x16C
    HuVecF rayRot; //sp+0x160
    HuVecF rayMoveDir; //sp+0x154
    HuVecF glowVel; //sp+0x148
    int hangMotId; //sp+0xC0
    int fallMotId; //sp+0xBC
    int focusMdlId; //sp+0xB8
    int starNo; //sp+0xB4
    int seNo1; //sp+0xB0
    int seNo2; //sp+0xAC
    int masuId; //sp+0xA8
    int starMasuId; //sp+0xA4
    int choice; //sp+0xA0
    BOOL starCancelF; //sp+0x9C
    GXColor effColor; //sp+0x98
    MBCAMERA *cameraP; //sp+0x94
    GXColor rayColor; //sp+0x90
    
    static HuVecF playerOfs[CHARNO_MAX] = {
        { 0, -130, 30.000002 },
        { 0, -135, 30.000002 },
        { -20, -120.00001, 80 },
        { 0, -130, 30.000002 },
        { 0, -120.00001, 40 },
        { -20, -130, 80 },
        { 0, -140, 30.000002 },
        { 0, -100, 50 },
        { 0, -104.99999, 70 },
        { -10, -130, 30.000002 },
        { -10, -130, 30.000002 },
        { -10, -130, 30.000002 },
        { -10, -130, 30.000002 }
    };
    static HuVecF ringVel[2] = {
        { 1.2f, 2.0f, 150 },
        { 1.2f, 2.0f, 700 },
    };
    static GXColor ringCol[2] = {
        { 255, 255, 64, 128 },
        { 255, 128, 255, 128 },
    };
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    MBCapsuleExplodeEffCreate(0);
    HuPrcVSleep();
    MBCapsuleGlowEffCreate();
    HuPrcVSleep();
    MBCapsuleHanachanRingCreate();
    HuPrcVSleep();
    MBCapsuleRingEffCreate();
    HuPrcVSleep();
    MBCapsuleRayEffCreate();
    HuPrcVSleep();
    fallMotId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_354);
    hangMotId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_422);
    motFile[0] = CAPSULECHAR1_HSF_hanachan_idle;
    motFile[1] = CAPSULECHAR1_HSF_hanachan_lift;
    motFile[2] = CAPSULECHAR1_HSF_hanachan_jump;
    motFile[3] = CAPSULECHAR1_HSF_hanachan_fly;
    motFile[4] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_hanachan, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    HuPrcVSleep();
    flowerMdlId = MBCapsuleModelCreate(CAPSULE_HSF_hanachanFlower, NULL, FALSE, 10, FALSE);
    MBModelDispSet(flowerMdlId, FALSE);
    HuPrcVSleep();
    focusMdlId = MBCapsuleModelCreate(CAPSULE_HSF_kinoko, NULL, FALSE, 10, FALSE);
    MBModelDispSet(focusMdlId, FALSE);
    HuPrcVSleep();
    MBPlayerPosGet(capsulePlayer, &playerPos);
    masuId = GwPlayer[capsulePlayer].masuId;
    for(i=1; i<MASU_MAX; i++) {
        if(MBMasuTypeGet(MASU_LAYER_DEFAULT, i) == MASU_TYPE_STAR) {
            starMasuId = i;
            break;
        }
    }
    if(i >= MASU_MAX) {
        HuPrcEnd();
        return;
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, starMasuId, &masuPos);
    MBModelPosSetV(focusMdlId, &playerPos);
    tempVec.x = 0;
    tempVec.y = 100;
    tempVec.z = 0;
    MBCameraModelViewSet(focusMdlId, NULL, &tempVec, -1, -1, 21);
    MBCameraMotionWait();
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_HANACHAN, 127, 0);
    MBModelPosSetV(flowerMdlId, &playerPos);
    MBModelDispSet(flowerMdlId, TRUE);
    MBMotionTimeSet(flowerMdlId, 0);
    MBAudFXPlay(MSM_SE_BOARD_45);
    if(fallMotId != MB_MOT_NONE) {
        MBPlayerMotionNoShiftSet(capsulePlayer, fallMotId, 0, 8, HU3D_MOTATTR_LOOP);
    }
    CharFXPlay(GwPlayer[capsulePlayer].charNo, CHARVOICEID(9));
    MBAudFXPlay(MSM_SE_BOARD_105);
    do {
        HuPrcVSleep();
        power = MBMotionTimeGet(flowerMdlId);
        effRadius = MBMotionMaxTimeGet(flowerMdlId);
        t = power/effRadius;
        if(t >= 1) {
            t = 1;
            MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        }
        Hu3DModelObjMtxGet(MBModelIdGet(flowerMdlId), "set", hookMtx);
        tempVec.x = hookMtx[0][3]+(t*125);
        tempVec.y = hookMtx[1][3];
        tempVec.z = hookMtx[2][3];
        MBPlayerPosSetV(capsulePlayer, &tempVec);
        MBModelPosSetV(focusMdlId, &tempVec);
    } while(!MBMotionEndCheck(flowerMdlId));
    for(i=0; i<30.0f; i++) {
        HuPrcVSleep();
    }
    MBPlayerPosGet(capsulePlayer, &playerPos);
    mdlPos.x = playerPos.x-250;
    mdlPos.y = playerPos.y;
    mdlPos.z = playerPos.z;
    VECSubtract(&playerPos, &mdlPos, &tempVec);
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y+700, mdlPos.z);
    MBModelRotSet(mdlId, 0, HuAtan(tempVec.x, tempVec.z), 0);
    MBModelDispSet(mdlId, TRUE);
    tempVec.x = mdlPos.x+50;
    tempVec.y = mdlPos.y+100;
    tempVec.z = mdlPos.z;
    explodeRot.x = explodeRot.y = explodeRot.z = 0;
    MBCapsuleDustExplodeAdd(0, tempVec);
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        tempVec = mdlPos;
        tempVec.y += HuCos(t*90)*130;
        MBModelPosSet(mdlId, tempVec.x, tempVec.y, tempVec.z);
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_BOARD_46);
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    MBCapsuleModelMotSet(mdlId, 2, HU3D_MOTATTR_NONE, TRUE);
    VECSubtract(&mdlPos, &playerPos, &tempVec);
    MBPlayerRotateStart(capsulePlayer, HuAtan(tempVec.x, tempVec.z), 15);
    MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
    MBCapsuleModelMotSet(mdlId, 4, HU3D_MOTATTR_LOOP, TRUE);
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    MBAudFXPlay(MSM_SE_GUIDE_41);
    MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX01_HANACHAN_STAR, HUWIN_SPEAKER_HANACHAN, 0);
    MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
    if(GwPlayer[capsulePlayer].comF) {
        if(MBPlayerCoinGet(capsulePlayer) >= 20) {
            MBCapsuleChoiceSet(0);
        } else {
            MBCapsuleChoiceSet(1);
        }
    }
    MBTopWinWait();
    choice = MBWinLastChoiceGet();
    if(choice == 0) {
        starCancelF = FALSE;
        if(MBPlayerStarGet(capsulePlayer) >= 999) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_42);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_HANACHAN_STAR_MAX, HUWIN_SPEAKER_HANACHAN);
            MBTopWinWait();
            MBTopWinKill();
            starCancelF = TRUE;
        } else if(MBPlayerCoinGet(capsulePlayer) < 20) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_42);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_HANACHAN_COIN_LOW, HUWIN_SPEAKER_HANACHAN);
            MBTopWinWait();
            MBTopWinKill();
            starCancelF = TRUE;
        }
    } else {
        starCancelF = TRUE;
    }
    if(starCancelF) {
        if(choice != 0) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_HANACHAN_STAR_MES, HUWIN_SPEAKER_HANACHAN);
            MBTopWinWait();
            MBTopWinKill();
        }
        MBAudFXPlay(MSM_SE_BOARD_35);
        MBAudFXPlay(MSM_SE_BOARD_10);
        for(i=0; i<64; i++) {
            effAngle = i*5.625f;
            power = (MBCapsuleEffRandF()*100)*0.33f;
            tempVec.x = mdlPos.x+(power*HuCos(effAngle));
            tempVec.y = mdlPos.y+(power*HuSin(effAngle))+50;
            tempVec.z = mdlPos.z+50;
            effRadius = ((MBCapsuleEffRandF()*0.04f)+0.005f)*100;
            explodeRot.x = effRadius*HuCos(effAngle);
            explodeRot.y = effRadius*HuSin(effAngle);
            explodeRot.z = 0;
            power = MBCapsuleEffRandF();
            effColor.r = 192+(power*63);
            effColor.g = 192+(power*63);
            effColor.b = 192+(power*63);
            effColor.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleExplodeEffLineAdd(0, tempVec, explodeRot, 
                ((MBCapsuleEffRandF()*0.5f)+1.0f)*100,
                2.5f,
                ((MBCapsuleEffRandF()*0.5f)+0.5f)*100,
                (MBCapsuleEffRandF()*0.66f)+0.33f,
                effColor);
        }
        MBModelDispSet(mdlId, FALSE);
        while(1){
            HuPrcVSleep();
            if(MBCapsuleExplodeEffCheck(0) == 0) {
                break;
            }
        }
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        HuPrcSleep(60);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        if(fallMotId != MB_MOT_NONE) {
            MBPlayerMotionNoShiftSet(capsulePlayer, fallMotId, 0, 8, HU3D_MOTATTR_LOOP);
        }
        MBMotionSpeedSet(flowerMdlId, -1);
        MBAudFXPlay(MSM_SE_BOARD_138);
        
        do {
            HuPrcVSleep();
            power = MBMotionTimeGet(flowerMdlId);
            effRadius = MBMotionMaxTimeGet(flowerMdlId);
            t = power/effRadius;
            if(t <= 0) {
                t = 0;
                MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
            }
            Hu3DModelObjMtxGet(MBModelIdGet(flowerMdlId), "set", hookMtx);
            tempVec.x = hookMtx[0][3]+(t*125);
            tempVec.y = hookMtx[1][3];
            tempVec.z = hookMtx[2][3];
            MBPlayerPosSetV(capsulePlayer, &tempVec);
            MBModelPosSetV(focusMdlId, &tempVec);
        } while(MBMotionTimeGet(flowerMdlId) > 0);
        MBCameraMotionWait();
        HuPrcSleep(12);
        MBPlayerRotateStart(capsulePlayer, 0, 15);
        while(!MBPlayerRotateCheck(capsulePlayer)) {
            HuPrcVSleep();
        }
        MBCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_ZOOMIN);
        MBCameraMotionWait();
        MBMusMainPlay();
        HuPrcEnd();
        return;
    }
    MBCoinAddDispExec(capsulePlayer, -20, TRUE);
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    seNo2 = HuAudFXPlay(MSM_SE_BOARD_125);
    for(i=0; i<64; i++) {
        tempVec.x = mdlPos.x+((0.5f-MBCapsuleEffRandF())*50);
        tempVec.y = mdlPos.y+(MBCapsuleEffRandF()*150);
        tempVec.z = mdlPos.z+((0.5f-MBCapsuleEffRandF())*50);
        ringDir.x = (MBCapsuleEffRandF()*0.7)+0.7f;
        ringDir.y = (MBCapsuleEffRandF()*0.5)+1.5;
        ringDir.z = (MBCapsuleEffRandF()*0.5)+2;
        effColor = kinokoGlowColorTbl[MBCapsuleEffRand(7)];
        MBCapsuleHanachanRingAdd(&tempVec, &ringDir, ((MBCapsuleEffRandF()*0.2f)+0.3f)*60, -1, ((MBCapsuleEffRandF()*0.2f)+0.3f)*60, effColor);
        HuPrcVSleep();
    }
    for(i=0; i<30.0f; i++) {
        HuPrcVSleep();
    }
    MBCapsuleHanachanRingClear();
    MBAudFXPlay(MSM_SE_BOARD_47);
    for(i=0; i<2; i++) {
        cameraP = MBCameraGet();
        tempVec = mdlPos;
        tempVec.y += 100;
        ringDir.x = cameraP->rot.x;
        ringDir.y = cameraP->rot.y;
        ringDir.z = cameraP->rot.z;
        MBCapsuleRingEffAdd(tempVec, ringDir, ringVel[i], 12, 15, i%3, ringCol[i]);
    }
    for(i=0; i<32; i++) {
        tempVec = mdlPos;
        tempVec.y += 100;
        rayRot.x = 0;
        rayRot.y = 0;
        rayRot.z = (i*11.25f)+((0.5f-MBCapsuleEffRandF())*10.0f);
        rayMoveDir.x = 20;
        rayMoveDir.y = ((MBCapsuleEffRandF()*0.5f)+0.7f)*500;
        rayMoveDir.z = ((MBCapsuleEffRandF()*0.4f)+0.8f)*80;
        rayColor.r = (MBCapsuleEffRandF()*64)+128;
        rayColor.g = (MBCapsuleEffRandF()*64)+128;
        rayColor.b = (MBCapsuleEffRandF()*64)+128;
        rayColor.a = (MBCapsuleEffRandF()*64)+128;
        MBCapsuleRayEffAdd(tempVec, rayRot, rayMoveDir, ((MBCapsuleEffRandF()*0.3f)+0.8f)*30, rayColor);
    }
    for(i=0; i<12.0f; i++) {
        HuPrcVSleep();
    }
    for(i=0, effAngle=0; i<128; i++){
        effAngle += (MBCapsuleEffRandF()+1.0f)*10;
        tempVec.x = mdlPos.x+((MBCapsuleEffRandF()+-0.5f)*75);
        tempVec.y = mdlPos.y+100+((MBCapsuleEffRandF()+-0.5f)*75);
        tempVec.z = mdlPos.z+50;
        glowSpeed = (MBCapsuleEffRandF()+1.0f)*5.0f;
        glowVel.x = glowSpeed*HuSin(effAngle);
        glowVel.y = glowSpeed*HuCos(effAngle);
        glowVel.z = 0;
        t = MBCapsuleEffRandF();
        effColor = kinokoGlowColorTbl[MBCapsuleEffRand(7)];
        MBCapsuleGlowEffAdd(tempVec, glowVel, 
            ((MBCapsuleEffRandF()*0.1f)+0.3f)*100,
            (MBCapsuleEffRandF()+1)*0.016666668f,
            0,
            0,
            effColor);
    }
    if(seNo2 != MSM_SENO_NONE) {
        HuAudFXStop(seNo2);
    }
    seNo2 = MSM_SENO_NONE;
    MBModelDispSet(mdlId, FALSE);
    motFile[0] = CAPSULECHAR1_HSF_hanachan_flutter_move;
    motFile[1] = CAPSULECHAR1_HSF_hanachan_flutter_hold;
    motFile[2] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_hanachan_flutter, motFile, FALSE, 10, FALSE);
    MBModelLayerSet(mdlId, 2);
    mdlPos.x = playerPos.x-200;
    mdlPos.y = playerPos.y+50;
    mdlPos.z = playerPos.z;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    VECSubtract(&playerPos, &mdlPos, &tempVec);
    MBModelRotSet(mdlId, 0, HuAtan(tempVec.x, tempVec.z), 0);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    omVibrate(capsulePlayer, 12, 4, 2);
    CharFXPlay(GwPlayer[capsulePlayer].charNo, CHARVOICEID(6));
    MBPlayerMotionNoSet(capsulePlayer, MB_PLAYER_MOT_SHOCK, HU3D_MOTATTR_NONE);
    obj = MBAddObj(32768, 0, 0, CapsuleHanachanEffExec);
    capsuleObj = obj;
    obj->work[0] = 0;
    obj->work[1] = mdlId;
    obj->work[2] = 0;
    seNo1 = MBAudFXPlay(MSM_SE_BOARD_48);
    for(i=0; i<30.0f; i++) {
        HuPrcVSleep();
    }
    MBCapsuleEffHookCall(3, mdlId, TRUE, FALSE, FALSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_HANACHAN_STAR_GOTO, HUWIN_SPEAKER_HANACHAN2);
    while(!MBTopWinDoneCheck()) {
        HuPrcVSleep();
    }
    MBTopWinKill();
    startPos = mdlPos;
    VECSubtract(&playerPos, &mdlPos, &tempVec);
    VECNormalize(&tempVec, &tempVec);
    endPos.x = playerPos.x+(50*tempVec.x);
    endPos.y = playerPos.y+100;
    endPos.z = playerPos.z+(50*tempVec.z);
    VECSubtract(&playerPos, &mdlPos, &tempVec);
    VECScale(&tempVec, &tempVec, 3);
    VECAdd(&playerPos, &tempVec, &ctrlPos);
    ctrlPos.y += 150;
    ctrlPos.z += 150;
    radius = playerOfs[GwPlayer[capsulePlayer].charNo].z;
    hookYOfs = playerOfs[GwPlayer[capsulePlayer].charNo].y;
    rotX = playerOfs[GwPlayer[capsulePlayer].charNo].x;
    for(i=0; i<90.0f; i++) {
        t = i/90.0f;
        MBCapsuleBezierGetV(t, (float *)&startPos, (float *)&ctrlPos, (float *)&endPos, (float *)&mdlPos);
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        VECSubtract(&playerPos, &mdlPos, &tempVec);
        angle = HuAtan(tempVec.x, tempVec.z);
        MBModelRotSet(mdlId, 0, angle, 0);
        HuPrcVSleep();
    }
    do {
        mdlPos.x += (0.1f*(endPos.x-mdlPos.x));
        mdlPos.y += (0.1f*(endPos.y-mdlPos.y));
        mdlPos.z += (0.1f*(endPos.z-mdlPos.z));
        angle = MBCapsuleAngleLerp(-90, angle, 1);
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, 0, angle, 0);
        HuPrcVSleep();
    } while(fabs(MBCapsuleAngleWrap(angle, -90)) > 1);
    mdlPos = endPos;
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
    if(hangMotId != MB_MOT_NONE) {
        MBPlayerMotionNoShiftSet(capsulePlayer, hangMotId, 0, 8, HU3D_MOTATTR_NONE);
    }
    MBMasuPlayerLightSet(capsulePlayer, FALSE);
    MBPlayerPosGet(capsulePlayer, &playerPos);
    for(i=0; i<8; i++) {
        t = i/7.0f;
        Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_C", hookMtx);
        hookMtx[1][3] += hookYOfs;
        hookMtx[0][3] += radius*HuSin(angle);
        hookMtx[2][3] += radius*HuCos(angle);
        tempVec.x = playerPos.x+(t*(hookMtx[0][3]-playerPos.x));
        tempVec.y = playerPos.y+(t*(hookMtx[1][3]-playerPos.y));
        tempVec.z = playerPos.z+(t*(hookMtx[2][3]-playerPos.z));
        MBPlayerPosSetV(capsulePlayer, &tempVec);
        MBPlayerRotGet(capsulePlayer, &tempVec);
        tempVec.x = t*rotX;
        MBPlayerRotSetV(capsulePlayer, &tempVec);
        HuPrcVSleep();
    }
    endPos = mdlPos;
    effAngle = angle;
    for(i=0; i<90.0f; i++) {
        t = i/90.0f;
        angle = effAngle+(t*180.0f);
        mdlPos.x = endPos.x+(HuSin(angle)*HuSin(t*90)*300);
        mdlPos.z = endPos.z+(HuCos(angle)*HuSin(t*90)*300);
        mdlPos.y = endPos.y+(t*650);
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, 0, angle, 0);
        Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_C", hookMtx);
        hookMtx[1][3] += hookYOfs;
        hookMtx[0][3] += radius*HuSin(angle);
        hookMtx[2][3] += radius*HuCos(angle);
        tempVec.x = hookMtx[0][3];
        tempVec.y = hookMtx[1][3];
        tempVec.z = hookMtx[2][3];
        MBPlayerPosSetV(capsulePlayer, &tempVec);
        MBPlayerRotSet(capsulePlayer, rotX, angle, 0);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBPlayerDispSet(capsulePlayer, FALSE);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBPlayerPosFixEvent(capsulePlayer, starMasuId, TRUE);
    GwPlayer[capsulePlayer].masuId = starMasuId;
    mdlPos = masuPos;
    mdlPos.y += 750;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    MBModelRotSet(mdlId, 0, 0, 0);
    playerPos = masuPos;
    playerPos.y += 650;
    MBModelDispSet(flowerMdlId, FALSE);
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(starMasuId, NULL, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    endPos = masuPos;
    endPos.y += 100;
    MBModelDispSet(mdlId, TRUE);
    MBPlayerDispSet(capsulePlayer, TRUE);
    for(i=0; i<150.0f; i++) {
        t = i/150.0f;
        angle = 180+(t*360);
        hanachanDist = HuSin(t*180)*450;
        mdlPos.x = endPos.x+(HuSin(angle)*hanachanDist);
        mdlPos.z = endPos.z+(HuCos(angle)*hanachanDist);
        mdlPos.y = endPos.y+((1-t)*650);
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, 0, angle+180, 0);
        playerPos.x = mdlPos.x+(50*HuSin(angle+180));
        playerPos.y = mdlPos.y-100;
        playerPos.z = mdlPos.z+(50*HuCos(angle+180));
        Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_C", hookMtx);
        hookMtx[1][3] += hookYOfs;
        hookMtx[0][3] += radius*HuSin(angle+180);
        hookMtx[2][3] += radius*HuCos(angle+180);
        tempVec.x = hookMtx[0][3];
        tempVec.y = hookMtx[1][3];
        tempVec.z = hookMtx[2][3];
        MBPlayerPosSetV(capsulePlayer, &tempVec);
        MBPlayerRotSet(capsulePlayer, rotX, angle+180, 0);
        HuPrcVSleep();
    }
    MBMasuPlayerLightSet(capsulePlayer, TRUE);
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    for(i=0; i<8; i++) {
        t = 1-(i/7.0f);
        Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_C", hookMtx);
        hookMtx[1][3] += hookYOfs;
        hookMtx[0][3] += radius*HuSin(angle+180);
        hookMtx[2][3] += radius*HuCos(angle+180);
        tempVec.x = playerPos.x+(t*(hookMtx[0][3]-playerPos.x));
        tempVec.y = playerPos.y+(t*(hookMtx[1][3]-playerPos.y));
        tempVec.z = playerPos.z+(t*(hookMtx[2][3]-playerPos.z));
        MBPlayerPosSetV(capsulePlayer, &tempVec);
        MBPlayerRotSet(capsulePlayer, t*rotX, angle+180, 0);
        HuPrcVSleep();
    }
    MBPlayerPosSet(capsulePlayer, playerPos.x, playerPos.y, playerPos.z);
    MBPlayerRotSet(capsulePlayer, 0, angle+180, 0);
    for(i=0; i<30.0f; i++) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleEffHookCall(3, mdlId, TRUE, FALSE,FALSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX01_HANACHAN_STAR_MES, HUWIN_SPEAKER_HANACHAN2);
    while(!MBTopWinDoneCheck()) {
        HuPrcVSleep();
    }
    MBTopWinKill();
    endPos = mdlPos;
    for(i=0; i<60.0f; i++) { 
        t = i/60.0f;
        mdlPos.z = endPos.z+(300*HuSin(t*90));
        mdlPos.y = endPos.y+(t*650);
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    if(!MBKillCheck()) {
        obj->work[0]++;
    }
    if(seNo1 != MSM_SENO_NONE) {
        MBAudFXStop(seNo1);
    }
    MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_FG)) {
        HuPrcVSleep();
    }
    MBPlayerRotateStart(capsulePlayer, 0, 15);
    while(!MBPlayerRotateCheck(capsulePlayer)) {
        HuPrcVSleep();
    }
    MBCameraMotionWait();
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    MBCameraViewNoSet(GwSystem.turnPlayerNo, MB_CAMERA_VIEW_ZOOMOUT);
    MBCameraMotionWait();
    if(MBPlayerStarGet(capsulePlayer) >= 999) {
        MBWinCreate(MBWIN_TYPE_RESULT, MESS_BOARD_STAR_CHORL_6, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
    } else {
        _SetFlag(FLAG_BOARD_STARMUS_RESET);
        HuPrcSleep(30);
        MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
        starNo = MBStarNoRandGet();
        MBStarGetExec(capsulePlayer, NULL);
        MBStarMapViewExec(capsulePlayer, starNo);
        MBStarMasuDispSet(GwPlayer[capsulePlayer].masuId, FALSE);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
        }
        WipeWait();
        MBMusMainPlay();
        _ClearFlag(FLAG_BOARD_STARMUS_RESET);
    }
    HuPrcEnd();
}

void MBCapsuleHanachanDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleGlowEffKill();
    MBCapsuleHanachanRingKill();
    MBCapsuleRingEffKill();
    MBCapsuleRayEffKill();
}

static void CapsuleHanachanEffExec(OMOBJ *obj)
{
    
}

void MBCapsuleStatusPosMoveWait(BOOL dispF, BOOL waitF)
{
    
}

BOOL MBCapsuleStatusDispCheck(int playerNo)
{
    
}

void MBPlayerMoveInit(void)
{
    
}

void MBPlayerMoveObjCreate(void)
{
    
}

void MBPlayerMoveObjKill(int playerNo)
{
    
}

void MBCapsuleExplodeEffCreate(int no)
{
    
}

void MBCapsuleExplodeEffKill(int no)
{
    
}

int MBCapsuleExplodeEffCheck(int no)
{
    
}

int MBCapsuleExplodeEffAdd(int no, HuVecF pos, HuVecF rot, float size, float speed, float animSpeed, GXColor color)
{
    
}

int MBCapsuleExplodeEffLineAdd(int no, HuVecF pos, HuVecF rot, float size, float speed, float radius, float animSpeed, GXColor color)
{
    
}

void MBCapsuleDustExplodeAdd(int no, HuVecF pos)
{
    
}

void MBCapsuleNKinokoEffCreate(void)
{
    
}

void MBCapsuleNKinokoEffKill(void)
{
    
}

int MBCapsuleNKinokoEffAdd(HuVecF pos, HuVecF vel, float scale, float rotSpeed, int maxTime, GXColor color)
{
    
}

void MBCapsuleGlowEffCreate(void)
{
    
}

void MBCapsuleGlowEffKill(void)
{
    
}

int MBCapsuleGlowEffCheck(void)
{
    
}

void MBCapsuleGlowEffBlendModeSet(int blendMode)
{
    
}

int MBCapsuleGlowEffAdd(HuVecF pos, HuVecF vel, float scale, float fadeSpeed, float rotSpeed, float gravity, GXColor color)
{
    
}

void MBCapsuleRingEffCreate(void)
{
    
}

void MBCapsuleRingEffKill(void)
{
    
}

int MBCapsuleRingEffAdd(HuVecF pos, HuVecF rot, HuVecF vel, int inTime, int rotTime, int no, GXColor color)
{
    
}

void MBCapsuleRayEffCreate(void)
{
    
}

void MBCapsuleRayEffKill(void)
{
    
}

int MBCapsuleRayEffAdd(HuVecF pos, HuVecF rot, HuVecF moveDir, int speed, GXColor color)
{
    
}

void MBCapsuleHanachanRingCreate(void)
{
    
}

void MBCapsuleHanachanRingKill(void)
{
    
}

int MBCapsuleHanachanRingAdd(HuVecF *pos, HuVecF *dir, int inTime, int holdTime, int outTime, GXColor color)
{
    
}

void MBCapsuleHanachanRingClear(void)
{
    
}

void MBCapsuleLoseEffCreate(void)
{
    
}

void MBCapsuleLoseEffKill(void)
{
    
}

int MBCapsuleLoseEffNumGet(void)
{
    
}

int MBCapsuleLoseEffAdd(HuVecF *pos, HuVecF *vel, float scale, int maxTime, int capsuleNo)
{
    
}

void MBCapsuleLoseEffAddMulti(int playerNo, int max, float scale)
{
    
}

static HU3DPARMANID CapsuleStarEffCreate(ANIMDATA *animP)
{
    
}

void MBCapsuleModelMotSet(int mdlId, int motNo, u32 attr, BOOL shiftF)
{
    
}

void MBCapsulePlayerMotSet(int playerNo, int motNo, u32 attr, BOOL shiftF)
{
    
}

void MBCapsuleCameraViewPlayerSet(int playerNo)
{
    
}

void MBCapsuleCameraViewNoSet(int playerNo, int viewNo)
{
    
}

void MBCapsuleCameraViewSet(int playerNo, int viewNo, BOOL masuF)
{
    
}

BOOL MBCapsuleTeamCheck(int player1, int player2)
{
    
}

int MBCapsulePlayerAliveFind(int playerNo)
{
    
}

void MBCapsuleChoiceSet(int choice)
{
    
}

s16 MBCapsuleMasuNextGet(s16 masuId, HuVecF *pos)
{
    
}

s16 MBCapsuleMasuNextRegularGet(s16 masuId, HuVecF *pos)
{
    
}

void MBCapsuleTimingHookCreate(HU3DMODELID modelId, HU3DMOTID motId, BOOL lagF)
{
    
}

float MBCapsuleAngleWrap(float a, float b)
{
    
}

float MBCapsuleAngleLerp(float a, float b, float t)
{
    
}

void MBCapsuleBezierGetV(float t, float *a, float *b, float *c, float *out)
{
    
}

void MBCapsuleBezierNormGetV(float t, float *a, float *b, float *c, float *out)
{
    
}