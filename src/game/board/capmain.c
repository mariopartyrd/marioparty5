#include "game/esprite.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/main.h"
#include "game/printfunc.h"

#include "game/board/mgcall.h"

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
#include "game/board/telop.h"

#include "datanum/capsule.h"
#include "datanum/effect.h"
#include "datanum/blast5.h"

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
#include "messnum/boardope.h"

//Fake Board Prototypes
extern BOOL MBCircuitGoalCheck(int playerNo);
extern void MBCircuitCarInit(void);
extern int MBCircuitKettouSaiExec(int playerNo);
extern BOOL MBCircuitCarMoveExec(int playerNo);

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
static int saiHookTime;
static int seDelayMax;
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
            if(!MBCapsuleStatusFocusCheck(capsulePlayer)) {
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

void MBBankCoinAdd(int coin)
{
    GwSystem.bankCoin = GwSystem.bankCoin+coin;
    if(GwSystem.bankCoin > 999) {
        GwSystem.bankCoin = 999;
    }
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

int MBMasuBomheiNumGet(int masuId)
{
    int capsuleNo = MBMasuCapsuleGet(MASU_LAYER_DEFAULT, masuId);
    return (capsuleNo >> 8) & 0xFF;
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
    OMOBJ *obj;
    CAPSULE_STAR **objWork;
    
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
    work->mem[no] = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
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
    HuVecF pos;
    
    if((int)obj->work[0] != 0) {
        (void)obj;
        goto delObj;
    }
    if(!(++obj->work[2] & 0x1)) {
        MBModelPosGet(obj->work[1], &pos);
        pos.y += 100;
        MBCapsuleHanachanGlowEffAdd(pos, ((MBCapsuleEffRandF()*0.05f)+0.15f)*100, 0.016666668f, 150, 150, 50, 1);
    }
    if(0) {
        delObj:
        omDelObjEx(mbObjMan, obj);
        return;
    }
    if(MBKillCheck() || capsuleObj == NULL) {
        omDelObjEx(mbObjMan, obj);
    }
}

void MBCapsuleHammerBroExec(void)
{
    int i; //r31
    int j; //r30
    int mdlId; //r29
    int coinNum; //r18
    int daizaMdlId; //r17
    
    float t; //f31
    float angleY; //f30
    float angleX; //f29
    
    HuVecF hammerCurve[2][3]; //sp+0xEC
    Mtx hammerMtx; //sp+0xBC
    HuVecF hammerPos[2]; //sp+0xA4
    int motFile[8]; //sp+0x84
    HuVecF playerPos; //sp+0x78
    HuVecF mdlPos; //sp+0x6C
    HuVecF coinVel; //sp+0x60
    HuVecF coinPos; //sp+0x54
    int hammerMdlId[2]; //sp+0x4C
    int kuruHammerMdlId[2]; //sp+0x44
    float kuruHammerTime[2]; //sp+0x3C
    BOOL coinTakeF; //sp+0x38
    int masuId; //sp+0x34
    
    
    MBCapsuleGlowEffCreate();
    MBCapsuleCoinEffCreate();
    MBPlayerMoveInit();
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    
    motFile[0] = CAPSULECHAR1_HSF_hammer_bros_idle;
    motFile[1] = CAPSULECHAR1_HSF_hammer_bros_throw;
    motFile[2] = CAPSULECHAR1_HSF_hammer_bros_end;
    motFile[3] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_hammer_bros, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    hammerMdlId[0] = MBCapsuleModelCreate(CAPSULECHAR1_HSF_hammer, NULL, FALSE, 10, FALSE);
    MBModelDispSet(hammerMdlId[0], FALSE);
    MBModelHookSet(mdlId, "itemhook_R", hammerMdlId[0]);
    hammerMdlId[1] = MBCapsuleModelCreate(CAPSULECHAR1_HSF_hammer, NULL, FALSE, 10, FALSE);
    MBModelDispSet(hammerMdlId[1], FALSE);
    MBModelHookSet(mdlId, "itemhook_L", hammerMdlId[1]);
    daizaMdlId = MBCapsuleModelCreate(CAPSULE_HSF_hammerBroDaiza, NULL, FALSE, 10, FALSE);
    MBModelDispSet(daizaMdlId, FALSE);
    for(j=0; j<2; j++) {
        kuruHammerMdlId[j] = MBCapsuleModelCreate(CAPSULECHAR1_HSF_kuru_hammer, NULL, FALSE, 10, FALSE);
        MBModelDispSet(kuruHammerMdlId[j], FALSE);
    }
    MBAudFXPlay(MSM_SE_BOARD_32);
    MBModelDispSet(mdlId, TRUE);
    MBModelDispSet(hammerMdlId[0], TRUE);
    MBModelDispSet(hammerMdlId[1], TRUE);
    MBModelDispSet(daizaMdlId, TRUE);
    for(j=0; j<60.0f; j++) {
        t = j/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = playerPos.y+((-470*HuSin(t*90))+700);
        mdlPos.z = -100+playerPos.z;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelPosSet(daizaMdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        if(t < 0.75f) {
            MBCapsuleEffHookCall(8, mdlId, FALSE, FALSE, FALSE);
        }
        HuPrcVSleep();
    }
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    for(j=0; j<30.0f; j++) {
        HuPrcVSleep();
    }
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
    for(i=0; i<2; i++) {
        hammerCurve[i][0] = mdlPos;
        hammerCurve[i][0].y += 100;
        if(i == 0) {
            hammerCurve[i][0].x -= 50;
        } else {
            hammerCurve[i][0].x += 50;
        }
        hammerCurve[i][2] = playerPos;
        hammerCurve[i][2].y += 150;
        VECSubtract(&hammerCurve[i][2], &hammerCurve[i][0], &hammerCurve[i][1]);
        VECScale(&hammerCurve[i][1], &hammerCurve[i][1], 0.5f);
        VECAdd(&hammerCurve[i][0], &hammerCurve[i][1], &hammerCurve[i][1]);
        if(i == 0) {
            hammerCurve[i][1].x -= ((MBCapsuleEffRandF()*0.3f)+0.8f)*300;
        } else {
            hammerCurve[i][1].x += ((MBCapsuleEffRandF()*0.3f)+0.8f)*300;
        }
        hammerCurve[i][1].y += ((MBCapsuleEffRandF()*0.3f)+0.8f)*400;
    }
    for(i=0; i<2; i++) {
        kuruHammerTime[i] = 0;
    }
    coinTakeF = FALSE;
    do {
        if(MBMotionShiftIDGet(mdlId) == HU3D_MOTID_NONE) {
            if(MBMotionTimeGet(mdlId) >= 30 && !MBModelDispGet(kuruHammerMdlId[0])) {
                Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_R", hammerMtx);
                hammerCurve[0][0].x = hammerMtx[0][3];
                hammerCurve[0][0].y = hammerMtx[1][3];
                hammerCurve[0][0].z = hammerMtx[2][3];
                MBModelDispSet(kuruHammerMdlId[0], TRUE);
                MBModelDispSet(hammerMdlId[0], FALSE);
                MBAudFXPlay(MSM_SE_BOARD_49);
            }
            if(MBMotionTimeGet(mdlId) >= 35 && !MBModelDispGet(kuruHammerMdlId[1])) {
                Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_L", hammerMtx);
                hammerCurve[1][0].x = hammerMtx[0][3];
                hammerCurve[1][0].y = hammerMtx[1][3];
                hammerCurve[1][0].z = hammerMtx[2][3];
                MBModelDispSet(kuruHammerMdlId[1], TRUE);
                MBModelDispSet(hammerMdlId[1], FALSE);
                MBAudFXPlay(MSM_SE_BOARD_49);
            }
        }
        for(i=0; i<2; i++) {
            if(MBModelDispGet(kuruHammerMdlId[i])) {
                kuruHammerTime[i] += 0.016666668f;
                MBCapsuleBezierGetV(kuruHammerTime[i], (float *)&hammerCurve[i][0], (float *)&hammerCurve[i][1], (float *)&hammerCurve[i][2], (float *)&hammerPos[i]);
                MBModelPosSet(kuruHammerMdlId[i], hammerPos[i].x, hammerPos[i].y, hammerPos[i].z);
                if(i == 0) {
                    MBModelRotSet(kuruHammerMdlId[i], 0, 0, 30);
                } else { 
                    MBModelRotSet(kuruHammerMdlId[i], 0, 0, -30);
                }
                mtxRot(hammerMtx, kuruHammerTime[i]*1440, 0, 0);
                MBCapsuleModelMtxSet(kuruHammerMdlId[i], &hammerMtx);
                if(kuruHammerTime[i] > 1.0f) {
                    if(i == 0) {
                        MBPlayerMoveHitCreate(capsulePlayer, TRUE, TRUE);
                        CharFXPlay(GwPlayer[capsulePlayer].charNo, CHARVOICEID(7));
                    }
                    omVibrate(capsulePlayer, 12, 4, 2);
                    MBModelDispSet(kuruHammerMdlId[i], FALSE);
                    MBAudFXPlay(MSM_SE_BOARD_50);
                }
            }
        }
        for(i=0; i<2; i++) {
            if(kuruHammerTime[i] < 1) {
                break;
            }
        }
        HuPrcVSleep();
    } while(i < 2);
    if(GWPartyFGet() != FALSE) {
        coinNum = 10;
    } else {
        coinNum = 5;
    }
    if(coinNum > MBPlayerCoinGet(capsulePlayer)) {
        coinNum = MBPlayerCoinGet(capsulePlayer);
    }
    angleY = MBCapsuleEffRandF()*360;
    for(j=0; j<coinNum; j++) {
        angleY += 360.0f/coinNum;
        coinPos = playerPos;
        coinPos.y += 100;
        angleX = (MBCapsuleEffRandF()*15)+70;
        t = ((MBCapsuleEffRandF()*0.3f)+0.8f)*65;
        coinVel.x = t*(HuSin(angleY)*HuCos(angleX));
        coinVel.z = t*(HuCos(angleY)*HuCos(angleX));
        coinVel.y = t*HuSin(angleX);
        MBCapsuleCoinEffAdd(&coinPos, &coinVel, 0.75f, 4.9f, 30, 0);
    }
    if(coinNum > 0) {
        MBPlayerCoinAdd(capsulePlayer, -coinNum);
    }
    do {
        HuPrcVSleep();
    } while(!MBPlayerMoveObjCheck(capsulePlayer));
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_LOOP);
    HuPrcSleep(60);
    if(coinNum > 0) {
        MBCapsuleCoinDispExec(capsulePlayer, -coinNum, FALSE, TRUE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_HAMMERBRO_COIN, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_HAMMERBRO_COIN_NONE, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
    }
    MBAudFXPlay(MSM_SE_BOARD_32);
    for(j=0; j<60.0f; j++) {
        t = j/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = playerPos.y+((-470*HuCos(t*90))+700);
        mdlPos.z = -100+playerPos.z;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelPosSet(daizaMdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    MBPlayerMoveObjKill(capsulePlayer);
    HuPrcEnd();
}

void MBCapsuleHammerBroDestroy(void)
{
    MBCapsuleGlowEffKill();
    MBCapsuleCoinEffKill();
}

void MBCapsuleCoinBlockExec(void)
{
    int i; //r29
    OMOBJ *blkObj; //r28
    int no; //r26
    int blkMdlId; //r25
    int coinNum; //r23
    
    float angle; //f31
    float power; //f30
    float coinSpeed; //f29
    float glowSpeed; //f28
    
    HuVecF playerPosOld; //sp+0x64
    HuVecF playerPos; //sp+0x58
    HuVecF glowVel; //sp+0x4C
    HuVecF effPosBase; //sp+0x40
    HuVecF effPos; //sp+0x34
    HuVecF effVel; //sp+0x28
    GXColor color; //sp+0xC
    
    MBCapsuleGlowEffCreate();
    MBCapsuleCoinEffCreate();
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerPosOld = playerPos;
    effPosBase = playerPos;
    effPosBase.y += 300;
    
    blkMdlId = MBCapsuleModelCreate(BOARD_HSF_sai5, NULL, FALSE, 10, FALSE);
    MBMotionSpeedSet(blkMdlId, 0);
    blkObj = MBCapsuleSaiHiddenCreate(capsulePlayer, blkMdlId, 5, FALSE, TRUE);
    MBCapsuleSaiHiddenStartSet(blkObj, FALSE);
    MBAudFXPlay(MSM_SE_BOARD_95);
    do {
        HuPrcVSleep();
    } while(!MBCapsuleSaiHiddenInCheck(blkObj));
    for(angle=0, i=0; i<128; i++) {
        angle += (MBCapsuleEffRandF()+1.0f)*10.0f;
        effPos.x = effPosBase.x+((-0.5f+MBCapsuleEffRandF())*75);
        effPos.y = effPosBase.y+((-0.5f+MBCapsuleEffRandF())*75);
        effPos.z = effPosBase.z+50;
        glowSpeed = (MBCapsuleEffRandF()+1.0f)*5.0f;
        glowVel.x = HuSin(angle)*glowSpeed;
        glowVel.y = HuCos(angle)*glowSpeed;
        glowVel.z = 0;
        power = MBCapsuleEffRandF();
        color.r = 192+(63*power);
        color.g = 192+(63*power);
        color.b = 192+(63*power);
        color.a = 192+(63*MBCapsuleEffRandF());
        MBCapsuleGlowEffAdd(effPos, glowVel, ((MBCapsuleEffRandF()*0.1f)+0.3f)*100, (MBCapsuleEffRandF()+1.0f)*0.016666668f, 0, 0, color);
    }
    if(!capsuleFlags.saiHiddenF) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_COIN_BLOCK_START, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            MBTutorialExec(TUTORIAL_INST_SAI_HIDDEN);
        } else {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX98_SAIHIDDEN, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTopWinKill();
        }
    }
    MBCapsuleSaiHiddenStartSet(blkObj, TRUE);
    seDelayMax = 24;
    for(no=0; no<5; no++) {
        do {
            HuPrcVSleep();
        } while(!MBCapsuleSaiHiddenHitCheck(blkObj, &effPos));
        angle = MBCapsuleEffRandF()*360;
        for(i=0; i<2; i++) {
            angle += 180;
            effPos = effPosBase;
            power = (MBCapsuleEffRandF()*15)+70;
            coinSpeed = ((MBCapsuleEffRandF()*0.2f)+0.8f)*50;
            effVel.x = HuSin(angle)*HuCos(power)*coinSpeed;
            effVel.z = HuCos(angle)*HuCos(power)*coinSpeed;
            effVel.y = HuSin(power)*coinSpeed;
            MBCapsuleCoinEffAdd(&effPos, &effVel, 0.75f, 4.9f, 15, 2);
        }
        MBCapsuleSeDelayPlay(MSM_SE_CMN_19, 15);
    }
    do {
        HuPrcVSleep();
    } while(!MBCapsuleSaiHiddenKillCheck(blkObj));
    MBCapsuleSaiHiddenKill(blkObj);
    coinNum = 10;
    MBCoinAddDispExec(capsulePlayer, coinNum, TRUE);
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    HuPrcEnd();
}

void MBCapsuleCoinBlockDestroy(void)
{
    MBCapsuleGlowEffKill();
    MBCapsuleCoinEffKill();
}

void MBCapsuleTogezoExec(void)
{
    int targetPlayer; //r31
    int i; //r30
    int mdlId; //r29
    int coinNum; //r27
    int masuIdTarget; //r23
    int masuId; //r21
    BOOL fadeF; //r20
    int fallMotId; //r17
    
    float speed; //f31
    float t; //f30
    float mdlRotY; //f29
    float angleY; //f28
    float mdlRotX; //f27
    float angleX; //f26
    
    
    HuVecF playerPos; //sp+0x48
    HuVecF playerPosTarget; //sp+0x3C
    HuVecF mdlVel; //sp+0x30
    HuVecF coinPos; //sp+0x24
    HuVecF coinVel; //sp+0x18
    HuVecF mdlPos; //sp+0xC
    
    MBCapsuleCoinEffCreate();
    if(MBPlayerAliveGet(NULL) >= 3) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_TOGEZO_ROULETTE, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        if(MBRouletteCreate(capsulePlayer, 3)) {
            MBRouletteWait();
        }
        targetPlayer = MBRouletteResultGet();
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_TOGEZO, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
        targetPlayer = MBCapsulePlayerAliveFind(capsulePlayer);
    }
    masuIdTarget = GwPlayer[targetPlayer].masuId;
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    MBPlayerPosGet(targetPlayer, &playerPosTarget);
    fallMotId = MBCapsulePlayerMotionCreate(targetPlayer, CHARMOT_HSF_c000m1_354);
    MBPlayerMoveInit();
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_togezo, NULL, FALSE, 10, FALSE);
    MBModelDispSet(mdlId, FALSE);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    
    if(masuId == masuIdTarget) {
        mdlPos = playerPos;
        playerPos = playerPosTarget;
        playerPosTarget = mdlPos;
        MBPlayerPosSetV(capsulePlayer, &playerPos);
        MBPlayerPosSetV(targetPlayer, &playerPosTarget);
    }
    MBStarMasuDispSet(masuIdTarget, FALSE);
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(masuIdTarget, NULL, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    WipeWait();
    MBAudFXPlay(MSM_SE_BOARD_51);
    MBModelDispSet(mdlId, TRUE);
    for(i=0, mdlRotY=0; i<120.0f; i++) {
        t = i/120.0f;
        if(t > 1) {
            t = 1;
        }
        mdlRotY += 10+(t*10);
        mdlPos = playerPosTarget;
        mdlPos.y += 300;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, 0, mdlRotY, 0);
        MBModelScaleSet(mdlId, t, t, t);
        HuPrcVSleep();
    }
    for(i=0, speed=1; i<15.0f || mdlRotY < 360; i++) {
        speed -= 0.05f;
        if(speed < 0) {
            speed = 0;
        }
        if(mdlRotY > 360) {
            mdlRotY -= 360;
        }
        mdlRotY += 10+(speed*10);
        MBModelRotSet(mdlId, 0, mdlRotY, 0);
        HuPrcVSleep();
    }
    MBModelRotSet(mdlId, 0, 0, 0);
    for(mdlRotX=0, i=0; i<15.0f; i++) {
        mdlRotX = MBCapsuleAngleLerp(179, mdlRotX, 15);
        MBModelRotSet(mdlId, mdlRotX, 0, 0);
        HuPrcVSleep();
    }
    mdlVel.x = mdlVel.y = mdlVel.z = 0;
    fadeF = FALSE;
    while(!fadeF) {
        if(!fadeF && mdlPos.y < playerPosTarget.y+200) {
            fadeF = TRUE;
            MBAudFXPlay(MSM_SE_BOARD_52);
            MBPlayerMotionNoSet(targetPlayer, fallMotId, HU3D_MOTATTR_NONE);
            MBPlayerMoveEjectCreate(targetPlayer, TRUE);
            omVibrate(targetPlayer, 12, 4, 2);
            CharFXPlay(GwPlayer[targetPlayer].charNo, CHARVOICEID(7));
            if(GWPartyFGet() != FALSE) {
                coinNum = 10;
            } else {
                coinNum = 5;
            }
            if(MBPlayerCoinGet(targetPlayer) < coinNum) {
                coinNum = MBPlayerCoinGet(targetPlayer);
            }
            MBPlayerCoinAdd(targetPlayer, -coinNum);
            angleY = MBCapsuleEffRandF()*360;
            for(i=0; i<coinNum; i++) {
                angleY += 360.0f/coinNum;
                coinPos = playerPosTarget;
                coinPos.y += 100;
                angleX = (MBCapsuleEffRandF()*15)+70;
                speed = ((MBCapsuleEffRandF()*0.3f)+0.8f)*65;
                coinVel.x = speed*(HuSin(angleY)*HuCos(angleX));
                coinVel.z = speed*(HuCos(angleY)*HuCos(angleX));
                coinVel.y = speed*HuSin(angleX);
                MBCapsuleCoinEffAdd(&coinPos, &coinVel, 0.75f, 4.9f, 30, 0);
            }
            mdlPos = playerPosTarget;
            mdlPos.y += 200;
            angleY = (frand() & 0x1) ? 90.0f : 270.0f;
            angleY += (0.5f-MBCapsuleEffRandF())*30;
            angleX = (10*MBCapsuleEffRandF())+45;
            speed = 35;
            mdlVel.x = speed*(HuSin(angleY)*HuCos(angleX));
            mdlVel.z = speed*(HuCos(angleY)*HuCos(angleX));
            mdlVel.y = speed*HuSin(angleX);
        }
        VECAdd(&mdlPos, &mdlVel, &mdlPos);
        mdlVel.y -= 1.633333357671897;
        mdlRotX = MBCapsuleAngleLerp(179, mdlRotX, 15);
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, mdlRotX, 0, 0);
        HuPrcVSleep();
    }
    t = 1;
    do {
        t -= 0.05f;
        if(t < 0) {
            t = 0;
        }
        VECAdd(&mdlPos, &mdlVel, &mdlPos);
        mdlVel.y -= 1.633333357671897;
        mdlRotX = MBCapsuleAngleLerp(180, mdlRotX, 5);
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, mdlRotX, 0, 0);
        MBModelAlphaSet(mdlId, 255*t);
        HuPrcVSleep();
    } while(t > 0 || !MBPlayerMoveObjCheck(targetPlayer));
    MBModelDispSet(mdlId, FALSE);
    if(coinNum > 0) {
        MBCapsuleCoinDispExec(targetPlayer, -coinNum, FALSE, TRUE);
    } else {
        HuPrcSleep(60);
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    if(masuId == masuIdTarget) {
        mdlPos = playerPos;
        playerPos = playerPosTarget;
        playerPosTarget = mdlPos;
        MBPlayerPosSetV(capsulePlayer, &playerPos);
        MBPlayerPosSetV(targetPlayer, &playerPosTarget);
    }
    MBStarMasuDispSet(masuIdTarget, TRUE);
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(masuId, NULL, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    WipeWait();
    if(coinNum > 0) {
        MBCoinAddExec(capsulePlayer, coinNum);
        MBCapsuleCoinDispExec(capsulePlayer, coinNum, TRUE, TRUE);
    }
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    MBCapsulePlayerMotSet(targetPlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    MBPlayerMoveObjKill(targetPlayer);
    HuPrcEnd();
}

void MBCapsuleTogezoDestroy(void)
{
    MBCapsuleCoinEffKill();
}

void MBCapsulePatapataExec(void)
{
    int i; //r31
    int j; //r30
    int mdlId; //r29
    int coinNum; //r26
    int coinTotal; //r24
    int coinId; //r23
    int *chanceTbl; //r21
    int saiValue; //r20
    int playerNum; //r19
    int k; //r18
    
    float t; //f31
    float yOfs; //f30
    float ySpeed; //f29
    
    int motFile[8]; //sp+0x98
    HuVecF playerPos; //sp+0x8C
    HuVecF coinVel; //sp+0x80
    HuVecF basePos; //sp+0x74
    HuVecF mdlPos; //sp+0x68
    HuVecF coinPos; //sp+0x5C
    HuVecF playerPosOld; //sp+0x50
    int motId[GW_PLAYER_MAX]; //sp+0x40
    char mes[16]; //sp+0x30
    int accumChance; //sp+0x2C
    int totalChance; //sp+0x28
    int seNo; //sp+0x24
    int coinResult; //sp+0x20
    int otherGrp; //sp+0x1C
    
    static int chanceTbl1[8] = {
        10,
        20,
        30,
        25,
        10,
        5,
    };
    
    static int chanceTbl2[8] = {
        10,
        10,
        20,
        20,
        20,
        15,
        5
    };
    
    MBCapsuleGlowEffCreate();
    MBCapsuleCoinEffCreate();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            motId[i] = MBCapsulePlayerMotionCreate(i, CHARMOT_HSF_c000m1_357);
        }
    }
    motFile[0] = CAPSULECHAR2_HSF_patapata_idle;
    motFile[1] = CAPSULECHAR2_HSF_patapata_up;
    motFile[2] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR2_HSF_patapata, motFile, FALSE, 10, FALSE);
    MBModelDispSet(mdlId, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBPlayerPosGet(capsulePlayer, &playerPos);
    MBAudFXPlay(MSM_SE_BOARD_32);
    capsuleObj = NULL;
    Hu3DMotionTimingHookSet(MBModelIdGet(mdlId), MBCapsuleTimingHookCreate);
    MBPlayerPosGet(capsulePlayer, &basePos);
    mdlPos = basePos;
    mdlPos.y += 600;
    MBModelDispSet(mdlId, TRUE);
    for(j=0; j<120.0f; j++) {
        t = j/120.0f;
        yOfs = (-400*HuSin(t*90))+600;
        mdlPos.y = basePos.y+yOfs+(0.125*(100*HuSin(t*1440)));
        mdlPos.z = -100+basePos.z;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        HuPrcVSleep();
    }
    for(i=0, coinTotal=0, playerNum=-1; i<GW_PLAYER_MAX; i++) {
        if(i == capsulePlayer) {
            continue;
        }
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(!GWTeamFGet() || !MBCapsuleTeamCheck(i, capsulePlayer)) {
            coinTotal += MBPlayerCoinGet(i);
            if(MBPlayerCoinGet(i) > 0) {
                playerNum = i;
            }
        }
    }
    if(coinTotal <= 0) {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_24);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_PATAPATA_COIN_NONE, HUWIN_SPEAKER_PATA2);
        MBTopWinWait();
        MBTopWinKill();
        for(j=0; j<120.0f; j++) {
            t = j/120.0f;
            yOfs = (-400*HuCos(t*90))+600;
            mdlPos.y = basePos.y+yOfs+(0.25*(100*HuSin(t*1440)));
            mdlPos.z = -100+basePos.z;
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
    } else {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_PATAPATA_COIN, HUWIN_SPEAKER_PATA2);
        MBTopWinWait();
        MBTopWinKill();
        if(GwSystem.turnNo < GwSystem.turnMax/2) {
            chanceTbl = chanceTbl1;
        } else {
            chanceTbl = chanceTbl2;
        }
        for(i=0, totalChance=0; i<8; i++) {
            totalChance += chanceTbl[i];
        }
        saiValue = MBCapsuleEffRandF()*totalChance;
        for(i=0, accumChance=0; i<8; i++) {
            accumChance += chanceTbl[i];
            if(saiValue < accumChance) {
                break;
            }
        }
        if(i < 8) {
            saiValue = i;
        } else {
            saiValue = 0;
        }
        coinResult = MBSaiExec(capsulePlayer, SAITYPE_PATAPATA_COIN, NULL, saiValue, TRUE, TRUE, NULL, SAI_COLOR_GREEN);
        do {
            HuPrcVSleep();
        } while(!MBSaiNumStopCheck(capsulePlayer));
        HuPrcSleep(30);
        MBSaiNumKill(capsulePlayer);
        if(GWTeamFGet()) {
            otherGrp = MBPlayerGrpGet(capsulePlayer)^1;
            if(MBPlayerCoinGet(MBPlayerGrpFindPlayer(otherGrp, 0)) <= coinResult) {
                for(i=0, playerNum=-1; i<GW_PLAYER_MAX; i++) {
                    if(i == capsulePlayer) {
                        continue;
                    }
                    if(!MBPlayerAliveCheck(i)) {
                        continue;
                    }
                    if(!GWTeamFGet() || !MBCapsuleTeamCheck(i, capsulePlayer)) {
                        playerNum = i;
                        break;
                    }
                }
            }
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_24);
        sprintf(mes, "%d", coinResult);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_PATAPATA_RESULT, HUWIN_SPEAKER_PATA2);
        MBTopWinInsertMesSet(MESSNUM_PTR(mes), 0);
        MBTopWinWait();
        MBTopWinKill();
        MBCapsuleStatusPosMoveWait(TRUE, TRUE);
        MBAudFXPlay(MSM_SE_BOARD_32);
        MBModelDispSet(mdlId, TRUE);
        for(j=0; j<120.0f; j++) {
            t = j/120.0f;
            yOfs = (400*HuSin(t*90))+200;
            mdlPos.y = basePos.y+yOfs+(0.125*(100*HuSin(t*1440)));
            mdlPos.z = -100+basePos.z;
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBStarDispSetAll(FALSE);
        MBMasuPlayerLightSet(capsulePlayer, FALSE);
        for(i=0, coinTotal=0; i<GW_PLAYER_MAX; i++) {
            if(i == capsulePlayer) {
                continue;
            }
            if(!MBPlayerAliveCheck(i)) {
                continue;
            }
            if(GWTeamFGet() && MBCapsuleTeamCheck(i, capsulePlayer)) {
                continue;
            }
            if(MBPlayerCoinGet(i) <= 0) {
                continue;
            }
            for(j=0; j<GW_PLAYER_MAX; j++) {
                if(j != i) {
                    MBPlayerDispSet(j, FALSE);
                }
            }
            MBPlayerPosGet(i, &playerPosOld);
            MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[i].masuId, &coinPos);
            MBPlayerPosSetV(i, &coinPos);
            MBCameraSkipSet(FALSE);
            MBCameraFocusPlayerSet(i);
            MBCameraMotionWait();
            MBCameraSkipSet(TRUE);
            MBPlayerPosGet(i, &basePos);
            mdlPos = basePos;
            mdlPos.y += 600;
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            MBModelDispSet(mdlId, TRUE);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_BOARD_32);
            MBModelDispSet(mdlId, TRUE);
            for(j=0; j<120.0f; j++) {
                t = j/120.0f;
                yOfs = (-400*HuSin(t*90))+600;
                mdlPos.y = basePos.y+yOfs+(0.125*(100*HuSin(t*1440)));
                mdlPos.z = -100+basePos.z;
                MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                HuPrcVSleep();
            }
            MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
            seNo = HuAudFXPlay(MSM_SE_BOARD_126);
            coinNum = coinResult;
            if(MBPlayerCoinGet(i) < coinNum) {
                coinNum = MBPlayerCoinGet(i);
            }
            omVibrate(i, 12, 6, 6);
            for(j=0; j<coinNum; j++) {
                coinPos = basePos;
                coinPos.x += (0.5f-MBCapsuleEffRandF())*100;
                coinPos.y += 100;
                coinPos.z += 30.000002f;
                yOfs = t = ((MBCapsuleEffRandF()*0.1f)+0.8f)*45;
                ySpeed = coinPos.y;
                for(k=0; yOfs>0 || ySpeed > mdlPos.y+50; k++) {
                    ySpeed += yOfs;
                    yOfs -= 4.900000194708507;
                }
                coinVel.x = (1.0f/k)*(basePos.x-coinPos.x);
                coinVel.z = (1.0f/k)*(basePos.z-coinPos.z);
                coinVel.y = t;
                coinId = MBCapsuleCoinEffAdd(&coinPos, &coinVel, 0.75f, 4.9f, 30, 0xFFFF);
                if(coinId != -1){
                    MBCapsuleCoinEffMaxYSet(coinId, mdlPos.y+50);
                }
                HuPrcSleep(2);
            }
            do {
                HuPrcVSleep();
            } while(MBCapsuleCoinEffNumGet());
            if(seNo != MSM_SENO_NONE) {
                HuAudFXStop(seNo);
            }
            seNo = MSM_SENO_NONE;
            MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
            if(coinNum > 0) {
                MBCoinAddExec(i, -coinNum);
                coinId = MBCapsuleCoinDispExec(i, -coinNum,  TRUE, FALSE);
            } else {
                coinId = -1;
            }
            if(i == playerNum) {
                while(!MBCoinDispKillCheck(coinId)) {
                    HuPrcVSleep();
                }
                MBAudFXPlay(MSM_SE_GUIDE_23);
                MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_PATAPATA_COIN_TAKE, HUWIN_SPEAKER_PATA2);
                MBTopWinWait();
                MBTopWinKill();
            }
            MBAudFXPlay(MSM_SE_BOARD_32);
            for(j=0; j<120.0f; j++) {
                t = j/120.0f;
                yOfs = (-400*HuCos(t*90))+600;
                mdlPos.x = basePos.x;
                mdlPos.y = basePos.y+yOfs+(0.25*(100*HuSin(t*1440)));
                mdlPos.z = -100+basePos.z;
                MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                HuPrcVSleep();
            }
            MBModelDispSet(mdlId, FALSE);
            while(!MBCoinDispKillCheck(coinId)) {
                HuPrcVSleep();
            }
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
            WipeWait();
            MBPlayerPosSetV(i, &playerPosOld);
            for(j=0; j<GW_PLAYER_MAX; j++) {
                MBPlayerDispSet(j, TRUE);
            }
            coinTotal += coinNum;
        }
        MBStarDispSetAll(TRUE);
        MBStarMasuDispSet(GwPlayer[capsulePlayer].masuId, FALSE);
        MBMasuPlayerLightSet(capsulePlayer, TRUE);
        MBCameraSkipSet(FALSE);
        MBCameraFocusPlayerSet(capsulePlayer);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
        WipeWait();
        MBModelDispSet(mdlId, FALSE);
    }
    MBCapsulePlayerIdleWait();
    HuPrcEnd();
    
}

void MBCapsulePatapataDestroy(void)
{
    MBCapsuleGlowEffKill();
    MBCapsuleCoinEffKill();
}

static void CapsuleKillerMain(void);

void MBCapsuleKillerExec(void)
{
    MBCapsuleExplodeEffCreate(capsulePlayer);
    CapsuleKillerMain();
    HuPrcEnd();
}

static void CapsuleKillerMain(void)
{
    int i; //r31
    int mdlId; //r28
    
    float t; //f31
    float scale; //f30
    float power; //f29
    float angle; //f28
    float rotEnd; //f27
    float rot; //f26
    float effSpeed; //f25
    float speed; //f24
    float yOfs; //f23
    
    HuVecF playerPos; //sp+0x5C
    HuVecF startPos; //sp+0x50
    HuVecF effVel; //sp+0x44
    HuVecF effPos; //sp+0x38
    HuVecF mdlPos; //sp+0x2C
    int masuId; //sp+0x10
    GXColor color; //sp+0xC
    
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_killer, NULL, FALSE, 10, FALSE);
    MBModelDispSet(mdlId, FALSE);
    MBPlayerPosGet(capsulePlayer, &playerPos);
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &mdlPos);
    mdlPos.y += 300;
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelDispSet(mdlId, TRUE);
    MBAudFXPlay(MSM_SE_BOARD_35);
    for(i=0; i<32; i++) {
        angle = i*11.25f;
        power = (MBCapsuleEffRandF()*100)*0.33f;
        effPos.x = mdlPos.x+(power*HuCos(angle));
        effPos.y = mdlPos.y+(power*HuSin(angle));
        effPos.z = mdlPos.z+50;
        effSpeed = ((MBCapsuleEffRandF()*0.04f)+0.005f)*100;
        effVel.x = effSpeed*HuCos(angle);
        effVel.y = effSpeed*HuSin(angle);
        effVel.z = 0;
        power = MBCapsuleEffRandF();
        color.r = 32+(power*32);
        color.g = 32+(power*32);
        color.b = 32+(power*32);
        color.a = 192+(MBCapsuleEffRandF()*63);
        MBCapsuleExplodeEffLineAdd(capsulePlayer, effPos, effVel, 
            ((MBCapsuleEffRandF()*0.5f)+1.0f)*100,
            2.5f,
            ((MBCapsuleEffRandF()*0.5f)+0.5f)*100,
            (MBCapsuleEffRandF()*0.66f)+0.33f,
            color);
    }
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        scale = HuSin(t*180)+1;
        MBModelScaleSet(mdlId, scale*0.5f, scale*0.5f, scale*0.5f);
        HuPrcVSleep();
    }
    for(i=0; i<90.0f; i++) {
        t = i/90.0f;
        mdlPos = playerPos;
        mdlPos.y += ((HuSin((t*360)*1.5f)*100)*0.1f)+300;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_BOARD_36);
    startPos.x = mdlPos.x;
    startPos.y = mdlPos.y;
    startPos.z = mdlPos.z-100;
    for(rot=0, i=0; i<120.0f; i++) {
        scale = t = 1-(i/120.0f);
        yOfs = (250*(t*t))+50;
        speed = HuSin((t*t)*180);
        mdlPos = playerPos;
        mdlPos.x += (speed*HuCos((-t*360)*3)*100)*1.5;
        mdlPos.z += (speed*HuSin((-t*360)*3)*100)*1.5;
        mdlPos.y += yOfs;
        VECSubtract(&mdlPos, &startPos, &effPos);
        rotEnd = HuAtan(effPos.x, effPos.z);
        if(t > 0.9f) {
            if(rotEnd <= 0 || rotEnd > 180) {
                rot = MBCapsuleAngleLerp(rotEnd, rot, t*60);
            }
        } else {
            rot = rotEnd;
        }
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, 0, rot, 0);
        MBModelScaleSet(mdlId, scale*0.5f, scale*0.5f, scale*0.5f);
        startPos = mdlPos;
        HuPrcVSleep();
    }
    GwPlayer[capsulePlayer].saiMode = MB_SAIMODE_KILLER;
    HuPrcSleep(30);
}

void MBCapsuleKillerMoveExec(int playerNo, BOOL flag)
{
    int i; //r31
    HuVecF *posTbl; //r30
    int j; //r28
    int *masuIdTbl; //r27
    int masuNo; //r26
    int masuMax; //r25
    int mdlId; //r24
    int masuId; //r22
    int masuIdNext; //r21
    int coinNum; //r19
    int stepNum; //r18
    int decideLinkNum; //r17
    
    float t; //f31
    float mag; //f22
    float speed; //f21
    
    Mtx rotMtx; //sp+0x1A8
    s16 linkTbl[MASU_LINK_MAX*2]; //sp+0x194
    HuVecF playerPos; //sp+0x188
    HuVecF playerRot; //sp+0x17C
    HuVecF vel; //sp+0x170
    HuVecF masuPosNext; //sp+0x164
    HuVecF masuPos; //sp+0x158
    HuVecF masuPosFirst; //sp+0x14C
    HuVecF dir; //sp+0x140
    HuVecF tempVec; //sp+0x134
    HuVecF mdlPos; //sp+0x128
    HuVecF mdlRot; //sp+0x11C
    HuVecF mdlEndRot; //sp+0x110
    HuVecF faceRot; //sp+0x104
    HuVecF faceEndRot; //sp+0xF8
    HuVecF prevPos; //sp+0xEC
    int coinDisp[GW_PLAYER_MAX]; //sp+0xDC
    BOOL hitFlag[GW_PLAYER_MAX]; //sp+0xCC
    int motIdRide; //sp+0x98
    int motIdRideStart; //sp+0x94
    int walkNum; //sp+0x90
    int masuType; //sp+0x8C
    int masuFlag; //sp+0x88
    int masuIdClimb; //sp+0x84
    int masuIdClimbNext; //sp+0x80
    int coinTotal; //sp+0x7C
    int seNo; //sp+0x78
    s16 decideLinkTbl[2]; //sp+0x74
    GXColor color; //sp+0x70
    static HuVecF ofsTbl[9] = {
        { 0, 0, 0 },
        { 80, 0, 80 },
        { -80, 0, 80 },
        { 80, 0, -80 },
        { -80, 0, -80 },
        { 100, 0, 0 },
        { -100, 0, 0 },
        { 0, 0, 100 },
        { 0, 0, -100 }
    };
    MBPlayerPosGet(playerNo, &playerPos);
    MBPlayerRotGet(playerNo, &playerRot);
    masuId = GwPlayer[playerNo].masuId;
    posTbl = MBCapsuleMalloc(MASU_MAX*sizeof(HuVecF));
    memset(posTbl, 0, MASU_MAX*sizeof(HuVecF));
    HuPrcVSleep();
    masuIdTbl = MBCapsuleMalloc(MASU_MAX*sizeof(int));
    memset(masuIdTbl, 0, MASU_MAX*sizeof(int));
    HuPrcVSleep();
    for(i=0; i<MASU_MAX; i++) {
        masuIdTbl[i] = masuId;
        posTbl[i] = playerPos;
        posTbl[i].y += 100;
    }
    HuPrcVSleep();
    for(i=0, decideLinkNum=0; i<2; i++) {
        if(MBMasuLinkTblGet(MASU_LAYER_DEFAULT, masuId, linkTbl) > 0) {
            masuId = linkTbl[0];
            decideLinkTbl[decideLinkNum] = masuId;
            decideLinkNum++;
        }
    }
    for(i=0, masuMax=0; i<decideLinkNum; i++) {
        masuIdTbl[i] = decideLinkTbl[(decideLinkNum-1)-i];
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[i], &posTbl[i]);
        posTbl[i].y += 100;
        masuMax++;
    }
    if(masuMax < 1) {
        masuMax = 1;
    }
    masuId = GwPlayer[playerNo].masuId;
    masuIdTbl[masuMax] = masuId;
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[masuMax], &posTbl[masuMax]);
    posTbl[masuMax].y += 100;
    HuPrcVSleep();
    for(i=masuMax+1; i<64; i++) {
        masuIdTbl[i] = MBCapsuleMasuNextRegularGet(masuId, &posTbl[i]);
        if(masuIdTbl[i] >= 1) {
            posTbl[i].y += 100;
        } else {
            if(i != masuMax) {
                posTbl[i] = posTbl[i-1];
            } else {
                posTbl[i] = posTbl[masuMax];
                posTbl[i].z -= 100;
            }
            masuIdTbl[i] = MASU_NULL;
            break;
        }
        masuId = masuIdTbl[i];
    }
    masuIdTbl[i] = MASU_NULL;
    if(i > 0) {
        posTbl[i] = posTbl[i-1];
    }
    if(decideLinkNum <= 0) {
        masuIdTbl[0] = GwPlayer[playerNo].masuId;
        VECSubtract(&posTbl[2], &posTbl[1], &posTbl[0]);
        VECNormalize(&posTbl[0], &posTbl[0]);
        VECScale(&posTbl[0], &posTbl[0], -200);
        VECAdd(&posTbl[1], &posTbl[0], &posTbl[0]);
    }
    HuPrcVSleep();
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_killer, NULL, FALSE, 10, FALSE);
    MBModelDispSet(mdlId, FALSE);
    motIdRideStart = MBCapsulePlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_436);
    motIdRide = MBCapsulePlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_435);
    if(capsuleFlags.killerMultiF) {
        do {
            HuPrcVSleep();
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(killerMoveF[i]) {
                   break; 
                }
            }
        } while(i<GW_PLAYER_MAX);
    }
    walkNum = MBSaiResultGet(playerNo);
    killerMoveF[playerNo] = TRUE;
    MBCapsuleStatusPosMoveWait(TRUE, FALSE);
    if(!capsuleFlags.killerMultiF) {
        MBCameraSkipSet(TRUE);
        MBCameraViewNoSet(playerNo, MB_CAMERA_VIEW_WALK);
    }
    mdlPos = posTbl[0];
    VECSubtract(&posTbl[1], &posTbl[0], &dir);
    mdlRot.x = mdlRot.z = 0;
    mdlRot.y = HuAtan(dir.x, dir.z);
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelRotSetV(mdlId, &mdlRot);
    MBModelDispSet(mdlId, TRUE);
    MBCapsuleDustCloudAdd(playerNo, mdlPos);
    MBAudFXPlay(MSM_SE_BOARD_35);
    seNo = MBAudFXPlay(MSM_SE_BOARD_54);
    for(i=0; i<decideLinkNum-1; i++) {
        masuId = masuIdTbl[i];
        masuIdNext = masuIdTbl[i+1];
        masuPos = posTbl[i];
        masuPosNext = posTbl[i+1];
        prevPos = masuPos;
        VECSubtract(&masuPosNext, &masuPos, &dir);
        mag = VECMag(&dir);
        stepNum = mag/20;
        for(j=1; j<=stepNum; j++) {
            t = (float)j/stepNum;
            if(i < 1) {
                MBCapsuleHermiteGetV(t, &posTbl[i], &posTbl[i], &posTbl[i+1], &posTbl[i+2], &mdlPos);
            } else {
                MBCapsuleHermiteGetV(t, &posTbl[i-1], &posTbl[i], &posTbl[i+1], &posTbl[i+2], &mdlPos);
            }
            VECSubtract(&mdlPos, &prevPos, &dir);
            mdlRot.y = HuAtan(dir.x, dir.z);
            mdlRot.x = HuAtan(dir.y, HuMagXZVecF(&dir));
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            MBModelRotSet(mdlId, -mdlRot.x, mdlRot.y, mdlRot.z);
            MBCapsuleEffHookCall(5, mdlId, FALSE, FALSE, FALSE);
            HuPrcVSleep();
        }
    }
    masuPosFirst = playerPos;
    MBPlayerMotionNoSet(playerNo, motIdRideStart, HU3D_MOTATTR_NONE);
    masuId = masuIdTbl[masuMax-1];
    masuIdNext = masuIdTbl[masuMax];
    masuPos = posTbl[masuMax-1];
    masuPosNext = posTbl[masuMax];
    prevPos = masuPos;
    VECSubtract(&masuPosNext, &masuPos, &dir);
    mag = VECMag(&dir);
    stepNum = mag/20;
    for(j=1; j<=stepNum; j++) {
        t = (float)j/stepNum;
        if(i < 1) {
            MBCapsuleHermiteGetV(t, &posTbl[i], &posTbl[i], &posTbl[i+1], &posTbl[i+2], &mdlPos);
        } else {
            MBCapsuleHermiteGetV(t, &posTbl[i-1], &posTbl[i], &posTbl[i+1], &posTbl[i+2], &mdlPos);
        }
        VECSubtract(&mdlPos, &prevPos, &dir);
        mdlRot.y = HuAtan(dir.x, dir.z);
        mdlRot.x = HuAtan(dir.y, HuMagXZVecF(&dir));
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelRotSet(mdlId, -mdlRot.x, mdlRot.y, mdlRot.z);
        MBCapsuleEffHookCall(5, mdlId, FALSE, FALSE, FALSE);
        
        mtxRot(rotMtx, -mdlRot.x, mdlRot.y, mdlRot.z);
        dir.x = 0;
        dir.y = 25;
        dir.z = -50;
        MTXMultVec(rotMtx, &dir, &dir);
        VECAdd(&mdlPos, &dir, &dir);
        VECSubtract(&dir, &masuPosFirst, &dir);
        VECScale(&dir, &dir, t);
        VECAdd(&masuPosFirst, &dir, &playerPos);
        playerPos.y += (HuSin(t*180)*100)*2;
        playerRot.y = MBCapsuleAngleSumLerp(t, playerRot.y, mdlRot.y);
        MBPlayerPosSetV(playerNo, &playerPos);
        MBPlayerRotSetV(playerNo, &playerRot);
        HuPrcVSleep();
    }
    killerMoveF[playerNo] = FALSE;
    MBPlayerMotionNoSet(playerNo, motIdRide, HU3D_MOTATTR_LOOP);
    if(walkNum > 64) {
        walkNum = 64;
    }
    speed=20;
    coinTotal=0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        hitFlag[i] = FALSE;
        coinDisp[i] = -1;
    }
    for(i=0, masuNo=masuMax; i<walkNum; masuNo++) {
        masuId = masuIdTbl[masuNo];
        masuIdNext = masuIdTbl[masuNo+1];
        if(masuIdNext <= 0 || MBCapsuleMasuNextGet(masuIdNext, NULL) <= 0) {
            playerRot.x = playerRot.y = playerRot.z = 0;
            break;
        }
        masuPosNext = posTbl[masuNo];
        VECSubtract(&masuPosNext, &mdlPos, &dir);
        if(i == 0) {
            faceRot.y = HuAtan(dir.x, dir.z);
            faceRot.x = HuAtan(dir.y, HuMagXZVecF(&dir));
            prevPos.x = mdlPos.x-(HuSin(mdlRot.y)*speed);
            prevPos.z = mdlPos.z-(HuCos(mdlRot.y)*speed);
        } else {
            faceRot = mdlEndRot;
        }
        VECSubtract(&masuPosNext, &mdlPos, &dir);
        mag = VECMag(&dir);
        VECSubtract(&posTbl[masuNo+1], &posTbl[masuNo], &dir);
        mag = VECMag(&dir);
        stepNum = mag/20;
        for(j=0; j<stepNum; j++) {
            t = (float)j/stepNum;
            if(masuNo < 1) {
                MBCapsuleHermiteGetV(t, &posTbl[masuNo], &posTbl[masuNo], &posTbl[masuNo+1], &posTbl[masuNo+2], &mdlPos);
            } else {
                MBCapsuleHermiteGetV(t, &posTbl[masuNo-1], &posTbl[masuNo], &posTbl[masuNo+1], &posTbl[masuNo+2], &mdlPos);
            }
            VECSubtract(&mdlPos, &prevPos, &dir);
            faceEndRot.y = HuAtan(dir.x, dir.z);
            faceEndRot.x = HuAtan(dir.y, HuMagXZVecF(&dir));
            tempVec.x = MBCapsuleAngleWrap(faceEndRot.x, faceRot.x);
            tempVec.y = MBCapsuleAngleWrap(faceEndRot.y, faceRot.y);
            t = 2.0*t;
            if(t > 1) {
                t = 1;
            }
            mdlEndRot.x = faceRot.x+(t*tempVec.x);
            mdlEndRot.y = faceRot.y+(t*tempVec.y);
            if((MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId) & MASU_FLAG_CLIMBFROM) && (MBMasuFlagGet(MASU_LAYER_DEFAULT, masuIdNext) & MASU_FLAG_CLIMBTO)) {
                masuIdClimb = masuIdTbl[masuNo+1];
                masuIdClimbNext = masuIdTbl[masuNo+2];
                if(masuIdClimb <= 0 || masuIdClimbNext <= 0) {
                    masuIdClimb = masuId;
                    masuIdClimbNext = masuIdNext;
                }
                MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdClimb, &dir);
                MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdClimbNext, &tempVec);
                VECSubtract(&tempVec, &dir, &dir);
                mdlEndRot.y = HuAtan(dir.x, dir.z);
            }
            mdlRot.x = MBCapsuleAngleLerp(mdlEndRot.x, mdlRot.x, 3.5f);
            mdlRot.y = MBCapsuleAngleLerp(mdlEndRot.y, mdlRot.y, 3.5f);
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            MBModelRotSet(mdlId, -mdlRot.x, mdlRot.y, mdlRot.z);
            mtxRot(rotMtx, -mdlRot.x, mdlRot.y, mdlRot.z);
            dir.x = 0;
            dir.y = 25;
            dir.z = -50;
            MTXMultVec(rotMtx, &dir, &dir);
            VECAdd(&mdlPos, &dir, &playerPos);
            playerRot = mdlRot;
            MBPlayerPosSetV(playerNo, &playerPos);
            MBPlayerRotSet(playerNo, -playerRot.x, playerRot.y, playerRot.z);
            mtxRot(rotMtx, -mdlRot.x, mdlRot.y, mdlRot.z);
            dir.x = 0;
            dir.y = 0;
            dir.z = -100;
            MTXMultVec(rotMtx, &dir, &dir);
            VECAdd(&mdlPos, &dir, &dir);
            VECSubtract(&mdlPos, &prevPos, &vel);
            VECNormalize(&vel, &vel);
            VECScale(&vel, &vel, 10);
            t = MBCapsuleEffRandF();
            color.r = (32+(t*32));
            color.g = (32+(t*32));
            color.b = (32+(t*32));
            color.a = (192+(MBCapsuleEffRandF()*63));
            MBCapsuleExplodeEffLineAdd(playerNo, dir, vel, 
                ((MBCapsuleEffRandF()*0.5f)+1.0f)*100,
                2.5f,
                ((MBCapsuleEffRandF()*0.25f)+0.25f)*100,
                (MBCapsuleEffRandF()*0.66f)+0.33f,
                color);
            MBCapsuleEffHookCall(5, mdlId, FALSE, FALSE, FALSE);
            VECSubtract(&masuPosNext, &mdlPos, &dir);
            prevPos = mdlPos;
            HuPrcVSleep();
        }
        if(MBMasuTypeGet(MASU_LAYER_DEFAULT, masuIdNext) != MASU_TYPE_NONE) {
            HuAudFXPlay(MSM_SE_BOARD_02);
        }
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(j == playerNo) {
                continue;
            }
            if(hitFlag[j]) {
                continue;
            }
            if(!MBPlayerAliveCheck(j)) {
                continue;
            }
            if(capsuleFlags.killerMultiF && !MBKillerEventCheck(j)) {
                continue;
            }
            if(masuIdNext == GwPlayer[j].masuId) {
                MBAudFXPlay(MSM_SE_BOARD_55);
                MBPlayerMoveHitCreate(j, TRUE, TRUE);
                CharFXPlay(GwPlayer[j].charNo, CHARVOICEID(7));
                omVibrate(j, 12, 4, 2);
                if(GWPartyFGet() != FALSE) {
                   coinNum = 30; 
                } else {
                   coinNum = 10;
                }
                if(MBPlayerCoinGet(j) < coinNum) {
                    coinNum = MBPlayerCoinGet(j);
                }
                if(coinNum > 0) {
                    MBPlayerPosGet(j, &dir);
                    dir.y += 250;
                    coinDisp[j] = MBCoinDispCreateSe(&dir, -coinNum);
                    MBPlayerCoinAdd(j, -coinNum);
                    MBPlayerCoinAdd(playerNo, coinNum);
                    coinTotal += coinNum;
                }
                hitFlag[j] = TRUE;
            }
            
        }
        GwPlayer[playerNo].masuIdPrev = GwPlayer[playerNo].masuId;
        GwPlayer[playerNo].masuId = masuIdNext;
        masuType = MBMasuTypeGet(MASU_LAYER_DEFAULT, masuIdNext);
        masuFlag = MBMasuFlagGet(MASU_LAYER_DEFAULT, masuIdNext);
        if(masuType != MASU_TYPE_NONE && masuType != MASU_TYPE_STAR && !(masuFlag & (MASU_FLAG_START|MASU_FLAG_BLOCKR))) {
            GwPlayer[playerNo].walkNum--;
            i++;
        }
    }
    MBWalkNumKill(playerNo);
    MBCapsuleEffHookCall(6, mdlId, FALSE, FALSE, FALSE);
    MBModelDispSet(mdlId, FALSE);
    MBCapsuleDustCloudAdd(playerNo, mdlPos);
    MBAudFXPlay(MSM_SE_BOARD_35);
    if(seNo != MSM_SENO_NONE) {
        MBAudFXStop(seNo);
    }
    MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_JUMP, HU3D_MOTATTR_NONE);
    masuPosFirst = playerPos;
    for(i=0; i<9; i++) {
        MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId, &masuPosNext);
        VECAdd(&masuPosNext, &ofsTbl[i], &masuPosNext);
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(j == playerNo) {
                continue;
            }
            if(!MBPlayerAliveCheck(j)) {
                continue;
            }
            MBPlayerPosGet(j, &dir);
            VECSubtract(&masuPosNext, &dir, &dir);
            if(VECMag(&dir) < 95) {
                break;
            }
        }
        if(j >= GW_PLAYER_MAX) {
            break;
        }
    }
    for(i=0; i<30.0f; i++) {
        t = i/30.0f;
        VECSubtract(&masuPosNext, &masuPosFirst, &dir);
        VECScale(&dir, &dir, t);
        VECAdd(&masuPosFirst, &dir, &playerPos);
        playerPos.y += (HuSin(t*180)*100)*2;
        playerRot.x = MBCapsuleAngleSumLerp(t, playerRot.x, 0);
        playerRot.y = MBCapsuleAngleSumLerp(t, playerRot.y, 0);
        playerRot.z = MBCapsuleAngleSumLerp(t, playerRot.z, 0);
        MBPlayerPosSetV(playerNo, &playerPos);
        MBPlayerRotSetV(playerNo, &playerRot);
        HuPrcVSleep();
    }
    MBPlayerPosSetV(playerNo, &masuPosNext);
    MBPlayerMotIdleSet(playerNo);
    MBCameraMotionWait();
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    if(coinTotal > 0) {
        MBCapsuleCoinDispExec(playerNo, coinTotal, TRUE, TRUE);
    }
    if(!capsuleFlags.killerMultiF) {
        MBPlayerPosFixPlayer(playerNo, GwPlayer[playerNo].masuId, FALSE);
        MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId, &playerPos);
        MBPlayerPosGet(playerNo, &dir);
        VECSubtract(&playerPos, &dir, &dir);
        if(VECMag(&dir) > 25) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[playerNo].masuId, &playerPos);
            MBPlayerWalkExec(playerNo, NULL, &playerPos, 30, NULL, TRUE);
            MBPlayerRotateStart(playerNo, 0, 15);
            while(!MBPlayerRotateCheck(playerNo)) {
                HuPrcVSleep();
            }
        }
    }
}

void MBCapsuleKillerDestroy(void)
{
    MBCapsuleExplodeEffKill(-1);
}

void MBCapsuleKillerMoveP1Exec(void)
{
    MBCapsuleExplodeEffCreate(0);
    MBCapsuleKillerMoveExec(0, FALSE);
    HuPrcEnd();
}

void MBCapsuleKillerMoveP2Exec(void)
{
    MBCapsuleExplodeEffCreate(1);
    MBCapsuleKillerMoveExec(1, FALSE);
    HuPrcEnd();
}

void MBCapsuleKillerMoveP3Exec(void)
{
    MBCapsuleExplodeEffCreate(2);
    MBCapsuleKillerMoveExec(2, FALSE);
    HuPrcEnd();
}

void MBCapsuleKillerMoveP4Exec(void)
{
    MBCapsuleExplodeEffCreate(3);
    MBCapsuleKillerMoveExec(3, FALSE);
    HuPrcEnd();
}

void MBCapsuleKillerMoveP1Destroy(void)
{
    int i;
    MBCapsuleExplodeEffKill(0);
    eventKillerProc[0] = NULL;
    if(MBKillerEventCheckAll()) {
        if(!MBKillCheck()) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerAliveCheck(i)) {
                    MBPlayerMotIdleSet(i);
                }
            }
        }
        MBPlayerMoveObjClose();
        HuDataDirClose(capsuleData[CAPSULE_KILLER].dirNum);
        CapsuleDataClose();
        _ClearFlag(FLAG_BOARD_TURN_NOSTART);
    }
}

void MBCapsuleKillerMoveP2Destroy(void)
{
    int i;
    MBCapsuleExplodeEffKill(1);
    eventKillerProc[1] = NULL;
    if(MBKillerEventCheckAll()) {
        if(!MBKillCheck()) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerAliveCheck(i)) {
                    MBPlayerMotIdleSet(i);
                }
            }
        }
        MBPlayerMoveObjClose();
        HuDataDirClose(capsuleData[CAPSULE_KILLER].dirNum);
        CapsuleDataClose();
        _ClearFlag(FLAG_BOARD_TURN_NOSTART);
    }
}

void MBCapsuleKillerMoveP3Destroy(void)
{
    int i;
    MBCapsuleExplodeEffKill(2);
    eventKillerProc[2] = NULL;
    if(MBKillerEventCheckAll()) {
        if(!MBKillCheck()) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerAliveCheck(i)) {
                    MBPlayerMotIdleSet(i);
                }
            }
        }
        MBPlayerMoveObjClose();
        HuDataDirClose(capsuleData[CAPSULE_KILLER].dirNum);
        CapsuleDataClose();
        _ClearFlag(FLAG_BOARD_TURN_NOSTART);
    }
}

void MBCapsuleKillerMoveP4Destroy(void)
{
    int i;
    MBCapsuleExplodeEffKill(3);
    eventKillerProc[3] = NULL;
    if(MBKillerEventCheckAll()) {
        if(!MBKillCheck()) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerAliveCheck(i)) {
                    MBPlayerMotIdleSet(i);
                }
            }
        }
        MBPlayerMoveObjClose();
        HuDataDirClose(capsuleData[CAPSULE_KILLER].dirNum);
        CapsuleDataClose();
        _ClearFlag(FLAG_BOARD_TURN_NOSTART);
    }
}

void MBCapsuleKuriboExec(void)
{
    int mdlId;
    int i;
    int swapPlayer;
    int coinNum;
    int daizaMdlId; 
    
    float t;
    
    int motFile[8];
    HuVecF pos2D;
    HuVecF pos;
    int coinTotal[2];
    
    MBCapsuleCoinManCreate();
    motFile[0] = CAPSULECHAR2_HSF_kuribo_idle;
    motFile[1] = CAPSULECHAR2_HSF_kuribo_jump;
    motFile[2] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR2_HSF_kuribo, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    daizaMdlId = MBCapsuleModelCreate(CAPSULE_HSF_kuriboDaiza, NULL, FALSE, 10, FALSE);
    MBModelDispSet(daizaMdlId, FALSE);
    MBModelDispSet(mdlId, TRUE);
    MBModelDispSet(daizaMdlId, TRUE);
    MBAudFXPlay(MSM_SE_BOARD_32);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        pos2D.x = 288;
        pos2D.y = -150+(HuSin(t*90)*300);
        pos2D.z = 2000;
        Hu3D2Dto3D(&pos2D, HU3D_CAM0, &pos);
        MBModelPosSetV(mdlId, &pos);
        MBModelPosSetV(daizaMdlId, &pos);
        if(t < 0.75f) {
            MBCapsuleEffHookCall(8, mdlId, FALSE, FALSE, FALSE);
        }
        HuPrcVSleep();
    }
    if(!GWTeamFGet()) {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_18);
        if(MBPlayerAliveGet(NULL) >= 3) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_KURIBO_ROULETTE, HUWIN_SPEAKER_KURIBO);
            MBTopWinWait();
            MBTopWinKill();
            if(MBRouletteCreate(capsulePlayer, 3)) {
                MBRouletteWait();
            }
            swapPlayer = MBRouletteResultGet();
        } else {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_KURIBO, HUWIN_SPEAKER_KURIBO);
            MBTopWinWait();
            MBTopWinKill();
            swapPlayer = MBCapsulePlayerAliveFind(capsulePlayer);
        }
    } else {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_18);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_KURIBO_TEAM, HUWIN_SPEAKER_KURIBO);
        MBTopWinWait();
        MBTopWinKill();
        if(capsulePlayer == MBPlayerGrpFindPlayer(0, 0) || capsulePlayer == MBPlayerGrpFindPlayer(0, 1)) {
            swapPlayer = MBPlayerGrpFindPlayer(1, 0);
        } else {
            swapPlayer = MBPlayerGrpFindPlayer(0, 0);
        }
    }
    coinTotal[0] = MBPlayerCoinGet(capsulePlayer);
    coinTotal[1] = MBPlayerCoinGet(swapPlayer);
    if(!GWTeamFGet()) {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_KURIBO_SWAP, HUWIN_SPEAKER_KURIBO);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(swapPlayer), 0);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleStatusPosMoveWait(0, 1);
    MBCapsuleStatusTradeInSet(capsulePlayer, swapPlayer, TRUE);
    if(coinTotal[0] > 0 || coinTotal[1] > 0) {
        do {
            if(coinTotal[0] > 0) {
                coinNum = MBCapsuleCoinManAdd(FALSE, swapPlayer, 1);
                if(coinNum) {
                    MBPlayerCoinAdd(capsulePlayer, -coinNum);
                    coinTotal[0] -= coinNum;
                }
            }
            if(coinTotal[1] > 0) {
                coinNum = MBCapsuleCoinManAdd(TRUE, capsulePlayer, 1);
                if(coinNum) {
                    MBPlayerCoinAdd(swapPlayer, -coinNum);
                    coinTotal[1] -= coinNum;
                }
            }
            HuPrcVSleep();
        } while(coinTotal[0] > 0 || coinTotal[1] > 0 || MBCapsuleCoinManNumGet() > 0);
    } else {
        MBAudFXPlay(MSM_SE_GUIDE_19);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_KURIBO_NOCOIN, HUWIN_SPEAKER_KURIBO);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBCapsuleStatusTradeOutSet(capsulePlayer, swapPlayer, TRUE);
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_KURIBO_END, HUWIN_SPEAKER_KURIBO);
    MBTopWinWait();
    MBTopWinKill();
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBAudFXPlay(MSM_SE_BOARD_32);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        pos2D.x = 288;
        pos2D.y = -150+(HuCos(t*90)*300);
        pos2D.z = 2000;
        Hu3D2Dto3D(&pos2D, HU3D_CAM0, &pos);
        MBModelPosSetV(mdlId, &pos);
        MBModelPosSetV(daizaMdlId, &pos);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBModelDispSet(daizaMdlId, FALSE);
    HuPrcEnd();
}

void MBCapsuleKuriboDestroy(void)
{
    MBCapsuleCoinManKill();
}

void MBCapsuleBomheiExec(void)
{
    int i; //r31
    int mdlId; //r30
    int num; //r23
    
    float t; //f31
    float scale; //f30
    float rot; //f29
    
    HuVecF playerPos; //sp+0xD4
    HuVecF numberPos; //sp+0xC8
    HuVecF bomheiPos; //sp+0xBC
    HuVecF coinPos; //sp+0xB0
    int bomheiMdlId[3]; //sp+0xA4
    int coinDisp[GW_PLAYER_MAX]; //sp+0x94
    int numberMdlId[2]; //sp+0x8C
    int playerMotId[2]; //sp+0x84
    int masuId; //sp+0x80
    int nextNum; //sp+0x7C
    int fuseMdlId; //sp+0x78
    int masuType; //sp+0x74
    int coinNum; //sp+0x70
    
    masuId = GwPlayer[capsulePlayer].masuId;
    masuType = MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId);
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerMotId[0] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_439);
    playerMotId[1] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_440);
    MBPlayerMoveInit();
    MBPlayerMoveObjCreate();
    if(capsuleCircuitF) {
        num = 0;
    } else {
        num = MBMasuBomheiNumGet(masuId);
    }
    if(num < 1) {
        num = 1;
    } else if(num > 9) {
        num = 9;
    }
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    MBCameraModelViewSet(MBPlayerModelGet(capsulePlayer), NULL, &capsuleViewOfs, 1800, -1, 21);
    MBCameraMotionWait();
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    numberMdlId[0] = MBCapsuleModelCreate(numberFileTbl[num], NULL, FALSE, 10, FALSE);
    MBModelLayerSet(numberMdlId[0], 2);
    MBModelDispSet(numberMdlId[0], FALSE);
    if((nextNum = num-1) < 0) {
        nextNum = 0;
    }
    numberMdlId[1] = MBCapsuleModelCreate(numberFileTbl[nextNum], NULL, FALSE, 10, FALSE);
    MBModelLayerSet(numberMdlId[1], 2);
    MBModelDispSet(numberMdlId[1], FALSE);
    mdlId = numberMdlId[0];
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        numberPos.x = playerPos.x;
        numberPos.y = playerPos.y+(300*t);
        numberPos.z = playerPos.z+100;
        scale = 2-t;
        rot = 360*t;
        MBModelDispSet(mdlId, TRUE);
        MBModelPosSet(mdlId, numberPos.x, numberPos.y, numberPos.z);
        MBModelRotSet(mdlId, 0, rot, 0);
        MBModelScaleSet(mdlId, 1, scale, 1);
        MBModelAlphaSet(mdlId, t*255);
        HuPrcVSleep();
    }
    for(i=0; i<15.0f; i++) {
        HuPrcVSleep();
    }
    num--;
    if(num < 0) {
        num = 0;
    }
    if((MBMasuCapsuleGet(MASU_LAYER_DEFAULT, masuId) & 0xFF) == CAPSULE_BOMHEI) {
        MBMasuBomheiNumSet(masuId, num);
    }
    if(num >= 1) {
        MBModelDispSet(numberMdlId[0], FALSE);
        MBModelDispSet(numberMdlId[1], TRUE);
        mdlId = numberMdlId[1];
        MBModelPosSet(mdlId, numberPos.x, numberPos.y, numberPos.z);
        MBModelLayerSet(mdlId, 2);
        MBAudFXPlay(MSM_SE_BOARD_57);
        for(i=0; i<30.0f; i++) {
            HuPrcVSleep();
        }
        for(i=0; i<15.0f; i++) {
            t = i/15.0f;
            numberPos.x = playerPos.x+(t*200);
            scale = 1-t;
            MBModelPosSet(mdlId, numberPos.x, numberPos.y, numberPos.z);
            MBModelScaleSet(mdlId, scale, 1+(0.5f*t), 1);
            MBModelAlphaSet(mdlId, (1-t)*255);
            HuPrcVSleep();
        }
    } else {
        MBModelDispSet(numberMdlId[0], FALSE);
        MBModelDispSet(numberMdlId[1], TRUE);
        mdlId = numberMdlId[1];
        MBModelPosSet(mdlId, numberPos.x, numberPos.y, numberPos.z);
        MBModelLayerSet(mdlId, 2);
        MBAudFXPlay(MSM_SE_BOARD_58);
        for(i=0; i<30.0f; i++) {
            HuPrcVSleep();
        }
        for(i=0; i<15.0f; i++) {
            t = i/15.0f;
            scale = 1+(t*3);
            MBModelScaleSet(mdlId, scale, scale, 1);
            MBModelAlphaSet(mdlId, (1-t)*255);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        bomheiMdlId[0] = MBCapsuleModelCreate(CAPSULECHAR0_HSF_bomhei, NULL, FALSE, 10, FALSE);
        MBModelLayerSet(bomheiMdlId[0], 2);
        MBModelDispSet(bomheiMdlId[0], FALSE);
        fuseMdlId = MBCapsuleModelCreate(CAPSULECHAR0_HSF_bomhei_fuse, NULL, FALSE, 10, FALSE);
        MBModelAttrSet(fuseMdlId, HU3D_MOTATTR_LOOP);
        MBModelLayerSet(fuseMdlId, 3);
        MBModelDispSet(fuseMdlId, FALSE);
        MBModelHookSet(bomheiMdlId[0], "itemhook_c", fuseMdlId);
        bomheiMdlId[1] = MBCapsuleModelCreate(CAPSULECHAR0_HSF_bomhei_flash, NULL, FALSE, 10, FALSE);
        MBModelLayerSet(bomheiMdlId[1], 2);
        MBModelDispSet(bomheiMdlId[1], FALSE);
        bomheiMdlId[2] = MBCapsuleModelCreate(CAPSULECHAR0_HSF_bomhei_explode, NULL, FALSE, 10, FALSE);
        MBModelLayerSet(bomheiMdlId[2], 3);
        MBModelDispSet(bomheiMdlId[2], FALSE);
        if(playerMotId[0] != MB_MOT_NONE) {
            MBPlayerMotionNoShiftSet(capsulePlayer, playerMotId[0], 0, 8, HU3D_MOTATTR_NONE);
        }
        MBModelDispSet(bomheiMdlId[0], TRUE);
        MBModelDispSet(fuseMdlId, TRUE);
        for(i=0; i<40; i++) {
            t = i/40.0f;
            bomheiPos.x = playerPos.x;
            bomheiPos.y = (playerPos.y+100.0f)+(1000*HuCos(t*90));
            bomheiPos.z = playerPos.z+50;
            MBModelPosSet(bomheiMdlId[0], bomheiPos.x, bomheiPos.y, bomheiPos.z);
            MBCapsuleEffHookCall(8, bomheiMdlId[0], FALSE, FALSE, FALSE);
            HuPrcVSleep();
        }
        MBModelPosSet(bomheiMdlId[0], 0, 0, 0);
        MBModelHookSet(MBPlayerModelGet(capsulePlayer), CharModelItemHookGet(GwPlayer[capsulePlayer].charNo, CHAR_MODEL1, 0), bomheiMdlId[0]);
        CharFXPlay(GwPlayer[capsulePlayer].charNo, CHARVOICEID(6));
        while(!MBPlayerMotionEndCheck(capsulePlayer)) {
            HuPrcVSleep();
        }
        if(playerMotId[1] != MB_MOT_NONE) {
            MBPlayerMotionNoShiftSet(capsulePlayer, playerMotId[1], 0, 8, HU3D_MOTATTR_NONE);
        }
        MBAudFXPlay(MSM_SE_GUIDE_26);
        MBModelHookReset(MBPlayerModelGet(capsulePlayer));
        MBModelHookReset(bomheiMdlId[0]);
        MBModelDispSet(bomheiMdlId[0], FALSE);
        MBModelAttrSet(bomheiMdlId[0], HU3D_ATTR_DISPOFF);
        MBModelDispSet(bomheiMdlId[1], TRUE);
        MBModelAttrReset(bomheiMdlId[1], HU3D_ATTR_DISPOFF);
        MBModelPosSet(bomheiMdlId[1], bomheiPos.x, bomheiPos.y, bomheiPos.z);
        MBModelAttrSet(bomheiMdlId[1], HU3D_MOTATTR_LOOP);
        MBModelHookSet(bomheiMdlId[1], "itemhook_c", fuseMdlId);
        MBModelPosSet(bomheiMdlId[1], 0, 0, 0);
        MBModelHookSet(MBPlayerModelGet(capsulePlayer), CharModelItemHookGet(GwPlayer[capsulePlayer].charNo, CHAR_MODEL1, 0), bomheiMdlId[1]);
        do {
            HuPrcVSleep();
        } while(!MBCapsulePlayerMotShiftCheck(capsulePlayer) || !MBPlayerMotionEndCheck(capsulePlayer));
        MBModelHookReset(MBPlayerModelGet(capsulePlayer));
        MBModelHookReset(bomheiMdlId[1]);
        MBModelDispSet(bomheiMdlId[1], FALSE);
        MBModelDispSet(fuseMdlId, FALSE);
        MBModelAttrReset(bomheiMdlId[2], HU3D_ATTR_DISPOFF);
        MBModelDispSet(bomheiMdlId[2], TRUE);
        MBModelPosSet(bomheiMdlId[2], bomheiPos.x, bomheiPos.y, bomheiPos.z);
        MBModelScaleSet(bomheiMdlId[2], 1.2f, 1.2f, 1.2f);
        MBMotionTimeSet(bomheiMdlId[2], 5);
        MBMotionSpeedSet(bomheiMdlId[2], 1);
        MBAudFXPlay(MSM_SE_BOARD_68);
        MBCapsuleEffHookCall(6, bomheiMdlId[2], FALSE, FALSE, FALSE);
        if(!capsuleCircuitF) {
            MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, CAPSULE_NULL);
        }
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(!MBPlayerAliveCheck(i)) {
                continue;
            }
            coinDisp[i] = -1;
            if(masuId == GwPlayer[i].masuId) {
                MBPlayerMoveHitCreate(i, TRUE, TRUE);
                CharFXPlay(GwPlayer[i].charNo, CHARVOICEID(7));
                omVibrate(i, 12, 4, 2);
                coinNum = 20;
                if(MBPlayerCoinGet(i) < coinNum) {
                    coinNum = MBPlayerCoinGet(i);
                }
                if(coinNum > 0) {
                    MBPlayerPosGet(i, &coinPos);
                    coinPos.y += 250;
                    coinDisp[i] = MBCoinDispCreateSe(&coinPos, -coinNum);
                    MBPlayerCoinAdd(i, -coinNum);
                }
            }
        }
        HuPrcSleep(90);
        do {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(!MBPlayerAliveCheck(i)) {
                    continue;
                }
                if(!MBPlayerMoveObjCheck(i)) {
                    break;
                }
                if(coinDisp[i] != -1 && !MBCoinDispKillCheck(coinDisp[i])) {
                    break;
                }
            }
            HuPrcVSleep();
        } while(i < GW_PLAYER_MAX);
        
    }
    MBCapsulePlayerIdleWait();
    MBPlayerMoveObjClose();
    HuPrcEnd();
}

void MBCapsuleBomheiDestroy(void)
{
    
}

void MBCapsuleBankExec(void)
{
    int mdlId; //r31
    int i; //r28
    int coinNum; //r25
    int daizaMdlId; //r24
    int decTime; //r23
    int coinTotal; //r22
    int j; //r19
    int coinId; //r18
    
    float t; //f31
    float ySpeed; //f30
    float yOfs; //f29
    
    int motFile[8]; //sp+0x50
    HuVecF playerPos; //sp+0x44
    HuVecF coinPos; //sp+0x38
    HuVecF coinVel; //sp+0x2C
    HuVecF mdlPos; //sp+0x20
    char mes[16]; //sp+0x10
    int masuId; //sp+0xC
    
    MBCapsuleGlowEffCreate();
    MBCapsuleCoinEffCreate();
    
    motFile[0] = CAPSULECHAR0_HSF_nokonoko_idle;
    motFile[1] = CAPSULECHAR0_HSF_nokonoko_fly_start;
    motFile[2] = CAPSULECHAR0_HSF_nokonoko_fly;
    motFile[3] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR0_HSF_nokonoko, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, TRUE, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    daizaMdlId = MBCapsuleModelCreate(CAPSULE_HSF_hammerBroDaiza, motFile, FALSE, 10, FALSE);
    MBModelDispSet(daizaMdlId, FALSE);
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    MBAudFXPlay(MSM_SE_BOARD_32);
    MBModelDispSet(mdlId, TRUE);
    MBModelDispSet(daizaMdlId, TRUE);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = (playerPos.y+1000)+(-770*HuSin(t*90));
        mdlPos.z = -100+playerPos.z;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelPosSetV(daizaMdlId, &mdlPos);
        if(t < 0.75f) {
            MBCapsuleEffHookCall(8, mdlId, FALSE, FALSE, FALSE);
        }
        HuPrcVSleep();
    }
    if(capsuleMasuF) {
        coinNum = 5;
        if(MBPlayerCoinGet(capsulePlayer) < coinNum) {
            coinNum = MBPlayerCoinGet(capsulePlayer);
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_23);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_BANK_COST, HUWIN_SPEAKER_NOKO2);
        MBTopWinWait();
        MBTopWinKill();
        if(coinNum > 0) {
            MBCoinAddExec(capsulePlayer, -coinNum);
            MBCapsuleCoinDispExec(capsulePlayer, -coinNum, TRUE, TRUE);
            MBBankCoinAdd(coinNum);
            sprintf(mes, "%d", GwSystem.bankCoin);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_BANK_COIN_NUM, HUWIN_SPEAKER_NOKO2);
            MBTopWinInsertMesSet(MESSNUM_PTR(mes), 0);
            MBTopWinWait();
            MBTopWinKill();
        } else {
            MBAudFXPlay(MSM_SE_GUIDE_24);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_BANK_FAIL, HUWIN_SPEAKER_NOKO2);
            MBTopWinWait();
            MBTopWinKill();
        }
    } else {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_23);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_BANK_GETALL, HUWIN_SPEAKER_NOKO2);
        MBTopWinWait();
        MBTopWinKill();
        if(GwSystem.bankCoin) {
            MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
            decTime=0;
            coinTotal=GwSystem.bankCoin;
            for(i=0; i<GwSystem.bankCoin; i++) {
                coinPos = mdlPos;
                coinPos.x += (0.5f-MBCapsuleEffRandF())*100;
                coinPos.y += 50;
                coinPos.z += 30.000002f;
                
                for(ySpeed=0, yOfs=coinPos.y, j=0; ySpeed > 0 || yOfs > playerPos.y+150; j++) {
                    yOfs += ySpeed;
                    ySpeed -= 4.900000194708507;
                }
                coinVel.x = 0;
                coinVel.z = (1.0f/j)*(playerPos.z-coinPos.z);
                coinVel.y = 0;
                coinId = MBCapsuleCoinEffAdd(&coinPos, &coinVel, 0.75f, 4.9f, 30, 1);
                if(coinId != -1){
                    MBCapsuleCoinEffMaxYSet(coinId, playerPos.y+150);
                }
                if(++decTime >= 6.0f && --coinTotal >= 0) {
                    MBAudFXPlay(MSM_SE_CMN_19);
                }
                HuPrcVSleep();
            }
            do {
                if(++decTime >= 6.0f && --coinTotal >= 0) {
                    MBAudFXPlay(MSM_SE_CMN_19);
                }
                HuPrcVSleep();
            } while(MBCapsuleCoinEffNumGet());
            MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
            MBCoinAddExec(capsulePlayer, GwSystem.bankCoin);
            MBCapsuleCoinDispExec(capsulePlayer, GwSystem.bankCoin, TRUE, TRUE);
        } else {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_24);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_BANK_EMPTY, HUWIN_SPEAKER_NOKO2);
            MBTopWinWait();
            MBTopWinKill();
        }
        GwSystem.bankCoin = 0;
    }
    MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX02_BANK_END, HUWIN_SPEAKER_NOKO2);
    MBTopWinWait();
    MBTopWinKill();
    MBAudFXPlay(MSM_SE_BOARD_32);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = (playerPos.y+1000)+(-770*HuCos(t*90));
        mdlPos.z = -100+playerPos.z;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelPosSetV(daizaMdlId, &mdlPos);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBModelDispSet(daizaMdlId, FALSE);
    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    HuPrcEnd();
}

void MBCapsuleBankDestroy(void)
{
    MBCapsuleGlowEffKill();
    MBCapsuleCoinEffKill();
}

static void KamekkuObjExec(OMOBJ *obj);

typedef struct KamekkuMagicWork_s {
    int mdlId;
    int mode;
    int mdlNo;
    int time;
    BOOL hookF;
    BOOL effectF;
    BOOL idleStartF;
    float tpLvl[16];
    HuVecF pos[16];
} KAMEKKU_OBJ_WORK;

typedef struct KamekkuCapsuleWork_s {
    int sprId;
    BOOL flag;
    int capsuleNo;
    int playerNo;
    int capsuleIdx;
    HuVecF sprPos;
    HuVecF startPos;
    HuVecF endPos;
} KAMEKKU_CAPSULE_WORK;

void MBCapsuleKamekkuExec(void)
{
    KAMEKKU_CAPSULE_WORK *capWorkP; //r31
    int i; //r30
    int no; //r29
    KAMEKKU_OBJ_WORK *objWork; //r28
    OMOBJ *obj; //r27
    int mdlId; //r26
    int playerNo; //r25
    int capsuleTotal; //r24
    int capWorkNum; //r22
    int dstIdx; //r20
    int capsuleNo; //r17
    
    float t; //f31
    float angle; //f30
    float rotOfs; //f29
    float rotSpeed; //f28
    
    KAMEKKU_CAPSULE_WORK capWork[24]; //sp+0xD8
    KAMEKKU_CAPSULE_WORK capWorkTemp; //sp+0xA0
    Mtx rotMtx; //sp+0x70
    int motFile[8]; //sp+0x50
    HuVecF rotPos; //sp+0x44
    HuVecF sprPos; //sp+0x38
    HuVecF mdlPos; //sp+0x2C
    int capsuleNum[GW_PLAYER_MAX]; //sp+0x1C
    int seNo[2]; //sp+0x14
    
    MBCapsuleGlowEffCreate();
    motFile[0] = CAPSULECHAR1_HSF_kamekku_wait;
    motFile[1] = CAPSULECHAR1_HSF_kamekku_up;
    motFile[2] = CAPSULECHAR1_HSF_kamekku_down;
    motFile[3] = CAPSULECHAR1_HSF_kamekku_idle;
    motFile[4] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_kamekku, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    obj = capsuleObj = MBAddObj(32768, 16, 0, KamekkuObjExec);
    obj->stat |= OM_STAT_MODELPAUSE;
    objWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(KAMEKKU_OBJ_WORK), HU_MEMNUM_OVL);
    memset(objWork, 0, sizeof(KAMEKKU_OBJ_WORK));
    objWork->mdlId = mdlId;
    objWork->mode = 0;
    objWork->mdlNo = 0;
    objWork->hookF = FALSE;
    objWork->effectF = FALSE;
    objWork->idleStartF = FALSE;
    for(i=0; i<16; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(CAPSULECHAR1_HSF_magic);
        Hu3DModelCameraSet(obj->mdlId[i], HU3D_CAM0);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_MOTATTR_LOOP);
        t = 0.25f*((16-i)/16.0f);
        Hu3DModelTPLvlSet(obj->mdlId[i], t);
        Hu3DModelLayerSet(obj->mdlId[i], 2);
        Hu3DModelDispOff(obj->mdlId[i]);
        Hu3DMotionTimeSet(obj->mdlId[i], Hu3DMotionMaxTimeGet(obj->mdlId[i])-(0.35f*i));
        objWork->tpLvl[i] = t;
    }
    memset(capWork, 0, sizeof(capWork));
    for(i=0, capWorkP=capWork; i<24; i++, capWorkP++) {
        capWorkP->sprId = -1;
    }
    MBAudFXPlay(MSM_SE_BOARD_32);
    seNo[0] = MBAudFXPlay(MSM_SE_BOARD_60);
    MBModelDispSet(mdlId, TRUE);
    for(i=0; i<=180.0f; i++) {
        t = i/180.0f;
        sprPos.x = 288;
        sprPos.y = -150+(HuSin(t*90)*300);
        sprPos.z = 2000;
        Hu3D2Dto3D(&sprPos, HU3D_CAM0, &mdlPos);
        angle = -t*720;
        mdlPos.x += (((1-t)*HuSin(angle+90))*100)*5;
        mdlPos.z += (((1-t)*HuCos(angle+90))*100)*5;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelRotSet(mdlId, 0, angle, 0);
        HuPrcVSleep();
    }
    if(seNo[0] != MSM_SENO_NONE) {
        MBAudFXStop(seNo[0]);
    }
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    MBAudFXPlay(MSM_SE_GUIDE_45);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KAMEKKU_SWAP, HUWIN_SPEAKER_KAMEKKU);
    MBTopWinWait();
    MBTopWinKill();
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    for(i=0; i<30.0f; i++) {
        HuPrcVSleep();
    }
    if(!GWTeamFGet()) {
        for(i=0, capWorkNum=0, capsuleTotal=0; i<GW_PLAYER_MAX; i++) {
            if(!MBPlayerAliveCheck(i)) {
                continue;
            }
            capsuleNum[i] = MBPlayerCapsuleNumGet(i);
            capsuleTotal += capsuleNum[i];
            for(no=MBPlayerCapsuleMaxGet()-1; no>=0; no--) {
                capsuleNo = MBPlayerCapsuleGet(i, no);
                if(capsuleNo != CAPSULE_NULL) {
                    capWork[capWorkNum].capsuleNo = capsuleNo;
                    capWork[capWorkNum].playerNo = i;
                    capWork[capWorkNum].capsuleIdx = no;
                    capWorkNum++;
                }
            }
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            capsuleNum[i] = 0;
        }
        for(i=0, capWorkNum=0, capsuleTotal=0; i<2; i++) {
            if(i == 0) {
                playerNo = MBPlayerGrpFindPlayer(0, 0);
            } else {
                playerNo = MBPlayerGrpFindPlayer(1, 0);
            }
            capsuleNum[playerNo] = MBPlayerCapsuleNumGet(playerNo);
            capsuleTotal += capsuleNum[playerNo];
            for(no=MBPlayerCapsuleMaxGet()-1; no>=0; no--) {
                capsuleNo = MBPlayerCapsuleGet(playerNo, no);
                if(capsuleNo != CAPSULE_NULL) {
                    capWork[capWorkNum].capsuleNo = capsuleNo;
                    capWork[capWorkNum].playerNo = playerNo;
                    capWork[capWorkNum].capsuleIdx = no;
                    capWorkNum++;
                }
            }
        }
    }
    if(capsuleTotal > 0) {
        HuAudFXPlay(MSM_SE_BOARD_61);
        objWork->mode++;
        HuPrcSleep(60);
        MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
        do {
            HuPrcVSleep();
        } while (MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
        objWork->idleStartF = TRUE;
        MBAudFXPlay(MSM_SE_BOARD_61);
        for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
            capWorkP->sprId = MBCapsuleSprCreate(BOARD_ANM_statusCapsule, 100, 0);
            espAttrSet(capWorkP->sprId, HUSPR_ATTR_DISPOFF);
            espBankSet(capWorkP->sprId, MBCapsuleColorNoGet(capWorkP->capsuleNo));
            capWorkP->flag = FALSE;
            MBCapsuleStatusCapsulePosGet(capWorkP->playerNo, capWorkP->capsuleIdx, &capWorkP->startPos);
            angle = (360.0f/capsuleTotal)*i;
            capWorkP->endPos.x = (HuSin(angle)*100)+288;
            capWorkP->endPos.y = (HuCos(angle)*100)+240;
        }
        for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
            espAttrReset(capWorkP->sprId, HUSPR_ATTR_DISPOFF);
            no = MBPlayerCapsuleFind(capWorkP->playerNo, capWorkP->capsuleNo);
            if(no != -1) {
                MBPlayerCapsuleRemove(capWorkP->playerNo, no);
            }
            MBAudFXPlay(MSM_SE_BOARD_33);
            for(no=0; no<15.0f; no++) {
                t = no/15.0f;
                t = HuSin(t*90);
                capWorkP->sprPos.x = capWorkP->startPos.x+((capWorkP->endPos.x-capWorkP->startPos.x)*t);
                capWorkP->sprPos.y = capWorkP->startPos.y+((capWorkP->endPos.y-capWorkP->startPos.y)*t);
                espPosSet(capWorkP->sprId, capWorkP->sprPos.x, capWorkP->sprPos.y);
                HuPrcVSleep();
            }
        }
        seNo[1] = MBAudFXPlay(MSM_SE_BOARD_118);
        for(rotOfs=0, rotSpeed=0, no=0; no<180.0f; no++) {
            if(no > 60.0f) {
                t = (no-60.0f)/120.0f;
                t = HuSin(t*90);
            } else {
                t = 0;
            }
            if(rotSpeed < 10) {
                if(++rotSpeed > 10) {
                    rotSpeed = 10;
                }
            }
            rotOfs = rotOfs+rotSpeed;
            MTXRotDeg(rotMtx, 'X', t*360);
            mtxRotCat(rotMtx, 0, HuSin(t*720)*45, 0);
            for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
                angle = rotOfs+((360.0f/capsuleTotal)*i);
                rotPos.x = HuSin(angle)*100;
                rotPos.y = HuCos(angle)*100;
                rotPos.z = 0;
                MTXMultVec(rotMtx, &rotPos, &rotPos);
                capWorkP->endPos.x = 288+rotPos.x;
                capWorkP->endPos.y = 240+rotPos.y;
                capWorkP->sprPos.x = capWorkP->endPos.x;
                capWorkP->sprPos.y = capWorkP->endPos.y;
                espPosSet(capWorkP->sprId, capWorkP->sprPos.x, capWorkP->sprPos.y);
            }
            HuPrcVSleep();
        }
        for(no=0; no<30.0f; no++) {
            t = no/30.0f;
            t = HuSin(t*90);
            rotOfs = rotOfs+rotSpeed;
            for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
                capWorkP->startPos.x = 288;
                capWorkP->startPos.y = 240;
                angle = rotOfs+((360.0f/capsuleTotal)*i);
                capWorkP->endPos.x = (HuSin(angle)*100)+288;
                capWorkP->endPos.y = (HuCos(angle)*100)+240;
                capWorkP->sprPos.x = capWorkP->endPos.x+((capWorkP->startPos.x-capWorkP->endPos.x)*t);
                capWorkP->sprPos.y = capWorkP->endPos.y+((capWorkP->startPos.y-capWorkP->endPos.y)*t);
                espPosSet(capWorkP->sprId, capWorkP->sprPos.x, capWorkP->sprPos.y);
            }
            HuPrcVSleep();
        }
        if(seNo[1] != MSM_SENO_NONE) {
            MBAudFXStop(seNo[1]);
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_NONE);
        do {
            HuPrcVSleep();
        } while (MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
        objWork->hookF = TRUE;
        if(capsuleTotal > 1) {
            for(i=0; i<capsuleTotal*3; i++) {
                dstIdx = i%capsuleTotal;
                no = MBCapsuleEffRand(capsuleTotal);
                if(dstIdx != no) {
                    capWorkTemp = capWork[dstIdx];
                    capWork[dstIdx] = capWork[no];
                    capWork[no] = capWorkTemp;
                }
            }
        }
        do {
            HuPrcVSleep();
        } while(MBMotionTimeGet(mdlId) < 29);
        if(!GWTeamFGet()) {
            playerNo = capsulePlayer;
        } else {
            if(capsulePlayer == MBPlayerGrpFindPlayer(0, 0) || capsulePlayer == MBPlayerGrpFindPlayer(0, 1)) {
                playerNo = MBPlayerGrpFindPlayer(1, 0);
            } else {
                playerNo = MBPlayerGrpFindPlayer(0, 0);
            }
        }
        for(i=0, capWorkP=capWork; i<capsuleTotal; capWorkP++, i++) {
            if(!GWTeamFGet()) {
                for(no=0; no<GW_PLAYER_MAX; no++) {
                    if(MBPlayerAliveCheck(playerNo)) {
                        break;
                    }
                    if(++playerNo >= GW_PLAYER_MAX) {
                        playerNo %= GW_PLAYER_MAX;
                    }
                }
                no = playerNo;
                if(++playerNo >= GW_PLAYER_MAX) {
                    playerNo %= GW_PLAYER_MAX;
                }
            } else {
                if(!(i & 0x1)) {
                    no = capsulePlayer;
                } else {
                    no = playerNo;
                }
            }
            capWorkP->startPos.x = 288;
            capWorkP->startPos.y = 240;
            MBCapsuleStatusCapsulePosGet(no, MBPlayerCapsuleNumGet(no), &capWorkP->endPos);
            MBAudFXPlay(MSM_SE_BOARD_38);
            for(dstIdx=0; dstIdx<15.0f; dstIdx++) {
                t = dstIdx/15.0f;
                t = HuSin(t*90);
                capWorkP->sprPos.x = capWorkP->startPos.x+((capWorkP->endPos.x-capWorkP->startPos.x)*t);
                capWorkP->sprPos.y = capWorkP->startPos.y+((capWorkP->endPos.y-capWorkP->startPos.y)*t);
                espPosSet(capWorkP->sprId, capWorkP->sprPos.x, capWorkP->sprPos.y);
                HuPrcVSleep();
            }
            MBPlayerCapsuleAdd(no, capWorkP->capsuleNo);
            espAttrSet(capWorkP->sprId, HUSPR_ATTR_DISPOFF);
        }
        do {
            HuPrcVSleep();
        } while(MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE || !MBMotionEndCheck(mdlId));
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KAMEKKU_SHUFFLE, HUWIN_SPEAKER_KAMEKKU);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_46);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KAMEKKU_FAIL, HUWIN_SPEAKER_KAMEKKU);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBAudFXPlay(MSM_SE_BOARD_32);
    seNo[0] = MBAudFXPlay(MSM_SE_BOARD_60);
    for(i=0; i<120.0f; i++) {
        t = i/120.0f;
        sprPos.x = 288;
        sprPos.y = -150+(HuCos(t*90)*300);
        sprPos.z = 2000;
        Hu3D2Dto3D(&sprPos, HU3D_CAM0, &mdlPos);
        MBModelPosSetV(mdlId, &mdlPos);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    if(seNo[0] != MSM_SENO_NONE) {
        MBAudFXStop(seNo[0]);
    }
    objWork->mode = 255;
    HuPrcEnd();
}

void MBCapsuleKamekkuDestroy(void)
{
    MBCapsuleGlowEffKill();
}

static void KamekkuObjExec(OMOBJ *obj)
{
    KAMEKKU_OBJ_WORK *work; //r31
    int i; //r30
    int no; //r28
    int mdlNo; //r27
    
    float t; //f31
    float angle; //f30
    float speed; //f29
    
    Mtx hookMtx; //sp+0x4C
    HuVecF hookPos; //sp+0x40
    HuVecF pos; //sp+0x34
    HuVecF vel; //sp+0x28
    GXColor color; //sp+0xC
    
    work = obj->data;
    if(work->mode >= 5 || capsuleObj == NULL || MBKillCheck()) {
        for(i=0; i<obj->mdlcnt; i++) {
            if(obj->mdlId[i] != MB_MODEL_NONE) {
                Hu3DModelKill(obj->mdlId[i]);
            }
            obj->mdlId[i] = MB_MODEL_NONE;
        }
        omDelObjEx(mbObjMan, obj);
        return;
    }
    if(work->idleStartF && MBMotionEndCheck(work->mdlId)) {
        MBMotionShiftSet(work->mdlId, 4, 0, 8, HU3D_MOTATTR_LOOP);
        work->idleStartF = FALSE;
    }
    if(!work->effectF && work->hookF && MBMotionShiftIDGet(work->mdlId) == MB_MOT_NONE && MBMotionTimeGet(work->mdlId) > 29) {
        Hu3DModelObjMtxGet(MBModelIdGet(work->mdlId), "itemhook_T", hookMtx);
        hookPos.x = hookMtx[0][3];
        hookPos.y = hookMtx[1][3];
        hookPos.z = hookMtx[2][3];
        
        for(i=0, angle=0; i<128; i++) {
            angle += (MBCapsuleEffRandF()+1)*10;
            pos.x = hookPos.x+((-0.5f+MBCapsuleEffRandF())*25);
            pos.y = hookPos.y+((-0.5f+MBCapsuleEffRandF())*25);
            pos.z = hookPos.z+25;
            speed = (MBCapsuleEffRandF()+1.0f)*2.5f;
            vel.x = speed*HuSin(angle);
            vel.y = speed*HuCos(angle);
            vel.z = 0;
            t = MBCapsuleEffRandF();
            color = kinokoGlowColorTbl[MBCapsuleEffRand(7)];
            MBCapsuleGlowEffAdd(pos, vel, 
                ((MBCapsuleEffRandF()*0.1f)+0.3f)*100,
                (MBCapsuleEffRandF()+1.0f)*0.016666668f,
                0,
                0,
                color);
        }
        work->effectF = TRUE;
        work->mode = 4;
    }
    switch(work->mode) {
        case 0:
            break;
            
        case 1:
            Hu3DModelObjMtxGet(MBModelIdGet(work->mdlId), "itemhook_T", hookMtx);
            for(i=0; i<obj->mdlcnt; i++) {
                work->pos[i].x = hookMtx[0][3];
                work->pos[i].y = hookMtx[1][3];
                work->pos[i].z = hookMtx[2][3];
                Hu3DModelDispOn(obj->mdlId[i]);
            }
            work->mode++;
            work->time = 0;
        
        case 2:
            t = ++work->time/60.0f;
            if(t > 1) {
                t = 1;
            }
            Hu3DModelObjMtxGet(MBModelIdGet(work->mdlId), "itemhook_T", hookMtx);
            work->pos[work->mdlNo].x = hookMtx[0][3];
            work->pos[work->mdlNo].y = hookMtx[1][3];
            work->pos[work->mdlNo].z = hookMtx[2][3];
            mdlNo = work->mdlNo;
            if(++work->mdlNo >= obj->mdlcnt) {
                work->mdlNo = 0;
            }
            for(i=0; i<obj->mdlcnt; i++) {
                no = mdlNo-i;
                if(no < 0) {
                    no += obj->mdlcnt;
                }
                Hu3DModelPosSet(obj->mdlId[i], work->pos[no].x, work->pos[no].y, work->pos[no].z);
                Hu3DModelTPLvlSet(obj->mdlId[i], work->tpLvl[i]*t);
            }
            if(t >= 1) {
                work->mode++;
            }
            break;
        
        case 3:
            Hu3DModelObjMtxGet(MBModelIdGet(work->mdlId), "itemhook_T", hookMtx);
            work->pos[work->mdlNo].x = hookMtx[0][3];
            work->pos[work->mdlNo].y = hookMtx[1][3];
            work->pos[work->mdlNo].z = hookMtx[2][3];
            mdlNo = work->mdlNo;
            if(++work->mdlNo >= obj->mdlcnt) {
                work->mdlNo = 0;
            }
            for(i=0; i<obj->mdlcnt; i++) {
                no = mdlNo-i;
                if(no < 0) {
                    no += obj->mdlcnt;
                }
                Hu3DModelPosSet(obj->mdlId[i], work->pos[no].x, work->pos[no].y, work->pos[no].z);
            }
            break;
        
        case 4:
            Hu3DModelObjMtxGet(MBModelIdGet(work->mdlId), "itemhook_T", hookMtx);
            work->pos[work->mdlNo].x = hookMtx[0][3];
            work->pos[work->mdlNo].y = hookMtx[1][3];
            work->pos[work->mdlNo].z = hookMtx[2][3];
            mdlNo = work->mdlNo;
            if(++work->mdlNo >= obj->mdlcnt) {
                work->mdlNo = 0;
            }
            for(i=0; i<obj->mdlcnt; i++) {
                no = mdlNo-i;
                if(no < 0) {
                    no += obj->mdlcnt;
                }
                if((work->tpLvl[i] *= 0.9f) < 0.01f) {
                    work->tpLvl[i] = 0;
                }
                Hu3DModelPosSet(obj->mdlId[i], work->pos[no].x, work->pos[no].y, work->pos[no].z);
                Hu3DModelTPLvlSet(obj->mdlId[i], work->tpLvl[i]);
            }
            break;
            
        case 5:
            break;
    }
}

void MBCapsuleThrowmanExec(void)
{
    int i; //r31
    int mdlId; //r22
    
    float t; //f31
    float tpLvl; //f30
    
    HuVecF playerPos; //sp+0x60
    HuVecF effPos; //sp+0x54
    HuVecF mdlPos; //sp+0x48
    HuVecF mdlVel; //sp+0x3C
    int motId[3]; //sp+0x30
    HU3DMODEL *modelP; //sp+0x20
    int masuId; //sp+0x1C
    int seNo; //sp+0x18
    float s; //sp+0x14
    
    MBCapsuleExplodeEffCreate(0);
    MBCapsuleSnowEffCreate();
    MBCapsuleLoseEffCreate();
    motId[0] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_325);
    motId[1] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_327);
    motId[2] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_368);
    MBPlayerMoveInit();
    MBPlayerMoveObjCreate();
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    seNo = MBAudFXPlay(MSM_SE_BOARD_136);
    for(i=0; i<120.0f; i++) {
        if((i & 0x7) == 0) {
            effPos.x = playerPos.x+((MBCapsuleEffRandF()-0.5f)*200);
            effPos.y = playerPos.y+(((MBCapsuleEffRandF()*0.2f)+1.0f)*600);
            effPos.z = playerPos.z+((MBCapsuleEffRandF()-0.5f)*200);
            MBCapsuleSnowEffAdd(&effPos, (MBCapsuleEffRandF()+3.5f)*60);
        }
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, motId[0], 0, 8, HU3D_MOTATTR_NONE);
    i = MBPlayerModelGet(capsulePlayer);
    i = MBModelIdGet(i);
    modelP = &Hu3DData[i];
    modelP->motShiftWork.speed = 3;
    i = 0;
    do {
        HuPrcVSleep();
        i++;
        if((i & 0x7) == 0) {
            effPos.x = playerPos.x+((MBCapsuleEffRandF()-0.5f)*200);
            effPos.y = playerPos.y+(((MBCapsuleEffRandF()*0.2f)+1.0f)*600);
            effPos.z = playerPos.z+((MBCapsuleEffRandF()-0.5f)*200);
            MBCapsuleSnowEffAdd(&effPos, (MBCapsuleEffRandF()+3.5f)*60);
        }
        
    } while(!MBCapsulePlayerMotShiftCheck(capsulePlayer) || !MBPlayerMotionEndCheck(capsulePlayer));
    MBPlayerMotionNoShiftSet(capsulePlayer, motId[1], 0, 8, HU3D_MOTATTR_NONE);
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_throwman, NULL, FALSE, 10, FALSE);
    mdlPos = playerPos;
    mdlPos.y += 1000;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    mdlVel.x = mdlVel.y = mdlVel.z = 0;
    do {
        HuPrcVSleep();
        VECAdd(&mdlPos, &mdlVel, &mdlPos);
        mdlVel.y -= 6.533333430687588;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    } while(mdlPos.y > playerPos.y);
    if(seNo != MSM_SENO_NONE) {
        MBAudFXStop(seNo);
    }
    MBAudFXPlay(MSM_SE_BOARD_63);
    MBCapsuleDustHeavyAdd(0, playerPos);
    mdlPos = playerPos;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    MBPlayerMotionNoSet(capsulePlayer, MB_PLAYER_MOT_FLATTEN, HU3D_MOTATTR_NONE);
    MBPlayerMotionSpeedSet(capsulePlayer, 0);
    MBPlayerMotionTimeSet(capsulePlayer, 20);
    CharFXPlay(GwPlayer[capsulePlayer].charNo, CHARVOICEID(12));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(i != capsulePlayer && masuId == GwPlayer[i].masuId) {
            MBPlayerMoveHitCreate(i, FALSE, TRUE);
        }
        if(masuId == GwPlayer[i].masuId) {
            omVibrate(i, 12, 4, 2);
        }
    }
    if(MBPlayerCapsuleNumGet(capsulePlayer) > 0) {
        MBCapsuleLoseEffAddMulti(capsulePlayer, MBPlayerCapsuleMaxGet(), 100);
        for(i=0; i<MBPlayerCapsuleMaxGet(); i++) {
            MBPlayerCapsuleRemove(capsulePlayer, 0);
        }
        do {
            HuPrcVSleep();
        } while (MBCapsuleLoseEffNumGet() > 0);
        for(i=0; i<60.0f; i++) {
            HuPrcVSleep();
        }
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_THROWMAN_RESULT, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        for(i=0; i<60.0f; i++) {
            HuPrcVSleep();
        }
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_THROWMAN_NULL, HUWIN_SPEAKER_NONE);
        MBTopWinWait();
        MBTopWinKill();
    }
    HuAudFXPlay(MSM_SE_BOARD_78);
    for(i=0; i<120.0f; i++) {
        t = i/120.0f;
        s = HuSin(t*90);
        if(t < 0.5f) {
            tpLvl = 0;
        } else {
            tpLvl = 2*(t-0.5f);
        }
        MBModelScaleSet(mdlId, t+1, 1-t, t+1);
        MBModelAlphaSet(mdlId, (1-tpLvl)*255);
        HuPrcVSleep();
    }
    MBPlayerMotionSpeedSet(capsulePlayer, 1);
    do {
        HuPrcVSleep();
    } while(!MBPlayerMotionEndCheck(capsulePlayer));
    MBPlayerMoveHitCreate(capsulePlayer, FALSE, TRUE);
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    MBCapsulePlayerIdleWait();
    MBPlayerMoveObjClose();
    HuPrcEnd();
}

void MBCapsuleThrowmanDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleSnowEffKill();
    MBCapsuleLoseEffKill();
}

void MBCapsulePakkunExec(void)
{
    int i; //r31
    int mdlId; //r30
    int j; //r27
    int coinNum; //r20
    
    float t; //f31
    
    Mtx hookMtx; //sp+0xF8
    int motFile[8]; //sp+0xD8
    HuVecF playerPos; //sp+0xCC
    HuVecF tempVec; //sp+0xC0
    HuVecF playerVel; //sp+0xB4
    HuVecF effRot; //sp+0xA8
    HuVecF playerPosOld; //sp+0x9C
    HuVecF mdlPos; //sp+0x90
    int motId[3]; //sp+0x84
    GXColor color; //sp+0x44
    int masuId; //sp+0x40
    int seNo; //sp+0x3C
    
    MBCapsuleExplodeEffCreate(0);
    MBPlayerMoveInit();
    MBPlayerMoveObjCreate();
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerPosOld = playerPos;
    motFile[0] = CAPSULECHAR0_HSF_pakkunflower_start;
    motFile[1] = CAPSULECHAR0_HSF_pakkunflower_wait;
    motFile[2] = CAPSULECHAR0_HSF_pakkunflower_eat;
    motFile[3] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR0_HSF_pakkunflower, motFile, FALSE, 10, FALSE);
    mdlPos = playerPos;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_NONE);
    MBModelDispSet(mdlId, FALSE);
    motId[0] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_441);
    motId[1] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_437);
    motId[2] = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_438);
    tempVec.x = 0;
    tempVec.y = 100;
    tempVec.z = 0;
    MBCameraMasuViewSet(masuId, NULL, &tempVec, -1, -1, 21);
    MBCameraMotionWait();
    MBCapsulePlayerMotSet(capsulePlayer, motId[0], HU3D_MOTATTR_NONE, TRUE);
    for(i=0; i<3; i++) {
        for(j=0; j<32; j++) {
            tempVec.x = playerPos.x+((MBCapsuleEffRandF()-0.5f)*100);
            tempVec.y = playerPos.y;
            tempVec.z = playerPos.z+((MBCapsuleEffRandF()-0.5f)*100);
            effRot.x = effRot.y = effRot.z = 0;
            t = MBCapsuleEffRandF();
            color.r = 192+(t*63);
            color.g = 192+(t*63);
            color.b = 192+(t*63);
            color.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleExplodeEffAdd(0, tempVec, effRot,
                (MBCapsuleEffRandF()+2)*100,
                0,
                (MBCapsuleEffRandF()*0.2f)+0.33f,
                color);
        }
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_BOARD_65);
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_NONE);
    MBMotionTimeSet(mdlId, 0);
    MBMotionSpeedSet(mdlId, 0.5f);
    MBModelScaleSet(mdlId, 0, 0, 0);
    MBCapsuleEffHookCall(7, mdlId, FALSE, FALSE, FALSE);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(i != capsulePlayer && masuId == GwPlayer[i].masuId) {
            MBPlayerMoveHitCreate(i, TRUE, FALSE);
        }
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, motId[1], 0, 5, HU3D_MOTATTR_NONE);
    for(i=0; i<10; i++) {
        t = i/9.0f;
        if(t > 1) {
            t = 1;
        }
        MBModelScaleSet(mdlId, t, t, t);
        Hu3DMotionCalc(MBModelIdGet(mdlId));
        Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_C", hookMtx);
        tempVec.x = hookMtx[0][3];
        tempVec.y = hookMtx[1][3];
        tempVec.z = hookMtx[2][3];
        if(tempVec.y > playerPos.y) {
            tempVec.y = playerPos.y;
        }
        VECSubtract(&tempVec, &playerPos, &playerVel);
        VECScale(&playerVel, &playerVel, i/9.0f);
        VECAdd(&playerPos, &playerVel, &tempVec);
        MBPlayerPosSetV(capsulePlayer, &tempVec);
        
        for(j=0; j<2; j++) {
            tempVec.x = playerPos.x+((MBCapsuleEffRandF()-0.5f)*100);
            tempVec.y = playerPos.y;
            tempVec.z = playerPos.z+((MBCapsuleEffRandF()-0.5f)*100);
            effRot.x = effRot.y = effRot.z = 0;
            t = MBCapsuleEffRandF();
            color.r = 192+(t*63);
            color.g = 192+(t*63);
            color.b = 192+(t*63);
            color.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleExplodeEffAdd(0, tempVec, effRot,
                (MBCapsuleEffRandF()+2)*100,
                0,
                (MBCapsuleEffRandF()*0.2f)+0.33f,
                color);
        }
        HuPrcVSleep();
    }
    MBModelDispSet(MBPlayerModelGet(capsulePlayer), FALSE);
    omVibrate(capsulePlayer, 12, 4, 2);
    do {
        HuPrcVSleep();
    } while(!MBMotionEndCheck(mdlId) || MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
    seNo = MBAudFXPlay(MSM_SE_BOARD_66);
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
    for(i=0; i<120.0f; i++) {
        HuPrcVSleep();
    }
    if(seNo != MSM_SENO_NONE) {
        MBAudFXStop(seNo);
    }
    seNo = MSM_SENO_NONE;
    MBAudFXPlay(MSM_SE_BOARD_67);
    MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_NONE);
    HuPrcSleep(38);
    Hu3DMotionCalc(MBModelIdGet(mdlId));
    Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_C", hookMtx);
    MBCapsuleEffHookCall(6, mdlId, FALSE, FALSE, FALSE);
    MBPlayerMotionNoSet(capsulePlayer, motId[2], HU3D_MOTATTR_NONE);
    MBModelDispSet(MBPlayerModelGet(capsulePlayer), TRUE);
    tempVec.x = playerPosOld.x;
    tempVec.y = hookMtx[1][3];
    tempVec.z = playerPosOld.z;
    MBPlayerPosSetV(capsulePlayer, &tempVec);
    MBPlayerMoveEjectCreate(capsulePlayer, TRUE);
    MBPlayerMoveMinYSet(capsulePlayer, playerPosOld.y);
    tempVec.x = 0;
    tempVec.y = 35;
    tempVec.z = 0;
    MBPlayerMoveVelSet(capsulePlayer, 3.266667f, tempVec);
    if(MBPlayerCoinGet(capsulePlayer) <= 1) {
        coinNum = 0;
    } else {
        coinNum = MBPlayerCoinGet(capsulePlayer)/2;
        MBPlayerCoinAdd(capsulePlayer, -coinNum);
    }
    do {
        HuPrcVSleep();
    } while(!MBMotionEndCheck(mdlId) || MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
    for(i=0; i<15.0f; i++) {
        t = i/15.0f;
        MBModelScaleSet(mdlId, 1-t, 1-t, 1-t);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    do {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(!MBPlayerMoveObjCheck(i)) {
                break;
            }
        }
        HuPrcVSleep();
    } while(i < GW_PLAYER_MAX);
    if(coinNum) {
        MBCapsuleCoinDispExec(capsulePlayer, -coinNum, FALSE, TRUE);
    }
    if(coinNum) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_PAKKUN_RESULT, HUWIN_SPEAKER_NONE);
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_PAKKUN_NULL, HUWIN_SPEAKER_NONE);
    }
    MBTopWinWait();
    MBTopWinKill();
    MBCapsulePlayerIdleWait();
    MBPlayerMoveObjClose();
    HuPrcEnd();
}

void MBCapsulePakkunDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
}

void MBCapsuleKokamekkuExec(void)
{
    KAMEKKU_CAPSULE_WORK *capWorkP; //r31
    int i; //r30
    int no; //r29
    int mdlId; //r28
    int capsuleTotal; //r27
    int capWorkNum; //r23
    int j; //r21
    int swapPlayer; //r19
    int seNo1; //r18
    int seNo2; //r17
    
    float t; //f31
    float angle; //f30
    float rotOfs; //f29
    float rotSpeed; //f28
    
    KAMEKKU_CAPSULE_WORK capWork[12];
    Mtx rotMtx; //sp+0x64
    int motFile[8]; //sp+0x44
    HuVecF rotPos; //sp+0x38
    HuVecF sprPos; //sp+0x2C
    HuVecF mdlPos; //sp+0x20
    int capsuleNum[2]; //sp+0x18
    int playerNo[2]; //sp+0x10
    
    motFile[0] = CAPSULECHAR1_HSF_kokamekku_wait;
    motFile[1] = CAPSULECHAR1_HSF_kokamekku_up;
    motFile[2] = CAPSULECHAR1_HSF_kokamekku_down;
    motFile[3] = CAPSULECHAR1_HSF_kokamekku_idle;
    motFile[4] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_kokamekku, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBAudFXPlay(MSM_SE_BOARD_32);
    seNo2 = MBAudFXPlay(MSM_SE_BOARD_79);
    for(i=0; i<=120.0f; i++) {
        t = i/120.0f;
        sprPos.x = 288;
        sprPos.y = -150+(300*HuSin(t*90));
        sprPos.z = 2000;
        Hu3D2Dto3D(&sprPos, HU3D_CAM0, &mdlPos);
        MBModelPosSetV(mdlId, &mdlPos);
        HuPrcVSleep();
    }
    for(i=0, capsuleTotal=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            capsuleTotal += MBPlayerCapsuleNumGet(i);
        }
    }
    if(capsuleTotal <= 0) {
        if(!GWTeamFGet()) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_47);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU, HUWIN_SPEAKER_KOKAMEKKU);
            MBTopWinWait();
            MBTopWinKill();
        } else {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_47);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU_TEAM, HUWIN_SPEAKER_KOKAMEKKU);
            MBTopWinWait();
            MBTopWinKill();
        }
        MBAudFXPlay(MSM_SE_GUIDE_48);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU_FAIL, HUWIN_SPEAKER_KOKAMEKKU);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        if(!GWTeamFGet()) {
            MBAudFXPlay(MSM_SE_GUIDE_47);
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU, HUWIN_SPEAKER_KOKAMEKKU);
            MBTopWinWait();
            MBTopWinKill();
            if(MBPlayerAliveGet(NULL) >= 3) {
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU_ROULETTE, HUWIN_SPEAKER_KOKAMEKKU);
                MBTopWinWait();
                MBTopWinKill();
            }
            if(MBRouletteCreate(capsulePlayer, 3)) {
                MBRouletteWait();
            }
            swapPlayer = MBRouletteResultGet();
        } else {
            MBAudFXPlay(MSM_SE_GUIDE_47);
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU_TEAM, HUWIN_SPEAKER_KOKAMEKKU);
            MBTopWinWait();
            MBTopWinKill();
            if(MBPlayerGrpFindPlayer(0, 0) == capsulePlayer || MBPlayerGrpFindPlayer(0, 1) == capsulePlayer) {
                swapPlayer = MBPlayerGrpFindPlayer(1, 0);
            } else {
                swapPlayer = MBPlayerGrpFindPlayer(0, 0);
            }
        }
        playerNo[0] = capsulePlayer;
        playerNo[1] = swapPlayer;
        memset(capWork, 0, sizeof(capWork));
        for(i=0, capWorkP=capWork; i<12; i++, capWorkP++) {
            capWorkP->sprId = -1;
        }
        MBCapsuleStatusPosMoveWait(TRUE, TRUE);
        for(i=0; i<30.0f; i++) {
            HuPrcVSleep();
        }
        for(i=0, capWorkNum=0, capsuleTotal=0; i<2; i++) {
            capsuleNum[i] = MBPlayerCapsuleNumGet(playerNo[i]);
            capsuleTotal += capsuleNum[i];
            for(no=MBPlayerCapsuleMaxGet()-1; no>=0; no--) {
                capWork[capWorkNum].capsuleNo = MBPlayerCapsuleGet(playerNo[i], no);
                capWork[capWorkNum].playerNo = playerNo[i];
                capWork[capWorkNum].capsuleIdx = no;
                if(capWork[capWorkNum].capsuleNo != CAPSULE_NULL) {
                    capWorkNum++;
                }
            }
        }
        if(capsuleTotal > 0) {
            MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
            do {
                HuPrcVSleep();
            } while(MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE || !MBMotionEndCheck(mdlId));
            MBMotionShiftSet(mdlId, 4, 0, 8, HU3D_MOTATTR_LOOP);
            for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
                capWorkP->sprId = MBCapsuleSprCreate(BOARD_ANM_statusCapsule, 100, 0);
                espAttrSet(capWorkP->sprId, HUSPR_ATTR_DISPOFF);
                espBankSet(capWorkP->sprId, MBCapsuleColorNoGet(capWorkP->capsuleNo));
                capWorkP->flag = FALSE;
                MBCapsuleStatusCapsulePosGet(capWorkP->playerNo, capWorkP->capsuleIdx, &capWorkP->startPos);
                angle = (360.0f/capsuleTotal)*i;
                capWorkP->endPos.x = (HuSin(angle)*75)+288;
                capWorkP->endPos.y = (HuCos(angle)*75)+240;
            }
            for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
                espAttrReset(capWorkP->sprId, HUSPR_ATTR_DISPOFF);
                no = MBPlayerCapsuleFind(capWorkP->playerNo, capWorkP->capsuleNo);
                if(no != -1) {
                    MBPlayerCapsuleRemove(capWorkP->playerNo, no);
                }
                seNo1 = MBAudFXPlay(MSM_SE_BOARD_33);
                for(no=0; no<15.0f; no++) {
                    t = no/15.0f;
                    t = HuSin(t*90);
                    capWorkP->sprPos.x = capWorkP->startPos.x+((capWorkP->endPos.x-capWorkP->startPos.x)*t);
                    capWorkP->sprPos.y = capWorkP->startPos.y+((capWorkP->endPos.y-capWorkP->startPos.y)*t);
                    espPosSet(capWorkP->sprId, capWorkP->sprPos.x, capWorkP->sprPos.y);
                    HuPrcVSleep();
                }
            }
            seNo1 = MBAudFXPlay(MSM_SE_BOARD_118);
            for(rotOfs=0, rotSpeed=0, no=0; no<180.0f; no++) {
                if(no > 60.0f) {
                    t = (no-60.0f)/120.0f;
                    t = HuSin(t*90);
                } else {
                    t = 0;
                }
                if(rotSpeed < 10) {
                    if(++rotSpeed > 10) {
                        rotSpeed = 10;
                    }
                }
                rotOfs = rotOfs+rotSpeed;
                MTXRotDeg(rotMtx, 'X', t*360);
                mtxRotCat(rotMtx, 0, HuSin(t*360)*30, 0);
                for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
                    angle = rotOfs+((360.0f/capsuleTotal)*i);
                    rotPos.x = HuSin(angle)*75;
                    rotPos.y = HuCos(angle)*75;
                    rotPos.z = 0;
                    MTXMultVec(rotMtx, &rotPos, &rotPos);
                    capWorkP->endPos.x = 288+rotPos.x;
                    capWorkP->endPos.y = 240+rotPos.y;
                    capWorkP->sprPos.x = capWorkP->endPos.x;
                    capWorkP->sprPos.y = capWorkP->endPos.y;
                    espPosSet(capWorkP->sprId, capWorkP->sprPos.x, capWorkP->sprPos.y);
                }
                HuPrcVSleep();
            }
            if(seNo1 != MSM_SENO_NONE) {
                MBAudFXStop(seNo1);
            }
            MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_NONE);
            do {
                HuPrcVSleep();
            } while(MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
            for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
                if(capWorkP->playerNo == playerNo[0]) {
                    capWorkP->playerNo = playerNo[1];
                } else {
                    capWorkP->playerNo = playerNo[0];
                }
            }
            for(i=0, capWorkP=capWork; i<capsuleTotal; i++, capWorkP++) {
                capWorkP->startPos.x = capWorkP->sprPos.x;
                capWorkP->startPos.y = capWorkP->sprPos.y;
                MBCapsuleStatusCapsulePosGet(capWorkP->playerNo, MBPlayerCapsuleNumGet(capWorkP->playerNo), &capWorkP->endPos);
                MBAudFXPlay(MSM_SE_BOARD_38);
                for(j=0; j<15.0f; j++) {
                    t = j/15.0f;
                    t = HuSin(t*90);
                    capWorkP->sprPos.x = capWorkP->startPos.x+((capWorkP->endPos.x-capWorkP->startPos.x)*t);
                    capWorkP->sprPos.y = capWorkP->startPos.y+((capWorkP->endPos.y-capWorkP->startPos.y)*t);
                    espPosSet(capWorkP->sprId, capWorkP->sprPos.x, capWorkP->sprPos.y);
                    HuPrcVSleep();
                }
                MBPlayerCapsuleAdd(capWorkP->playerNo, capWorkP->capsuleNo);
                espAttrSet(capWorkP->sprId, HUSPR_ATTR_DISPOFF);
            }
            do {
                HuPrcVSleep();
            } while(MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE || !MBMotionEndCheck(mdlId));
            MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        } else {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_48);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU_FAIL, HUWIN_SPEAKER_KOKAMEKKU);
            MBTopWinWait();
            MBTopWinKill();
        }
    }
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_KOKAMEKKU_END, HUWIN_SPEAKER_KOKAMEKKU);
    MBTopWinWait();
    MBTopWinKill();
    MBAudFXPlay(MSM_SE_BOARD_32);
    for(i=0; i<120.0f; i++) {
        t = i/120.0f;
        sprPos.x = 288;
        sprPos.y = -150+(300*HuCos(t*90));
        sprPos.z = 2000;
        Hu3D2Dto3D(&sprPos, HU3D_CAM0, &mdlPos);
        MBModelPosSetV(mdlId, &mdlPos);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    if(seNo2 != MSM_SENO_NONE) {
        MBAudFXStop(seNo2);
    }
    HuPrcEnd();
}

void MBCapsuleKokamekkuDestroy(void)
{
    
}

void MBCapsuleUkkiExec(void)
{
    int i; //r31
    int mdlId; //r30
    int otherPlayer; //r29
    int j; //r28
    int ropeMdlId; //r27
    int num; //r23
    int time; //r19
    
    float t; //f31
    
    HuVecF capsulePos[6][3]; //sp+0x1BC
    int capsuleMasuId[6]; //sp+0x1A4
    int capsuleNo[6]; //sp+0x18C
    int capsuleMdlId[6]; //sp+0x174
    float capsuleTime[6]; //sp+0x15C
    int motFile[8]; //sp+0x13C
    HuVecF playerPos; //sp+0x130
    HuVecF playerPosOther; //sp+0x124
    HuVecF mdlPos; //sp+0x118
    HuVecF mdlRot; //sp+0x10C
    HuVecF endPos; //sp+0x100
    HuVecF startPos; //sp+0xF4
    HuVecF effPos; //sp+0xE8
    HuVecF effRot; //sp+0xDC
    HuVecF playerPosOld; //sp+0xD0
    int motId[3]; //sp+0xC4
    int playerList[GW_PLAYER_MAX]; //sp+0xB4
    GXColor color; //sp+0x80
    int validNum; //sp+0x7C
    int masuId; //sp+0x78
    int masuIdOther; //sp+0x74
    int mode; //sp+0x70
    int sp6C; 
    int takeDelay; //sp+0x68
    int takeNum; //sp+0x64
    float sp60;
    float sp5C;
    float sp58;
    BOOL voicePlayF; //sp+0x54
    
    voicePlayF = FALSE;
    MBCapsuleExplodeEffCreate(0);
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    motFile[0] = CAPSULECHAR2_HSF_ukki_hold;
    motFile[1] = CAPSULECHAR2_HSF_ukki_climb;
    motFile[2] = CAPSULECHAR2_HSF_ukki_punch;
    motFile[3] = CAPSULECHAR2_HSF_ukki_away;
    motFile[4] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR2_HSF_ukki, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    ropeMdlId = MBCapsuleModelCreate(CAPSULE_HSF_ukkiRope, NULL, FALSE, 10, FALSE);
    MBModelLayerSet(ropeMdlId, 2);
    MBModelDispSet(ropeMdlId, FALSE);
    MBAudFXPlay(MSM_SE_BOARD_70);
    MBModelDispSet(mdlId, TRUE);
    MBModelDispSet(ropeMdlId, TRUE);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = playerPos.y+1000+(-800*HuSin(t*90));
        mdlPos.z = playerPos.z-50;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelPosSet(ropeMdlId, mdlPos.x, mdlPos.y+200, mdlPos.z);
        HuPrcVSleep();
    }
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
    for(i=0, num=0, validNum=0; i<GW_PLAYER_MAX; i++) {
        if(i == capsulePlayer) {
            continue;
        }
        playerList[num] = i;
        num++;
        if(!GWTeamFGet() || !MBCapsuleTeamCheck(capsulePlayer, i)) {
            if(MBPlayerCapsuleNumGet(i) > 0 && MBPlayerAliveCheck(i)) {
                validNum++;
            }
        }
    }
    if(validNum <= 0) {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_40);
        MBAudFXPlay(MSM_SE_BOARD_69);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_UKKI_FAIL, HUWIN_SPEAKER_UKKI);
        MBTopWinWait();
        MBTopWinKill();
        MBAudFXPlay(MSM_SE_BOARD_70);
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.y = playerPos.y+1000+(-800*HuCos(t*90));
            mdlPos.z = playerPos.z-50;
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelPosSet(ropeMdlId, mdlPos.x, mdlPos.y+200, mdlPos.z);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        MBModelDispSet(ropeMdlId, FALSE);
        HuPrcEnd();
        return;
    }
    if(!GWTeamFGet()) {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_39);
        MBAudFXPlay(MSM_SE_BOARD_69);
        MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX03_UKKI_CHOICE, HUWIN_SPEAKER_UKKI, 0);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(playerList[0]), 0);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(playerList[1]), 1);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(playerList[2]), 2);
        for(i=0; i<num; i++) {
            if(!MBPlayerAliveCheck(playerList[i])) {
                MBTopWinChoiceDisable(i);
            }
            if(MBPlayerCapsuleNumGet(playerList[i]) <= 0) {
                MBTopWinChoiceDisable(i);
            }
            if(GWTeamFGet() && MBCapsuleTeamCheck(capsulePlayer, playerList[i])) {
                MBTopWinChoiceDisable(i);
            }
        }
        MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
        if(GwPlayer[capsulePlayer].comF) {
            if(GWPartyFGet() == TRUE) {
                MBCapsuleChoiceSet(MBCapsuleEffRand(3));
            } else {
                MBCapsuleChoiceSet(0);
            }
        }
        MBTopWinWait();
        otherPlayer = MBWinLastChoiceGet();
        MBTopWinKill();
        if(otherPlayer >= 3) {
            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_CAPSULE);
        } else {
            otherPlayer = playerList[otherPlayer];
        }
    } else {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_39);
        MBAudFXPlay(MSM_SE_BOARD_69);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_UKKI_START, HUWIN_SPEAKER_UKKI);
        MBTopWinWait();
        MBTopWinKill();
        otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_CAPSULE);
    }
    MBAudFXPlay(MSM_SE_BOARD_69);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_UKKI_END, HUWIN_SPEAKER_UKKI);
    MBTopWinWait();
    MBTopWinKill();
    MBAudFXPlay(MSM_SE_BOARD_70);
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = playerPos.y+1000+(-800*HuCos(t*90));
        mdlPos.z = playerPos.z-50;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelPosSet(ropeMdlId, mdlPos.x, mdlPos.y+200, mdlPos.z);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBModelDispSet(ropeMdlId, FALSE);
    masuIdOther = GwPlayer[otherPlayer].masuId;
    MBPlayerPosGet(otherPlayer, &playerPosOther);
    num = MBPlayerCapsuleNumGet(otherPlayer);
    motId[0] = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_354);
    motId[1] = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_321);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBStarMasuDispSet(masuIdOther, FALSE);
    MBMasuPlayerLightSet(capsulePlayer, FALSE);
    for(j=0; j<GW_PLAYER_MAX; j++) {
        if(otherPlayer != j) {
            MBPlayerDispSet(j, FALSE);
        }
    }
    MBPlayerPosGet(otherPlayer, &playerPosOld);
    MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[otherPlayer].masuId, &endPos);
    MBPlayerPosSetV(otherPlayer, &endPos);
    MBPlayerPosGet(otherPlayer, &playerPosOther);
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(masuIdOther, FALSE, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    MBAudFXPlay(MSM_SE_BOARD_70);
    MBModelDispSet(mdlId, TRUE);
    MBModelDispSet(ropeMdlId, TRUE);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPosOther.x;
        mdlPos.y = playerPosOther.y+1000+(-800*HuSin(t*90));
        mdlPos.z = playerPosOther.z-50;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelPosSet(ropeMdlId, mdlPos.x, mdlPos.y+200, mdlPos.z);
        HuPrcVSleep();
    }
    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
    if(num) {
        for(i=0; i<num;) {
            capsuleMasuId[i] = MBCapsuleEffRand(MBMasuNumGet(MASU_LAYER_DEFAULT))+1;
            if(masuId != capsuleMasuId[i]) {
                if(MBMasuTypeGet(MASU_LAYER_DEFAULT, capsuleMasuId[i]) == MASU_TYPE_BLUE ||
                    MBMasuTypeGet(MASU_LAYER_DEFAULT, capsuleMasuId[i]) == MASU_TYPE_RED) {
                        for(j=0; j<i; j++) {
                            if(capsuleMasuId[i] == capsuleMasuId[j]) {
                                break;
                            }
                        }
                        if(j >= i) {
                            MBMasuPosGet(MASU_LAYER_DEFAULT, capsuleMasuId[i], &capsulePos[i][2]);
                            i++;
                        }
                    }
            }
        }
        for(i=0; i<num; i++) {
            capsulePos[i][0].x = playerPosOther.x;
            capsulePos[i][0].y = playerPosOther.y;
            capsulePos[i][0].z = playerPosOther.z;
            VECSubtract(&capsulePos[i][2], &capsulePos[i][0], &endPos);
            endPos.y = 0;
            VECNormalize(&endPos, &endPos);
            VECScale(&endPos, &endPos, 1000);
            VECAdd(&capsulePos[i][0], &endPos, &capsulePos[i][2]);
            VECSubtract(&capsulePos[i][2], &capsulePos[i][0], &endPos);
            VECScale(&endPos, &endPos, 0.5f);
            VECAdd(&capsulePos[i][0], &endPos, &capsulePos[i][1]);
            capsulePos[i][1].y += 2000;
            capsuleTime[i] = 0;
            capsuleNo[i] = MBPlayerCapsuleGet(otherPlayer, i);
            capsuleMdlId[i] = MBCapsuleModelCreate(MBCapsuleMdlGet(capsuleNo[i]), NULL, FALSE, 10, FALSE);
            MBModelPosSetV(capsuleMdlId[i], &capsulePos[i][0]);
            MBModelAttrSet(capsuleMdlId[i], HU3D_MOTATTR_LOOP);
            MBModelLayerSet(capsuleMdlId[i], 2);
            MBModelDispSet(capsuleMdlId[i], FALSE);
        }
        MBMotionShiftSet(mdlId, 4, 0, 8, HU3D_MOTATTR_NONE);
        startPos = mdlPos;
        endPos.x = playerPosOther.x+20;
        endPos.y = playerPosOther.y;
        endPos.z = playerPosOther.z+150;
        mdlRot.x = mdlRot.y = mdlRot.z = 0;
        for(i=0; i<30.0f; i++) {
            t = i/30.0f;
            mdlPos.x = startPos.x+(t*(endPos.x-startPos.x));
            mdlPos.z = startPos.z+(t*(endPos.z-startPos.z));
            mdlPos.y = startPos.y+(t*(endPos.y-startPos.y))+(200*HuSin(t*180));
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelRotSetV(mdlId, &mdlRot);
            HuPrcVSleep();
        }
        CharEffectLayerSet(3);
        MBPlayerMotionNoSet(otherPlayer, MB_PLAYER_MOT_SHOCK, HU3D_MOTATTR_NONE);
        MBMotionShiftSet(mdlId, 4, 0, 8, HU3D_MOTATTR_NONE);
        startPos = mdlPos;
        endPos.x = playerPosOther.x;
        endPos.y = playerPosOther.y+20;
        endPos.z = playerPosOther.z+35;
        mdlRot.x = mdlRot.y = mdlRot.z = 0;
        for(i=0; i<30.0f; i++) {
            t = i/30.0f;
            mdlPos.x = startPos.x+(t*(endPos.x-startPos.x));
            mdlPos.z = startPos.z+(t*(endPos.z-startPos.z));
            mdlPos.y = startPos.y+(t*(endPos.y-startPos.y))+(200*HuSin(t*180));
            mdlRot.y = MBCapsuleAngleLerp(180, mdlRot.y, 10);
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelRotSetV(mdlId, &mdlRot);
            if(t > 0.7f) {
                if(!voicePlayF) {
                    CharFXPlay(GwPlayer[otherPlayer].charNo, CHARVOICEID(3));
                    MBAudFXPlay(MSM_SE_BOARD_71);
                    voicePlayF = TRUE;
                }
                for(j=0; j<8; j++) {
                    effPos.x = playerPosOther.x+((0.5f-MBCapsuleEffRandF())*175);
                    effPos.y = playerPosOther.y+(MBCapsuleEffRandF()*200);
                    effPos.z = playerPosOther.z+75+(MBCapsuleEffRandF()*20);
                    effRot.x = 0;
                    effRot.y = 0;
                    effRot.z = 0;
                    t = MBCapsuleEffRandF();
                    color.r = 192+(t*63);
                    color.g = 192+(t*63);
                    color.b = 192+(t*63);
                    color.a = 192+(MBCapsuleEffRandF()*63);
                    MBCapsuleExplodeEffAdd(0, effPos, effRot, (MBCapsuleEffRandF()+2)*100, 0, 0.33f, color);
                }
            }
            HuPrcVSleep();
        }
        CharEffectLayerSet(2);
        mode = 0;
        time = 0;
        sp6C = 0;
        takeDelay = 0;
        takeNum = 0;
        sp60 = 0;
        sp5C = 0;
        sp58  = 0;
        do {
            switch(mode) {
                case 0:
                    if(++time > 30.0f) {
                        time = 0;
                        mode = 1;
                    }
                    break;
                
                case 1:
                    MBPlayerPosSetV(otherPlayer, &playerPosOther);
                    MBPlayerRotSet(otherPlayer, 0, 0, 0);
                    MBPlayerMotionNoSet(otherPlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_NONE);
                    endPos = playerPosOther;
                    endPos.z -= 100;
                    MBModelPosSetV(mdlId, &endPos);
                    MBModelRotSet(mdlId, 0, 0, 0);
                    if(takeDelay >= 2 && (takeDelay & 0x1) && takeNum < num) {
                        i = MBPlayerCapsuleFind(otherPlayer, capsuleNo[i]);
                        if(i != -1) {
                            MBPlayerCapsuleRemove(otherPlayer, i);
                        }
                        MBAudFXPlay(MSM_SE_BOARD_25);
                        takeNum++;
                    }
                    takeDelay++;
                    time = 0;
                    if(takeDelay < 5 || takeNum < num) {
                        mode = 2;
                    } else {
                        mode = 3;
                    }
                    
                    break;
                
                case 2:
                    if((time & 0x7) == 0) {
                        omVibrate(otherPlayer, 12, 6, 6);
                    }
                    if(++time > 30.0f) {
                        mode = 1;
                        time = 0;
                    }
                    break;
                
                case 3:
                    break;
            }
            for(i=0; i<num && i<takeNum; i++) {
                capsuleTime[i] += 0.0055555557f;
                if(capsuleTime[i] >= 0.5f) {
                    MBModelDispSet(capsuleMdlId[i], FALSE);
                } else {
                    MBCapsuleBezierGetV(capsuleTime[i], (float *)&capsulePos[i][0], (float *)&capsulePos[i][1], (float *)&capsulePos[i][2], (float *)&endPos);
                    MBModelPosSetV(capsuleMdlId[i], &endPos);
                    MBModelDispSet(capsuleMdlId[i], TRUE);
                }
            }
            endPos.x = playerPosOther.x+((0.5f-MBCapsuleEffRandF())*175);
            endPos.y = playerPosOther.y+(MBCapsuleEffRandF()*200);
            endPos.z = playerPosOther.z+75+(MBCapsuleEffRandF()*20);
            startPos.x = 0;
            startPos.y = 0;
            startPos.z = 0;
            t = MBCapsuleEffRandF();
            color.r = 192+(t*63);
            color.g = 192+(t*63);
            color.b = 192+(t*63);
            color.a = 192+(MBCapsuleEffRandF()*63);
            MBCapsuleExplodeEffAdd(0, endPos, startPos, (MBCapsuleEffRandF()+2)*100, 0, 0.33f, color);
            HuPrcVSleep();
        } while(capsuleTime[num-1] < 0.5f || mode < 3);
        MBPlayerMotionNoSet(otherPlayer, MB_PLAYER_MOT_DIZZY, HU3D_MOTATTR_LOOP);
        MBPlayerPosSetV(otherPlayer, &playerPosOther);
        MBPlayerRotSet(otherPlayer, 0, 0, 0);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBModelDispSet(mdlId, FALSE);
        MBModelDispSet(ropeMdlId, FALSE);
        for(i=0; i<num; i++) {
            MBCameraSkipSet(FALSE);
            MBCameraFocusMasuSet(capsuleMasuId[i]);
            MBCameraMotionWait();
            MBCameraSkipSet(TRUE);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
            WipeWait();
            MBMasuPosGet(MASU_LAYER_DEFAULT, capsuleMasuId[i], &capsulePos[i][2]);
            VECSubtract(&capsulePos[i][2], &capsulePos[i][0], &endPos);
            VECScale(&endPos, &capsulePos[i][1], 0.5f);
            capsulePos[i][1].y += 2000;
            if(capsulePos[i][2].y > capsulePos[i][0].y) {
                capsulePos[i][1].y = capsulePos[i][2].y+1000;
            } else {
                capsulePos[i][1].y = capsulePos[i][0].y+1000;
            }
            MBCapsuleThrowAutoExec(capsuleMasuId[i], capsuleNo[i], &capsulePos[i][0], &capsulePos[i][1], &capsulePos[i][2]);
            HuPrcSleep(60);
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
            WipeWait();
        }
        MBCameraSkipSet(FALSE);
        MBCameraMasuViewSet(masuIdOther, FALSE, &capsuleViewOfs, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        MBModelRotSet(mdlId, 0, 0, 0);
        mdlPos.x = playerPosOther.x;
        mdlPos.y = playerPosOther.y+200;
        mdlPos.z = playerPosOther.z-50;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelPosSet(ropeMdlId, mdlPos.x, mdlPos.y+200, mdlPos.z);
        MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_LOOP);
        MBModelDispSet(mdlId, TRUE);
        MBModelDispSet(ropeMdlId, TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
        WipeWait();
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_BOARD_69);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_UKKI_RESULT, HUWIN_SPEAKER_UKKI);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_BOARD_69);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_UKKI_HELP_FAIL, HUWIN_SPEAKER_UKKI);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBAudFXPlay(MSM_SE_BOARD_70);
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPosOther.x;
        mdlPos.y = playerPosOther.y+1000+(-800*HuCos(t*90));
        mdlPos.z = playerPosOther.z-50;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelPosSet(ropeMdlId, mdlPos.x, mdlPos.y+200, mdlPos.z);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBModelDispSet(ropeMdlId, FALSE);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBStarMasuDispSet(masuIdOther, TRUE);
    MBMasuPlayerLightSet(capsulePlayer, TRUE);
    for(j=0; j<GW_PLAYER_MAX; j++) {
        MBPlayerDispSet(j, TRUE);
    }
    MBPlayerPosSetV(otherPlayer, &playerPosOld);
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(masuId, FALSE, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    WipeWait();
    MBCapsulePlayerMotSet(otherPlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    HuPrcEnd();
}

void MBCapsuleUkkiDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
}

void MBCapsuleJugemExec(void)
{
    int i; //r31
    int otherPlayer; //r30
    int mdlId; //r29
    int capsuleMdlId; //r28
    int j; //r19
    int hariMdlId; //r18
    int capsuleNo; //r17
    
    float t; //f31
    float scale; //f30
    
    Mtx hookMtx; //sp+0xD0
    int motFile[8]; //sp+0xB0
    HuVecF playerPos; //sp+0xA4
    HuVecF playerPosOther; //sp+0x98
    HuVecF hariPos; //sp+0x8C
    HuVecF hariPosNext; //sp+0x80
    HuVecF mdlPos; //sp+0x74
    HuVecF playerPosOld; //sp+0x68
    int playerTbl[GW_PLAYER_MAX]; //sp+0x58
    int num; //sp+0x54
    int validNum; //sp+0x50
    int masuId; //sp+0x4C
    int motId; //sp+0x48
    int masuIdOther; //sp+0x44
    int seNo; //sp+0x40
    
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    motFile[0] = CAPSULECHAR0_HSF_jugem_idle;
    motFile[1] = CAPSULECHAR0_HSF_jugem_fish;
    motFile[2] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR0_HSF_jugem, motFile, FALSE, 10, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    hariMdlId = MBCapsuleModelCreate(CAPSULECHAR0_HSF_jugem_hari, NULL, FALSE, 10, FALSE);
    MBModelDispSet(hariMdlId, FALSE);
    seNo = MBAudFXPlay(MSM_SE_BOARD_32);
    MBModelDispSet(mdlId, TRUE);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = playerPos.y+500+(-270*HuSin(t*90));
        mdlPos.z = -100+playerPos.z;
        MBModelPosSetV(mdlId, &mdlPos);
        if(t < 0.75f) {
            MBCapsuleEffHookCall(8, mdlId, FALSE, FALSE, FALSE);
        }
        HuPrcVSleep();
    }
    for(i=0, num=0, validNum=0; i<GW_PLAYER_MAX; i++) {
        if(i == capsulePlayer) {
            continue;
        }
        playerTbl[num] = i;
        num++;
        if(!GWTeamFGet() || !MBCapsuleTeamCheck(capsulePlayer, i)) {
            if(MBPlayerCapsuleNumGet(i) > 0 && MBPlayerAliveCheck(i)) {
                validNum++;
            }
        }
    }
    if(validNum <= 0) {
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_44);
        if(!GWTeamFGet()) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_CAPSULE_EMPTY, HUWIN_SPEAKER_JUGEM);
            MBTopWinWait();
            MBTopWinKill();
        } else {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_OTHER_TEAM, HUWIN_SPEAKER_JUGEM);
            MBTopWinWait();
            MBTopWinKill();
        }
    } else {
        if(!GWTeamFGet()) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_43);
            MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX03_JUGEM_CHOICE, HUWIN_SPEAKER_JUGEM, 0);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(playerTbl[0]), 0);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(playerTbl[1]), 1);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(playerTbl[2]), 2);
            for(i=0; i<num; i++) {
                if(!MBPlayerAliveCheck(playerTbl[i])) {
                    MBTopWinChoiceDisable(i);
                }
                if(MBPlayerCapsuleNumGet(playerTbl[i]) <= 0) {
                    MBTopWinChoiceDisable(i);
                }
                if(GWTeamFGet() && MBCapsuleTeamCheck(capsulePlayer, playerTbl[i])) {
                    MBTopWinChoiceDisable(i);
                }
            }
            MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
            if(GwPlayer[capsulePlayer].comF) {
                if(GWPartyFGet() == TRUE) {
                    MBCapsuleChoiceSet(MBCapsuleEffRand(3));
                } else {
                    MBCapsuleChoiceSet(0);
                }
            }
            MBTopWinWait();
            otherPlayer = MBWinLastChoiceGet();
            MBTopWinKill();
            if(otherPlayer >= 3) {
                otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_CAPSULE);
            } else {
                otherPlayer = playerTbl[otherPlayer];
            }
        } else {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBAudFXPlay(MSM_SE_GUIDE_43);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_TEAM, HUWIN_SPEAKER_JUGEM);
            MBTopWinWait();
            MBTopWinKill();
            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_CAPSULE);
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_END, HUWIN_SPEAKER_JUGEM);
        MBTopWinWait();
        MBTopWinKill();
        MBAudFXPlay(MSM_SE_BOARD_32);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.y = playerPos.y+500+(-270*HuCos(t*90));
            mdlPos.z = -100+playerPos.z;
            MBModelPosSetV(mdlId, &mdlPos);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        masuIdOther = GwPlayer[otherPlayer].masuId;
        MBPlayerPosGet(otherPlayer, &playerPosOther);
        num = MBPlayerCapsuleNumGet(otherPlayer);
        motId = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_325);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBStarMasuDispSet(masuIdOther, FALSE);
        MBMasuPlayerLightSet(capsulePlayer, FALSE);
        for(j=0; j<GW_PLAYER_MAX; j++) {
            if(otherPlayer != j) {
                MBPlayerDispSet(j, FALSE);
            }
        }
        MBPlayerPosGet(otherPlayer, &playerPosOld);
        MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[otherPlayer].masuId, &hariPos);
        MBPlayerPosSetV(otherPlayer, &hariPos);
        MBPlayerPosGet(otherPlayer, &playerPosOther);
        MBCameraSkipSet(FALSE);
        MBCameraMasuViewSet(masuIdOther, NULL, &capsuleViewOfs, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
        seNo = MBAudFXPlay(MSM_SE_BOARD_32);
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPosOther.x;
            mdlPos.y = playerPosOther.y+500+(-270*HuSin(t*90));
            mdlPos.z = -100+playerPosOther.z;
            MBModelPosSetV(mdlId, &mdlPos);
            HuPrcVSleep();
        }
        MBPlayerMotionNoShiftSet(otherPlayer, motId, 0, 8, HU3D_MOTATTR_NONE);
        MBModelDispSet(hariMdlId, TRUE);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_S", hookMtx);
            hariPosNext.x = hookMtx[0][3];
            hariPosNext.y = hookMtx[1][3];
            hariPosNext.z = hookMtx[2][3];
            hariPos.x = playerPosOther.x;
            hariPos.y = playerPosOther.y+100;
            hariPos.z = playerPosOther.z;
            VECSubtract(&hariPos, &hariPosNext, &hariPos);
            VECScale(&hariPos, &hariPos, t);
            VECAdd(&hariPosNext, &hariPos, &hariPos);
            MBModelPosSetV(hariMdlId, &hariPos);
            MBModelRotSet(hariMdlId, 0, 360*t, 0);
            HuPrcVSleep();
        }
        MBAudFXPlay(MSM_SE_BOARD_74);
        MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_NONE);
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        do {
            HuPrcVSleep();
        } while(MBMotionTimeGet(mdlId) < 10.0f);
        omVibrate(otherPlayer, 12, 6, 6);
        MBPlayerMotionNoShiftSet(otherPlayer, MB_PLAYER_MOT_SHOCK, 0, 8, HU3D_MOTATTR_NONE);
        if(MBPlayerCapsuleNumGet(otherPlayer) > 0) {
            capsuleNo = MBPlayerCapsuleGet(otherPlayer, MBCapsuleEffRand(MBPlayerCapsuleNumGet(otherPlayer)));
            if(capsuleNo != CAPSULE_NULL) {
                i = MBPlayerCapsuleFind(otherPlayer, capsuleNo);
                if(i != -1) {
                    MBPlayerCapsuleRemove(otherPlayer, i);
                }
            }
        } else {
            capsuleNo = CAPSULE_NULL;
        }
        if(capsuleNo != CAPSULE_NULL) {
            capsuleMdlId = MBCapsuleModelCreate(MBCapsuleMdlGet(capsuleNo), NULL, FALSE, 10, FALSE);
            MBModelAttrSet(capsuleMdlId, HU3D_MOTATTR_LOOP);
            MBModelLayerSet(capsuleMdlId, 2);
        } else {
            capsuleMdlId = MB_MODEL_NONE;
        }
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            hariPos.x = playerPosOther.x;
            hariPos.y = playerPosOther.y+100+(t*100);
            hariPos.z = playerPosOther.z;
            hariPos.x -= HuSin(t*180)*100;
            hariPos.y += 3*(100*fabs(HuSin(t*120)));
            hariPos.z += (HuSin(t*120)*100)*2;
            MBModelPosSetV(hariMdlId, &hariPos);
            MBModelRotSet(hariMdlId, -60*HuSin(t*180), 60*HuCos(t*90), 0);
            MBModelScaleSet(hariMdlId, 1-(t*0.5f), 1-(t*0.5f), 1-(t*0.5f));
            if(capsuleMdlId != MB_MODEL_NONE) {
                Hu3DModelObjMtxGet(MBModelIdGet(hariMdlId), "itemhook_G", hookMtx);
                hariPos.x = hookMtx[0][3];
                hariPos.y = hookMtx[1][3];
                hariPos.z = hookMtx[2][3];
                MBModelPosSetV(capsuleMdlId, &hariPos);
                MBModelScaleSet(capsuleMdlId, 1-(t*0.5f), 1-(t*0.5f), 1-(t*0.5f));
            }
            HuPrcVSleep();
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBModelDispSet(hariMdlId, FALSE);
        MBModelDispSet(capsuleMdlId, FALSE);
        MBStarMasuDispSet(masuIdOther, TRUE);
        MBMasuPlayerLightSet(capsulePlayer, TRUE);
        for(j=0; j<GW_PLAYER_MAX; j++) {
            MBPlayerDispSet(j, TRUE);
        }
        MBPlayerPosSetV(otherPlayer, &playerPosOld);
        MBCameraSkipSet(FALSE);
        MBCameraMasuViewSet(masuId, NULL, &capsuleViewOfs, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
        seNo = MBAudFXPlay(MSM_SE_BOARD_32);
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.y = playerPos.y+500+(-270*HuSin(t*90));
            mdlPos.z = -100+playerPos.z;
            MBModelPosSetV(mdlId, &mdlPos);
            HuPrcVSleep();
        }
        if(capsuleNo != CAPSULE_NULL) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_CAPSULE, HUWIN_SPEAKER_JUGEM);
            MBTopWinWait();
            MBTopWinKill();
            if(MBPlayerCapsuleNumGet(capsulePlayer) < MBPlayerCapsuleMaxGet()) {
                capsuleMdlId = MBCapsuleModelCreate(MBCapsuleMdlGet(capsuleNo), NULL, FALSE, 10, FALSE);
                MBModelAttrSet(capsuleMdlId, HU3D_MOTATTR_LOOP);
                MBModelLayerSet(capsuleMdlId, 2);
                playerPosOther.x = playerPos.x;
                playerPosOther.y = playerPos.y+240.00002f;
                playerPosOther.z = playerPos.z;
                MBModelPosSet(capsuleMdlId, playerPosOther.x, playerPosOther.y, playerPosOther.z);
                MBAudFXPlay(MSM_SE_BOARD_110);
                for(i=0; i<15.0f; i++) {
                    t = i/15.0f;
                    scale = t;
                    MBModelScaleSet(capsuleMdlId, scale, scale, scale);
                    HuPrcVSleep();
                }
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_RESULT, HUWIN_SPEAKER_NONE);
                MBTopWinInsertMesSet(MBCapsuleMesGet(capsuleNo), 0);
                MBTopWinWait();
                MBTopWinKill();
                for(i=0; i<15.0f; i++) {
                    t = i/15.0f;
                    scale = 1+(0.5f*t);
                    playerPosOther.x = playerPos.x;
                    playerPosOther.y = playerPos.y+240.00002f;
                    playerPosOther.z = playerPos.z;
                    MBModelPosSet(capsuleMdlId, playerPosOther.x, playerPosOther.y, playerPosOther.z);
                    MBModelScaleSet(capsuleMdlId, scale, scale, scale);
                    HuPrcVSleep();
                }
                MBAudFXPlay(MSM_SE_BOARD_38);
                for(i=0; i<60.0f; i++) {
                    t = 1-(i/60.0f);
                    scale = t*1.5f;
                    playerPosOther.x = playerPos.x+(HuSin(t*360)*HuSin(t*180)*100);
                    playerPosOther.y = ((playerPos.y+240.00002f)-140)+(140*t);
                    playerPosOther.z = playerPos.z+(HuCos(t*360)*HuSin(t*180)*100);
                    MBModelPosSet(capsuleMdlId, playerPosOther.x, playerPosOther.y, playerPosOther.z);
                    MBModelScaleSet(capsuleMdlId, scale, scale, scale);
                    HuPrcVSleep();
                }
                MBPlayerCapsuleAdd(capsulePlayer, capsuleNo);
                omVibrate(capsulePlayer, 12, 4, 2);
                MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
                MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINWIN, HU3D_MOTATTR_NONE, TRUE);
                MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_LUCK, HUWIN_SPEAKER_JUGEM);
                MBTopWinWait();
                MBTopWinKill();
            } else {
                MBAudFXPlay(MSM_SE_GUIDE_43);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_INSSPACE, HUWIN_SPEAKER_JUGEM);
                MBTopWinWait();
                MBTopWinKill();
            }
        } else {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_FAIL, HUWIN_SPEAKER_JUGEM);
            MBTopWinWait();
            MBTopWinKill();
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX03_JUGEM_LUCK, HUWIN_SPEAKER_JUGEM);
            MBTopWinWait();
            MBTopWinKill();
        }
    }
    seNo = MBAudFXPlay(MSM_SE_BOARD_32);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        mdlPos.x = playerPos.x;
        mdlPos.y = playerPos.y+500+(-270*HuCos(t*90));
        mdlPos.z = -100+playerPos.z;
        MBModelPosSetV(mdlId, &mdlPos);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    HuPrcEnd();
}

void MBCapsuleJugemDestroy(void)
{
    
}

void MBCapsuleTumujikunExec(void)
{
    int i; //r31
    int starId; //r25
    int mdlId; //r22
    
    float t; //f31
    float rotY; //f30
    float yOfs; //f29
    float rotSpeed; //f28
    
    int motFile[8]; //sp+0x7C
    HuVecF masuPos; //sp+0x70
    HuVecF mdlPos; //sp+0x64
    HuVecF guidePos; //sp+0x58
    int starNoNext; //sp+0x54
    int starMasuIdNext; //sp+0x50
    int starMasuId; //sp+0x4C
    int kazeMdlId; //sp+0x48
    int seNo; //sp+0x44
    int guideMdlId; //sp+0x40
    u8 starNo; //sp+0x8
    
    starNo = MBStarNextNoGet();
    starMasuId = MBMasuStarGet(starNo);
    MBMasuPosGet(MASU_LAYER_DEFAULT, starMasuId, &masuPos);
    motFile[0] = CAPSULECHAR1_HSF_tsumuji_idle;
    motFile[1] = CAPSULECHAR1_HSF_tsumuji_in;
    motFile[2] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_tsumuji, motFile, FALSE, 10, FALSE);
    MBModelDispSet(mdlId, FALSE);
    MBModelLayerSet(mdlId, 2);
    kazeMdlId = MBCapsuleModelCreate(CAPSULECHAR1_HSF_tsumuji_kaze, motFile, FALSE, 10, FALSE);
    MBModelDispSet(kazeMdlId, FALSE);
    MBModelLayerSet(mdlId, 2);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    MBCameraSkipSet(FALSE);
    MBCameraFocusMasuSet(starMasuId);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBStarMasuDispSet(starMasuId, FALSE);
    starId = MBCapsuleStarCreate();
    MBCapsuleStarPosSet(starId, masuPos.x, masuPos.y+300, masuPos.z);
    MBCapsuleStarObjCreate(starId);
    for(i=0; i<GW_PLAYER_MAX; i++) {
         if(!MBPlayerAliveCheck(i)) {
             continue;
         }
         if(starMasuId == GwPlayer[i].masuId) {
             MBPlayerDispSet(i, FALSE);
         }
    }
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    WipeWait();
    mdlPos = masuPos;
    mdlPos.y += 10;
    MBMotionTimeSet(kazeMdlId, 0);
    MBMotionSpeedSet(kazeMdlId, 0.5f);
    MBModelPosSetV(kazeMdlId, &mdlPos);
    MBModelDispSet(kazeMdlId, TRUE);
    for(i=0; i<15; i++) {
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_BOARD_75);
    seNo = MBAudFXPlay(MSM_SE_BOARD_76);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelDispSet(mdlId, TRUE);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        MBModelScaleSet(mdlId, 1, HuSin(t*90), 1);
        HuPrcVSleep();
    }
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_TUMUJIKUN, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBTopWinKill();
    MBAudFXPlay(MSM_SE_BOARD_77);
    for(i=0, rotY=0, rotSpeed=0; i<120.0f; i++) {
        t = i/120.0f;
        MBModelScaleSet(mdlId, t+1, t+1, t+1);
        MBCapsuleStarRotSet(starId, 0, -rotY, 0);
        if((rotY += rotSpeed) > 360) {
            rotY -= 360;
        }
        rotSpeed = t*20;
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, TRUE);
    for(i=0; i<120.0f; i++) {
        t = i/120.0f;
        if(t >= 0.5f) {
            yOfs = (HuSin((t-0.5f)*90)*1000)+10;
        } else {
            yOfs = 10;
        }
        mdlPos.x = masuPos.x;
        mdlPos.y = masuPos.y+yOfs;
        mdlPos.z = masuPos.z;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        yOfs = 300+(5*(t*100));
        MBCapsuleStarPosSet(starId, masuPos.x, masuPos.y+yOfs, masuPos.z);
        MBCapsuleStarRotSet(starId, 0, -rotY, 0);
        if((rotY += rotSpeed) > 360) {
            rotY -= 360;
        }
        HuPrcVSleep();
    }
    starNoNext = MBStarNoRandGet();
    if(starNoNext == -1) {
        starNoNext = 0;
    }
    starMasuIdNext = MBMasuStarGet(starNoNext);
    MBMasuPosGet(MASU_LAYER_DEFAULT, starMasuIdNext, &masuPos);
    if(starMasuIdNext == GwPlayer[capsulePlayer].masuId) {
        capsuleFlags.moveF = TRUE;
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(starMasuId == GwPlayer[i].masuId) {
            MBPlayerDispSet(i, TRUE);
        }
    }
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(GwPlayer[capsulePlayer].masuId, NULL, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBAudFXPlay(MSM_SE_BOARD_78);
    if(seNo != MSM_SENO_NONE) {
        MBAudFXStop(seNo);
    }
    MBModelDispSet(mdlId, FALSE);
    MBDataAsyncWait(MBDataDirReadAsync(DATA_guide));
    guidePos.x = 68;
    guidePos.y = 384;
    guidePos.z = 1000;
    guideMdlId = MBGuideCreateIn(&guidePos, TRUE, FALSE, FALSE);
    MBModelLayerSet(guideMdlId, 2);
    HuDataDirClose(DATA_guide);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    WipeWait();
    MBAudFXPlay(MSM_SE_GUIDE_51);
    MBWinCreate(MBWIN_TYPE_GUIDE, MESS_CAPSULE_EX04_TUMUJIKUN_RESULT, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBTopWinKill();
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    MBGuideEnd(guideMdlId);
    MBStarNoReset(starNo);
    MBCapsuleStarKill(starId);
    MBStarDispSetAll(TRUE);
    MBMasuPosGet(MASU_LAYER_DEFAULT, starMasuId, &guidePos);
    MBStarMapViewProcExec(capsulePlayer, &guidePos, starNoNext);
    MBCameraSkipSet(FALSE);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBStarMasuDispSet(GwPlayer[capsulePlayer].masuId, FALSE);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
    }
    WipeWait();
    HuPrcEnd();
}

void MBCapsuleTumujikunDestroy(void)
{
    
}

typedef struct MgResult_s {
    s16 playerNo1;
    s16 playerNo2;
    s16 coinNum;
    s16 starNum;
    s16 resultNo;
} MG_RESULT;

static MG_RESULT MgResultData;

void MBCapsuleKettouExec(void)
{
    int i; //r31
    int otherPlayer; //r30
    int mdlId; //r29
    int coinNum; //r28
    int coinTakeNum; //r27
    int playerNum; //r26
    int sprId; //r25
    int time; //r24
    int seNo; //r23
    BOOL starEnableF; //r22
    int coinDir; //r21
    int coinDelay; //r20
    BOOL initCoinNumF; //r19
    int coinWinSprId; //r18
    int coinSprId; //r17
    
    float t; //f31
    float scaleY; //f30
    
    int motFile[8]; //sp+0x10C
    HuVecF playerPos; //sp+0x100
    HuVecF mdlPos; //sp+0xF4
    HuVecF coinPos; //sp+0xE8
    int playerNo[GW_PLAYER_MAX]; //sp+0xD8
    int playerCoinNum[GW_PLAYER_MAX]; //sp+0xC8
    int coinNumSprId[3]; //sp+0xBC
    char insertMes1[16]; //sp+0xAC
    char insertMes2[16]; //sp+0x9C
    int winId[2]; //sp+0x70
    int coinDispId[2]; //sp+0x68
    int starNumDispId[2]; //sp+0x60
    int masuId; //sp+0x5C
    int choiceStar; //sp+0x58
    int choice; //sp+0x54
    int prevCoin; //sp+0x50
    int padBtn; //sp+0x4C
    int padStk; //sp+0x48
    int motId; //sp+0x44
    int circuitSaiResult; //sp+0x40
    int circuitPlayer; //sp+0x3C
    int prevCoinDir; //sp+0x38
    BOOL coinEnableF; //sp+0x34
    BOOL coinChoiceF; //sp+0x30
    BOOL starChoiceF; //sp+0x2C
    
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    motFile[0] = CAPSULECHAR0_HSF_heiho_zenmai_mot;
    motFile[1] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR0_HSF_heiho_zenmai, motFile, FALSE, 10, FALSE);
    MBModelDispSet(mdlId, FALSE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    coinWinSprId = sprId = MBCapsuleSprCreate(CAPSULE_ANM_duelCoinWin, 120, 0);
    espPosSet(sprId, 288, 224);
    espTPLvlSet(sprId, 0.8f);
    espDispOff(sprId);
    coinSprId = sprId = MBCapsuleSprCreate(CAPSULE_ANM_duelCoin, 110, 0);
    espPosSet(sprId, 248, 224);
    espDispOff(sprId);
    coinNumSprId[0] = sprId = MBCapsuleSprCreate(CAPSULE_ANM_duelCoinNum, 100, 0);
    espPosSet(sprId, 282, 224);
    espBankSet(sprId, 10);
    espDispOff(sprId);
    coinNumSprId[1] = sprId = MBCapsuleSprCreate(CAPSULE_ANM_duelCoinNum, 100, 0);
    espPosSet(sprId, 306, 224);
    espBankSet(sprId, 0);
    espDispOff(sprId);
    coinNumSprId[2] = sprId = MBCapsuleSprCreate(CAPSULE_ANM_duelCoinNum, 100, 0);
    espPosSet(sprId, 330, 224);
    espBankSet(sprId, 1);
    espDispOff(sprId);
    initCoinNumF = FALSE;
    seNo = MSM_SENO_NONE;
    
    if(!capsuleFlags.kettouMgEndF && !capsuleFlags.circuitKettouF && !capsuleFlags.circuitKettouMgEndF && GWPartyFGet() == TRUE) {
        MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_BG)) {
            HuPrcVSleep();
        }
        seNo = MBAudFXPlay(MSM_SE_BOARD_79);
        MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_BATTLE, 127, 0);
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<=60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.z = -100+playerPos.z;
            mdlPos.y = playerPos.y+500+(-300*HuSin(t*90));
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_07);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU, HUWIN_SPEAKER_HEYHO);
        MBTopWinWait();
        MBTopWinKill();
        if(MBPlayerStarGet(capsulePlayer) <= 0 && MBPlayerCoinGet(capsulePlayer) < 50) {
            starEnableF = FALSE;
        } else {
            starEnableF = TRUE;
        }
        if(GWPartyFGet() == FALSE) {
            starEnableF = FALSE;
        }
        if(MBPlayerCoinGet(capsulePlayer) <= 0) {
            coinEnableF = FALSE;
        } else {
            coinEnableF = TRUE;
        }
        if(!starEnableF && !coinEnableF) {
            playerNum = 0;
        } else {
            if(!capsuleFlags.kettouF) {
                for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
                    if(!MBPlayerAliveCheck(i)) {
                        continue;
                    }
                    if(MBPlayerCoinGet(i) <= 0 && MBPlayerStarGet(i) <= 0) {
                        continue;
                    }
                    if(!starEnableF && MBPlayerCoinGet(i) <= 0) {
                        continue;
                    }
                    if(!coinEnableF && MBPlayerStarGet(i) <= 0) {
                        continue;
                    }
                    if(GWTeamFGet() && MBCapsuleTeamCheck(capsulePlayer, i)) {
                        continue;
                    }
                    if(i != capsulePlayer) {
                        playerNo[playerNum] = i;
                        playerNum++;
                    }
                }
            } else {
                for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
                    if(!MBPlayerAliveCheck(i)) {
                        continue;
                    }
                    if(MBPlayerCoinGet(i) <= 0 && MBPlayerStarGet(i) <= 0) {
                        continue;
                    }
                    if(!starEnableF && MBPlayerCoinGet(i) <= 0) {
                        continue;
                    }
                    if(!coinEnableF && MBPlayerStarGet(i) <= 0) {
                        continue;
                    }
                    if(GWTeamFGet() && MBCapsuleTeamCheck(capsulePlayer, i)) {
                        continue;
                    }
                    if(i != capsulePlayer && masuId == GwPlayer[i].masuId) {
                        playerNo[playerNum] = i;
                        playerNum++;
                    }
                }
            }
            if(MBPlayerCoinGet(capsulePlayer) <= 0 && MBPlayerStarGet(capsulePlayer) <= 0) {
                playerNum = 0;
            }
        }
        repeatChoice:
        switch(playerNum) {
            case 3:
                repeat3:
                MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE3, HUWIN_SPEAKER_HEYHO, 0);
                if(GwPlayer[capsulePlayer].comF) {
                    if(GWPartyFGet() == TRUE) {
                        if(starEnableF) {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_STAR);
                        } else {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                        }
                        if(otherPlayer == playerNo[0]) {
                            MBCapsuleChoiceSet(0);
                        } else if(otherPlayer == playerNo[1]) {
                            MBCapsuleChoiceSet(1);
                        } else if(otherPlayer == playerNo[2]) {
                            MBCapsuleChoiceSet(2);
                        } else {
                            MBCapsuleChoiceSet(3);
                        }
                    } else {
                        MBCapsuleChoiceSet(0);
                    }
                }
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[0]), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[1]), 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[2]), 2);
                MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
                MBTopWinWait();
                otherPlayer = MBWinLastChoiceGet();
                MBTopWinKill();
                if(otherPlayer == 4) {
                    MBScrollExec(capsulePlayer);
                    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                }
                if(otherPlayer == 4) {
                    goto repeat3;
                }
                if(otherPlayer >= 3) {
                    if(starEnableF) {
                        if(!capsuleFlags.kettouF) {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_STAR);
                        } else {
                            otherPlayer = MBCapsulePlayerSameRandGet(capsulePlayer, CAPSULE_PLAYERRAND_STAR, TRUE);
                        }
                    } else {
                        if(!capsuleFlags.kettouF) {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                        } else {
                            otherPlayer = MBCapsulePlayerSameRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN, TRUE);
                        }
                    }
                    if(otherPlayer < 0) {
                        otherPlayer = playerNo[0];
                    }
                } else {
                    otherPlayer = playerNo[otherPlayer];
                }
                break;
            
            case 2:
                repeat2:
                MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE2, HUWIN_SPEAKER_HEYHO, 0);
                if(GwPlayer[capsulePlayer].comF) {
                    if(GWPartyFGet() == TRUE) {
                        if(starEnableF) {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_STAR);
                        } else {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                        }
                        if(otherPlayer == playerNo[0]) {
                            MBCapsuleChoiceSet(0);
                        } else if(otherPlayer == playerNo[1]) {
                            MBCapsuleChoiceSet(1);
                        }else {
                            MBCapsuleChoiceSet(2);
                        }
                    } else {
                        MBCapsuleChoiceSet(0);
                    }
                }
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[0]), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[1]), 1);
                MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
                MBTopWinWait();
                otherPlayer = MBWinLastChoiceGet();
                MBTopWinKill();
                if(otherPlayer == 3) {
                    MBScrollExec(capsulePlayer);
                    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                }
                if(otherPlayer == 3) {
                    goto repeat2;
                }
                if(otherPlayer >= 2) {
                    if(starEnableF) {
                        if(!capsuleFlags.kettouF) {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_STAR);
                        } else {
                            otherPlayer = MBCapsulePlayerSameRandGet(capsulePlayer, CAPSULE_PLAYERRAND_STAR, TRUE);
                        }
                    } else {
                        if(!capsuleFlags.kettouF) {
                            otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                        } else {
                            otherPlayer = MBCapsulePlayerSameRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN, TRUE);
                        }
                    }
                    if(otherPlayer < 0) {
                        otherPlayer = playerNo[0];
                    }
                } else {
                    otherPlayer = playerNo[otherPlayer];
                }
                break;
                
            case 1:
                otherPlayer = playerNo[0];
                break;
            
            default:
                MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                MBAudFXPlay(MSM_SE_GUIDE_08);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_FAIL, HUWIN_SPEAKER_HEYHO);
                MBTopWinWait();
                MBTopWinKill();
                MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                while(MBMusCheck(MB_MUS_CHAN_FG)) {
                    HuPrcVSleep();
                }
                for(i=0; i<60.0f; i++) {
                    t = i/60.0f;
                    mdlPos.x = playerPos.x;
                    mdlPos.z = -100+playerPos.z;
                    mdlPos.y = playerPos.y+500+(-300*HuCos(t*90));
                    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                    HuPrcVSleep();
                }
                MBModelDispSet(mdlId, FALSE);
                MBMusMainPlay();
                goto end;
        }
        repeatResult:
        if(playerNum > 1) {
            MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE_RESULT, HUWIN_SPEAKER_HEYHO, 0);
        } else {
            MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE_RESULT_NOCANCEL, HUWIN_SPEAKER_HEYHO, 0);
        }
        MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
        coinChoiceF = starChoiceF = TRUE;
        if(MBPlayerCoinGet(otherPlayer) <= 0 || !coinEnableF) {
            MBTopWinChoiceDisable(0);
            coinChoiceF = FALSE;
        }
        if(MBPlayerStarGet(otherPlayer) <= 0 || !starEnableF) {
            MBTopWinChoiceDisable(1);
            starChoiceF = FALSE;
        }
        if(GwPlayer[capsulePlayer].comF) {
            if(coinChoiceF && starChoiceF) {
                MBComChoiceSetDown();
            } else {
                MBComChoiceSetUp();
            }
        }
        MBTopWinWait();
        choice = MBWinLastChoiceGet();
        if(choice == 2) {
            goto repeatChoice;
        }
        if(choice == 0) {
            prevCoin = coinNum = 1;
            sprintf(insertMes1, "%d", coinNum);
            prevCoinDir = coinDir = coinDelay = 0;
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_COIN_START, HUWIN_SPEAKER_HEYHO);
            MBTopWinWait();
            MBTopWinKill();
            if(!initCoinNumF) {
                espDispOn(coinWinSprId);
                espDispOn(coinSprId);
                for(i=0; i<3; i++) {
                    espDispOn(coinNumSprId[i]);
                }
                espBankSet(coinNumSprId[1], 0);
                espBankSet(coinNumSprId[2], 1);
                for(time=0; time<=12.0f; time++) {
                    t = time/12.0f;
                    scaleY = HuSin(t*90);
                    espScaleSet(coinWinSprId, 1, scaleY);
                    espScaleSet(coinSprId, 1, scaleY);
                    for(i=0; i<3; i++) {
                        espScaleSet(coinNumSprId[i], 1, scaleY);
                    }
                    HuPrcVSleep();
                }
                winId[1] = MBWinCreateHelp(MESS_CAPSULE_EX04_KETTOU_COIN_HELP);
                MBWinPosSet(winId[1], 115, 288);
                initCoinNumF = TRUE;
            }
            if(GwPlayer[capsulePlayer].comF) {
                coinTakeNum = MBPlayerCoinGet(capsulePlayer);
                switch(GwPlayer[capsulePlayer].charNo) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        coinTakeNum *= (MBCapsuleEffRandF()*0.5f)+0.2f;
                        break;
                    
                    case 10:
                        coinTakeNum--;
                        break;
                    
                    case 11:
                        coinTakeNum /= 2;
                        break;
                    
                    case 12:
                        if(frand() & 0x1) {
                            (void)coinTakeNum;
                            break;
                        } else {
                            coinTakeNum *= (MBCapsuleEffRandF()*0.5f)+0.2f;
                        }
                        break;
                    
                    default:
                        coinTakeNum *= MBCapsuleEffRandF();
                        break;
                }
                if(coinTakeNum > MBPlayerCoinGet(otherPlayer)) {
                    coinTakeNum = MBPlayerCoinGet(otherPlayer);
                }
                if(coinTakeNum < 1) {
                    coinTakeNum = 1;
                }
                if(coinTakeNum > 99) {
                    coinTakeNum = 99;
                }
            }
            while(1) {
                padBtn = HuPadBtnDown[GwPlayer[capsulePlayer].padNo];
                padStk = MBPadStkYGet(GwPlayer[capsulePlayer].padNo);
                if(GwPlayer[capsulePlayer].comF) {
                    padBtn = 0;
                    padStk = 0;
                    if(coinNum < coinTakeNum) {
                        padStk = 32;
                    } else {
                        HuPrcSleep(5);
                        padBtn = PAD_BUTTON_A;
                    }
                }
                if(padBtn & PAD_BUTTON_A) {
                    MBAudFXPlay(MSM_SE_CMN_03);
                    break;
                } else if(padBtn & PAD_BUTTON_B) {
                    if(!initCoinNumF) {
                        goto repeatResult;
                    }
                    for(time=0; time<=12.0f; time++) {
                        t = time/12.0f;
                        scaleY = HuCos(t*90);
                        espScaleSet(coinWinSprId, 1, scaleY);
                        espScaleSet(coinSprId, 1, scaleY);
                        for(i=0; i<3; i++) {
                            espScaleSet(coinNumSprId[i], 1, scaleY);
                        }
                        HuPrcVSleep();
                    }
                    espDispOff(coinWinSprId);
                    espDispOff(coinSprId);
                    for(i=0; i<3; i++) {
                        espDispOff(coinNumSprId[i]);
                    }
                    MBWinKill(winId[1]);
                    initCoinNumF = FALSE;
                    goto repeatResult;
                } else {
                    if(fabs(padStk) >= 8.0) {
                        if(padStk >= 10) {
                            coinDir = 1;
                        }
                        if(padStk <= 10) {
                            coinDir = -1;
                        }
                        if(coinDir == 0) {
                            coinDelay = 0;
                        } else if(prevCoinDir == coinDir) {
                            if(++coinDelay > 30.0f) {
                                coinNum += coinDir;
                                coinDelay -= 2;
                            }
                        } else {
                            coinNum += coinDir;
                            coinDelay = 0;
                        }
                        prevCoinDir = coinDir;
                        if(coinNum < 1) {
                            coinNum = 1;
                        } else if(coinNum > 99) {
                            coinNum = 99;
                        } else if(coinNum > MBPlayerCoinGet(capsulePlayer)) {
                            coinNum = MBPlayerCoinGet(capsulePlayer);
                        } else if(coinNum > MBPlayerCoinGet(otherPlayer)) {
                            coinNum = MBPlayerCoinGet(otherPlayer);
                        }
                    } else {
                        prevCoinDir = coinDir = coinDelay = 0;
                    }
                    if(prevCoin != coinNum) {
                        MBAudFXPlay(MSM_SE_CMN_01);
                        if(coinNum >= 100) {
                            espBankSet(coinNumSprId[1], 9);
                        } else {
                            espBankSet(coinNumSprId[1], coinNum/10);
                        }
                        espBankSet(coinNumSprId[2], coinNum%10);
                        prevCoin = coinNum;
                        if(coinDelay < 27.0f) {
                            HuPrcSleep(3);
                        }
                    }
                }
                HuPrcVSleep();
            }
            if(initCoinNumF) {
                for(time=0; time<=12.0f; time++) {
                    t = time/12.0f;
                    scaleY = HuCos(t*90);
                    espScaleSet(coinWinSprId, 1, scaleY);
                    espScaleSet(coinSprId, 1, scaleY);
                    for(i=0; i<3; i++) {
                        espScaleSet(coinNumSprId[i], 1, scaleY);
                    }
                    HuPrcVSleep();
                }
                espDispOff(coinWinSprId);
                espDispOff(coinSprId);
                for(i=0; i<3; i++) {
                    espDispOff(coinNumSprId[i]);
                }
                MBWinKill(winId[1]);
                initCoinNumF = FALSE;
            }
            choiceStar = 0;
        } else {
            MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE_RESULT_STAR, HUWIN_SPEAKER_HEYHO, 0);
            MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
            coinChoiceF = starChoiceF = TRUE;
            if(MBPlayerCoinGet(capsulePlayer) < 50) {
                MBTopWinChoiceDisable(0);
                coinChoiceF = FALSE;
            }
            if(MBPlayerStarGet(capsulePlayer) <= 0) {
                MBTopWinChoiceDisable(1);
                starChoiceF = FALSE;
            }
            if(GwPlayer[capsulePlayer].comF) {
                if(coinChoiceF) {
                    MBComChoiceSetUp();
                } else {
                    MBComChoiceSetRight();
                }
            }
            MBTopWinWait();
            choiceStar = MBWinLastChoiceGet();
            if(choiceStar == 2) {
                goto repeatResult;
            }
            if(choiceStar == 0) {
                coinNum = 50;
            } else {
                coinNum = 0;
            }
        }
        MBCapsuleStatusPosMoveWait(FALSE, TRUE);
        MBCapsuleStatusKettouInSet(capsulePlayer, otherPlayer, TRUE);
        if(choice == 0) {
            coinTakeNum = coinNum;
            if(GwSystem.last5Effect != MB_LAST5_EFF_TAKEDOUBLE) {
                sprintf(insertMes1, "%d", coinNum);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_COIN_NUM, HUWIN_SPEAKER_HEYHO);
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes1), 0);
                MBTopWinWait();
                MBTopWinKill();
            } else {
                sprintf(insertMes1, "%d", coinNum);
                sprintf(insertMes2, "%d", coinTakeNum*2);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_COIN_TAKE_DOUBLE, HUWIN_SPEAKER_HEYHO);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 0);
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes1), 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 2);
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes2), 3);
                MBTopWinWait();
                MBTopWinKill();
            }
            if(GwSystem.last5Effect == MB_LAST5_EFF_TAKEDOUBLE) {
                coinTakeNum *= 2;
                if(coinTakeNum > MBPlayerCoinGet(otherPlayer)) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_COIN_INS, HUWIN_SPEAKER_HEYHO);
                    MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 0);
                    MBTopWinWait();
                    MBTopWinKill();
                    coinTakeNum = MBPlayerCoinGet(otherPlayer);
                }
            }
            for(i=0; i<GW_PLAYER_MAX; i++) {
                playerCoinNum[i] = 0;
            }
            playerCoinNum[capsulePlayer] = -coinNum;
            playerCoinNum[otherPlayer] = -coinNum;
            MBCoinAddAllDispExec(playerCoinNum[0], playerCoinNum[1], playerCoinNum[2], playerCoinNum[3], TRUE);
            sprintf(insertMes1, "%d", coinNum+coinTakeNum);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_COIN_TOTAL, HUWIN_SPEAKER_HEYHO);
            MBTopWinInsertMesSet(MESSNUM_PTR(insertMes1), 0);
            MBTopWinWait();
            MBTopWinKill();
            memset(&MgResultData, 0, sizeof(MgResultData));
            MgResultData.playerNo1 = capsulePlayer;
            MgResultData.playerNo2 = otherPlayer;
            MgResultData.coinNum = coinNum+coinTakeNum;
        } else if(choiceStar != 0) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_STAR, HUWIN_SPEAKER_HEYHO);
            MBTopWinWait();
            MBTopWinKill();
            MBPlayerStarAdd(capsulePlayer, -1);
            MBPlayerStarAdd(otherPlayer, -1);
            starNumDispId[0] = MBCapsuleStarNumCreate(capsulePlayer, -1);
            HuPrcVSleep();
            starNumDispId[1] = MBCapsuleStarNumCreate(otherPlayer, -1);
            do {
                HuPrcVSleep();
            } while(!MBCapsuleStarNumCheck(starNumDispId[0]) || !MBCapsuleStarNumCheck(starNumDispId[1]));
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_STAR_WIN, HUWIN_SPEAKER_HEYHO);
            MBTopWinWait();
            MBTopWinKill();
            memset(&MgResultData, 0, sizeof(MgResultData));
            MgResultData.playerNo1 = capsulePlayer;
            MgResultData.playerNo2 = otherPlayer;
            MgResultData.starNum = 2;
        } else {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_TAKE_COIN_STAR_START, HUWIN_SPEAKER_HEYHO);
            MBTopWinWait();
            MBTopWinKill();
            MBCoinAddExec(capsulePlayer, -coinNum);
            MBPlayerStarAdd(otherPlayer, -1);
            MBPlayerPosGet(capsulePlayer, &coinPos);
            coinPos.y +=  250;
            coinDispId[0] = MBCoinDispCreateSe(&coinPos, -coinNum);
            starNumDispId[1] = MBCapsuleStarNumCreate(otherPlayer, -1);
            do {
                HuPrcVSleep();
            } while(!MBCoinDispKillCheck(coinDispId[0]) || !MBCapsuleStarNumCheck(starNumDispId[1]));
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_TAKE_COIN_STAR, HUWIN_SPEAKER_HEYHO);
            MBTopWinWait();
            MBTopWinKill();
            memset(&MgResultData, 0, sizeof(MgResultData));
            MgResultData.playerNo1 = capsulePlayer;
            MgResultData.playerNo2 = otherPlayer;
            MgResultData.coinNum = coinNum;
            MgResultData.starNum = 1;
        }
        MBCapsuleStatusKettouOutSet(capsulePlayer, otherPlayer, TRUE);
        GWMgCoinBonusSet(MgResultData.playerNo1, 0);
        GWMgCoinBonusSet(MgResultData.playerNo2, 0);
        if(GwPlayer[MgResultData.playerNo1].comF && GwPlayer[MgResultData.playerNo2].comF && !GWMgComFGet()) {
            int chanceTbl[3];
            int totalChance;
            int chance;
            int difDelta;
            difDelta = abs(GwPlayer[MgResultData.playerNo1].dif-GwPlayer[MgResultData.playerNo2].dif);
            if(difDelta < 0) {
                difDelta = 0;
            } else if(difDelta > 3) {
                difDelta = 3;
            }
            chanceTbl[0] = 15-(difDelta*4);
            chanceTbl[1] = (GwPlayer[MgResultData.playerNo1].dif*20)+50;
            chanceTbl[2] = (GwPlayer[MgResultData.playerNo2].dif*20)+50;
            totalChance = chanceTbl[0]+chanceTbl[1]+chanceTbl[2];
            chance = MBCapsuleEffRand(totalChance);
            if(chance < chanceTbl[0]) {
                GWMgCoinBonusSet(MgResultData.playerNo1, 0);
                GWMgCoinBonusSet(MgResultData.playerNo2, 0);
            } else if(chance < chanceTbl[1]) {
                GWMgCoinBonusSet(MgResultData.playerNo1, 10);
            } else {
                GWMgCoinBonusSet(MgResultData.playerNo2, 10);
            }
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            MBCapsuleCameraViewPlayerSet(capsulePlayer);
            MBCameraMotionWait();
            MBCapsuleStatusPosMoveWait(TRUE, FALSE);
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            goto partyEnd;
        } else {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grp = 2;
            }
            GwPlayerConf[capsulePlayer].grp = 0;
            GwPlayerConf[otherPlayer].grp = 1;
            MBMgCallKettouExec();
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            MBMusMainPlay();
            goto end;
        }
    } else if(!capsuleFlags.kettouMgEndF && !capsuleFlags.circuitKettouF && !capsuleFlags.circuitKettouMgEndF && GWPartyFGet() == FALSE) {
        MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_BG)) {
            HuPrcVSleep();
        }
        seNo = MBAudFXPlay(MSM_SE_BOARD_79);
        MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_BATTLE, 127, 0);
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<=60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.z = -100+playerPos.z;
            mdlPos.y = playerPos.y+500+(-300*HuSin(t*90));
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_07);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU, HUWIN_SPEAKER_HEYHO);
        MBTopWinWait();
        MBTopWinKill();
        if(!capsuleFlags.kettouF) {
            for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
                if(!MBPlayerAliveCheck(i)) {
                    continue;
                }
                if(i != capsulePlayer) {
                    playerNo[playerNum] = i;
                    playerNum++;
                }
            }
        } else {
            for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
                if(!MBPlayerAliveCheck(i)) {
                    continue;
                }
                if(masuId == GwPlayer[i].masuId && i != capsulePlayer) {
                    playerNo[playerNum] = i;
                    playerNum++;
                }
            }
        }
        switch(playerNum) {
            case 3:
                repeat3story:
                MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE3, HUWIN_SPEAKER_HEYHO, 0);
                if(GwPlayer[capsulePlayer].comF) {
                    if(MBPlayerStoryComCheck(capsulePlayer)) {
                        MBCapsuleChoiceSet(0);
                    } else {
                        MBCapsuleChoiceSet(MBCapsuleEffRand(3));
                    }
                }
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[0]), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[1]), 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[2]), 2);
                MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
                MBTopWinWait();
                otherPlayer = MBWinLastChoiceGet();
                MBTopWinKill();
                if(otherPlayer == 4) {
                    MBScrollExec(capsulePlayer);
                    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                }
                if(otherPlayer == 4) {
                    goto repeat3story;
                }
                if(otherPlayer >= 3) {
                    if(!capsuleFlags.kettouF) {
                        otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                    } else {
                        otherPlayer = MBCapsulePlayerSameRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN, TRUE);
                    }
                    if(otherPlayer < 0) {
                        otherPlayer = playerNo[0];
                    }
                } else {
                    otherPlayer = playerNo[otherPlayer];
                }
                break;
                
            case 2:
                repeat2story:
                MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE2, HUWIN_SPEAKER_HEYHO, 0);
                if(GwPlayer[capsulePlayer].comF) {
                    if(MBPlayerStoryComCheck(capsulePlayer)) {
                        MBCapsuleChoiceSet(0);
                    } else {
                        MBCapsuleChoiceSet(MBCapsuleEffRand(2));
                    }
                }
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[0]), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[1]), 1);
                MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
                MBTopWinWait();
                otherPlayer = MBWinLastChoiceGet();
                MBTopWinKill();
                if(otherPlayer == 3) {
                    MBScrollExec(capsulePlayer);
                    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                }
                if(otherPlayer == 3) {
                    goto repeat2story;
                }
                if(otherPlayer >= 2) {
                    if(!capsuleFlags.kettouF) {
                        otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                    } else {
                        otherPlayer = MBCapsulePlayerSameRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN, TRUE);
                    }
                    if(otherPlayer < 0) {
                        otherPlayer = playerNo[0];
                    }
                } else {
                    otherPlayer = playerNo[otherPlayer];
                }
                break;
            
            case 1:
                otherPlayer = playerNo[0];
                break;
            
            default:
                MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                MBAudFXPlay(MSM_SE_GUIDE_08);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_FAIL, HUWIN_SPEAKER_HEYHO);
                MBTopWinWait();
                MBTopWinKill();
                MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                while(MBMusCheck(MB_MUS_CHAN_FG)) {
                    HuPrcVSleep();
                }
                for(i=0; i<60.0f; i++) {
                    t = i/60.0f;
                    mdlPos.x = playerPos.x;
                    mdlPos.z = -100+playerPos.z;
                    mdlPos.y = playerPos.y+500+(-300*HuCos(t*90));
                    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                    HuPrcVSleep();
                }
                MBModelDispSet(mdlId, FALSE);
                MBMusMainPlay();
                goto end;
        }
        MBCapsuleStatusPosMoveWait(FALSE, TRUE);
        MBCapsuleStatusKettouInSet(capsulePlayer, otherPlayer, TRUE);
        if(GwSystem.last5Effect != MB_LAST5_EFF_TAKEDOUBLE) {
            coinNum = kettouCoinLose;
            prevCoin = kettouOppCoinLose;
        } else {
            coinNum = kettouCoinLose*2;
            prevCoin = kettouOppCoinLose*2;
        }
        if(MBPlayerStoryPlayerCheck(capsulePlayer)) {
            coinNum *= 1.5f;
        }
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_STORYEVENT_KETTOU_START, HUWIN_SPEAKER_HEYHO);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 0);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 1);
        sprintf(insertMes1, "%d", coinNum);
        sprintf(insertMes2, "%d", prevCoin);
        MBTopWinInsertMesSet(MESSNUM_PTR(insertMes1), 2);
        MBTopWinInsertMesSet(MESSNUM_PTR(insertMes2), 3);
        MBTopWinWait();
        MBTopWinKill();
        memset(&MgResultData, 0, sizeof(MgResultData));
        MgResultData.playerNo1 = capsulePlayer;
        MgResultData.playerNo2 = otherPlayer;
        MgResultData.coinNum = coinNum;
        MBCapsuleStatusKettouOutSet(capsulePlayer, otherPlayer, TRUE);
        GWMgCoinBonusSet(MgResultData.playerNo1, 0);
        GWMgCoinBonusSet(MgResultData.playerNo2, 0);
        if(GwPlayer[MgResultData.playerNo1].comF && GwPlayer[MgResultData.playerNo2].comF && !GWMgComFGet()) {
            int chanceTbl[3];
            int totalChance;
            int chance;
            int difDelta;
            difDelta = abs(GwPlayer[MgResultData.playerNo1].dif-GwPlayer[MgResultData.playerNo2].dif);
            if(difDelta < 0) {
                difDelta = 0;
            } else if(difDelta > 3) {
                difDelta = 3;
            }
            chanceTbl[0] = 15-(difDelta*4);
            chanceTbl[1] = (GwPlayer[MgResultData.playerNo1].dif*20)+50;
            chanceTbl[2] = (GwPlayer[MgResultData.playerNo2].dif*20)+50;
            totalChance = chanceTbl[0]+chanceTbl[1]+chanceTbl[2];
            chance = MBCapsuleEffRand(totalChance);
            if(chance < chanceTbl[0]) {
                GWMgCoinBonusSet(MgResultData.playerNo1, 0);
                GWMgCoinBonusSet(MgResultData.playerNo2, 0);
            } else if(chance < chanceTbl[1]) {
                GWMgCoinBonusSet(MgResultData.playerNo1, 10);
            } else {
                GWMgCoinBonusSet(MgResultData.playerNo2, 10);
            }
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            MBCapsuleCameraViewPlayerSet(capsulePlayer);
            MBCameraMotionWait();
            MBCapsuleStatusPosMoveWait(TRUE, FALSE);
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            goto storyEnd;
        } else {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grp = 2;
            }
            GwPlayerConf[capsulePlayer].grp = 0;
            GwPlayerConf[otherPlayer].grp = 1;
            MBMgCallKettouExec();
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            MBMusMainPlay();
            goto end;
        }
    } else if(capsuleFlags.circuitKettouF) {
        MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_BG)) {
            HuPrcVSleep();
        }
        seNo = MBAudFXPlay(MSM_SE_BOARD_79);
        MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_BATTLE, 127, 0);
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<=60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.z = -100+playerPos.z;
            mdlPos.y = playerPos.y+500+(-300*HuSin(t*90));
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
        MBAudFXPlay(MSM_SE_GUIDE_07);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU, HUWIN_SPEAKER_HEYHO);
        MBTopWinWait();
        MBTopWinKill();
        for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
            if(MBCircuitGoalCheck(i)) {
                continue;
            }
            if(MBPlayerCoinGet(i) <= 0) {
                continue;
            }
            if(i != capsulePlayer) {
                playerNo[playerNum] = i;
                playerNum++;
            }
        }
        switch(playerNum) {
            case 3:
            repeat3circuit:
                MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE3, HUWIN_SPEAKER_HEYHO, 0);
                if(GwPlayer[capsulePlayer].comF) {
                    otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                    if(otherPlayer == playerNo[0]) {
                        otherPlayer = 0;
                    } else if(otherPlayer == playerNo[1]) {
                        otherPlayer = 1;
                    } else if(otherPlayer == playerNo[2]) {
                        otherPlayer = 2;
                    } else {
                        otherPlayer = MBCapsuleEffRand(3);
                    }
                    MBCapsuleChoiceSet(otherPlayer);
                }
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[0]), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[1]), 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[2]), 2);
                MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
                MBTopWinWait();
                otherPlayer = MBWinLastChoiceGet();
                MBTopWinKill();
                if(otherPlayer == 4) {
                    MBScrollExec(capsulePlayer);
                    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                }
                if(otherPlayer == 4) {
                    goto repeat3circuit;
                }
                if(otherPlayer >= 3) {
                    otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                } else {
                    otherPlayer = playerNo[otherPlayer];
                }
                break;
            
            case 2:
                repeat2circuit:
                MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_KETTOU_CHOICE2, HUWIN_SPEAKER_HEYHO, 0);
                if(GwPlayer[capsulePlayer].comF) {
                    otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                    if(otherPlayer == playerNo[0]) {
                        otherPlayer = 0;
                    } else if(otherPlayer == playerNo[1]) {
                        otherPlayer = 1;
                    } else {
                        otherPlayer = MBCapsuleEffRand(2);
                    }
                    MBCapsuleChoiceSet(otherPlayer);
                }
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[0]), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[1]), 1);
                MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
                MBTopWinWait();
                otherPlayer = MBWinLastChoiceGet();
                MBTopWinKill();
                if(otherPlayer == 3) {
                    MBScrollExec(capsulePlayer);
                    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                }
                if(otherPlayer == 3) {
                    goto repeat2circuit;
                }
                if(otherPlayer >= 2) {
                    otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                } else {
                    otherPlayer = playerNo[otherPlayer];
                }
                break;
            
            case 1:
                otherPlayer = playerNo[0];
                break;
                
            default:
                MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                MBAudFXPlay(MSM_SE_GUIDE_08);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KETTOU_FAIL, HUWIN_SPEAKER_HEYHO);
                MBTopWinWait();
                MBTopWinKill();
                MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                while(MBMusCheck(MB_MUS_CHAN_FG)) {
                    HuPrcVSleep();
                }
                for(i=0; i<60.0f; i++) {
                    t = i/60.0f;
                    mdlPos.x = playerPos.x;
                    mdlPos.z = -100+playerPos.z;
                    mdlPos.y = playerPos.y+500+(-300*HuCos(t*90));
                    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                    HuPrcVSleep();
                }
                MBModelDispSet(mdlId, FALSE);
                MBMusMainPlay();
                goto end;
        }
        MBCapsuleStatusPosMoveWait(FALSE, TRUE);
        MBCapsuleStatusKettouInSet(capsulePlayer, otherPlayer, TRUE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KETTOU_START, HUWIN_SPEAKER_HEYHO);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 0);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 1);
        MBTopWinWait();
        MBTopWinKill();
        memset(&MgResultData, 0, sizeof(MgResultData));
        MgResultData.playerNo1 = capsulePlayer;
        MgResultData.playerNo2 = otherPlayer;
        GWMgCoinBonusSet(MgResultData.playerNo1, 0);
        GWMgCoinBonusSet(MgResultData.playerNo2, 0);
        if(GwPlayer[MgResultData.playerNo1].comF && GwPlayer[MgResultData.playerNo2].comF && !GWMgComFGet()) {
            int chanceTbl[3];
            int totalChance;
            int chance;
            int difDelta;
            difDelta = abs(GwPlayer[MgResultData.playerNo1].dif-GwPlayer[MgResultData.playerNo2].dif);
            if(difDelta < 0) {
                difDelta = 0;
            } else if(difDelta > 3) {
                difDelta = 3;
            }
            chanceTbl[0] = 15-(difDelta*4);
            chanceTbl[1] = (GwPlayer[MgResultData.playerNo1].dif*20)+50;
            chanceTbl[2] = (GwPlayer[MgResultData.playerNo2].dif*20)+50;
            totalChance = chanceTbl[0]+chanceTbl[1]+chanceTbl[2];
            chance = MBCapsuleEffRand(totalChance);
            if(chance < chanceTbl[0]) {
                GWMgCoinBonusSet(MgResultData.playerNo1, 0);
                GWMgCoinBonusSet(MgResultData.playerNo2, 0);
            } else if(chance < chanceTbl[1]) {
                GWMgCoinBonusSet(MgResultData.playerNo1, 10);
            } else {
                GWMgCoinBonusSet(MgResultData.playerNo2, 10);
            }
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            MBCapsuleCameraViewPlayerSet(capsulePlayer);
            MBCameraMotionWait();
            MBCapsuleStatusPosMoveWait(TRUE, FALSE);
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            goto circuitEnd;
        } else {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                GwPlayerConf[i].grp = 2;
            }
            GwPlayerConf[capsulePlayer].grp = 0;
            GwPlayerConf[otherPlayer].grp = 1;
            MBMgCallKettouExec();
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            MBMusMainPlay();
            goto end;
        }
    } else if(capsuleFlags.circuitKettouMgEndF) {
        circuitEnd:
        MBCapsuleStatusPosMoveWait(FALSE, TRUE);
        MBCapsuleStatusKettouInSet(MgResultData.playerNo1, MgResultData.playerNo2, TRUE);
        MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_RESULT_KETTOU, 127, 0);
        MBCircuitCarInit();
        mdlPos.x = playerPos.x;
        mdlPos.z = -100+playerPos.z;
        mdlPos.y = 200+playerPos.y;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelDispSet(mdlId, TRUE);
        HuPrcVSleep();
        MBCameraSkipSet(FALSE);
        MBCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_ZOOMIN);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        if(seNo == MSM_SENO_NONE) {
            seNo = MBAudFXPlay(MSM_SE_BOARD_79);
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
        WipeWait();
        if(GWMgCoinBonusGet(MgResultData.playerNo1) > 0 && GWMgCoinBonusGet(MgResultData.playerNo2) == 0) {
            otherPlayer = MgResultData.playerNo1;
        } else if(GWMgCoinBonusGet(MgResultData.playerNo2) > 0 && GWMgCoinBonusGet(MgResultData.playerNo1) == 0) {
            otherPlayer = MgResultData.playerNo2;
        } else {
            otherPlayer = -1;
        }
        if(otherPlayer == capsulePlayer) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KETTOU_RESULT, HUWIN_SPEAKER_HEYHO);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 0);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(MgResultData.playerNo2), 1);
            MBTopWinWait();
            MBTopWinKill();
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
            WipeWait();
            MBCameraSkipSet(FALSE);
            MBCameraViewNoSet(MgResultData.playerNo2, MB_CAMERA_VIEW_ZOOMOUT);
            MBCameraMotionWait();
            MBCameraSkipSet(TRUE);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
            WipeWait();
            circuitSaiResult = MBCircuitKettouSaiExec(MgResultData.playerNo2);
            MBSaiNumKill(MgResultData.playerNo2);
            GwPlayer[MgResultData.playerNo2].walkNum = -circuitSaiResult;
            MBCircuitCarMoveExec(MgResultData.playerNo2);
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
            WipeWait();
            MBCameraSkipSet(FALSE);
            MBCameraViewNoSet(MgResultData.playerNo1, MB_CAMERA_VIEW_ZOOMIN);
            MBCameraMotionWait();
            MBCameraSkipSet(TRUE);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
            WipeWait();
        } else {
            if(otherPlayer != -1) {
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KETTOU_LOSER, HUWIN_SPEAKER_HEYHO);
            } else {
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KETTOU_TIE, HUWIN_SPEAKER_HEYHO);
            }
            MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 0);
            MBTopWinWait();
            MBTopWinKill();
        }
        MBAudFXPlay(MSM_SE_GUIDE_07);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_END, HUWIN_SPEAKER_HEYHO);
        MBTopWinWait();
        MBTopWinKill();
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<=60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.z = -100+playerPos.z;
            mdlPos.y = playerPos.y+500+(-300*HuCos(t*90));
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        MBCapsuleStatusKettouOutSet(MgResultData.playerNo1, MgResultData.playerNo2, TRUE);
        MBCapsuleStatusPosMoveWait(TRUE, TRUE);
        memset(&MgResultData, 0, sizeof(MgResultData));
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBMusMainPlay();
        goto end;
    } else if(GWPartyFGet() == FALSE) {
        storyEnd:
        MBCapsuleStatusPosMoveWait(FALSE, TRUE);
        MBCapsuleStatusKettouInSet(MgResultData.playerNo1, MgResultData.playerNo2, TRUE);
        MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_RESULT_KETTOU, 127, 0);
        MBPlayerPosRestoreAll();
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &playerPos);
        mdlPos.x = playerPos.x;
        mdlPos.z = -100+playerPos.z;
        mdlPos.y = 200+playerPos.y;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelDispSet(mdlId, TRUE);
        HuPrcVSleep();
        MBCameraSkipSet(FALSE);
        MBCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_ZOOMIN);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        if(seNo == MSM_SENO_NONE) {
            seNo = MBAudFXPlay(MSM_SE_BOARD_79);
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
        WipeWait();
        if(GWMgCoinBonusGet(MgResultData.playerNo1) > 0 && GWMgCoinBonusGet(MgResultData.playerNo2) == 0) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_STORYEVENT_KETTOU_RESULT_WIN, HUWIN_SPEAKER_HEYHO);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(MgResultData.playerNo1), 0);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(MgResultData.playerNo2), 1);
            MBTopWinWait();
            MBTopWinKill();
            MgResultData.coinNum = kettouCoinLose;
            if(GwSystem.last5Effect == MB_LAST5_EFF_TAKEDOUBLE) {
                MgResultData.coinNum *= 2;
            }
            if(MBPlayerStoryPlayerCheck(MgResultData.playerNo1)) {
                MgResultData.coinNum *= 1.5f;
            }
            if(MBPlayerCoinGet(MgResultData.playerNo2) < MgResultData.coinNum) {
                MgResultData.coinNum = MBPlayerCoinGet(MgResultData.playerNo2);
            }
            if(MgResultData.coinNum > 0) {
                MBCoinAddExec(MgResultData.playerNo2, -MgResultData.coinNum);
                MBCapsuleCoinDispExec(MgResultData.playerNo2, -MgResultData.coinNum, TRUE, TRUE);
            }
        } else if(GWMgCoinBonusGet(MgResultData.playerNo2) > 0 && GWMgCoinBonusGet(MgResultData.playerNo1) == 0) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_STORYEVENT_KETTOU_RESULT_LOSE, HUWIN_SPEAKER_HEYHO);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(MgResultData.playerNo1), 0);
            MBTopWinWait();
            MBTopWinKill();
            MgResultData.coinNum = kettouOppCoinLose;
            if(GwSystem.last5Effect == MB_LAST5_EFF_TAKEDOUBLE) {
                MgResultData.coinNum *= 2;
            }
            if(MBPlayerCoinGet(MgResultData.playerNo1) < MgResultData.coinNum) {
                MgResultData.coinNum = MBPlayerCoinGet(MgResultData.playerNo1);
            }
            if(MgResultData.coinNum > 0) {
                MBCoinAddExec(MgResultData.playerNo1, -MgResultData.coinNum);
                MBCapsuleCoinDispExec(MgResultData.playerNo1, -MgResultData.coinNum, TRUE, TRUE);
            }
        } else {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_STORYEVENT_KETTOU_RESULT_TIE, HUWIN_SPEAKER_HEYHO);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(MgResultData.playerNo2), 0);
            MBTopWinWait();
            MBTopWinKill();
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_07);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_END, HUWIN_SPEAKER_HEYHO);
        MBTopWinWait();
        MBTopWinKill();
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<=60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.z = -100+playerPos.z;
            mdlPos.y = playerPos.y+500+(-300*HuCos(t*90));
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        MBMusMainPlay();
        MBCapsuleStatusKettouOutSet(MgResultData.playerNo1, MgResultData.playerNo2, TRUE);
        if(GWPartyFGet() != FALSE || !MBPlayerStoryComCheck(capsulePlayer) || GwSystem.playerMode != MB_PLAYER_MODE_WALK) {
            MBCapsuleStatusPosMoveWait(TRUE, TRUE);
        }
        memset(&MgResultData, 0, sizeof(MgResultData));
    } else {
        partyEnd:
        MBCapsuleStatusPosMoveWait(FALSE, TRUE);
        MBCapsuleStatusKettouInSet(MgResultData.playerNo1, MgResultData.playerNo2, TRUE);
        MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_RESULT_KETTOU, 127, 0);
        MBPlayerPosRestoreAll();
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &playerPos);
        mdlPos.x = playerPos.x;
        mdlPos.z = -100+playerPos.z;
        mdlPos.y = 200+playerPos.y;
        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
        MBModelDispSet(mdlId, TRUE);
        HuPrcVSleep();
        MBCameraSkipSet(FALSE);
        MBCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_ZOOMIN);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        if(seNo == MSM_SENO_NONE) {
            seNo = MBAudFXPlay(MSM_SE_BOARD_79);
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
        WipeWait();
        if(GWMgCoinBonusGet(MgResultData.playerNo1) > 0 && GWMgCoinBonusGet(MgResultData.playerNo2) == 0) {
            otherPlayer = MgResultData.playerNo1;
        } else if(GWMgCoinBonusGet(MgResultData.playerNo2) > 0 && GWMgCoinBonusGet(MgResultData.playerNo1) == 0) {
            otherPlayer = MgResultData.playerNo2;
        } else {
            otherPlayer = -1;
        }
        if(otherPlayer != -1) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            if(MgResultData.playerNo1 == otherPlayer) {
                circuitPlayer = MgResultData.playerNo2;
            } else {
                circuitPlayer = MgResultData.playerNo1;
            }
            
            if(MgResultData.coinNum > 0 && MgResultData.starNum > 0) {
                MBAudFXPlay(MSM_SE_GUIDE_07);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_RESULT_COINSTAR, HUWIN_SPEAKER_HEYHO);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 0);
            } else if(MgResultData.starNum > 0) {
                MBAudFXPlay(MSM_SE_GUIDE_07);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_RESULT_STAR, HUWIN_SPEAKER_HEYHO);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 0);
            } else {
                MBAudFXPlay(MSM_SE_GUIDE_07);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_RESULT_COIN, HUWIN_SPEAKER_HEYHO);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 0);
                sprintf(insertMes1, "%d", MgResultData.coinNum);
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes1), 1);
            }
            MBTopWinWait();
            MBTopWinKill();
            if(MgResultData.coinNum) {
                MBPlayerPosGet(otherPlayer, &coinPos);
                coinPos.y +=  250;
                coinDispId[0] = MBCoinDispCreateSe(&coinPos, MgResultData.coinNum);
                MBCoinAddExec(otherPlayer, MgResultData.coinNum);
                MBPlayerWinLoseVoicePlay(otherPlayer, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
                MBPlayerMotionNoShiftSet(otherPlayer, MB_PLAYER_MOT_COINWIN, 0, 8, HU3D_MOTATTR_NONE);
                MBPlayerWinLoseVoicePlay(circuitPlayer, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
                MBPlayerMotionNoShiftSet(circuitPlayer, MB_PLAYER_MOT_COINLOSE, 0, 8, HU3D_MOTATTR_NONE);
                do {
                    HuPrcVSleep();
                } while(MBMotionShiftIDGet(MBPlayerModelGet(MgResultData.playerNo1)) != HU3D_MOTID_NONE ||
                    !MBPlayerMotionEndCheck(MgResultData.playerNo1) ||
                    MBMotionShiftIDGet(MBPlayerModelGet(MgResultData.playerNo2)) != HU3D_MOTID_NONE ||
                    !MBPlayerMotionEndCheck(MgResultData.playerNo2));
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(MBPlayerAliveCheck(i)) {
                        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
                    }
                }
                do {
                    HuPrcVSleep();
                } while (!MBCoinDispKillCheck(coinDispId[0]));
            }
            if(MgResultData.starNum) {
                MBPlayerStarAdd(otherPlayer, MgResultData.starNum);
                starNumDispId[0] = MBCapsuleStarNumCreate(otherPlayer, MgResultData.starNum);
                MBPlayerWinLoseVoicePlay(otherPlayer, MB_PLAYER_MOT_WIN, CHARVOICEID(0));
                MBPlayerMotionNoShiftSet(otherPlayer, MB_PLAYER_MOT_WIN, 0, 8, HU3D_MOTATTR_NONE);
                motId = MBCapsulePlayerMotionCreate(circuitPlayer, CHARMOT_HSF_c000m1_360);
                MBPlayerWinLoseVoicePlay(circuitPlayer, motId, CHARVOICEID(10));
                MBPlayerMotionNoShiftSet(circuitPlayer, motId, 0, 8, HU3D_MOTATTR_NONE);
                do {
                    HuPrcVSleep();
                } while(MBMotionShiftIDGet(MBPlayerModelGet(MgResultData.playerNo1)) != HU3D_MOTID_NONE ||
                    !MBPlayerMotionEndCheck(MgResultData.playerNo1) ||
                    MBMotionShiftIDGet(MBPlayerModelGet(MgResultData.playerNo2)) != HU3D_MOTID_NONE ||
                    !MBPlayerMotionEndCheck(MgResultData.playerNo2) ||
                    !MBCapsuleStarNumCheck(starNumDispId[0]));
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(MBPlayerAliveCheck(i)) {
                        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
                    }
                }
            }
        } else {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            if(MgResultData.coinNum > 0 && MgResultData.starNum > 0) {
                MBAudFXPlay(MSM_SE_GUIDE_07);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_TIE_COINSTAR, HUWIN_SPEAKER_HEYHO);
            } else if(MgResultData.starNum > 0) {
                MBAudFXPlay(MSM_SE_GUIDE_07);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_TIE_STAR, HUWIN_SPEAKER_HEYHO);
            } else {
                MBAudFXPlay(MSM_SE_GUIDE_07);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_TIE_COIN, HUWIN_SPEAKER_HEYHO);
            }
            MBTopWinWait();
            MBTopWinKill();
            if(MgResultData.coinNum > 0 && MgResultData.starNum > 0) {
                MBCoinAddExec(MgResultData.playerNo1, MgResultData.coinNum);
                MBPlayerStarAdd(MgResultData.playerNo2, MgResultData.starNum);
            } else if(MgResultData.starNum > 0) {
                MBPlayerStarAdd(MgResultData.playerNo1, MgResultData.starNum/2);
                MBPlayerStarAdd(MgResultData.playerNo2, MgResultData.starNum/2);
            } else {
                MBCoinAddExec(MgResultData.playerNo1, MgResultData.coinNum/2);
                MBCoinAddExec(MgResultData.playerNo2, MgResultData.coinNum/2);
            }
            MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINLOSE, HU3D_MOTATTR_NONE, TRUE);
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
        }
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        MBAudFXPlay(MSM_SE_GUIDE_07);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KETTOU_END, HUWIN_SPEAKER_HEYHO);
        MBTopWinWait();
        MBTopWinKill();
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, TRUE);
        for(i=0; i<=60.0f; i++) {
            t = i/60.0f;
            mdlPos.x = playerPos.x;
            mdlPos.z = -100+playerPos.z;
            mdlPos.y = playerPos.y+500+(-300*HuCos(t*90));
            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        MBMusMainPlay();
        MBCapsuleStatusKettouOutSet(MgResultData.playerNo1, MgResultData.playerNo2, TRUE);
        MBCapsuleStatusPosMoveWait(TRUE, TRUE);
        memset(&MgResultData, 0, sizeof(MgResultData));
    }
    end:
    if(seNo != MSM_SENO_NONE) {
        MBAudFXStop(seNo);
    }
    HuPrcEnd();
}

void MBCapsuleKettouDestroy(void)
{
    
}

static void WanwanPlayerUpdate(OMOBJ *obj);

void MBCapsuleWanwanExec(void)
{
    int i; //r31
    int mdlId; //r30
    int otherPlayer; //r29
    int j; //r28
    int coinNum; //r27
    int masuNum; //r26
    int masuId; //r25
    int moveTime; //r24
    int seNo; //r22
    int playerNum; //r21
    int stealType; //r20
    int masuIdOther; //r19
    MBMODELID focusMdlId; //r18
    OMOBJ *obj; //r17
    
    float t; //f31
    float rotY; //f30
    float scale; //f29
    float moveDist; //f28
    float rotEndX; //f27
    float rotEndY; //f26
    float rotYSpeed; //f25
    float effRotX; //f24
    float effRotY; //f23
    
    Mtx hookMtx; //sp+0x1B8
    int masuIdTbl[MASU_LINK_MAX]; //sp+0x1A4
    int playerAttackMot[5]; //sp+0x190
    s16 linkTbl[MASU_LINK_MAX*2]; //sp+0x17C
    int motFile[8]; //sp+0x15C
    char insertMes[32]; //sp+0x13C
    HuVecF playerPos; //sp+0x130
    HuVecF mdlPos; //sp+0x124
    HuVecF playerPosOther; //sp+0x118
    HuVecF masuPosOther; //sp+0x10C
    HuVecF mdlVel; //sp+0x100
    HuVecF mdlRot; //sp+0xF4
    HuVecF masuPos; //sp+0xE8
    HuVecF masuNextPos; //sp+0xDC
    HuVecF moveDir; //sp+0xD0
    HuVecF playerPosOld; //sp+0xC4
    int playerNo[GW_PLAYER_MAX]; //sp+0xB4
    OMOBJ *honeObj; //sp+0x68
    int linkNum; //sp+0x64
    int playerMasuId; //sp+0x60
    int btnNum; //sp+0x5C
    int starNum; //sp+0x58 
    int starId; //sp+0x54
    int rideMotId; //sp+0x50
    int recoverMotId; //sp+0x4C
    BOOL honeEnterF; //sp+0x48
    int sp44; //sp+0x44
    int seNo2; //sp+0x40
    int seNo3; //sp+0x3C
    int seNo4; //sp+0x38
    BOOL attackF; //sp+0x34
    BOOL stealCancelF; //sp+0x30
    BOOL coinChoiceF; //sp+0x2C
    BOOL starChoiceF; //sp+0x28
    s16 honeSlot; //sp+0xA
    MBMODELID honeMdlId; //sp+0x8
    static float heightTbl[CHARNO_MAX] = {
        94,
        94,
        94,
        94,
        94,
        94,
        94,
        70,
        133,
        94,
        94,
        94,
        94
    };
    sp44 = -1;
    attackF = FALSE;
    stealCancelF = FALSE;
    MBCapsuleExplodeEffCreate(0);
    MBCapsuleCoinEffCreate();
    rideMotId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_435);
    recoverMotId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_324);
    MBPlayerMoveInit();
    MBPlayerMoveObjCreate();
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
    playerMasuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerPosOld = playerPos;
    motFile[0] = CAPSULECHAR0_HSF_wanwan_wait;
    motFile[1] = CAPSULECHAR0_HSF_wanwan_wait_fast;
    motFile[2] = CAPSULECHAR0_HSF_wanwan_breath;
    motFile[3] = CAPSULECHAR0_HSF_wanwan_lick;
    motFile[4] = CAPSULECHAR0_HSF_wanwan_speak;
    motFile[5] = CAPSULECHAR0_HSF_wanwan_tailmove;
    motFile[6] = HU_DATANUM_NONE;
    
    mdlId = MBCapsuleModelCreate(CAPSULECHAR0_HSF_wanwan, motFile, FALSE, 10, FALSE);
    mdlPos = playerPos;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBModelDispSet(mdlId, FALSE);
    focusMdlId = MBCapsuleModelCreate(CAPSULE_HSF_kinoko, NULL, FALSE, 10, FALSE);
    MBModelDispSet(focusMdlId, FALSE);
    MBModelPosSetV(focusMdlId, &playerPosOld);
    MBCameraModelViewSet(focusMdlId, NULL, &capsuleViewOfs, -1, -1, 21);
    MBCameraMotionWait();
    MBCameraQuakeSet(60, 50);
    HuPrcSleep(60);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(playerMasuId == GwPlayer[i].masuId && i != capsulePlayer) {
            MBPlayerDispSet(i, FALSE);
        }
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 60);
    WipeWait();
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_WANWAN, 127, 0);
    seNo = HuAudFXPlay(MSM_SE_BOARD_81);
    obj = capsuleObj = MBAddObj(32768, 0, 0, WanwanPlayerUpdate);
    obj->work[0] = capsulePlayer;
    obj->work[1] = mdlId;
    obj->work[2] = TRUE;
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 4, HU3D_MOTATTR_NONE);
    for(i=0, rotY=0; i<120.0f; i++) {
        t = i/120.0f;
        scale = t;
        rotYSpeed = (HuSin(t*180)*10)+20;
        rotY += rotYSpeed;
        if(rotY > 360) {
            rotY -= 360;
        }
        MBModelRotSet(mdlId, 0, rotY, 0);
        MBModelScaleSet(mdlId, scale, scale, scale);
        masuPos = playerPosOld;
        masuPos.y += 2*(100*t);
        MBModelPosSetV(focusMdlId, &masuPos);
        if(t >= 0.2f) {
            if(MBPlayerMotionNoGet(capsulePlayer) != rideMotId && MBMotionShiftIDGet(MBPlayerModelGet(capsulePlayer)) == HU3D_MOTID_NONE) {
                MBPlayerMotionNoShiftSet(capsulePlayer, rideMotId, 0, 8, HU3D_MOTATTR_LOOP);
            }
        }
        MBCapsuleExplodeEffCircleAdd(0, mdlPos, 75, t, MBCapsuleEffRandF()*360);
        HuPrcVSleep();
    }
    do {
        if(rotY > 360) {
            rotY -= 360;
        }
        rotY += rotYSpeed;
        if(rotYSpeed > 5) {
            rotYSpeed -= 0.5f;
        }
        MBModelRotSet(mdlId, 0, rotY, 0);
        MBCapsuleExplodeEffCircleAdd(0, mdlPos, 75, 1, MBCapsuleEffRandF()*360);
        HuPrcVSleep();
    } while(rotYSpeed > 5 || rotY < 360);
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBModelRotSet(mdlId, 0, 0, 0);
    MBModelScaleSet(mdlId, 1, 1, 1);
    if(seNo != MSM_SENO_NONE) {
        HuAudFXStop(seNo);
    }
    seNo = MSM_SENO_NONE;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(i == capsulePlayer) {
            continue;
        }
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<60.0f; i++) {
        Hu3DModelObjMtxGet(MBModelIdGet(mdlId), "itemhook_C", hookMtx);
        playerPos.x = hookMtx[0][3];
        playerPos.y = hookMtx[1][3];
        playerPos.z = hookMtx[2][3];
        MBPlayerPosSetV(capsulePlayer, &playerPos);
        HuPrcVSleep();
    }
    for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
        if(i == capsulePlayer) {
            continue;
        }
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        playerNo[playerNum] = i;
        playerNum++;
    }
    MBAudFXPlay(MSM_SE_GUIDE_27);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_START, HUWIN_SPEAKER_WAN2);
    MBTopWinWait();
    MBTopWinKill();
    coinChoiceF = TRUE;
    starChoiceF = TRUE;
    for(i=0; i<playerNum; i++) {
        if(!GWTeamFGet() || !MBCapsuleTeamCheck(capsulePlayer, playerNo[i])) {
            if(MBPlayerCoinGet(playerNo[i]) > 0) {
                break;
            }
        }
    }
    if(i >= playerNum) {
        coinChoiceF = FALSE;
    }
    for(i=0; i<playerNum; i++) {
        if(!GWTeamFGet() || !MBCapsuleTeamCheck(capsulePlayer, playerNo[i])) {
            if(MBPlayerStarGet(playerNo[i]) > 0) {
                break;
            }
        }
    }
    if(i >= playerNum || MBPlayerCoinGet(capsulePlayer) < 30) {
        starChoiceF = FALSE;
    }
    if(!coinChoiceF && !starChoiceF) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_FAIL, HUWIN_SPEAKER_WAN2);
        MBTopWinWait();
        MBTopWinKill();
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    } else {
        repeatChoice:
        MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_WANWAN_CHOICE, HUWIN_SPEAKER_WAN2, 0);
        MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
        if(!coinChoiceF) {
            MBTopWinChoiceDisable(0);
        }
        if(!starChoiceF) {
            MBTopWinChoiceDisable(1);
        }
        if(GwPlayer[capsulePlayer].comF) {
            if(coinChoiceF && starChoiceF) {
                MBCapsuleChoiceSet(1);
            } else {
                MBCapsuleChoiceSet(0);
            }
        }
        MBTopWinWait();
        stealType = MBWinLastChoiceGet();
        MBTopWinKill();
        switch(stealType) {
            case 2:
                MBAudFXPlay(MSM_SE_GUIDE_27);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_CHEAP, HUWIN_SPEAKER_WAN2);
                MBTopWinWait();
                MBTopWinKill();
                MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                stealCancelF = TRUE;
                break;
            
            case 3:
                MBScrollExec(capsulePlayer);
                MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                
            case 0:
            case 1:
            default:
                if(stealType == 3) {
                    goto repeatChoice;
                }
                MBWinCreateChoice(MBWIN_TYPE_TALKCHOICE, MESS_CAPSULE_EX04_WANWAN_CHOICE_PLAYER, HUWIN_SPEAKER_WAN2, 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[0]), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[1]), 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo[2]), 2);
                MBTopWinAttrSet(HUWIN_ATTR_NOCANCEL);
                switch(stealType) {
                    case 0:
                        for(i=0; i<playerNum; i++) {
                            if(MBPlayerCoinGet(playerNo[i]) <= 0 || !MBPlayerAliveCheck(playerNo[i])) {
                                MBTopWinChoiceDisable(i);
                            }
                        }
                        break;
                    
                    case 1:
                        for(i=0; i<playerNum; i++) {
                            if(MBPlayerStarGet(playerNo[i]) <= 0 || !MBPlayerAliveCheck(playerNo[i])) {
                                MBTopWinChoiceDisable(i);
                            }
                        }
                        break;
                    
                    default:
                        break;
                }
                for(i=0; i<playerNum; i++) {
                    if(GWTeamFGet() && MBCapsuleTeamCheck(capsulePlayer, playerNo[i])) {
                        MBTopWinChoiceDisable(i);
                    }
                }
                if(GwPlayer[capsulePlayer].comF) {
                    if(GWPartyFGet() == TRUE) {
                        MBCapsuleChoiceSet(MBCapsuleEffRand(3));
                    } else {
                        MBCapsuleChoiceSet(0);
                    }
                }
                MBTopWinWait();
                otherPlayer = MBWinLastChoiceGet();
                MBTopWinKill();
                if(otherPlayer >= 3) {
                    if(stealType == 0) {
                        otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_COIN);
                    } else {
                        otherPlayer = MBCapsulePlayerRandGet(capsulePlayer, CAPSULE_PLAYERRAND_STAR);
                    }
                } else {
                    otherPlayer = playerNo[otherPlayer];
                }
                masuIdOther = GwPlayer[otherPlayer].masuId;
                MBPlayerPosGet(otherPlayer, &playerPosOther);
                MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdOther, &masuPosOther);
                if(stealType == 1) {
                    MBCoinAddExec(capsulePlayer, -30);
                }
                seNo = HuAudFXPlay(MSM_SE_BOARD_122);
                MBMotionShiftSet(mdlId, 4, 0, 8, HU3D_MOTATTR_LOOP);
                for(i=0, rotY=0; i<120.0f; i++) {
                    t = i/120.0f;
                    scale = 1-t;
                    rotYSpeed = (HuSin(t*180)*10)+20;
                    rotY += rotYSpeed;
                    if(rotY > 360) {
                        rotY -= 360;
                    }
                    MBModelRotSet(mdlId, 0, rotY, 0);
                    MBModelScaleSet(mdlId, scale, scale, scale);
                    
                    if(t >= 0.75f) {
                        if(MBPlayerMotionNoGet(capsulePlayer) != MB_PLAYER_MOT_IDLE && MBMotionShiftIDGet(MBPlayerModelGet(capsulePlayer)) == HU3D_MOTID_NONE) {
                            MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
                        }
                    }
                    masuPos = playerPosOld;
                    masuPos.y += 2*(100*(1-t));
                    MBModelPosSetV(focusMdlId, &masuPos);
                    MBCapsuleExplodeEffCircleAdd(0, mdlPos, 75, t, MBCapsuleEffRandF()*360);
                    HuPrcVSleep();
                }
                if(seNo != MSM_SENO_NONE) {
                    HuAudFXStop(seNo);
                }
                seNo = MSM_SENO_NONE;
                WipeColorSet(255, 255, 255);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
                WipeWait();
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(!MBPlayerAliveCheck(i)) {
                        continue;
                    }
                    if(i != otherPlayer) {
                        MBModelDispSet(MBPlayerModelGet(i), FALSE);
                    }
                }
                MBPlayerDispSet(otherPlayer, TRUE);
                MBStarDispSetAll(FALSE);
                MBMasuPlayerLightSet(capsulePlayer, FALSE);
                MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdOther, &masuPosOther);
                MBPlayerPosSetV(otherPlayer, &masuPosOther);
                MBCameraSkipSet(FALSE);
                MBCameraFocusPlayerSet(otherPlayer);
                MBCameraMotionWait();
                MBCameraSkipSet(TRUE);
                playerAttackMot[0] = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_325);
                playerAttackMot[1] = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_458);
                playerAttackMot[2] = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_368);
                playerAttackMot[3] = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_303);
                playerAttackMot[4] = MBCapsulePlayerMotionCreate(otherPlayer, CHARMOT_HSF_c000m1_322);
                for(masuId=masuIdOther, masuNum=0, i=0; i<5; i++) {
                    linkNum = MBMasuLinkTblGet(MASU_LAYER_DEFAULT, masuId, linkTbl);
                    for(j=0; j<linkNum; j++) {
                        if((MBMasuFlagGet(MASU_LAYER_DEFAULT, linkTbl[j]) & (MBBranchFlagGet() | (MASU_FLAG_BLOCKL|MASU_FLAG_BLOCKR))) == 0) {
                            masuIdTbl[masuNum] = masuId = linkTbl[j];
                            masuNum++;
                            break;
                        }
                    }
                }
                if(masuNum < 2) {
                    for(masuId=masuIdOther, masuNum=0, i=0; i<5; i++) {
                        if((masuIdTbl[i] = masuId = MBCapsuleMasuNextGet(masuId, NULL)) <= 0) {
                            break;
                        }
                        masuNum++;
                    }
                }
                if(masuNum >= 2) {
                    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[masuNum-1], &mdlPos);
                    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[masuNum-2], &masuPos);
                } else if(masuNum == 1) {
                    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[0], &mdlPos);
                    masuPos = masuPosOther;
                } else {
                    mdlPos = masuPosOther;
                    mdlPos.y += 300;
                    masuPos.x = mdlPos.x;
                    masuPos.y = mdlPos.y;
                    masuPos.z = mdlPos.z-100;
                }
                VECSubtract(&masuPos, &mdlPos, &masuPos);
                mdlRot.x = HuAtan(masuPos.y, HuMagXZVecF(&masuPos));
                mdlRot.y = HuAtan(masuPos.x, masuPos.z);
                mdlRot.z = 0;
                MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                MBModelScaleSet(mdlId, 1, 1, 1);
                MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_LOOP);
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
                WipeWait();
                MBPlayerMotionNoShiftSet(otherPlayer, playerAttackMot[0], 0, 8, HU3D_MOTATTR_LOOP);
                HuPrcSleep(60);
                MBCameraFocusSet(mdlId);
                honeSlot = MBPlayerCapsuleFind(otherPlayer, CAPSULE_HONE);
                if(masuNum >= 2) {
                    for(i=masuNum-1; i>=1; i--) {
                        MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[i], &masuPos);
                        MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[i-1], &masuNextPos);
                        VECSubtract(&masuNextPos, &masuPos, &moveDir);
                        moveDist = VECMag(&moveDir);
                        moveTime = moveDist/20;
                        rotEndX = 0;
                        rotEndY = HuAtan(moveDir.x, moveDir.z);
                        for(j=0; j<moveTime; j++) {
                            t = (float)j/(float)moveTime;
                            mdlRot.x = MBCapsuleAngleLerp(rotEndX, mdlRot.x, 5);
                            mdlRot.y = MBCapsuleAngleLerp(rotEndY, mdlRot.y, 5);
                            mdlRot.z = 0;
                            mdlPos.x = masuPos.x+(t*moveDir.x);
                            mdlPos.y = masuPos.y+(t*moveDir.y)+((moveDist*0.5f)*HuSin(t*180));
                            mdlPos.z = masuPos.z+(t*moveDir.z);
                            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                            MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                            HuPrcVSleep();
                        }
                        MBAudFXPlay(MSM_SE_BOARD_80);
                    }
                    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
                    for(i=0; i<30.0f; i++) {
                        MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[otherPlayer].masuId, &masuPosOther);
                        VECSubtract(&masuPosOther, &mdlPos, &moveDir);
                        mdlRot.x = MBCapsuleAngleLerp(0, mdlRot.x, 5);
                        mdlRot.y = MBCapsuleAngleLerp(HuAtan(moveDir.x, moveDir.z), mdlRot.y, 5);
                        MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                        HuPrcVSleep();
                    }
                } else if(masuNum == 1) {
                    MBMasuPosGet(MASU_LAYER_DEFAULT, masuIdTbl[0], &mdlPos);
                    VECSubtract(&masuPosOther, &mdlPos, &moveDir);
                    mdlRot.y = HuAtan(moveDir.x, moveDir.z);
                    mdlRot.x = mdlRot.z = 0;
                    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                    MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
                    for(i=0; i<30.0f; i++) {
                        HuPrcVSleep();
                    }
                } else {
                    mdlPos.x = masuPosOther.x;
                    mdlPos.y = masuPosOther.y+300;
                    mdlPos.z = masuPosOther.z;
                    mdlRot.x = 0;
                    mdlRot.y = 0;
                    mdlRot.z = 0;
                    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                    MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
                    for(i=0; i<30.0f; i++) {
                        HuPrcVSleep();
                    }
                }
                if(honeSlot == -1) {
                    if(stealType == 0) {
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_ATTACK, HUWIN_SPEAKER_NONE);
                        MBTopWinPlayerDisable(otherPlayer);
                        MBTopWinInsertMesSet(MBPlayerNameMesGet(otherPlayer), 0);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBWinCreateHelp(MESS_CAPSULE_EX04_WANWAN_ATTACK_HELP);
                        attackF = TRUE;
                    }
                    if(masuNum >= 1) {
                        masuPos = mdlPos;
                        masuNextPos = masuPosOther;
                        masuNextPos.y += heightTbl[GwPlayer[otherPlayer].charNo];
                        VECSubtract(&masuNextPos, &masuPos, &moveDir);
                        moveDist = VECMag(&moveDir);
                        moveTime = moveDist/25.0;
                        rotEndX = 0;
                        rotEndY = HuAtan(moveDir.x, moveDir.z);
                    } else {
                        masuPos = mdlPos;
                        masuNextPos = masuPosOther;
                        masuNextPos.y += heightTbl[GwPlayer[otherPlayer].charNo];
                        VECSubtract(&masuNextPos, &masuPos, &moveDir);
                        moveTime = 19;
                        rotEndX = 30;
                        rotEndY = 0;
                    }
                    MBMotionShiftSet(mdlId, 5, 0, 8, HU3D_MOTATTR_LOOP);
                    for(j=0; j<moveTime; j++) {
                        t = (float)j/(float)moveTime;
                        mdlRot.x = MBCapsuleAngleLerp(rotEndX, mdlRot.x, 5);
                        mdlRot.y = MBCapsuleAngleLerp(rotEndY, mdlRot.y, 5);
                        mdlRot.z = 0;
                        mdlPos.x = masuPos.x+(t*moveDir.x);
                        mdlPos.y = masuPos.y+(t*moveDir.y)+(200*HuSin(t*180));
                        mdlPos.z = masuPos.z+(t*moveDir.z);
                        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                        MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                        HuPrcVSleep();
                    }
                    mdlPos = masuNextPos;
                    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                    MBCameraFocusPlayerSet(otherPlayer);
                    if(stealType == 0) { 
                        MBPlayerMotionNoShiftSet(otherPlayer, playerAttackMot[1], 0, 8, HU3D_MOTATTR_LOOP);
                        for(i=0, btnNum=0; i<120.0f; i++) {
                            t = i/120.0f;
                            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y+(HuSin(t*720)*10), mdlPos.z);
                            MBModelRotSet(mdlId, mdlRot.x+(HuSin(t*720)*10), mdlRot.y, mdlRot.z);
                            if(HuPadBtnDown[GwPlayer[otherPlayer].padNo] & PAD_BUTTON_A) {
                                btnNum++;
                            }
                            HuPrcVSleep();
                        }
                        if(attackF) {
                            MBTopWinKill();
                        }
                    }
                    MBMotionShiftSet(mdlId, 6, 0, 8, HU3D_MOTATTR_LOOP);
                    mdlVel.x = 0;
                    mdlVel.z = 0;
                    mdlVel.y = 50;
                    do {
                        VECAdd(&mdlPos, &mdlVel, &mdlPos);
                        mdlVel.y -= 5.716666569312414;
                        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                        MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                        if(MBPlayerMotionNoGet(otherPlayer) != playerAttackMot[2] && mdlVel.y < 0) {
                            if(mdlPos.y < masuPosOther.y+heightTbl[GwPlayer[otherPlayer].charNo]) {
                                MBPlayerMotionNoSet(otherPlayer, playerAttackMot[2], HU3D_MOTATTR_NONE);
                            }
                        }
                        HuPrcVSleep();
                    } while(mdlPos.y > masuPosOther.y);
                    mdlPos = masuPosOther;
                    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                    MBAudFXPlay(MSM_SE_BOARD_80);
                    omVibrate(otherPlayer, 12, 4, 2);
                    for(i=0; i<32; i++) {
                        MBCapsuleExplodeEffCircleAdd(0, mdlPos, 100, 1, i*11);
                    }
                    MBPlayerMotionNoSet(otherPlayer, playerAttackMot[2], HU3D_MOTATTR_NONE);
                    MBPlayerMotionSpeedSet(otherPlayer, 0);
                    MBPlayerMotionTimeSet(otherPlayer, 51);
                    if(stealType == 0) {
                        if(GwSystem.turnNo <= 5) {
                            coinNum = 15;
                        } else if(GwSystem.turnNo <= 15) {
                            coinNum = 25;
                        } else if(GwSystem.turnNo <= 25) {
                            coinNum = 27;
                        } else if(GwSystem.turnNo <= 35) {
                            coinNum = 30;
                        } else if(GwSystem.turnNo <= 45) {
                            coinNum = 32;
                        } else {
                            coinNum = 35;
                        }
                        if(!GwPlayer[otherPlayer].comF) {
                            t = btnNum/32.0f;
                            if(t > 1) {
                                t = 1;
                            }
                        } else {
                            t = (0.2f*GwPlayer[otherPlayer].dif)+(MBCapsuleEffRandF()*0.4f);
                            if(t > 1) {
                                t = 1;
                            }
                        }
                        coinNum = (1-t)*coinNum;
                        if(coinNum < 3) {
                            coinNum = 3;
                        }
                        if(coinNum > MBPlayerCoinGet(otherPlayer)) {
                            coinNum = MBPlayerCoinGet(otherPlayer);
                        }
                        MBPlayerCoinAdd(otherPlayer, -coinNum);
                        seDelayMax = 24;
                        for(i=0, effRotX = MBCapsuleEffRandF()*360; i<coinNum; i++) {
                            effRotX += (360.0f/coinNum)*((MBCapsuleEffRandF()*0.2f)+0.8f);
                            masuPos.x = masuPosOther.x+(100*HuSin(effRotX));
                            masuPos.z = masuPosOther.z+(100*HuCos(effRotX));
                            masuPos.y = masuPosOther.y;
                            effRotY = (MBCapsuleEffRandF()*15)+70;
                            t = ((MBCapsuleEffRandF()*0.3f)+0.8f)*65;
                            mdlVel.x = HuSin(effRotX)*HuCos(effRotY)*t;
                            mdlVel.z = HuCos(effRotX)*HuCos(effRotY)*t;
                            mdlVel.y = HuSin(effRotY)*t;
                            MBCapsuleCoinEffAdd(&masuPos, &mdlVel, 0.75f, 4.9f, 30, 0);
                        }
                        MBCapsuleSeDelayPlay(MSM_SE_CMN_19, 30);
                    }
                    for(i=0; i<45.0f; i++) {
                        HuPrcVSleep();
                    }
                    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
                    for(i=0, masuId=masuIdOther; i<3; i++) {
                        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
                        masuId = MBCapsuleMasuNextGet(masuId, &masuNextPos);
                        if(masuId <= 0) {
                            break;
                        }
                        VECSubtract(&masuNextPos, &masuPos, &moveDir);
                        moveDist = VECMag(&moveDir);
                        moveTime = moveDist/20;
                        rotEndX = 0;
                        rotEndY = HuAtan(moveDir.x, moveDir.z);
                        for(j=0; j<moveTime; j++) {
                            t = (float)j/moveTime;
                            mdlRot.x = MBCapsuleAngleLerp(rotEndX, mdlRot.x, 5);
                            mdlRot.y = MBCapsuleAngleLerp(rotEndY, mdlRot.y, 5);
                            mdlRot.z = 0;
                            mdlPos.x = masuPos.x+(t*moveDir.x);
                            mdlPos.y = masuPos.y+(t*moveDir.y)+((moveDist*0.5f)*HuSin(t*180));
                            mdlPos.z = masuPos.z+(t*moveDir.z);
                            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                            MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                            HuPrcVSleep();
                        }
                        MBAudFXPlay(MSM_SE_BOARD_80);
                    }
                } else { 
                    if(honeSlot != -1) {
                        MBPlayerCapsuleRemove(otherPlayer, honeSlot);
                    }
                    MBCameraFocusPlayerSet(otherPlayer);
                    if(MBCapsuleMasuNextGet(masuIdOther, &masuNextPos) > 0) {
                        VECSubtract(&masuNextPos, &masuPosOther, &masuNextPos);
                        MBPlayerRotateStart(otherPlayer, HuAtan(masuNextPos.x, masuNextPos.z), 15);
                        while(!MBPlayerRotateCheck(otherPlayer)) {
                            HuPrcVSleep();
                        }
                    } else {
                        VECSubtract(&mdlPos, &masuPosOther, &masuNextPos);
                        MBPlayerRotateStart(otherPlayer, HuAtan(masuNextPos.x, masuNextPos.z), 15);
                        while(!MBPlayerRotateCheck(otherPlayer)) {
                            HuPrcVSleep();
                        }
                    }
                    honeMdlId = MBCapsuleModelCreate(CAPSULE_HSF_hone, NULL, FALSE, 10, FALSE);
                    MBModelDispSet(honeMdlId, FALSE);
                    MBPlayerMotionNoShiftSet(otherPlayer, playerAttackMot[3], 0, 8, HU3D_MOTATTR_NONE);
                    do {
                        HuPrcVSleep();
                    } while(!MBPlayerMotionEndCheck(otherPlayer));
                    honeEnterF = FALSE;
                    if(masuNum >= 1) {
                        if(MBCapsuleMasuNextGet(masuIdOther, NULL) == masuIdTbl[0]) {
                            honeEnterF = TRUE;
                        }
                    }
                    masuPos = masuPosOther;
                    masuPos.y += 120.00001f;
                    honeObj = MBCapsuleHoneObjCreate(&masuPos, honeMdlId, masuIdOther, 3, honeEnterF);
                    MBModelDispSet(honeMdlId, TRUE);
                    MBAudFXPlay(MSM_SE_BOARD_84);
                    MBPlayerMotIdleSet(otherPlayer);
                    do {
                        HuPrcVSleep();
                    } while(!MBCapsulePlayerMotShiftCheck(otherPlayer));
                    if(honeEnterF) {
                        HuPrcSleep(45);
                    }
                    masuPos = mdlPos;
                    masuId = MBCapsuleMasuNextGet(masuIdOther, &masuNextPos);
                    if(masuId <= 0) {
                        masuNextPos.x = masuPosOther.x;
                        masuNextPos.y = masuPosOther.y+500;
                        masuNextPos.z = masuPosOther.z;
                    }
                    VECSubtract(&masuNextPos, &masuPos, &moveDir);
                    moveDist = VECMag(&moveDir);
                    moveTime = moveDist/20;
                    rotEndX = 0;
                    rotEndY = HuAtan(moveDir.x, moveDir.z);
                    for(j=0; j<moveTime; j++) {
                        t = (float)j/moveTime;
                        mdlRot.x = MBCapsuleAngleLerp(rotEndX, mdlRot.x, 5);
                        mdlRot.y = MBCapsuleAngleLerp(rotEndY, mdlRot.y, 5);
                        mdlRot.z = 0;
                        mdlPos.x = masuPos.x+(t*moveDir.x);
                        mdlPos.y = masuPos.y+(t*moveDir.y)+((moveDist*0.5f)*HuSin(t*180));
                        mdlPos.z = masuPos.z+(t*moveDir.z);
                        MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                        MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                        HuPrcVSleep();
                    }
                    MBAudFXPlay(MSM_SE_BOARD_80);
                    for(i=0; i<2; i++) {
                        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
                        masuId = MBCapsuleMasuNextGet(masuId, &masuNextPos);
                        if(masuId <= 0) {
                            break;
                        }
                        VECSubtract(&masuNextPos, &masuPos, &moveDir);
                        moveDist = VECMag(&moveDir);
                        moveTime = moveDist/20;
                        rotEndX = 0;
                        rotEndY = HuAtan(moveDir.x, moveDir.z);
                        for(j=0; j<moveTime; j++) {
                            t = (float)j/moveTime;
                            mdlRot.x = MBCapsuleAngleLerp(rotEndX, mdlRot.x, 5);
                            mdlRot.y = MBCapsuleAngleLerp(rotEndY, mdlRot.y, 5);
                            mdlRot.z = 0;
                            mdlPos.x = masuPos.x+(t*moveDir.x);
                            mdlPos.y = masuPos.y+(t*moveDir.y)+((moveDist*0.5f)*HuSin(t*180));
                            mdlPos.z = masuPos.z+(t*moveDir.z);
                            MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                            MBModelRotSet(mdlId, mdlRot.x, mdlRot.y, mdlRot.z);
                            HuPrcVSleep();
                        }
                        MBAudFXPlay(MSM_SE_BOARD_80);
                    }
                }
                MBModelDispSet(mdlId, FALSE);
                MBCapsuleDustCloudAdd(0, mdlPos);
                MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
                if(honeSlot == -1) {
                    if(stealType == 1 && (starNum = MBPlayerStarGet(otherPlayer)) != 0) {
                        MBAudFXPlay(MSM_SE_BOARD_83);
                        starId = MBCapsuleStarCreate();
                        masuPos = masuPosOther;
                        MBCapsuleStarPosSet(starId, masuPos.x, masuPos.y, masuPos.z);
                        seNo2 = MBAudFXPlay(MSM_SE_BOARD_90);
                        seNo3 = MBAudFXPlay(MSM_SE_BOARD_91);
                        seNo4 = MBAudFXPlay(MSM_SE_BOARD_92);
                        for(i=0; i<60.0f; i++) {
                            t = i/60.0f;
                            scale = t;
                            masuPos.x = masuPosOther.x;
                            masuPos.z = masuPosOther.z;
                            masuPos.y = masuPosOther.y+(300*HuSin(t*90));
                            MBCapsuleStarPosSet(starId, masuPos.x, masuPos.y, masuPos.z);
                            MBCapsuleStarRotSet(starId, 0, 720*HuSin(t*90), 0);
                            MBCapsuleStarScaleSet(starId, scale, scale, scale);
                            HuPrcVSleep();
                        }
                        for(i=0; i<60.0f; i++) {
                            t = i/60.0f;
                            scale = t;
                            masuPos.y += (15.000000953674316*HuSin(t*90));
                            MBCapsuleStarPosSet(starId, masuPos.x, masuPos.y, masuPos.z);
                            MBCapsuleStarRotSet(starId, 0, 720*HuSin(t*90), 0);
                            MBCapsuleStarScaleSet(starId, 1-scale, 1+scale, 1-scale);
                            HuPrcVSleep();
                        }
                        if(seNo2 != MSM_SENO_NONE) {
                            MBAudFXStop(seNo2);
                        }
                        if(seNo3 != MSM_SENO_NONE) {
                            MBAudFXStop(seNo3);
                        }
                        if(seNo4 != MSM_SENO_NONE) {
                            MBAudFXStop(seNo4);
                        }
                        MBCapsuleStarKill(starId);
                        MBPlayerStarAdd(otherPlayer, -1);
                        starNum = 1;
                    }
                    MBPlayerMotionSpeedSet(otherPlayer, 1);
                    while(!MBPlayerMotionEndCheck(otherPlayer)) {
                        HuPrcVSleep();
                    }
                    MBPlayerMotionNoShiftSet(otherPlayer, playerAttackMot[4], 0, 8, HU3D_MOTATTR_LOOP);
                }
                for(i=0; i<120.0f; i++) {
                    HuPrcVSleep();
                }
                WipeColorSet(255, 255, 255);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
                WipeWait();
                MBStarDispSetAll(TRUE);
                MBStarMasuDispSet(GwPlayer[capsulePlayer].masuId, FALSE);
                MBMasuPlayerLightSet(capsulePlayer, TRUE);
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(!MBPlayerAliveCheck(i)) {
                        continue;
                    }
                    if(playerMasuId != GwPlayer[i].masuId) {
                        MBPlayerDispSet(i, TRUE);
                    } else {
                        MBPlayerDispSet(i, FALSE);
                    }
                }
                MBPlayerDispSet(capsulePlayer, TRUE);
                MBPlayerPosSetV(otherPlayer, &playerPosOther);
                MBPlayerRotSet(otherPlayer, 0, 0, 0);
                MBCapsulePlayerMotSet(otherPlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                mdlPos = playerPosOld;
                MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
                MBModelRotSet(mdlId, 0, 0, 0);
                MBModelScaleSet(mdlId, 1, 1, 1);
                MBModelDispSet(mdlId, TRUE);
                MBPlayerMotionNoShiftSet(capsulePlayer, rideMotId, 0, 8, HU3D_MOTATTR_NONE);
                MBCameraSkipSet(FALSE);
                masuPos = playerPosOld;
                masuPos.y += 200;
                MBModelPosSetV(focusMdlId, &masuPos);
                MBCameraModelViewSet(focusMdlId, NULL, &capsuleViewOfs, -1, -1, 21);
                MBCameraMotionWait();
                MBCameraSkipSet(TRUE);
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
                if(honeSlot == -1) {
                    if(stealType == 0) {
                        MBAudFXPlay(MSM_SE_GUIDE_27);
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_RESULT_COIN, HUWIN_SPEAKER_WAN2);
                        sprintf(insertMes, "%d", coinNum);
                        MBTopWinInsertMesSet(MESSNUM_PTR(insertMes), 0);
                        MBTopWinWait();
                        MBTopWinKill();
                        if(coinNum > 0){
                            MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
                            MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_COINWIN, 0, 8, HU3D_MOTATTR_NONE);
                            MBCoinAddDispExec(capsulePlayer, coinNum, TRUE);
                        }
                    } else if(starNum != 0){
                        MBAudFXPlay(MSM_SE_GUIDE_27);
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_RESULT_STAR, HUWIN_SPEAKER_WAN2);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                        stealCancelF = TRUE;
                        while(MBMusCheck(MB_MUS_CHAN_FG)) {
                            HuPrcVSleep();
                        }
                        MBCapsulePlayerMotSet(capsulePlayer, recoverMotId, HU3D_MOTATTR_NONE, TRUE);
                        MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                        MBPlayerPosGet(capsulePlayer, &masuPos);
                        masuPos.y +=  350;
                        MBStarGetExec(capsulePlayer, &masuPos);
                        stealCancelF = FALSE;
                        MBMusMainPlay();
                        MBCameraModelViewSet(focusMdlId, NULL, &capsuleViewOfs, 2100, -1, 21);
                        MBCameraMotionWait();
                    }
                    MBPlayerMotionNoShiftSet(capsulePlayer, rideMotId, 0, 8, HU3D_MOTATTR_NONE);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_END, HUWIN_SPEAKER_WAN2);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    MBAudFXPlay(MSM_SE_GUIDE_27);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_WANWAN_RESULT_NONE, HUWIN_SPEAKER_WAN2);
                    MBTopWinWait();
                    MBTopWinKill();
                    CharFXPlay(GwPlayer[capsulePlayer].charNo, CHARVOICEID(10));
                    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_COINLOSE, 0, 8, HU3D_MOTATTR_NONE);
                    HuPrcSleep(60);
                }
                break;
        }
    }
    if(!stealCancelF) {
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    }
    seNo = HuAudFXPlay(MSM_SE_BOARD_122);
    MBMotionNoSet(mdlId, 4, HU3D_MOTATTR_NONE);
    for(i=0, rotY=0; i<120.0f; i++) {
        t = i/120.0f;
        scale = 1-t;
        rotYSpeed = (HuSin(t*180)*10)+20;
        rotY += rotYSpeed;
        if(rotY > 360) {
            rotY -= 360;
        }
        MBModelRotSet(mdlId, 0, rotY, 0);
        MBModelScaleSet(mdlId, scale, scale, scale);
        
        if(t >= 0.75f) {
            if(MBPlayerMotionNoGet(capsulePlayer) != MB_PLAYER_MOT_IDLE && MBMotionShiftIDGet(MBPlayerModelGet(capsulePlayer)) == HU3D_MOTID_NONE) {
                MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
            }
        }
        masuPos = playerPosOld;
        masuPos.y += 2*(100*(1-t));
        MBModelPosSetV(focusMdlId, &masuPos);
        MBCapsuleExplodeEffCircleAdd(0, mdlPos, 75, 1-t, MBCapsuleEffRandF()*360);
        HuPrcVSleep();
    }
    MBPlayerPosSet(capsulePlayer, playerPos.x, playerPos.y, playerPos.z);
    if(seNo != MSM_SENO_NONE) {
        HuAudFXStop(seNo);
    }
    seNo = MSM_SENO_NONE;
    MBCameraMotionWait();
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    MBPlayerMotIdleSet(capsulePlayer);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            MBPlayerDispSet(i, TRUE);
        }
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 60);
    
    while(MBMusCheck(MB_MUS_CHAN_FG)){ 
        HuPrcVSleep();
    }
    MBMusMainPlay();
    WipeWait();
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    if(!MBKillCheck()) {
        obj->work[2] = 2;
    }
    MBCapsulePlayerIdleWait();
    MBPlayerMoveObjClose();
    playerMasuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosFixEvent(capsulePlayer, playerMasuId, FALSE);
    HuPrcEnd();
}

void MBCapsuleWanwanDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleCoinEffKill();
}

static void WanwanPlayerUpdate(OMOBJ *obj)
{
    Mtx hookMtx;
    HuVecF pos;
    switch(obj->work[2]) {
        case 1:
            Hu3DModelObjMtxGet(MBModelIdGet(obj->work[1]), "itemhook_C", hookMtx);
            pos.x = hookMtx[0][3];
            pos.y = hookMtx[1][3];
            pos.z = hookMtx[2][3];
            MBPlayerPosSetV(obj->work[0], &pos);
            break;
        
        case 0:
            break;
            
        default:
            omDelObjEx(mbObjMan, obj);
            return;
    }
    if(MBKillCheck() || capsuleObj == NULL) {
        omDelObjEx(mbObjMan, obj);
    }
}

void MBCapsuleHoneExec(void)
{
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_HONE, HUWIN_SPEAKER_NONE);
    MBTopWinWait();
    MBTopWinKill();
    HuPrcEnd();
}

void MBCapsuleHoneDestroy(void)
{
    
}

static void KoopaCapsuleGetExec(int mdlId, BOOL partyF);
static void KoopaLast5Exec(int mdlId, BOOL partyF);
static void KoopaMasuExec(int mdlId, BOOL partyF);
static void KoopaCircuitExec(int mdlId, BOOL partyF);
static void KoopaEndGameExec(int mdlId, BOOL partyF);
static void KoopaMgEndExec(int mdlId, BOOL partyF);
static void KoopaCircuitMgEndExec(int mdlId, BOOL partyF);
static u16 KoopaLoseSaiPadHook(int playerNo);
static void KoopaSaiMotHook(int playerNo);

void MBCapsuleKoopaExec(void)
{
    int motFile[32];
    int mdlId;
    BOOL partyF;
    MBCapsuleExplodeEffCreate(0);
    MBCapsuleGlowEffCreate();
    MBCapsuleGlowEffAnimSet(CAPSULE_ANM_effStar);
    MBPlayerMoveInit();
    MBPlayerMoveObjCreate();
    if(!capsuleFlags.last5KoopaF || GWPartyFGet() == FALSE) {
        motFile[0] = CAPSULECHAR4_HSF_koopa_idle;
        motFile[1] = CAPSULECHAR4_HSF_koopa_stomp;
        motFile[2] = CAPSULECHAR4_HSF_koopa_laugh;
        motFile[3] = CAPSULECHAR4_HSF_koopa_walk;
        motFile[4] = CAPSULECHAR4_HSF_koopa_jump_start;
        motFile[5] = CAPSULECHAR4_HSF_koopa_lose;
        motFile[6] = CAPSULECHAR4_HSF_koopa_side_hit;
        motFile[7] = CAPSULECHAR4_HSF_koopa_last5_jump;
        motFile[8] = CAPSULECHAR4_HSF_koopa_last5_lose;
        motFile[9] = CAPSULECHAR4_HSF_koopa_last5_slam;
        motFile[10] = CAPSULECHAR4_HSF_koopa_sai_hit;
        motFile[11] = HU_DATANUM_NONE;
        mdlId = MBCapsuleModelCreate(CAPSULECHAR4_HSF_koopa, motFile, FALSE, 10, FALSE);
        partyF = FALSE;
    } else {
        int statId;
        HuDataDirClose(capsuleData[capsuleCurNo].dirNum);
        statId = MBDataDirReadAsync(DATA_capsulechar2);
        if(statId != HU_DATA_STAT_NONE) {
            MBDataAsyncWait(statId);
        }
        motFile[0] = CAPSULECHAR2_HSF_koopa_idle;
        motFile[1] = CAPSULECHAR2_HSF_koopa_laugh;
        motFile[2] = CAPSULECHAR2_HSF_koopa_laugh;
        motFile[3] = CAPSULECHAR2_HSF_koopa_walk;
        motFile[4] = CAPSULECHAR2_HSF_koopa_jump;
        motFile[5] = HU_DATANUM_NONE;
        mdlId = MBCapsuleModelCreate(CAPSULECHAR2_HSF_koopa, motFile, FALSE, 10, FALSE);
        partyF = TRUE;
        HuDataDirClose(DATA_capsulechar2);
        
    }
    MBModelDispSet(mdlId, FALSE);
    if(capsuleFlags.capsuleKoopaGetF) {
        KoopaCapsuleGetExec(mdlId, partyF);
    } else if(capsuleFlags.last5KoopaF) {
        KoopaLast5Exec(mdlId, partyF);
    } else if(capsuleFlags.koopaF) {
        KoopaMasuExec(mdlId, partyF);
    } else if(capsuleFlags.circuitKoopaF) {
        KoopaCircuitExec(mdlId, partyF);
    } else if(capsuleFlags.gameEndF) {
        KoopaEndGameExec(mdlId, partyF);
    } else if(capsuleFlags.circuitKoopaMgEndF) {
        KoopaCircuitMgEndExec(mdlId, partyF);
    } else {
        KoopaMgEndExec(mdlId, partyF);
    }
    MBCapsuleEffHookCall(1, MB_MODEL_NONE, FALSE, FALSE, FALSE);
    MBCapsulePlayerIdleWait();
    MBPlayerMoveObjClose();
    HuPrcEnd();
}

void MBCapsuleKoopaDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
    MBCapsuleGlowEffKill();
}

static void KoopaLast5SaiHit(int result)
{
    saiHookTime = 0;
    MBAudFXPlay(MSM_SE_BOARD_14);
}

static void KoopaCapsuleGetExec(int mdlId, BOOL partyF)
{
    int i; //r31
    int masuId; //r25
    int capsuleMdlId; //r22
    int *masuRedTbl; //r21
    int masuRedNum; //r20
    int stunNum; //r17
    
    float scale; //f31
    float t; //f30
    
    HuVecF playerPos; //sp+0x98
    HuVecF masuPos; //sp+0x8C
    HuVecF mdlPos; //sp+0x80
    HuVecF capsulePos; //sp+0x74
    int squishTbl[GW_PLAYER_MAX]; //sp+0x64
    int playerMasuId; //sp+0x30
    int motId; //sp+0x2C
    
    playerMasuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    HuAudFXPlay(MSM_SE_BOARD_85);
    capsuleMdlId = MBCapsuleModelCreate(CAPSULE_HSF_selKoopa, NULL, FALSE, 10, FALSE);
    MBModelLayerSet(capsuleMdlId, 2);
    capsulePos.x = playerPos.x;
    capsulePos.y = playerPos.y+300;
    capsulePos.z = playerPos.z;
    MBModelPosSetV(capsuleMdlId, &capsulePos);
    MBCapsuleDustCloudAdd(0, capsulePos);
    for(i=0; i<18.0f; i++) {
        t = i/18.0f;
        scale = t+(HuSin(t*180)*0.5);
        MBModelScaleSet(capsuleMdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    if(GWPartyFGet() == FALSE && MBPlayerStoryComCheck(capsulePlayer)) {
        MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
        MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINWIN, HU3D_MOTATTR_NONE, TRUE);
    } else {
        MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
        MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINLOSE, HU3D_MOTATTR_NONE, TRUE);
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KOOPA, HUWIN_SPEAKER_CAPMACHINE);
    MBTopWinWait();
    MBTopWinKill();
    MBAudFXPlay(MSM_SE_BOARD_103);
    for(i=0; i<18.0f; i++) {
        t = i/18.0f;
        scale = HuCos(t*90);
        MBModelScaleSet(capsuleMdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    MBModelDispSet(capsuleMdlId, FALSE);
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
    motId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_325);
    MBPlayerMotionNoShiftSet(capsulePlayer, motId, 0, 8, HU3D_MOTATTR_NONE);
    HuPrcSleep(60);
    MBCameraModelViewSet(MBPlayerModelGet(capsulePlayer), NULL, &capsuleViewOfs, 1500, -1, 21);
    MBCameraMotionWait();
    mdlPos = playerPos;
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_NONE);
    MBMotionTimeSet(mdlId, MBMotionMaxTimeGet(mdlId)*0.5f);
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_KOOPA, 127, 0);
    stunNum = 0;
    do {
        if(stunNum == 0 && MBMotionTimeGet(mdlId) > 137) {
            MBCapsuleDustHeavyAdd(0, mdlPos);
            MBAudFXPlay(MSM_SE_BOARD_115);
            MBCapsuleVibrate(1);
            stunNum = MBCapsulePlayerSquishSet(squishTbl, playerMasuId);
        }
        HuPrcVSleep();
    } while(!MBMotionEndCheck(mdlId));
    MBAudFXPlay(MSM_SE_GUIDE_12);
    MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KOOPA_MASU_START, HUWIN_SPEAKER_KOOPA);
    MBTopWinWait();
    MBTopWinKill();
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KOOPA_MASU_LOC, HUWIN_SPEAKER_KOOPA);
    MBTopWinWait();
    MBTopWinKill();
    HuAudFXPlay(MSM_SE_BOARD_59);
    MBMotionShiftSet(mdlId, 5, 0, 8, HU3D_MOTATTR_NONE);
    do {
        HuPrcVSleep();
    }  while(MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
    HuAudFXPlay(MSM_SE_GUIDE_14);
    for(i=0; i<90.0f; i++) {
        HuPrcVSleep();
    }
    masuRedTbl = MBCapsuleMalloc(MASU_MAX*sizeof(int));
    memset(masuRedTbl, 0,MASU_MAX*sizeof(int));
    for(masuRedNum=0, i=1; i<MASU_MAX; i++) {
        if(MBMasuTypeGet(MASU_LAYER_DEFAULT, i) == MASU_TYPE_RED) {
            masuRedTbl[masuRedNum] = i;
            masuRedNum++;
        }
    }
    if(masuRedNum > 0) {
        masuId = masuRedTbl[MBCapsuleEffRand(masuRedNum)];
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
    } else {
        masuId = MASU_NULL;
    }
    if(masuId != MASU_NULL) {
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBCameraSkipSet(FALSE);
        MBCameraFocusMasuSet(masuId);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
        WipeWait();
        capsulePos = masuPos;
        MBCapsuleDustCloudAdd(0, masuPos);
        capsulePos = masuPos;
        capsulePos.y -= 50;
        MBCapsuleDustCloudAdd(0, masuPos);
        MBMasuTypeSet(MASU_LAYER_DEFAULT, masuId, MASU_TYPE_KUPA);
        MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, CAPSULE_KOOPA);
        MBAudFXPlay(MSM_SE_BOARD_85);
        do {
            HuPrcVSleep();
        } while(MBCapsuleExplodeEffCheck(0) > 0);
        for(i=0; i<60.0f; i++) {
            HuPrcVSleep();
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBCameraSkipSet(FALSE);
        MBCameraFocusPlayerSet(capsulePlayer);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    }
    MBAudFXPlay(MSM_SE_GUIDE_12);
    MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_KOOPA_RETURN, HUWIN_SPEAKER_KOOPA);
    MBTopWinWait();
    MBTopWinKill();
    MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_FG)) {
        HuPrcVSleep();
    }
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
    HuAudFXPlay(MSM_SE_GUIDE_15);
    for(i=0; i<90.0f; i++) {
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBMusMainPlay();
    MBCapsulePlayerStunSet(squishTbl, stunNum, FALSE);
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
}

static void KoopaLast5Exec(int mdlId, BOOL partyF)
{
    int i; //r31
    int j; //r30
    int rank; //r28
    int playerNo; //r27
    int resultPlayer; //r24
    int masuId; //r23
    int effectNo; //r19
    int rouletteMdlId; //r18
    
    float time; //f31
    float prevTime; //f30
    float t; //f29
    
    int rouletteData[10]; //sp+0x10C
    int playerOrder[GW_PLAYER_MAX][GW_PLAYER_MAX]; //sp+0xCC
    HuVecF playerPos; //sp+0xC0
    HuVecF playerVel; //sp+0xB4
    HuVecF mdlPos; //sp+0xA8
    HuVecF masuPos; //sp+0x9C
    HuVecF viewRot; //sp+0x90
    HuVecF statusPosBegin; //sp+0x84
    HuVecF statusPosEnd; //sp+0x78
    char insertMes[16]; //sp+0x68
    int playerMasuId; //sp+0x58
    int helpWinId; //sp+0x54
    int guideMdlId; //sp+0x50
    int turnOld; //sp+0x4C
    int hammerMdlId; //sp+0x48
    int motId; //sp+0x44
    BOOL dustEffF; //sp+0x40
    int srcIdx; //sp+0x3C
    int dstIdx; //sp+0x38
    int temp; //sp+0x34
    s8 storyComNum; //sp+0x8
    static u32 rankMes[GW_PLAYER_MAX] = {
        MESS_LAST5KOOPA_RANK_1,
        MESS_LAST5KOOPA_RANK_2,
        MESS_LAST5KOOPA_RANK_3,
        MESS_LAST5KOOPA_RANK_4
    };
    static u32 rouletteEffMes[4] = {
        MESS_LAST5KOOPA_EFFECT_1,
        MESS_LAST5KOOPA_EFFECT_2,
        MESS_LAST5KOOPA_EFFECT_3,
        MESS_LAST5KOOPA_EFFECT_4
    };
    
    static int rouletteTbl[10] = {
        0,
        0,
        0,
        0,
        1,
        1,
        1,
        2,
        2,
        3
    };
    MBMusFadeOutDefault(MB_MUS_CHAN_BG);
    while(MBMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_LAST5, 127, 0);
    playerMasuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    MBCapsuleStatusPosMoveWait(FALSE, TRUE);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            MBModelDispSet(MBPlayerModelGet(i), FALSE);
        }
    }
    if(GWPartyFGet() == TRUE) {
        for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
            if(MBMasuFlagGet(MASU_LAYER_DEFAULT, i) & MASU_FLAG_START) {
                break;
            }
        }
        if(i <= MBMasuNumGet(MASU_LAYER_DEFAULT)) {
            masuId = i;
        } else {
            masuId = 1;
        }
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &mdlPos);
        mdlPos.x += 200;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelDispSet(mdlId, TRUE);
        MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
        rouletteMdlId = MBCapsuleModelCreate(BLAST5_HSF_roulette, NULL, FALSE, 10, FALSE);
        MBModelAttrSet(rouletteMdlId, HU3D_MOTATTR_LOOP);
        MBMotionTimeSet(rouletteMdlId, 0);
        MBMotionSpeedSet(rouletteMdlId, 0);
        MBModelPosSet(rouletteMdlId, mdlPos.x-200, mdlPos.y+200, mdlPos.z-100);
        MBCameraSkipSet(FALSE);
        viewRot.x = -15;
        viewRot.y = 0;
        viewRot.z = 0;
        masuPos.x = 0;
        masuPos.y = 150;
        masuPos.z = 0;
        MBCameraModelViewSet(mdlId, &viewRot, &masuPos, 500, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        WipeWait();
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        HuPrcSleep(60);
        turnOld = GwSystem.turnNo;
        if(GwSystem.turnMax-GwSystem.turnNo >= 5) {
            GwSystem.turnNo = GwSystem.turnMax-4;
        }
        MBTelopLast5Create();
        GwSystem.turnNo = turnOld;
        hammerMdlId = MBCapsuleModelCreate(CAPSULE_HSF_hammer, NULL, FALSE, 10, FALSE);
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
        masuPos.y = 250;
        MBModelPosSetV(hammerMdlId, &masuPos);
        MBModelDispSet(hammerMdlId, FALSE);
        viewRot.x = -20;
        viewRot.y = 0;
        viewRot.z = 0;
        masuPos.x = 0;
        masuPos.y = 150;
        masuPos.z = 0;
        MBCameraMasuViewSet(masuId, &viewRot, &masuPos, 1900, -1, 21);
        MBCameraMotionWait();
    } else {
        for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
            if(MBMasuFlagGet(MASU_LAYER_DEFAULT, i) & MASU_FLAG_START) {
                break;
            }
        }
        if(i <= MBMasuNumGet(MASU_LAYER_DEFAULT)) {
            masuId = i;
        } else {
            masuId = 1;
        }
        rouletteMdlId = MB_MODEL_NONE;
        viewRot.x = -20;
        viewRot.y = 0;
        viewRot.z = 0;
        masuPos.x = 0;
        masuPos.y = 150;
        masuPos.z = 0;
        MBCameraMasuViewSet(masuId, &viewRot, &masuPos, 1900, -1, 21);
        MBCameraMotionWait();
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &mdlPos);
        mdlPos.x += 200;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelDispSet(mdlId, TRUE);
        MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_NONE);
        MBMotionTimeSet(mdlId, MBMotionMaxTimeGet(mdlId)*0.5f);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY,60);
        dustEffF = FALSE;
        do {
            if(!dustEffF && MBMotionTimeGet(mdlId) > 137) {
                MBCapsuleDustHeavyAdd(0, mdlPos);
                MBAudFXPlay(MSM_SE_BOARD_115);
                MBCapsuleVibrate(1);
                dustEffF = TRUE;
            }
            HuPrcVSleep();
        } while(!MBMotionEndCheck(mdlId) || WipeCheckEnd());
        dustEffF = FALSE;
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        HuPrcSleep(30);
        turnOld = GwSystem.turnNo;
        if(GwSystem.turnMax-GwSystem.turnNo >= 5) {
            GwSystem.turnNo = GwSystem.turnMax-4;
        }
        MBTelopLast5Create();
        GwSystem.turnNo = turnOld;
    }
    MBAudFXPlay(MSM_SE_GUIDE_12);
    MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
    if(GWPartyFGet() == TRUE) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_START_PARTY, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_START_STORY, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBCapsuleModelMotSet(mdlId, 4, HU3D_MOTATTR_LOOP, TRUE);
    for(i=0; i<72.0f; i++) {
        t = i/72.0f;
        MBModelRotSet(mdlId, 0, -90*t, 0);
        time = MBMotionTimeGet(mdlId);
        if((prevTime < 20 && time >= 20) || (prevTime < 70 && time >= 70)) {
            MBAudFXPlay(MSM_SE_GUIDE_10);
        }
        prevTime = time;
        HuPrcVSleep();
    }
    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
    if(GWPartyFGet() == TRUE) {
        if(!GWTeamFGet()) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    playerOrder[i][j] = -1;
                }
            }
            for(i=0; i<GW_PLAYER_MAX; i++) {
                rank = GwPlayer[i].rank;
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(playerOrder[rank][j] == -1) {
                        playerOrder[rank][j] = i;
                        break;
                    }
                }
            }
            resultPlayer = playerNo = -1;
            rank = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(playerOrder[i][j] != -1) {
                        statusPosBegin.x = -120;
                        statusPosBegin.y = 80.0f+(75.0f*rank);
                        statusPosEnd.x = 120;
                        statusPosEnd.y = 80.0f+(75.0f*rank);
                        MBStatusLayoutSet(playerOrder[i][j], STATUS_LAYOUT_TOP);
                        MBStatusMoveSet(playerOrder[i][j], &statusPosBegin, &statusPosEnd, STATUS_MOVE_SIN, 30);
                        rank++;
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, rankMes[i], HUWIN_SPEAKER_KOOPA);
                        MBTopWinInsertMesSet(MBPlayerNameMesGet(playerOrder[i][j]), 0);
                        MBTopWinWait();
                        MBTopWinKill();
                        if(resultPlayer == -1) {
                            resultPlayer = playerOrder[i][j];
                        }
                        playerNo = playerOrder[i][j];
                    }
                }
            }
            if(resultPlayer == -1) {
                resultPlayer = 0;
            }
            if(playerNo == -1) {
                playerNo = 0;
            }
            MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_RESULT_PREDICT, HUWIN_SPEAKER_KOOPA);
            MBTopWinInsertMesSet(MBPlayerNameMesGet(resultPlayer), 0);
            MBTopWinWait();
            MBTopWinKill();
            rank = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(playerOrder[i][j] != -1) {
                        statusPosBegin.x = -120;
                        statusPosBegin.y = 80.0f+(75.0f*rank);
                        statusPosEnd.x = 120;
                        statusPosEnd.y = 80.0f+(75.0f*rank);
                        MBStatusLayoutSet(playerOrder[i][j], STATUS_LAYOUT_TOP);
                        MBStatusMoveSet(playerOrder[i][j], &statusPosEnd, &statusPosBegin, STATUS_MOVE_REVCOS, 30);
                        rank++;
                    }
                }
            }
        } else {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    playerOrder[i][j] = -1;
                }
            }
            for(i=0; i<2; i++) {
                rank = MBPlayerGrpRankGet(i);
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(playerOrder[rank][j] == -1) {
                        playerOrder[rank][j] = i;
                        break;
                    }
                }
            }
            resultPlayer = playerNo = -1;
            rank = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(playerOrder[i][j] != -1) {
                        statusPosBegin.x = -140;
                        statusPosBegin.y = 80.0f+(75.0f*rank);
                        statusPosEnd.x = 140;
                        statusPosEnd.y = 80.0f+(75.0f*rank);
                        MBStatusLayoutSet(MBPlayerGrpFindPlayer(playerOrder[i][j], 0), STATUS_LAYOUT_TOP);
                        MBStatusMoveSet(MBPlayerGrpFindPlayer(playerOrder[i][j], 0), &statusPosBegin, &statusPosEnd, STATUS_MOVE_SIN, 30);
                        rank++;
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, rankMes[i], HUWIN_SPEAKER_KOOPA);
                        MBTopWinInsertMesSet(MBCapsuleTeamNameGet(GwPlayer[MBPlayerGrpFindPlayer(playerOrder[i][j], 0)].charNo, GwPlayer[MBPlayerGrpFindPlayer(playerOrder[i][j], 1)].charNo), 0);
                        MBTopWinWait();
                        MBTopWinKill();
                        if(resultPlayer == -1) {
                            resultPlayer = playerOrder[i][j];
                        }
                        playerNo = playerOrder[i][j];
                    }
                }
            }
            if(resultPlayer == -1) {
                resultPlayer = 0;
            }
            if(playerNo == -1) {
                playerNo = 0;
            }
            MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_RESULT_PREDICT, HUWIN_SPEAKER_KOOPA);
            MBTopWinInsertMesSet(MBCapsuleTeamNameGet(GwPlayer[MBPlayerGrpFindPlayer(resultPlayer, 0)].charNo, GwPlayer[MBPlayerGrpFindPlayer(resultPlayer, 1)].charNo), 0);
            MBTopWinWait();
            MBTopWinKill();
            rank = 0;
            for(i=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(playerOrder[i][j] != -1) {
                        statusPosBegin.x = -140;
                        statusPosBegin.y = 80.0f+(75.0f*rank);
                        statusPosEnd.x = 140;
                        statusPosEnd.y = 80.0f+(75.0f*rank);
                        MBStatusLayoutSet(MBPlayerGrpFindPlayer(playerOrder[i][j], 0), STATUS_LAYOUT_TOP);
                        MBStatusMoveSet(MBPlayerGrpFindPlayer(playerOrder[i][j], 0), &statusPosEnd, &statusPosBegin, STATUS_MOVE_REVCOS, 30);
                        rank++;
                    }
                }
            }
            playerNo = MBPlayerGrpFindPlayer(playerNo, frand() & 0x1);
        }
    } else {
        rank = 0;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i)) {
                statusPosBegin.x = -120;
                statusPosBegin.y = 80.0f+(75.0f*rank);
                statusPosEnd.x = 120;
                statusPosEnd.y = 80.0f+(75.0f*rank);
                MBStatusLayoutSet(i, STATUS_LAYOUT_TOP);
                MBStatusMoveSet(i, &statusPosBegin, &statusPosEnd, STATUS_MOVE_SIN, 30);
                rank++;
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_COINNUM, HUWIN_SPEAKER_KOOPA);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(i), 0);
                sprintf(insertMes, "%d", MBPlayerCoinGet(i));
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes), 1);
                MBTopWinWait();
                MBTopWinKill();
            }
        }
        for(i=0, storyComNum=0; i<GW_PLAYER_MAX; i++) {
            if(!MBPlayerAliveCheck(i)) {
                continue;
            }
            if(MBPlayerStoryComCheck(i)) {
                storyComNum++;
            }
        }
        if(storyComNum >= 2) {
            MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_LOOP);
            MBAudFXPlay(MSM_SE_GUIDE_12);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_MINIK_MULTI, HUWIN_SPEAKER_KOOPA);
        } else {
            MBMotionShiftSet(mdlId, 9, 0, 8, HU3D_MOTATTR_LOOP);
            MBAudFXPlay(MSM_SE_GUIDE_13);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_MINIK_SINGLE, HUWIN_SPEAKER_KOOPA);
        }
        MBTopWinWait();
        MBTopWinKill();
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        rank = 0;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i)) {
                statusPosBegin.x = -120;
                statusPosBegin.y = 80.0f+(75.0f*rank);
                statusPosEnd.x = 120;
                statusPosEnd.y = 80.0f+(75.0f*rank);
                MBStatusLayoutSet(i, STATUS_LAYOUT_TOP);
                MBStatusMoveSet(i, &statusPosEnd, &statusPosBegin, STATUS_MOVE_REVCOS, 30);
                rank++;
            }
        }
        playerNo = 0;
    }
    do {
        HuPrcVSleep();
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i) && !MBStatusMoveCheck(i)) {
                break;
            }
        }
    } while(i < GW_PLAYER_MAX);
    MBStatusDispSetAll(FALSE);
    MBCapsuleModelMotSet(mdlId, 4, HU3D_MOTATTR_LOOP, TRUE);
    for(i=0; i<72.0f; i++) {
        t = i/72.0f;
        MBModelRotSet(mdlId, 0, -90+(45*t), 0);
        time = MBMotionTimeGet(mdlId);
        if((prevTime < 20 && time >= 20) || (prevTime < 70 && time >= 70)) {
            MBAudFXPlay(MSM_SE_GUIDE_10);
        }
        prevTime = time;
        HuPrcVSleep();
    }
    MBAudFXPlay(MSM_SE_GUIDE_14);
    MBCapsuleModelMotSet(mdlId, 5, HU3D_MOTATTR_NONE, TRUE);
    MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
    playerPos.x = mdlPos.x-350;
    playerPos.y = mdlPos.y+700;
    playerPos.z = mdlPos.z+200;
    playerVel.x = playerVel.y = playerVel.z = 0;
    MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_DIZZY, HU3D_MOTATTR_LOOP);
    do {
        HuPrcVSleep();
        VECAdd(&playerPos, &playerVel, &playerPos);
        playerVel.y -= 1.6333334f;
        MBPlayerPosSetV(playerNo, &playerPos);
        MBPlayerRotSet(playerNo, 0, 0, 0);
        MBModelDispSet(MBPlayerModelGet(playerNo), TRUE);
    } while(playerPos.y > mdlPos.y);
    omVibrate(playerNo, 12, 4, 2);
    playerPos.y = mdlPos.y;
    MBPlayerPosSetV(playerNo, &playerPos);
    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
    HuPrcSleep(60);
    MBPlayerRotateStart(playerNo, 135, 15);
    while(!MBPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
    if(GWPartyFGet() == TRUE) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_ROULETTE, HUWIN_SPEAKER_KOOPA);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 0);
        MBTopWinWait();
        MBTopWinKill();
        MBSaiExec(playerNo, SAITYPE_BLOCK, NULL, -1, FALSE, FALSE, NULL, SAI_COLOR_GREEN);
        saiHookTime = 1;
        MBSaiHitHookSet(playerNo, KoopaLast5SaiHit);
        for(i=0; i<10; i++) {
            rouletteData[i] = rouletteTbl[i];
        }
        for(i=0; i<50; i++) {
            srcIdx = MBCapsuleEffRand(10);
            dstIdx = MBCapsuleEffRand(10);
            if(srcIdx != dstIdx) {
                temp = rouletteData[srcIdx];
                rouletteData[srcIdx] = rouletteData[dstIdx];
                rouletteData[dstIdx] = temp;
            }
        }
        helpWinId = MBWinCreateHelp(MESS_BOARDOPE_PAD_SAI);
        effectNo = MBCapsuleEffRand(4);
        i = 0;
        do {
            if(saiHookTime) {
                if(++i >= 2) {
                    i = 0;
                    if(++effectNo >= 10){
                        effectNo =0;
                    }
                }
                MBMotionTimeSet(rouletteMdlId, 1.5f+rouletteData[effectNo]);
            } else {
                if(helpWinId != MBWIN_NONE) {
                    MBWinKill(helpWinId);
                }
                helpWinId = MBWIN_NONE;
                MBSaiNumKill(playerNo);
            }
            HuPrcVSleep();
        } while(!MBSaiKillCheck(playerNo));
        effectNo = rouletteData[effectNo];
        HuPrcSleep(30);
        MBAudFXPlay(MSM_SE_GUIDE_12);
        MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_ROULETTE_RESULT, HUWIN_SPEAKER_KOOPA);
        MBTopWinInsertMesSet(rouletteEffMes[effectNo], 0);
        MBTopWinWait();
        MBTopWinKill();
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBModelDispSet(MBPlayerModelGet(playerNo), FALSE);
        MBModelDispSet(rouletteMdlId, FALSE);
        MBModelDispSet(mdlId, FALSE);
        masuId = MBMasuFlagMatchFind(MASU_LAYER_DEFAULT, MASU_FLAG_START);
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
        guideMdlId = MBGuideCreateDirect();
        MBModelPosSet(guideMdlId, masuPos.x, masuPos.y+200, masuPos.z);
        MBModelLayerSet(guideMdlId, 2);
        HuDataDirClose(DATA_guide);
        MBCameraSkipSet(FALSE);
        viewRot.x = -33;
        viewRot.y = 0;
        viewRot.z = 0;
        MBCameraMasuViewSet(masuId, &viewRot, &capsuleViewOfs, 1600, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        switch(effectNo) {
            case 0:
                GwSystem.last5Effect = MB_LAST5_EFF_COINMUL;
                break;
            
            case 1:
                GwSystem.last5Effect = MB_LAST5_EFF_RANDCAPSULE;
                for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
                    if(MBMasuTypeGet(MASU_LAYER_DEFAULT, i) == MASU_TYPE_BLUE || 
                        MBMasuTypeGet(MASU_LAYER_DEFAULT, i) == MASU_TYPE_RED) {
                            do {
                                j = MBCapsuleNextGet(-1);
                            } while(j == CAPSULE_HONE || j == CAPSULE_KOOPA || j == CAPSULE_MIRACLE);
                            MBCapsuleMasuSet(i, j);
                        }
                }
                break;
            
            case 2:
                GwSystem.last5Effect = MB_LAST5_EFF_REDKUPA;
                for(i=1; i<MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
                    if(MBMasuTypeGet(MASU_LAYER_DEFAULT, i) == MASU_TYPE_RED) {
                        MBMasuTypeSet(MASU_LAYER_DEFAULT, i, MASU_TYPE_KUPA);
                        MBMasuCapsuleSet(MASU_LAYER_DEFAULT, i, CAPSULE_KOOPA);
                    }
                }
                break;
            
            default:
                GwSystem.last5Effect = MB_LAST5_EFF_5STAR;
                for(i=0; i<4; i++) {
                    MBStarCreate(MBStarNoRandGet());
                }
                break;
        }
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        WipeWait();
        MBAudFXPlay(MSM_SE_GUIDE_51);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_RESULT, MBGuideSpeakerNoGet());
        MBTopWinInsertMesSet(rouletteEffMes[effectNo], 0);
        MBTopWinWait();
        MBTopWinKill();
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        if(guideMdlId != MB_MODEL_NONE) {
            MBGuideEnd(guideMdlId);
        }
    } else {
        MBAudFXPlay(MSM_SE_GUIDE_12);
        MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, 1);
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_LAST5KOOPA_KETTOU_2X, HUWIN_SPEAKER_KOOPA);
        MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo), 0);
        MBTopWinWait();
        MBTopWinKill();
        GwSystem.last5Effect = MB_LAST5_EFF_TAKEDOUBLE;
        MBAudFXPlay(MSM_SE_GUIDE_14);
        MBMotionShiftSet(mdlId, 5, 0, 8, HU3D_MOTATTR_NONE);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        motId = MBCapsulePlayerMotionCreate(playerNo, CHARMOT_HSF_c000m1_484);
        MBCapsulePlayerMotSet(playerNo, motId, HU3D_MOTATTR_NONE, TRUE);
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        HuPrcSleep(30);
        MBPlayerMotionNoSet(playerNo, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
    }
    MBModelDispSet(mdlId, FALSE);
    if(rouletteMdlId != MB_MODEL_NONE) {
        MBModelDispSet(rouletteMdlId, FALSE);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        MBModelDispSet(MBPlayerModelGet(i), TRUE);
        MBPlayerPosFixPlayer(capsulePlayer, GwPlayer[i].masuId, TRUE);
        MBPlayerRotSet(i, 0, 0, 0);
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, GwPlayer[capsulePlayer].masuId, &playerPos);
    MBPlayerPosSetV(capsulePlayer, &playerPos);
    MBPlayerRotSet(capsulePlayer, 0, 0, 0);
    MBCameraSkipSet(FALSE);
    viewRot.x = -33;
    viewRot.y = 0;
    viewRot.z = 0;
    MBCameraModelViewSet(MBPlayerModelGet(0), &viewRot, &capsuleViewOfs, -1, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBMusMainPlay();
}

static void KoopaMasuExec(int mdlId, BOOL partyF)
{
    int i; //r31
    int donkeyMdlId; //r29
    int coinNum; //r25
    int masuId; //r20
    int no; //r19
    int aliveNum; //r18
    int eventNo; //r17
    
    float rotY; //f31
    float scale; //f30
    float speed; //f29
    float yOfs; //f28
    float motTime; //f27
    float dist; //f26
    float prevTime; //f25
    double tpLvl; //f24
    
    HuVecF playerPos[GW_PLAYER_MAX]; //sp+0x1DC
    int eventTbl[10]; //sp+0x1B4
    int motFile[16]; //sp+0x174
    HuVecF capsulePos; //sp+0x168
    HuVecF donkeyVel; //sp+0x15C
    HuVecF mdlPos; //sp+0x150
    HuVecF otherMdlPos; //sp+0x144
    HuVecF mdlRot; //sp+0x138
    HuVecF donkeyDir; //sp+0x12C
    HuVecF donkeyAimPos; //sp+0x120
    HuVecF vel; //sp+0x114
    int playerMasuId[GW_PLAYER_MAX]; //sp+0x104
    int squishTbl[GW_PLAYER_MAX]; //sp+0xF4
    HuVecF masuPos; //sp+0xE8
    HuVecF masuNextPos; //sp+0xDC
    HuVecF donkeyMasuPos; //sp+0xD0
    int playerMax; //sp+0xA8
    int idleNum; //sp+0xA4
    int statId; //sp+0xA0
    int squishNum; //sp+0x9C
    int coinDispId; //sp+0x98
    int starNumId; //sp+0x94
    int seNo; //sp+0x90
    int motId; //sp+0x8C
    int fadeSprId; //sp+0x88
    int resultNo; //sp+0x84
    int ztarMdlId; //sp+0x80
    int donkeyCoin; //sp+0x7C
    int donkeySpeed; //sp+0x78
    int musNo; //sp+0x74
    int winId; //sp+0x70
    int eventNum; //sp+0x6C
    int srcIdx; //sp+0x68
    int dstIdx; //sp+0x64
    int temp; //sp+0x60
    int nextMasuId; //sp+0x5C
    GXColor color; //sp+0x58
    int donkeyMasuId; //sp+0x54
    static int eventFileTbl[4] = {
        CAPSULE_ANM_koopaEvent1,
        CAPSULE_ANM_koopaEvent2,
        CAPSULE_ANM_koopaEvent3,
        CAPSULE_ANM_koopaEvent4
    };
    static u32 resultMesTbl[3] = {
        MESS_KOOPAMASU_EVENT_RESULT1,
        MESS_KOOPAMASU_EVENT_RESULT2,
        MESS_KOOPAMASU_EVENT_RESULT3
    };
    static int rouletteTbl[10] = {
        0,
        0,
        1,
        1,
        2,
        2,
        3,
        3,
        3,
        3
    };
    
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &capsulePos);
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
    motId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_325);
    fadeSprId = MBCapsuleSprCreate(CAPSULE_ANM_koopaFade, 100, 0);
    espPosSet(fadeSprId, 288, 240);
    espScaleSet(fadeSprId, 4, 4);
    espTPLvlSet(fadeSprId, 0);
    espDispOff(fadeSprId);
    espAttrSet(fadeSprId, HUSPR_ATTR_LINEAR);
    MBCameraModelViewSet(MBPlayerModelGet(capsulePlayer), NULL, &capsuleViewOfs, 1500, -1, 21);
    musNo = MBMusPlayJingle(MSM_STREAM_JNGL_BOWSER_IN);
    MBEffFadeCreate(30, 160);
    MBCameraMotionWait();
    MBPlayerMotionNoShiftSet(capsulePlayer, motId, 0, 10, HU3D_MOTATTR_LOOP);
    MBPlayerMotionSpeedSet(capsulePlayer, 2);
    HuPrcSleep(30);
    espDispOn(fadeSprId);
    for(i=0; i<360; i += 2) {
        if(i == 60) {
            MBAudFXPlay(MSM_SE_BOARD_103);
        }
        if(i == 90 || i == 270 || i == 450) {
            omVibrate(capsulePlayer, 12, 4, 2);
        }
        if(HuSin(MBAngleWrap(i)) < 0) {
            tpLvl = -HuSin(MBAngleWrap(i));
        } else {
            tpLvl = HuSin(MBAngleWrap(i));
        }
        espTPLvlSet(fadeSprId, tpLvl);
        HuPrcVSleep();
    }
    espDispOff(fadeSprId);
    mdlPos = capsulePos;
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &mdlPos);
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_NONE);
    MBMotionTimeSet(mdlId, MBMotionMaxTimeGet(mdlId)*0.5f);
    squishNum = 0;
    do {
        if(squishNum == 0 && MBMotionTimeGet(mdlId) > 137) {
            MBCapsuleDustHeavyAdd(0, mdlPos);
            MBAudFXPlay(MSM_SE_BOARD_115);
            MBCapsuleVibrate(1);
            squishNum = MBCapsulePlayerSquishSet(squishTbl, masuId);
        }
        HuPrcVSleep();
    } while(!MBMotionEndCheck(mdlId));
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBEffFadeOutSet(30);
    HuPrcSleep(30);
    while(MBMusJingleStatGet(musNo) != MSM_MUS_DONE) {
        HuPrcVSleep();
    }
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_KOOPA, 127, 0);
    MBAudFXPlay(MSM_SE_GUIDE_12);
    MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
    MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
    if(GWPartyFGet() == TRUE) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_START_PARTY, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
        for(i=0, eventNum=0; i<10; i++) {
            if((MBPlayerStarGet(capsulePlayer) >= 1 || rouletteTbl[i]) && (!GWTeamFGet() || rouletteTbl[i] != 3)) {
                eventTbl[eventNum] = rouletteTbl[i];
                eventNum++;
            }
        }
        for(i=0; i<eventNum*5; i++) {
            srcIdx = i%eventNum;
            dstIdx = MBCapsuleEffRand(eventNum);
            temp = eventTbl[srcIdx];
            eventTbl[srcIdx] = eventTbl[dstIdx];
            eventTbl[dstIdx] = temp;
        }
        eventNo = eventTbl[MBCapsuleEffRand(eventNum)];
        if(MBPlayerStarGet(capsulePlayer) < 1 && eventNo == 0) {
            eventNo = 1;
        }
        if(GWTeamFGet() && eventNo == 3) {
            eventNo = 2;
        }
        if((i=frand()%8) == 0) {
            nextMasuId = MBCapsuleMasuNextGet(masuId, &masuNextPos);
            if(nextMasuId > 0) {
                MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(nextMasuId == GwPlayer[i].masuId) {
                        break;
                    }
                }
                if(fabs(masuPos.y-masuNextPos.y) < 1 && i >= GW_PLAYER_MAX) {
                    eventNo = 4;
                }
            }
        }
        if(MBPlayerCoinGet(capsulePlayer) <= 0) {
            eventNo = 5;
        }
        if(eventNo < 4) {
            MBMotionShiftSet(mdlId, 5, 0, 8, HU3D_MOTATTR_NONE);
            HuPrcSleep(30);
            MBAudFXPlay(MSM_SE_BOARD_113);
            MBCapsuleSpriteShow(eventFileTbl[eventNo], 288, 240, FALSE);
            MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
        }
        switch(eventNo) {
            case 0:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_START_EVENT1, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
                MBCapsuleModelMotSet(mdlId, 5, HU3D_MOTATTR_NONE, 1);
                motFile[0] = CAPSULE_HSF_ztarmot;
                motFile[1] = HU_DATANUM_NONE;
                ztarMdlId = MBCapsuleModelCreate(CAPSULE_HSF_ztar, motFile, FALSE, 10, FALSE);
                MBMotionNoSet(ztarMdlId, 1, HU3D_MOTATTR_LOOP);
                otherMdlPos.x = mdlPos.x;
                otherMdlPos.y = mdlPos.y+150;
                otherMdlPos.z = mdlPos.z+150;
                rotY = 0;
                MBModelPosSetV(ztarMdlId, &otherMdlPos);
                seNo = MBAudFXPlay(MSM_SE_BOARD_114);
                for(i=0; i<60.0f; i++) {
                    scale = i/60.0f;
                    rotY += 10;
                    MBModelRotSet(ztarMdlId, 0, rotY, 0);
                    MBModelScaleSet(ztarMdlId, scale, scale, scale);
                    HuPrcVSleep();
                }
                for(i=0; i<60.0f; i++) {
                    rotY += 10;
                    MBModelRotSet(ztarMdlId, 0, rotY, 0);
                    HuPrcVSleep();
                }
                for(i=0; i<30.0f; i++) {
                    scale = i/30.0f;
                    rotY += 10;
                    otherMdlPos.x = mdlPos.x;
                    otherMdlPos.y = mdlPos.y+150+(scale*300);
                    otherMdlPos.z = mdlPos.z+150;
                    MBModelPosSetV(ztarMdlId, &otherMdlPos);
                    MBModelRotSet(ztarMdlId, 0, rotY, 0);
                    MBModelScaleSet(ztarMdlId, 1-scale, 1+scale, 1-scale);
                    HuPrcVSleep();
                }
                if(seNo != MSM_SENO_NONE) {
                    MBAudFXStop(seNo);
                }
                MBModelDispSet(ztarMdlId, FALSE);
                MBPlayerStarAdd(capsulePlayer, -1);
                starNumId = MBCapsuleStarNumCreate(capsulePlayer, -1);
                do {
                    HuPrcVSleep();
                } while(!MBCapsuleStarNumCheck(starNumId));
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                break;
            
            case 1:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_START_EVENT2, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
                MBCapsuleStatusPosMoveWait(TRUE, TRUE);
                MBAudFXPlay(MSM_SE_GUIDE_14);
                MBCapsuleModelMotSet(mdlId, 5, HU3D_MOTATTR_NONE, TRUE);
                MBCapsuleVibrate(2);
                WipeColorSet(127, 127, 127);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 1);
                WipeWait();
                MBAudFXPlay(MSM_SE_BOARD_59);
                for(i=0, coinNum=0; i<GW_PLAYER_MAX; i++) {
                    coinNum += MBPlayerCoinGet(i);
                }
                coinNum /= 4;
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    MBPlayerCoinSet(i, coinNum);
                }
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                WipeWait();
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                break;
            
            case 2:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_START_EVENT3, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
                MBAudFXPlay(MSM_SE_GUIDE_14);
                MBCapsuleModelMotSet(mdlId, 5, HU3D_MOTATTR_NONE, TRUE);
                MBCapsuleVibrate(2);
                WipeColorSet(255, 255, 255);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                WipeWait();
                MBAudFXPlay(MSM_SE_BOARD_59);
                MBModelDispSet(mdlId, FALSE);
                for(i=0, aliveNum=0; i<GW_PLAYER_MAX; i++) {
                    if(MBPlayerAliveCheck(i)) {
                        MBPlayerPosGet(i, &playerPos[aliveNum]);
                        playerMasuId[aliveNum] = GwPlayer[i].masuId;
                        aliveNum = aliveNum+1;
                    }
                }
                playerMax = MBCapsuleEffRand(aliveNum);
                if(playerMax == 0 || playerMax == aliveNum) {
                    playerMax = 1;
                }
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if(!MBPlayerAliveCheck(i)) {
                        continue;
                    }
                    if(aliveNum) {
                       if((no = playerMax+i) >= aliveNum) {
                           no %= aliveNum;
                       }
                    }
                    GwPlayer[i].masuId = playerMasuId[no];
                    MBPlayerPosSetV(i, &playerPos[no]);
                    MBModelDispSet(MBPlayerModelGet(i), FALSE);
                    MBPlayerScaleSet(i, 1, 1, 1);
                    MBPlayerMotionNoSet(i, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
                    MBPlayerMotionSpeedSet(i, 1);
                }
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    if((no = i+capsulePlayer) >= GW_PLAYER_MAX) {
                        no -= GW_PLAYER_MAX;
                    }
                    if(!MBPlayerAliveCheck(no)) {
                        continue;
                    }
                    MBCameraSkipSet(FALSE);
                    MBCameraModelViewSet(MBPlayerModelGet(no), NULL, &capsuleViewOfs, -1, -1, 1);
                    MBCameraMotionWait();
                    MBCameraSkipSet(TRUE);
                    MBModelDispSet(MBPlayerModelGet(no), TRUE);
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                    HuPrcSleep(60);
                    WipeColorSet(255, 255, 255);
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                    WipeWait();
                }
                squishNum = MBCapsulePlayerSquishSet(squishTbl, masuId);
                MBModelDispSet(mdlId, TRUE);
                MBCameraSkipSet(FALSE);
                MBCameraMasuViewSet(masuId, NULL, &capsuleViewOfs, -1, -1, 1);
                MBCameraMotionWait();
                MBCameraSkipSet(TRUE);
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
                WipeWait();
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                if(squishNum) {
                    capsulePlayer = squishTbl[0];
                }
                break;
            
            case 3:
                MBAudFXPlay(MSM_SE_GUIDE_14);
                MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_LOOP);
                resultNo = MBCapsuleEffRand(3);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_START_EVENT4, HUWIN_SPEAKER_KOOPA);
                MBTopWinInsertMesSet(resultMesTbl[resultNo], 0);
                MBTopWinWait();
                MBTopWinKill();
                MBAudFXPlay(MSM_SE_GUIDE_14);
                MBMotionShiftSet(mdlId, 5, 0, 8, HU3D_MOTATTR_NONE);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_ROULETTE, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
                memset(&MgResultData, 0, sizeof(MgResultData));
                MgResultData.resultNo = resultNo;
                GWMgCoinBonusSet(0, 0);
                GWMgCoinBonusSet(1, 0);
                GWMgCoinBonusSet(2, 0);
                GWMgCoinBonusSet(3, 0);
                MBMgCallKupaExec();
                MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                while(MBMusCheck(MB_MUS_CHAN_FG)) {
                    HuPrcVSleep();
                }
                MBMusMainPlay();
                HuPrcEnd();
                return;
            
            case 4:
                MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                while(MBMusCheck(MB_MUS_CHAN_FG)) {
                    HuPrcVSleep();
                }
                HuDataDirClose(DATA_capsulechar2);
                HuDataDirClose(DATA_capsulechar4);
                statId = MBDataDirReadAsync(DATA_capsulechar3);
                if(statId != HU_DATA_STAT_NONE) {
                    MBDataAsyncWait(statId);
                }
                motFile[0] = CAPSULECHAR3_HSF_donkey_idle;
                motFile[1] = CAPSULECHAR3_HSF_donkey_roll;
                motFile[2] = CAPSULECHAR3_HSF_donkey_win;
                motFile[3] = CAPSULECHAR3_HSF_donkey_lose;
                motFile[4] = CAPSULECHAR3_HSF_donkey_walk;
                motFile[5] = CAPSULECHAR3_HSF_donkey_jump_start;
                motFile[6] = CAPSULECHAR3_HSF_donkey_jump;
                motFile[7] = CAPSULECHAR3_HSF_donkey_punch_hard;
                motFile[8] = CAPSULECHAR3_HSF_donkey_sad;
                motFile[9] = HU_DATANUM_NONE;
                donkeyMdlId = MBCapsuleModelCreate(CAPSULECHAR3_HSF_donkey, motFile, FALSE, 10, FALSE);
                MBModelDispSet(donkeyMdlId, FALSE);
                HuDataDirClose(DATA_capsulechar3);
                donkeyMasuId = MBCapsuleMasuNextGet(masuId, &donkeyMasuPos);
                if(donkeyMasuId <= 0) {
                    donkeyMasuId = masuId;
                    donkeyMasuPos.x = capsulePos.x+200;
                    donkeyMasuPos.y = capsulePos.y;
                    donkeyMasuPos.z = capsulePos.z;
                } else {
                    MBPlayerPosFixPlayer(capsulePlayer, masuId, FALSE);
                }
                otherMdlPos.x = 0;
                otherMdlPos.y = 100;
                otherMdlPos.z = 0;
                MBCameraMasuViewSet(donkeyMasuId, NULL, &otherMdlPos, 2500, -1, 21);
                MBCameraMotionWait();
                MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_DONKEY, 127, 0);
                otherMdlPos.x = donkeyMasuPos.x;
                otherMdlPos.y = donkeyMasuPos.y+700;
                otherMdlPos.z = donkeyMasuPos.z;
                donkeyVel.x = donkeyVel.y = donkeyVel.z = 0;
                MBMotionNoSet(donkeyMdlId, 6, HU3D_MOTATTR_NONE);
                do {
                    HuPrcVSleep();
                    VECAdd(&otherMdlPos, &donkeyVel, &otherMdlPos);
                    donkeyVel.y -= 1.633333357671897;
                    MBModelPosSetV(donkeyMdlId, &otherMdlPos);
                    MBModelDispSet(donkeyMdlId, TRUE);
                } while(otherMdlPos.y > donkeyMasuPos.y);
                MBModelPosSetV(donkeyMdlId, &donkeyMasuPos);
                MBCapsuleModelMotSet(donkeyMdlId, 7, HU3D_MOTATTR_NONE, TRUE);
                MBAudFXPlay(MSM_SE_GUIDE_30);
                MBCapsuleModelMotSet(donkeyMdlId, 3, HU3D_MOTATTR_NONE, TRUE);
                otherMdlPos.x = 0;
                otherMdlPos.y = 100;
                otherMdlPos.z = 0;
                MBCameraMasuViewSet(masuId, NULL, &otherMdlPos, 2500, -1, 21);
                VECSubtract(&donkeyMasuPos, &mdlPos, &otherMdlPos);
                mdlRot.x = mdlRot.y = mdlRot.z = 0;
                rotY = HuAtan(otherMdlPos.x, otherMdlPos.z);
                MBCapsuleModelMotSet(mdlId, 4, HU3D_MOTATTR_LOOP, TRUE);
                do {
                    HuPrcVSleep();
                    mdlRot.y = MBCapsuleAngleLerp(rotY, mdlRot.y, 5);
                    MBModelRotSetV(mdlId, &mdlRot);
                } while(fabs(MBCapsuleAngleWrap(rotY, mdlRot.y)) > 5);
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                mdlRot.y = rotY;
                MBModelRotSetV(mdlId, &mdlRot);
                MBCameraMotionWait();
                MBAudFXPlay(MSM_SE_GUIDE_14);
                MBMotionShiftSet(mdlId, 8, 0, 8, HU3D_MOTATTR_NONE);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_DONKEY_START, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                VECSubtract(&donkeyMasuPos, &mdlPos, &donkeyDir);
                donkeyDir.y = 0;
                VECNormalize(&donkeyDir, &donkeyDir);
                VECScale(&donkeyDir, &donkeyDir, 200);
                VECAdd(&mdlPos, &donkeyDir, &donkeyAimPos);
                VECSubtract(&donkeyAimPos, &donkeyMasuPos, &donkeyDir);
                mdlRot.x = 0;
                mdlRot.z = 0;
                mdlRot.y = HuAtan(donkeyDir.x, donkeyDir.z);
                dist = VECMag(&donkeyDir);
                yOfs = 0.5f*dist;
                donkeySpeed = 30;
                if(yOfs < 200) {
                    yOfs = 200;
                }
                if(dist > 50) {
                    MBMotionShiftSet(donkeyMdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
                    for(i=0; i<=donkeySpeed; i++) {
                        scale = (float)i/(float)donkeySpeed;
                        VECScale(&donkeyDir, &otherMdlPos, scale);
                        VECAdd(&donkeyMasuPos, &otherMdlPos, &otherMdlPos);
                        otherMdlPos.y += yOfs*HuSin(scale*180);
                        MBModelPosSetV(donkeyMdlId, &otherMdlPos);
                        MBModelRotSetV(donkeyMdlId, &mdlRot);
                        HuPrcVSleep();
                    }
                    MBCapsuleModelMotSet(donkeyMdlId, 7, HU3D_MOTATTR_NONE, TRUE);
                }
                MBMotionShiftSet(donkeyMdlId, 8, 0, 8, HU3D_MOTATTR_NONE);
                i = 0;
                do {
                    HuPrcVSleep();
                    if(i == 0 && MBMotionShiftIDGet(donkeyMdlId) == HU3D_MOTID_NONE && MBMotionTimeGet(donkeyMdlId) >= 97) {
                        MBMotionNoSet(mdlId, 10, HU3D_MOTATTR_NONE);
                        i = 1;
                    }
                } while(MBMotionShiftIDGet(donkeyMdlId) != HU3D_MOTID_NONE || MBMotionTimeGet(donkeyMdlId) < 98);
                VECSubtract(&donkeyMasuPos, &mdlPos, &donkeyDir);
                donkeyDir.y = 0;
                VECNormalize(&donkeyDir, &donkeyDir);
                VECScale(&donkeyDir, &donkeyDir, 75);
                VECAdd(&mdlPos, &donkeyDir, &donkeyAimPos);
                otherMdlPos.x = donkeyAimPos.x;
                otherMdlPos.y = donkeyAimPos.y+100;
                otherMdlPos.z = donkeyAimPos.z;
                for(i=0; i<32; i++) {
                    rotY = (i*22.5f)+((MBCapsuleEffRandF()-0.5f)*10);
                    speed = (MBCapsuleEffRandF()+0.5f)*5;
                    vel.x = HuSin(rotY)*HuCos(mdlRot.y)*speed;
                    vel.y = speed*HuCos(rotY);
                    vel.z = speed*HuCos(mdlRot.y);
                    speed = ((MBCapsuleEffRandF()-0.5f)*4)*1.25f;
                    vel.x += speed*HuSin(mdlRot.y+180);
                    vel.z += speed*HuCos(mdlRot.y+180);
                    color.r = 255;
                    color.g = 255;
                    color.b = 0;
                    color.a = 255;
                    MBCapsuleGlowEffAdd(otherMdlPos, vel, 
                        ((MBCapsuleEffRandF()*0.3f)+0.3f)*100,
                        (MBCapsuleEffRandF()+1.0f)*0.016666668f,
                        0,
                        0,
                        color);
                }
                i = MBCapsuleModelCreate(CAPSULE_HSF_dkeff, NULL, FALSE, 0, FALSE);
                MBModelPosSetV(i, &otherMdlPos);
                MBModelScaleSet(i, 2.5f, 2.5f, 2.5f);
                MBModelLayerSet(i, 3);
                MBAudFXPlay(MSM_SE_BOARD_56);
                MBModelPosGet(donkeyMdlId, &donkeyMasuPos);
                VECSubtract(&mdlPos, &donkeyMasuPos, &donkeyDir);
                VECNormalize(&donkeyDir, &donkeyDir);
                VECScale(&donkeyDir, &donkeyDir, 150);
                VECAdd(&mdlPos, &donkeyDir, &vel);
                donkeySpeed = 12;
                VECSubtract(&vel, &mdlPos, &donkeyDir);
                for(i=0; i<donkeySpeed; i++) {
                    scale = (float)i/(float)donkeySpeed;
                    VECScale(&donkeyDir, &otherMdlPos, scale);
                    VECAdd(&mdlPos, &otherMdlPos, &otherMdlPos);
                    MBModelPosSetV(mdlId, &otherMdlPos);
                    HuPrcVSleep();
                }
                do {
                    HuPrcVSleep();
                } while(!MBMotionEndCheck(donkeyMdlId) || MBMotionShiftIDGet(donkeyMdlId) != HU3D_MOTID_NONE);
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                HuPrcSleep(30);
                MBMotionShiftSet(donkeyMdlId, 9, 0, 8, HU3D_MOTATTR_LOOP);
                MBCapsuleModelMotSet(mdlId, 9, HU3D_MOTATTR_LOOP, TRUE);
                MBAudFXPlay(MSM_SE_GUIDE_13);
                winId = MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_DONKEY_CANCEL, HUWIN_SPEAKER_KOOPA);
                while(!MBWinDoneCheck(winId)) {
                    motTime = MBMotionTimeGet(mdlId);
                    if((prevTime < 20 && motTime >= 20) || (prevTime < 55 && motTime >= 55)) {
                        MBAudFXPlay(MSM_SE_GUIDE_10);
                    }
                    prevTime = motTime;
                    HuPrcVSleep();
                }
                MBTopWinKill();
                MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                while(MBMusCheck(MB_MUS_CHAN_FG)) {
                    HuPrcVSleep();
                }
                WipeColorSet(255, 255, 255);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
                WipeWait();
                MBModelDispSet(mdlId, FALSE);
                MBModelDispSet(donkeyMdlId, FALSE);
                MBCameraViewNoSet(capsulePlayer, MB_CAMERA_VIEW_ZOOMIN);
                MBCameraMotionWait();
                for(idleNum=0; idleNum<squishNum; idleNum++) {
                    MBPlayerMotionNoSet(squishTbl[idleNum], MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP);
                    MBPlayerMotionSpeedSet(squishTbl[idleNum], 1);
                    MBPlayerScaleSet(squishTbl[idleNum], 1, 1, 1);
                }
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
                WipeWait();
                if(GwSystem.turnMax*0.66f > GwSystem.turnNo) {
                    donkeyCoin = 10;
                } else {
                    donkeyCoin = 20;
                }
                if(donkeyCoin > MBPlayerCoinGet(capsulePlayer)) {
                    donkeyCoin = MBPlayerCoinGet(capsulePlayer);
                }
                if(donkeyCoin > 0) {
                    MBMusMainPlay();
                    MBCoinAddExec(capsulePlayer, -donkeyCoin);
                    MBCapsuleCoinDispExec(capsulePlayer, -donkeyCoin, TRUE, TRUE);
                    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                } else {
                    MBMusMainPlay();
                }
                HuPrcEnd();
                return;
                
            default:
                MBCapsuleModelMotSet(mdlId, 6, HU3D_MOTATTR_NONE, TRUE);
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_DONKEY_FAIL, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
                MBCapsuleModelMotSet(mdlId, 5, HU3D_MOTATTR_NONE, TRUE);
                MBCoinAddDispExec(capsulePlayer, 10, TRUE);
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                break;
        }
        MBAudFXPlay(MSM_SE_GUIDE_12);
        MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
        MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_END, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
        MBCapsuleVibrate(1);
        HuAudFXPlay(MSM_SE_GUIDE_15);
        for(i=0; i<90.0f; i++) {
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        MBMusMainPlay();
        MBCapsulePlayerStunSet(squishTbl, squishNum, FALSE);
        for(i=0; i<60.0f; i++) {
            HuPrcVSleep();
        }
    } else {
        coinNum = 0;
        if(!MBPlayerStoryComCheck(capsulePlayer)) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_STORY_START, HUWIN_SPEAKER_KOOPA);
            MBTopWinWait();
            MBTopWinKill();
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_STORY_STEAL, HUWIN_SPEAKER_KOOPA);
            MBTopWinWait();
            MBTopWinKill();
            if(MBPlayerCoinGet(capsulePlayer) > 1) {
                coinNum = -(MBPlayerCoinGet(capsulePlayer)/2);
                MBCoinAddExec(capsulePlayer, coinNum);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_STORY_STEAL_SUCCESS, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
            } else {
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_STORY_STEAL_FAIL, HUWIN_SPEAKER_KOOPA);
                MBTopWinWait();
                MBTopWinKill();
            }
        } else {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_STORYCOM_START, HUWIN_SPEAKER_KOOPA);
            MBTopWinWait();
            MBTopWinKill();
            coinNum = 5;
            MBCoinAddExec(capsulePlayer, coinNum);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_STORYCOM_LOSE, HUWIN_SPEAKER_KOOPA);
            MBTopWinWait();
            MBTopWinKill();
        }
        MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
        MBCapsuleVibrate(1);
        HuAudFXPlay(MSM_SE_GUIDE_15);
        for(i=0; i<90.0f; i++) {
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId, FALSE);
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        MBMusMainPlay();
        MBCapsulePlayerStunSet(squishTbl, squishNum, TRUE);
        if(!MBPlayerStoryComCheck(capsulePlayer)) {
            for(i=0; i<60.0f; i++) {
                HuPrcVSleep();
            }
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
            if(coinNum < 0) {
                MBPlayerPosGet(capsulePlayer, &otherMdlPos);
                otherMdlPos.y += 250;
                coinDispId = MBCoinDispCreateSe(&otherMdlPos, coinNum);
                MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
                MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINLOSE, HU3D_MOTATTR_NONE, TRUE);
                MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                do {
                    HuPrcVSleep();
                } while(!MBCoinDispKillCheck(coinDispId));
            }
        } else {
            MBPlayerPosGet(capsulePlayer, &otherMdlPos);
            otherMdlPos.y += 250;
            coinDispId = MBCoinDispCreateSe(&otherMdlPos, coinNum);
            MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINWIN, HU3D_MOTATTR_NONE, TRUE);
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
            do {
                HuPrcVSleep();
            } while(!MBCoinDispKillCheck(coinDispId));
        }
    }
}

static void KoopaCircuitExec(int mdlId, BOOL partyF)
{
    int i; //r31
    int fadeSprId; //r29
    MASU *masuP; //r27
    int linkMasuId; //r25
    int masuId; //r24
    BOOL dustEffF; //r23
    int musNo; //r21
    
    double tpLvl; //f31
    
    HuVecF masuPos; //sp+0x20
    HuVecF pos; //sp+0x14
    
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &masuPos);
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, MBMasuTargetGet(masuId));
    for(i=0, linkMasuId=MASU_NULL; i<masuP->linkNum; i++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & MASU_FLAG_BLOCKR) {
            linkMasuId = masuP->linkTbl[i];
            break;
        }
    }
    if(linkMasuId == MASU_NULL) {
        linkMasuId = masuId;
    }
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
    fadeSprId = MBCapsuleSprCreate(CAPSULE_ANM_koopaFade, 100, 0);
    espPosSet(fadeSprId, 288, 240);
    espScaleSet(fadeSprId, 4, 4);
    espTPLvlSet(fadeSprId, 0);
    espDispOff(fadeSprId);
    espAttrSet(fadeSprId, HUSPR_ATTR_LINEAR);
    MBCameraModelViewSet(MBPlayerModelGet(capsulePlayer), NULL, &capsuleViewOfs, 1500, -1, 21);
    musNo = MBMusPlayJingle(MSM_STREAM_JNGL_BOWSER_IN);
    MBEffFadeCreate(30, 160);
    MBCameraMotionWait();
    espDispOn(fadeSprId);
    for(i=0; i<360; i += 2) {
        if(i == 60) {
            MBAudFXPlay(MSM_SE_BOARD_103);
        }
        if(i == 90 || i == 270 || i == 450) {
            omVibrate(capsulePlayer, 12, 4, 2);
        }
        if(HuSin(MBAngleWrap(i)) < 0) {
            tpLvl = -HuSin(MBAngleWrap(i));
        } else {
            tpLvl = HuSin(MBAngleWrap(i));
        }
        espTPLvlSet(fadeSprId, tpLvl);
        HuPrcVSleep();
    }
    espDispOff(fadeSprId);
    MBMasuPosGet(MASU_LAYER_DEFAULT, linkMasuId, &pos);
    MBModelPosSetV(mdlId, &pos);
    MBCameraModelViewSet(mdlId, NULL, &capsuleViewOfs, 1500, -1, 21);
    MBCameraMotionWait();
    MBMasuPosGet(MASU_LAYER_DEFAULT, linkMasuId, &pos);
    MBModelPosSetV(mdlId, &pos);
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_NONE);
    MBMotionTimeSet(mdlId, MBMotionMaxTimeGet(mdlId)*0.5f);
    dustEffF = FALSE;
    do {
        if(!dustEffF && MBMotionTimeGet(mdlId) > 137) {
            MBCapsuleDustHeavyAdd(0, pos);
            MBAudFXPlay(MSM_SE_BOARD_115);
            MBCapsuleVibrate(1);
            dustEffF = TRUE;
        }
        HuPrcVSleep();
    } while(!MBMotionEndCheck(mdlId));
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBEffFadeOutSet(30);
    HuPrcSleep(30);
    while(MBMusJingleStatGet(musNo)) {
        HuPrcVSleep();
    }
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_KOOPA, 127, 0);
    MBAudFXPlay(MSM_SE_GUIDE_12);
    MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KOOPA_MG_START, HUWIN_SPEAKER_KOOPA);
    MBTopWinWait();
    MBTopWinKill();
    MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_LOOP);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KOOPA_MG, HUWIN_SPEAKER_KOOPA);
    MBTopWinWait();
    MBTopWinKill();
    MBMotionShiftSet(mdlId, 5, 0, 8, HU3D_MOTATTR_NONE);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KOOPA_MG_ROULETTE, HUWIN_SPEAKER_KOOPA);
    MBTopWinWait();
    MBTopWinKill();
    GWMgCoinBonusSet(0, 0);
    GWMgCoinBonusSet(1, 0);
    GWMgCoinBonusSet(2, 0);
    GWMgCoinBonusSet(3, 0);
    MBMgCallKupaExec();
    MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_FG)) {
        HuPrcVSleep();
    }
    MBMusMainPlay();
    HuPrcEnd();
}

static void KoopaEndGameExec(int mdlId, BOOL partyF)
{
    int masuId;
    int squishNum;
    
    HuVecF pos;
    HuVecF mdlPos;
    int squishTbl[GW_PLAYER_MAX];
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &pos);
    MBPlayerPosRestoreAll();
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &mdlPos);
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_NONE);
    MBMotionTimeSet(mdlId, 0.5f*MBMotionMaxTimeGet(mdlId));
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    squishNum = 0;
    do {
        if(squishNum == 0 && MBMotionTimeGet(mdlId) > 137) {
            MBCapsuleDustHeavyAdd(0, mdlPos);
            MBAudFXPlay(MSM_SE_BOARD_115);
            MBCapsuleVibrate(1);
            squishNum = MBCapsulePlayerSquishSet(squishTbl, masuId);
        }
        HuPrcVSleep();
    } while(WipeCheckEnd() || ! MBMotionEndCheck(mdlId));
    MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
    MBAudFXPlay(MSM_SE_GUIDE_14);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_STORYCOM_END, HUWIN_SPEAKER_KOOPA);
    MBTopWinWait();
    MBTopWinKill();
    HuPrcSleep(30);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
    HuPrcEnd();
}

static void KoopaMgEndExec(int mdlId, BOOL partyF)
{
    int i; //r31
    int loseNum; //r29
    int j; //r28
    int masuId; //r27
    int resultNo; //r26
    int squishNum; //r25
    int coinNum; //r24
    
    HuVecF pos; //sp+0x34
    HuVecF mdlPos; //sp+0x28
    int squishTbl[GW_PLAYER_MAX];
    int loseTbl[GW_PLAYER_MAX];
    static u32 resultMesTbl[3] = {
        MESS_KOOPAMASU_EVENT_RESULT1,
        MESS_KOOPAMASU_EVENT_RESULT2,
        MESS_KOOPAMASU_EVENT_RESULT3
    };
    
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_KOOPA, 127, 0);
    masuId = GwPlayer[capsulePlayer].masuId;
    
    MBPlayerPosGet(capsulePlayer, &pos);
    MBPlayerPosRestoreAll();
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &pos);
    MBPlayerPosGet(capsulePlayer, &mdlPos);
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBCameraSkipSet(FALSE);
    MBCameraMasuViewSet(masuId, NULL, &capsuleViewOfs, 1500, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    squishNum = MBCapsulePlayerSquishVoiceSet(squishTbl, masuId, TRUE);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    resultNo = MgResultData.resultNo;
    for(loseNum=0, i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i) && GWMgCoinBonusGet(i) <= 0) {
            loseTbl[loseNum++] = i;
        }
    }
    if(loseNum > 0) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_RESULT, HUWIN_SPEAKER_KOOPA);
        MBTopWinInsertMesSet(resultMesTbl[resultNo], 0);
        MBTopWinWait();
        MBTopWinKill();
        MBAudFXPlay(MSM_SE_GUIDE_14);
        MBAudFXPlay(MSM_SE_BOARD_59);
        MBCapsuleModelMotSet(mdlId, 5, HU3D_MOTATTR_NONE, TRUE);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        HuPrcSleep(30);
        switch(resultNo) {
            case 0:
                for(i=0; i<loseNum; i++) {
                    if(MBPlayerCoinGet(loseTbl[i]) > 0) {
                        break;
                    }
                }
                if(i >= loseNum) {
                    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_RESULT_NOCOIN, HUWIN_SPEAKER_KOOPA);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    for(i=0; i<loseNum; i++) {
                        MBPlayerCoinSet(loseTbl[i], 0);
                    }
                    MBAudFXPlay(MSM_SE_BOARD_124);
                }
                break;
            
            case 1:
                for(i=0; i<loseNum; i++) {
                    if(MBPlayerCoinGet(loseTbl[i]) > 0) {
                        break;
                    }
                }
                if(i >= loseNum) {
                    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_RESULT_NOCOIN, HUWIN_SPEAKER_KOOPA);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    for(i=0; i<loseNum; i++) {
                        coinNum = MBPlayerCoinGet(loseTbl[i])/2;
                        MBPlayerCoinAdd(loseTbl[i], -coinNum);
                    }
                    MBAudFXPlay(MSM_SE_BOARD_124);
                }
                break;
            
            default:
                for(i=0; i<loseNum; i++) {
                    if(MBPlayerCapsuleNumGet(loseTbl[i]) > 0) {
                        break;
                    }
                }
                if(i >= loseNum) {
                    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_RESULT_NOCAPSULE, HUWIN_SPEAKER_KOOPA);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    for(i=0; i<loseNum; i++) {
                        for(j=0; j<MBPlayerCapsuleMaxGet(); j++) {
                            MBPlayerCapsuleRemove(loseTbl[i], 0);
                        }
                    }
                    
                }
                break;
        }
        MBAudFXPlay(MSM_SE_GUIDE_12);
        MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
        MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
        MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_END, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        MBMotionShiftSet(mdlId, 6, 0, 8, HU3D_MOTATTR_NONE);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_KOOPAMASU_RESULT_NOLOSE, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_FG)) {
        HuPrcVSleep();
    }
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
    MBCapsuleVibrate(1);
    HuAudFXPlay(MSM_SE_GUIDE_15);
    for(i=0; i<90.0f; i++) {
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBMusMainPlay();
    MBCapsulePlayerStunSet(squishTbl, squishNum, FALSE);
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
}

static void KoopaCircuitMgEndExec(int mdlId, BOOL partyF)
{
    int i; //r31
    MASU *masuP; //r29
    int linkMasuId; //r28
    int loseNum; //r27
    int masuId; //r26
    int saiResult; //r25
    
    HuVecF playerPos; //sp+0x30
    HuVecF mdlPos; //sp+0x24
    HuVecF saiPos; //sp+0x18
    int loseTbl[GW_PLAYER_MAX]; //sp+0x8
    
    static s8 saiTbl[10] = {
        0,
        1,
        2,
        3,
        4,
        5,
        -1,
        -1,
        -1,
        -1
    };
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_KOOPA, 127, 0);
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, MBMasuTargetGet(masuId));
    for(i=0, linkMasuId=MASU_NULL; i<masuP->linkNum; i++) {
        if(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & MASU_FLAG_BLOCKR) {
            linkMasuId = masuP->linkTbl[i];
            break;
        }
    }
    if(linkMasuId == MASU_NULL) {
        linkMasuId = masuId;
    }
    MBCircuitCarInit();
    MBMasuPosGet(MASU_LAYER_DEFAULT, linkMasuId, &mdlPos);
    MBModelPosSetV(mdlId, &mdlPos);
    MBModelDispSet(mdlId, TRUE);
    MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
    MBCameraSkipSet(FALSE);
    MBMasuPosGet(MASU_LAYER_DEFAULT, linkMasuId, &mdlPos);
    MBModelPosSetV(mdlId, &mdlPos);
    MBCameraModelViewSet(mdlId, NULL, &capsuleViewOfs, 1500, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBCapsuleStatusPosMoveWait(TRUE, TRUE);
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    for(loseNum=0, i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i) && GWMgCoinBonusGet(i) <= 0) {
            loseTbl[loseNum++] = i;
        }
    }
    if(loseNum > 0) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KOOPA_MG_RESULT, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
        saiPos.x = mdlPos.x;
        saiPos.y = mdlPos.y += 50;
        saiPos.z = mdlPos.z;
        saiHookTime = (MBCapsuleEffRandF()+0.33f)*60;
        saiHookMdlId = mdlId;
        MBSaiExec(-1, SAITYPE_EVENT, saiTbl, SAI_VALUE_ANY, FALSE, FALSE, &saiPos, SAI_COLOR_RED);
        MBSaiPadBtnHookSet(-1, KoopaLoseSaiPadHook);
        MBSaiMotHookSet(-1, KoopaSaiMotHook);
        while(!MBSaiKillCheck(-1)) {
            HuPrcVSleep();
        }
        saiResult = MBSaiResultGet(-1);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBSaiNumKill(-1);
        for(i=0; i<loseNum; i++) {
            if(MBCircuitGoalCheck(i)) {
                continue;
            }
            MBCameraSkipSet(FALSE);
            MBCameraViewNoSet(loseTbl[i], MB_CAMERA_VIEW_ZOOMIN);
            MBCameraMotionWait();
            MBCameraSkipSet(TRUE);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
            WipeWait();
            CharFXPlay(GwPlayer[loseTbl[i]].charNo, CHARVOICEID(3));
            GwPlayer[loseTbl[i]].walkNum = -saiResult;
            MBCircuitCarMoveExec(loseTbl[i]);
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
            WipeWait();
        }
        MBCameraSkipSet(FALSE);
        MBCameraModelViewSet(mdlId, NULL, &capsuleViewOfs, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
        WipeWait();
        MBAudFXPlay(MSM_SE_GUIDE_12);
        MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KOOPA_MG_END, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
    } else {
        MBMotionShiftSet(mdlId, 9, 0, 8, HU3D_MOTATTR_LOOP);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CIRCUITEVENT_KOOPA_MG_TIE, HUWIN_SPEAKER_KOOPA);
        MBTopWinWait();
        MBTopWinKill();
    }
    MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
    MBCapsuleVibrate(1);
    HuAudFXPlay(MSM_SE_GUIDE_15);
    for(i=0; i<90.0f; i++) {
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_FG)) {
        HuPrcVSleep();
    }
    MBMusMainPlay();
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
    WipeWait();
}

static u16 KoopaLoseSaiPadHook(int playerNo)
{
    if(--saiHookTime <= 0) {
        return PAD_BUTTON_A;
    } else {
        return 0;
    }
}

static void KoopaSaiMotHook(int playerNo)
{
    int time;
    if(saiHookMdlId == MB_MODEL_NONE) {
        return;
    }
    MBMotionNoSet(saiHookMdlId, 11, HU3D_MOTATTR_NONE);
    time = 0;
    do {
        if(time++ == 27) {
            MBSaiObjHit(playerNo);
        }
        HuPrcVSleep();
    } while(!MBMotionEndCheck(saiHookMdlId));
    MBMotionNoSet(saiHookMdlId, 1, HU3D_MOTATTR_LOOP);
}

static void ChanceSaiHitHook(int result);
static void ChanceIconDispSet(int mdlId1, int mdlId2, int speed, BOOL fadeOutF);
static int ChanceSaiExec(int playerNo, int mdlId, int speed, int tblNum, int *tbl);

void MBCapsuleChanceExec(void)
{
    int i; //r31
    int playerNo1; //r30
    int playerNo2; //r29
    int coinNum; //r28
    int charMdlId1; //r27
    int playerNum; //r26
    int starTimer; //r25
    int charMdlId2; //r24
    int resultMdlId; //r17
    
    float scale; //f31
    float t; //f30
    float yOfs; //f29
    float dist; //f28
    float radius; //f27
    
    int rouletteData[32]; //sp+0xF4
    HuVecF pos; //sp+0xE8
    HuVecF daizaPos; //sp+0xDC
    HuVecF playerPos; //sp+0xD0
    HuVecF mdlRot; //sp+0xC4
    HuVecF mdlPos; //sp+0xB8
    int playerNo[GW_PLAYER_MAX]; //sp+0xA8
    int playerCharNo[GW_PLAYER_MAX]; //sp+0x98
    int playerCoin[2]; //sp+0x90
    int playerStar[2]; //sp+0x88
    MBCAMERA *cameraP; //sp+0x84
    int daizaMdlId1; //sp+0x80
    int daizaMdlId2; //sp+0x7C
    int glassMdlId; //sp+0x78
    int guideMdlId; //sp+0x74
    int srcIdx; //sp+0x70
    int dstIdx; //sp+0x6C
    int result; //sp+0x68
    int saiResult; //sp+0x64
    int capsuleCoin; //sp+0x60
    int capsuleStar; //sp+0x5C
    BOOL highF; //sp+0x58
    float newAngle; //sp+0x54
    static int rouletteTbl[16] = {
        0,
        0,
        0,
        1,
        1,
        1,
        2,
        2,
        2,
        3,
        3,
        4,
        4,
        5,
        5,
        6
    };
    static HuVecF sprPosTbl[3] = {
        { 138, 90, 0 },
        { 288, 90, 0 },
        { 438, 90, 0 },
    };
    
    highF = FALSE;
    MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
    MBCapsuleGlowEffCreate();
    HuPrcVSleep();
    MBCapsuleCoinManCreate();
    HuPrcVSleep();
    MBCapsuleStarManCreate();
    HuPrcVSleep();
    MBCapsuleChanceSprCreate();
    HuPrcVSleep();
    yOfs = 300;
    dist = 0;
    radius = 0;
    if(MBMasuFlagGet(MASU_LAYER_DEFAULT, GwPlayer[capsulePlayer].masuId) & MASU_FLAG_RAISECHANCE) {
        yOfs = 800;
        if(MBBoardNoGet() == 0) {
            dist = 100;
        }
        radius = 0;
        highF = TRUE;
    }
    MBPlayerPosGet(capsulePlayer, &playerPos);
    daizaPos = pos = playerPos;
    pos.y += 300;
    daizaMdlId1 = MBCapsuleModelCreate(CAPSULECHAR4_HSF_chance_daiza, NULL, FALSE, 10, FALSE);
    MBModelPosSetV(daizaMdlId1, &playerPos);
    MBModelDispSet(daizaMdlId1, FALSE);
    HuPrcVSleep();
    daizaMdlId2 = MBCapsuleModelCreate(CAPSULECHAR4_HSF_chance_daiza, NULL, TRUE, 10, FALSE);
    MBModelPosSetV(daizaMdlId2, &playerPos);
    MBModelDispSet(daizaMdlId2, FALSE);
    HuPrcVSleep();
    pos.x = 0;
    pos.y = 150;
    pos.z = 0;
    mdlRot.x = -25;
    mdlRot.y = 0;
    mdlRot.z = 0;
    MBCameraModelViewSet(daizaMdlId2, &mdlRot, &pos, 1500, -1, 21);
    MBCameraMotionWait();
    mdlPos.x = playerPos.x;
    mdlPos.y = playerPos.y+200+yOfs;
    mdlPos.z = playerPos.z-75;
    cameraP = MBCameraGet();
    mdlRot = cameraP->rot;
    charMdlId2 = MBCapsuleModelCreate(CAPSULECHAR4_HSF_chance_char, NULL, FALSE, 10, FALSE);
    MBMotionTimeSet(charMdlId2, GwPlayer[capsulePlayer].charNo+1.5f);
    MBMotionSpeedSet(charMdlId2, 0);
    MBModelPosSetV(charMdlId2, &mdlPos);
    MBModelRotSetV(charMdlId2, &mdlRot);
    MBModelScaleSet(charMdlId2, 0.75f, 0.75f, 0.75f);
    MBModelDispSet(charMdlId2, FALSE);
    MBModelLayerSet(charMdlId2, 2);
    resultMdlId = MBCapsuleModelCreate(CAPSULECHAR4_HSF_chance_result, NULL, FALSE, 10, FALSE);
    MBMotionTimeSet(resultMdlId, 1.5f);
    MBMotionSpeedSet(resultMdlId, 0);
    MBModelPosSetV(resultMdlId, &mdlPos);
    MBModelRotSetV(resultMdlId, &mdlRot);
    MBModelScaleSet(resultMdlId, 0.75f, 0.75f, 0.75f);
    MBModelDispSet(resultMdlId, FALSE);
    MBModelLayerSet(resultMdlId, 2);
    charMdlId1 = MBCapsuleModelCreate(CAPSULECHAR4_HSF_chance_char, NULL, FALSE, 10, FALSE);
    MBMotionTimeSet(charMdlId1, 0);
    MBMotionSpeedSet(charMdlId1, 0);
    MBModelPosSetV(charMdlId1, &mdlPos);
    MBModelRotSetV(charMdlId1, &mdlRot);
    MBModelDispSet(charMdlId1, FALSE);
    MBModelLayerSet(charMdlId1, 2);
    glassMdlId = MBCapsuleModelCreate(CAPSULECHAR4_HSF_last5_glass, NULL, FALSE, 10, FALSE);
    MBModelPosSetV(glassMdlId, &mdlPos);
    MBModelRotSetV(glassMdlId, &mdlRot);
    MBModelDispSet(glassMdlId, FALSE);
    MBModelLayerSet(glassMdlId, 2);
    MBAudFXPlay(MSM_SE_BOARD_119);
    MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_CHANCE, 127, 0);
    MBModelDispSet(daizaMdlId1, TRUE);
    for(i=0; i<=90.0f; i++) {
        t = i/90.0f;
        daizaPos.x = playerPos.x+(HuSin(t*90)*125)+(dist*HuSin(t*180));
        daizaPos.y = playerPos.y+(yOfs*HuSin(t*90));
        daizaPos.z = playerPos.z+(HuSin(t*180)*radius);
        MBModelPosSetV(daizaMdlId1, &daizaPos);
        pos = daizaPos;
        pos.x = playerPos.x;
        pos.z = playerPos.z;
        MBModelPosSetV(daizaMdlId2, &pos);
        MBModelRotSet(daizaMdlId1, 0, 720*HuSin(t*90), 0);
        scale = HuSin(t*90)*0.5;
        MBModelScaleSet(daizaMdlId1, scale, scale, scale);
        newAngle = (HuSin(t*90)*720)+90;
        pos = daizaPos;
        MBPlayerPosSetV(capsulePlayer, &pos);
        MBPlayerRotSet(capsulePlayer, 0, 720*HuSin(t*90), 0);
        HuPrcVSleep();
    }
    MBDataAsyncWait(MBDataDirReadAsync(DATA_guide));
    MBPlayerPosGet(capsulePlayer, &pos);
    pos.x -= 250;
    pos.y += 100;
    guideMdlId = MBGuideCreateFlag(MB_GUIDE_CHORL, &pos, FALSE, TRUE, TRUE, FALSE);
    MBModelLayerSet(guideMdlId, 2);
    HuDataDirClose(DATA_guide);
    MBAudFXPlay(MSM_SE_GUIDE_49);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_START, MBGuideSpeakerNoGet());
    MBTopWinWait();
    MBTopWinKill();
    MBAudFXPlay(MSM_SE_BOARD_120);
    MBModelDispSet(charMdlId1, TRUE);
    MBModelDispSet(glassMdlId, TRUE);
    for(i=0; i<60.0f; i++) {
        t = i/60.0f;
        scale = 0.75*HuSin(t*90);
        MBModelScaleSet(charMdlId1, scale, scale, scale);
        MBModelScaleSet(glassMdlId, scale, scale, scale);
        HuPrcVSleep();
    }
    if(capsuleCircuitF) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_P1, MBGuideSpeakerNoGet());
        MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 0);
        MBTopWinWait();
        MBTopWinKill();
        playerNo1 = capsulePlayer;
        MBCapsuleChanceCharAdd(&sprPosTbl[0], GwPlayer[playerNo1].charNo);
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_ROULETTE_P1, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
        for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i)) {
                playerNo[playerNum] = i;
                playerCharNo[playerNum] = GwPlayer[i].charNo;
                playerNum++;
            }
        }
        ChanceIconDispSet(charMdlId1, charMdlId2, 60, TRUE);
        saiResult = ChanceSaiExec(capsulePlayer, charMdlId2, 3, playerNum, playerCharNo);
        playerNo1 = playerNo[saiResult];
        MBCapsuleChanceCharAdd(&sprPosTbl[0], GwPlayer[playerNo1].charNo);
        ChanceIconDispSet(charMdlId2, charMdlId1, 60, FALSE);
    }
    HuPrcSleep(60);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_ROULETTE_P2, MBGuideSpeakerNoGet());
    MBTopWinWait();
    MBTopWinKill();
    for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i) && (i != playerNo1)) {
            playerNo[playerNum] = i;
            playerCharNo[playerNum] = GwPlayer[i].charNo;
            playerNum++;
        }
    }
    MBMotionTimeSet(charMdlId2, playerCharNo[0]+1.5f);
    MBMotionSpeedSet(charMdlId2, 0);
    ChanceIconDispSet(charMdlId1, charMdlId2, 60, TRUE);
    saiResult = ChanceSaiExec(capsulePlayer, charMdlId2, 3, playerNum, playerCharNo);
    playerNo2 = playerNo[saiResult];
    MBCapsuleChanceCharAdd(&sprPosTbl[2], GwPlayer[playerNo2].charNo);
    ChanceIconDispSet(charMdlId2, charMdlId1, 60, FALSE);
    HuPrcSleep(60);
    if(GWTeamFGet() && MBCapsuleTeamCheck(playerNo1, playerNo2)) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_SAMETEAM, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
        MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
        MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINLOSE, HU3D_MOTATTR_NONE, TRUE);
        MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
        MBCapsuleChanceSprRemove();
        MBAudFXPlay(MSM_SE_BOARD_121);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            scale = 0.75*HuCos(t*90);
            MBModelScaleSet(charMdlId1, scale, scale, scale);
            MBModelScaleSet(glassMdlId, scale, scale, scale);
            HuPrcVSleep();
        }
        MBModelDispSet(charMdlId1, FALSE);
        MBModelDispSet(glassMdlId, FALSE);
    } else {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_ROULETTE_RESULT, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
        for(i=0, playerNum=0; i<16u; i++) {
            rouletteData[playerNum] = rouletteTbl[i];
            playerNum++;
        }
        for(i=0; i<64; i++) {
            srcIdx = i%playerNum;
            dstIdx = MBCapsuleEffRand(playerNum);
            if(srcIdx != dstIdx) {
                coinNum = rouletteData[srcIdx];
                rouletteData[srcIdx] = rouletteData[dstIdx];
                rouletteData[dstIdx] = coinNum;
            }
        }
        ChanceIconDispSet(charMdlId1, resultMdlId, 60, TRUE);
        saiResult = ChanceSaiExec(capsulePlayer, resultMdlId, 3, playerNum, rouletteData);
        result = rouletteData[saiResult];
        if(result < 0 || result > 6) {
            result = 0;
            MBMotionTimeSet(resultMdlId, result+1.5f);
        }
        MBCapsuleChanceSprAdd(&sprPosTbl[1], result);
        ChanceIconDispSet(resultMdlId, charMdlId1, 60, FALSE);
        HuPrcSleep(60);
        MBAudFXPlay(MSM_SE_BOARD_121);
        for(i=0; i<60.0f; i++) {
            t = i/60.0f;
            scale = 0.75*HuCos(t*90);
            MBModelScaleSet(charMdlId1, scale, scale, scale);
            MBModelScaleSet(glassMdlId, scale, scale, scale);
            HuPrcVSleep();
        }
        MBModelDispSet(charMdlId1, FALSE);
        MBModelDispSet(glassMdlId, FALSE);
        MBCapsuleChanceSprRotStart();
        MBAudFXPlay(MSM_SE_GUIDE_50);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_START, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
        playerCoin[0] = MBPlayerCoinGet(playerNo1);
        playerStar[0] = MBPlayerStarGet(playerNo1);
        playerCoin[1] = MBPlayerCoinGet(playerNo2);
        playerStar[1] = MBPlayerStarGet(playerNo2);
        capsuleCoin = MBPlayerCoinGet(capsulePlayer);
        capsuleStar = MBPlayerStarGet(capsulePlayer);
        MBCapsuleChanceSprRemove();
        switch(result) {
            case 0:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_TAKE, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 0);
                MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COIN, 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 2);
                MBTopWinWait();
                MBTopWinKill();
                if(playerCoin[1] <= 0) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_EMPTY_TAKE, MBGuideSpeakerNoGet());
                    MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 0);
                    MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COIN, 1);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                    MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                    if(playerCoin[1] > 20) {
                        playerCoin[1] = 20;
                    }
                    do {
                        if(playerCoin[1] > 0) {
                            if(coinNum = MBCapsuleCoinManAdd(TRUE, playerNo1, 1)) {
                                MBPlayerCoinAdd(playerNo2, -coinNum);
                                playerCoin[1] -= coinNum;
                            }
                        }
                        HuPrcVSleep();
                    } while(playerCoin[1] > 0 || MBCapsuleCoinManNumGet() > 0);
                    MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                }
                break;
            
            case 1:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_TAKE, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 0);
                MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COIN, 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 2);
                MBTopWinWait();
                MBTopWinKill();
                if(playerCoin[0] <= 0) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_EMPTY_TAKE, MBGuideSpeakerNoGet());
                    MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 0);
                    MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COIN, 1);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                    MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                    if(playerCoin[0] > 20) {
                        playerCoin[0] = 20;
                    }
                    do {
                        if(playerCoin[0] > 0) {
                            if(coinNum = MBCapsuleCoinManAdd(FALSE, playerNo2, 1)) {
                                MBPlayerCoinAdd(playerNo1, -coinNum);
                                playerCoin[0] -= coinNum;
                            }
                        }
                        HuPrcVSleep();
                    } while(playerCoin[0] > 0 || MBCapsuleCoinManNumGet() > 0);
                    MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                }
                break;
            
            case 2:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_SWAP, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 1);
                MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COIN, 2);
                MBTopWinWait();
                MBTopWinKill();
                if(playerCoin[0] > 0 || playerCoin[1] > 0) {
                    MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                    MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                    do {
                        if(playerCoin[0] > 0) {
                            if(coinNum = MBCapsuleCoinManAdd(FALSE, playerNo2, 1)) {
                                MBPlayerCoinAdd(playerNo1, -coinNum);
                                playerCoin[0] -= coinNum;
                            }
                        }
                        if(playerCoin[1] > 0) {
                            if(coinNum = MBCapsuleCoinManAdd(TRUE, playerNo1, 1)) {
                                MBPlayerCoinAdd(playerNo2, -coinNum);
                                playerCoin[1] -= coinNum;
                            }
                        }
                        HuPrcVSleep();
                    } while(playerCoin[0] > 0 || playerCoin[1] > 0 || MBCapsuleCoinManNumGet() > 0);
                    MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                } else {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_EMPTY_SWAP, MBGuideSpeakerNoGet());
                    MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COIN, 0);
                    MBTopWinWait();
                    MBTopWinKill();
                }
                break;
            
            case 3:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_TAKE, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 0);
                MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_STAR, 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 2);
                MBTopWinWait();
                MBTopWinKill();
                if(playerStar[1] <= 0) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_EMPTY_TAKE, MBGuideSpeakerNoGet());
                    MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 0);
                    MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_STAR, 1);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                    MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                    if(playerStar[1] > 1) {
                        playerStar[1] = 1;
                    }
                    do {
                        if(playerStar[1] > 0) {
                            if(coinNum = MBCapsuleStarManAdd(TRUE, playerNo1, 1)) {
                                MBPlayerStarAdd(playerNo2, -coinNum);
                                playerStar[1] -= coinNum;
                            }
                        }
                        HuPrcVSleep();
                    } while(playerStar[1] > 0 || MBCapsuleStarManNumGet() > 0);
                    MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                }
                break;
            
            case 4:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_TAKE, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 0);
                MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_STAR, 1);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 2);
                MBTopWinWait();
                MBTopWinKill();
                if(playerStar[1] <= 0) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_EMPTY_TAKE, MBGuideSpeakerNoGet());
                    MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 0);
                    MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_STAR, 1);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                    MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                    if(playerStar[1] > 2) {
                        playerStar[1] = 2;
                    }
                    starTimer = 0;
                    do {
                        if(playerStar[1] > 0 && starTimer == 0) {
                            if(coinNum = MBCapsuleStarManAdd(TRUE, playerNo1, 1)) {
                                MBPlayerStarAdd(playerNo2, -coinNum);
                                playerStar[1] -= coinNum;
                            }
                        }
                        if(++starTimer >= 30.0f) {
                            starTimer = 0;
                        }
                        HuPrcVSleep();
                    } while(playerStar[1] > 0 || MBCapsuleStarManNumGet() > 0);
                    MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                }
                break;
                
            case 5:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_SWAP, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 1);
                MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_STAR, 2);
                MBTopWinWait();
                MBTopWinKill();
                if(playerStar[0] > 0 || playerStar[1] > 0) {
                    MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                    MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                    starTimer = 0;
                    do {
                        if(playerStar[0] > 0 && starTimer == 0) {
                            if(coinNum = MBCapsuleStarManAdd(FALSE, playerNo2, 1)) {
                                MBPlayerStarAdd(playerNo1, -coinNum);
                                playerStar[0] -= coinNum;
                            }
                        }
                        if(playerStar[1] > 0 && starTimer == 0) {
                            if(coinNum = MBCapsuleStarManAdd(TRUE, playerNo1, 1)) {
                                MBPlayerStarAdd(playerNo2, -coinNum);
                                playerStar[1] -= coinNum;
                            }
                        }
                        if(++starTimer >= 30.0f) {
                            starTimer = 0;
                        }
                        HuPrcVSleep();
                    } while(playerStar[0] > 0 || playerStar[1] > 0 || MBCapsuleStarManNumGet() > 0);
                    MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                } else {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_EMPTY_SWAP, MBGuideSpeakerNoGet());
                    MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_STAR, 0);
                    MBTopWinWait();
                    MBTopWinKill();
                }
                break;
            
            case 6:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_SWAP, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 1);
                MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COINSTAR, 2);
                MBTopWinWait();
                MBTopWinKill();
                if(playerCoin[0] <= 0 && playerCoin[1] <= 0 && playerStar[0] <= 0 && playerStar[1] <= 0) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_RESULT_EMPTY_SWAP, MBGuideSpeakerNoGet());
                    MBTopWinInsertMesSet(MESS_CAPSULE_EX04_CHANCE_COINSTAR, 0);
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    if(playerCoin[0] > 0 || playerCoin[1] > 0) {
                        MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                        MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                        do {
                            if(playerCoin[0] > 0) {
                                if(coinNum = MBCapsuleCoinManAdd(FALSE, playerNo2, 1)) {
                                    MBPlayerCoinAdd(playerNo1, -coinNum);
                                    playerCoin[0] -= coinNum;
                                }
                            }
                            if(playerCoin[1] > 0) {
                                if(coinNum = MBCapsuleCoinManAdd(TRUE, playerNo1, 1)) {
                                    MBPlayerCoinAdd(playerNo2, -coinNum);
                                    playerCoin[1] -= coinNum;
                                }
                            }
                            HuPrcVSleep();
                        } while(playerCoin[0] > 0 || playerCoin[1] > 0 || MBCapsuleCoinManNumGet() > 0);
                        MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                    }
                    if(playerStar[0] > 0 || playerStar[1] > 0) {
                        MBCapsuleStatusTradeInSet(playerNo1, playerNo2, TRUE);
                        starTimer = 0;
                        do {
                            if(playerStar[0] > 0 && starTimer == 0) {
                                if(coinNum = MBCapsuleStarManAdd(FALSE, playerNo2, 1)) {
                                    MBPlayerStarAdd(playerNo1, -coinNum);
                                    playerStar[0] -= coinNum;
                                }
                            }
                            if(playerStar[1] > 0 && starTimer == 0) {
                                if(coinNum = MBCapsuleStarManAdd(TRUE, playerNo1, 1)) {
                                    MBPlayerStarAdd(playerNo2, -coinNum);
                                    playerStar[1] -= coinNum;
                                }
                            }
                            if(++starTimer >= 30.0f) {
                                starTimer = 0;
                            }
                            HuPrcVSleep();
                        } while(playerStar[0] > 0 || playerStar[1] > 0 || MBCapsuleStarManNumGet() > 0);
                        MBCapsuleStatusTradeOutSet(playerNo1, playerNo2, TRUE);
                    }
                }
                break;
        }
        if(capsuleStar < MBPlayerStarGet(capsulePlayer)) {
            MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_WIN, CHARVOICEID(0));
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_WIN, HU3D_MOTATTR_NONE, TRUE);
        } else if(capsuleStar > MBPlayerStarGet(capsulePlayer)) {
            MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_LOSE, CHARVOICEID(10));
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_LOSE, HU3D_MOTATTR_NONE, TRUE);
        } else if(capsuleCoin < MBPlayerCoinGet(capsulePlayer)) {
            MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINWIN, HU3D_MOTATTR_NONE, TRUE);
        } else if(capsuleCoin > MBPlayerCoinGet(capsulePlayer)) {
            MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINLOSE, HU3D_MOTATTR_NONE, TRUE);
        }
        MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
    }
    MBAudFXPlay(MSM_SE_GUIDE_49);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_CHANCE_END, MBGuideSpeakerNoGet());
    MBTopWinWait();
    MBTopWinKill();
    MBGuideEnd(guideMdlId);
    MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
    while(MBMusCheck(MB_MUS_CHAN_FG)) {
        HuPrcVSleep();
    }
    MBMusMainPlay();
    for(i=0; i<=60.0f; i++) {
        t = i/60.0f;
        daizaPos.x = playerPos.x+(HuSin((1-t)*90)*125)+(dist*HuSin(t*180));
        daizaPos.y = playerPos.y+(yOfs*HuSin((1-t)*90));
        daizaPos.z = playerPos.z+(HuSin(t*180)*radius);
        MBModelPosSetV(daizaMdlId1, &daizaPos);
        pos = daizaPos;
        pos.x = playerPos.x;
        pos.z = playerPos.z;
        MBModelPosSetV(daizaMdlId2, &pos);
        MBModelRotSet(daizaMdlId1, 0, 720*t, 0);
        scale = HuCos(t*90)*0.5;
        MBModelScaleSet(daizaMdlId1, scale, scale, scale);
        pos = daizaPos;
        MBPlayerPosSetV(capsulePlayer, &pos);
        MBPlayerRotSet(capsulePlayer, 0, 720*t, 0);
        HuPrcVSleep();
    }
    MBModelDispSet(daizaMdlId1, FALSE);
    MBCapsuleCameraViewPlayerSet(capsulePlayer);
    MBCameraMotionWait();
    HuPrcEnd();
}

static void ChanceSaiHitHook(int result)
{
    saiHookTime = 0;
}

static void ChanceIconDispSet(int mdlId1, int mdlId2, int speed, BOOL fadeOutF)
{
    int i;
    float t;
    if(fadeOutF) {
        MBModelDispSet(mdlId1, TRUE);
        MBModelLayerSet(mdlId1, 2);
        MBModelDispSet(mdlId2, TRUE);
        MBModelAlphaSet(mdlId2, 255);
        MBModelLayerSet(mdlId2, 2);
        for(i=0; i<speed; i++) {
            t = (float)i/(float)speed;
            MBModelAlphaSet(mdlId2, t*255);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId1, FALSE);
        MBModelDispSet(mdlId2, TRUE);
        MBModelAlphaSet(mdlId2, 255);
        MBModelLayerSet(mdlId2, 2);
    } else {
        MBModelDispSet(mdlId1, TRUE);
        MBModelAlphaSet(mdlId1, 255);
        MBModelLayerSet(mdlId1, 2);
        MBModelDispSet(mdlId2, TRUE);
        MBModelLayerSet(mdlId2, 2);
        for(i=0; i<speed; i++) {
            t = (float)i/(float)speed;
            MBModelAlphaSet(mdlId1, (1-t)*255);
            HuPrcVSleep();
        }
        MBModelDispSet(mdlId1, FALSE);
        MBModelDispSet(mdlId2, TRUE);
        MBModelLayerSet(mdlId2, 2);
    }
}

static int ChanceSaiExec(int playerNo, int mdlId, int speed, int tblNum, int *tbl)
{
    int result;
    int time;
    int winId;
    winId = MBWinCreateHelp(MESS_BOARDOPE_PAD_SAI);
    MBSaiExec(playerNo, SAITYPE_BLOCK, NULL, SAI_VALUE_ANY, FALSE, FALSE, NULL, SAI_COLOR_GREEN);
    saiHookTime = 1;
    MBSaiHitHookSet(playerNo, ChanceSaiHitHook);
    result = MBCapsuleEffRand(tblNum);
    time = 0;
    do {
        if(saiHookTime) {
            if(++time >= speed) {
                time = 0;
                if(++result >= tblNum) {
                    result = 0;
                }
            }
            MBMotionTimeSet(mdlId, tbl[result]+1.5f);
        }
        HuPrcVSleep();
    } while(!MBSaiKillCheck(playerNo));
    if(winId != MBWIN_NONE) {
        MBWinKill(winId);
    }
    return result;
}

void MBCapsuleChanceDestroy(void)
{
    MBCapsuleGlowEffKill();
    MBCapsuleCoinManKill();
    MBCapsuleStarManKill();
    MBCapsuleChanceSprKill();
}

typedef struct MiracleCapsule_s {
    int mdlId;
    int capsuleNo;
    int no;
    float angle;
    Mtx matrix;
    HuVecF pos;
    HuVecF rot;
} MIRACLE_CAPSULE;

void MBCapsuleMiracleExec(void)
{
    MIRACLE_CAPSULE *capsuleP; //r31
    int i; //r29
    int playerNo1; //r28
    int playerNo2; //r27
    OMOBJ *obj; //r24
    int playerNum; //r23
    int starDelay; //r20
    int j; //r19
    int starNum; //r18
    int starChange; //r17
    
    float t; //f31
    float yOfs; //f30
    float radius; //f29
    float capsuleScale; //f28
    float rotSpeed; //f27
    float rotYSpeed; //f26
    float rotYSpeedOrig; //f25
    
    MIRACLE_CAPSULE capsule[3]; //sp+0x1B4
    int motFile[16]; //sp+0x174
    HuVecF playerPos; //sp+0x168
    HuVecF pos; //sp+0x15C
    HuVecF capsuleOfs; //sp+0x150
    HuVecF guidePos; //sp+0x144
    int playerNo[GW_PLAYER_MAX]; //sp+0x134
    HuVecF rayRot; //sp+0x128
    HuVecF rayDir; //sp+0x11C
    int maxRank; //sp+0xA0
    int guideMdlId; //sp+0x9C
    int *objWork; //sp+0x98
    int loseMotId; //sp+0x94
    int playerMdlId; //sp+0x90
    int charNo; //sp+0x8C
    int otherStarNum; //sp+0x88
    int seNo3; //sp+0x84
    int seNo2; //sp+0x80
    int seNo1; //sp+0x7C
    MBCAMERA *cameraP; //sp+0x78
    GXColor color; //sp+0x74
    
    MBCapsuleGlowEffCreate();
    MBCapsuleRingEffCreate();
    MBCapsuleRayEffCreate();
    MBCapsuleStarManCreate();
    
    MBPlayerPosGet(capsulePlayer, &playerPos);
    HuDataDirClose(DATA_capsule);
    HuPrcVSleep();
    charNo = GwPlayer[capsulePlayer].charNo;
    motFile[0] = charMotIdleFileTbl[charNo];
    motFile[1] = charMotWinFileTbl[charNo];
    motFile[2] = HU_DATANUM_NONE;
    playerMdlId = MBCapsuleModelCreate(charMdlFileTbl[charNo], motFile, FALSE, 10, TRUE);
    MBModelDispSet(playerMdlId, FALSE);
    HuPrcVSleep();
    HuPrcVSleep();
    HuDataDirClose(charMdlFileTbl[charNo]);
    HuDataDirClose(charMotIdleFileTbl[charNo]);
    HuPrcVSleep();
    MBDataAsyncWait(MBDataDirReadAsync(DATA_capsule));
    for(i=0, capsuleP=&capsule[0]; i<3; i++, capsuleP++) {
        capsuleP->mdlId = MBCapsuleModelCreate(MBCapsuleMdlGet(CAPSULE_MIRACLE), NULL, FALSE, 10, FALSE);
        MBModelAttrSet(capsuleP->mdlId, HU3D_MOTATTR_LOOP);
        MBModelScaleSet(capsuleP->mdlId, 0.75f, 0.75f, 0.75f);
        MBModelLayerSet(capsuleP->mdlId, 2);
        MBModelDispSet(capsuleP->mdlId, FALSE);
        capsuleP->no = i;
        capsuleP->angle = i*120.0f;
        capsuleP->pos.x = capsuleP->pos.y = capsuleP->pos.z = 0;
        capsuleP->rot.x = capsuleP->rot.y = capsuleP->rot.z = 0;
        MTXIdentity(capsuleP->matrix);
    }
    for(i=0; i<MBPlayerCapsuleMaxGet(); i++) {
        if(MBPlayerCapsuleGet(capsulePlayer, 0) == CAPSULE_MIRACLE) {
            MBPlayerCapsuleRemove(capsulePlayer, 0);
        }
    }
    seNo3 = MBAudFXPlay(MSM_SE_BOARD_87);
    for(j=0; j<120.0f; j++) {
        t = j/120.0f;
        yOfs = 300*HuSin(t*90);
        radius = 150*HuSin(t*90);
        if(t < 0.5f) {
            capsuleScale = 0.75*HuSin((t*2)*90);
        } else {
            capsuleScale = 0.75f;
        }
        pos = playerPos;
        pos.y += yOfs;
        for(i=0, capsuleP=&capsule[0]; i<3; i++, capsuleP++) {
            if((capsuleP->angle += 2.5f) > 360) {
                capsuleP->angle -= 360;
            }
            capsuleP->pos.x = pos.x+(radius*HuSin(capsuleP->angle));
            capsuleP->pos.y = pos.y;
            capsuleP->pos.z = pos.z+(radius*HuCos(capsuleP->angle));
            MBModelPosSet(capsuleP->mdlId, capsuleP->pos.x, capsuleP->pos.y, capsuleP->pos.z);
            MBModelScaleSet(capsuleP->mdlId, capsuleScale, capsuleScale, capsuleScale);
            MBModelDispSet(capsuleP->mdlId, TRUE);
        }
        HuPrcVSleep();
    }
    rotSpeed = rotYSpeed = rotYSpeedOrig = 0;
    yOfs = 300;
    radius = 150;
    seNo2 = MBAudFXPlay(MSM_SE_BOARD_88);
    for(j=0; j<90.0f; j++) {
        t = j/90.0f;
        radius = 150*HuSin((1-t)*90);
        pos = playerPos;
        pos.y += yOfs;
        if(yOfs > 150) {
            yOfs += 0.02f*(150.0f-yOfs);
        }
        rotSpeed = 10*HuSin(t*90);
        rotYSpeed = 10*HuSin(t*90);
        rotYSpeedOrig = 10*HuSin(t*90);
        for(i=0, capsuleP=&capsule[0]; i<3; i++, capsuleP++) {
            static float rotTbl[3] = {
                45,
                -45,
                0
            };
            MTXIdentity(capsuleP->matrix);
            mtxRotCat(capsuleP->matrix, 0, capsuleP->rot.y, 0);
            mtxRotCat(capsuleP->matrix, capsuleP->rot.x, 0, capsuleP->rot.z);
            capsuleP->rot.y += rotYSpeed;
            capsuleP->rot.x = MBCapsuleAngleLerp(rotTbl[i], capsuleP->rot.x, rotSpeed);
            if((capsuleP->angle += 2.5f) > 360) {
                capsuleP->angle -= 360;
            }
            capsuleOfs.x = radius*HuSin(capsuleP->angle);
            capsuleOfs.y = 0;
            capsuleOfs.z = radius*HuCos(capsuleP->angle);
            MTXMultVec(capsuleP->matrix, &capsuleOfs, &capsuleOfs);
            VECAdd(&pos, &capsuleOfs, &capsuleP->pos);
            MBModelPosSet(capsuleP->mdlId, capsuleP->pos.x, capsuleP->pos.y, capsuleP->pos.z);
            MBCapsuleMiracleGlowEffAdd(capsuleP->pos, ((MBCapsuleEffRandF()*0.05f)+0.15f)*100, 0.016666668f, 100, 100, 50, 2);
            
        }
        HuPrcVSleep();
    }
    for(i=0, capsuleP=&capsule[0]; i<3; i++, capsuleP++) {
        MBModelDispSet(capsuleP->mdlId, FALSE);
    }
    if(seNo3 != MSM_SENO_NONE) {
        MBAudFXStop(seNo3);
    }
    if(seNo2 != MSM_SENO_NONE) {
        MBAudFXStop(seNo2);
    }
    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 1);
    WipeWait();
    obj = capsuleObj = MBAddObj(32768, 0, 0, MBCapsuleMiracleGlowExec);
    objWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(int), HU_MEMNUM_OVL);
    *objWork = playerMdlId;
    obj->work[0] = capsulePlayer;
    obj->work[1] = 0;
    seNo1 = MBAudFXPlay(MSM_SE_BOARD_89);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    for(i=0; i<2; i++) {
        static HuVecF rotTbl[2] = {
            { -90, 0, 35 },
            { -90, 0, -33 },
        };
        static HuVecF dirTbl[2] = {
            { 1.5f, 3.0f, 175 },
            { 1.5f, 3.0f, 150 },
        };
        static GXColor colTbl[2] = {
            { 255, 0, 255, 255 },
            { 255, 128, 192, 255 },
        };
        MBCapsuleRingEffAdd(pos, rotTbl[i], dirTbl[i], 9, 27, (i+1)%3, colTbl[i]);
    }
    for(i=0; i<3; i++) {
        static HuVecF dirTbl[3] = {
            { 1.2f, 2.0f, 300 },
            { 1.2f, 2.0f, 500 },
            { 1.2f, 2.0f, 700 },
        };
        static GXColor colTbl[3] = {
            { 255, 255, 64, 128 },
            { 255, 0, 255, 128 },
            { 255, 128, 255, 128 },
        };
        cameraP = MBCameraGet();
        capsuleOfs.x = cameraP->rot.x;
        capsuleOfs.y = cameraP->rot.y;
        capsuleOfs.z = cameraP->rot.z;
        MBCapsuleRingEffAdd(pos, capsuleOfs, dirTbl[i], 12, 15, i%3, colTbl[i]);
    }
    for(i=0; i<64; i++) {
        rayRot.x = 0;
        rayRot.y = 0;
        rayRot.z = (i*5.625f)+((0.5f-MBCapsuleEffRandF())*10);
        rayDir.x = 20;
        rayDir.y = ((MBCapsuleEffRandF()*0.5f)+0.7f)*500;
        rayDir.z = ((MBCapsuleEffRandF()*0.4f)+0.8f)*80;
        color.r = (MBCapsuleEffRandF()*64)+128;
        color.g = (MBCapsuleEffRandF()*64)+128;
        color.b = (MBCapsuleEffRandF()*64)+128;
        color.a = (MBCapsuleEffRandF()*64)+128;
        MBCapsuleRayEffAdd(pos, rayRot, rayDir, ((MBCapsuleEffRandF()*0.3f)+0.8f)*30, color);
    }
    do {
        HuPrcVSleep();
    } while(MBCapsuleRayEffNumGet() > 0 && MBCapsuleRingEffNumGet() > 0);
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    MBDataAsyncWait(MBDataDirReadAsync(DATA_guide));
    MBPlayerPosGet(capsulePlayer, &guidePos);
    guidePos.y += 300;
    guideMdlId = MBGuideCreateFlag(MB_GUIDE_CHORL, &guidePos, FALSE, TRUE, TRUE, FALSE);
    MBModelLayerSet(guideMdlId, 2);
    HuDataDirClose(DATA_guide);
    MBAudFXPlay(MSM_SE_GUIDE_49);
    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_MIRACLE_3, HUWIN_SPEAKER_CHORL);
    MBTopWinWait();
    MBTopWinKill();
    if(!GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i) && MBPlayerStarGet(i) > 0) {
                break;
            }
        }
        if(i >= GW_PLAYER_MAX) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_MIRACLE_STAR_NONE, HUWIN_SPEAKER_CHORL);
            MBTopWinWait();
            MBTopWinKill();
        } else {
            if(GwPlayer[capsulePlayer].rank == 0) {
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_MIRACLE_STAR_SWAP_CANCEL, HUWIN_SPEAKER_CHORL);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 0);
                MBTopWinWait();
                MBTopWinKill();
            } else {
                for(i=0, maxRank=-1; i<GW_PLAYER_MAX; i++) {
                    if(MBPlayerAliveCheck(i) && GwPlayer[i].rank > maxRank) {
                        maxRank = GwPlayer[i].rank;
                    }
                }
                for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
                    if(MBPlayerAliveCheck(i) && GwPlayer[i].rank == maxRank) {
                        playerNo[playerNum] = i;
                        playerNum++;
                    }
                }
                if(playerNum > 0) {
                    playerNo2 = playerNo[MBCapsuleEffRand(playerNum)];
                } else {
                    playerNo2 = 0;
                }
                for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
                    if(MBPlayerAliveCheck(i) && i != playerNo2 && GwPlayer[i].rank == 0) {
                        playerNo[playerNum] = i;
                        playerNum++;
                    }
                }
                if(playerNum > 0) {
                    playerNo1 = playerNo[MBCapsuleEffRand(playerNum)];
                } else {
                    playerNo1 = GW_PLAYER_MAX-1;
                }
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_MIRACLE_STAR_FIRST_LAST, HUWIN_SPEAKER_CHORL);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo1), 0);
                MBTopWinInsertMesSet(MBPlayerNameMesGet(playerNo2), 1);
                MBTopWinWait();
                MBTopWinKill();
                MBCapsuleStatusPosMoveWait(FALSE, TRUE);
                MBCapsuleStatusTradeInSet(playerNo2, playerNo1, TRUE);
                starNum = MBPlayerStarGet(playerNo1);
                starDelay = 0;
                do {
                    if(starNum > 0 && starDelay == 0) {
                        if(starChange = MBCapsuleStarManAdd(TRUE, playerNo2, 1)) {
                            MBPlayerStarAdd(playerNo1, -starChange);
                            starNum -= starChange;
                        }
                    }
                    if(++starDelay >= 30.0f) {
                        starDelay = 0;
                    }
                    HuPrcVSleep();
                } while(starNum > 0 || MBCapsuleStarManNumGet() > 0);
                MBCapsuleStatusTradeOutSet(playerNo2, playerNo1, TRUE);
                MBPlayerWinLoseVoicePlay(playerNo2, MB_PLAYER_MOT_WIN, CHARVOICEID(0));
                MBPlayerMotionNoShiftSet(playerNo2, MB_PLAYER_MOT_WIN, 0, 8, HU3D_MOTATTR_NONE);
                if(playerNo2 == capsulePlayer) {
                    MBMotionShiftSet(playerMdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
                }
                loseMotId = MBCapsulePlayerMotionCreate(playerNo1, CHARMOT_HSF_c000m1_360);
                MBPlayerWinLoseVoicePlay(playerNo1, loseMotId, CHARVOICEID(10));
                MBPlayerMotionNoShiftSet(playerNo1, loseMotId, 0, 8, HU3D_MOTATTR_NONE);
                do {
                    HuPrcVSleep();
                } while(MBMotionShiftIDGet(MBPlayerModelGet(playerNo2)) != HU3D_MOTID_NONE ||
                    !MBPlayerMotionEndCheck(playerNo2) ||
                    MBMotionShiftIDGet(MBPlayerModelGet(playerNo1)) != HU3D_MOTID_NONE ||
                    !MBPlayerMotionEndCheck(playerNo1));
            }
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i) && MBPlayerStarGet(i) > 0) {
                break;
            }
        }
        if(i >= GW_PLAYER_MAX) {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_MIRACLE_STAR_NONE, HUWIN_SPEAKER_CHORL);
            MBTopWinWait();
            MBTopWinKill();
        } else {
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_MIRACLE_STAR_FIRST_LAST_END, HUWIN_SPEAKER_CHORL);
            MBTopWinWait();
            MBTopWinKill();
            playerNo2 = MBPlayerGrpFindPlayer(0, 0);
            playerNo1 = MBPlayerGrpFindPlayer(1, 0);
            MBCapsuleStatusPosMoveWait(FALSE, TRUE);
            MBCapsuleStatusTradeInSet(playerNo2, playerNo1, TRUE);
            starNum = MBPlayerStarGet(playerNo2);
            otherStarNum = MBPlayerStarGet(playerNo1);
            starDelay = 0;
            do {
                if(starNum > 0 && starDelay == 0) {
                    if(starChange = MBCapsuleStarManAdd(FALSE, playerNo1, 1)) {
                        MBPlayerStarAdd(playerNo2, -starChange);
                        starNum -= starChange;
                    }
                }
                if(otherStarNum > 0 && starDelay == 0) {
                    if(starChange = MBCapsuleStarManAdd(TRUE, playerNo2, 1)) {
                        MBPlayerStarAdd(playerNo1, -starChange);
                        otherStarNum -= starChange;
                    }
                }
                if(++starDelay >= 30.0f) {
                    starDelay = 0;
                }
                HuPrcVSleep();
            } while(starNum > 0 || otherStarNum > 0 || MBCapsuleStarManNumGet() > 0);
            MBCapsuleStatusTradeOutSet(playerNo2, playerNo1, TRUE);
            MBPlayerWinLoseVoicePlay(playerNo2, MB_PLAYER_MOT_WIN, CHARVOICEID(0));
            MBPlayerMotionNoShiftSet(playerNo2, MB_PLAYER_MOT_WIN, 0, 8, HU3D_MOTATTR_NONE);
            if(playerNo2 == capsulePlayer) {
                MBMotionShiftSet(playerMdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
            }
            loseMotId = MBCapsulePlayerMotionCreate(playerNo1, CHARMOT_HSF_c000m1_360);
            MBPlayerWinLoseVoicePlay(playerNo1, loseMotId, CHARVOICEID(10));
            MBPlayerMotionNoShiftSet(playerNo1, loseMotId, 0, 8, HU3D_MOTATTR_NONE);
            do {
                HuPrcVSleep();
            } while(MBMotionShiftIDGet(MBPlayerModelGet(playerNo2)) != HU3D_MOTID_NONE ||
                !MBPlayerMotionEndCheck(playerNo2) ||
                MBMotionShiftIDGet(MBPlayerModelGet(playerNo1)) != HU3D_MOTID_NONE ||
                !MBPlayerMotionEndCheck(playerNo1));
        }
    }
    for(i=0; i<60.0f; i++) {
        HuPrcVSleep();
    }
    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
    if(!MBKillCheck()) {
        MBMotionShiftSet(playerMdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            MBPlayerMotionNoShiftSet(i, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
        }
    }
    MBGuideEnd(guideMdlId);
    if(!MBKillCheck()) {
        obj->work[1]++;
        obj->work[3] = 0;
        if(seNo1 != MSM_SENO_NONE) {
            MBAudFXStop(seNo1);
        }
        do {
            HuPrcVSleep();
        } while(!MBCapsuleMiracleGlowCheck(obj));
        obj->work[1]++;
    }
    HuPrcEnd();
}

void MBCapsuleMiracleDestroy(void)
{
    MBCapsuleGlowEffKill();
    MBCapsuleRingEffKill();
    MBCapsuleRayEffKill();
    MBCapsuleStarManKill();
}

static u16 DonkeySaiPadHook(int playerNo);
static void DonkeySaiMotHook(int playerNo);

void MBCapsuleDonkeyExec(void)
{
    int mdlId; //r31
    int i; //r30
    int sprNo; //r29
    int treeMdlId; //r28
    int coinNum; //r21
    int hammerMdlId; //r20
    
    float t; //f31
    float scale; //f30
    float bananaNum; //f29
    float effRotY; //f28
    float bananaNumSpeed; //f27
    float effRotX; //f26
    
    Mtx hookMtx; //sp+0x138
    int motFile[8]; //sp+0x118
    HuVecF playerPos; //sp+0x10C
    HuVecF mdlPos; //sp+0x100
    HuVecF viewRot; //sp+0xF4
    HuVecF mdlVel; //sp+0xE8
    HuVecF donkeyPos; //sp+0xDC
    HuVecF tempVec; //sp+0xD0
    HuVecF effPos; //sp+0xC4
    HuVecF effRot; //sp+0xB8
    int bonusTbl[GW_PLAYER_MAX]; //sp+0xA8
    char insertMes[16]; //sp+0x98
    int sprId[2]; //sp+0x90
    int masuId; //sp+0x8C
    int prevSprNo; //sp+0x88
    int donkeyMasuId; //sp+0x84
    int motId; //sp+0x80
    int eventNo; //sp+0x7C
    int bananaTickNum; //sp+0x78
    int mgCoinNum; //sp+0x74
    int saiDonkeyResult; //sp+0x70
    int coinEffNum; //sp+0x6C
    static int eventFileTbl[3] = {
        CAPSULE_ANM_donkeyEvent1,
        CAPSULE_ANM_donkeyEvent2,
        CAPSULE_ANM_donkeyEvent3
    };
    static s8 saiTbl[10] = {
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        -1
    };
    t = 0;
    MBCapsuleGlowEffCreate();
    HuPrcVSleep();
    MBCapsuleCoinEffCreate();
    HuPrcVSleep();
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    motFile[0] = CAPSULECHAR3_HSF_donkey_idle;
    motFile[1] = CAPSULECHAR3_HSF_donkey_roll;
    motFile[2] = CAPSULECHAR3_HSF_donkey_win;
    motFile[3] = CAPSULECHAR3_HSF_donkey_lose;
    motFile[4] = CAPSULECHAR3_HSF_donkey_sai_fall;
    motFile[5] = CAPSULECHAR3_HSF_donkey_sai_hit;
    motFile[6] = CAPSULECHAR3_HSF_donkey_anger;
    motFile[7] = CAPSULECHAR3_HSF_donkey_punch_hard;
    motFile[8] = CAPSULECHAR3_HSF_donkey_walk;
    motFile[9] = HU_DATANUM_NONE;
    mdlId = MBCapsuleModelCreate(CAPSULECHAR3_HSF_donkey, motFile, FALSE, 10, FALSE);
    MBModelDispSet(mdlId, FALSE);
    treeMdlId = MBCapsuleModelCreate(CAPSULE_HSF_donkeyTree, NULL, FALSE, 10, FALSE);
    MBModelDispSet(treeMdlId, FALSE);
    MBModelLayerSet(treeMdlId, 2);
    motId = MBCapsulePlayerMotionCreate(capsulePlayer, CHARMOT_HSF_c000m1_325);
    if(capsuleFlags.donkeyOpeningF) {
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
        donkeyMasuId = MBMasuDonkeySet(GwSystem.mgEvent);
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        MBCameraSkipSet(FALSE);
        viewRot.x = -33;
        viewRot.y = 0;
        viewRot.z = 0;
        tempVec.x = 0;
        tempVec.y = 250;
        tempVec.z = 0;
        MBCameraModelViewSet(MB_MODEL_NONE, &viewRot, &tempVec, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraFocusMasuSet(donkeyMasuId);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        MBMasuTypeSet(MASU_LAYER_DEFAULT, donkeyMasuId, MASU_TYPE_DONKEY);
        MBMasuCapsuleSet(MASU_LAYER_DEFAULT, donkeyMasuId, CAPSULE_DONKEY);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
        WipeWait();
        MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_LOOP);
        MBMasuPosGet(MASU_LAYER_DEFAULT, donkeyMasuId, &mdlPos);
        donkeyPos = mdlPos;
        scale = 0;
        mdlVel.x = 0;
        mdlVel.y = 40;
        mdlVel.z = 0;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelScaleSet(mdlId, 0, 0, 0);
        MBModelDispSet(mdlId, TRUE);
        MBAudFXPlay(MSM_SE_BOARD_106);
        do {
            HuPrcVSleep();
            VECAdd(&mdlPos, &mdlVel, &mdlPos);
            mdlVel.y -= 2.2866669f;
            if(mdlPos.y < donkeyPos.y) {
                mdlPos.y = donkeyPos.y;
            }
            if(scale < 1) {
                scale += 0.05f;
            }
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelScaleSet(mdlId, scale, scale, scale);
        } while(mdlPos.y > donkeyPos.y || scale < 1);
        MBModelPosSetV(mdlId, &donkeyPos);
        MBAudFXPlay(MSM_SE_GUIDE_30);
        MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
        HuPrcSleep(30);
        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_MASU, MBGuideSpeakerNoGet());
        MBTopWinWait();
        MBTopWinKill();
        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        WipeWait();
        for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
            if(MBMasuFlagGet(MASU_LAYER_DEFAULT, i) & MASU_FLAG_START) {
                break;
            }
        }
        if(i <= MBMasuNumGet(MASU_LAYER_DEFAULT)) {
            donkeyMasuId = i;
        } else {
            donkeyMasuId = 1;
        }
        MBCameraSkipSet(FALSE);
        viewRot.x = -10;
        viewRot.y = 0;
        viewRot.z = 0;
        tempVec.x = 0;
        tempVec.y = 250;
        tempVec.z = 0;
        MBCameraMasuViewSet(donkeyMasuId, &viewRot, &tempVec, 1800, -1, 1);
        MBCameraMotionWait();
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 30);
    } else {
        if(capsuleFlags.donkeyF) {
            MBMusPauseFadeOut(MB_MUS_CHAN_BG, TRUE, 0);
            hammerMdlId = MBCapsuleModelCreate(CAPSULE_HSF_hammer, NULL, FALSE, 10, FALSE);
            MBModelDispSet(hammerMdlId, FALSE);
            MBPlayerPosGet(capsulePlayer, &tempVec);
            MBModelPosSetV(hammerMdlId, &tempVec);
            MBCameraModelViewSet(hammerMdlId, FALSE, &capsuleViewOfs, -1, -1, 21);
            MBCameraMotionWait();
            MBPlayerPosGet(capsulePlayer, &playerPos);
            MBModelPosSetV(treeMdlId, &playerPos);
            MBModelDispSet(treeMdlId, TRUE);
            MBModelLayerSet(treeMdlId, 2);
            MBMotionTimeSet(treeMdlId, 0);
            MBMotionSpeedSet(treeMdlId, 1);
            MBPlayerMotionNoShiftSet(capsulePlayer,motId, 0, 8, HU3D_MOTATTR_LOOP);
            MBAudFXPlay(MSM_SE_BOARD_105);
            MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_DONKEY, 127, 0);
            do {
                t = MBMotionTimeGet(treeMdlId)/MBMotionMaxTimeGet(treeMdlId);
                if(t > 1) {
                    t = 1;
                }
                Hu3DModelObjMtxGet(MBModelIdGet(treeMdlId), "tree_fook", hookMtx);
                if(t <= 0.5f) {
                    tempVec.x = hookMtx[0][3]-(80*HuSin(90*(2*t)));
                    tempVec.y = hookMtx[1][3];
                    tempVec.z = hookMtx[2][3]+(80*HuSin(90*(2*t)));
                } else {
                    tempVec.x = hookMtx[0][3]-(80*HuSin(90));
                    tempVec.y = hookMtx[1][3];
                    tempVec.z = hookMtx[2][3]+(80*HuSin(90));
                }
                MBPlayerPosSetV(capsulePlayer, &tempVec);
                tempVec.x = hookMtx[0][3];
                tempVec.y = hookMtx[1][3];
                tempVec.z = hookMtx[2][3];
                MBModelPosSetV(hammerMdlId, &tempVec);
                HuPrcVSleep();
            } while(t < 1);
            MBPlayerRotateStart(capsulePlayer, 135, 15);
            while(!MBPlayerRotateCheck(capsulePlayer)) {
                HuPrcVSleep();
            }
            MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_LOOP);
            MBPlayerPosGet(capsulePlayer, &mdlPos);
            mdlPos.x += 160;
            mdlPos.z -= 160;
            donkeyPos = mdlPos;
            scale = 0;
            mdlVel.x = 0;
            mdlVel.y = 40;
            mdlVel.z = 0;
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelRotSet(mdlId, 0, -45, 0);
            MBModelScaleSet(mdlId, 0, 0, 0);
            MBModelDispSet(mdlId, TRUE);
            MBAudFXPlay(MSM_SE_BOARD_106);
            do {
                HuPrcVSleep();
                VECAdd(&mdlPos, &mdlVel, &mdlPos);
                mdlVel.y -= 2.2866669f;
                if(mdlPos.y < donkeyPos.y) {
                    mdlPos.y = donkeyPos.y;
                }
                if(scale < 1) {
                    scale += 0.05f;
                }
                MBModelPosSetV(mdlId, &mdlPos);
                MBModelScaleSet(mdlId, scale, scale, scale);
            } while(mdlPos.y > donkeyPos.y || scale < 1);
            MBModelPosSetV(mdlId, &donkeyPos);
            MBAudFXPlay(MSM_SE_GUIDE_30);
            MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBCameraModelViewSet(hammerMdlId, FALSE, &capsuleViewOfs, 1800, -1, 21);
            MBCameraMotionWait();
            if(GWPartyFGet() == TRUE) {
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_LUCK, HUWIN_SPEAKER_NONE);
                MBTopWinWait();
                MBTopWinKill();
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_EVENT, HUWIN_SPEAKER_NONE);
                MBTopWinWait();
                MBTopWinKill();
                eventNo = MBCapsuleEffRand(3);
                MBMotionShiftSet(mdlId, 7, 0, 8, HU3D_MOTATTR_NONE);
                do {
                    HuPrcVSleep();
                } while(MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
                do {
                    HuPrcVSleep();
                } while(MBMotionTimeGet(mdlId) < MBMotionMaxTimeGet(mdlId)*0.5f);
                MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                MBAudFXPlay(MSM_SE_BOARD_113);
                MBCapsuleSpriteShow(eventFileTbl[eventNo], 288, 240, FALSE);
                HuPrcSleep(30);
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                switch(eventNo) {
                    case 0:
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_STAR, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_STAR_DONKEY, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        tempVec.x = mdlPos.x;
                        tempVec.y = mdlPos.y;
                        tempVec.z = mdlPos.z;
                        saiHookTime = (MBCapsuleEffRandF()+0.33f)*60;
                        saiHookMdlId = mdlId;
                        MBMotionShiftSet(mdlId, 9, 0, 8, HU3D_MOTATTR_LOOP);
                        for(i=0; i<30.0f; i++) {
                            t = i/30.0f;
                            MBModelRotSet(mdlId, 0, -45+(t*45), 0);
                            HuPrcVSleep();
                        }
                        MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
                        MBSaiExec(-1, SAITYPE_EVENT, saiTbl, SAI_VALUE_ANY, FALSE, FALSE, &tempVec, SAI_COLOR_RED);
                        MBSaiPadBtnHookSet(-1, DonkeySaiPadHook);
                        MBSaiMotHookSet(-1, DonkeySaiMotHook);
                        while(!MBSaiKillCheck(-1)) {
                            HuPrcVSleep();
                        }
                        saiDonkeyResult = MBSaiResultGet(-1);
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_STAR_PLAYER, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBPlayerRotateStart(capsulePlayer, 0, 15);
                        while(!MBPlayerRotateCheck(capsulePlayer)) {
                            HuPrcVSleep();
                        }
                        coinNum = MBSaiPlayerExec(capsulePlayer, SAITYPE_EVENT);
                        HuPrcSleep(30);
                        MBSaiNumKill(capsulePlayer);
                        MBSaiNumKill(-1);
                        if(saiDonkeyResult < coinNum) {
                            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_STAR_WIN, HUWIN_SPEAKER_NONE);
                            MBTopWinWait();
                            MBTopWinKill();
                            MBMusFadeOut(MB_MUS_CHAN_FG, 5);
                            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                                HuPrcVSleep();
                            }
                            MBPlayerRotateStart(capsulePlayer, 0, 15);
                            while(!MBPlayerRotateCheck(capsulePlayer)) {
                                HuPrcVSleep();
                            }
                            MBAudFXPlay(MSM_SE_GUIDE_30);
                            MBMotionShiftSet(mdlId, 3, 0, 8, HU3D_MOTATTR_NONE);
                            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                            MBPlayerPosGet(capsulePlayer, &tempVec);
                            tempVec.y += 350;
                            MBStarGetExec(capsulePlayer, &tempVec);
                            MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_RESULT_DONKEY, 127, 0);
                            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                            MBCapsuleCameraViewPlayerSet(capsulePlayer);
                            MBCameraMotionWait();
                        } else {
                            MBAudFXPlay(MSM_SE_GUIDE_32);
                            MBCapsuleModelMotSet(mdlId, 4, HU3D_MOTATTR_NONE, TRUE);
                            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_STAR_LOSE, HUWIN_SPEAKER_NONE);
                            MBTopWinWait();
                            MBTopWinKill();
                        }
                        HuPrcSleep(30);
                        MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                        break;
                    
                    case 1:
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_COIN, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_COIN_START, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBPlayerRotateStart(capsulePlayer, 0, 15);
                        while(!MBPlayerRotateCheck(capsulePlayer)) {
                            HuPrcVSleep();
                        }
                        coinNum = MBSaiPlayerExec(capsulePlayer, SAITYPE_PATAPATA_COIN);
                        sprintf(insertMes, "%d", coinNum);
                        HuPrcSleep(30);
                        MBSaiNumKill(capsulePlayer);
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_ROULETTE_COIN_RESULT, HUWIN_SPEAKER_NONE);
                        MBTopWinInsertMesSet(MESSNUM_PTR(insertMes), 0);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBAudFXPlay(MSM_SE_GUIDE_31);
                        MBCapsuleModelMotSet(mdlId, 7, HU3D_MOTATTR_NONE, TRUE);
                        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                        for(i=0; i<coinNum; i++) {
                            MBPlayerPosGet(capsulePlayer, &tempVec);
                            tempVec.x += (0.5f-MBCapsuleEffRandF())*100;
                            tempVec.y += 700;
                            mdlVel.x = mdlVel.y = mdlVel.z = 0;
                            coinEffNum = MBCapsuleCoinEffAdd(&tempVec, &mdlVel, 0.75f, 4.9f, 30, 1);
                            if(coinEffNum != -1) {
                                MBPlayerPosGet(capsulePlayer, &tempVec);
                                MBCapsuleCoinEffMaxYSet(coinEffNum, tempVec.y+150);
                            }
                            HuPrcVSleep();
                        }
                        MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                        while(MBCapsuleCoinEffNumGet()) {
                            HuPrcVSleep();
                        }
                        MBCoinAddExec(capsulePlayer, coinNum);
                        MBCapsuleCoinDispExec(capsulePlayer, coinNum, TRUE, TRUE);
                        MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                        break;
                    
                    default:
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_MG, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_BANANA_ROULETTE, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        sprId[0] = MBCapsuleSprCreate(CAPSULE_ANM_donkeyBanana, 100, 0);
                        espPosSet(sprId[0], 288, 240);
                        espScaleSet(sprId[0], 0, 0);
                        sprId[1] = MBCapsuleSprCreate(CAPSULE_ANM_donkeyBananaNum, 100, 0);
                        espPosSet(sprId[1], 288, 240);
                        espScaleSet(sprId[1], 0, 0);
                        espAttrSet(sprId[1], HUSPR_ATTR_NOANIM);
                        espBankSet(sprId[1], 0);
                        for(i=0; i<=30.0f; i++) {
                            t = i/30.0f;
                            for(sprNo=0; sprNo<2; sprNo++) {
                                espScaleSet(sprId[sprNo], t, t);
                            }
                            HuPrcVSleep();
                        }
                        bananaTickNum = (MBCapsuleEffRandF()*60)+120;
                        bananaNum = 0;
                        bananaNumSpeed = 0.33f;
                        for(sprNo=prevSprNo=0, i=0; i<=bananaTickNum; i++) {
                            bananaNum += bananaNumSpeed;
                            if(bananaNum >= 3) {
                                bananaNum -= 3.0;
                            }
                            sprNo = bananaNum;
                            if(sprNo >= 3) {
                                sprNo = 2;
                            }
                            espBankSet(sprId[1], sprNo);
                            if(sprNo != prevSprNo) {
                                MBAudFXPlay(MSM_SE_BOARD_05);
                            }
                            prevSprNo = sprNo;
                            HuPrcVSleep();
                        }
                        for(i=0; i<=60; i++) {
                            if(bananaNumSpeed > 0.02f) {
                                bananaNumSpeed -= 0.005f;
                            }
                            bananaNum += bananaNumSpeed;
                            if(bananaNum >= 3) {
                                bananaNum -= 3.0;
                            }
                            sprNo = bananaNum;
                            if(sprNo >= 3) {
                                sprNo = 2;
                            }
                            espBankSet(sprId[1], sprNo);
                            if(sprNo != prevSprNo) {
                                MBAudFXPlay(MSM_SE_BOARD_05);
                            }
                            prevSprNo = sprNo;
                            HuPrcVSleep();
                        }
                        MBAudFXPlay(MSM_SE_BOARD_130);
                        for(i=0; i<=60; i++) {
                            t = i/60.0f;
                            t = 1+(0.5*HuSin(t*720));
                            espScaleSet(sprId[1], t, t);
                            HuPrcVSleep();
                        }
                        for(i=0; i<=30.0f; i++) {
                            t = i/30.0f;
                            for(sprNo=0; sprNo<2; sprNo++) {
                                espScaleSet(sprId[sprNo], 1-t, 1-t);
                            }
                            HuPrcVSleep();
                        }
                        for(sprNo=0; sprNo<2; sprNo++) {
                            espDispOff(sprId[sprNo]);
                        }
                        coinNum = ((int)bananaNum)+1;
                        sprintf(insertMes, "%d", coinNum);
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_BANANA_ROULETTE_COIN, HUWIN_SPEAKER_NONE);
                        MBTopWinInsertMesSet(MESSNUM_PTR(insertMes), 0);
                        MBTopWinWait();
                        MBTopWinKill();
                        MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                        MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_MG_ROULETTE, HUWIN_SPEAKER_NONE);
                        MBTopWinWait();
                        MBTopWinKill();
                        memset(&MgResultData, 0, sizeof(MgResultData));
                        MgResultData.playerNo1 = capsulePlayer;
                        MgResultData.coinNum = coinNum;
                        GWMgCoinBonusSet(0, 0);
                        GWMgCoinBonusSet(1, 0);
                        GWMgCoinBonusSet(2, 0);
                        GWMgCoinBonusSet(3, 0);
                        MBMgCallDonkeyExec();
                        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
                        while(MBMusCheck(MB_MUS_CHAN_FG)) {
                            HuPrcVSleep();
                        }
                        MBMusMainPlay();
                        HuPrcEnd();
                        return;
                }
            } else {
                MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                if(!MBPlayerStoryComCheck(capsulePlayer)) {
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_MG_RESULT, MBGuideSpeakerNoGet());
                    MBTopWinWait();
                    MBTopWinKill();
                    MBPlayerRotateStart(capsulePlayer, 0, 15);
                    while(!MBPlayerRotateCheck(capsulePlayer)) {
                        HuPrcVSleep();
                    }
                    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                    MBCapsuleModelMotSet(mdlId, 7, HU3D_MOTATTR_NONE, TRUE);
                    for(i=0; i<10; i++) {
                        MBPlayerPosGet(capsulePlayer, &tempVec);
                        tempVec.x += (0.5f-MBCapsuleEffRandF())*100;
                        tempVec.y += 700;
                        mdlVel.x = mdlVel.y = mdlVel.z = 0;
                        coinEffNum = MBCapsuleCoinEffAdd(&tempVec, &mdlVel, 0.75f, 4.9f, 30, 1);
                        if(coinEffNum != -1) {
                            MBPlayerPosGet(capsulePlayer, &tempVec);
                            MBCapsuleCoinEffMaxYSet(coinEffNum, tempVec.y+150);
                        }
                        HuPrcVSleep();
                    }
                    MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
                    while(MBCapsuleCoinEffNumGet()) {
                        HuPrcVSleep();
                    }
                    mgCoinNum = 10;
                    MBCoinAddExec(capsulePlayer, mgCoinNum);
                    MBCapsuleCoinDispExec(capsulePlayer, mgCoinNum, TRUE, TRUE);
                    MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
                    MBAudFXPlay(MSM_SE_GUIDE_52);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_MG_RESULT_END, MBGuideSpeakerNoGet());
                    MBTopWinWait();
                    MBTopWinKill();
                } else {
                    mgCoinNum = MBPlayerCoinGet(capsulePlayer)/2;
                    MBPlayerMotionNoSet(capsulePlayer, MB_PLAYER_MOT_SHOCK, HU3D_MOTATTR_NONE);
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_COM_START, HUWIN_SPEAKER_NONE);
                    MBTopWinWait();
                    MBTopWinKill();
                    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
                    MBMotionShiftSet(mdlId, 8, 0, 8, HU3D_MOTATTR_NONE);
                    i = 0;
                    do {
                        HuPrcVSleep();
                        if(i == 0
                        && MBMotionShiftIDGet(mdlId) == HU3D_MOTID_NONE
                        && MBMotionTimeGet(mdlId) >= 87
                        && MBMotionTimeGet(mdlId) < 97) {
                            t = (MBMotionTimeGet(mdlId)-87)/10;
                            tempVec.x = mdlPos.x-(t*40);
                            tempVec.z = mdlPos.z+(t*40);
                            tempVec.y = mdlPos.y;
                            MBModelPosSetV(mdlId, &tempVec);
                        }
                        if(i == 0
                        && MBMotionShiftIDGet(mdlId) == HU3D_MOTID_NONE
                        && MBMotionTimeGet(mdlId) >= 97) {
                            seDelayMax = 24;
                            effRotY = MBCapsuleEffRandF()*360;
                            for(i=0; i<mgCoinNum; i++) {
                                if(mgCoinNum < 16) {
                                    effRotY += 360.0f/mgCoinNum;
                                } else {
                                    effRotY += (MBCapsuleEffRandF()*10)+22.5f;
                                }
                                MBPlayerPosGet(capsulePlayer, &effPos);
                                effPos.y += 100;
                                effRotX = (MBCapsuleEffRandF()*15)+70;
                                t = ((MBCapsuleEffRandF()*0.3f)+0.8f)*65;
                                effRot.x = HuSin(effRotY)*HuCos(effRotX)*t;
                                effRot.z = HuCos(effRotY)*HuCos(effRotX)*t;
                                effRot.y = t*HuSin(effRotX);
                                MBCapsuleCoinEffAdd(&effPos, &effRot, 0.75f, 4.9f, 30, 0);
                            }
                            MBCapsuleSeDelayPlay(MSM_SE_CMN_19, 30);
                            i = MBCapsuleModelCreate(CAPSULE_HSF_dkeff, NULL, FALSE, 0, FALSE);
                            tempVec.x = mdlPos.x-100;
                            tempVec.y = mdlPos.y+50;
                            tempVec.z = mdlPos.z+80;
                            MBModelPosSetV(i, &tempVec);
                            MBModelScaleSet(i, 2.5f, 2.5f, 2.5f);
                            MBModelLayerSet(i, 3);
                            MBPlayerCoinAdd(capsulePlayer, -mgCoinNum);
                            MBPlayerMotionNoSet(capsulePlayer, MB_PLAYER_MOT_SHOCK, HU3D_MOTATTR_NONE);
                            i = 1;
                            HuAudFXPlay(MSM_SE_BOARD_56);
                        }
                    } while(MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE || MBMotionTimeGet(mdlId) < 98);
                    for(i=10; i>=0; i--) {
                        t = i/10.0f;
                        tempVec.x = mdlPos.x-(t*40);
                        tempVec.z = mdlPos.z+(t*40);
                        tempVec.y = mdlPos.y;
                        MBModelPosSetV(mdlId, &tempVec);
                        HuPrcVSleep();
                    }
                    MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
                    do {
                        HuPrcVSleep();
                    } while(!MBPlayerMotionEndCheck(capsulePlayer) || MBMotionShiftIDGet(MBPlayerModelGet(capsulePlayer)) != HU3D_MOTID_NONE);
                    MBPlayerMotionNoShiftSet(capsulePlayer, MB_PLAYER_MOT_IDLE, 0, 8, HU3D_MOTATTR_LOOP);
                    do {
                        HuPrcVSleep();
                    } while(!MBMotionEndCheck(mdlId) || MBMotionShiftIDGet(mdlId) != HU3D_MOTID_NONE);
                    MBMotionShiftSet(mdlId, 1, 0, 8, HU3D_MOTATTR_LOOP);
                    HuPrcSleep(60);
                    if(mgCoinNum >= 1) {
                        MBCapsuleCoinDispExec(capsulePlayer, -mgCoinNum, TRUE, TRUE);
                    }
                    MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_COIN_HALF, HUWIN_SPEAKER_NONE);
                    MBTopWinInsertMesSet(MBPlayerNameMesGet(capsulePlayer), 0);
                    MBTopWinWait();
                    MBTopWinKill();
                }
            }
        } else {
            MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_RESULT_DONKEY, 127, 0);
            MBPlayerPosRestoreAll();
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &playerPos);
            MBPlayerRotSet(capsulePlayer, 0, 135, 0);
            hammerMdlId = MBCapsuleModelCreate(CAPSULE_HSF_hammer, NULL, FALSE, 10, FALSE);
            tempVec = playerPos;
            MBModelPosSetV(hammerMdlId, &tempVec);
            MBModelDispSet(hammerMdlId, FALSE);
            MBCameraModelViewSet(hammerMdlId, NULL, &capsuleViewOfs, -1, -1, 1);
            tempVec = playerPos;
            MBModelPosSetV(treeMdlId, &tempVec);
            MBModelDispSet(treeMdlId, TRUE);
            MBMotionTimeSet(treeMdlId, MBMotionMaxTimeGet(treeMdlId));
            HuPrcVSleep();
            Hu3DModelObjMtxGet(MBModelIdGet(treeMdlId), "tree_fook", hookMtx);
            tempVec.x = hookMtx[0][3]-(80*HuSin(90));
            tempVec.y = hookMtx[1][3];
            tempVec.z = hookMtx[2][3]+(80*HuSin(90));
            MBPlayerPosSetV(capsulePlayer, &tempVec);
            tempVec.x = hookMtx[0][3];
            tempVec.y = hookMtx[1][3];
            tempVec.z = hookMtx[2][3];
            MBModelPosSetV(hammerMdlId, &tempVec);
            MBCameraModelViewSet(MB_MODEL_NONE, NULL, &capsuleViewOfs, -1, -1, 1);
            MBPlayerPosGet(capsulePlayer, &mdlPos);
            mdlPos.x += 160;
            mdlPos.z -= 160;
            donkeyPos = mdlPos;
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelRotSet(mdlId, 0, -45, 0);
            MBMotionNoSet(mdlId, 1, HU3D_MOTATTR_LOOP);
            MBModelDispSet(mdlId, TRUE);
            MBCameraModelViewSet(MB_MODEL_NONE, NULL, &capsuleViewOfs, 1800, -1, 21);
            MBCameraMotionWait();
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            coinNum = MgResultData.coinNum;
            sprintf(insertMes, "%d", coinNum);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_BANANA_ROULETTE_COIN_RESULT, HUWIN_SPEAKER_NONE);
            MBTopWinInsertMesSet(MESSNUM_PTR(insertMes), 0);
            MBTopWinWait();
            MBTopWinKill();
            MBPlayerRotateStart(capsulePlayer, 0, 15);
            while(!MBPlayerRotateCheck(capsulePlayer)) {
                HuPrcVSleep();
            }
            for(i=0; i<GW_PLAYER_MAX; i++) {
                bonusTbl[i] = 0;
                if(MBPlayerAliveCheck(i)) {
                    bonusTbl[i] = GWMgCoinBonusGet(i)*coinNum;
                }
            }
            MBCoinAddAllDispExec(bonusTbl[0], bonusTbl[1], bonusTbl[2], bonusTbl[3], TRUE);
            if(GWMgCoinBonusGet(capsulePlayer) > 0) {
                MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINWIN, CHARVOICEID(5));
                MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINWIN, HU3D_MOTATTR_NONE, TRUE);
            } else {
                MBPlayerWinLoseVoicePlay(capsulePlayer, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
                MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_COINLOSE, HU3D_MOTATTR_NONE, TRUE);
            }
            MBCapsulePlayerMotSet(capsulePlayer, MB_PLAYER_MOT_IDLE, HU3D_MOTATTR_LOOP, TRUE);
        }
        MBPlayerRotateStart(capsulePlayer, 0, 15);
        while(!MBPlayerRotateCheck(capsulePlayer)) {
            HuPrcVSleep();
        }
        if(GWPartyFGet() == TRUE) {
            MBCapsuleEffHookCall(3, mdlId, FALSE, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX04_DONKEY_STRANGER, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTopWinKill();
        }
        if(capsuleFlags.donkeyF || capsuleFlags.donkeyMgEndF) {
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
        }
        MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_LOOP);
        MBPlayerPosGet(capsulePlayer, &mdlPos);
        mdlPos.x += 160;
        mdlPos.z -= 160;
        donkeyPos = mdlPos;
        scale = 1;
        mdlVel.x = 0;
        mdlVel.y = 40;
        mdlVel.z = 0;
        MBModelPosSetV(mdlId, &mdlPos);
        MBModelScaleSet(mdlId, 1, 1, 1);
        MBModelDispSet(mdlId, TRUE);
        MBAudFXPlay(MSM_SE_BOARD_131);
        do {
            HuPrcVSleep();
            VECAdd(&mdlPos, &mdlVel, &mdlPos);
            mdlVel.y -= 2.2866669f;
            if(mdlPos.y < donkeyPos.y) {
                mdlPos.y = donkeyPos.y;
            }
            if(scale > 0) {
                scale -= 0.035f;
            }
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelScaleSet(mdlId, HuSin(scale*90), HuSin(scale*90), HuSin(scale*90));
        } while(mdlPos.y > donkeyPos.y || scale > 0);
        MBModelDispSet(mdlId, FALSE);
        MBModelDispSet(treeMdlId, TRUE);
        MBMotionTimeSet(treeMdlId, MBMotionMaxTimeGet(treeMdlId));
        MBMotionSpeedSet(treeMdlId, -1);
        MBAudFXPlay(MSM_SE_BOARD_138);
        do {
            t = MBMotionTimeGet(treeMdlId)/MBMotionMaxTimeGet(treeMdlId);
            if(t < 0) {
                t = 0;
            }
            Hu3DModelObjMtxGet(MBModelIdGet(treeMdlId), "tree_fook", hookMtx);
            if(t >= 0.5f) {
                tempVec.x = hookMtx[0][3]-(80*HuSin(90));
                tempVec.y = hookMtx[1][3];
                tempVec.z = hookMtx[2][3]+(80*HuSin(90));
            } else {
                tempVec.x = hookMtx[0][3]-(80*HuSin(90*(2*t)));
                tempVec.y = hookMtx[1][3];
                tempVec.z = hookMtx[2][3]+(80*HuSin(90*(2*t)));
            }
            MBPlayerPosSetV(capsulePlayer, &tempVec);
            tempVec.x = hookMtx[0][3];
            tempVec.y = hookMtx[1][3];
            tempVec.z = hookMtx[2][3];
            MBModelPosSetV(hammerMdlId, &tempVec);
            HuPrcVSleep();
        } while(t > 0);
        MBPlayerPosSetV(capsulePlayer, &playerPos);
        MBModelDispSet(treeMdlId, FALSE);
        MBCameraMotionWait();
        if(capsuleFlags.donkeyF || capsuleFlags.donkeyMgEndF) {
            MBMusMainPlay();
        }
        MBPlayerRotateStart(capsulePlayer, 0, 15);
        while(!MBPlayerRotateCheck(capsulePlayer)) {
            HuPrcVSleep();
        }
        MBCameraModelViewSet(MBPlayerModelGet(capsulePlayer), NULL, &capsuleViewOfs, -1, -1, 21);
        MBCameraMotionWait();
    }
    HuPrcEnd();
}

void MBCapsuleDonkeyDestroy(void)
{
    MBCapsuleGlowEffKill();
    MBCapsuleCoinEffKill();
}

static u16 DonkeySaiPadHook(int playerNo)
{
    if(--saiHookTime <= 0) {
        return PAD_BUTTON_A;
    } else {
        return 0;
    }
}

static void DonkeySaiMotHook(int playerNo)
{
    int i;
    if(saiHookMdlId != MB_MODEL_NONE) {
        MBMotionNoSet(saiHookMdlId, 6, HU3D_MOTATTR_NONE);
        i=0;
        do {
            if(i++ == 27) {
                MBSaiObjHit(playerNo);
            }
            HuPrcVSleep();
        } while(!MBMotionEndCheck(saiHookMdlId));
        MBMotionNoSet(saiHookMdlId, 1, HU3D_MOTATTR_LOOP);
    }
}

void MBCapsuleDonkeyAddRandom(void)
{
    int masuId;
    int i;
    MASU *masuP;
    int ofs;
    int masuMax;
    masuMax = MBMasuNumGet(MASU_LAYER_DEFAULT);
    ofs = ((frand()%0x7FFF)%masuMax)+1;
    for(i=0; i<MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        if((masuId=ofs+i) > MBMasuNumGet(MASU_LAYER_DEFAULT)) {
            masuId %= MBMasuNumGet(MASU_LAYER_DEFAULT);
            masuId++;
        }
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
        if(masuP != NULL && !masuP->hasStarF && MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId) == MASU_TYPE_BLUE) {
            break;
        }
    }
    if(i > MBMasuNumGet(MASU_LAYER_DEFAULT)) {
        masuId = MBMasuNumGet(MASU_LAYER_DEFAULT);
    }
    MBMasuTypeSet(MASU_LAYER_DEFAULT, masuId, MASU_TYPE_DONKEY);
    MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, CAPSULE_DONKEY);
}

void MBCapsuleVsExec(void)
{
    int i; //r31
    int coinNum; //r30
    int mdlId; //r29
    int playerNum; //r28
    int totalCoin; //r25
    int guideMdlId; //r24
    int masuId; //r23
    BOOL partyF; //r21
    int squishNum; //r20
    int bonusNum; //r19
    
    int motFile[16]; //sp+0xB8
    char insertMes1[32]; //sp+0x98
    char insertMes2[32]; //sp+0x78
    HuVecF playerPos; //sp+0x6C
    HuVecF playerPosOld; //sp+0x60
    HuVecF mdlPos; //sp+0x54
    HuVecF guidePos; //sp+0x48
    int squishTbl[GW_PLAYER_MAX]; //sp+0x38
    int bonusTbl[GW_PLAYER_MAX]; //sp+0x28
    
    MBCapsuleExplodeEffCreate(0);
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &playerPos);
    playerPosOld = playerPos;
    if(!capsuleFlags.vsMgEndF) {
        MBMusFadeOut(MB_MUS_CHAN_BG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_BG)) {
            HuPrcVSleep();
        }
        MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_BATTLE, 127, 0);
        MBCapsuleSpriteShow(BOARD_ANM_telopStoryMgVs, 288, 240, FALSE);
        MBDataAsyncWait(MBDataDirReadAsync(DATA_guide));
        guidePos = playerPos;
        guidePos.y += 300;
        guidePos.z -= 50;
        guideMdlId = MBGuideCreateIn(&guidePos, FALSE, TRUE, FALSE);
        MBModelLayerSet(guideMdlId, 2);
        HuDataDirClose(DATA_guide);
        for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i) && MBPlayerStoryComCheck(i)) {
                playerNum++;
            }
        }
        MBAudFXPlay(MSM_SE_GUIDE_53);
        switch(playerNum) {
            case 3:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_START_3P, MBGuideSpeakerNoGet());
                MBTopWinWait();
                MBTopWinKill();
                coinNum = 5;
                break;
            
            case 2:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_START_2P, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MBPlayerNameMesGet(MBPlayerStoryPlayerGet()), 0);
                MBTopWinWait();
                MBTopWinKill();
                coinNum = 10;
                break;
                
            default:
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_START_4P, MBGuideSpeakerNoGet());
                MBTopWinWait();
                MBTopWinKill();
                coinNum = 20;
                break;
        }
        memset(&MgResultData, 0, sizeof(MgResultData));
        MgResultData.playerNo1 = MBPlayerStoryPlayerGet();
        MgResultData.coinNum = coinNum;
        GWMgCoinBonusSet(0, 0);
        GWMgCoinBonusSet(1, 0);
        GWMgCoinBonusSet(2, 0);
        GWMgCoinBonusSet(3, 0);
        GWMgCoinSet(0, 0);
        GWMgCoinSet(1, 0);
        GWMgCoinSet(2, 0);
        GWMgCoinSet(3, 0);
        MBMgCallVsExec();
        MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
        while(MBMusCheck(MB_MUS_CHAN_FG)) {
            HuPrcVSleep();
        }
        MBMusMainPlay();
        MBGuideEnd(guideMdlId);
    } else {
        MBPlayerPosRestoreAll();
        masuId = GwPlayer[MgResultData.playerNo1].masuId;
        MBPlayerPosGet(MgResultData.playerNo1, &playerPos);
        playerPosOld = playerPos;
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &playerPos);
        MBPlayerRotSet(MgResultData.playerNo1, 0, 0, 0);
        for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i) && MBPlayerStoryComCheck(i)) {
                playerNum++;
            }
        }
        if(playerNum < 1) {
            playerNum = 1;
        }
        MBCameraSkipSet(FALSE);
        MBCapsuleCameraViewPlayerSet(MgResultData.playerNo1);
        MBCameraMotionWait();
        MBCameraMasuViewSet(masuId, NULL, &capsuleViewOfs, -1, -1, 1);
        MBCameraMotionWait();
        MBCameraSkipSet(TRUE);
        coinNum = GWMgCoinBonusGet(MgResultData.playerNo1)+GWMgCoinGet(MgResultData.playerNo1);
        for(bonusNum=0, i=0; i<GW_PLAYER_MAX; i++) {
            if(MBPlayerAliveCheck(i)) {
                bonusTbl[i] = GWMgCoinBonusGet(i)+GWMgCoinGet(i);
                if(i != MgResultData.playerNo1 && bonusTbl[i] > 0) {
                    bonusNum = bonusTbl[i];
                }
            }
        }
        if(coinNum > 0 && bonusNum <= 0) {
            guidePos = playerPos;
            guidePos.y += 300;
            guidePos.z -= 50;
            guideMdlId = MBGuideCreateFlag(MB_GUIDE_NERL, &guidePos, FALSE, FALSE, TRUE, FALSE);
            MBModelLayerSet(guideMdlId, 2);
            HuDataDirClose(DATA_guide);
            MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_BOARD_RESULT_KETTOU, 127, 0);
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            MBAudFXPlay(MSM_SE_GUIDE_52);
            for(i=0, totalCoin=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                    coinNum = MgResultData.coinNum;
                    if(coinNum > MBPlayerCoinGet(i)) {
                        coinNum = MBPlayerCoinGet(i);
                    }
                    totalCoin += coinNum;
                }
            }
            if(playerNum > 1) {
                sprintf(insertMes1, "%d", MgResultData.coinNum);
                sprintf(insertMes2, "%d", totalCoin);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_RESULT_WIN_MULTI, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes1), 0);
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes2), 1);
                MBTopWinWait();
                MBTopWinKill();
            } else {
                sprintf(insertMes2, "%d", totalCoin);
                MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_RESULT_WIN, MBGuideSpeakerNoGet());
                MBTopWinInsertMesSet(MESSNUM_PTR(insertMes2), 0);
                MBTopWinWait();
                MBTopWinKill();
            }
            for(i=0, totalCoin=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                    coinNum = MgResultData.coinNum;
                    if(coinNum > MBPlayerCoinGet(i)) {
                        coinNum = MBPlayerCoinGet(i);
                    }
                    MBCoinAddProcExec(i, -coinNum, FALSE, TRUE);
                    totalCoin += coinNum;
                }
            }
            MBCoinAddProcExec(MgResultData.playerNo1, totalCoin, FALSE, TRUE);
            MBCapsuleCoinDispExec(MgResultData.playerNo1, totalCoin, TRUE, TRUE);
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            MBGuideEnd(guideMdlId);
            MBMusMainPlay();
        } else if(coinNum <= 0 && bonusNum > 0) {
            MBPlayerMoveInit();
            motFile[0] = CAPSULECHAR4_HSF_koopa_idle;
            motFile[1] = CAPSULECHAR4_HSF_koopa_stomp;
            motFile[2] = CAPSULECHAR4_HSF_koopa_laugh;
            motFile[3] = CAPSULECHAR4_HSF_koopa_walk;
            motFile[4] = CAPSULECHAR4_HSF_koopa_jump_start;
            motFile[5] = CAPSULECHAR4_HSF_koopa_lose;
            motFile[6] = CAPSULECHAR4_HSF_koopa_side_hit;
            motFile[7] = HU_DATANUM_NONE;
            mdlId = MBCapsuleModelCreate(CAPSULECHAR4_HSF_koopa, motFile, FALSE, 10, FALSE);
            partyF = FALSE;
            MBMusPlay(MB_MUS_CHAN_FG, MSM_STREAM_CAPSULE_KOOPA, 127, 0);
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &mdlPos);
            MBModelPosSetV(mdlId, &mdlPos);
            MBModelDispSet(mdlId, TRUE);
            MBMotionNoSet(mdlId, 2, HU3D_MOTATTR_NONE);
            MBMotionTimeSet(mdlId, MBMotionMaxTimeGet(mdlId)*0.5f);
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
            squishNum = 0;
            do {
                if(squishNum == 0 && MBMotionTimeGet(mdlId) > 137) {
                    MBCapsuleDustHeavyAdd(0, mdlPos);
                    MBAudFXPlay(MSM_SE_BOARD_115);
                    MBCapsuleVibrate(1);
                    squishNum = MBCapsulePlayerSquishSet(squishTbl, masuId);
                }
                HuPrcVSleep();
            } while(WipeCheckEnd() || !MBMotionEndCheck(mdlId));
            MBMotionShiftSet(mdlId, 1, 0, 30, HU3D_MOTATTR_LOOP);
            MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
            HuPrcSleep(30);
            MBAudFXPlay(MSM_SE_GUIDE_12);
            MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
            MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
            MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
            sprintf(insertMes1, "%d", MgResultData.coinNum);
            sprintf(insertMes2, "%d", MgResultData.coinNum*playerNum);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_RESULT_LOSE, HUWIN_SPEAKER_KOOPA);
            MBTopWinInsertMesSet(MESSNUM_PTR(insertMes1), 0);
            MBTopWinInsertMesSet(MESSNUM_PTR(insertMes2), 1);
            MBTopWinWait();
            MBTopWinKill();
            MBAudFXPlay(MSM_SE_GUIDE_12);
            MBCapsuleModelMotSet(mdlId, 3, HU3D_MOTATTR_NONE, TRUE);
            MBCapsuleModelMotSet(mdlId, 1, HU3D_MOTATTR_LOOP, TRUE);
            MBCapsuleEffHookCall(3, mdlId, partyF, FALSE, FALSE);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_RESULT_LOSE_END, HUWIN_SPEAKER_KOOPA);
            MBTopWinWait();
            MBTopWinKill();
            MBMusFadeOut(MB_MUS_CHAN_FG, 1000);
            while(MBMusCheck(MB_MUS_CHAN_FG)) {
                HuPrcVSleep();
            }
            MBMotionShiftSet(mdlId, 2, 0, 8, HU3D_MOTATTR_NONE);
            MBCapsuleVibrate(1);
            HuAudFXPlay(MSM_SE_GUIDE_15);
            for(i=0; i<90.0f; i++) {
                HuPrcVSleep();
            }
            MBModelDispSet(mdlId, FALSE);
            MBMusMainPlay();
            MBCapsulePlayerStunSet(squishTbl, squishNum, FALSE);
            for(i=0; i<60.0f; i++) {
                HuPrcVSleep();
            }
            for(i=0, playerNum=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerAliveCheck(i) && MBPlayerStoryComCheck(i)) {
                    playerNum++;
                }
            }
            coinNum = MgResultData.coinNum*playerNum;
            if(coinNum > MBPlayerCoinGet(MgResultData.playerNo1)) {
                coinNum = MBPlayerCoinGet(MgResultData.playerNo1);
            }
            for(i=0; i<GW_PLAYER_MAX; i++){ 
                if(MBPlayerStoryComCheck(i) && MBPlayerAliveCheck(i)) {
                    MBCoinAddProcExec(i, coinNum/playerNum, FALSE, TRUE);
                }
            }
            coinNum = MgResultData.coinNum*playerNum;
            if(coinNum > MBPlayerCoinGet(MgResultData.playerNo1)) {
                coinNum = MBPlayerCoinGet(MgResultData.playerNo1);
            }
            if(coinNum > 0) {
                MBCoinAddProcExec(MgResultData.playerNo1, -coinNum, TRUE, TRUE);
            }
            MBCapsulePlayerIdleWait();
            MBPlayerMoveObjClose();
            MBCapsuleEffHookCall(1, MB_MODEL_NONE, FALSE, FALSE, FALSE);
        } else {
            guidePos = playerPos;
            guidePos.y += 300;
            guidePos.z -= 50;
            guideMdlId = MBGuideCreateFlag(MB_GUIDE_NERL, &guidePos, FALSE, FALSE, TRUE, FALSE);
            MBModelLayerSet(guideMdlId, 2);
            HuDataDirClose(DATA_guide);
            MBMusMainPlay();
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            MBAudFXPlay(MSM_SE_GUIDE_53);
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_VSMASU_RESULT_TIE, MBGuideSpeakerNoGet());
            MBTopWinWait();
            MBTopWinKill();
            MBPlayerWinLoseVoicePlay(MgResultData.playerNo1, MB_PLAYER_MOT_COINLOSE, CHARVOICEID(10));
            MBCapsulePlayerMotSet(MgResultData.playerNo1, MB_PLAYER_MOT_COINLOSE, HU3D_MOTATTR_NONE, TRUE);
            MBGuideEnd(guideMdlId);
        }
        
    }
    HuPrcEnd();
}

void MBCapsuleVsDestroy(void)
{
    MBCapsuleExplodeEffKill(0);
}

void MBCapsuleDebugCamExec(void)
{
    MBCAMERA *cameraP; //r31
    int mdlId; //r27
    int time; //r25
    BOOL moveTextF; //r24
    BOOL mdlDispF; //r23
    
    float moveSpeed; //f31
    float zoom; //f30
    float textX; //f29
    float textY; //f28
    float oldZoom; //f27
    
    HuVecF center; //sp+0x74
    HuVecF rot; //sp+0x68
    HuVecF oldCenter; //sp+0x5C
    HuVecF oldRot; //sp+0x50
    
    MBCameraMotionWait();
    cameraP = MBCameraGet();
    center.x = oldCenter.x = cameraP->target.x+cameraP->offset.x;
    center.y = oldCenter.y = cameraP->target.y+cameraP->offset.y;
    center.z = oldCenter.z = cameraP->target.z+cameraP->offset.z;
    rot.x = oldRot.x = cameraP->rot.x;
    rot.y = oldRot.y = cameraP->rot.y;
    rot.z = oldRot.z = cameraP->rot.z;
    zoom = oldZoom = cameraP->zoom;
    moveTextF = FALSE;
    mdlDispF = FALSE;
    textX = 32;
    textY = 50;
    moveSpeed = 1;
    time = 0;
    mdlId = MBCapsuleModelCreate(CAPSULE_HSF_kinoko, NULL, FALSE, 10, FALSE);
    MBModelPosSet(mdlId, center.x, center.y, center.z);
    MBModelDispSet(mdlId, FALSE);
    MBCameraFocusSet(mdlId);
    do {
        if(HuPadBtnDown[0] & PAD_BUTTON_X) {
            moveTextF ^= 1;
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_Y) {
            mdlDispF ^= 1;
        }
        time++;
        MBModelDispSet(mdlId, mdlDispF);
        if(!moveTextF) {
            if(HuPadBtn[0] & PAD_TRIGGER_L) {
                moveSpeed = 2;
            } else if(HuPadBtn[0] & PAD_TRIGGER_R) {
                moveSpeed = 0.5;
            } else {
                moveSpeed = 1;
            }
            if(fabs(MBPadStkXGet(0)) > 8) {
                rot.y += 0.05f*MBPadStkXGet(0)*moveSpeed;
            }
            if(fabs(MBPadStkYGet(0)) > 8) {
                rot.x += 0.05f*MBPadStkYGet(0)*moveSpeed;
            }
            if(rot.x >= 360.0f) {
                rot.x -= 360.0f;
            } else if(rot.x < 0) {
                rot.x += 360;
            }
            if(rot.y >= 360.0f) {
                rot.y -= 360.0f;
            } else if(rot.y < 0) {
                rot.y += 360;
            }
            if(fabs(HuPadSubStkX[0]) > 8) {
                center.x += 0.5f*HuPadSubStkX[0]*moveSpeed;
            }
            if(fabs(HuPadSubStkY[0]) > 8) {
                center.z -= 0.5f*HuPadSubStkY[0]*moveSpeed;
            }
            if(HuPadBtn[0] & PAD_BUTTON_UP) {
                center.y += 25*moveSpeed;
            }
            if(HuPadBtn[0] & PAD_BUTTON_DOWN) {
                center.y -= 25*moveSpeed;
            }
            if(HuPadBtn[0] & PAD_BUTTON_A) {
                zoom += moveSpeed*25;
            }
            if(HuPadBtn[0] & PAD_BUTTON_B) {
                zoom -= moveSpeed*25;
            }
            if(HuPadBtnDown[0] & PAD_BUTTON_START) {
                center = oldCenter;
                rot = oldRot;
                zoom = oldZoom;
            }
            fontcolor = FONT_COLOR_WHITE;
        } else {
            if(fabs(MBPadStkXGet(0)) > 8) {
                textX += 0.05f*MBPadStkXGet(0);
            }
            if(fabs(MBPadStkYGet(0)) > 8) {
                textY += 0.05f*MBPadStkYGet(0);
            }
            fontcolor = FONT_COLOR_YELLOW;
        }
        print8(textX+36.0, textY+96.0, 1.5f, "\xFD\x01" "RX:%.4f RY:%.4f RZ:%.4f", rot.x, rot.y, rot.z);
        print8(textX+36.0, textY+108.0, 1.5f, "\xFD\x01" "CX:%.4f CY:%.4f CZ:%.4f", center.x, center.y, center.z);
        print8(textX+36.0, textY+120.0, 1.5f, "\xFD\x01" "ZM:%.4f", zoom);
        if(time & 0x1) {
            MBModelPosSet(mdlId, center.x, center.y, center.z);
            MBCameraModelViewSet(mdlId, &rot, NULL, zoom, -1, 1);
        }
        HuPrcVSleep();
    } while(!(HuPadBtn[0] & PAD_TRIGGER_L) || !(HuPadBtn[0] & PAD_TRIGGER_R));
    MBCameraSkipSet(TRUE);
    MBCameraModelViewSet(MB_MODEL_NONE, &oldRot, NULL, oldZoom, -1, 1);
    MBCameraFocusPlayerSet(capsulePlayer);
    HuPrcEnd();
}

void MBCapsuleDebugCamDestroy(void)
{
    
}

void MBCapsuleDebugWarpExec(void)
{
    int i; //r31
    int masuId; //r29
    int linkNum; //r27
    int sprId; //r26
    int mdlId; //r23
    MASU *masuP; //r22
    
    float stkX; //f31
    float stkY; //f30
    float t; //f29
    
    float angleTbl[MASU_LINK_MAX*2]; //sp+0x40
    s16 linkTbl[MASU_LINK_MAX*2]; //sp+0x2C
    HuVecF masuPos; //sp+0x20
    HuVecF masuPrevPos; //sp+0x14
    HuVecF pos; //sp+0x8
    
    masuId = GwPlayer[capsulePlayer].masuId;
    MBPlayerPosGet(capsulePlayer, &masuPos);
    mdlId = MBCapsuleModelCreate(CAPSULE_HSF_kinoko, NULL, FALSE, 10, FALSE);
    MBModelPosSet(mdlId, masuPos.x, masuPos.y, masuPos.z);
    MBModelDispSet(mdlId, FALSE);
    MBCameraFocusSet(mdlId);
    sprId = MBCapsuleSprCreate(BOARD_ANM_cursor, 2000, 0);
    espDispOn(sprId);
    espDrawNoSet(sprId, 0);
    espAttrSet(sprId, HUSPR_ATTR_NOANIM);
    espPosSet(sprId, 0, 0);
    espBankSet(sprId, 0);
    do {
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
        for(linkNum=0; linkNum<masuP->linkNum; linkNum++) {
            linkTbl[linkNum] = masuP->linkTbl[linkNum];
        }
        linkNum += MBMasuLinkTblGet(MASU_LAYER_DEFAULT, masuId, &linkTbl[linkNum]);
        for(i=0; i<linkNum; i++) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, linkTbl[i], &pos);
            VECSubtract(&pos, &masuPos, &pos);
            angleTbl[i] = MBCaspuleAngleRotCamera(HuAtan(pos.x, pos.z));
        }
        stkX = MBPadStkXGet(0);
        stkY = MBPadStkYGet(0);
        if(fabs(stkX) > 8 || fabs(stkY) > 8) {
            for(i=0; i<linkNum; i++) {
                if(fabs(MBCapsuleAngleWrap(angleTbl[i], HuAtan(stkX, -stkY))) < 30) {
                    masuId = linkTbl[i];
                    masuPrevPos = masuPos;
                    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
                    for(i=0; i<18.0f; i++) {
                        t = i/18.0f;
                        VECSubtract(&masuPos, &masuPrevPos, &pos);
                        VECScale(&pos, &pos, t);
                        VECAdd(&masuPrevPos, &pos, &pos);
                        MBModelPosSetV(mdlId, &pos);
                        Hu3D3Dto2D(&pos, HU3D_CAM0, &pos);
                        pos.x -= 32;
                        pos.y -= 16;
                        espPosSet(sprId, pos.x, pos.y);
                        HuPrcVSleep();
                    }
                    MBModelPosSetV(mdlId, &masuPos);
                }
            }
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_A) {
            GwPlayer[0].masuId = masuId;
            MBPlayerPosSetV(0, &masuPos);
            MBPlayerPosFixPlayer(0, masuId, TRUE);
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_B) {
            GwPlayer[1].masuId = masuId;
            MBPlayerPosSetV(1, &masuPos);
            MBPlayerPosFixPlayer(1, masuId, TRUE);
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_X) {
            GwPlayer[2].masuId = masuId;
            MBPlayerPosSetV(2, &masuPos);
            MBPlayerPosFixPlayer(2, masuId, TRUE);
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_Y) {
            GwPlayer[3].masuId = masuId;
            MBPlayerPosSetV(3, &masuPos);
            MBPlayerPosFixPlayer(3, masuId, TRUE);
        }
        Hu3D3Dto2D(&masuPos, HU3D_CAM0, &pos);
        pos.x -= 32;
        pos.y -= 16;
        espPosSet(sprId, pos.x, pos.y);
        HuPrcVSleep();
    } while(!(HuPadBtn[0] & PAD_TRIGGER_L) || !(HuPadBtn[0] & PAD_TRIGGER_R));
    MBCameraSkipSet(TRUE);
    MBCameraFocusPlayerSet(capsulePlayer);
    HuPrcEnd();
}

void MBCapsuleDebugWarpDestroy(void)
{
    
}

void MBCapsuleDebugPosSetExec(void)
{
    MBCapsulePosSetExec();
    HuPrcEnd();
}

void MBCapsuleDebugPosSetDestroy(void)
{
    
}

void MBCapsuleDonkeyAdd(int masuId)
{
    MBMasuTypeSet(MASU_LAYER_DEFAULT, masuId, MASU_TYPE_DONKEY);
    MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, CAPSULE_DONKEY);
}

void MBCapsuleStatusTradeInSet(int leftPlayer, int rightPlayer, BOOL waitF)
{
    HuVecF startPos;
    HuVecF endPos;
    startPos.x = -130;
    startPos.y = 240;
    startPos.z = 0;
    endPos.x = 130;
    endPos.y = 240;
    endPos.z = 0;
    MBStatusMoveSet(leftPlayer, &startPos, &endPos, STATUS_MOVE_SIN, 30);
    startPos.x = 706;
    startPos.y = 240;
    startPos.z = 0;
    endPos.x = 446;
    endPos.y = 240;
    endPos.z = 0;
    MBStatusMoveSet(rightPlayer, &startPos, &endPos, STATUS_MOVE_SIN, 30);
    if(waitF) {
        do {
            HuPrcVSleep();
        } while(!MBStatusMoveCheck(leftPlayer) || !MBStatusMoveCheck(rightPlayer));
    }
}

void MBCapsuleStatusTradeOutSet(int leftPlayer, int rightPlayer, BOOL waitF)
{
    HuVecF startPos;
    HuVecF endPos;
    startPos.x = -130;
    startPos.y = 240;
    startPos.z = 0;
    endPos.x = 130;
    endPos.y = 240;
    endPos.z = 0;
    MBStatusMoveSet(leftPlayer, &endPos, &startPos, STATUS_MOVE_REVCOS, 30);
    startPos.x = 706;
    startPos.y = 240;
    startPos.z = 0;
    endPos.x = 446;
    endPos.y = 240;
    endPos.z = 0;
    MBStatusMoveSet(rightPlayer, &endPos, &startPos, STATUS_MOVE_REVCOS, 30);
    if(waitF) {
        do {
            HuPrcVSleep();
        } while(!MBStatusMoveCheck(leftPlayer) || !MBStatusMoveCheck(rightPlayer));
    }
}

void MBCapsuleStatusKettouInSet(int leftPlayer, int rightPlayer, BOOL waitF)
{
    HuVecF posOff;
    HuVecF posOn;
    MBStatusPosOffGet(0, &posOff);
    MBStatusPosOnGet(0, &posOn);
    MBStatusLayoutSet(leftPlayer, STATUS_LAYOUT_TOP);
    MBStatusMoveSet(leftPlayer, &posOff, &posOn, STATUS_MOVE_SIN, 30);
    MBStatusPosOffGet(1, &posOff);
    MBStatusPosOnGet(1, &posOn);
    MBStatusLayoutSet(rightPlayer, STATUS_LAYOUT_TOP);
    MBStatusMoveSet(rightPlayer, &posOff, &posOn, STATUS_MOVE_SIN, 30);
    if(waitF) {
        do {
            HuPrcVSleep();
        } while(!MBStatusMoveCheck(leftPlayer) || !MBStatusMoveCheck(rightPlayer));
    }
}

void MBCapsuleStatusKettouOutSet(int leftPlayer, int rightPlayer, BOOL waitF)
{
    HuVecF posOff;
    HuVecF posOn;
    MBStatusPosOffGet(0, &posOff);
    MBStatusPosOnGet(0, &posOn);
    MBStatusMoveSet(leftPlayer, &posOn, &posOff, STATUS_MOVE_REVCOS, 30);
    MBStatusPosOffGet(1, &posOff);
    MBStatusPosOnGet(1, &posOn);
    MBStatusMoveSet(rightPlayer, &posOn, &posOff, STATUS_MOVE_REVCOS, 30);
    if(waitF) {
        do {
            HuPrcVSleep();
        } while(!MBStatusMoveCheck(leftPlayer) || !MBStatusMoveCheck(rightPlayer));
        MBStatusDispForceSet(leftPlayer, FALSE);
        MBStatusDispForceSet(rightPlayer, FALSE);
    }
}

void MBCapsuleStatusPosMoveWait(BOOL dispF, BOOL waitF)
{
    int i;
    int dispNum;
    int aliveNum;
    BOOL showAllF;
    BOOL hideAllF;
    for(i=0, aliveNum=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            aliveNum++;
        }
    }
    for(i=0, dispNum=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i) && MBStatusDispGet(i)) {
            dispNum++;
        }
    }
    if(dispNum == aliveNum) {
        showAllF = TRUE;
    } else {
        showAllF = FALSE;
    }
    for(i=0, dispNum=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i) && !MBStatusDispGet(i)) {
            dispNum++;
        }
    }
    if(dispNum == aliveNum) {
        hideAllF = TRUE;
    } else {
        hideAllF = FALSE;
    }
    if(dispF) {
        if(showAllF) {
            return;
        }
        if(!showAllF && !hideAllF) {
            MBCapsuleStatusPosMoveWait(FALSE, TRUE);
        }
    } else if(hideAllF) {
        return;
    }
    MBStatusDispSetAll(dispF);
    if(waitF) {
        do {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                if(MBPlayerAliveCheck(i) && !MBStatusMoveCheck(i)) {
                    break;
                }
            }
            HuPrcVSleep();
        } while(i < GW_PLAYER_MAX);
    }
}

BOOL MBCapsuleStatusFocusCheck(int playerNo)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(!MBPlayerAliveCheck(i)) {
            continue;
        }
        if(i == playerNo) {
            if(!MBStatusDispGet(i)) {
                break;
            }
        } else {
            if(MBStatusDispGet(i)) {
                break;
            }
        }
    }
    if(i >= GW_PLAYER_MAX) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void MBCapsuleStatusCapsulePosGet(int playerNo, int capsuleNo, HuVecF *pos)
{
    MBStatusPosGet(playerNo, pos);
    if(!GWTeamFGet()) {
        pos->x += (capsuleNo*24)+24;
        if(playerNo >= GW_PLAYER_MAX/2) {
            pos->y -= 34;
        } else {
            pos->y += 34;
        }
    } else {
        pos->x += (capsuleNo*24)-24;
        pos->y += 34;
    }
}

void MBCapsuleSpriteShow(int file, int xPos, int yPos, BOOL fastF)
{
    int time; //r31
    int mode; //r30
    int sprId; //r29
    BOOL endF; //r26
    
    float t; //f31
    float scale; //f30
    
    mode = 0;
    time = 0;
    endF = FALSE;
    do {
        switch(mode) {
            case 0:
                sprId = MBCapsuleSprCreate(file, 100, 0);
                espPosSet(sprId, xPos, yPos);
                espScaleSet(sprId, 0, 0);
                mode++;
                time = 0;
                break;
            
            case 1:
                if(!fastF) {
                    t = ++time/60.0f;
                    scale = t+(HuSin(t*180)*0.5);
                } else {
                    t = ++time/30.0f;
                    scale = t+(HuSin(t*180)*0.5);
                }
                espScaleSet(sprId, scale, scale);
                if(t >= 1.0f) {
                    espScaleSet(sprId, 1, 1);
                    mode++;
                    time = 0;
                }
                break;
            
            case 2:
                if(!fastF) {
                    t = ++time/30.0f;
                    if(t >= 1.0f) {
                        mode++;
                        time = 0;
                    }
                } else {
                    t = ++time/60.0f;
                    scale = 1+(HuSin(t*720)*0.2f);
                    espScaleSet(sprId, scale, scale);
                    if(t >= 1.0f) {
                        mode++;
                        time = 0;
                    }
                }
                break;
            
            case 3:
                t = ++time/30.0f;
                scale = 1+t;
                espTPLvlSet(sprId, 1-t);
                espScaleSet(sprId, scale, scale);
                if(t >= 1.0f) {
                    mode++;
                    time = 0;
                }
                break;
            
            case 4:
                espDispOff(sprId);
                endF = TRUE;
                break;
        }
        HuPrcVSleep();
    } while(!endF);
}

static OMOBJ *moveObj[GW_PLAYER_MAX];
typedef struct PlayerMoveWork_s {
    int playerNo;
    int mode;
    int startMotNo;
    int shiftMotNo;
    BOOL startMotF;
    BOOL shiftMotF;
    float minY;
    float gravity;
    HuVecF pos;
    HuVecF vel;
} PLAYER_MOVE_WORK;

void MBPlayerMoveInit(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        moveObj[i] = NULL;
    }
}

void MBPlayerMoveObjCreate(void)
{
    PLAYER_MOVE_WORK *work;
    int i;
    OMOBJ *obj;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(MBPlayerAliveCheck(i)) {
            obj = moveObj[i] = MBAddObj(32768, 0, 0, MBPlayerMoveObjExec);
            work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(PLAYER_MOVE_WORK), HU_MEMNUM_OVL);
            memset(work, 0, sizeof(PLAYER_MOVE_WORK));
            work->playerNo = i;
            work->mode = 255;
            work->startMotF = FALSE;
            work->shiftMotF = FALSE;
            work->startMotNo = MB_PLAYER_MOT_SHOCK;
            work->shiftMotNo = MB_PLAYER_MOT_DIZZY;
        }
    }
}

void MBPlayerMoveHitCreate(int playerNo, BOOL startMotF, BOOL shiftMotF)
{
    PLAYER_MOVE_WORK *work;
    OMOBJ *obj;
    if(!MBPlayerAliveCheck(playerNo)) {
        return;
    }
    if(moveObj[playerNo] == NULL) {
        obj = moveObj[playerNo] = MBAddObj(32768, 0, 0, MBPlayerMoveObjExec);
        work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(PLAYER_MOVE_WORK), HU_MEMNUM_OVL);
        memset(work, 0, sizeof(PLAYER_MOVE_WORK));
        if(startMotF) {
            work->startMotNo = MB_PLAYER_MOT_SHOCK;
        } else {
            work->startMotNo = MB_PLAYER_MOT_NONE;
        }
        if(shiftMotF) {
            work->shiftMotNo = MB_PLAYER_MOT_DIZZY;
        } else {
            work->shiftMotNo = MB_PLAYER_MOT_NONE;
        }
    } else {
        obj = moveObj[playerNo];
        work = obj->data;
    }
    work->playerNo = playerNo;
    work->mode = 0;
    work->startMotF = startMotF;
    work->shiftMotF = shiftMotF;
    MBPlayerPosGet(playerNo, &work->pos);
    work->minY = work->pos.y;
    work->gravity = 6.5333333f;
    work->vel.x = work->vel.z = 0;
    work->vel.y = 60.000004f;
    if(work->startMotF && work->startMotNo != MB_PLAYER_MOT_NONE) {
        work->mode = 0;
        MBPlayerMotionNoSet(playerNo, work->startMotNo, HU3D_MOTATTR_NONE);
    } else {
        work->mode = 1;
        if(work->shiftMotF && work->shiftMotNo != MB_PLAYER_MOT_NONE) {
            MBPlayerMotionNoShiftSet(playerNo, work->shiftMotNo, 0, 8, HU3D_MOTATTR_LOOP);
        }
    }
}

void MBPlayerMoveEjectCreate(int playerNo, BOOL shiftMotF)
{
    PLAYER_MOVE_WORK *work;
    OMOBJ *obj;
    if(!MBPlayerAliveCheck(playerNo)) {
        return;
    }
    if(moveObj[playerNo] == NULL) {
        obj = moveObj[playerNo] = MBAddObj(32768, 0, 0, MBPlayerMoveObjExec);
        work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(PLAYER_MOVE_WORK), HU_MEMNUM_OVL);
        memset(work, 0, sizeof(PLAYER_MOVE_WORK));
        work->startMotNo = MB_PLAYER_MOT_NONE;
        if(shiftMotF) {
            work->shiftMotNo = MB_PLAYER_MOT_DIZZY;
        } else {
            work->shiftMotNo = MB_PLAYER_MOT_NONE;
        }
    } else {
        obj = moveObj[playerNo];
        work = obj->data;
    }
    work->playerNo = playerNo;
    work->mode = 0;
    work->startMotF = TRUE;
    work->shiftMotF = shiftMotF;
    MBPlayerPosGet(playerNo, &work->pos);
    work->minY = work->pos.y;
    work->gravity = 6.5333333f;
    work->vel.x = work->vel.z = 0;
    work->vel.y = 80.0f;
    work->mode = 0;
}

void MBPlayerMoveMinYSet(int playerNo, float minY)
{
    OMOBJ *obj;
    PLAYER_MOVE_WORK *work;
    obj = moveObj[playerNo];
    if(obj != NULL) {
        work = obj->data;
        work->minY = minY;
    }
}

void MBPlayerMoveVelSet(int playerNo, float gravity, HuVecF vel)
{
    OMOBJ *obj;
    PLAYER_MOVE_WORK *work;
    obj = moveObj[playerNo];
    if(obj != NULL) {
        work = obj->data;
        work->gravity = gravity;
        work->vel = vel;
    }
}

void MBPlayerMoveObjExec(OMOBJ *obj)
{
    PLAYER_MOVE_WORK *work = obj->data;
    switch(work->mode) {
        case 0:
            VECAdd(&work->pos, &work->vel, &work->pos);
            work->vel.y -= work->gravity;
            if(work->pos.y <= work->minY) {
                work->pos.y = work->minY;
                work->mode++;
                if(work->shiftMotF && work->shiftMotNo != MB_PLAYER_MOT_NONE) {
                    MBPlayerMotionNoShiftSet(work->playerNo, work->shiftMotNo, 0, 8, HU3D_MOTATTR_LOOP);
                }
            }
            MBPlayerPosSetV(work->playerNo, &work->pos);
            break;
        
        case 1:
            break;
    }
    if(MBKillCheck() || moveObj[work->playerNo] == NULL) {
        HuMemDirectFree(work);
        obj->data = NULL;
        omDelObjEx(mbObjMan, obj);
        moveObj[work->playerNo] = NULL;
    }
}

BOOL MBPlayerMoveObjCheck(int playerNo)
{
    OMOBJ *obj;
    PLAYER_MOVE_WORK *work;
    obj = moveObj[playerNo];
    if(obj == NULL) {
        return TRUE;
    }
    work = obj->data;
    if(work->mode >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void MBPlayerMoveObjKill(int playerNo)
{
    moveObj[playerNo] = NULL;
}

void MBPlayerMoveObjClose(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        moveObj[i] = NULL;
    }
}

void MBCapsuleMiracleGlowExec(OMOBJ *obj)
{
    
}

BOOL MBCapsuleMiracleGlowCheck(OMOBJ *obj)
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

void MBCapsuleExplodeEffCircleAdd(int no, HuVecF pos, float radius, float scale, float angle)
{
    
}

void MBCapsuleDustCloudAdd(int no, HuVecF pos)
{
    
}

void MBCapsuleDustExplodeAdd(int no, HuVecF pos)
{
    
}

void MBCapsuleDustHeavyAdd(int no, HuVecF pos)
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

void MBCapsuleSnowEffCreate(void)
{
    
}

void MBCapsuleSnowEffKill(void)
{
    
}

int MBCapsuleSnowEffAdd(HuVecF *pos, int maxTime)
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

void MBCapsuleGlowEffAnimSet(int dataNum)
{
    
}

int MBCapsuleGlowEffAdd(HuVecF pos, HuVecF vel, float scale, float fadeSpeed, float rotSpeed, float gravity, GXColor color)
{
    
}

void MBCapsuleHanachanGlowEffAdd(HuVecF pos, float scale, float fadeSpeed, float radiusX, float radiusY, float radiusZ, int mode)
{
    
}

int MBCapsuleMiracleGlowEffAdd(HuVecF pos, float scale, float fadeSpeed, float radiusX, float radiusY, float radiusZ, int mode)
{
    
}

void MBCapsuleRingEffCreate(void)
{
    
}

void MBCapsuleRingEffKill(void)
{
    
}

int MBCapsuleRingEffNumGet(void)
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

int MBCapsuleRayEffNumGet(void)
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


void MBCapsuleCoinEffCreate(void)
{
    
}

void MBCapsuleCoinEffKill(void)
{
    
}

int MBCapsuleCoinEffNumGet(void)
{
    
}

int MBCapsuleCoinEffAdd(HuVecF *pos, HuVecF *rot, float scale, float gravity, int maxTime, int mode)
{
    
}

int MBCapsuleCoinEffMaxYSet(int id, float maxY)
{
    
}

void MBCapsuleCoinManCreate(void)
{
    
}

void MBCapsuleCoinManKill(void)
{
    
}

int MBCapsuleCoinManNumGet(void)
{
    
}

int MBCapsuleCoinManAdd(BOOL downF, int playerNo, int num)
{
    
}

void MBCapsuleStarManCreate(void)
{
    
}

void MBCapsuleStarManKill(void)
{
    
}

int MBCapsuleStarManAdd(BOOL downF, int playerNo, int num)
{
    
}

int MBCapsuleStarManNumGet(void)
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

OMOBJ *MBCapsuleHoneObjCreate(HuVecF *pos, int mdlId, int masuId, int dispDelay, BOOL stompF)
{
    
}

OMOBJ *MBCapsuleSaiHiddenCreate(int playerNo, int mdlId, int effDelay, BOOL rotF, BOOL skipF)
{
    
}

void MBCapsuleSaiHiddenKill(OMOBJ *obj)
{
    
}

BOOL MBCapsuleSaiHiddenKillCheck(OMOBJ *obj)
{
    
}

BOOL MBCapsuleSaiHiddenHitCheck(OMOBJ *obj, HuVecF *pos)
{
    
}

BOOL MBCapsuleSaiHiddenInCheck(OMOBJ *obj)
{
    
}

void MBCapsuleSaiHiddenStartSet(OMOBJ *obj, BOOL startF)
{
    
}

void MBCapsuleChanceSprCreate(void)
{
    
}

void MBCapsuleChanceSprKill(void)
{
    
}

void MBCapsuleChanceSprAdd(HuVecF *pos, int no)
{
    
}

void MBCapsuleChanceSprRotStart(void)
{
    
}

void MBCapsuleChanceSprRemove(void)
{
    
}

int MBCapsuleStarNumCreate(int playerNo, int num)
{
    
}

int MBCapsuleStarNumCheck(int id)
{
    
}

static HU3DPARMANID CapsuleStarEffCreate(ANIMDATA *animP)
{
    
}

BOOL MBCapsulePlayerMotShiftCheck(int playerNo)
{
    
}

void MBCapsuleModelMotSet(int mdlId, int motNo, u32 attr, BOOL shiftF)
{
    
}

void MBCapsulePlayerMotSet(int playerNo, int motNo, u32 attr, BOOL shiftF)
{
    
}

int MBCapsulePlayerSquishSet(int *playerNo, int masuId)
{
    
}

int MBCapsulePlayerSquishVoiceSet(int *playerNo, int masuId, BOOL voiceF)
{
    
}

void MBCapsulePlayerStunSet(int *playerNo, int playerNum, int type)
{
    
}

void MBCapsulePlayerIdleWait(void)
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

void MBCapsuleVibrate(int type)
{
    
}

BOOL MBCapsuleTeamCheck(int player1, int player2)
{
    
}

u32 MBCapsuleTeamNameGet(s16 charNo1, s16 charNo2)
{
    
}

int MBCapsulePlayerAliveFind(int playerNo)
{
    
}

int MBCapsuleCoinDispExec(int playerNo, int coinNum, BOOL winMotF, BOOL waitF)
{
    
}

void MBCapsuleChoiceSet(int choice)
{
    
}

void MBCapsuleModelMtxSet(MBMODELID mdlId, Mtx *matrix)
{
    
}

s16 MBCapsuleMasuNextGet(s16 masuId, HuVecF *pos)
{
    
}

s16 MBCapsuleMasuNextRegularGet(s16 masuId, HuVecF *pos)
{
    
}

int MBCapsulePlayerRandGet(int playerNo, int type)
{
    
}

int MBCapsulePlayerSameRandGet(int playerNo, int type, BOOL sameF)
{
    
}

void MBCapsuleTimingHookCreate(HU3DMODELID modelId, HU3DMOTID motId, BOOL lagF)
{
    
}

void MBCapsuleSeDelayPlay(int seId, int delay)
{
    
}

float MBCapsuleAngleWrap(float a, float b)
{
    
}

float MBCapsuleAngleLerp(float a, float b, float t)
{
    
}

float MBCapsuleAngleSumLerp(float t, float a, float b)
{
    
}

float MBCaspuleAngleRotCamera(float angle)
{
    
}

void MBCapsuleHermiteGetV(float t, HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, HuVecF *out)
{
    
}

void MBCapsuleBezierGetV(float t, float *a, float *b, float *c, float *out)
{
    
}

void MBCapsuleBezierNormGetV(float t, float *a, float *b, float *c, float *out)
{
    
}