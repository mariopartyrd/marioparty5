#define MBPAUSE_ATTRSET_HACK
#include "game/board/pause.h"
#include "game/board/main.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/audio.h"
#include "game/board/window.h"
#include "game/board/camera.h"
#include "game/board/tutorial.h"
#include "game/board/effect.h"

#include "game/disp.h"
#include "game/hsfex.h"

#include "game/pad.h"
#include "game/wipe.h"

#include "game/esprite.h"

#include "game/sprite.h"
#include "game/hu3d.h"
#include "game/flag.h"

#include "datanum/w20.h"
#include "messnum/boardope.h"
#include "messnum/board_pause.h"

#define PADOPT_DIF_MAN 4

typedef struct PauseOption_s {
    int no;
    int value;
    int mdlNum;
    int sprNum;
    float mdlScaleEnd;
    float mdlScale;
    float sprScaleEnd;
    float sprScale;
    HuVecF pos;
    HuVecF posEnd;
    HuVecF posSpr;
    MBMODELID mdlId[5];
    s16 sprId[4];
} PAUSE_OPTION;

typedef struct PausePadOption_s {
    int padNo;
    int no;
    int dif;
    int padStat;
    float mdlScaleEnd;
    float mdlScale;
    float sprScaleEnd;
    float sprScale;
    ANIMDATA *contCharAnim;
    HU3DANIMID toueiAnimId;
    HuVecF pos;
    HuVecF posSpr;
    MBMODELID mdlId[1];
    s16 sprId[2];
} PAUSE_PADOPTION;

typedef struct PauseCursor_s {
    HuVecF pos;
    HuVecF vel;
    HuVecF accel;
    BOOL dispF;
    int cursorType;
    int dStkBit;
    int moveBit;
    int moveTimer;
    float tplvl[4];
    s16 sprId[4];
    s16 glowSprId[4];
} PAUSE_CURSOR;

typedef struct PausePadStat_s {
    int padNo;
    int no;
    int stat;
} PAUSE_PADSTAT;

typedef struct PauseWork_s {
    int playerNo;
    int cursorPos;
    int padCursorPos;
    int cursorDelay;
    int idleDelay;
    BOOL padSelF;
    BOOL guideF;
    BOOL exitF;
    int helpWinId;
    int winId;
    int unk28[7];
    int capsuleMdlId;
    PAUSE_OPTION option[8];
    PAUSE_PADOPTION padOption[GW_PLAYER_MAX];
    PAUSE_CURSOR cursor;
    PAUSE_PADSTAT padStat[GW_PLAYER_MAX];
    ANIMDATA *contBackAnim;
    HU3DMODELID contBackMdlId;
    s16 contBackShowF;
    Mtx posMtx;
    Mtx optionMtx;
} PAUSE_WORK;

static PAUSE_WORK pauseWork;

static BOOL pauseWriteF;
static BOOL pauseResult;
static void *pauseDispCopyBuf;
static BOOL pauseDispCopyF;
static int pauseDispCopyMdl;
static HUPROCESS *pauseOptionProc;

static int pauseSaveValTbl[3] = {
    GW_SAVEMODE_ALL,
    GW_SAVEMODE_TURN,
    GW_SAVEMODE_NONE
};

static HuVecF guidePos = { 84, 376, 1000 };

static s16 pausePosTbl[8][2] = {
    { 0, 0 },
    { 1, 0 },
    { 2, 0 },
    { 3, 0 },
    { 0, 1 },
    { 1, 1 },
    { 2, 1 },
    { 3, 1 },
};

static unsigned int optionMdlFileTbl[8][5] = {
    {
        BPAUSE_HSF_cont
    },
    {
        BPAUSE_HSF_mgInstOn,
        BPAUSE_HSF_mgInstOff
    },
    {
        BPAUSE_HSF_mgComOn,
        BPAUSE_HSF_mgComOff
    },
    {
        BPAUSE_HSF_mgPackAll,
        BPAUSE_HSF_mgPackEasy,
        BPAUSE_HSF_mgPackAction,
        BPAUSE_HSF_mgPackSkill,
        BPAUSE_HSF_mgPackGoofy
    },
    {
        BPAUSE_HSF_rumbleOn,
        BPAUSE_HSF_rumbleOff
    },
    {
        BPAUSE_HSF_messSpeedFast,
        BPAUSE_HSF_messSpeedMid,
        BPAUSE_HSF_messSpeedSlow
    },
    {
        BPAUSE_HSF_saveAlways,
        BPAUSE_HSF_saveOnce,
        BPAUSE_HSF_saveNever
    },
    {
        BPAUSE_HSF_terl
    },
};

static unsigned int optionSprFileTbl[8][4] = {
    {
        BPAUSE_ANM_contOption,
        BPAUSE_ANM_batsu
    },
    {
        BPAUSE_ANM_mgFlag
    },
    {
        BPAUSE_ANM_mgFlag
    },
    {
        BPAUSE_ANM_mgPack
    },
    {
        BPAUSE_ANM_rumbleValue
    },
    {
        BPAUSE_ANM_messSpeed
    },
    {
        BPAUSE_ANM_saveMode
    },
    {
        BPAUSE_ANM_quit
    }
};

static u32 optionMessTbl[8][5] = {
    {
        MESS_BOARD_PAUSE_PLAYER_CONFIG,
        MESS_BOARD_PAUSE_PLAYER_TYPE_MAN,
        MESS_BOARD_PAUSE_PLAYER_TYPE_COM
    },
    {
        MESS_BOARD_PAUSE_MGINST_ON,
        MESS_BOARD_PAUSE_MGINST_OFF
    },
    {
        MESS_BOARD_PAUSE_MGCOM_ON,
        MESS_BOARD_PAUSE_MGCOM_OFF
    },
    {
        MESS_BOARD_PAUSE_MGPACK_ALL,
        MESS_BOARD_PAUSE_MGPACK_EASY,
        MESS_BOARD_PAUSE_MGPACK_ACTION,
        MESS_BOARD_PAUSE_MGPACK_SKILL,
        MESS_BOARD_PAUSE_MGPACK_GOOFY
    },
    {
        MESS_BOARD_PAUSE_RUMBLE_ON,
        MESS_BOARD_PAUSE_RUMBLE_OFF
    },
    {
        MESS_BOARD_PAUSE_MESS_SPEED_FAST,
        MESS_BOARD_PAUSE_MESS_SPEED_MID,
        MESS_BOARD_PAUSE_MESS_SPEED_SLOW
    },
    {
        MESS_BOARD_PAUSE_SAVE_ALWAYS,
        MESS_BOARD_PAUSE_SAVE_ONCE,
        MESS_BOARD_PAUSE_SAVE_NEVER
    },
    {
        MESS_BOARD_PAUSE_MES_QUIT,
        MESS_BOARD_PAUSE_MES_QUIT_CONFIRM
    },
};

static char *optionHookTbl[8][5] = {
    {
        "ys54_000-itemhook"
    },
    {
        "ys54_010-itemhook",
        "ys54_011-itemhook"
    },
    {
        "ys54_020-itemhook",
        "ys54_021-itemhook"
    },
    {
        "ys54_030-itemhook",
        "ys54_031-itemhook",
        "ys54_032-itemhook",
        "ys54_033-itemhook",
        "ys54_034-itemhook"
    },
    {
        "ys54_040-itemhook",
        "ys54_041-itemhook"
    },
    {
        "ys54_050-itemhook",
        "ys54_051-itemhook",
        "ys54_052-itemhook"
    },
    {
        "ys54_060-itemhook",
        "ys54_061-itemhook",
        "ys54_062-itemhook"
    },
    {
        "ys54_070-itemhook"
    },
};

static void PauseCommonRead(void);
static void PauseCommonWrite(void);
static void PauseOptionProcExec(void);
static void PauseOptionDestroy(void);

BOOL MBPauseOptionExec(int playerNo)
{
    pauseWriteF = FALSE;
    pauseResult = FALSE;
    memset(&pauseWork, 0, sizeof(pauseWork));
    pauseWork.helpWinId = pauseWork.winId = MBWIN_NONE;
    pauseWork.playerNo = playerNo;
    pauseWork.cursorPos = -1;
    PauseCommonRead();
    MBCameraLookAtGetInv(pauseWork.posMtx);
    MTXCopy(pauseWork.posMtx, pauseWork.optionMtx);
    pauseWork.optionMtx[0][3] = 0;
    pauseWork.optionMtx[1][3] = 0;
    pauseWork.optionMtx[2][3] = 0;
    pauseOptionProc = MBPrcCreate(PauseOptionProcExec, 8209, 14336);
    HuPrcSetStat(pauseOptionProc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    MBPrcDestructorSet(pauseOptionProc, PauseOptionDestroy);
    while(!pauseWriteF) {
        HuPrcVSleep();
    }
    PauseCommonWrite();
    return pauseResult;
}

static void PauseOptionDestroy(void)
{
    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
        PAUSE_OPTION *opt;
        int i;
        int j;
        for(opt=&pauseWork.option[0], i=0; i<8; i++, opt++) {
            for(j=0; j<5; j++) {
                if(opt->mdlId[j] >= 0) {
                    MBModelKill(opt->mdlId[j]);
                    opt->mdlId[j] = MB_MODEL_NONE;
                }
            }
            for(j=0; j<4; j++) {
                if(opt->sprId[j] >= 0) {
                    espKill(opt->sprId[j]);
                    opt->sprId[j] = -1;
                }
            }
        }
    }
    pauseOptionProc = NULL;
    pauseWriteF = TRUE;
}

static void PauseOptionInit(void);
static void PauseOptionDispUpdate(void);
static void PauseOptionKill(void);
static void PauseOptionMain(void);
static BOOL PauseExitWinExec(void);
static BOOL PadStatCheckAll(BOOL initF);
static void PauseWinPadMesSet(int no);
static void PauseCursorMoveStart(int type, int moveBit, int dStkBit);
static void PauseCursorCreate(void);
static void PauseCursorKill(void);

static void PauseOptionProcExec(void)
{
    int i;
    int guideMdlId = MB_MODEL_NONE;
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        if(PauseExitWinExec()) {
            WipeColorSet(255, 255, 255);
            if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
            }
            WipeWait();
            _SetFlag(FLAG_BOARD_EXIT);
        }
    } else {
        PadStatCheckAll(TRUE);
        PauseOptionInit();
        for(i=0; i<15; i++) {
            PauseOptionDispUpdate();
            HuPrcVSleep();
        }
        guideMdlId = (MBMODELID)MBGuideCreateIn(&guidePos, TRUE, FALSE, FALSE);
        MBModelAttrSet(guideMdlId, HU3D_ATTR_NOPAUSE);
        MBModelLayerSet((MBMODELID)guideMdlId, 6);
        PauseWinPadMesSet(0);
        pauseWork.guideF = TRUE;
        while(!pauseWork.exitF) {
            PauseOptionMain();
            PauseOptionDispUpdate();
            HuPrcVSleep();
        }
        PauseCursorMoveStart(-1, 0, 0);
        if(pauseWork.winId >= 0) {
            MBWinKill(pauseWork.winId);
            pauseWork.winId = MBWIN_NONE;
        }
        PauseWinPadMesSet(-1);
        if(guideMdlId >= 0) {
            MBGuideEnd((MBMODELID)guideMdlId);
        }
        for(i=0; i<8; i++) {
            pauseWork.option[i].mdlScaleEnd = 0;
            pauseWork.option[i].sprScaleEnd = 0;
        }
        PauseOptionKill();
        PauseCursorKill();
        MBEffKill(pauseWork.contBackMdlId);
        MBModelKill(pauseWork.capsuleMdlId);
    }
    pauseWork.guideF = FALSE;
    HuPrcEnd();
}

static void PauseOptionModelInit(int modelId, HuVecF *pos, float scale);
static void PausePadBGEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix);
static HuVecF *PauseOptionPosGet(HuVecF *in);

static void PauseOptionInit(void)
{
    PAUSE_OPTION *opt;
    int j;
    int i;
    HuVecF pos;
    for(opt=&pauseWork.option[0], i=0; i<8; i++, opt++) {
        for(j=0; j<5; j++) {
            opt->mdlId[j] = MB_MODEL_NONE;
        }
        for(j=0; j<4; j++) {
            opt->sprId[j] = -1;
        }
    }
    if(_CheckFlag(FLAG_MG_CIRCUIT)){
        return;
    }
    pauseWork.padSelF =FALSE;
    pauseWork.capsuleMdlId = MBModelCreate(BPAUSE_HSF_contCapsule, NULL, TRUE);
    MBModelDispSet(pauseWork.capsuleMdlId, TRUE);
    if(_CheckFlag(FLAG_MG_CIRCUIT)){
        return;
    }
    for(opt=&pauseWork.option[0], i=0; i<8; i++, opt++) {
        opt->no = i;
        opt->pos.x = -49.5f+(33.0f*pausePosTbl[i][0]);
        opt->pos.y = 35.2f-(32.0f*pausePosTbl[i][1]);
        opt->pos.z = -300;
        opt->posEnd = opt->pos;
        opt->posSpr.x = ((opt->pos.x/159.95999f)+0.5f)*576.0f;
        opt->posSpr.y = ((-opt->pos.y/133.29999f)+0.5f)*480.0f;
        opt->posSpr.z = 0;
        opt->mdlScaleEnd = 0.1f;
        opt->sprScaleEnd = 0.75f;
        for(j=0; j<5; j++) {
            opt->mdlId[j] = MB_MODEL_NONE;
            if(optionMdlFileTbl[i][j]) {
                opt->mdlNum++;
                if(opt->value == j) {
                    int modelId;
                    opt->mdlId[j] = modelId = MBModelCreate(optionMdlFileTbl[i][j], NULL, FALSE);
                    PauseOptionModelInit(modelId, &opt->pos, opt->mdlScale);
                    MBModelHookSet(modelId, optionHookTbl[i][j], pauseWork.capsuleMdlId);
                }
            }
        }
        for(j=0; j<4; j++) {
            opt->sprId[j] = -1;
            if(optionSprFileTbl[i][j]) {
                int sprId;
                opt->sprNum++;
                if(j == 0) {
                    opt->sprId[j] = sprId = espEntry(optionSprFileTbl[i][j], 100, opt->value);
                    espAttrSet(sprId, HUSPR_ATTR_LINEAR);
                    espPosSet(sprId, opt->posSpr.x, opt->posSpr.y+50);
                } else if(GWPartyFGet() == FALSE){
                    opt->sprId[j] = sprId = espEntry(optionSprFileTbl[i][j], 200, 0);
                    espAttrSet(sprId, HUSPR_ATTR_LINEAR);
                    espPosSet(sprId, opt->posSpr.x, opt->posSpr.y);
                    espTPLvlSet(sprId, 0.67f);
                }
            }
        }
        MBModelDispSet(opt->mdlId[opt->value], TRUE);
    }
    PauseCursorCreate();
    pauseWork.contBackAnim = HuSprAnimRead(HuDataReadNum(BPAUSE_ANM_contBack, HU_MEMNUM_OVL));
    pauseWork.contBackMdlId = MBEffCreate(pauseWork.contBackAnim, 1);
    MBEffHookSet(pauseWork.contBackMdlId, PausePadBGEffHook);
    Hu3DModelLayerSet(pauseWork.contBackMdlId, 5);
    Hu3DModelAttrSet(pauseWork.contBackMdlId, HU3D_ATTR_NOPAUSE);
    pos.x = pos.y = pos.z = 0;
    pos.y = 10;
    pos.z = -300;
    Hu3DModelPosSetV(pauseWork.contBackMdlId, PauseOptionPosGet(&pos));
    MTXCopy(pauseWork.optionMtx, Hu3DData[pauseWork.contBackMdlId].mtx);
}

static void PauseOptionKill(void)
{
    PAUSE_OPTION *opt;
    int i;
    int j;
    if(_CheckFlag(FLAG_MG_CIRCUIT)){
        return;
    }
    opt = &pauseWork.option[0];
    if(opt->mdlScaleEnd > 0) {
        opt->posEnd.x = -52.5f;
        opt->posEnd.y = 10;
        opt->mdlScaleEnd *= 1.2f;
    }
    for(i=0; i<18.0f; i++) {
        PauseOptionDispUpdate();
        HuPrcVSleep();
    }
    for(opt=&pauseWork.option[0], i=0; i<8; i++, opt++) {
        if(opt->mdlScaleEnd > 0) {
            continue;
        }
        MBModelHookReset(opt->mdlId[opt->value]);
        MBModelKill(opt->mdlId[opt->value]);
        opt->mdlId[opt->value] = MB_MODEL_NONE;
        for(j=0; j<4; j++){
            if(opt->sprId[j] >= 0) {
                espDispOff(opt->sprId[j]);
            }
        }
    }
}

static void PauseOptionValueInc(PAUSE_OPTION *opt)
{
    int value;
    if(opt->mdlNum <= 1) {
        return;
    }
    value = opt->value;
    opt->value++;
    if(opt->value >= opt->mdlNum) {
        opt->value = 0;
    }
    MBModelHookReset(opt->mdlId[value]);
    MBModelKill(opt->mdlId[value]);
    opt->mdlId[value] = MB_MODEL_NONE;
    if(optionMdlFileTbl[opt->no][opt->value]) {
        int modelId;
        opt->mdlId[opt->value] = modelId = MBModelCreate(optionMdlFileTbl[opt->no][opt->value], NULL, FALSE);
        PauseOptionModelInit(modelId, &opt->pos, opt->mdlScale);
        MBModelHookSet(modelId, optionHookTbl[opt->no][opt->value], pauseWork.capsuleMdlId);
    }
    MBModelDispSet(opt->mdlId[opt->value], TRUE);
    if(opt->sprId[0] >= 0) {
        espBankSet(opt->sprId[0], opt->value);
    }
    PauseOptionDispUpdate();
    HuPrcVSleep();
}

static Mtx *PauseOptionMtxGet(HuVecF *target);

static void PauseOptionDispUpdate(void)
{
    PAUSE_OPTION *opt;
    int j;
    int i;
    Mtx mtx;
    for(opt=&pauseWork.option[0], i=0; i<8; i++, opt++) {
        if(opt->mdlId[opt->value] >= 0) {
            int mdlId = opt->mdlId[opt->value];
            opt->pos.x = opt->pos.x+(0.125f*(opt->posEnd.x-opt->pos.x));
            opt->pos.y = opt->pos.y+(0.125f*(opt->posEnd.y-opt->pos.y));
            opt->posSpr.x = ((opt->pos.x/159.95999f)+0.5f)*576.0f;
            opt->posSpr.y = ((-opt->pos.y/133.29999f)+0.5f)*480.0f;
            opt->mdlScale = opt->mdlScale+(0.25f*(opt->mdlScaleEnd-opt->mdlScale));
            opt->sprScale = opt->sprScale+(0.25f*(opt->sprScaleEnd-opt->sprScale));
            MBModelPosSetV(mdlId, PauseOptionPosGet(&opt->pos));
            MBModelScaleSet(mdlId, opt->mdlScale, opt->mdlScale, opt->mdlScale);
            MTXConcat(pauseWork.optionMtx, *PauseOptionMtxGet(&opt->pos), mtx);
            MBModelMtxSet(mdlId, &mtx);
            for(j=0; j<4; j++) {
                if(opt->sprId[j] >= 0) {
                    if(j == 0) {
                        espScaleSet(opt->sprId[j], opt->sprScale, opt->sprScale);
                        espPosSet(opt->sprId[j], opt->posSpr.x, opt->posSpr.y+50);
                    } else {
                        espScaleSet(opt->sprId[j], opt->sprScale*2.75f, opt->sprScale*2.75f);
                        espPosSet(opt->sprId[j], opt->posSpr.x, opt->posSpr.y);
                    }
                }
            }
        }
    }
}

static void PauseCursorMove(int speed, float posX, float posY);

static int PauseCursorUpdate(void);
static void PauseWinMesSet(int no);

static void PausePadInit(void);
static void PausePadDispUpdate(void);
static BOOL PausePadMain(void);
static void PausePadExit(void);

static void PauseOptionReset(void)
{
    PAUSE_OPTION *opt;
    int j;
    int i;
    for(opt=&pauseWork.option[0], i=0; i<8; i++, opt++) {
        if(opt->mdlId[opt->value] < 0) {
            int mdlId;
            opt->mdlScaleEnd = 0.1f;
            opt->sprScaleEnd = 0.75f;
            j = opt->value;
            opt->mdlId[j] = mdlId = MBModelCreate(optionMdlFileTbl[i][j], NULL, FALSE);
            PauseOptionModelInit(mdlId, &opt->pos, opt->mdlScale);
            MBModelHookSet(mdlId, optionHookTbl[i][j], pauseWork.capsuleMdlId);
            for(j=0; j<4; j++) {
                if(opt->sprId[j] >= 0) {
                    espDispOn(opt->sprId[j]);
                }
            }
            MBModelDispSet(opt->mdlId[opt->value], TRUE);
        }
    }
}

static void PauseOptionMain(void)
{
    int stkDirTbl[8] = {
        PAD_BUTTON_RIGHT|PAD_BUTTON_DOWN,
        PAD_BUTTON_LEFT|PAD_BUTTON_RIGHT|PAD_BUTTON_DOWN,
        PAD_BUTTON_LEFT|PAD_BUTTON_RIGHT|PAD_BUTTON_DOWN,
        PAD_BUTTON_LEFT|PAD_BUTTON_DOWN,
        PAD_BUTTON_RIGHT|PAD_BUTTON_UP,
        PAD_BUTTON_LEFT|PAD_BUTTON_RIGHT|PAD_BUTTON_UP,
        PAD_BUTTON_LEFT|PAD_BUTTON_RIGHT|PAD_BUTTON_UP,
        PAD_BUTTON_LEFT|PAD_BUTTON_UP,
    };
    int padNo = GwPlayer[pauseWork.playerNo].padNo;
    BOOL padOff = HuPadStatGet(padNo) != 0;
    int i;
    if(PauseCursorUpdate() != 0) {
        return;
    } else {
        int cursorPos = pauseWork.cursorPos;
        int prevPos = cursorPos;
        int stkDir;
        if(cursorPos < 0) {
            cursorPos = 0;
        }
        stkDir = HuPadDStkRep[padNo] & stkDirTbl[cursorPos];
        if(padOff) {
            stkDir = 0;
        }
        if(stkDir & PAD_BUTTON_UP) {
            MBAudFXPlay(MSM_SE_CMN_01);
            cursorPos = (cursorPos+4)&0x7;
        } else if(stkDir & PAD_BUTTON_DOWN) {
            MBAudFXPlay(MSM_SE_CMN_01);
            cursorPos = (cursorPos-4)&0x7;
        }
        if(stkDir & PAD_BUTTON_RIGHT) {
            MBAudFXPlay(MSM_SE_CMN_01);
            cursorPos = (cursorPos+1)&0x7;
        } else if(stkDir & PAD_BUTTON_LEFT) {
            MBAudFXPlay(MSM_SE_CMN_01);
            cursorPos = (cursorPos-1)&0x7;
        }
        pauseWork.cursorPos = cursorPos;
        if(prevPos >= 0) {
            pauseWork.option[prevPos].mdlScaleEnd = 0.1f;
            pauseWork.option[prevPos].sprScaleEnd = 0.75f;
            PauseCursorMoveStart(0, stkDirTbl[cursorPos], HuPadDStk[padNo]);
        } else { 
            PauseCursorMoveStart(0, stkDirTbl[cursorPos], HuPadDStk[padNo]);
        }
        pauseWork.option[pauseWork.cursorPos].mdlScaleEnd = 0.13f;
        pauseWork.option[pauseWork.cursorPos].sprScaleEnd = 1.0f;
        if(prevPos != cursorPos) {
            PauseCursorMove(15, pauseWork.option[cursorPos].posSpr.x, pauseWork.option[cursorPos].posSpr.y);
        }
        {
            BOOL statChangeF = PadStatCheckAll(FALSE);
            if(pauseWork.cursorDelay > 0){
                pauseWork.cursorDelay--;
            }
            if(pauseWork.cursorDelay == 0) {
                pauseWork.cursorDelay = -1;
                prevPos = -1;
            }
            if(prevPos != cursorPos || (cursorPos == 0 && statChangeF)) {
                pauseWork.cursorDelay = -1;
                PauseWinMesSet(cursorPos);
            }
        }
        if((HuPadBtnDown[padNo] & PAD_BUTTON_A) && !padOff) {
            pauseWork.padSelF = TRUE;
        }
        if(pauseWork.idleDelay == 0 && pauseWork.padSelF) {
            pauseWork.idleDelay = 10;
            pauseWork.padSelF = FALSE;
            if(cursorPos == 0) {
                if(GWPartyFGet() != FALSE) {
                    MBAudFXPlay(MSM_SE_CMN_02);
                    for(i=1; i<8; i++){
                        pauseWork.option[i].mdlScaleEnd = 0;
                        pauseWork.option[i].sprScaleEnd = 0;
                    }
                    PauseCursorMoveStart(-1, 0, 0);
                    PauseOptionKill();
                    pauseWork.cursorDelay = -1;
                    PausePadInit();
                    while(!PausePadMain()){
                        PausePadDispUpdate();
                        HuPrcVSleep();
                        if(pauseWork.cursorDelay > 0) {
                            pauseWork.cursorDelay--;
                        }
                        if(pauseWork.cursorDelay == 0) {
                            pauseWork.cursorDelay = -1;
                            PauseWinMesSet(cursorPos);
                        }
                    }
                    MBAudFXPlay(MSM_SE_CMN_04);
                    PauseCursorMoveStart(-1, 0, 0);
                    PausePadExit();
                    pauseWork.padSelF = FALSE;
                    if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
                        PauseOptionReset();
                    }
                    pauseWork.cursorPos = -1;
                } else {
                    MBAudFXPlay(MSM_SE_CMN_05);
                }
            } else {
                if(cursorPos == 7) {
                    MBAudFXPlay(MSM_SE_CMN_02);
                    if(PauseExitWinExec()) {
                        WipeColorSet(255, 255, 255);
                        if(!_CheckFlag(FLAG_BOARD_TUTORIAL) || !MBTutorialExitFlagGet()) {
                            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
                        }
                        WipeWait();
                        _SetFlag(FLAG_BOARD_EXIT);
                        pauseWork.exitF = TRUE;
                    } else {
                        pauseWork.padSelF = FALSE;
                        if(!_CheckFlag(FLAG_MG_CIRCUIT)) {
                            PauseOptionReset();
                        }
                        PauseWinPadMesSet(0);
                        pauseWork.cursorPos = -1;
                    }
                } else {
                    if((GWPartyFGet() == FALSE || !SLSaveFlagGet()) && cursorPos == 6) {
                        MBAudFXPlay(MSM_SE_CMN_05);
                    } else {
                        MBAudFXPlay(MSM_SE_CMN_02);
                        PauseOptionValueInc(&pauseWork.option[cursorPos]);
                        prevPos = -1;
                        pauseWork.cursorDelay = 15;
                        if(cursorPos == 5) {
                            GWMessSpeedSet(pauseWork.option[5].value);
                        } else if(cursorPos == 4){
                            GWRumbleFSet(pauseWork.option[4].value == 0);
                            omVibrate(pauseWork.playerNo, 12, 4, 2);
                        }
                    }
                }
            }
        }
        if((HuPadBtnDown[padNo] & PAD_BUTTON_B) && !padOff){
            if(!pauseWork.exitF) {
                pauseWork.exitF = TRUE;
                MBAudFXPlay(MSM_SE_CMN_04);
            }
        }
        if(pauseWork.idleDelay) {
            pauseWork.idleDelay--;
        }
    }
}

static BOOL PauseExitWinExec(void)
{
    int sprId[3];
    int sprFile[3] = {
        BPAUSE_ANM_quitLine,
        BPAUSE_ANM_quitBack,
        BPAUSE_ANM_quitLine
    };
    s16 sprPos[3][2] = {
        { 288, 146 },
        { 288, 200 },
        { 288, 254 },
    };
    HuVec2F sprScale[3] = {
        { 72, 1 },
        { 1, 12.5f },
        { 72, -1 },
    };
    float sprTPLvl[3] = {
        1.0f,
        0.8f,
        1.0f
    };
    int i;
    int winId;
    int choice;
    BOOL type;
    
    MBWIN *winP;
    
    int padStat;
    
    for(i=0; i<8; i++){
        pauseWork.option[i].mdlScaleEnd = 0;
        pauseWork.option[i].sprScaleEnd = 0;
    }
    PauseCursorMoveStart(-1, 0, 0);
    PauseWinMesSet(-1);
    PauseOptionKill();
    for(i=0; i<3; i++){
        sprId[i] = espEntry(sprFile[i], 200, 0);
        espAttrSet(sprId[i], HUSPR_ATTR_LINEAR);
        espPosSet(sprId[i], sprPos[i][0], sprPos[i][1]);
        espScaleSet(sprId[i], sprScale[i].x, sprScale[i].y);
        espTPLvlSet(sprId[i], sprTPLvl[i]);
    }
    espColorSet(sprId[1], 128, 128, 128);
    type = GwPlayer[pauseWork.playerNo].comF;
    GwPlayer[pauseWork.playerNo].comF = FALSE;
    GwPlayerConf[pauseWork.playerNo].type = GW_TYPE_MAN;
    padStat = HuPadStatGet(GwPlayer[pauseWork.playerNo].padNo);
    winId = MBWinCreateChoice(MBWIN_TYPE_TALKEVENT, MESS_BOARD_PAUSE_MES_QUIT_CONFIRM, HUWIN_SPEAKER_NONE, 0);
    MBWinPosSet(winId, 150, 150);
    MBWinPriSet(winId, 100);
    MBWinPlayerDisable(winId, pauseWork.playerNo);
    winP = MBWinGet(winId);
    winP->type = MBWIN_TYPE_HELP;
    PauseWinPadMesSet(1);
    if(!padStat) {
        MBWinAttrReset(winId, HUWIN_ATTR_NOCANCEL);
        MBWinWait(winId);
        choice = MBWinChoiceGet(winId);
    } else {
        choice = 0;
    }
    MBWinKill(winId);
    PauseWinPadMesSet(-1);
    GwPlayer[pauseWork.playerNo].comF = type;
    GwPlayerConf[pauseWork.playerNo].type = type;
    for(i=0; i<3; i++) {
        espKill(sprId[i]);
    }
    pauseResult = choice == 1;
    return pauseResult;
}

static void PauseWinMesSet(int no)
{
    int i;
    HuVec2F pos;
    if(pauseWork.winId >= 0) {
        MBWinKill(pauseWork.winId);
        pauseWork.winId = MBWIN_NONE;
    }
    if(no < 0) {
        return;
    }
    if(no == 0) {
        pauseWork.winId = MBWinCreate(MBWIN_TYPE_PAUSEGUIDE, optionMessTbl[no][0], HUWIN_SPEAKER_NONE);
        for(i=0; i<4; i++) {
            MBWinInsertMesSet(pauseWork.winId, optionMessTbl[no][((GwPlayer[pauseWork.padStat[i].no].comF) ? 2 : 1)], i);
        }
    } else{
        pauseWork.winId = MBWinCreate(MBWIN_TYPE_PAUSEGUIDE, optionMessTbl[no][pauseWork.option[no].value], HUWIN_SPEAKER_NONE);
    }
    MBWinPosGet(pauseWork.winId, &pos);
    MBWinPosSet(pauseWork.winId, 160, pos.y);
    MBWinMesMaxSizeGet(pauseWork.winId, &pos);
    MBWinSizeSet(pauseWork.winId, 380, pos.y);
    MBWinPause(pauseWork.winId);
}

static void PauseWinPadMesSet(int no)
{
    u32 mesTbl[2] = {
        MESS_BOARD_PAUSE_PAD_OPTION,
        MESS_BOARD_PAUSE_PAD_QUIT
    };
    HuVec2F pos;
    if(pauseWork.helpWinId >= 0) {
        MBWinKill(pauseWork.helpWinId);
        pauseWork.helpWinId = MBWIN_NONE;
    }
    if(no < 0) {
        return;
    }
    if(no >= 2) {
        return;
    }
    pauseWork.helpWinId = MBWinCreateHelp(mesTbl[no]);
    MBWinCenterGet(pauseWork.helpWinId, &pos);
    MBWinPosSet(pauseWork.helpWinId, pos.x, 288);
}

static void PauseCommonRead(void)
{
    int i;
    int saveMode;
    pauseWork.option[0].value = 0;
    pauseWork.option[1].value = (GWMgInstFGet() != FALSE) ? 0 : 1;
    pauseWork.option[2].value = (GWMgComFGet() != FALSE) ? 0 : 1;
    pauseWork.option[3].value = GWMgPackGet();
    pauseWork.option[4].value = (GWRumbleFGet() != FALSE) ? 0 : 1;
    pauseWork.option[5].value = GWMessSpeedGet();
    saveMode = GWSaveModeGet();
    if(GWPartyFGet() == FALSE || SLSaveFlagGet() == FALSE) {
        saveMode = GW_SAVEMODE_NONE;
    }
    pauseWork.option[6].value = 0;
    for(i=0; i<3; i++) {
        if(saveMode == pauseSaveValTbl[i]) {
            pauseWork.option[6].value = i;
        }
    }
}

static void PauseCommonWrite(void)
{
    GWMgInstFSet(pauseWork.option[1].value == 0);
    GWMgComFSet(pauseWork.option[2].value == 0);
    GWMgPackSet(pauseWork.option[3].value);
    GWRumbleFSet(pauseWork.option[4].value == 0);
    GWMessSpeedSet(pauseWork.option[5].value);
    if(GWPartyFGet() != FALSE && SLSaveFlagGet() != FALSE) {
        GWSaveModeSet(pauseSaveValTbl[pauseWork.option[6].value]);
    }
}

static void PausePadInit(void)
{
    PAUSE_PADOPTION *padopt;
    int i;
    int id;
    int j;
    int anmId;
    pauseWork.padCursorPos = -1;
    pauseWork.padSelF = FALSE;
    PadStatCheckAll(FALSE);
    for(padopt=&pauseWork.padOption[0], i=0; i<GW_PLAYER_MAX; i++, padopt++) {
        for(j=0; j<1; j++) {
            padopt->mdlId[j]= MB_MODEL_NONE;
        }
        for(j=0; j<2; j++) {
            padopt->sprId[j]= -1;
        }
        padopt->contCharAnim = NULL;
        padopt->toueiAnimId = -1;
        padopt->no = pauseWork.padStat[i].no;
        padopt->padNo = pauseWork.padStat[i].padNo;
    }
    pauseWork.contBackShowF = TRUE;
    for(padopt=&pauseWork.padOption[0], anmId=HU3D_ANIMID_NONE, i=0; i<GW_PLAYER_MAX; i++, padopt++) {
        padopt->dif = GwPlayer[padopt->no].dif;
        padopt->padStat = HuPadStatGet(padopt->padNo);
        if(!padopt->padStat) { 
            if(!GwPlayer[padopt->no].comF) {
                padopt->dif = PADOPT_DIF_MAN;
            }
        } else {
            GwPlayer[padopt->no].comF = TRUE;
            GwPlayerConf[padopt->no].type = GW_TYPE_COM;
        }
        padopt->pos.x = -20.0f+(25.0f*i);
        padopt->pos.y = 10;
        padopt->pos.z = -300;
        padopt->posSpr.x = ((padopt->pos.x/159.95999f)+0.5f)*576.0f;
        padopt->posSpr.y = ((-padopt->pos.y/133.29999f)+0.5f)*480.0f;
        padopt->posSpr.z = 0;
        padopt->mdlScaleEnd = 0.1f;
        padopt->sprScaleEnd = 0.8f;
        padopt->mdlId[0] = id = MBModelCreate(BPAUSE_HSF_contTouei, NULL, TRUE);
        PauseOptionModelInit(id, &padopt->pos, padopt->mdlScale);
        MBModelScaleSet(padopt->mdlId[0], padopt->mdlScale, padopt->mdlScale, 0.1f);
        padopt->sprId[0] = id = espEntry(BPAUSE_ANM_playerNum, 100, padopt->padNo);
        espAttrSet(id, HUSPR_ATTR_LINEAR);
        espPosSet(id, padopt->posSpr.x, padopt->posSpr.y-35);
        if(padopt->dif < PADOPT_DIF_MAN) {
            espBankSet(id, 4);
        }
        padopt->sprId[1] = id = espEntry(BPAUSE_ANM_comDif, 100, 0);
        espAttrSet(id, HUSPR_ATTR_LINEAR);
        espPosSet(id, padopt->posSpr.x, padopt->posSpr.y-35);
        espDispOff(id);
        if(padopt->dif < PADOPT_DIF_MAN) {
            espBankSet(id, padopt->dif);
        }
        if(i == 0) {
            padopt->contCharAnim = HuSprAnimDataRead(BPAUSE_ANM_contChar);
            anmId = padopt->toueiAnimId = Hu3DAnimCreate(padopt->contCharAnim, MBModelIdGet(padopt->mdlId[0]), "S3TC_touei");
        } else {
            padopt->toueiAnimId = Hu3DAnimLink(anmId, MBModelIdGet(padopt->mdlId[0]), "S3TC_touei");
        }
        Hu3DAnmNoSet(padopt->toueiAnimId, GwPlayer[padopt->no].charNo);
        MBModelDispSet(padopt->mdlId[0], TRUE);
    }
}

static void PausePadDispUpdate(void)
{
    PAUSE_PADOPTION *padopt;
    int i;
    for(padopt=&pauseWork.padOption[0], i=0; i<GW_PLAYER_MAX; i++, padopt++) {
        if(padopt->mdlId[0] >= 0){
            int id = padopt->mdlId[0];
            padopt->mdlScale = padopt->mdlScale+(0.25f*(padopt->mdlScaleEnd-padopt->mdlScale));
            padopt->sprScale = padopt->sprScale+(0.25f*(padopt->sprScaleEnd-padopt->sprScale));
            MBModelScaleSet(id, padopt->mdlScale, padopt->mdlScale, padopt->mdlScale);
            if(padopt->sprId[0] >= 0) {
                espScaleSet(padopt->sprId[0], padopt->sprScale, padopt->sprScale);
            }
            if(padopt->sprId[1] >= 0) {
                espScaleSet(padopt->sprId[1], padopt->sprScale, padopt->sprScale);
            }
        }
    }
}

static void PausePadExit(void)
{
    PAUSE_OPTION *opt = &pauseWork.option[0];
    PAUSE_PADOPTION *padopt;
    int i;
    if(opt->mdlScaleEnd > 0) {
        opt->posEnd.x = -49.5f+(33.0f*pausePosTbl[0][0]);
        opt->posEnd.y = 35.2f-(32.0f*pausePosTbl[0][1]);
        opt->mdlScaleEnd = 0.13f;
    }
    pauseWork.contBackShowF = FALSE;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        pauseWork.padOption[i].mdlScaleEnd = pauseWork.padOption[i].sprScaleEnd = 0;
    }
    for(i=0; i<18.0f; i++) {
        PauseOptionDispUpdate();
        PausePadDispUpdate();
        HuPrcVSleep();
    }
    for(padopt=&pauseWork.padOption[0], i=0; i<GW_PLAYER_MAX; i++, padopt++) {
        Hu3DAnimKill(padopt->toueiAnimId);
        padopt->toueiAnimId = HU3D_ANIMID_NONE;
        MBModelKill(padopt->mdlId[0]);
        padopt->mdlId[0] = MB_MODEL_NONE;
        espKill(padopt->sprId[0]);
        espKill(padopt->sprId[1]);
    }
    if(pauseWork.padOption[0].contCharAnim) {
        HuSprAnimKill(pauseWork.padOption[0].contCharAnim);
    }
    pauseWork.padOption[0].contCharAnim = NULL;
}

static BOOL PausePadMain(void)
{
    BOOL result = FALSE;
    int stkDirTbl[GW_PLAYER_MAX] = {
        PAD_BUTTON_RIGHT,
        PAD_BUTTON_RIGHT|PAD_BUTTON_LEFT,
        PAD_BUTTON_RIGHT|PAD_BUTTON_LEFT,
        PAD_BUTTON_LEFT
    };
    int cursorPos;
    int prevPos;
    int i;
    int padStat;
    int padNo = GwPlayer[pauseWork.playerNo].padNo;
    int stkDir;
    BOOL padOff = HuPadStatGet(padNo) != 0;
    BOOL statChangeF = PadStatCheckAll(0);
    
    if(PauseCursorUpdate() != 0) {
        return FALSE;
    } else {
        PAUSE_PADOPTION *padopt;
        
        //Find better match for this code
        cursorPos = (pauseWork.padCursorPos+1)-1;
        prevPos = cursorPos;
    
        if(cursorPos < 0) {
            pauseWork.padCursorPos = cursorPos = 0;
        }
        stkDir = HuPadDStkRep[padNo] & stkDirTbl[cursorPos];
        if(padOff) {
            stkDir = 0;
        }
        if(stkDir & PAD_BUTTON_RIGHT) {
            MBAudFXPlay(MSM_SE_CMN_01);
            cursorPos = (cursorPos+1)&(GW_PLAYER_MAX-1);
        } else if(stkDir & PAD_BUTTON_LEFT) {
            MBAudFXPlay(MSM_SE_CMN_01);
            cursorPos = (cursorPos-1)&(GW_PLAYER_MAX-1);
        }
        pauseWork.padCursorPos = cursorPos;
        if(cursorPos != prevPos && pauseWork.cursorDelay > 0) {
            pauseWork.cursorDelay = 0;
        }
        padopt = &pauseWork.padOption[pauseWork.padCursorPos];
        if(prevPos >= 0) {
            pauseWork.padOption[prevPos].mdlScaleEnd = 0.1f;
            pauseWork.padOption[prevPos].sprScaleEnd = 0.8f;
            espDispOff(pauseWork.padOption[prevPos].sprId[1]);
            espDispOn(pauseWork.padOption[prevPos].sprId[0]);
        }
        padopt->mdlScaleEnd = 0.120000005f;
        padopt->sprScaleEnd = 1.0f;
        PauseCursorMoveStart(1, stkDirTbl[cursorPos], HuPadDStk[padNo]);
        if(cursorPos != prevPos) {
            PauseCursorMove(15, padopt->posSpr.x, padopt->posSpr.y);
        }
        if(HuPadBtnDown[padNo] & PAD_BUTTON_A){
            pauseWork.padSelF = TRUE;
        }
        if(padOff) {
            pauseWork.padSelF = FALSE;
        }
        if(pauseWork.idleDelay == 0 && pauseWork.padSelF) {
            int maxDif = 4;
            padopt->dif++;
            pauseWork.idleDelay = 10;
            pauseWork.padSelF = FALSE;
            MBAudFXPlay(MSM_SE_CMN_02);
            if(!GwCommon.veryHardUnlock && padopt->dif == maxDif-1) {
                padopt->dif++;
            }
            if(padopt->dif > maxDif) {
                padopt->dif = 0;
            }
            if(padopt->dif == maxDif) {
                if(!HuPadStatGet(padopt->padNo)) {
                    GwPlayer[padopt->no].comF = FALSE;
                    GwPlayerConf[padopt->no].type = GW_TYPE_MAN;
                    espBankSet(padopt->sprId[0], padopt->padNo);
                } else {
                    padopt->dif = 0;
                }
            }
            if(padopt->dif < maxDif) {
                GwPlayer[padopt->no].comF = TRUE;
                GwPlayerConf[padopt->no].type = GW_TYPE_COM;
                GwPlayer[padopt->no].dif = padopt->dif;
                GwPlayerConf[padopt->no].dif = padopt->dif;
                espBankSet(padopt->sprId[1], padopt->dif);
                espBankSet(padopt->sprId[0], 4);
            }
            pauseWork.cursorDelay = 15;
        }
        if(padopt->dif < PADOPT_DIF_MAN) {
            espDispOff(padopt->sprId[0]);
            espDispOn(padopt->sprId[1]);
            espBankSet(padopt->sprId[1], padopt->dif);
        }
        for(padopt=&pauseWork.padOption[0], i=0; i<GW_PLAYER_MAX; i++, padopt++) {
            padStat = HuPadStatGet(padopt->padNo);
            if(padStat && !padopt->padStat) {
                GwPlayer[padopt->no].comF = TRUE;
                GwPlayerConf[padopt->no].type = GW_TYPE_COM;
                padopt->dif = GwPlayer[padopt->no].dif;
                espBankSet(padopt->sprId[1], padopt->dif);
                espBankSet(padopt->sprId[0], 4);
            } else if(!padStat && padopt->padStat) {
                padopt->dif = PADOPT_DIF_MAN;
                GwPlayer[padopt->no].comF = FALSE;
                GwPlayerConf[padopt->no].type = GW_TYPE_MAN;
                espBankSet(padopt->sprId[0], padopt->padNo);
                espDispOn(padopt->sprId[0]);
                espDispOff(padopt->sprId[1]);
            }
            padopt->padStat = padStat;
        }
        if(statChangeF) {
            PauseWinMesSet(pauseWork.cursorPos);
        }
        if((HuPadBtnDown[padNo] & PAD_BUTTON_B) && !padOff){
            result = TRUE;
            if(pauseWork.cursorDelay > 0) {
                pauseWork.cursorDelay = 0;
            }
        }
        if(pauseWork.idleDelay != 0) {
            pauseWork.idleDelay--;
        }
        return result;
    }
}

static void PausePadBGEffHook(HU3DMODEL *modelP, MBEFFECT *effP, Mtx matrix)
{
    MBEFFECTDATA *effDataP;
    int i;
    int a;
    static HuVecF result;
    if(effP->count == 0) {
        for(effDataP=&effP->data[0], i=0; i<effP->num; i++, effDataP++) {
            effDataP->dispF = TRUE;
            effDataP->pos.x = effDataP->pos.y = effDataP->pos.z = 0;
            effDataP->pos.x = effDataP->pos.y = effDataP->pos.z = 0;
            effDataP->scale = 49;
            effDataP->color.r = effDataP->color.g = effDataP->color.b = effDataP->color.a = 255;
            effDataP->color.a = 0;
        }
        effP->count = 1;
        effP->vel.x = effP->vel.y = 0;
    }
    MTXScaleApply(matrix, matrix, 2.9818594f, 1.0f, 1.0f);
    effP->vel.y = 0;
    if(pauseWork.contBackShowF) {
        effP->vel.y = 1;
    }
    effP->vel.x += 0.1875f*(effP->vel.y-effP->vel.x);
    a = effP->vel.x*255;
    for(effDataP=&effP->data[0], i=0; i<effP->num; i++, effDataP++) {
        effDataP->color.a = a;
    }
    (void)result;
    DCStoreRange(&effP->data[0], effP->num*sizeof(MBEFFECTDATA));
}


 
static HuVecF *PauseOptionPosGet(HuVecF *in)
{
    static HuVecF result;
    MTXMultVec(pauseWork.posMtx, in, &result);
    return &result;
}

static Mtx *PauseOptionMtxGet(HuVecF *target)
{
    HuVecF pos;
    HuVecF up;
    static Mtx result;
    pos.x = pos.y = pos.z = 0;
    up = pos;
    up.y = 1;
    MTXLookAt(result, &pos, &up, target);
    MTXInverse(result, result);
    return &result;
}

static void PauseOptionModelInit(int modelId, HuVecF *pos, float scale)
{
    Mtx temp;
    MBModelAttrSet((MBMODELID)modelId, HU3D_MOTATTR_LOOP);
    MBModelAttrSet((MBMODELID)modelId, HU3D_ATTR_NOPAUSE);
    MBModelLayerSet(modelId, 6);
    MBModelDispSet(modelId, FALSE);
    MBModelPosSetV(modelId, PauseOptionPosGet(pos));
    MBModelScaleSet(modelId, scale, scale, scale);
    MTXConcat(pauseWork.optionMtx, *PauseOptionMtxGet(pos), temp);
    MBModelMtxSet(modelId, &temp);
}

static s16 cursorSprOfsTbl[2][4][2] = {
    {
        { 0, -62 },
        { 0, 68 },
        { 62, 0 },
        { -62, 0 }
    },
    {
        { 0, -20 },
        { 0, 20 },
        { 43, -60 },
        { -45, -60 }
    }
};

static int cursorDStkBitTbl[4] = {
    PAD_BUTTON_UP,
    PAD_BUTTON_DOWN,
    PAD_BUTTON_RIGHT,
    PAD_BUTTON_LEFT
};

static void PauseCursorCreate(void)
{
    PAUSE_CURSOR *cursor = &pauseWork.cursor;
    int i;
    cursor->dispF = FALSE;
    cursor->dStkBit = 0;
    cursor->moveBit = 0;
    cursor->cursorType = -1;
    cursor->moveTimer = -1;
    cursor->pos.x = cursor->pos.y = cursor->pos.z = 0;
    for(i=0; i<4; i++) {
        int sprId;
        cursor->tplvl[i] = 0;
        cursor->sprId[i] = sprId = espEntry(BPAUSE_ANM_cursor, 90, i);
        espAttrSet(sprId, HUSPR_ATTR_LINEAR);
        espPosSet(sprId, cursor->pos.x, cursor->pos.y);
        espDispOff(sprId);
        cursor->glowSprId[i] = sprId = espEntry(BPAUSE_ANM_cursorGlow, 89, i);
        espAttrSet(sprId, HUSPR_ATTR_LINEAR|HUSPR_ATTR_ADDCOL);
        espPosSet(sprId, cursor->pos.x, cursor->pos.y);
        espDispOff(sprId);
    }
}

static void PauseCursorKill(void)
{
    PAUSE_CURSOR *cursor = &pauseWork.cursor;
    int i;
    for(i=0; i<4; i++) {
        if(cursor->sprId[i] >= 0) {
            espKill(cursor->sprId[i]);
        }
        if(cursor->glowSprId[i] >= 0) {
            espKill(cursor->glowSprId[i]);
        }
        cursor->sprId[i] = cursor->glowSprId[i] = -1;
    }
}

static void PauseCursorMoveStart(int type, int moveBit, int dStkBit)
{
    PAUSE_CURSOR *cursor = &pauseWork.cursor;
    int i;
    if(_CheckFlag(FLAG_MG_CIRCUIT)) {
        return;
    }
    cursor->dispF = FALSE;
    if(type >= 0) {
        cursor->dispF = TRUE;
    }
    if(cursor->cursorType < 0 && type >= 0) {
        cursor->moveTimer = -1;
    }
    cursor->dStkBit = dStkBit;
    cursor->moveBit = moveBit;
    cursor->cursorType = type;
    for(i=0; i<4; i++) {
        espDispOff(cursor->glowSprId[i]);
        if(cursor->dispF) {
            espDispOn(cursor->sprId[i]);
            if(dStkBit & cursorDStkBitTbl[i]) {
                espDispOn(cursor->glowSprId[i]);
            }
        } else {
            espDispOff(cursor->sprId[i]);
        }
    }
}

static void PauseCursorMove(int speed, float posX, float posY)
{
    PAUSE_CURSOR *cursor = &pauseWork.cursor;
    int i;
    if(cursor->moveTimer < 0 || speed <= 0) {
        cursor->moveTimer = 0;
        cursor->pos.x = posX;
        cursor->pos.y = posY;
        for(i=0; i<4; i++) {
            cursor->tplvl[i] = 0;
            if(cursor->moveBit & cursorDStkBitTbl[i]) {
                cursor->tplvl[i] = 1;
            }
        }
    } else {
        float accel;
        cursor->moveTimer = speed;
        accel = 2.0f/(speed*speed);
        cursor->accel.x = accel*-(posX-cursor->pos.x);
        cursor->accel.y = accel*-(posY-cursor->pos.y);
        cursor->accel.z = 0;
        cursor->vel.x = -cursor->accel.x*(0.5f+speed);
        cursor->vel.y = -cursor->accel.y*(0.5f+speed);
        cursor->vel.z = 0;
    }
    for(i=0; i<4; i++) {
        espPosSet(cursor->sprId[i], cursor->pos.x+cursorSprOfsTbl[cursor->cursorType][i][0], cursor->pos.y+cursorSprOfsTbl[cursor->cursorType][i][1]);
        espTPLvlSet(cursor->sprId[i], cursor->tplvl[i]);
        espPosSet(cursor->glowSprId[i], cursor->pos.x+cursorSprOfsTbl[cursor->cursorType][i][0], cursor->pos.y+cursorSprOfsTbl[cursor->cursorType][i][1]);
        espTPLvlSet(cursor->glowSprId[i], cursor->tplvl[i]);
    }
}

static int PauseCursorUpdate(void)
{
    PAUSE_CURSOR *cursor = &pauseWork.cursor;
    int i;
    if(cursor->moveTimer <= 0) {
        return 0;
    }
    cursor->moveTimer--;
    VECAdd(&cursor->vel, &cursor->accel, &cursor->vel);
    VECAdd(&cursor->pos, &cursor->vel, &cursor->pos);
    for(i=0; i<4; i++) {
        if(cursor->moveBit & cursorDStkBitTbl[i]) {
            if(cursor->tplvl[i] < 1) {
                cursor->tplvl[i] += 0.1f;
            }
            if(cursor->tplvl[i] > 1) {
                cursor->tplvl[i] = 1;
            }
        } else {
            if(cursor->tplvl[i] > 0) {
                cursor->tplvl[i] -= 0.1f;
            }
            if(cursor->tplvl[i] < 0) {
                cursor->tplvl[i] = 0;
            }
        }
    }
    for(i=0; i<4; i++) {
        espPosSet(cursor->sprId[i], cursor->pos.x+cursorSprOfsTbl[cursor->cursorType][i][0], cursor->pos.y+cursorSprOfsTbl[cursor->cursorType][i][1]);
        espTPLvlSet(cursor->sprId[i], cursor->tplvl[i]);
        espPosSet(cursor->glowSprId[i], cursor->pos.x+cursorSprOfsTbl[cursor->cursorType][i][0], cursor->pos.y+cursorSprOfsTbl[cursor->cursorType][i][1]);
        espTPLvlSet(cursor->glowSprId[i], cursor->tplvl[i]);
    }
    return cursor->moveTimer;
}

static BOOL PadStatCheckAll(BOOL initF)
{
    PAUSE_PADSTAT *padstat = &pauseWork.padStat[0];
    BOOL changeF = FALSE;
    int i;
    int j;
    int temp;
    if(initF) {
        for(i=0; i<GW_PLAYER_MAX; i++){ 
            padstat[i].no = i;
            padstat[i].padNo = GwPlayer[i].padNo;
            padstat[i].stat = 0;
        }
        for(i=0; i<GW_PLAYER_MAX-1; i++) { 
            for(j=i+1; j<GW_PLAYER_MAX; j++) {
                if(padstat[i].padNo > padstat[j].padNo || (padstat[i].padNo == padstat[j].padNo && padstat[i].no > padstat[j].no)) {
                    temp = padstat[i].padNo;
                    padstat[i].padNo = padstat[j].padNo;
                    padstat[j].padNo = temp;
                    temp = padstat[i].no;
                    padstat[i].no = padstat[j].no;
                    padstat[j].no = temp;
                }
            }
        }
        if(GWPartyFGet() != FALSE) {
            for(i=0; i<GW_PLAYER_MAX; i++) {
                padstat[i].stat = HuPadStatGet(padstat[i].padNo);
                if(padstat[i].stat && !GwPlayer[padstat[i].no].comF) {
                    GwPlayer[padstat[i].no].comF = TRUE;
                    GwPlayerConf[padstat[i].no].type = GW_TYPE_COM;
                }
            }
        }
    }
    if(GWPartyFGet() == FALSE) {
        return FALSE;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        temp = HuPadStatGet(padstat[i].padNo);
        if(padstat[i].stat && !temp) {
            GwPlayer[padstat[i].no].comF = FALSE;
            GwPlayerConf[padstat[i].no].type = GW_TYPE_MAN;
            changeF = TRUE;
        } else if(!padstat[i].stat && temp) {
            GwPlayer[padstat[i].no].comF = TRUE;
            GwPlayerConf[padstat[i].no].type = GW_TYPE_COM;
            changeF = TRUE;
        }
        padstat[i].stat = temp;
    }
    return changeF;
}

static BOOL playerDispF[GW_PLAYER_MAX];

static void PauseDispCopyDraw(HU3DMODEL *modelP, Mtx *mtx);

void MBPauseDispCopyCreate(void)
{
    int i;
    int fbSize;
    pauseDispCopyBuf = NULL;
    pauseDispCopyF = FALSE;
    fbSize = GXGetTexBufferSize(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_FALSE, 0);
    pauseDispCopyBuf = HuMemDirectMalloc(HEAP_HEAP, fbSize);
    DCFlushRange(pauseDispCopyBuf, fbSize);
    pauseDispCopyMdl = Hu3DHookFuncCreate(PauseDispCopyDraw);
    Hu3DModelLayerSet(pauseDispCopyMdl, 5);
    HuPrcVSleep();
    for(i=0; i<GW_PLAYER_MAX; i++) {
        playerDispF[i] = MBPlayerDispGet(i);
        MBPlayerDispSet2(i, FALSE);
    }
}

void MBPauseDispCopyKill(void)
{
    int i;
    pauseDispCopyF = FALSE;
    HuMemDirectFree(pauseDispCopyBuf);
    pauseDispCopyBuf = NULL;
    Hu3DModelKill(pauseDispCopyMdl);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(playerDispF[i]) {
            MBPlayerDispSet2(i, TRUE);
        }
    }
}

static void PauseDispCopyDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    if(!pauseDispCopyF) {
        GXSetTexCopySrc(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
        GXSetTexCopyDst(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(pauseDispCopyBuf, FALSE);
        GXPixModeSync();
        pauseDispCopyF++;
    } else {
        Mtx modelview;
        Mtx44 proj;
        GXTexObj texObj;
        
        MTXOrtho(proj, 0, HU_FB_HEIGHT, 0, HU_FB_WIDTH, 0,100);
        GXSetProjection(proj, GX_ORTHOGRAPHIC);
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
        GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
        MTXIdentity(modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_CLAMP, GX_AF_NONE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GXInitTexObj(&texObj, pauseDispCopyBuf, HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_TRUE);
        GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texObj, GX_TEXMAP0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16(0, 0, -50);
        GXTexCoord2s16(0, 0);
        GXPosition3s16(HU_FB_WIDTH, 0, -50);
        GXTexCoord2s16(256, 0);
        GXPosition3s16(HU_FB_WIDTH, HU_FB_HEIGHT, -50);
        GXTexCoord2s16(256, 256);
        GXPosition3s16(0, HU_FB_HEIGHT, -50);
        GXTexCoord2s16(0, 256);
        GXEnd();
    }
}