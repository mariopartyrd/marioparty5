#include "REL/mdpartyDll.h"

#include "REL/mdpartyDll/effect.c"

#include "game/wipe.h"

#include "messnum/partymode_sys.h"
#include "messnum/mapname.h"
#include "messnum/teamname.h"
#include "messnum/syshelp.h"
#include "messnum/mgmode_sys.h"


#define MDPARTY_STAR_NONE 0
#define MDPARTY_STAR_BACK_START 1
#define MDPARTY_STAR_BACK 2
#define MDPARTY_STAR_BACK_END 3
#define MDPARTY_STAR_DECENTER_START 4
#define MDPARTY_STAR_DECENTER 5
#define MDPARTY_STAR_DECENTER_END 6
#define MDPARTY_STAR_MOVE_LEFT_START 7
#define MDPARTY_STAR_MOVE_LEFT 8
#define MDPARTY_STAR_MOVE_LEFT_END 9
#define MDPARTY_STAR_SNAP_LEFT 10
#define MDPARTY_STAR_CENTER_START 11
#define MDPARTY_STAR_CENTER 12
#define MDPARTY_STAR_CENTER_END 13

#define MDPARTY_STAR_MOT_NONE 0
#define MDPARTY_STAR_MOT_RESET 1
#define MDPARTY_STAR_MOT_TALK 2
#define MDPARTY_STAR_MOT_IDLE 3

#define MDPARTY_PLAYERSPR_NONE 0
#define MDPARTY_PLAYERSPR_GROW_START 1
#define MDPARTY_PLAYERSPR_GROW 2
#define MDPARTY_PLAYERSPR_GROW_END 3
#define MDPARTY_PLAYERSPR_SHRINK_START 4
#define MDPARTY_PLAYERSPR_SHRINK 5
#define MDPARTY_PLAYERSPR_SHRINK_END 6

#define MDPARTY_CHR_NONE 0
#define MDPARTY_CHR_INIT 1
#define MDPARTY_CHR_SELECT 2
#define MDPARTY_CHR_SELECT_WAIT 3
#define MDPARTY_CHR_SELECT_MOVE 4
#define MDPARTY_CHR_SELECT_MOVE_WAIT 5

#define MDPARTY_AUD_MUS_START 0
#define MDPARTY_AUD_MUS_STOP 1
#define MDPARTY_AUD_SE_CMN_MOVE 2
#define MDPARTY_AUD_SE_CMN_CONFIRM 3
#define MDPARTY_AUD_SE_CMN_CHRSEL 4
#define MDPARTY_AUD_SE_CMN_CANCEL 5
#define MDPARTY_AUD_SE_MENU_MOVE 6
#define MDPARTY_AUD_SE_BOARD_START 7
#define MDPARTY_AUD_SE_BOARD_ENTER1 8
#define MDPARTY_AUD_SE_BOARD_ENTER2 9
#define MDPARTY_AUD_SE_STAR_ENTER 10
#define MDPARTY_AUD_SE_STAR_CONFIRM 11
#define MDPARTY_AUD_SE_CHR_MARIO 12
#define MDPARTY_AUD_SE_CHR_LUIGI 13
#define MDPARTY_AUD_SE_CHR_PEACH 14
#define MDPARTY_AUD_SE_CHR_YOSHI 15
#define MDPARTY_AUD_SE_CHR_WARIO 16
#define MDPARTY_AUD_SE_CHR_DAISY 17
#define MDPARTY_AUD_SE_CHR_WALUIGI 18
#define MDPARTY_AUD_SE_CHR_KINOPIO 19
#define MDPARTY_AUD_SE_CHR_TERESA 20
#define MDPARTY_AUD_SE_CHR_MINIKOOPA 21
#define MDPARTY_AUD_SE_ALL_STOP 22

#define MdPartyReset() MdProcCall(MDPARTY_PROC_RESET, 0)
#define MdPartyNameShow() MdProcWait(MDPARTY_PROC_NAME, 0)
#define MdPartyNameHide() MdProcWait(MDPARTY_PROC_NAME, 1)

#define MdPartyWinOpen() MdProcWait(MDPARTY_PROC_WIN, FALSE)
#define MdPartyWinClose() MdProcWait(MDPARTY_PROC_WIN, TRUE)

#define MdPartyCursorIn() MdProcWait(MDPARTY_PROC_CURSOR, 0)
#define MdPartyCursorOut() MdProcWait(MDPARTY_PROC_CURSOR, 1)
#define MdPartyCursorDirLR(dir) MdProcWait(MDPARTY_PROC_CURSOR, dir+2)
#define MdPartyCursorDirUD(dir) MdProcWait(MDPARTY_PROC_CURSOR, dir+4)

#define MdPartyGameTypeStart() MdProcWait(MDPARTY_PROC_GAME_TYPE, 0)
#define MdPartyGameTypeChange() MdProcWait(MDPARTY_PROC_GAME_TYPE, 2)
#define MdPartyGameTypeEnd() MdProcWait(MDPARTY_PROC_GAME_TYPE, 3)
#define MdPartyGameTypeEnterBack() MdProcWait(MDPARTY_PROC_GAME_TYPE, 4)

#define MdPartyCapPlayerStart() MdProcWait(MDPARTY_PROC_CAP_PLAYER, 0)
#define MdPartyCapPlayerEnd() MdProcWait(MDPARTY_PROC_CAP_PLAYER, 1)
#define MdPartyCapPlayerMove(dir) MdProcWait(MDPARTY_PROC_CAP_PLAYER, dir+2)

#define MdPartyChrStart() MdProcWait(MDPARTY_PROC_CHR, 0)
#define MdPartyChrCancel() MdProcWait(MDPARTY_PROC_CHR, 1)
#define MdPartyChrCenter() MdProcWait(MDPARTY_PROC_CHR, 2)
#define MdPartyChrReset() MdProcWait(MDPARTY_PROC_CHR, 3)
#define MdPartyChrTeamStart() MdProcWait(MDPARTY_PROC_CHR, 4)
#define MdPartyChrTeamCenter() MdProcWait(MDPARTY_PROC_CHR, 5)
#define MdPartyChrTeamMove() MdProcWait(MDPARTY_PROC_CHR, 6)
#define MdPartyChrTeamMoveEnd() MdProcWait(MDPARTY_PROC_CHR, 7)
#define MdPartyChrTeamReset() MdProcWait(MDPARTY_PROC_CHR, 8)
#define MdPartyChrHandicapStart() MdProcWait(MDPARTY_PROC_CHR, 9)
#define MdPartyChrHandicapCancel() MdProcWait(MDPARTY_PROC_CHR, 10)
#define MdPartyChrTeamHandicapStart() MdProcWait(MDPARTY_PROC_CHR, 11)
#define MdPartyChrTeamHandicapCancel() MdProcWait(MDPARTY_PROC_CHR, 12)


#define MdPartyTeamStart() MdProcWait(MDPARTY_PROC_TEAM, 0)
#define MdPartyTeamSelEnd() MdProcWait(MDPARTY_PROC_TEAM, 1)
#define MdPartyTeamCancel() MdProcWait(MDPARTY_PROC_TEAM, 2)
#define MdPartyTeamReset() MdProcWait(MDPARTY_PROC_TEAM, 3)

#define MdPartyMapStart() MdProcWait(MDPARTY_PROC_MAP, 0)
#define MdPartyMapCancel() MdProcWait(MDPARTY_PROC_MAP, 1)
#define MdPartyMapMove() MdProcWait(MDPARTY_PROC_MAP, 2)
#define MdPartyMapEnd() MdProcWait(MDPARTY_PROC_MAP, 3)
#define MdPartyMapReset() MdProcWait(MDPARTY_PROC_MAP, 4)

#define MdPartyMapNameStart() MdProcWait(MDPARTY_PROC_MAP_NAME, 0)
#define MdPartyMapNameCancel() MdProcWait(MDPARTY_PROC_MAP_NAME, 1)

#define MdPartyFingerStart() MdProcWait(MDPARTY_PROC_FINGER, 0)

#define MdPartyCapTurnStart() MdProcWait(MDPARTY_PROC_CAP_TURN, 0)
#define MdPartyCapTurnCancel() MdProcWait(MDPARTY_PROC_CAP_TURN, 1)
#define MdPartyCapTurnUpdate() MdProcWait(MDPARTY_PROC_CAP_TURN, 2)
#define MdPartyCapTurnEnd() MdProcWait(MDPARTY_PROC_CAP_TURN, 3)
#define MdPartyCapTurnReset() MdProcWait(MDPARTY_PROC_CAP_TURN, 4)

#define MdPartyCapPackStart() MdProcWait(MDPARTY_PROC_CAP_PACK, 0)
#define MdPartyCapPackCancel() MdProcWait(MDPARTY_PROC_CAP_PACK, 2)
#define MdPartyCapPackEnd() MdProcWait(MDPARTY_PROC_CAP_PACK, 3)
#define MdPartyCapPackReset() MdProcWait(MDPARTY_PROC_CAP_PACK, 4)

#define MdPartyCapBonusStart() MdProcWait(MDPARTY_PROC_CAP_BONUS, 0)
#define MdPartyCapBonusCancel() MdProcWait(MDPARTY_PROC_CAP_BONUS, 1)
#define MdPartyCapBonusUpdate() MdProcWait(MDPARTY_PROC_CAP_BONUS, 2)
#define MdPartyCapBonusEnd() MdProcWait(MDPARTY_PROC_CAP_BONUS, 3)
#define MdPartyCapBonusReset() MdProcWait(MDPARTY_PROC_CAP_BONUS, 4)

#define MdPartyHandicapStart() MdProcWait(MDPARTY_PROC_HANDICAP, 0)
#define MdPartyHandicapCancel() MdProcWait(MDPARTY_PROC_HANDICAP, 1)

#define MdPartyDispSetupStart() MdProcWait(MDPARTY_PROC_DISP_SETUP, 0)
#define MdPartyDispSetupCancel() MdProcWait(MDPARTY_PROC_DISP_SETUP, 1)
#define MdPartyDispSetupReset() MdProcWait(MDPARTY_PROC_DISP_SETUP, 2)

#define MdPartyDispMapEnter() MdProcWait(MDPARTY_PROC_MAP_ENTER, 0)

#define MDPARTY_CURSOR_START 0
#define MDPARTY_CURSOR_PLAYERSEL 1
#define MDPARTY_CURSOR_CHR_DIF_SEL 2
#define MDPARTY_CURSOR_TEAM_DIF_SEL 3
#define MDPARTY_CURSOR_TEAM_SEL 4
#define MDPARTY_CURSOR_MAP_SEL 5
#define MDPARTY_CURSOR_TURN_SEL 6
#define MDPARTY_CURSOR_PACK_SEL 7
#define MDPARTY_CURSOR_BONUS_SEL 8
#define MDPARTY_CURSOR_HANDICAP 9
#define MDPARTY_CURSOR_HANDICAP_TEAM 10

void MdPartyModelBBoardSet(int mdlNo, int cameraNo);
void MdPartyAudExec(int cmd);
int MdPartyFXPanGet(int mdlNo, int cameraNo);
void MdPartyPadSetup(void);
void MdPartyPlayerSetupInit(void);
BOOL MdPartyChrSelCursorUpdate(int playerNo, BOOL comSelF);
void MdPartyPlayerSprUpdate(void);
void MdPartyChrComInit(void);
void MdPartyTeamPlayerUpdate(void);
void MdPartyTeamOrderBackup(void);
void MdPartySaveInit(void);

void MdPartyFlowInit(void);
void MdPartyFlowStart(void);
void MdPartyFlowPlayerNumSel(void);
void MdPartyFlowChrSel(void);
void MdPartyFlowTeamSel(void);
void MdPartyFlowMapSel(void);
void MdPartyFlowTurnSel(void);
void MdPartyFlowPackSel(void);
void MdPartyFlowBonusSel(void);
void MdPartyFlowHandicap(void);

BOOL MdPartyProcReset(int param);
BOOL MdPartyProcCamera(int param);
BOOL MdPartyProcCursor(int param);
BOOL MdPartyProcWin(int param);
BOOL MdPartyProcGameType(int param);
BOOL MdPartyProcCapPlayer(int param);
BOOL MdPartyProcChr(int param);
BOOL MdPartyProcTeam(int param);
BOOL MdPartyProcVs(int param);
BOOL MdPartyProcMap(int param);
BOOL MdPartyProcMapName(int param);
BOOL MdPartyProcCapTurn(int param);
BOOL MdPartyProcCapPack(int param);
BOOL MdPartyProcCapBonus(int param);
BOOL MdPartyProcHandicap(int param);
BOOL MdPartyProcDispSetup(int param);
BOOL MdPartyProcFinger(int param);
BOOL MdPartyProcMapEnter(int param);
BOOL MdPartyProcName(int param);

//TODO: Get rid of extern
extern void MBPartySaveInit(int boardNo);
extern void MBPlayerSaveInit(BOOL teamF, BOOL bonusStarF, int mgPack, int turnMax, int handicapP1, int handicapP2, int handicapP3, int handicapP4);

char *lbl_1_data_144 = "26";

static OMOVL MBDll[7] = {
    DLL_w01dll,
    DLL_w02dll,
    DLL_w03dll,
    DLL_w04dll,
    DLL_w05dll,
    DLL_w06dll,
    DLL_w07dll,
};

static int MBDataDir[7] = {
    DATA_w01,
    DATA_w02,
    DATA_w03,
    DATA_w04,
    DATA_w05,
    DATA_w06,
    DATA_w07
};

static HuVecF CharPos[10][2] = {
	{ { -250, 0, 0 }, { -250, 0, 0 } },
	{ { -125, 0, 0 }, { -125, 0, 0 } },
	{ { 0, 0, 0 },  { 0, 0, 0 } },
	{ { 125, 0, 0 }, { 139, 17.5, -500 } },
	{ { 275, 9, -300 }, { 266, 2, -200 } },
	{ { -250, -250, 0 }, { -250, -250, 0 } },
	{ { -125, -250, 0 }, { -140.5, -270, -400 } },
	{ { 0, -270, -300 }, { -8.5, -290.5, -629 } },
	{ { 125, -250, -1800 }, { 122, -261, -1800 } },
	{ { 250, -250, -1000 }, { 245.5, -254, -1000 } }
};

static HuVecF CharRot[10][2] = {
	{ { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 } },
    { { 10, -2, 0 }, { 10, -4, 0 } },
    { { 0, -3, 0 }, { 0, -6, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, -4, 0 }, { 0, -2, 0 } }
};

MD_FUNC MdFlowTbl[MDPARTY_FLOW_MAX] = {
	MdPartyFlowInit,
	MdPartyFlowStart,
	MdPartyFlowPlayerNumSel,
	MdPartyFlowChrSel,
	MdPartyFlowTeamSel,
	MdPartyFlowMapSel,
	MdPartyFlowTurnSel,
	MdPartyFlowPackSel,
	MdPartyFlowBonusSel,
	MdPartyFlowHandicap,
};

MD_PROC MdProcTbl[MDPARTY_PROC_MAX] = {
	MdPartyProcReset,
	MdPartyProcCamera,
    MdPartyProcCursor,
	MdPartyProcWin,
	MdPartyProcGameType,
	MdPartyProcCapPlayer,
	MdPartyProcChr,
	MdPartyProcTeam,
	MdPartyProcVs,
	MdPartyProcMap,
	MdPartyProcMapName,
	MdPartyProcCapTurn,
	MdPartyProcCapPack,
	MdPartyProcCapBonus,
	MdPartyProcHandicap,
	MdPartyProcDispSetup,
	MdPartyProcFinger,
	MdPartyProcMapEnter,
	MdPartyProcName,
};

SYS sysWork;
SYS *sys;
SYS *sysP;

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void ObjectSetup(void);

int _prolog(void) {
    const VoidFunc* ctors = _ctors;
    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

void _epilog(void) {
    const VoidFunc* dtors = _dtors;
    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}


void ObjectSetup(void)
{
    static int closeDir[10] = {
        DATA_board,
        DATA_w01,
        DATA_w02,
        DATA_w03,
        DATA_w04,
        DATA_w05,
        DATA_w06,
        DATA_w07,
        DATA_w10,
        DATA_w20
    };
    
    int i;
    sys = &sysWork;
    sysP = sys;
    sys->subMode = 0;
    sys->unk4 = 0;
    sys->flowTime = 0;
    sys->flowMode = MDPARTY_FLOW_INIT;
    sys->ovlCallMode = MD_OVLCALL_GOTO;
    sys->nextDll = DLL_selmenuDLL;
    sys->exitF = FALSE;
    sys->skipWipeF = FALSE;
    GWPartyFSet(TRUE);
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    for(i=0; i<10; i++) {
        HuARDirFree(closeDir[i]);
        HuDataDirClose(closeDir[i]);
    }
    sys->objman = omInitObjMan(512, 8192);
    sys->ovlHis = omOvlHisGet(0);
    sys->evtNo = sys->ovlHis->evtno;
    omGameSysInit(sys->objman);
    sys->obj[MDPARTY_OBJ_DEBUGFONT] = omAddObj(sys->objman, 100, 0, 0, MdObjDebugFontCreate);
    sys->obj[MDPARTY_OBJ_DEBUGFONT]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDDEBUGFONT_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDPARTY_OBJ_DEBUGFONT]->data) {
        OSReport("sys->obj[MDPARTY_OBJ_DEBUGFONT]->data = NULL...\n");
    }
    sys->debugFont = sys->obj[MDPARTY_OBJ_DEBUGFONT]->data;
    sys->debugFont->createF = FALSE;
    sys->obj[MDPARTY_OBJ_PAD] = omAddObj(sys->objman, 101, 0, 0, MdObjPadMain);
    sys->obj[MDPARTY_OBJ_PAD]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDPAD_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDPARTY_OBJ_PAD]->data) {
        OSReport("sys->obj[MDPARTY_OBJ_PAD]->data = NULL...\n");
    }
    sys->pad = sys->obj[MDPARTY_OBJ_PAD]->data;
    sys->obj[MDPARTY_OBJ_CAMERA] = omAddObj(sys->objman, 102, 0, 0, MdObjCameraCreate);
    sys->obj[MDPARTY_OBJ_CAMERA]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDCAMERA_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDPARTY_OBJ_CAMERA]->data) {
        OSReport("sys->obj[MDPARTY_OBJ_CAMERA]->data = NULL...\n");
    }
    sys->camera = sys->obj[MDPARTY_OBJ_CAMERA]->data;
    sys->camera->createF = FALSE;
    sys->camera->debugFont = sys->debugFont;
    sys->obj[MDPARTY_OBJ_LIGHT] = omAddObj(sys->objman, 103, 0, 0, MdObjLightCreate);
    sys->obj[MDPARTY_OBJ_LIGHT]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDLIGHT_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDPARTY_OBJ_LIGHT]->data) {
        OSReport("sys->obj[MDPARTY_OBJ_LIGHT]->data = NULL...\n");
    }
    sys->light = sys->obj[MDPARTY_OBJ_LIGHT]->data;
    sys->light->createF = FALSE;
    sys->light->debugFont = sys->debugFont;
    sys->sprite = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDSPRITE_WORK), HU_MEMNUM_OVL);
    if(!sys->sprite) {
        OSReport("sys->sprite = NULL...\n");
    }
    sys->model = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDMODEL_WORK), HU_MEMNUM_OVL);
    if(!sys->model) {
        OSReport("sys->model = NULL...\n");
    }
    sys->texbuf = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDTEXBUF_WORK), HU_MEMNUM_OVL);
    if(!sys->texbuf) {
        OSReport("sys->texbuf = NULL...\n");
    }
    sys->ring = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDRING_WORK), HU_MEMNUM_OVL);
    if(!sys->ring) {
        OSReport("sys->ring = NULL...\n");
    }
    sys->star = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDPARTY_STAR), HU_MEMNUM_OVL);
    if(!sys->star) {
        OSReport("sys->star = NULL...\n");
    }
    sys->mess = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDMESS_WORK), HU_MEMNUM_OVL);
    if(!sys->mess) {
        OSReport("sys->mess = NULL...\n");
    }
    MdMessCreate(sys->mess);
    MdMathStdCurveInit();
    Hu3DBGColorSet(0, 0, 0);
    Hu3DNoSyncSet(FALSE);
    while(1) {
        if(sys->debugFont->createF == TRUE && sys->camera->createF == TRUE && sys->light->createF == TRUE) {
            break;
        }
        HuPrcVSleep();
    }
    sys->mainProc = HuPrcChildCreate(MdMain, 105, 12288, 0, sys->objman);
}

void MdMain(void)
{
    if(sys->evtNo == 1) {
        omOvlReturn(1);
        HuPrcEnd();
        while(1) {
            HuPrcVSleep();
        }
    }
    MdInit();
    while(!omSysExitReq && !sys->exitF) {
        MdFlowTbl[sys->flowMode]();
        if(++sys->flowTime > 36000) {
            sys->flowTime = 0;
        }
        HuPrcVSleep();
    }
    MdClose();
}

static void LoadProc(void)
{
    OSTick tick;
    int i;
    int charNo;
    
    while(1) {
        switch(sys->loadState) {
            case MD_LOAD_STATE_NONE:
                break;

            case MD_LOAD_STATE_BOARD_LOAD:
                CharDataClose(-1);
                tick = OSGetTick();
                sys->loadAsyncStat = HuDataDirReadAsync(DATA_board);
                while(!HuDataGetAsyncStat(sys->loadAsyncStat)) {
                    HuPrcVSleep();
                }
                OSReport("Load Time:%d\n", OSTicksToMilliseconds(OSGetTick()-tick));
                OSReport("Finished!\n");
                HuAR_MRAMtoARAM(DATA_board);
                while(HuARDMACheck());
                HuDataDirClose(DATA_board);
                sys->loadProcReady = FALSE;
                sys->loadState = MD_LOAD_STATE_WAIT;
                break;
            
            case MD_LOAD_STATE_WAIT:
                if(sys->loadProcReady == TRUE) {
                    sys->loadState = MD_LOAD_STATE_CHAR_LOAD;
                }
                break;
            
            case MD_LOAD_STATE_CHAR_LOAD:
                for(i=0; i<GW_PLAYER_MAX; i++) {
                    charNo = GwPlayerConf[i].charNo;
                    CharMotionInitAsync(charNo);
                }
                sys->loadState = MD_LOAD_STATE_LOAD;
                break;
            
            case MD_LOAD_STATE_LOAD:
                tick = OSGetTick();
                sys->loadAsyncStat = HuDataDirReadAsync(sys->loadDir);
                while(!HuDataGetAsyncStat(sys->loadAsyncStat)) {
                    HuPrcVSleep();
                }
                OSReport("Load Time:%d\n", OSTicksToMilliseconds(OSGetTick()-tick));
                OSReport("Finished!\n");
                sys->loadState = MD_LOAD_STATE_DONE;
                break;

            case MD_LOAD_STATE_DONE:
                HuPrcEnd();
                break;
        }
        HuPrcVSleep();
    }
}

static void StarUpdate(void)
{
    MDPARTY_STAR *star; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *posP; //r29
    MDCAMERA_WORK *camera; //r28
    int i; //r27
    HuVecF *rotP; //r26
    MDMESS_WORK *mess; //r25
    HuVecF *scaleP; //r24
    
    static float curveData[5] = {
        0,
        16,
        64,
        32,
        -65
    };
    
    static HuVecF pos2D[2];
    static HuVecF pos3D[2];
    static float lerpTime;
    static float lerpOfsTime;
    static float curveTime;
    static float lerpT;
    static float lerpOfsT;
    static float curveT;

    model = sys->model;
    camera = sys->camera;
    mess = sys->mess;
    star = sys->star;
    posP = model->posP[MDMODEL_STAR];
    rotP = model->rotP[MDMODEL_STAR];
    scaleP = model->scaleP[MDMODEL_STAR];
    star->mode = MDPARTY_STAR_NONE;
    star->motNext = 1;
    
    posP->x = 0;
    posP->y = 0;
    posP->z = 1800;
    MdPartyModelBBoardSet(MDMODEL_STAR, MDCAMERA_FG);
    while(1) {
        model = sys->model;
        camera = sys->camera;
        mess = sys->mess;
        star = sys->star;
        posP = model->posP[MDMODEL_STAR];
        rotP = model->rotP[MDMODEL_STAR];
        scaleP = model->scaleP[MDMODEL_STAR];
        
        switch(star->mode) {
            case MDPARTY_STAR_NONE:
                break;

            case MDPARTY_STAR_BACK_START:
                lerpTime = 0;
                lerpOfsTime = 0;
                curveTime = 0;
                lerpT = 0;
                lerpOfsT = 0;
                curveT = 0;
                MdMathCurveInit(5, sys->curveTime, curveData, sys->curveSlope);
                star->mode = MDPARTY_STAR_BACK;
                break;
            
            case MDPARTY_STAR_BACK:
                MdMathOscillate(MDMATH_OSC_90_REV, &lerpTime, &lerpT, 60);
                MdMathOscillate(MDMATH_OSC_180, &lerpOfsTime, &lerpOfsT, 60);
                MdMathOscillate(MDMATH_OSC_0, &curveTime, &curveT, 60);
                MdMathLerp(&posP->x, 0, 45, lerpOfsT);
                MdMathCurveGet(&posP->y, 5, sys->curveTime, curveData, sys->curveSlope, curveT);
                MdMathLerp(&posP->z, 1800, 1300, lerpT);
                MdMathLerp(&rotP->y, 0, 360, lerpT);
                if(lerpTime >= 1) {
                    star->mode = MDPARTY_STAR_BACK_END;
                }
                break;
            
            case MDPARTY_STAR_BACK_END:
                break;

            case MDPARTY_STAR_DECENTER_START:
                lerpTime = 0;
                lerpOfsTime = 0;
                lerpT = 0;
                lerpOfsT = 0;
                pos2D[0].x = 64;
                pos2D[0].y = 320;
                pos2D[0].z = 1000;
                pos2D[1].x = 288;
                pos2D[1].y = 240;
                pos2D[1].z = 1000;
                for(i=0; i<2; i++) {
                    Hu3D2Dto3D(&pos2D[i], camera->camera[MDCAMERA_FG].bit, &pos3D[i]);
                }
                star->mode = MDPARTY_STAR_DECENTER;
                break;
            
            case MDPARTY_STAR_DECENTER:
                MdMathOscillate(MDMATH_OSC_90, &lerpTime, &lerpT, 30);
                MdMathLerp(&posP->x, pos3D[0].x, pos3D[1].x, lerpT);
                MdMathLerp(&posP->y, pos3D[0].y, pos3D[1].y, lerpT);
                MdMathLerp(&posP->z, pos3D[0].z, pos3D[1].z, lerpT);
                if(lerpTime >= 1) {
                    star->mode = MDPARTY_STAR_DECENTER_END;
                }
                break;
            
            case MDPARTY_STAR_DECENTER_END:
                break;

            case MDPARTY_STAR_MOVE_LEFT_START:
                lerpTime = 0;
                lerpOfsTime = 0;
                curveTime = 0;
                lerpT = 0;
                lerpOfsT = 0;
                curveT = 0;
                pos3D[0].x = 0;
                pos3D[0].y = -65;
                pos3D[0].z = 1300;
                pos2D[1].x = 54;
                pos2D[1].y = 312;
                pos2D[1].z = 1200;
                Hu3D2Dto3D(&pos2D[1], camera->camera[MDCAMERA_FG].bit, &pos3D[1]);
                star->mode = MDPARTY_STAR_MOVE_LEFT;
                break;
            
            case MDPARTY_STAR_MOVE_LEFT:
                MdMathOscillate(MDMATH_OSC_90, &lerpTime, &lerpT, 30);
                MdMathOscillate(MDMATH_OSC_180, &lerpOfsTime, &lerpOfsT, 30);
                MdMathLerp(&posP->x, pos3D[0].x, pos3D[1].x, lerpT);
                MdMathLerp(&posP->y, pos3D[0].y, pos3D[1].y, lerpT);
                MdMathLerpOfs(&posP->z, pos3D[0].z, pos3D[1].z, lerpT, -1.0f*lerpOfsT);
                if(lerpTime >= 1) {
                    star->mode = MDPARTY_STAR_MOVE_LEFT_END;
                }
                MdPartyModelBBoardSet(MDMODEL_STAR, MDCAMERA_FG);
                break;
            
            case MDPARTY_STAR_SNAP_LEFT:
                pos2D[1].x = 64;
                pos2D[1].y = 300;
                pos2D[1].z = 1000;
                Hu3D2Dto3D(&pos2D[1], camera->camera[MDCAMERA_FG].bit, &pos3D[1]);
                *posP = pos3D[1];
                MdPartyModelBBoardSet(MDMODEL_STAR, MDCAMERA_FG);
                star->mode = MDPARTY_STAR_MOVE_LEFT_END;
                break;
            
            case MDPARTY_STAR_CENTER_START:
                lerpTime = 0;
                lerpOfsTime = 0;
                curveTime = 0;
                lerpT = 0;
                lerpOfsT = 0;
                curveT = 0;
                pos3D[0].x = 0;
                pos3D[0].y = -65;
                pos3D[0].z = 1300;
                pos2D[1].x = 54;
                pos2D[1].y = 312;
                pos2D[1].z = 1200;
                Hu3D2Dto3D(&pos2D[1], camera->camera[MDCAMERA_FG].bit, &pos3D[1]);
                star->mode = MDPARTY_STAR_CENTER;
                break;
            
            case MDPARTY_STAR_CENTER:
                MdMathOscillate(MDMATH_OSC_90_REV, &lerpTime, &lerpT, 30);
                MdMathOscillate(MDMATH_OSC_180, &lerpOfsTime, &lerpOfsT, 30);
                MdMathLerp(&posP->x, pos3D[1].x, pos3D[0].x, lerpT);
                MdMathLerp(&posP->y, pos3D[1].y, pos3D[0].y, lerpT);
                MdMathLerpOfs(&posP->z, pos3D[1].z, pos3D[0].z, lerpT, -1.0f*lerpOfsT);
                if(lerpTime >= 1) {
                    star->mode = MDPARTY_STAR_CENTER_END;
                }
                MdPartyModelBBoardSet(MDMODEL_STAR, MDCAMERA_FG);
                break;
                
            
            case MDPARTY_STAR_CENTER_END:
                break;
        }
        switch(star->motNext) {
            case MDPARTY_STAR_MOT_NONE:
                break;
                
            case MDPARTY_STAR_MOT_RESET:
                Hu3DModelAttrSet(model->mdlId[MDMODEL_STAR], HU3D_MOTATTR_LOOP);
                Hu3DModelAttrReset(model->mdlId[MDMODEL_STAR], HU3D_MOTATTR_PAUSE);
                Hu3DMotionSet(model->mdlId[MDMODEL_STAR], model->motId[MDMODEL_MOT_STAR_IDLE]);
                star->motNext = MDPARTY_STAR_MOT_NONE;
                break;
            
            case MDPARTY_STAR_MOT_TALK:
                Hu3DMotionShiftSet(model->mdlId[MDMODEL_STAR], model->motId[MDMODEL_MOT_STAR_TALK], 0, 32, HU3D_MOTATTR_LOOP);
                star->motNext = MDPARTY_STAR_MOT_NONE;
                break;
            
            case MDPARTY_STAR_MOT_IDLE:
                Hu3DMotionShiftSet(model->mdlId[MDMODEL_STAR], model->motId[MDMODEL_MOT_STAR_IDLE], 0, 32, HU3D_MOTATTR_LOOP);
                star->motNext = MDPARTY_STAR_MOT_NONE;
                break;
        }
        HuPrcVSleep();
    }
}

static void PlayerSprUpdate(void)
{
    MDPARTY_PLAYERSPR *playerSpr; //r31
    HuVec2f *playerNoScale; //r30
    HuVec2f *comDifScale; //r29
    int i; //r28
    MDSPRITE_WORK *sprite; //r27
    HUSPRGRPID playerNoGrpId; //r26
    HUSPRGRPID comDifGrpId; //r25
    MDCAMERA_WORK *camera; //r24
    MDMODEL_WORK *model; //r23
    
    camera = sys->camera;
    model = sys->model;
    sprite = sys->sprite;
    playerNoGrpId = sprite->grpId[MDSPR_GRP_PLAYERNO];
    comDifGrpId = sprite->grpId[MDSPR_GRP_COMDIF];
    for(playerSpr=&sys->playerSpr[0], i=0; i<4; i++, playerSpr++) {
        playerSpr->mode = MDPARTY_PLAYERSPR_NONE;
    }
    while(1) {
        camera = sys->camera;
        model = sys->model;
        sprite = sys->sprite;
        playerNoGrpId = sprite->grpId[MDSPR_GRP_PLAYERNO];
        comDifGrpId = sprite->grpId[MDSPR_GRP_COMDIF];
        HuSprGrpPosSet(playerNoGrpId, 0, 0);
        HuSprGrpPosSet(comDifGrpId, 0, 0);
        for(playerSpr=&sys->playerSpr[0], i=0; i<4; i++, playerSpr++) {
            playerNoScale = MdSpriteScaleGet(sprite, MDSPR_GRP_PLAYERNO, i);
            comDifScale = MdSpriteScaleGet(sprite, MDSPR_GRP_COMDIF, i);
            switch(playerSpr->mode) {
                case MDPARTY_PLAYERSPR_NONE:
                    break;
                
                case MDPARTY_PLAYERSPR_GROW_START:
                    HuSprDispOn(playerNoGrpId, i);
                    HuSprDispOn(comDifGrpId, i);
                    playerNoScale->x = 0;
                    playerNoScale->y = 0;
                    comDifScale->x = 0;
                    comDifScale->y = 0;
                    playerSpr->time = 0;
                    playerSpr->ofsTime = 0;
                    playerSpr->t = 0;
                    playerSpr->ofsT = 0;
                    playerSpr->mode = MDPARTY_PLAYERSPR_GROW;
                    break;
                
                case MDPARTY_PLAYERSPR_GROW:
                    MdMathOscillate(MDMATH_OSC_90, &playerSpr->time, &playerSpr->t, 15);
                    MdMathOscillate(MDMATH_OSC_180, &playerSpr->ofsTime, &playerSpr->ofsT, 15);
                    MdMathLerpOfs(&playerNoScale->x, 0, 1, playerSpr->t, 0.5f*playerSpr->ofsT);
                    playerNoScale->y = playerNoScale->x;
                    MdMathLerpOfs(&comDifScale->x, 0, 1, playerSpr->t, 0.5f*playerSpr->ofsT);
                    comDifScale->y = comDifScale->x;
                    if(playerSpr->time >= 1) {
                        playerSpr->mode = MDPARTY_PLAYERSPR_GROW_END;
                    }
                    break;
                
                case MDPARTY_PLAYERSPR_GROW_END:
                    break;
                
                case MDPARTY_PLAYERSPR_SHRINK_START:
                    playerSpr->time = 0;
                    playerSpr->ofsTime = 0;
                    playerSpr->t = 0;
                    playerSpr->ofsT = 0;
                    playerSpr->mode = MDPARTY_PLAYERSPR_SHRINK;
                    break;
                
                case MDPARTY_PLAYERSPR_SHRINK:
                    MdMathOscillate(MDMATH_OSC_90, &playerSpr->time, &playerSpr->t, 15);
                    MdMathLerp(&playerNoScale->x, 1, 0, playerSpr->t);
                    playerNoScale->y = playerNoScale->x;
                    MdMathLerp(&comDifScale->x, 1, 0, playerSpr->t);
                    comDifScale->y = comDifScale->x;
                    if(playerSpr->time >= 1) {
                        HuSprDispOff(playerNoGrpId, i);
                        HuSprDispOff(comDifGrpId, i);
                        playerSpr->mode = MDPARTY_PLAYERSPR_SHRINK_END;
                    }
                    break;
                
                case MDPARTY_PLAYERSPR_SHRINK_END:
                    break;
            }
        }
        HuPrcVSleep();
    }   
}

static void ChrUpdate(void)
{
    MDPARTY_CHR *chr; //r31
    int i; //r30
    MDMODEL_WORK *model; //r29
    HuVecF *posP; //r28
    HuVecF *rotP; //r27
    MDCAMERA_WORK *camera; //r26
    MDSPRITE_WORK *sprite; //r25
    
    HuVecF *scaleP; //sp+0x8
    camera = sys->camera;
    model = sys->model;
    sprite = sys->sprite;

    for(chr=&sys->chr[0], i=0; i<10; i++, chr++) {
        chr->mode = MDPARTY_CHR_NONE;
    }
    while(1) {
        camera = sys->camera;
        model = sys->model;
        sprite = sys->sprite;
        for(chr=&sys->chr[0], i=0; i<10; i++, chr++) {
            if(i != 0 && i != 1 && i != 2 && i != 5) {
                posP = model->posP[i+MDMODEL_MARIO];
                rotP = model->rotP[i+MDMODEL_MARIO];
                scaleP = model->scaleP[i+MDMODEL_MARIO];
                switch(chr->mode) {
                    case MDPARTY_CHR_NONE:
                        break;
                    
                    case MDPARTY_CHR_INIT:
                        *posP = CharPos[i][0];
                        *rotP = CharRot[i][0];
                        chr->mode = MDPARTY_CHR_NONE;
                        break;
                        
                    case MDPARTY_CHR_SELECT:
                    case MDPARTY_CHR_SELECT_WAIT:
                        chr->origPos = *posP;
                        chr->origRot = *rotP;
                        if(chr->mode == MDPARTY_CHR_SELECT) {
                            chr->pos = CharPos[i][1];
                            chr->rot = CharRot[i][1];
                        } else if(chr->mode == MDPARTY_CHR_SELECT_WAIT) {
                            chr->pos = CharPos[i][0];
                            chr->rot = CharRot[i][0];
                        }
                        chr->time = 0;
                        chr->ofsTime = 0;
                        chr->t = 0;
                        chr->ofsT = 0;
                        if(chr->mode == MDPARTY_CHR_SELECT) {
                            chr->mode = MDPARTY_CHR_SELECT_MOVE;
                        } else if(chr->mode == MDPARTY_CHR_SELECT_WAIT) {
                            chr->mode = MDPARTY_CHR_SELECT_MOVE_WAIT;
                        }
                        break;
                    
                    case MDPARTY_CHR_SELECT_MOVE:
                    case MDPARTY_CHR_SELECT_MOVE_WAIT:
                        if(chr->mode == MDPARTY_CHR_SELECT_MOVE) {
                            MdMathOscillate(MDMATH_OSC_0, &chr->time, &chr->t, 15);
                        } else if(chr->mode == MDPARTY_CHR_SELECT_MOVE_WAIT) {
                            MdMathOscillate(MDMATH_OSC_0, &chr->time, &chr->t, 15);
                        }
                        MdMathLerp(&posP->x, chr->origPos.x, chr->pos.x, chr->t);
                        MdMathLerp(&posP->y, chr->origPos.y, chr->pos.y, chr->t);
                        MdMathLerp(&posP->z, chr->origPos.z, chr->pos.z, chr->t);
                        MdMathLerp(&rotP->x, chr->origRot.x, chr->rot.x, chr->t);
                        MdMathLerp(&rotP->y, chr->origRot.y, chr->rot.y, chr->t);
                        MdMathLerp(&rotP->z, chr->origRot.z, chr->rot.z, chr->t);
                        if(chr->time >= 1) {
                            chr->mode = MDPARTY_CHR_NONE;
                        }
                        break;
                }
            }
        }
        HuPrcVSleep();
    }
}

static void RingUpdate(void)
{
    while(1) {
        MdRingMain();
        HuPrcVSleep();
    }
}

static void MainWinUpdate(void)
{
    MDMESS_WORK *mess;
    
    mess = sys->mess;
    while(1) {
        mess = sys->mess;
        MdMessMain(mess, MDMESS_WIN_MAIN);
        HuPrcVSleep();
    }
}

static void TextWinUpdate(void)
{
    MDMESS_WORK *mess;
    
    mess = sys->mess;
    while(1) {
        mess = sys->mess;
        MdMessMain(mess, MDMESS_WIN_HELP);
        MdMessMain(mess, MDMESS_WIN_TEAM1);
        MdMessMain(mess, MDMESS_WIN_TEAM2);
        HuPrcVSleep();
    }
}

void MdInit(void)
{
    MdSpriteCreate(sys->sprite);
    HuPrcVSleep();
    MdRingCreate();
    MdModelCreate(sys->model, sys->camera);
    MdTexBufInit();
    HuPrcVSleep();
    HuDataDirClose(DATA_mdparty);
    HuPrcVSleep();
    MdModelAnimInit();
    sys->starProc = HuPrcChildCreate(StarUpdate, 107, 12288, 0, sys->objman);
    sys->playerSprProc = HuPrcChildCreate(PlayerSprUpdate, 108, 12288, 0, sys->objman);
    sys->chrProc = HuPrcChildCreate(ChrUpdate, 109, 12288, 0, sys->objman);
    sys->mainWinProc = HuPrcChildCreate(MainWinUpdate, 110, 4096, 0, sys->objman);
    sys->textWinProc = HuPrcChildCreate(TextWinUpdate, 111, 4096, 0, sys->objman);
    sys->loadState = MD_LOAD_STATE_NONE;
    sys->loadAsyncStat = 0;
    sys->loadProcReady = FALSE;
    sys->loadDir = DATA_selmenu;
    sys->loadProc = HuPrcChildCreate(LoadProc, 106, 12288, 0, sys->objman);
    HuPrcVSleep();
}

void MdClose(void)
{
    if(!sys->skipWipeF) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        WipeWait();
    }
    HuAudAllStop();
    MdMessKill(sys->mess);
    if(sys->ovlCallMode == MD_OVLCALL_GOTO) {
        omOvlGoto(sys->nextDll, 0, 0);
    } else if(sys->ovlCallMode == MD_OVLCALL_CALL) {
        omOvlCall(sys->nextDll, 0, 0);
    } else if(sys->ovlCallMode == MD_OVLCALL_RETURN) {
        omOvlReturn(1);
    }
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

void MdPartyFlowInit(void)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    MDSPRITE_WORK *sprite; //r29
    MDPARTY_CHR *chr; //r28
    
    MDCAMERA_WORK *camera; //sp+0x10
    MDMESS_WORK *mess; //sp+0xC
    MDPARTY_STAR *star; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    chr = &sys->chr[0];
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_WIN, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_LR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_UDLR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_PLAYERNO, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_COMDIF, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_TEAM, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_MAP_NAME, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_HANDICAP, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_NAME, FALSE);
    Hu3DModelDispOn(model->mdlId[MDMODEL_STAR]);
    Hu3DModelDispOn(model->mdlId[MDMODEL_STAGE]);
    Hu3DModelDispOff(model->mdlId[MDMODEL_STAGE_CHR]);
    for(i=0; i<10; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_MARIO]);
    }
    for(i=0; i<4; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_MAN_1P]);
    }
    for(i=0; i<4; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_COM_1P]);
    }
    for(i=0; i<10; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_CHR_00]);
    }
    for(i=0; i<2; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_BROYAL]);
    }
    Hu3DModelDispOff(model->mdlId[MDMODEL_VS]);
    for(i=0; i<7; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_MAP1]);
    }
    for(i=0; i<5; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_PACK0]);
    }
    Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_TURN]);
    Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_BONUS]);
    for(i=0; i<4; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_FINGER_1P]);
    }
    for(i=0; i<4; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_RING_1P]);
    }
    Hu3DModelDispOn(model->mdlId[MDMODEL_FONTBGFILTER]);
    CharMotionVoiceOnSet(0, 47, 0);
    CharMotionVoiceOnSet(1, 6, 0);
    CharMotionVoiceOnSet(2, 6, 0);
    CharMotionVoiceOnSet(3, 6, 0);
    CharMotionVoiceOnSet(4, 6, 0);
    CharMotionVoiceOnSet(5, 6, 0);
    CharMotionVoiceOnSet(6, 6, 0);
    CharMotionVoiceOnSet(7, 6, 0);
    CharMotionVoiceOnSet(8, 6, 0);
    CharMotionVoiceOnSet(9, 6, 0);
    for(i=0; i<4; i++) {
        MdRingDispSet(i, FALSE);
        MdRingBaseOfsSet(i, 0, 0, 0);
        MdRingScaleSet(i, 1.0);
    }
    sys->ringProc = HuPrcChildCreate(RingUpdate, 104, 12288, 0, sys->objman);
    for(i=0; i<10; i++) {
        chr[i].mode = MDPARTY_CHR_INIT;
    }
    HuPrcSleep(2);
    MdPartyReset();
    HuPrcVSleep();
    sys->loadState = MD_LOAD_STATE_BOARD_LOAD;
    sys->subMode = 0;
    sys->flowMode = MDPARTY_FLOW_START;
}

void MdPartyFlowStart(void)
{
    MDMESS_WORK *mess; //r31
    MDPARTY_STAR *star; //r30
    int choice; //r29
    
    MDCAMERA_WORK *camera; //sp+0x14
    MDSPRITE_WORK *sprite; //sp+0x10
    MDMODEL_WORK *model; //sp+0xC
    MDPARTY_PLAYERSPR *playerSpr; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            sys->gameType = 0;
            sys->subMode = 1;
            break;
        
        case 1:
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
            MdPartyAudExec(MDPARTY_AUD_MUS_START);
            WipeWait();
            MdPartyNameShow();
            MdPartyReset();
            MdMessWinOpen(mess, MDMESS_WIN_MAIN, MDMESS_WIN_NOSPEAKER, 0, 0, 0, 32, 372, 512, 64, 0);
            MdMessWinOpen(mess, MDMESS_WIN_HELP, MDMESS_WIN_NOSPEAKER, 0, 0, 1, 64, 340, 448, 32, 0);
            MdMessWinOpen(mess, MDMESS_WIN_TEAM1, MDMESS_WIN_NOSPEAKER, 0, 0, 2, 288, -64, 224, 32, 0);
            MdMessWinOpen(mess, MDMESS_WIN_TEAM2, MDMESS_WIN_NOSPEAKER, 0, 0, 3, 288, -64, 224, 32, 0);
            MdMessWinSpeedSet(mess, MDMESS_WIN_HELP, 0);
            MdMessWinSpeedSet(mess, MDMESS_WIN_TEAM1, 0);
            MdMessWinSpeedSet(mess, MDMESS_WIN_TEAM2, 0);
            MdMessWinAttrSet(mess, MDMESS_WIN_MAIN, HUWIN_ATTR_ALIGN_CENTER);
            MdMessWinAttrSet(mess, MDMESS_WIN_HELP, HUWIN_ATTR_ALIGN_CENTER);
            MdMessWinAttrSet(mess, MDMESS_WIN_TEAM1, HUWIN_ATTR_ALIGN_CENTER);
            MdMessWinAttrSet(mess, MDMESS_WIN_TEAM2, HUWIN_ATTR_ALIGN_CENTER);
            MdMessWinPad1Set(mess, MDMESS_WIN_MAIN);
            while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                HuPrcVSleep();
            }
            MdPartyWinOpen();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_ENTER, MDMESS_WIN_LIST_END);
            MdPartyAudExec(MDPARTY_AUD_SE_STAR_ENTER);
            while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                HuPrcVSleep();
            }
            repeatCont:
            if(GwCommon.partyContinue == TRUE) {
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_PARTYMODE_SYS_SAVE_CONTINUE, MDMESS_WIN_LIST_END);
                while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                    HuPrcVSleep();
                }
                choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                if(choice == -1) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_PARTYMODE_SYS_QUIT, MDMESS_WIN_LIST_END);
                    while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                        HuPrcVSleep();
                    }
                    choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                    if(choice == -1) {
                        goto repeatCont;
                    }
                    if(choice == 1) {
                        goto repeatCont;
                    }
                    if(choice == 0) {
                        star->motNext = MDPARTY_STAR_MOT_IDLE;
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        MdPartyWinClose();
                        MdPartyReset();
                        WipeColorSet(255, 255, 255);
                        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
                        MdPartyAudExec(MDPARTY_AUD_MUS_STOP);
                        WipeWait();
                        while(sys->loadState != MD_LOAD_STATE_WAIT) {
                            HuPrcVSleep();
                        }
                        omOvlHisChg(0, sys->ovlHis->ovl, 0, sys->ovlHis->stat);
                        sys->ovlCallMode = MD_OVLCALL_RETURN;
                        sys->nextDll = DLL_mdseldll;
                        sys->skipWipeF = TRUE;
                        sys->exitF = TRUE;
                        return;
                    }
                } else if(choice == 0) {
                    SLBoardLoad();
                    _SetFlag(FLAG_BOARD_STAR_RESET);
                    sys->loadProcReady = TRUE;
                    sys->loadDir = MBDataDir[GwSystem.boardNo];
                    star->motNext = MDPARTY_STAR_MOT_IDLE;
                    MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                    MdPartyNameHide();
                    MdPartyReset();
                    MdPartyWinClose();
                    MdPartyReset();
                    WipeColorSet(255, 255, 255);
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
                    MdPartyAudExec(MDPARTY_AUD_MUS_STOP);
                    WipeWait();
                    while(sys->loadState != MD_LOAD_STATE_DONE) {
                        HuPrcVSleep();
                    }
                    omOvlHisChg(0, sys->ovlHis->ovl, 1, sys->ovlHis->stat);
                    sys->ovlCallMode = MD_OVLCALL_CALL;
                    sys->nextDll = MBDll[GwSystem.boardNo];
                    sys->skipWipeF = TRUE;
                    sys->exitF = TRUE;
                    return;
                } else if(choice == 1) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_PARTYMODE_SYS_SAVE_WARN, MDMESS_WIN_LIST_END);
                    while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                        HuPrcVSleep();
                    }
                }
                
            }
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_PARTYMODE_SYS_INST, MDMESS_WIN_LIST_END);
            while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                HuPrcVSleep();
            }
            choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
            if(choice != -1 && choice != 1) {
                if(choice == 0) {
                    sys->loadProcReady = TRUE;
                    sys->loadDir = DATA_w10;
                    //TODO: Use Character Index constants
                    GwPlayerConf[0].charNo = 3;
                    GwPlayerConf[1].charNo = 0;
                    GwPlayerConf[2].charNo = 2;
                    GwPlayerConf[3].charNo = 4;
                    star->motNext = MDPARTY_STAR_MOT_IDLE;
                    MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                    MdPartyNameHide();
                    MdPartyReset();
                    MdPartyWinClose();
                    MdPartyReset();
                    WipeColorSet(255, 255, 255);
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
                    MdPartyAudExec(MDPARTY_AUD_MUS_STOP);
                    WipeWait();
                    while(sys->loadState != MD_LOAD_STATE_DONE) {
                        HuPrcVSleep();
                    }
                    _SetFlag(FLAG_BOARD_TUTORIAL);
                    GWTeamFSet(FALSE);
                    MBPartySaveInit(7); //TODO: Use Actual Board Index
                    omOvlHisChg(0, sys->ovlHis->ovl, 0, sys->ovlHis->stat);
                    sys->ovlCallMode = MD_OVLCALL_CALL;
                    sys->nextDll = DLL_w10dll;
                    sys->skipWipeF = TRUE;
                    sys->exitF = TRUE;
                    return;
                }
            }
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_START, MDMESS_WIN_LIST_END);
            while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                HuPrcVSleep();
            }
            MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
            MdPartyNameHide();
            MdPartyReset();
            star->mode = MDPARTY_STAR_BACK_START;
            star->motNext = MDPARTY_STAR_MOT_IDLE;
            while(star->mode != MDPARTY_STAR_BACK_END) {
                HuPrcVSleep();
            }
            MdPartyGameTypeStart();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BROYAL+sys->gameType, MDMESS_WIN_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            sys->cursorMode = MDPARTY_CURSOR_START;
            MdPartyCursorIn();
            MdPartyReset();
            sys->subMode = 2;
            break;
            
        case 2:
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(--sys->gameType == -1) {
                        sys->gameType = 1;
                    }
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->gameType = ++sys->gameType % 2;
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BROYAL+sys->gameType, MDMESS_WIN_LIST_END);
                MdPartyGameTypeChange();
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                sys->cursorMode = MDPARTY_CURSOR_START;
                MdPartyCursorOut();
                MdPartyReset();
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_PARTYMODE_SYS_QUIT, MDMESS_WIN_LIST_END);
                while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                    HuPrcVSleep();
                }
                choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                if(choice == -1 || choice == 1) {
                    sys->cursorMode = MDPARTY_CURSOR_START;
                    MdPartyCursorIn();
                    MdPartyReset();
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BROYAL+sys->gameType, MDMESS_WIN_LIST_END);
                    MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                } else if(choice == 0) {
                    star->motNext = MDPARTY_STAR_MOT_IDLE;
                    MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                    MdPartyWinClose();
                    MdPartyReset();
                    WipeColorSet(255, 255, 255);
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
                    MdPartyAudExec(MDPARTY_AUD_MUS_STOP);
                    WipeWait();
                    while(sys->loadState != MD_LOAD_STATE_WAIT) {
                        HuPrcVSleep();
                    }
                    omOvlHisChg(0, sys->ovlHis->ovl, 0, sys->ovlHis->stat);
                    sys->ovlCallMode = MD_OVLCALL_RETURN;
                    sys->nextDll = DLL_mdseldll;
                    sys->skipWipeF = TRUE;
                    sys->exitF = TRUE;
                }
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_START;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyGameTypeEnd();
                MdPartyReset();
                sys->subMode = 0;
                sys->flowMode = MDPARTY_FLOW_PLAYER_NUM_SEL;
            }
            break;
    }
}

void MdPartyFlowPlayerNumSel(void)
{
    MDCAMERA_WORK *camera = sys->camera;
    MDSPRITE_WORK *sprite = sys->sprite;
    MDMODEL_WORK *model = sys->model;
    MDMESS_WORK *mess = sys->mess;
    MDPARTY_STAR *star = sys->star;
    MDPARTY_PLAYERSPR *playerSpr = &sys->playerSpr[0];
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            MdPartyPadSetup();
            sys->prevPlayerNum = sys->playerNum;
            sys->subMode = 1;
            break;
        
        case 1:
            MdPartyCapPlayerStart();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            if(sys->playerNum == 3) {
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_PLAYER_NUM_4, MDMESS_WIN_LIST_END);
            } else {
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                    MESS_PARTYMODE_SYS_PLAYER_NUM,
                    MESS_PARTYMODE_SYS_PLAYER_NUM_1+sys->playerNum,
                    (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum,
                    MDMESS_WIN_LIST_END);
            }
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            if(sys->maxPlayerNum > 1) {
                sys->cursorMode = MDPARTY_CURSOR_PLAYERSEL;
                MdPartyCursorIn();
                MdPartyReset();
            }
            sys->subMode = 2;
            break;
        
        case 2:
            if(sys->maxPlayerNum > 1 && ((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT))) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    sys->prevPlayerNum = sys->playerNum;
                    if(--sys->playerNum == -1) {
                        sys->playerNum = sys->maxPlayerNum-1;
                    }
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->prevPlayerNum = sys->playerNum;
                    sys->playerNum = (++sys->playerNum)%sys->maxPlayerNum;
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                if(sys->playerNum == 3) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_PLAYER_NUM_4, MDMESS_WIN_LIST_END);
                } else {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                        MESS_PARTYMODE_SYS_PLAYER_NUM,
                        MESS_PARTYMODE_SYS_PLAYER_NUM_1+sys->playerNum,
                        (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum,
                        MDMESS_WIN_LIST_END);
                }
                MdPartyCapPlayerMove(sys->cursorDir);
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                if(sys->maxPlayerNum > 1) {
                    sys->cursorMode = MDPARTY_CURSOR_PLAYERSEL;
                    MdPartyCursorOut();
                    MdPartyReset();
                }
                MdPartyCapPlayerEnd();
                MdPartyReset();
                MdPartyGameTypeEnterBack();
                MdPartyReset();
                star->motNext = MDPARTY_STAR_MOT_TALK;
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BROYAL+sys->gameType, MDMESS_WIN_LIST_END);
                MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                sys->cursorMode = MDPARTY_CURSOR_START;
                MdPartyCursorIn();
                MdPartyReset();
                sys->subMode = 2;
                sys->flowMode = MDPARTY_FLOW_START;
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdPartyPlayerSetupInit();
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                if(sys->maxPlayerNum > 1) {
                    sys->cursorMode = MDPARTY_CURSOR_PLAYERSEL;
                    MdPartyCursorOut();
                    MdPartyReset();
                }
                MdPartyCapPlayerEnd();
                MdPartyReset();
                sys->subMode = 0;
                sys->flowMode = MDPARTY_FLOW_CHR_SEL;
            }
            break;
    }
}

void MdPartyFlowChrSel(void)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    MDPARTY_PLAYERSPR *playerSpr; //r29
    MDMESS_WORK *mess; //r28
    MDPARTY_STAR *star; //r27
    MDPARTY_CHR *chr; //r26
    MDSPRITE_WORK *sprite; //r25
    int difMax; //r24
    BOOL result; //r23
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    chr = &sys->chr[0];
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            for(i=0; i<4; i++) {
                if(sys->padEnable[i] == FALSE) {
                    sys->chrSel[i] = 0;
                } else {
                    sys->chrSel[i] = i;
                }
                sys->chrSelEnd[i] = FALSE;
                sys->comDif[i] = 1;
                HuSprBankSet(sprite->grpId[MDSPR_GRP_COMDIF], (int)i, sys->comDif[i]);
            }
            sys->subMode = 1;
            break;
        
        case 1: 
            MdPartyChrStart();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_CHR_SEL_MAN, MDMESS_WIN_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            MdPartyFingerStart();
            MdPartyReset();
            sys->subMode = 2;
            break;
        
        case 2:
            if(sys->chrSelEnd[0] == FALSE &&
                (playerSpr[0].mode == MDPARTY_PLAYERSPR_NONE ||
                playerSpr[0].mode == MDPARTY_PLAYERSPR_SHRINK_END) &&
                (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                    star->motNext = MDPARTY_STAR_MOT_IDLE;
                    MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                    MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                    for(i=0; i<4; i++) {
                        Hu3DModelDispOff(model->mdlId[i+MDMODEL_FINGER_1P]);
                    }
                    for(i=0; i<4; i++) {
                        playerSpr[i].mode = MDPARTY_PLAYERSPR_NONE;
                    }
                    MdSpriteGrpDispSet(sprite, MDSPR_GRP_PLAYERNO, FALSE);
                    MdSpriteGrpDispSet(sprite, MDSPR_GRP_COMDIF, FALSE);
                    for(i=0; i<4; i++) {
                        if(sys->chrSelEnd[i] == TRUE) {
                            Hu3DMotionShiftSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], model->motId[(sys->chrSel[i]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                            chr[sys->chrSel[i]].mode = MDPARTY_CHR_SELECT_WAIT;
                        }
                    }
                    for(i=0; i<4; i++) {
                        MdRingDispSet(i, FALSE);
                    }
                    for(i=0; i<4; i++) {
                        sys->chrSelEnd[i] = FALSE;
                    }
                    MdPartyChrCancel();
                    MdPartyReset();
                    MdPartyPadSetup();
                    sys->prevPlayerNum = sys->playerNum;
                    MdPartyCapPlayerStart();
                    MdPartyReset();
                    star->motNext = MDPARTY_STAR_MOT_TALK;
                    if(sys->playerNum == 3) {
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_PLAYER_NUM_4, MDMESS_WIN_LIST_END);
                    } else {
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                            MESS_PARTYMODE_SYS_PLAYER_NUM,
                            MESS_PARTYMODE_SYS_PLAYER_NUM_1+sys->playerNum,
                            (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum,
                            MDMESS_WIN_LIST_END);
                    }
                    MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                    if(sys->maxPlayerNum > 1) {
                        sys->cursorMode = MDPARTY_CURSOR_PLAYERSEL;
                        MdPartyCursorIn();
                        MdPartyReset();
                    }
                    sys->subMode = 2;
                    sys->flowMode = MDPARTY_FLOW_PLAYER_NUM_SEL;
                } else {
                    for(i=0; i<4; i++) {
                        if(sys->padEnable[i] == FALSE) {
                            continue;
                        }
                        if(playerSpr[i].mode == MDPARTY_PLAYERSPR_NONE ||
                            playerSpr[i].mode == MDPARTY_PLAYERSPR_GROW_END ||
                            playerSpr[i].mode == MDPARTY_PLAYERSPR_SHRINK_END) {
                                result = MdPartyChrSelCursorUpdate(i, FALSE);
                                if(result == TRUE) {
                                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                                    MdPartyPlayerSprUpdate();
                                } else {
                                    if((HuPadBtnDown[i] & PAD_BUTTON_B) && sys->chrSelEnd[i] == TRUE) {
                                        sys->chrSelEnd[i] = FALSE;
                                        Hu3DModelDispOn(model->mdlId[i+MDMODEL_FINGER_1P]);
                                        playerSpr[i].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                                        Hu3DMotionShiftSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], model->motId[(sys->chrSel[i]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                                        chr[sys->chrSel[i]].mode = MDPARTY_CHR_SELECT_WAIT;
                                        MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                                        MdRingModeSet(i, 4);
                                    } else if((HuPadBtnDown[i] & PAD_BUTTON_A) && sys->chrSelEnd[i] == FALSE) {
                                        sys->chrSelEnd[i] = TRUE;
                                        Hu3DModelDispOff(model->mdlId[i+MDMODEL_FINGER_1P]);
                                        playerSpr[i].mode = MDPARTY_PLAYERSPR_GROW_START;
                                        Hu3DMotionShiftSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO],
                                            model->motId[(sys->chrSel[i]*2)+MDMODEL_MOT_MARIO_WIN],
                                            0,
                                            8,
                                            HU3D_MOTATTR_NONE);
                                        chr[sys->chrSel[i]].mode = MDPARTY_CHR_SELECT;
                                        MdPartyAudExec(MDPARTY_AUD_SE_CMN_CHRSEL);
                                        MdPartyAudExec(sys->chrSel[i]+MDPARTY_AUD_SE_CHR_MARIO);
                                        MdRingModeSet(i, 1);
                                        MdRingBaseMdlSet(i, sys->chrSel[i]+MDMODEL_CAP_CHR_00);
                                    }
                                }
                            }
                    }
                    if(((sys->chrSelEnd[0] == TRUE && playerSpr[0].mode == MDPARTY_PLAYERSPR_GROW_END) || sys->padEnable[0] == FALSE) &&
                        ((sys->chrSelEnd[1] == TRUE && playerSpr[1].mode == MDPARTY_PLAYERSPR_GROW_END) || sys->padEnable[1] == FALSE) &&
                        ((sys->chrSelEnd[2] == TRUE && playerSpr[2].mode == MDPARTY_PLAYERSPR_GROW_END) || sys->padEnable[2] == FALSE) && 
                        ((sys->chrSelEnd[3] == TRUE && playerSpr[3].mode == MDPARTY_PLAYERSPR_GROW_END) || sys->padEnable[3] == FALSE)) {
                        if(sys->playerNum == 3) {
                            if(sys->gameType == 0) {
                                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                                star->motNext = MDPARTY_STAR_MOT_IDLE;
                                MdPartyChrCenter();
                                MdPartyReset();
                                sys->subMode = 0;
                                sys->flowMode = MDPARTY_FLOW_MAP_SEL;
                            } else if(sys->gameType == 1) {
                                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                                star->motNext = MDPARTY_STAR_MOT_IDLE;
                                MdPartyChrTeamStart();
                                MdPartyReset();
                                for(i=0; i<4; i++) {
                                    if(i < 2) {
                                        MdRingModeSet(i, 9);
                                    } else {
                                        MdRingModeSet(i, 11);
                                    }
                                }
                                sys->subMode = 0;
                                sys->flowMode = MDPARTY_FLOW_TEAM_SEL;
                            }
                        } else {
                            sys->selComNo = 0;
                            MdPartyChrComInit();
                            MdPartyPlayerSprUpdate();
                            Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                            model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->x = 4;
                            model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->y = 4;
                            model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->z = 4;
                            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                                MESS_PARTYMODE_SYS_CHR_SEL_COM,
                                (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum-sys->selComNo,
                                MDMESS_WIN_LIST_END);
                            sys->subMode = 3;
                        }
                    }
                }
            break;
        
        case 3:
            if(sys->selComNo == 0 && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                Hu3DModelDispOff(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                for(i=0; i<4; i++) {
                    if(sys->chrSelEnd[i] == TRUE) {
                        Hu3DMotionShiftSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], model->motId[(sys->chrSel[i]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                        chr[sys->chrSel[i]].mode = MDPARTY_CHR_SELECT_WAIT;
                        MdRingModeSet(i, 4);
                        playerSpr[i].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                        sys->chrSelEnd[i] = FALSE;
                    }
                }
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_CHR_SEL_MAN, MDMESS_WIN_LIST_END);
                MdPartyFingerStart();
                MdPartyReset();
                sys->subMode = 2;
            } else {
                result = MdPartyChrSelCursorUpdate(sys->comPlayerNo[sys->selComNo], TRUE);
                if(result == TRUE) {
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                    MdPartyPlayerSprUpdate();
                } else if((HuPadBtnDown[0] & PAD_BUTTON_B) && sys->selComNo > 0) {
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                    Hu3DModelDispOff(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                    sys->selComNo--;
                    Hu3DMotionShiftSet(model->mdlId[sys->chrSel[sys->comPlayerNo[sys->selComNo]]+MDMODEL_MARIO], model->motId[(sys->chrSel[sys->comPlayerNo[sys->selComNo]]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                    chr[sys->chrSel[sys->comPlayerNo[sys->selComNo]]].mode = MDPARTY_CHR_SELECT_WAIT;
                    MdRingModeSet(sys->comPlayerNo[sys->selComNo], 4);
                    playerSpr[sys->comPlayerNo[sys->selComNo]].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                                MESS_PARTYMODE_SYS_CHR_SEL_COM,
                                (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum-sys->selComNo,
                                MDMESS_WIN_LIST_END);
                    while(playerSpr[sys->comPlayerNo[sys->selComNo]].mode != MDPARTY_PLAYERSPR_SHRINK_END) {
                        HuPrcVSleep();
                    }
                    Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                    sys->chrSelEnd[sys->comPlayerNo[sys->selComNo]] = FALSE;
                    MdPartyPlayerSprUpdate();
                } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                    if(sys->gameType == 0) {
                        MdPartyAudExec(MDPARTY_AUD_SE_CMN_CHRSEL);
                    } else if(sys->gameType == 1) {
                        if(sys->selComNo+1 < (3-sys->playerNum)) {
                            MdPartyAudExec(MDPARTY_AUD_SE_CMN_CHRSEL);
                        } else {
                            MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                        }
                    }
                    playerSpr[sys->comPlayerNo[sys->selComNo]].mode = MDPARTY_PLAYERSPR_GROW_START;
                    if(sys->gameType == 0) {
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_MGMODE_SYS_DIF_SEL, MDMESS_WIN_LIST_END);
                    }
                    Hu3DModelDispOff(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                    Hu3DMotionShiftSet(model->mdlId[sys->chrSel[sys->comPlayerNo[sys->selComNo]]+MDMODEL_MARIO],
                        model->motId[(sys->chrSel[sys->comPlayerNo[sys->selComNo]]*2)+MDMODEL_MOT_MARIO_WIN],
                        0,
                        8,
                        HU3D_MOTATTR_NONE);
                    chr[sys->chrSel[sys->comPlayerNo[sys->selComNo]]].mode = MDPARTY_CHR_SELECT;
                    MdPartyAudExec(sys->chrSel[sys->comPlayerNo[sys->selComNo]]+MDPARTY_AUD_SE_CHR_MARIO);
                    MdRingModeSet(sys->comPlayerNo[sys->selComNo], 1);
                    MdRingBaseOfsSet(sys->comPlayerNo[sys->selComNo], 0, 0, 0);
                    MdRingBaseMdlSet(sys->comPlayerNo[sys->selComNo], sys->chrSel[sys->comPlayerNo[sys->selComNo]]+MDMODEL_CAP_CHR_00);
                    MdRingScaleSet(sys->comPlayerNo[sys->selComNo], 1.0);
                    while(playerSpr[sys->comPlayerNo[sys->selComNo]].mode != MDPARTY_PLAYERSPR_GROW_END) {
                        HuPrcVSleep();
                    }
                    if(sys->gameType == 0) {
                        sys->cursorMode = MDPARTY_CURSOR_CHR_DIF_SEL;
                        MdPartyCursorIn();
                        MdPartyReset();
                        HuSprDispOff(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->comPlayerNo[sys->selComNo]);
                        HuSprDispOn(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->comPlayerNo[sys->selComNo]);
                        MdPartyPlayerSprUpdate();
                        sys->subMode = 4;
                    } else if(sys->gameType == 1) {
                        sys->chrSelEnd[sys->comPlayerNo[sys->selComNo]] = TRUE;
                        if(sys->selComNo+1 < 3-sys->playerNum) {
                            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                                MESS_PARTYMODE_SYS_CHR_SEL_COM,
                                (MESS_PARTYMODE_SYS_PLAYER_NUM_1+1)-sys->playerNum-sys->selComNo,
                                MDMESS_WIN_LIST_END);
                        } else {
                            MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                            MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                            star->motNext = MDPARTY_STAR_MOT_IDLE;
                        }
                        sys->selComNo++;
                        if(sys->selComNo < 3-sys->playerNum) {
                            MdPartyChrComInit();
                            MdPartyPlayerSprUpdate();
                            Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                            model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->x = 4;
                            model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->y = 4;
                            model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->z = 4;
                        } else if(sys->selComNo >= 3-sys->playerNum) {
                            for(i=0; i<4; i++) {
                                if(i < 2) {
                                    MdRingModeSet(i, 9);
                                } else {
                                    MdRingModeSet(i, 11);
                                }
                            }
                            MdPartyChrTeamStart();
                            MdPartyReset();
                            sys->subMode = 0;
                            sys->flowMode = MDPARTY_FLOW_TEAM_SEL;
                        }
                    }
                }
            }
            break;
        
        case 4:
            if(GwCommon.veryHardUnlock == TRUE) {
                difMax = 4;
            } else {
                difMax = 3;
            }
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(--sys->comDif[sys->comPlayerNo[sys->selComNo]] == -1) {
                        sys->comDif[sys->comPlayerNo[sys->selComNo]] = difMax-1;
                    }
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->comDif[sys->comPlayerNo[sys->selComNo]] = ++sys->comDif[sys->comPlayerNo[sys->selComNo]]%difMax;
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                MdPartyPlayerSprUpdate();
                HuSprBankSet(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->comPlayerNo[sys->selComNo], sys->comDif[sys->comPlayerNo[sys->selComNo]]);
                sys->cursorMode = MDPARTY_CURSOR_CHR_DIF_SEL;
                MdPartyCursorDirLR(sys->cursorDir);
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                playerSpr[sys->comPlayerNo[sys->selComNo]].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                Hu3DMotionShiftSet(model->mdlId[sys->chrSel[sys->comPlayerNo[sys->selComNo]]+MDMODEL_MARIO], model->motId[(sys->chrSel[sys->comPlayerNo[sys->selComNo]]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                chr[sys->chrSel[sys->comPlayerNo[sys->selComNo]]].mode = MDPARTY_CHR_SELECT_WAIT;
                MdRingModeSet(sys->comPlayerNo[sys->selComNo], 4);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                    MESS_PARTYMODE_SYS_CHR_SEL_COM,
                    (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum-sys->selComNo,
                    MDMESS_WIN_LIST_END);
                sys->cursorMode = MDPARTY_CURSOR_CHR_DIF_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                sys->subMode = 3;
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                sys->chrSelEnd[sys->comPlayerNo[sys->selComNo]] = TRUE;
                if(sys->selComNo+1 < 3-sys->playerNum) {
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_CHRSEL);
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                        MESS_PARTYMODE_SYS_CHR_SEL_COM,
                        (MESS_PARTYMODE_SYS_PLAYER_NUM_1+1)-sys->playerNum-sys->selComNo,
                        MDMESS_WIN_LIST_END);
                } else {
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                    MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                    MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                    star->motNext = MDPARTY_STAR_MOT_IDLE;
                }
                sys->cursorMode = MDPARTY_CURSOR_CHR_DIF_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                sys->selComNo++;
                if(sys->selComNo < 3-sys->playerNum) {
                    MdPartyChrComInit();
                    MdPartyPlayerSprUpdate();
                    Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                    model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->x = 4;
                    model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->y = 4;
                    model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->z = 4;
                    sys->subMode = 3;
                } else if(sys->selComNo >= 3-sys->playerNum) {
                    MdPartyChrCenter();
                    MdPartyReset();
                    sys->subMode = 0;
                    sys->flowMode = MDPARTY_FLOW_MAP_SEL;
                }
            }
            break;
    }
}

void MdPartyFlowTeamSel(void)
{
    int i; //r31
    MDMESS_WORK *mess; //r30
    MDSPRITE_WORK *sprite; //r29
    MDMODEL_WORK *model; //r28
    MDPARTY_STAR *star; //r27
    int choice; //r26
    MDPARTY_PLAYERSPR *playerSpr; //r25
    MDPARTY_CHR *chr; //r24
    int difMax; //r23
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    chr = &sys->chr[0];
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            sys->teamOrderNo = 0;
            MdPartyTeamPlayerUpdate();
            MdPartyTeamOrderBackup();
            sys->subMode = 1;
            break;
        
        case 1:
            MdMessWinMesSet(mess, MDMESS_WIN_TEAM1, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_TEAM2, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TEAM_SEL, MDMESS_WIN_LIST_END);
            MdPartyTeamStart();
            MdPartyReset();
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            sys->cursorMode = MDPARTY_CURSOR_TEAM_SEL;
            MdPartyCursorIn();
            MdPartyReset();
            sys->subMode = 2;
            break;
        
        case 2:
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    sys->cursorDir = 0;
                    if(--sys->teamOrderNo == -1) {
                        sys->teamOrderNo = 2;
                    }
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->cursorDir = 1;
                    sys->teamOrderNo = (++sys->teamOrderNo)%3;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                MdPartyTeamOrderBackup();
                MdPartyTeamPlayerUpdate();
                for(i=0; i<4; i++) {
                    if(sys->teamOrder[i] < 2) {
                        MdRingModeSet(i, 9);
                    } else {
                        MdRingModeSet(i, 11);
                    }
                }
                MdPartyChrTeamMove();
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_TEAM_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyTeamCancel();
                MdPartyReset();
                if(sys->playerNum == 3) {
                    sys->selComNo = 0;
                    for(i=0; i<4; i++) {
                        sys->chrSelEnd[i] = FALSE;
                        playerSpr[i].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                        Hu3DMotionShiftSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], model->motId[(sys->chrSel[i]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                        chr[sys->chrSel[i]].mode = MDPARTY_CHR_SELECT_WAIT;
                        MdRingModeSet(i, 4);
                    }
                    MdPartyChrTeamCenter();
                    MdPartyReset();
                    star->motNext = MDPARTY_STAR_MOT_TALK;
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_CHR_SEL_MAN, MDMESS_WIN_LIST_END);
                    MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                    MdPartyFingerStart();
                    MdPartyReset();
                    sys->subMode = 2;
                    sys->flowMode = MDPARTY_FLOW_CHR_SEL;
                } else {
                    sys->selComNo = 0;
                    for(i=0; i<3; i++) {
                        if(sys->comPlayerNo[i] != -1) {
                            sys->chrSelEnd[sys->comPlayerNo[i]] = FALSE;
                            playerSpr[sys->comPlayerNo[i]].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                            Hu3DMotionShiftSet(model->mdlId[sys->chrSel[sys->comPlayerNo[i]]+MDMODEL_MARIO], model->motId[(sys->chrSel[sys->comPlayerNo[i]]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                            chr[sys->chrSel[sys->comPlayerNo[i]]].mode = MDPARTY_CHR_SELECT_WAIT;
                            MdRingModeSet(sys->comPlayerNo[i], 4);
                        }
                    }
                    for(i=0; i<4; i++) {
                        if(sys->padEnable[i]) {
                            MdRingModeSet(i, 13);
                        }
                    }
                    MdPartyChrTeamCenter();
                    MdPartyReset();
                    MdPartyChrComInit();
                    MdPartyPlayerSprUpdate();
                    Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                    model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->x = 4;
                    model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->y = 4;
                    model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->z = 4;
                    star->motNext = MDPARTY_STAR_MOT_TALK;
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                        MESS_PARTYMODE_SYS_CHR_SEL_COM,
                        (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum-sys->selComNo,
                        MDMESS_WIN_LIST_END);
                    MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                    sys->subMode = 3;
                    sys->flowMode = MDPARTY_FLOW_CHR_SEL;
                }
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                MdMessWinMesSet(mess, MDMESS_WIN_TEAM1, FALSE, sys->teamNo[0]+MESS_TEAMNAME_START, MDMESS_WIN_LIST_END);
                MdMessWinMesSet(mess, MDMESS_WIN_TEAM2, FALSE, sys->teamNo[1]+MESS_TEAMNAME_START, MDMESS_WIN_LIST_END);
                sys->cursorMode = MDPARTY_CURSOR_TEAM_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                if(sys->playerNum == 3) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE,
                        MESS_PARTYMODE_SYS_TEAM_PAIR,
                        sys->teamNo[0]+MESS_TEAMNAME_START,
                        sys->teamNo[1]+MESS_TEAMNAME_START,
                        MDMESS_WIN_LIST_END);
                    while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                        HuPrcVSleep();
                    }
                    choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                    if(choice == -1 || choice == 1) {
                        MdMessWinMesSet(mess, MDMESS_WIN_TEAM1, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_TEAM2, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TEAM_SEL, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        sys->cursorMode = MDPARTY_CURSOR_TEAM_SEL;
                        MdPartyCursorIn();
                        MdPartyReset();
                    } else if(choice == 0) {
                        star->motNext = MDPARTY_STAR_MOT_IDLE;
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                        MdPartyTeamSelEnd();
                        MdPartyReset();
                        MdPartyChrTeamMoveEnd();
                        MdPartyReset();
                        sys->subMode = 0;
                        sys->flowMode = MDPARTY_FLOW_MAP_SEL;
                    }
                } else {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_DIF_SEL, MDMESS_WIN_LIST_END);
                    for(i=0; i<4; i++) {
                        sys->comDif[i] = 1;
                        HuSprBankSet(sprite->grpId[MDSPR_GRP_COMDIF], (int)i, sys->comDif[i]);
                    }
                    sys->selComNo = 0;
                    sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                    MdPartyCursorIn();
                    MdPartyReset();
                    HuSprDispOff(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->teamComPlayerNo[sys->selComNo]);
                    HuSprDispOn(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo]);
                    MdPartyPlayerSprUpdate();
                    sys->subMode = 3;
                }
            }
            break;
        
        case 3:
            if(GwCommon.veryHardUnlock == TRUE) {
                difMax = 4;
            } else {
                difMax = 3;
            }
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(--sys->comDif[sys->teamComPlayerNo[sys->selComNo]] == -1) {
                        sys->comDif[sys->teamComPlayerNo[sys->selComNo]] = difMax-1;
                    }
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->comDif[sys->teamComPlayerNo[sys->selComNo]] = ++sys->comDif[sys->teamComPlayerNo[sys->selComNo]]%difMax;
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                MdPartyPlayerSprUpdate();
                HuSprBankSet(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo], sys->comDif[sys->teamComPlayerNo[sys->selComNo]]);
                sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                MdPartyCursorDirLR(sys->cursorDir);
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                if(sys->selComNo == 0) {
                    HuSprDispOn(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->teamComPlayerNo[sys->selComNo]);
                    HuSprDispOff(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo]);
                    MdMessWinMesSet(mess, MDMESS_WIN_TEAM1, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
                    MdMessWinMesSet(mess, MDMESS_WIN_TEAM2, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TEAM_SEL, MDMESS_WIN_LIST_END);
                    sys->cursorMode = MDPARTY_CURSOR_TEAM_SEL;
                    MdPartyCursorIn();
                    MdPartyReset();
                    sys->subMode = 2;
                } else {
                    HuSprDispOn(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->teamComPlayerNo[sys->selComNo]);
                    HuSprDispOff(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo]);
                    sys->selComNo--;
                    sys->chrSelEnd[sys->comPlayerNo[sys->selComNo]] = FALSE;
                    sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                    MdPartyCursorIn();
                    MdPartyReset();
                    HuSprDispOff(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->teamComPlayerNo[sys->selComNo]);
                    HuSprDispOn(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo]);
                    MdPartyPlayerSprUpdate();
                }
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                sys->chrSelEnd[sys->comPlayerNo[sys->selComNo]] = TRUE;
                if(sys->selComNo+1 < 3-sys->playerNum) {
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_CHRSEL);
                } else {
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                    MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                }
                sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                sys->selComNo++;
                if(sys->selComNo < 3-sys->playerNum) {
                    sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                    MdPartyCursorIn();
                    MdPartyReset();
                    HuSprDispOff(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->teamComPlayerNo[sys->selComNo]);
                    HuSprDispOn(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo]);
                    MdPartyPlayerSprUpdate();
                } else if(sys->selComNo >= 3-sys->playerNum) {
                    MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE,
                        MESS_PARTYMODE_SYS_TEAM_PAIR,
                        sys->teamNo[0]+MESS_TEAMNAME_START,
                        sys->teamNo[1]+MESS_TEAMNAME_START,
                        MDMESS_WIN_LIST_END);
                    while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                        HuPrcVSleep();
                    }
                    choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                    if(choice == -1 || choice == 1) {
                        sys->selComNo--;
                        sys->chrSelEnd[sys->comPlayerNo[sys->selComNo]] = FALSE;
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_DIF_SEL, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                        MdPartyCursorIn();
                        MdPartyReset();
                        HuSprDispOff(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->teamComPlayerNo[sys->selComNo]);
                        HuSprDispOn(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo]);
                        MdPartyPlayerSprUpdate();
                    } else if(choice == 0) {
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                        star->motNext = MDPARTY_STAR_MOT_IDLE;
                        MdPartyTeamSelEnd();
                        MdPartyReset();
                        MdPartyChrTeamMoveEnd();
                        MdPartyReset();
                        sys->subMode = 0;
                        sys->flowMode = MDPARTY_FLOW_MAP_SEL;
                    }
                }
            }
            break;
    }
}

void MdPartyFlowMapSel(void)
{
    int i; //r31
    MDMESS_WORK *mess; //r30
    MDMODEL_WORK *model; //r29
    MDSPRITE_WORK *sprite; //r28
    MDPARTY_STAR *star; //r27
    int mapMax; //r26
    MDPARTY_PLAYERSPR *playerSpr; //r25
    MDPARTY_CHR *chr; //r24
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    chr = &sys->chr[0];
    mapMax = 7;
    if(GwCommon.w07Unlock == TRUE) {
        mapMax = 7;
    } else {
        mapMax = 6;
    }
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            sys->selMapNo = 0;
            sys->selMapRadius = 300;
            if(mapMax == 7) {
                sys->selMapAngle = ((360.0f/7.0f)*sys->selMapNo)+90;
            } else {
                sys->selMapAngle = ((360.0f/6.0f)*sys->selMapNo)+90;
            }
            sys->subMode = 1;
            break;
        
        case 1:
            MdPartyMapStart();
            MdPartyReset();
            MdPartyMapNameStart();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, (sys->selMapNo*2)+MESS_MAPNAME_DESC_1, MDMESS_WIN_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            sys->cursorMode = MDPARTY_CURSOR_MAP_SEL;
            MdPartyCursorIn();
            MdPartyReset();
            sys->subMode = 2;
            break;
        
        case 2:
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                sys->prevSelMapNo = sys->selMapNo;
                if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->selMapNo = ++sys->selMapNo%mapMax;
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(--sys->selMapNo == -1) {
                        sys->selMapNo = mapMax-1;
                    }
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, (sys->selMapNo*2)+MESS_MAPNAME_DESC_1, MDMESS_WIN_LIST_END);
                for(i=0; i<7; i++) {
                    if(i == sys->selMapNo) {
                        HuSprDispOn(sprite->grpId[MDSPR_GRP_MAP_NAME], (int)i);
                    } else {
                        HuSprDispOff(sprite->grpId[MDSPR_GRP_MAP_NAME], (int)i);
                    }
                }
                MdPartyMapMove();
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_MAP_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyMapNameCancel();
                MdPartyReset();
                MdPartyMapCancel();
                MdPartyReset();
                if(sys->gameType == 0) {
                    if(sys->playerNum == 3) {
                        for(i=0; i<4; i++) {
                            if(sys->chrSelEnd[i] == TRUE) {
                                Hu3DMotionShiftSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], model->motId[(sys->chrSel[i]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                                chr[sys->chrSel[i]].mode = MDPARTY_CHR_SELECT_WAIT;
                                MdRingModeSet(i, 4);
                                playerSpr[i].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                                sys->chrSelEnd[i] = FALSE;
                            }
                        }
                        MdPartyChrReset();
                        MdPartyReset();
                        star->motNext = MDPARTY_STAR_MOT_TALK;
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_CHR_SEL_MAN, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        MdPartyFingerStart();
                        MdPartyReset();
                        sys->subMode = 2;
                        sys->flowMode = MDPARTY_FLOW_CHR_SEL;
                    } else {
                        for(i=0; i<3; i++) {
                            if(sys->comPlayerNo[i] != -1) {
                                Hu3DMotionShiftSet(model->mdlId[sys->chrSel[sys->comPlayerNo[i]]+MDMODEL_MARIO], model->motId[(sys->chrSel[sys->comPlayerNo[i]]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                                chr[sys->chrSel[sys->comPlayerNo[i]]].mode = MDPARTY_CHR_SELECT_WAIT;
                                MdRingModeSet(sys->comPlayerNo[i], 4);
                                playerSpr[sys->comPlayerNo[i]].mode = MDPARTY_PLAYERSPR_SHRINK_START;
                                sys->chrSelEnd[sys->comPlayerNo[i]] = FALSE;
                            }
                        }
                        MdPartyChrReset();
                        MdPartyReset();
                        sys->selComNo = 0;
                        MdPartyChrComInit();
                        MdPartyPlayerSprUpdate();
                        Hu3DModelDispOn(model->mdlId[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]);
                        model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->x = 4;
                        model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->y = 4;
                        model->scaleP[sys->comPlayerNo[sys->selComNo]+MDMODEL_FINGER_1P]->z = 4;
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE,
                            MESS_PARTYMODE_SYS_CHR_SEL_COM,
                            (MESS_PARTYMODE_SYS_PLAYER_NUM_1+2)-sys->playerNum-sys->selComNo,
                            MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        star->motNext = MDPARTY_STAR_MOT_TALK;
                        sys->subMode = 3;
                        sys->flowMode = MDPARTY_FLOW_CHR_SEL;
                    }
                } else if(sys->gameType == 1) {
                    if(sys->playerNum == 3) {
                        MdPartyChrTeamReset();
                        MdPartyReset();
                        MdMessWinMesSet(mess, MDMESS_WIN_TEAM1, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_TEAM2, FALSE, MESS_TEAMNAME_UNKNOWN, MDMESS_WIN_LIST_END);
                        star->motNext = MDPARTY_STAR_MOT_TALK;
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TEAM_SEL, MDMESS_WIN_LIST_END);
                        MdPartyTeamReset();
                        MdPartyReset();
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        sys->cursorMode = MDPARTY_CURSOR_TEAM_SEL;
                        MdPartyCursorIn();
                        MdPartyReset();
                        sys->subMode = 2;
                        sys->flowMode = MDPARTY_FLOW_TEAM_SEL;
                    } else {
                        MdPartyChrTeamReset();
                        MdPartyReset();
                        MdPartyTeamReset();
                        MdPartyReset();
                        for(i=0; i<3; i++) {
                            if(sys->comPlayerNo[i] != -1) {
                                HuSprDispOn(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->comPlayerNo[i]);
                                HuSprDispOff(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->comPlayerNo[i]);
                            }
                        }
                        star->motNext = MDPARTY_STAR_MOT_TALK;
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_DIF_SEL, MDMESS_WIN_LIST_END);
                        for(i=0; i<4; i++) {
                            HuSprBankSet(sprite->grpId[MDSPR_GRP_COMDIF], (int)i, sys->comDif[i]);
                        }
                        sys->selComNo = 0;
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        sys->cursorMode = MDPARTY_CURSOR_TEAM_DIF_SEL;
                        MdPartyCursorIn();
                        MdPartyReset();
                        HuSprDispOff(sprite->grpId[MDSPR_GRP_PLAYERNO], (int)sys->teamComPlayerNo[sys->selComNo]);
                        HuSprDispOn(sprite->grpId[MDSPR_GRP_COMDIF], (int)sys->teamComPlayerNo[sys->selComNo]);
                        MdPartyPlayerSprUpdate();
                        sys->subMode = 3;
                        sys->flowMode = MDPARTY_FLOW_TEAM_SEL;
                    }
                }
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_MAP_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyMapEnd();
                MdPartyReset();
                sys->subMode = 0;
                sys->flowMode = MDPARTY_FLOW_TURN_SEL;
            }
            break;
    }
}

void MdPartyFlowTurnSel(void)
{
    MDMESS_WORK *mess; //r31
    MDPARTY_STAR *star; //r30
    MDMODEL_WORK *model; //r29
    
    MDCAMERA_WORK *camera; //sp+0x14
    MDSPRITE_WORK *sprite; //sp+0x10
    MDPARTY_PLAYERSPR *playerSpr; //sp+0xC
    MDPARTY_CHR *chr; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    chr = &sys->chr[0];
    
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            sys->selTurn = 2;
            Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_CAP_TURN], sys->selTurn);
            sys->subMode = 1;
            break;
        
        case 1:
            MdPartyCapTurnStart();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TURN_SEL, MESS_PARTYMODE_SYS_TURN_10+sys->selTurn, MDMESS_WIN_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            sys->cursorMode = MDPARTY_CURSOR_TURN_SEL;
            MdPartyCursorIn();
            MdPartyReset();
            sys->subMode = 2;
            break;
        
        case 2:
            if(((HuPadBtnDown[0] & PAD_TRIGGER_L) && sys->selTurn != 0) ||
                ((HuPadBtnDown[0] & PAD_TRIGGER_R) && sys->selTurn != 8)) {
                    if(HuPadBtnDown[0] & PAD_TRIGGER_L) {
                        sys->selTurn = 0;
                        sys->cursorDir = 0;
                    } else if(HuPadBtnDown[0] & PAD_TRIGGER_R) {
                        sys->selTurn = 8;
                        sys->cursorDir = 1;
                    }
                    MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TURN_SEL, MESS_PARTYMODE_SYS_TURN_10+sys->selTurn, MDMESS_WIN_LIST_END);
                    MdPartyCapTurnUpdate();
                    MdPartyReset();
                } else if(((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) && sys->selTurn != 0) ||
                    ((sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) && sys->selTurn != 8)) {
                        if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                            if(--sys->selTurn == -1) {
                                sys->selTurn = 0;
                            }
                            sys->cursorDir = 0;
                        } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                            if(++sys->selTurn == 9) {
                                sys->selTurn = 8;
                            }
                            sys->cursorDir = 1;
                        }
                        MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TURN_SEL, MESS_PARTYMODE_SYS_TURN_10+sys->selTurn, MDMESS_WIN_LIST_END);
                        MdPartyCapTurnUpdate();
                        MdPartyReset();
                    } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                        MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                        star->motNext = MDPARTY_STAR_MOT_IDLE;
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                        sys->cursorMode = MDPARTY_CURSOR_TURN_SEL;
                        MdPartyCursorOut();
                        MdPartyReset();
                        MdPartyCapTurnCancel();
                        MdPartyReset();
                        MdPartyMapReset();
                        MdPartyReset();
                        star->motNext = MDPARTY_STAR_MOT_TALK;
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, (sys->selMapNo*2)+MESS_MAPNAME_DESC_1, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        sys->cursorMode = MDPARTY_CURSOR_MAP_SEL;
                        MdPartyCursorIn();
                        MdPartyReset();
                        sys->subMode = 2;
                        sys->flowMode = MDPARTY_FLOW_MAP_SEL;
                    } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                        MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                        star->motNext = MDPARTY_STAR_MOT_IDLE;
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                        sys->cursorMode = MDPARTY_CURSOR_TURN_SEL;
                        MdPartyCursorOut();
                        MdPartyReset();
                        MdPartyCapTurnEnd();
                        MdPartyReset();
                        sys->subMode = 0;
                        sys->flowMode = MDPARTY_FLOW_PACK_SEL;
                    }
            break;
    }
}

void MdPartyFlowPackSel(void)
{
    MDMESS_WORK *mess; //r31
    MDPARTY_STAR *star; //r30
    int i; //r29
    MDMODEL_WORK *model; //r28
    
    MDCAMERA_WORK *camera; //sp+0x14
    MDSPRITE_WORK *sprite; //sp+0x10
    MDPARTY_PLAYERSPR *playerSpr; //sp+0xC
    MDPARTY_CHR *chr; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    chr = &sys->chr[0];
    
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            sys->selPack = 0;
            sys->subMode = 1;
            break;
        
        case 1:
            MdPartyCapPackStart();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_PACK0+sys->selPack, MDMESS_WIN_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            sys->cursorMode = MDPARTY_CURSOR_PACK_SEL;
            MdPartyCursorIn();
            MdPartyReset();
            sys->subMode = 2;
            break;
       
        case 2:
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(--sys->selPack == -1) {
                        sys->selPack = 4;
                    }
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->selPack = ++sys->selPack % 5;
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_PACK0+sys->selPack, MDMESS_WIN_LIST_END);
                for(i=0; i<5; i++) {
                    if(i == sys->selPack) {
                        Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_PACK0+i]);
                    } else {
                        Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_PACK0+i]);
                    }
                }
                sys->cursorMode = MDPARTY_CURSOR_PACK_SEL;
                MdPartyCursorDirLR(sys->cursorDir);
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_PACK_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyCapPackCancel();
                MdPartyReset();
                MdPartyCapTurnReset();
                MdPartyReset();
                star->motNext = MDPARTY_STAR_MOT_TALK;
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_TURN_SEL, MESS_PARTYMODE_SYS_TURN_10+sys->selTurn, MDMESS_WIN_LIST_END);
                MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                sys->cursorMode = MDPARTY_CURSOR_TURN_SEL;
                MdPartyCursorIn();
                MdPartyReset();
                sys->subMode = 2;
                sys->flowMode = MDPARTY_FLOW_TURN_SEL;
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_PACK_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyCapPackEnd();
                MdPartyReset();
                sys->subMode = 0;
                sys->flowMode = MDPARTY_FLOW_BONUS_SEL;
            }
            break;
    }
}

void MdPartyFlowBonusSel(void)
{
    MDMESS_WORK *mess; //r31
    MDPARTY_STAR *star; //r30
    MDMODEL_WORK *model; //r29
    
    MDCAMERA_WORK *camera; //sp+0x14
    MDSPRITE_WORK *sprite; //sp+0x10
    MDPARTY_PLAYERSPR *playerSpr; //sp+0xC
    MDPARTY_CHR *chr; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    chr = &sys->chr[0];
    
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            sys->selBonus = 0;
            Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_CAP_BONUS], sys->selBonus);
            sys->subMode = 1;
            break;
        
        case 1:
            MdPartyCapBonusStart();
            MdPartyReset();
            star->motNext = MDPARTY_STAR_MOT_TALK;
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BONUS_ON+sys->selBonus, MDMESS_WIN_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            sys->cursorMode = MDPARTY_CURSOR_BONUS_SEL;
            MdPartyCursorIn();
            MdPartyReset();
            sys->subMode = 2;
            break;
       
        case 2:
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(--sys->selBonus == -1) {
                        sys->selBonus = 1;
                    }
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->selBonus = ++sys->selBonus % 2;
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BONUS_ON+sys->selBonus, MDMESS_WIN_LIST_END);
                MdPartyCapBonusUpdate();
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_BONUS_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyCapBonusCancel();
                MdPartyReset();
                MdPartyCapPackReset();
                MdPartyReset();
                star->motNext = MDPARTY_STAR_MOT_TALK;
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_PACK0+sys->selPack, MDMESS_WIN_LIST_END);
                MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                sys->cursorMode = MDPARTY_CURSOR_PACK_SEL;
                MdPartyCursorIn();
                MdPartyReset();
                sys->subMode = 2;
                sys->flowMode = MDPARTY_FLOW_PACK_SEL;
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->cursorMode = MDPARTY_CURSOR_BONUS_SEL;
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyCapBonusEnd();
                MdPartyReset();
                sys->subMode = 0;
                sys->flowMode = MDPARTY_FLOW_HANDICAP;
            }
            break;
    }
}

void MdPartyFlowHandicap(void)
{
    MDMESS_WORK *mess; //r31
    MDPARTY_STAR *star; //r30
    int i; //r29
    int choice; //r28
    MDSPRITE_WORK *sprite; //r27
    
    MDCAMERA_WORK *camera; //sp+0x14
    MDMODEL_WORK *model; //sp+0x10
    MDPARTY_PLAYERSPR *playerSpr; //sp+0xC
    MDPARTY_CHR *chr; //sp+0x8
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    playerSpr = &sys->playerSpr[0];
    chr = &sys->chr[0];
    
    switch(sys->subMode) {
        case 0:
            MdPartyReset();
            sys->selHandicapPlayer = 0;
            for(i=0; i<4; i++) {
                sys->selHandicap[i] = 0;
                HuSprBankSet(sprite->grpId[MDSPR_GRP_HANDICAP], (int)i, sys->selHandicap[i]);
            }
            sys->subMode = 1;
            break;
        
        case 1:
            if(sys->gameType == 0) {
                MdPartyChrHandicapStart();
                MdPartyReset();
            } else if(sys->gameType == 1) {
                MdPartyChrTeamHandicapStart();
                MdPartyReset();
            }
            star->motNext = MDPARTY_STAR_MOT_TALK;
            if(sys->gameType == 0) {
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_HANDICAP, (u32)sys->chrSel[sys->selHandicapPlayer], MDMESS_WIN_LIST_END);
            } else if(sys->gameType == 1) {
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_HANDICAP, MESS_TEAMNAME_START+sys->teamNo[sys->selHandicapPlayer], MDMESS_WIN_LIST_END);
            }
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
            if(sys->gameType == 0) {
                sys->cursorMode = MDPARTY_CURSOR_HANDICAP;
            } else if(sys->gameType == 1) {
                sys->cursorMode = MDPARTY_CURSOR_HANDICAP_TEAM;
            }
            MdPartyCursorIn();
            MdPartyReset();
            MdPartyHandicapStart();
            MdPartyReset();
            sys->subMode = 2;
            break;
        
        case 2:
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(sys->gameType == 0) {
                        if(--sys->selHandicapPlayer == -1) {
                            sys->selHandicapPlayer = 3;
                        }
                    } else if(sys->gameType == 1) {
                        if(--sys->selHandicapPlayer == -1) {
                            sys->selHandicapPlayer = 1;
                        }
                    }
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    if(sys->gameType == 0) {
                        sys->selHandicapPlayer = ++sys->selHandicapPlayer%4;
                    } else if(sys->gameType == 1) {
                        sys->selHandicapPlayer = ++sys->selHandicapPlayer%2;
                    }
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                if(sys->gameType == 0) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_HANDICAP, (u32)sys->chrSel[sys->selHandicapPlayer], MDMESS_WIN_LIST_END);
                } else if(sys->gameType == 1) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_HANDICAP, MESS_TEAMNAME_START+sys->teamNo[sys->selHandicapPlayer], MDMESS_WIN_LIST_END);
                }
                if(sys->gameType == 0) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP;
                } else if(sys->gameType == 1) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP_TEAM;
                }
                MdPartyCursorDirLR(sys->cursorDir);
                MdPartyReset();
            } else if((sys->pad->DStkDown[0] & PAD_BUTTON_UP) || (sys->pad->DStkDown[0] & PAD_BUTTON_DOWN)) {
                if(sys->pad->DStkDown[0] & PAD_BUTTON_UP) {
                    sys->selHandicap[sys->selHandicapPlayer] = (++sys->selHandicap[sys->selHandicapPlayer])%10;
                    sys->cursorDir = 0;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_DOWN) {
                    if(--sys->selHandicap[sys->selHandicapPlayer] == -1) {
                        sys->selHandicap[sys->selHandicapPlayer] = 9;
                    }
                    sys->cursorDir = 1;
                }
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_MOVE);
                HuSprBankSet(sprite->grpId[MDSPR_GRP_HANDICAP], (int)sys->selHandicapPlayer, sys->selHandicap[sys->selHandicapPlayer]);
                if(sys->gameType == 0) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP;
                } else if(sys->gameType == 1) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP_TEAM;
                }
                MdPartyCursorDirUD(sys->cursorDir);
                MdPartyReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CANCEL);
                star->motNext = MDPARTY_STAR_MOT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                MdPartyHandicapCancel();
                MdPartyReset();
                if(sys->gameType == 0) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP;
                } else if(sys->gameType == 1) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP_TEAM;
                }
                MdPartyCursorOut();
                MdPartyReset();
                if(sys->gameType == 0) {
                    MdPartyChrHandicapCancel();
                    MdPartyReset();
                } else if(sys->gameType == 1) {
                    MdPartyChrTeamHandicapCancel();
                    MdPartyReset();
                }
                MdPartyCapBonusReset();
                MdPartyReset();
                star->motNext = MDPARTY_STAR_MOT_TALK;
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BONUS_ON+sys->selBonus, MDMESS_WIN_LIST_END);
                MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                sys->cursorMode = MDPARTY_CURSOR_BONUS_SEL;
                MdPartyCursorIn();
                MdPartyReset();
                sys->subMode = 2;
                sys->flowMode = MDPARTY_FLOW_BONUS_SEL;
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdPartyAudExec(MDPARTY_AUD_SE_CMN_CONFIRM);
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                if(sys->gameType == 0) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP;
                } else if(sys->gameType == 1) {
                    sys->cursorMode = MDPARTY_CURSOR_HANDICAP_TEAM;
                }
                MdPartyCursorOut();
                MdPartyReset();
                MdPartyDispSetupStart();
                MdPartyReset();
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_PARTYMODE_SYS_CONFIRM, MDMESS_WIN_LIST_END);
                MdPartyAudExec(MDPARTY_AUD_SE_STAR_CONFIRM);
                while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                    HuPrcVSleep();
                }
                choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                if(choice == -1 || choice == 1) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_PARTYMODE_SYS_RESET, MDMESS_WIN_LIST_END);
                    while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                        HuPrcVSleep();
                    }
                    choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                    if(choice == -1 || choice == 1) {
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        MdPartyDispSetupCancel();
                        MdPartyReset();
                        if(sys->gameType == 0) {
                            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_HANDICAP, (u32)sys->chrSel[sys->selHandicapPlayer], MDMESS_WIN_LIST_END);
                        } else if(sys->gameType == 1) {
                            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_HANDICAP, MESS_TEAMNAME_START+sys->teamNo[sys->selHandicapPlayer], MDMESS_WIN_LIST_END);
                        }
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        if(sys->gameType == 0) {
                            sys->cursorMode = MDPARTY_CURSOR_HANDICAP;
                        } else if(sys->gameType == 1) {
                            sys->cursorMode = MDPARTY_CURSOR_HANDICAP_TEAM;
                        }
                        MdPartyCursorIn();
                        MdPartyReset();
                    } else if(choice == 0) {
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        star->mode = MDPARTY_STAR_CENTER_START;
                        star->motNext = MDPARTY_STAR_MOT_IDLE;
                        MdPartyDispSetupReset();
                        MdPartyReset();
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_START, MDMESS_WIN_LIST_END);
                        while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                            HuPrcVSleep();
                        }
                        MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                        star->motNext = MDPARTY_STAR_MOT_IDLE;
                        MdPartyGameTypeStart();
                        MdPartyReset();
                        MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BROYAL+sys->gameType, MDMESS_WIN_LIST_END);
                        MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_WIN_LIST_END);
                        sys->cursorMode = MDPARTY_CURSOR_START;
                        MdPartyCursorIn();
                        MdPartyReset();
                        sys->subMode = 2;
                        sys->flowMode = MDPARTY_FLOW_START;
                    }
                } else if(choice == 0) {
                    MdPartyReset();
                    MdPartySaveInit();
                    sys->loadProcReady = TRUE;
                    sys->loadDir = MBDataDir[GwSystem.boardNo];
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_PARTYMODE_SYS_BOARD_START, MDMESS_WIN_LIST_END);
                    while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                        HuPrcVSleep();
                    }
                    MdPartyDispMapEnter();
                    WipeColorSet(255, 255, 255);
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 55);
                    MdPartyAudExec(MDPARTY_AUD_SE_BOARD_ENTER2);
                    MdPartyAudExec(MDPARTY_AUD_MUS_STOP);
                    HuPrcVSleep();
                    while(WipeCheckEnd()) {
                        MdProcCall(MDPARTY_PROC_MAP_ENTER, 0);
                        HuPrcVSleep();
                    }
                    while(sys->loadState != MD_LOAD_STATE_DONE) {
                        HuPrcVSleep();
                    }
                    omOvlHisChg(0, sys->ovlHis->ovl, 1, sys->ovlHis->stat);
                    sys->ovlCallMode = MD_OVLCALL_CALL;
                    sys->nextDll = MBDll[GwSystem.boardNo];
                    sys->skipWipeF = TRUE;
                    sys->exitF = TRUE;
                }
            }
            break;
    }
}

BOOL MdPartyProcReset(int param)
{
    sys->dirtyF = FALSE;
    sys->teamInitF = FALSE;
    sys->winInitF = FALSE;
    sys->cursorInitF = FALSE;
    sys->handicapInitF = FALSE;
    sys->nameInitF = FALSE;
    sys->unk3E8 = FALSE;
    sys->procTime = 0.0f;
    sys->procOfsTime = 0.0f;
    sys->cameraTime = 0.0f;
    sys->cameraTotalTime = 0.0f;
    sys->sprTime = 0.0f;
    sys->nameTime = 0.0f;
    sys->procT = 0.0f;
    sys->procOfsT = 0.0f;
    sys->cameraT = 0.0f;
    sys->cameraTotalT = 0.0f;
    sys->sprT = 0.0f;
    sys->nameT = 0.0f;
    return FALSE;
}

typedef struct MdPartyLerpData_s {
    HuVecF hvrA;
    HuVecF centerA;
    HuVecF hvrB;
    HuVecF centerB;
    int type;
    float step;
} MDPARTY_LERP_DATA;

BOOL MdPartyProcCamera(int param)
{
    MDPARTY_LERP_DATA *lerpP;
    MDCAMERA_WORK *cameraWork;
    MDCAMERA *camera;

    static MDPARTY_LERP_DATA lerpData[1] = {
        {
            { 90, 0, 1920 },
            { 0, 0, 0 },
            { 90, 0, 500 },
            { 0, 150, 0 },
            MDMATH_OSC_90,
            60
        }
    };
    
    cameraWork = sys->camera;
    camera = &cameraWork->camera[MDCAMERA_MAIN];
    lerpP = &lerpData[param];
    MdMathOscillate(lerpP->type, &sys->cameraTime, &sys->cameraT, lerpP->step);
    MdMathOscillate(MDMATH_OSC_180, &sys->cameraTotalTime, &sys->cameraTotalT, lerpP->step);
    MdMathLerp(&camera->hvr.x, lerpP->hvrA.x, lerpP->hvrB.x, sys->cameraT);
    MdMathLerp(&camera->hvr.y, lerpP->hvrA.y, lerpP->hvrB.y, sys->cameraT);
    MdMathLerp(&camera->hvr.z, lerpP->hvrA.z, lerpP->hvrB.z, sys->cameraT);
    MdMathLerp(&camera->center.x, lerpP->centerA.x, lerpP->centerB.x, sys->cameraT);
    MdMathLerp(&camera->center.y, lerpP->centerA.y, lerpP->centerB.y, sys->cameraT);
    MdMathLerp(&camera->center.z, lerpP->centerA.z, lerpP->centerB.z, sys->cameraT);
    if(sys->cameraTotalTime >= 1.0f) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcCursor(int param)
{
    HuVec2f *sprPos; //r31
    HuVec2f *sprScale; //30
    HUSPRGRPID grpId; //r29
    MDSPRITE_WORK *sprite; //r28
    MDCAMERA_WORK *camera; //r27
    MDMODEL_WORK *model; //r25
    
    static HuVecF pos[8]; //bss+0x24C
    
    HuVecF pos2D; //sp+0x14
    HuVecF pos3D; //sp+0x8
    
    camera = sys->camera;
    model = sys->model;
    sprite = sys->sprite;
    grpId = sprite->grpId[MDSPR_GRP_CURSOR_UDLR];
    if(!sys->cursorInitF) {
        if(sys->cursorMode == MDPARTY_CURSOR_START) {
            pos3D.x = -150;
            pos3D.y = 250;
            pos3D.z = 0;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-88;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos3D.x = 150;
            pos3D.y = 250;
            pos3D.z = 0;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+88;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_PLAYERSEL) {
            pos3D.x = 0;
            pos3D.y = 176.8f;
            pos3D.z = -580.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-192;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+192;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL) {
            pos3D = *model->posP[sys->chrSel[sys->comPlayerNo[sys->selComNo]]+MDMODEL_CAP_CHR_00];
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos2D.y -= 44.0f;
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-46;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = (pos2D.x+46)-3;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
            pos3D = *model->posP[sys->chrSel[sys->teamComPlayerNo[sys->selComNo]]+MDMODEL_CAP_CHR_00];
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos2D.y -= 44.0f;
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-46;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = (pos2D.x+46)-3;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_TEAM_SEL) {
            pos3D.x = -460.0f;
            pos3D.y = -80.0f;
            pos3D.z = -600.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-64;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos3D.x = 460.0f;
            pos3D.y = -80.0f;
            pos3D.z = -600.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+64;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_MAP_SEL) {
            pos3D.x = 0.0f;
            pos3D.y = 180.0f;
            pos3D.z = -200.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-192;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+192;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_TURN_SEL) {
            pos3D.x = 0.0f;
            pos3D.y = 145.0f;
            pos3D.z = 400.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-96;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+96;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_PACK_SEL) {
            pos3D.x = 0.0f;
            pos3D.y = 145.0f;
            pos3D.z = 400.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-96;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+96;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_BONUS_SEL) {
            pos3D.x = 0.0f;
            pos3D.y = 145.0f;
            pos3D.z = 400.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-96;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+96;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_HANDICAP) {
            static HuVecF handicapPos[4] = {
                { -360, 100, -600 },
                { -120, 100, -600 },
                { 120, 100, -600 },
                { 360, 100, -600 }
            };
            
            pos3D.x = -360.0f;
            pos3D.y = 100.0f;
            pos3D.z = -600.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-64;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos3D.x = 360.0f;
            pos3D.y = 100.0f;
            pos3D.z = -600.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+64;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
            pos3D = handicapPos[sys->selHandicapPlayer];
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[4].x = pos2D.x;
            pos[4].y = -64.0f;
            pos[4].z = 0;
            pos[5].x = pos2D.x;
            pos[5].y = pos2D.y-72;
            pos[5].z = 0;
            pos3D = handicapPos[sys->selHandicapPlayer];
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[6].x = pos2D.x;
            pos[6].y = 544.0f;
            pos[6].z = 0;
            pos[7].x = pos2D.x;
            pos[7].y = pos2D.y+64;
            pos[7].z = 0;
        } else if(sys->cursorMode == MDPARTY_CURSOR_HANDICAP_TEAM) {
            static HuVecF handicapPos[2] = {
                { -340, 100, -600 },
                { 340, 100, -600 }
            };
            
            pos3D.x = -360.0f;
            pos3D.y = 100.0f;
            pos3D.z = -600.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[0].x = -64.0f;
            pos[0].y = pos2D.y;
            pos[0].z = 0;
            pos[1].x = pos2D.x-96;
            pos[1].y = pos2D.y;
            pos[1].z = 0;
            pos3D.x = 360.0f;
            pos3D.y = 100.0f;
            pos3D.z = -600.0f;
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[2].x = 640.0f;
            pos[2].y = pos2D.y;
            pos[2].z = 0;
            pos[3].x = pos2D.x+96;
            pos[3].y = pos2D.y;
            pos[3].z = 0;
            pos3D = handicapPos[sys->selHandicapPlayer];
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[4].x = pos2D.x;
            pos[4].y = -64.0f;
            pos[4].z = 0;
            pos[5].x = pos2D.x;
            pos[5].y = pos2D.y-72;
            pos[5].z = 0;
            pos3D = handicapPos[sys->selHandicapPlayer];
            Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
            pos[6].x = pos2D.x;
            pos[6].y = 544.0f;
            pos[6].z = 0;
            pos[7].x = pos2D.x;
            pos[7].y = pos2D.y+64;
            pos[7].z = 0;
        } else {
            pos[0].x = -64;
            pos[0].y = 240;
            pos[0].z = 0;
            pos[1].x = 128.0f;
            pos[1].y = 240.0f;
            pos[1].z = 0;
            pos[2].x = 640;
            pos[2].y = 240;
            pos[2].z = 0;
            pos[3].x = 448.0f;
            pos[3].y = 240.0f;
            pos[3].z = 0;
        }
        if(param == 0) {
            HuSprGrpPosSet(grpId, 0, 0);
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
            sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
            sprPos->x = pos[0].x;
            sprPos->y = pos[0].y;
            if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
                sprScale->x = sprScale->y = 0.7f;
            } else {
                sprScale->x = sprScale->y = 1.0f;
            }
            HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
            HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
            sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
            sprPos->x = pos[2].x;
            sprPos->y = pos[2].y;
            if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
                sprScale->x = sprScale->y = 0.7f;
            } else {
                sprScale->x = sprScale->y = 1.0f;
            }
            HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
            HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
            if(sys->cursorMode == MDPARTY_CURSOR_HANDICAP || sys->cursorMode == MDPARTY_CURSOR_HANDICAP_TEAM) {
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
                sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
                sprPos->x = pos[4].x;
                sprPos->y = pos[4].y;
                sprScale->x = sprScale->y = 1.0f;
                HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
                HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U));
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
                sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
                sprPos->x = pos[6].x;
                sprPos->y = pos[6].y;
                sprScale->x = sprScale->y = 1.0f;
                HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
                HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D));
            }
            if(sys->cursorMode == MDPARTY_CURSOR_TURN_SEL) {
                if(sys->selTurn == 0) {
                    HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                    HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                } else if(sys->selTurn == 8) {
                    HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                    HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                } else {
                    HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                    HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                }
            }
        } else if(param != 1) {
            if(param == 2) {
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                sprPos->x = pos[1].x;
                sprPos->y = pos[1].y;
                if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
                    sprScale->x = sprScale->y = 0.7f;
                } else {
                    sprScale->x = sprScale->y = 1.0f;
                }
                HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R), 0.0f);
                HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                if(sys->cursorMode == MDPARTY_CURSOR_HANDICAP || sys->cursorMode == MDPARTY_CURSOR_HANDICAP_TEAM) {
                    sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
                    sprPos->x = pos[5].x;
                    sprPos->y = pos[5].y;
                    sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
                    sprPos->x = pos[7].x;
                    sprPos->y = pos[7].y;
                }
                if(sys->cursorMode == MDPARTY_CURSOR_TURN_SEL) {
                    if(sys->selTurn == 0) {
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                    } else if(sys->selTurn == 8) {
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                    } else {
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                    }
                }
            } else if(param == 3) {
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                sprPos->x = pos[3].x;
                sprPos->y = pos[3].y;
                if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
                    sprScale->x = sprScale->y = 0.7f;
                } else {
                    sprScale->x = sprScale->y = 1.0f;
                }
                HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L), 0.0f);
                HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                if(sys->cursorMode == MDPARTY_CURSOR_HANDICAP || sys->cursorMode == MDPARTY_CURSOR_HANDICAP_TEAM) {
                    sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
                    sprPos->x = pos[5].x;
                    sprPos->y = pos[5].y;
                    sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
                    sprPos->x = pos[7].x;
                    sprPos->y = pos[7].y;
                }
                if(sys->cursorMode == MDPARTY_CURSOR_TURN_SEL) {
                    if(sys->selTurn == 0) {
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                    } else if(sys->selTurn == 8) {
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                    } else {
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
                        HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
                        HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
                    }
                }
            } else if(param == 4) {
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U));
                sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
                sprPos->x = pos[5].x;
                sprPos->y = pos[5].y;
                sprScale->x = sprScale->y = 1;
                HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U), 0.0f);
                HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U));
            } else if(param == 5) {
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D));
                sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
                sprPos->x = pos[7].x;
                sprPos->y = pos[7].y;
                sprScale->x = sprScale->y = 1;
                HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D), 0.0f);
                HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D));
            }
        }
        sys->cursorInitF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 15.0f);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
        MdMathLerp(&sprPos->x, pos[0].x, pos[1].x, sys->sprT);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
        MdMathLerp(&sprPos->x, pos[2].x, pos[3].x, sys->sprT);
        if(sys->cursorMode == MDPARTY_CURSOR_HANDICAP || sys->cursorMode == MDPARTY_CURSOR_HANDICAP_TEAM) {
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
            MdMathLerp(&sprPos->y, pos[4].y, pos[5].y, sys->sprT);
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
            MdMathLerp(&sprPos->y, pos[6].y, pos[7].y, sys->sprT);
        }
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 15.0f);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
        MdMathLerp(&sprPos->x, pos[1].x, pos[0].x, sys->sprT);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
        MdMathLerp(&sprPos->x, pos[3].x, pos[2].x, sys->sprT);
        if(sys->cursorMode == MDPARTY_CURSOR_HANDICAP || sys->cursorMode == MDPARTY_CURSOR_HANDICAP_TEAM) {
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
            MdMathLerp(&sprPos->y, pos[5].y, pos[4].y, sys->sprT);
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
            MdMathLerp(&sprPos->y, pos[7].y, pos[6].y, sys->sprT);
        }
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_180, &sys->sprTime, &sys->sprT, 15.0f);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_R));
        MdMathLerp(&sprPos->x, pos[1].x, pos[1].x-24.0f, sys->sprT);
        if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
            MdMathLerp(&sprScale->x, 0.7f, 1.0f, sys->sprT);
            MdMathLerp(&sprScale->y, 0.7f, 1.0f, sys->sprT);
        } else {
            MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
            MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        }
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
        MdMathLerp(&sprPos->x, pos[1].x, pos[1].x-24.0f, sys->sprT);
        if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
            MdMathLerp(&sprScale->x, 0.7f, 1.0f, sys->sprT);
            MdMathLerp(&sprScale->y, 0.7f, 1.0f, sys->sprT);
        } else {
            MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
            MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        }
        HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R), sys->sprT);
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_180, &sys->sprTime, &sys->sprT, 15.0f);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_L));
        MdMathLerp(&sprPos->x, pos[3].x, pos[3].x+24.0f, sys->sprT);
        if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
            MdMathLerp(&sprScale->x, 0.7f, 1.0f, sys->sprT);
            MdMathLerp(&sprScale->y, 0.7f, 1.0f, sys->sprT);
        } else {
            MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
            MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        }
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
        MdMathLerp(&sprPos->x, pos[3].x, pos[3].x+24.0f, sys->sprT);
        if(sys->cursorMode == MDPARTY_CURSOR_CHR_DIF_SEL || sys->cursorMode == MDPARTY_CURSOR_TEAM_DIF_SEL) {
            MdMathLerp(&sprScale->x, 0.7f, 1.0f, sys->sprT);
            MdMathLerp(&sprScale->y, 0.7f, 1.0f, sys->sprT);
        } else {
            MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
            MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        }
        HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L), sys->sprT);
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_180, &sys->sprTime, &sys->sprT, 15.0f);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_U));
        MdMathLerp(&sprPos->y, pos[5].y, pos[5].y-24.0f, sys->sprT);
        MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
        MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U));
        MdMathLerp(&sprPos->y, pos[5].y, pos[5].y-24.0f, sys->sprT);
        MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
        MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U), sys->sprT);
    } else if(param == 5) {
        MdMathOscillate(MDMATH_OSC_180, &sys->sprTime, &sys->sprT, 15.0f);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_UDLR_D));
        MdMathLerp(&sprPos->y, pos[7].y, pos[7].y+24.0f, sys->sprT);
        MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
        MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D));
        sprScale = MdSpriteScaleGet(sprite, MDSPR_GRP_CURSOR_UDLR, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D));
        MdMathLerp(&sprPos->y, pos[7].y, pos[7].y+24.0f, sys->sprT);
        MdMathLerp(&sprScale->x, 1.0f, 1.5f, sys->sprT);
        MdMathLerp(&sprScale->y, 1.0f, 1.5f, sys->sprT);
        HuSprTPLvlSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D), sys->sprT);
    }
    if(sys->sprTime >= 1.0f) {
        if(param == 1) {
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_UDLR, FALSE);
        } else if(param == 2) {
            HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_R));
        } else if(param == 3) {
            HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_L));
        } else if(param == 4) {
            HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_U));
        } else if(param == 5) {
            HuSprDispOff(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_UDLR, MDSPR_CURSOR_GLOW_UDLR_D));
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcWin(int param)
{
    MDSPRITE_WORK *sprite; //r31
    HuVec2f *bgScale; //r30
    HuVec2f *topPos; //r29
    HuVec2f *bottomPos; //r28
    HuVec2f *topScale; //r27
    HuVec2f *bottomScale; //r26
    
    HuVec2f *bgPos; //sp+0xC
    HUSPRGRPID grpId; //sp+0x8
    
    static int updateMode; //bss+0x248
    static float openY; //bss+0x244
    static float closeY; //bss+0x240
    static float bgScaleY; //bss+0x23C
    
    sprite = sys->sprite;
    grpId = sprite->grpId[MDSPR_GRP_WIN];
    topPos = MdSpritePosGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_TOP));
    bottomPos = MdSpritePosGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_BOTTOM));
    bgPos = MdSpritePosGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_BG));
    topScale = MdSpriteScaleGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_TOP));
    bottomScale = MdSpriteScaleGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_BOTTOM));
    bgScale = MdSpriteScaleGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_BG));
    if(!sys->winInitF) {
        updateMode = 0;
        openY = topPos->y;
        closeY = bottomPos->y;
        bgScaleY = bgScale->y;
        if(param == FALSE) {
            topPos->y = 0;
            bottomPos->y = 0;
            topScale->x = 0;
            bottomScale->x = 0;
            bgScale->x = 0;
            bgScale->y = 0;
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_WIN, TRUE);
        }
        sys->winInitF = TRUE;
    }
    if(param == FALSE) {
        if(updateMode == 0) {
            MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 12);
            MdMathLerp(&topPos->y, 0, -2, sys->sprT);
            MdMathLerp(&bottomPos->y, 0, 2, sys->sprT);
            MdMathLerp(&topScale->x, 0, 72, sys->sprT);
            MdMathLerp(&bottomScale->x, 0, 72, sys->sprT);
            MdMathLerp(&bgScale->x, 0, 1, sys->sprT);
            if(sys->sprTime >= 1) {
                sys->sprTime = 0;
                updateMode = 1;
            }
        } else if(updateMode == 1) {
            MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 15);
            MdMathLerp(&topPos->y, -2, -28, sys->sprT);
            MdMathLerp(&bottomPos->y, 2, 32, sys->sprT);
            MdMathLerp(&bgScale->y, 0, 8, sys->sprT);
            if(sys->sprTime >= 1) {
                updateMode = 2;
            }
        }
    } else if(param == TRUE) {
        if(updateMode == 0) {
            MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 15);
            MdMathLerp(&topPos->y, openY, -2, sys->sprT);
            MdMathLerp(&bottomPos->y, closeY, 2, sys->sprT);
            MdMathLerp(&bgScale->y, bgScaleY, 0, sys->sprT);
            if(sys->sprTime >= 1) {
                sys->sprTime = 0;
                updateMode = 1;
            }
            
        } else if(updateMode == 1) {
            MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 12);
            MdMathLerp(&topPos->y, -2, 0, sys->sprT);
            MdMathLerp(&bottomPos->y, 2, 0, sys->sprT);
            MdMathLerp(&topScale->x, 72, 0, sys->sprT);
            MdMathLerp(&bottomScale->x, 72, 0, sys->sprT);
            MdMathLerp(&bgScale->x, 1, 0, sys->sprT);
            if(sys->sprTime >= 1) {
                updateMode = 2;
            }
        }
    }
    if(updateMode == 2) {
        if(param == TRUE) {
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_WIN, FALSE);
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcGameType(int param)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *posP; //r29
    HuVecF *scaleP; //r28
    
    MDCAMERA_WORK *camera; //sp+0x10
    MDSPRITE_WORK *sprite; //sp+0xC
    HuVecF *rotP; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    if(!sys->dirtyF) {
        if(param == 0) {
            model->posP[MDMODEL_CAP_BROYAL]->x = -150.0f;
            model->posP[MDMODEL_CAP_BROYAL]->y = 250.0f;
            model->posP[MDMODEL_CAP_BROYAL]->z = 0.0f;
            model->posP[MDMODEL_CAP_TMATCH]->x = 150.0f;
            model->posP[MDMODEL_CAP_TMATCH]->y = 250.0f;
            model->posP[MDMODEL_CAP_TMATCH]->z = 0.0f;
            model->scaleP[MDMODEL_CAP_BROYAL]->x = 0.0f;
            model->scaleP[MDMODEL_CAP_BROYAL]->y = 0.0f;
            model->scaleP[MDMODEL_CAP_BROYAL]->z = 0.0f;
            model->scaleP[MDMODEL_CAP_TMATCH]->x = 0.0f;
            model->scaleP[MDMODEL_CAP_TMATCH]->y = 0.0f;
            model->scaleP[MDMODEL_CAP_TMATCH]->z = 0.0f;
            for(i=0; i<2; i++) {
                Hu3DModelDispOn(model->mdlId[i+MDMODEL_CAP_BROYAL]);
            }
            MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
        } else if(param == 3) {
            MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
        } else if(param == 4) {
            MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 15);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15);
        for(i=0; i<2; i++) {
            scaleP = model->scaleP[i+MDMODEL_CAP_BROYAL];
            if(i == sys->gameType) {
                MdMathLerpScaleOfs(scaleP, 0, 1.25f, sys->procT, sys->procOfsT);
            } else {
                MdMathLerpScale(scaleP, 0, 0.75f, sys->procT);
            }
        }
        for(i=0; i<2; i++) {
            MdPartyModelBBoardSet(i+MDMODEL_CAP_BROYAL, MDCAMERA_MAIN);
        }
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 15);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15);
        for(i=0; i<2; i++) {
            rotP = model->rotP[i+MDMODEL_CAP_BROYAL];
            scaleP = model->scaleP[i+MDMODEL_CAP_BROYAL];
            if(i != sys->gameType) {
                MdMathLerpScale(scaleP, 1.25f, 0.75f, sys->procT);
            } else {
                MdMathLerpScaleOfs(scaleP, 0.75f, 1.25f, sys->procT, sys->procOfsT*1.5f);
            }
        }
        sys->cursorMode = MDPARTY_CURSOR_START;
        MdProcCall(MDPARTY_PROC_CURSOR, sys->cursorDir+2);
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15);
        for(i=0; i<2; i++) {
            posP = model->posP[i+MDMODEL_CAP_BROYAL];
            scaleP = model->scaleP[i+MDMODEL_CAP_BROYAL];
            if(i == sys->gameType) {
                MdMathLerpScale(scaleP, 1.25f, 0.8f, sys->procT);
                if(i == 0) {
                    MdMathLerp(&posP->x, -150.0f, -525.0f, sys->procT);
                    MdMathLerp(&posP->y, 250.0f, 365.0f, sys->procT);
                    MdMathLerpOfs(&posP->z, 0.0f, -50.0f, sys->procT, sys->procOfsT*-8.0f);
                } else if(i == 1) {
                    MdMathLerp(&posP->x, 150.0f, -525.0f, sys->procT);
                    MdMathLerp(&posP->y, 250.0f, 365.0f, sys->procT);
                    MdMathLerpOfs(&posP->z, 0.0f, -50.0f, sys->procT, sys->procOfsT*-8.0f);
                }
            } else {
                MdMathLerpScale(scaleP, 0.75f, 0.0f, sys->procT);
            }
        }
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 15);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15);
        for(i=0; i<2; i++) {
            posP = model->posP[i+MDMODEL_CAP_BROYAL];
            scaleP = model->scaleP[i+MDMODEL_CAP_BROYAL];
            if(i == sys->gameType) {
                MdMathLerpScale(scaleP, 0.8f, 1.25f, sys->procT);
                if(i == 0) {
                    MdMathLerp(&posP->x, -525.0f, -150.0f, sys->procT);
                    MdMathLerp(&posP->y, 365.0f, 250.0f, sys->procT);
                    MdMathLerpOfs(&posP->z, -50.0f, 0.0f, sys->procT, sys->procOfsT*8.0f);
                } else if(i == 1) {
                    MdMathLerp(&posP->x, -525.0f, 150.0f, sys->procT);
                    MdMathLerp(&posP->y, 365.0f, 250.0f, sys->procT);
                    MdMathLerpOfs(&posP->z, -50.0f, 0.0f, sys->procT, sys->procOfsT*8.0f);
                }
            } else {
                MdMathLerpScale(scaleP, 0.0f, 0.75f, sys->procT);
            }
        }
    }
    for(i=0; i<2; i++) {
        MdPartyModelBBoardSet(i+MDMODEL_CAP_BROYAL, MDCAMERA_MAIN);
    }
    if(sys->procTime >= 1.0f) {
        return TRUE;
    } else {
        return FALSE;
    }
}

//Required to get proper bss ordering with this variable
extern s8 capPlayerInitF;

BOOL MdPartyProcCapPlayer(int param)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    MDCAMERA_WORK *camera; //r28
    
    HuVecF *posP[4][2]; //sp+0x48
    HuVecF *rotP[4][2]; //sp+0x28
    HuVecF *scaleP[4][2]; //sp+0x8
    
    static HuVecF centerPos = { 0, 0, 0 }; //data+0x638
    static HuVecF playerPos[4] = { }; //data+0x644
    
    static HuVecF centerPos3D; //bss+0x230
    static HuVecF playerPos3D[4]; //bss+0x200
    static s8 playerType[4][2]; //bss+0x1F8
    static float radius; //bss+0x1F4
    static float angle; //bss+0x1F0
    static HuVecF oldPlayerPos3D[4]; //bss+0x1C0
    
    model = sys->model;
    
    camera = sys->camera;
    if(!capPlayerInitF) {
        centerPos.x = 288;
        centerPos.y = 176.64f;
        centerPos.z = 4000.0f;
        for(i=0; i<4; i++) {
            playerPos[i].x = 168.0f+(i*80.0f);
            playerPos[i].y = centerPos.y;
            playerPos[i].z = 2500;
        }
        capPlayerInitF = TRUE;
    }
    for(i=0; i<4; i++) {
        posP[i][0] = model->posP[i+MDMODEL_CAP_MAN_1P];
        posP[i][1] = model->posP[i+MDMODEL_CAP_COM_1P];
        
        rotP[i][0] = model->rotP[i+MDMODEL_CAP_MAN_1P];
        rotP[i][1] = model->rotP[i+MDMODEL_CAP_COM_1P];
        
        scaleP[i][0] = model->scaleP[i+MDMODEL_CAP_MAN_1P];
        scaleP[i][1] = model->scaleP[i+MDMODEL_CAP_COM_1P];
    }
    if(!sys->dirtyF) {
        for(i=0; i<4; i++) {
            if(i < sys->playerNum+1) {
                playerType[i][0] = 0;
            } else {
                playerType[i][0] = 1;
            }
            if(i < sys->prevPlayerNum+1) {
                playerType[i][1] = 0;
            } else {
                playerType[i][1] = 1;
            }
        }
        if(param == 0) {
            Hu3D2Dto3D(&centerPos, camera->camera[MDCAMERA_MAIN].bit, &centerPos3D);
            for(i=0; i<4; i++) {
                Hu3D2Dto3D(&playerPos[i], camera->camera[MDCAMERA_MAIN].bit, &playerPos3D[i]);
            }
            for(i=0; i<4; i++) {
                posP[i][playerType[i][0]]->x = centerPos3D.x;
                posP[i][playerType[i][0]]->y = centerPos3D.y;
                posP[i][playerType[i][0]]->z = centerPos3D.z;
                rotP[i][playerType[i][0]]->x = 0;
                rotP[i][playerType[i][0]]->y = 0;
                rotP[i][playerType[i][0]]->z = 0;
                scaleP[i][playerType[i][0]]->x = 0;
                scaleP[i][playerType[i][0]]->y = 0;
                scaleP[i][playerType[i][0]]->z = 0;
                if(playerType[i][0] == 0) {
                    Hu3DModelDispOn(model->mdlId[i+MDMODEL_CAP_MAN_1P]);
                    Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_COM_1P]);
                } else if(playerType[i][0] == 1) {
                    Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_MAN_1P]);
                    Hu3DModelDispOn(model->mdlId[i+MDMODEL_CAP_COM_1P]);
                }
            }
        } else if(param != 1 && (param == 2 || param == 3)) {
            for(i=0; i<4; i++) {
                if(playerType[i][0] != playerType[i][1]) {
                    posP[i][playerType[i][1]]->x = centerPos3D.x;
                    posP[i][playerType[i][1]]->y = centerPos3D.y;
                    posP[i][playerType[i][1]]->z = centerPos3D.z;
                    rotP[i][playerType[i][1]]->x = 0;
                    rotP[i][playerType[i][1]]->y = 0;
                    rotP[i][playerType[i][1]]->z = 0;
                    scaleP[i][playerType[i][1]]->x = 0;
                    scaleP[i][playerType[i][1]]->y = 0;
                    scaleP[i][playerType[i][1]]->z = 0;
                    Hu3DModelDispOn(model->mdlId[i+MDMODEL_CAP_MAN_1P+(playerType[i][0]*4)]);
                }
            }
            radius = 200;
            angle = 90;
            for(i=0; i<4; i++) {
                oldPlayerPos3D[i] = playerPos3D[i];
                oldPlayerPos3D[i].z -= radius;
            }
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 30);
        for(i=0; i<4; i++) {
            MdMathLerp(&posP[i][playerType[i][0]]->x, centerPos3D.x, playerPos3D[i].x, sys->procT);
            MdMathLerp(&posP[i][playerType[i][0]]->y, centerPos3D.y, playerPos3D[i].y, sys->procT);
            MdMathLerp(&posP[i][playerType[i][0]]->z, centerPos3D.z, playerPos3D[i].z, sys->procT);
            MdMathLerpScale(scaleP[i][playerType[i][0]], 0, 1, sys->procT);
        }
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15);
        for(i=0; i<4; i++) {
            MdMathLerpScale(scaleP[i][playerType[i][0]], 1, 0, sys->procT);
        }
    } else if(param == 2 || param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 20);
        if(param == 2) {
            MdMathLerp(&angle, 90, 270, sys->procT);
        } else if(param == 3) {
            MdMathLerp(&angle, 90, -90, sys->procT);
        }
        for(i=0; i<4; i++) {
            if(playerType[i][0] != playerType[i][1]) {
                MdMathLerpScale(scaleP[i][playerType[i][1]], 1, 0, sys->procT);
                posP[i][playerType[i][1]]->x = oldPlayerPos3D[i].x+(radius*HuCos(angle));
                posP[i][playerType[i][1]]->y = oldPlayerPos3D[i].y;
                posP[i][playerType[i][1]]->z = oldPlayerPos3D[i].z+(radius*HuSin(angle));
                MdMathLerpScale(scaleP[i][playerType[i][0]], 0, 1, sys->procT);
                posP[i][playerType[i][0]]->x = oldPlayerPos3D[i].x+(radius*HuCos(180.0f+angle));
                posP[i][playerType[i][0]]->y = oldPlayerPos3D[i].y;
                posP[i][playerType[i][0]]->z = oldPlayerPos3D[i].z+(radius*HuSin(180.0f+angle));
            }
        }
        if(sys->maxPlayerNum > 1) {
            sys->cursorMode = MDPARTY_CURSOR_PLAYERSEL;
            MdProcCall(MDPARTY_PROC_CURSOR, sys->cursorDir+2);
        }
    }
    for(i=0; i<4; i++) {
        MdPartyModelBBoardSet(i+MDMODEL_CAP_MAN_1P, MDCAMERA_MAIN);
        MdPartyModelBBoardSet(i+MDMODEL_CAP_COM_1P, MDCAMERA_MAIN);
    }
    if(sys->procTime >= 1) {
        if(param != 0) {
            if(param == 1) {
                for(i=0; i<4; i++) {
                    Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_MAN_1P+(playerType[i][0]*4)]);
                }
            } else if(param == 2 || param == 3) {
                for(i=0; i<4; i++) {
                    if(playerType[i][0] != playerType[i][1]) {
                        Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_MAN_1P+(playerType[i][1]*4)]);
                    }
                }
            }
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcChr(int param)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *posP; //r29
    HuVecF *scaleP; //r28
    HuVecF *rotP; //r26
    MDPARTY_STAR *star; //r25
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    static HuVecF centerPos = { 0, 0, -1000 }; //data+0x674
    static HuVecF chrPos[10] = { //data+0x680
        { -440, 400, -600 },
        { -220, 400, -600 },
        { 0, 400, -600 },
        { 220, 400, -600 },
        { 440, 400, -600 },
        { -440, 130, -600 },
        { -220, 130, -600 },
        { 0, 130, -600 },
        { 220, 130, -600 },
        { 440, 130, -600 }
    };
    static HuVecF playerPos[4] = { //data+0x6F8
        { -360, -160, 250 },
        { -120, -160, 500 },
        { 120, -160, 750 },
        { 360, -160, 1000 }
    };
    static HuVecF teamPlayerPos[4] = { //data+0x728
        { -460, -80, 250 },
        { -220, -80, 500 },
        { 220, -80, 750 },
        { 460, -80, 1000 }
    };
    static HuVec2f cursorTeamPos[4] = { //data+0x758
        { -130, -900 },
        { 130, -1100 },
        { -30, 900 },
        { 80, 1100 },
    };
    static HuVecF vsTeamPos[4] = { //data+0x778
        { -460, -160, -600 },
        { -220, -160, -600 },
        { 220, -160, -600 },
        { 460, -160, -600 },
    };
    static HuVecF handicapPlayerPos[4] = { //data+0x7A8
        { -360, 100, -600 },
        { -120, 100, -600 },
        { 120, 100, -600 },
        { 360, 100, -600 },
    };
    static HuVecF handicapTeamPos[4] = { //data+0x7D8
        { -460, 100, -600 },
        { -220, 100, -600 },
        { 220, 100, -600 },
        { 460, 100, -600 },
    };
    camera = sys->camera;
    model = sys->model;
    star = sys->star;
    if(!sys->dirtyF) {
        if(param == 0) {
            for(i=0; i<10; i++) {
                posP = model->posP[i+MDMODEL_CAP_CHR_00];
                rotP = model->rotP[i+MDMODEL_CAP_CHR_00];
                scaleP = model->scaleP[i+MDMODEL_CAP_CHR_00];
                *posP = centerPos;
                scaleP->x = 0;
                scaleP->y = 0;
                scaleP->z = 0;
                Hu3DModelDispOn(model->mdlId[i+MDMODEL_CAP_CHR_00]);
                Hu3DModelDispOn(model->mdlId[i+MDMODEL_MARIO]);
            }
            Hu3DModelDispOn(model->mdlId[MDMODEL_STAGE_CHR]);
            Hu3DLayerHookSet(1+HU3D_LAYER_HOOK_POST, MdTexBufLayerHook);
            MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
        } else if(param != 1) {
            if(param == 2) {
                star->mode = MDPARTY_STAR_MOVE_LEFT_START;
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 3) {
                star->mode = MDPARTY_STAR_CENTER_START;
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 4) {
                star->mode = MDPARTY_STAR_MOVE_LEFT_START;
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 5) {
                star->mode = MDPARTY_STAR_CENTER_START;
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 6) {
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 7) {
                
            } else if(param == 8) {
                
            } else if(param == 9) {
                
            } else if(param == 10) {
                
            } else if(param == 11) {
                
            } else if(param == 12) {
                
            }
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 30);
        for(i=0; i<10; i++) {
            posP = model->posP[i+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[i+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[i+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->x, centerPos.x, chrPos[i].x, sys->procT);
            MdMathLerp(&posP->y, centerPos.y, chrPos[i].y, sys->procT);
            MdMathLerp(&posP->z, centerPos.z, chrPos[i].z, sys->procT);
            MdMathLerp(&scaleP->x, 0, 1, sys->procT);
            MdMathLerp(&scaleP->y, 0, 1, sys->procT);
            MdMathLerp(&scaleP->z, 0, 0.1f, sys->procT);
        }
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15);
        for(i=0; i<10; i++) {
            posP = model->posP[i+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[i+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[i+MDMODEL_CAP_CHR_00];
            MdMathLerp(&scaleP->x, 1, 0, sys->procT);
            MdMathLerp(&scaleP->y, 1, 0, sys->procT);
            MdMathLerp(&scaleP->z, 0.1f, 0, sys->procT);
        }
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->x, chrPos[sys->chrSel[i]].x, playerPos[i].x, sys->procT);
            MdMathLerp(&posP->y, chrPos[sys->chrSel[i]].y, playerPos[i].y, sys->procT);
            MdMathLerp(&posP->z, chrPos[sys->chrSel[i]].z, playerPos[i].z, sys->procOfsT);
        }
        for(i=0; i<10; i++) {
            posP = model->posP[i+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[i+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[i+MDMODEL_CAP_CHR_00];
            if(i != sys->chrSel[0] && i != sys->chrSel[1] && i != sys->chrSel[2] && i != sys->chrSel[3]) {
                MdMathLerp(&scaleP->x, 1, 0, sys->procT);
                MdMathLerp(&scaleP->y, 1, 0, sys->procT);
                MdMathLerp(&scaleP->z, 0.1f, 0, sys->procT);
            }
        }
        MdPartyPlayerSprUpdate();
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->x, playerPos[i].x, chrPos[sys->chrSel[i]].x, sys->procT);
            MdMathLerp(&posP->y, playerPos[i].y, chrPos[sys->chrSel[i]].y, sys->procT);
            MdMathLerp(&posP->z, chrPos[sys->chrSel[i]].z, playerPos[i].z, sys->procOfsT);
        }
        for(i=0; i<10; i++) {
            posP = model->posP[i+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[i+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[i+MDMODEL_CAP_CHR_00];
            if(i != sys->chrSel[0] && i != sys->chrSel[1] && i != sys->chrSel[2] && i != sys->chrSel[3]) {
                MdMathLerp(&scaleP->x, 0, 1, sys->procT);
                MdMathLerp(&scaleP->y, 0, 1, sys->procT);
                MdMathLerp(&scaleP->z, 0.0f, 0.1f, sys->procT);
            }
        }
        MdPartyPlayerSprUpdate();
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->x, chrPos[sys->chrSel[i]].x, teamPlayerPos[i].x, sys->procT);
            MdMathLerp(&posP->y, chrPos[sys->chrSel[i]].y, teamPlayerPos[i].y, sys->procT);
            MdMathLerp(&posP->z, chrPos[sys->chrSel[i]].z, teamPlayerPos[i].z, sys->procOfsT);
        }
        for(i=0; i<10; i++) {
            posP = model->posP[i+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[i+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[i+MDMODEL_CAP_CHR_00];
            if(i != sys->chrSel[0] && i != sys->chrSel[1] && i != sys->chrSel[2] && i != sys->chrSel[3]) {
                MdMathLerp(&scaleP->x, 1, 0, sys->procT);
                MdMathLerp(&scaleP->y, 1, 0, sys->procT);
                MdMathLerp(&scaleP->z, 0.1f, 0, sys->procT);
            }
        }
        MdPartyPlayerSprUpdate();
    } else if(param == 5) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->x, teamPlayerPos[sys->teamOrder[i]].x, chrPos[sys->chrSel[i]].x, sys->procT);
            MdMathLerp(&posP->y, teamPlayerPos[sys->teamOrder[i]].y, chrPos[sys->chrSel[i]].y, sys->procT);
            MdMathLerp(&posP->z, chrPos[sys->chrSel[i]].z, teamPlayerPos[sys->teamOrder[i]].z, sys->procOfsT);
        }
        for(i=0; i<10; i++) {
            posP = model->posP[i+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[i+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[i+MDMODEL_CAP_CHR_00];
            if(i != sys->chrSel[0] && i != sys->chrSel[1] && i != sys->chrSel[2] && i != sys->chrSel[3]) {
                MdMathLerp(&scaleP->x, 0, 1, sys->procT);
                MdMathLerp(&scaleP->y, 0, 1, sys->procT);
                MdMathLerp(&scaleP->z, 0.0f, 0.1f, sys->procT);
            }
        }
        MdPartyPlayerSprUpdate();
    } else if(param == 6) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15);
        for(i=0; i<4; i++) {
            if(sys->teamOrder[i] != sys->teamOrderBackup[i]) {
                posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
                scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
                MdMathLerp(&posP->x, teamPlayerPos[sys->teamOrderBackup[i]].x, teamPlayerPos[sys->teamOrder[i]].x, sys->procT);
                MdMathLerp(&posP->y, teamPlayerPos[sys->teamOrderBackup[i]].y, cursorTeamPos[sys->teamOrder[i]].x, sys->procOfsT);
                MdMathLerp(&posP->z, -600, cursorTeamPos[sys->teamOrder[i]].y, sys->procOfsT);
            }
        }
        MdPartyPlayerSprUpdate();
        sys->cursorMode = MDPARTY_CURSOR_TEAM_SEL;
        MdProcCall(MDPARTY_PROC_CURSOR, sys->cursorDir+2);
    } else if(param == 7) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, teamPlayerPos[sys->teamOrder[i]].y, vsTeamPos[sys->teamOrder[i]].y, sys->procT);
        }
        MdPartyPlayerSprUpdate();
        MdProcCall(MDPARTY_PROC_VS, 4);
    } else if(param == 8) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, vsTeamPos[sys->teamOrder[i]].y, teamPlayerPos[sys->teamOrder[i]].y, sys->procT);
        }
        MdPartyPlayerSprUpdate();
        MdProcCall(MDPARTY_PROC_VS, 5);
    } else if(param == 9) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, playerPos[i].y, handicapPlayerPos[i].y, sys->procT);
        }
        MdPartyPlayerSprUpdate();
    } else if(param == 10) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, handicapPlayerPos[i].y, playerPos[i].y, sys->procT);
        }
        MdPartyPlayerSprUpdate();
    } else if(param == 11) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, vsTeamPos[sys->teamOrder[i]].y, handicapTeamPos[sys->teamOrder[i]].y, sys->procT);
        }
        MdPartyPlayerSprUpdate();
        MdProcCall(MDPARTY_PROC_VS, 2);
    } else if(param == 12) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 30);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            rotP = model->rotP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, handicapTeamPos[sys->teamOrder[i]].y, vsTeamPos[sys->teamOrder[i]].y, sys->procT);
        }
        MdPartyPlayerSprUpdate();
        MdProcCall(MDPARTY_PROC_VS, 3);
    }
    if(sys->procTime >= 1.0f) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcTeam(int param)
{
    HuVec2f *sprPos; //r31
    MDSPRITE_WORK *sprite; //r29
    MDMESS_WORK *mess; //r28
    HUSPRGRPID grpId; //r27
    
    static HuVecF winOfs = { 114, 8, 0 }; //data+0x808
    static HuVecF pos[4]; //bss+0x190
    
    sprite = sys->sprite;
    mess = sys->mess;
    grpId = sprite->grpId[MDSPR_GRP_TEAM];
    if(!sys->teamInitF) {
        pos[0].x = -128;
        pos[0].y = 160;
        pos[0].z = 0;
        pos[1].x = 140;
        pos[1].y = 160;
        pos[1].z = 0;
        pos[2].x = 704;
        pos[2].y = 160;
        pos[2].z = 0;
        pos[3].x = 436;
        pos[3].y = 160;
        pos[3].z = 0;
        if(param == 0 || param == 3) {
            HuSprGrpPosSet(grpId, 0, 0);
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_TEAM, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_RED));
            sprPos->x = pos[0].x;
            sprPos->y = pos[0].y;
            HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_RED));
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_TEAM, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_BLUE));
            sprPos->x = pos[2].x;
            sprPos->y = pos[2].y;
            HuSprDispOn(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_BLUE));
        } else if(param == 1 || param == 2) {
            
        }
        sys->teamInitF = TRUE;
    }
    if(param == 0 || param == 3) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 30);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_TEAM, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_RED));
        MdMathLerp(&sprPos->x, pos[0].x, pos[1].x, sys->procT);
        MdMessWinPosSet(mess, MDMESS_WIN_TEAM1, sprPos->x-winOfs.x, sprPos->y-winOfs.y);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_TEAM, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_BLUE));
        MdMathLerp(&sprPos->x, pos[2].x, pos[3].x, sys->procT);
        MdMessWinPosSet(mess, MDMESS_WIN_TEAM2, sprPos->x-winOfs.x, sprPos->y-winOfs.y);
        if(param == 0) {
            MdProcCall(MDPARTY_PROC_VS, 0);
        }
    } else if(param == 1 || param == 2) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 30);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_TEAM, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_RED));
        MdMathLerp(&sprPos->x, pos[1].x, pos[0].x, sys->procT);
        MdMessWinPosSet(mess, MDMESS_WIN_TEAM1, sprPos->x-winOfs.x, sprPos->y-winOfs.y);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_TEAM, MDSPR_GRP_MEMBERNO(MDSPR_GRP_TEAM, MDSPR_TEAM_BLUE));
        MdMathLerp(&sprPos->x, pos[3].x, pos[2].x, sys->procT);
        MdMessWinPosSet(mess, MDMESS_WIN_TEAM2, sprPos->x-winOfs.x, sprPos->y-winOfs.y);
        if(param == 2) {
            MdProcCall(MDPARTY_PROC_VS, 1);
        }
    }
    if(sys->procTime >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcVs(int param)
{
    HuVecF *posP; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *rotP; //r29
    HuVecF *scaleP; //r28
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    static HuVecF mdlPos[5] = { //data+0x814
        { -20, 150, 500 },
        { -20, -50, 0 },
        { -20, 150, -500 },
        { -20, -120, 0 },
        { -20, 75, 0 }
    };
    static HuVecF mdlRot[3] = { //data+0x850
        { 60, 0, 270 },
        { 0, 0, 0 },
        { 0, -270, 0 }
    };
    
    camera = sys->camera;
    model = sys->model;
    posP = model->posP[MDMODEL_VS];
    rotP = model->rotP[MDMODEL_VS];
    scaleP = model->scaleP[MDMODEL_VS];
    if(!sys->winInitF) {
        if(param == 0) {
            *posP = mdlPos[0];
            *rotP = mdlRot[0];
            scaleP->x = 3;
            scaleP->y = 3;
            scaleP->z = 3;
            Hu3DModelDispOn(model->mdlId[MDMODEL_VS]);
        }
        sys->winInitF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 30);
        MdMathLerp(&posP->x, mdlPos[0].x, mdlPos[1].x, sys->sprT);
        MdMathLerp(&posP->y, mdlPos[0].y, mdlPos[1].y, sys->sprT);
        MdMathLerp(&posP->z, mdlPos[0].z, mdlPos[1].z, sys->sprT);
        MdMathLerp(&rotP->x, mdlRot[0].x, mdlRot[1].x, sys->sprT);
        MdMathLerp(&rotP->y, mdlRot[0].y, mdlRot[1].y, sys->sprT);
        MdMathLerp(&rotP->z, mdlRot[0].z, mdlRot[1].z, sys->sprT);
        MdMathLerpScale(scaleP, 3, 1, sys->sprT);
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 30);
        MdMathLerp(&posP->x, mdlPos[1].x, mdlPos[2].x, sys->sprT);
        MdMathLerp(&posP->y, mdlPos[1].y, mdlPos[2].y, sys->sprT);
        MdMathLerp(&posP->z, mdlPos[1].z, mdlPos[2].z, sys->sprT);
        MdMathLerp(&rotP->x, mdlRot[1].x, mdlRot[2].x, sys->sprT);
        MdMathLerp(&rotP->y, mdlRot[1].y, mdlRot[2].y, sys->sprT);
        MdMathLerp(&rotP->z, mdlRot[1].z, mdlRot[2].z, sys->sprT);
        MdMathLerpScale(scaleP, 1, 0, sys->sprT);
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->sprTime, &sys->sprT, 30);
        MdMathLerp(&posP->x, mdlPos[3].x, mdlPos[4].x, sys->sprT);
        MdMathLerp(&posP->y, mdlPos[3].y, mdlPos[4].y, sys->sprT);
        MdMathLerp(&posP->z, mdlPos[3].z, mdlPos[4].z, sys->sprT);
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->sprTime, &sys->sprT, 30);
        MdMathLerp(&posP->x, mdlPos[4].x, mdlPos[3].x, sys->sprT);
        MdMathLerp(&posP->y, mdlPos[4].y, mdlPos[3].y, sys->sprT);
        MdMathLerp(&posP->z, mdlPos[4].z, mdlPos[3].z, sys->sprT);
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->sprTime, &sys->sprT, 30);
        MdMathLerp(&posP->x, mdlPos[1].x, mdlPos[3].x, sys->sprT);
        MdMathLerp(&posP->y, mdlPos[1].y, mdlPos[3].y, sys->sprT);
        MdMathLerp(&posP->z, mdlPos[1].z, mdlPos[3].z, sys->sprT);
    } else if(param == 5) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->sprTime, &sys->sprT, 30);
        MdMathLerp(&posP->x, mdlPos[3].x, mdlPos[1].x, sys->sprT);
        MdMathLerp(&posP->y, mdlPos[3].y, mdlPos[1].y, sys->sprT);
        MdMathLerp(&posP->z, mdlPos[3].z, mdlPos[1].z, sys->sprT);
    }
    if(sys->sprTime >= 1) {
        if(param == 1) {
            Hu3DModelDispOff(model->mdlId[MDMODEL_VS]);
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcMap(int param)
{
    int i; //r31
    HuVecF *scaleP; //r30
    HuVecF *posP; //r29
    MDMODEL_WORK *model; //r28
    int mapMax; //r27
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    static HuVecF centerPos = { 0, 180, -200 }; //data+0x874
    static float lerpScale[3] = { 1.5f, 0.5f, 2.0f }; //data+0x880
    static float angle; //bss+0x18C
    
    mapMax = 7;
    camera = sys->camera;
    model = sys->model;
    if(GwCommon.w07Unlock == TRUE) {
        mapMax = 7;
    } else {
        mapMax = 6;
    }
    if(!sys->dirtyF) {
        if(param == 0) {
            if(mapMax == 7) {
                for(i=0; i<7; i++) {
                    posP = model->posP[i+MDMODEL_MAP1];
                    scaleP = model->scaleP[i+MDMODEL_MAP1];
                    posP->x = centerPos.x+(sys->selMapRadius*HuCos(sys->selMapAngle-((360.0f/7.0f)*i)));
                    posP->y = centerPos.y;
                    posP->z = centerPos.z+(sys->selMapRadius*HuSin(sys->selMapAngle-((360.0f/7.0f)*i)));
                    scaleP->x = 0;
                    scaleP->y = 0;
                    scaleP->z = 0;
                    Hu3DModelDispOn(model->mdlId[i+MDMODEL_MAP1]);
                }
            } else {
                for(i=0; i<6; i++) {
                    posP = model->posP[i+MDMODEL_MAP1];
                    scaleP = model->scaleP[i+MDMODEL_MAP1];
                    posP->x = centerPos.x+(sys->selMapRadius*HuCos(sys->selMapAngle-((360.0f/6.0f)*i)));
                    posP->y = centerPos.y;
                    posP->z = centerPos.z+(sys->selMapRadius*HuSin(sys->selMapAngle-((360.0f/6.0f)*i)));
                    scaleP->x = 0;
                    scaleP->y = 0;
                    scaleP->z = 0;
                    Hu3DModelDispOn(model->mdlId[i+MDMODEL_MAP1]);
                }
            }
            if(sys->gameType == 1) {
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            }
        } else if(param == 2) {
            angle = sys->selMapAngle;
        } else if(param == 3) {
            
        } else if(param == 4) {
            
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        if(mapMax == 7) {
            MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 60.0f);
            MdMathLerp(&sys->selMapAngle, 450.0f+((360.0f/7.0f)*sys->selMapNo), 90.0f+((360.0f/7.0f)*sys->selMapNo), sys->procT);
            MdMathLerp(&sys->selMapRadius, 50, 300, sys->procT);
            for(i=0; i<7; i++) {
                posP = model->posP[i+MDMODEL_MAP1];
                posP->x = centerPos.x+(sys->selMapRadius*HuCos(sys->selMapAngle-((360.0f/7.0f)*i)));
                posP->y = centerPos.y;
                posP->z = centerPos.z+(sys->selMapRadius*HuSin(sys->selMapAngle-((360.0f/7.0f)*i)));
            }
            for(i=0; i<7; i++) {
                scaleP = model->scaleP[i+MDMODEL_MAP1];
                if(i == sys->selMapNo) {
                    MdMathLerpScale(scaleP, 0, lerpScale[0], sys->procT);
                } else {
                    MdMathLerpScale(scaleP, 0, lerpScale[1], sys->procT);
                }
            }
        } else {
            MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 60.0f);
            MdMathLerp(&sys->selMapAngle, 450.0f+((360.0f/6.0f)*sys->selMapNo), 90.0f+((360.0f/6.0f)*sys->selMapNo), sys->procT);
            MdMathLerp(&sys->selMapRadius, 50, 300, sys->procT);
            for(i=0; i<6; i++) {
                posP = model->posP[i+MDMODEL_MAP1];
                posP->x = centerPos.x+(sys->selMapRadius*HuCos(sys->selMapAngle-((360.0f/6.0f)*i)));
                posP->y = centerPos.y;
                posP->z = centerPos.z+(sys->selMapRadius*HuSin(sys->selMapAngle-((360.0f/6.0f)*i)));
            }
            for(i=0; i<6; i++) {
                scaleP = model->scaleP[i+MDMODEL_MAP1];
                if(i == sys->selMapNo) {
                    MdMathLerpScale(scaleP, 0, lerpScale[0], sys->procT);
                } else {
                    MdMathLerpScale(scaleP, 0, lerpScale[1], sys->procT);
                }
            }
        }
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        if(mapMax == 7) {
            for(i=0; i<7; i++) {
                scaleP = model->scaleP[i+MDMODEL_MAP1];
                if(i == sys->selMapNo) {
                    MdMathLerpScale(scaleP, lerpScale[0], 0, sys->procT);
                } else {
                    MdMathLerpScale(scaleP, lerpScale[1], 0, sys->procT);
                }
            }
        } else {
            for(i=0; i<6; i++) {
                scaleP = model->scaleP[i+MDMODEL_MAP1];
                if(i == sys->selMapNo) {
                    MdMathLerpScale(scaleP, lerpScale[0], 0, sys->procT);
                } else {
                    MdMathLerpScale(scaleP, lerpScale[1], 0, sys->procT);
                }
            }
        }
    } else if(param == 2) {
        if(mapMax == 7) {
            MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
            if(sys->cursorDir == 0) {
                MdMathLerp(&sys->selMapAngle, angle, angle+(360.0f/7.0f), sys->procT);
            } else if(sys->cursorDir == 1) {
                MdMathLerp(&sys->selMapAngle, angle, angle-(360.0f/7.0f), sys->procT);
            }
            for(i=0; i<7; i++) {
                posP = model->posP[i+MDMODEL_MAP1];
                posP->x = centerPos.x+(sys->selMapRadius*HuCos(sys->selMapAngle-((360.0f/7.0f)*i)));
                posP->z = centerPos.z+(sys->selMapRadius*HuSin(sys->selMapAngle-((360.0f/7.0f)*i)));
            }
            for(i=0; i<7; i++) {
                scaleP = model->scaleP[i+MDMODEL_MAP1];
                if(i == sys->prevSelMapNo) {
                    MdMathLerpScale(scaleP, lerpScale[0], lerpScale[1], sys->procT);
                } else if(i == sys->selMapNo) {
                    MdMathLerpScale(scaleP, lerpScale[1], lerpScale[0], sys->procT);
                } else {
                    scaleP->x = lerpScale[1];
                    scaleP->y = lerpScale[1];
                    scaleP->z = lerpScale[1];
                }
            }
        } else {
            MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
            if(sys->cursorDir == 0) {
                MdMathLerp(&sys->selMapAngle, angle, angle+(360.0f/6.0f), sys->procT);
            } else if(sys->cursorDir == 1) {
                MdMathLerp(&sys->selMapAngle, angle, angle-(360.0f/6.0f), sys->procT);
            }
            for(i=0; i<6; i++) {
                posP = model->posP[i+MDMODEL_MAP1];
                posP->x = centerPos.x+(sys->selMapRadius*HuCos(sys->selMapAngle-((360.0f/6.0f)*i)));
                posP->z = centerPos.z+(sys->selMapRadius*HuSin(sys->selMapAngle-((360.0f/6.0f)*i)));
            }
            for(i=0; i<6; i++) {
                scaleP = model->scaleP[i+MDMODEL_MAP1];
                if(i == sys->prevSelMapNo) {
                    MdMathLerpScale(scaleP, lerpScale[0], lerpScale[1], sys->procT);
                } else if(i == sys->selMapNo) {
                    MdMathLerpScale(scaleP, lerpScale[1], lerpScale[0], sys->procT);
                } else {
                    scaleP->x = lerpScale[1];
                    scaleP->y = lerpScale[1];
                    scaleP->z = lerpScale[1];
                }
            }
        }
        sys->cursorMode = MDPARTY_CURSOR_MAP_SEL;
        if(sys->cursorDir == 0) {
            MdProcCall(MDPARTY_PROC_CURSOR, 3);
        } else if(sys->cursorDir == 1) {
            MdProcCall(MDPARTY_PROC_CURSOR, 2);
        }
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15.0f);
        for(i=0; i<mapMax; i++) {
            scaleP = model->scaleP[i+MDMODEL_MAP1];
            if(i == sys->selMapNo) {
                MdMathLerpScaleOfs(scaleP, lerpScale[0], lerpScale[2], sys->procT, sys->procOfsT*4);
            } else {
                MdMathLerpScale(scaleP, lerpScale[1], 0, sys->procT);
            }
        }
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        for(i=0; i<mapMax; i++) {
            scaleP = model->scaleP[i+MDMODEL_MAP1];
            if(i == sys->selMapNo) {
                MdMathLerpScale(scaleP, lerpScale[2], lerpScale[0], sys->procT);
            } else {
                MdMathLerpScale(scaleP, 0, lerpScale[1], sys->procT);
            }
        }
    }
    if(sys->procTime >= 1.0f) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcMapName(int param)
{
    int i; //r31
    HuVec2f *sprPos; //r30
    MDSPRITE_WORK *sprite; //r29
    HUSPRGRPID grpId; //r28
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    static HuVecF pos[2] = { { 288, -128, 0 }, { 288, 64, 0 } };
    
    camera = sys->camera;
    sprite = sys->sprite;
    grpId = sprite->grpId[MDSPR_GRP_MAP_NAME];
    if(!sys->dirtyF) {
        if(param == 0) {
            HuSprGrpPosSet(grpId, 0, 0);
            for(i=0; i<7; i++) {
                if(i == sys->selMapNo) {
                    HuSprDispOn(grpId, (int)i);
                } else {
                    HuSprDispOff(grpId, (int)i);
                }
                HuSprPosSet(grpId, (int)i, pos[0].x, pos[0].y);
            }
        } else if(param == 1) {
            
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 15.0f);
        for(i=0; i<7; i++) {
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_MAP_NAME, i);
            MdMathLerp(&sprPos->x,  pos[0].x, pos[1].x, sys->procT);
            MdMathLerp(&sprPos->y, pos[0].y, pos[1].y, sys->procT);
        }
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        for(i=0; i<7; i++) {
            sprPos = MdSpritePosGet(sprite, MDSPR_GRP_MAP_NAME, i);
            MdMathLerp(&sprPos->x,  pos[1].x, pos[0].x, sys->procT);
            MdMathLerp(&sprPos->y, pos[1].y, pos[0].y, sys->procT);
        }
    }
    if(sys->procTime >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcCapTurn(int param)
{
    HuVecF *posP; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *rotP; //r29
    HuVecF *scaleP;
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    camera = sys->camera;
    model = sys->model;
    posP = model->posP[MDMODEL_CAP_TURN];
    rotP = model->rotP[MDMODEL_CAP_TURN];
    scaleP = model->scaleP[MDMODEL_CAP_TURN];
    
    if(!sys->dirtyF) {
        if(param == 0) {
            posP->x = 0;
            posP->y = 145;
            posP->z = 400;
            
            rotP->x = 0;
            rotP->y = 0;
            rotP->z = 0;
            
            scaleP->x = 0;
            scaleP->y = 0;
            scaleP->z = 0;
            Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_TURN]);
        } else if(param != 1) {
            if(param == 2) {
                Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_CAP_TURN], sys->selTurn);
            } else if(param == 3) {
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 4) {
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            }
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathLerpScale(scaleP, 0, 1, sys->procT);
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathLerpScale(scaleP, 1, 0, sys->procT);
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        sys->cursorMode = MDPARTY_CURSOR_TURN_SEL;
        MdProcCall(MDPARTY_PROC_CURSOR, sys->cursorDir+2);
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15.0f);
        MdMathLerp(&posP->x, 0, 525, sys->procT);
        MdMathLerp(&posP->y, 145, 365, sys->procT);
        MdMathLerpOfs(&posP->z, 400, -50, sys->procT, sys->procOfsT*-1);
        MdMathLerpScale(scaleP, 1, 0.8f, sys->procT);
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15.0f);
        MdMathLerp(&posP->x, 525, 0, sys->procT);
        MdMathLerp(&posP->y, 365, 145, sys->procT);
        MdMathLerpOfs(&posP->z, -50, 400, sys->procT, sys->procOfsT*1);
        MdMathLerpScale(scaleP, 0.8f, 1.0f, sys->procT);
    }
    MdPartyModelBBoardSet(MDMODEL_CAP_TURN, MDCAMERA_MAIN);
    if(sys->procTime >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcCapPack(int param)
{
    
    int i; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *posP[5]; //sp+0x34
    HuVecF *rotP[5]; //sp+0x20
    HuVecF *scaleP[5]; //sp+0xC
    MDCAMERA_WORK *camera; //sp+0x8
    
    static HuVecF lerpPos[4]; //bss+0x15C
    
    camera = sys->camera;
    model = sys->model;
    for(i=0; i<5; i++) {
        posP[i] = model->posP[i+MDMODEL_CAP_PACK0];
        rotP[i] = model->rotP[i+MDMODEL_CAP_PACK0];
        scaleP[i] = model->scaleP[i+MDMODEL_CAP_PACK0];
    }
    if(!sys->dirtyF) {
        if(param == 0) {
            for(i=0; i<5; i++) {
                posP[i]->x = 0;
                posP[i]->y = 145;
                posP[i]->z = 400;

                scaleP[i]->x = 0;
                scaleP[i]->y = 0;
                scaleP[i]->z = 0;
            }
            for(i=0; i<5; i++) {
                if(i == sys->selPack) {
                    Hu3DModelDispOn(model->mdlId[i+MDMODEL_CAP_PACK0]);
                } else {
                    Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_PACK0]);
                }
            }
        } else if(param != 1 && param != 2) {
            if(param == 3) {
                lerpPos[0] = *posP[sys->selPack];
                lerpPos[1].x = -525;
                lerpPos[1].y = 180;
                lerpPos[1].z = -50;
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 4) {
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            }
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        for(i=0; i<5; i++) {
            MdMathLerpScale(scaleP[i], 0, 1, sys->procT);
        }
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        sys->cursorMode = MDPARTY_CURSOR_PACK_SEL;
        MdPartyCursorDirLR(sys->cursorDir);
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 15.0f);
        for(i=0; i<5; i++) {
            MdMathLerpScale(scaleP[i], 1, 0, sys->procT);
        }
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15.0f);
        MdMathLerp(&posP[0]->x, lerpPos[0].x, lerpPos[1].x, sys->procT);
        MdMathLerp(&posP[0]->y, lerpPos[0].y, lerpPos[1].y, sys->procT);
        MdMathLerpOfs(&posP[0]->z, lerpPos[0].z, lerpPos[1].z, sys->procT, sys->procOfsT*-1);
        *posP[1] = *posP[0];
        *posP[2] = *posP[0];
        *posP[3] = *posP[0];
        *posP[4] = *posP[0];
        MdMathLerpScale(scaleP[0], 1.0f, 0.8f, sys->procT);
        *scaleP[1] = *scaleP[0];
        *scaleP[2] = *scaleP[0];
        *scaleP[3] = *scaleP[0];
        *scaleP[4] = *scaleP[0];
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 15.0f);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15.0f);
        MdMathLerp(&posP[0]->x, lerpPos[1].x, lerpPos[0].x, sys->procT);
        MdMathLerp(&posP[0]->y, lerpPos[1].y, lerpPos[0].y, sys->procT);
        MdMathLerpOfs(&posP[0]->z, lerpPos[1].z, lerpPos[0].z, sys->procT, sys->procOfsT*1);
        *posP[1] = *posP[0];
        *posP[2] = *posP[0];
        *posP[3] = *posP[0];
        *posP[4] = *posP[0];
        MdMathLerpScale(scaleP[0], 0.8f, 1.0f, sys->procT);
        *scaleP[1] = *scaleP[0];
        *scaleP[2] = *scaleP[0];
        *scaleP[3] = *scaleP[0];
        *scaleP[4] = *scaleP[0];
    }
    for(i=0; i<5; i++) {
        MdPartyModelBBoardSet(i+MDMODEL_CAP_PACK0, MDCAMERA_MAIN);
    }
    if(sys->procTime >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcCapBonus(int param)
{
    HuVecF *posP; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *rotP; //r29
    HuVecF *scaleP;
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    camera = sys->camera;
    model = sys->model;
    posP = model->posP[MDMODEL_CAP_BONUS];
    rotP = model->rotP[MDMODEL_CAP_BONUS];
    scaleP = model->scaleP[MDMODEL_CAP_BONUS];
    
    if(!sys->dirtyF) {
        if(param == 0) {
            posP->x = 0;
            posP->y = 145;
            posP->z = 400;
            
            rotP->x = 0;
            rotP->y = 0;
            rotP->z = 0;
            
            scaleP->x = 0;
            scaleP->y = 0;
            scaleP->z = 0;
            Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_BONUS]);
        } else if(param != 1) {
            if(param == 2) {
                Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_CAP_BONUS], sys->selBonus);
            } else if(param == 3) {
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            } else if(param == 4) {
                MdPartyAudExec(MDPARTY_AUD_SE_MENU_MOVE);
            }
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathLerpScale(scaleP, 0, 1, sys->procT);
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathLerpScale(scaleP, 1, 0, sys->procT);
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        sys->cursorMode = MDPARTY_CURSOR_BONUS_SEL;
        MdProcCall(MDPARTY_PROC_CURSOR, sys->cursorDir+2);
    } else if(param == 3) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15.0f);
        MdMathLerp(&posP->x, 0, 525, sys->procT);
        MdMathLerp(&posP->y, 145, 180, sys->procT);
        MdMathLerpOfs(&posP->z, 400, -50, sys->procT, sys->procOfsT*-1);
        MdMathLerpScale(scaleP, 1, 0.8f, sys->procT);
    } else if(param == 4) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 15.0f);
        MdMathLerp(&posP->x, 525, 0, sys->procT);
        MdMathLerp(&posP->y, 180, 145, sys->procT);
        MdMathLerpOfs(&posP->z, -50, 400, sys->procT, sys->procOfsT*1);
        MdMathLerpScale(scaleP, 0.8f, 1.0f, sys->procT);
    }
    MdPartyModelBBoardSet(MDMODEL_CAP_BONUS, MDCAMERA_MAIN);
    if(sys->procTime >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcHandicap(int param)
{
    MDMODEL_WORK *model; //r30
    int i; //r30
    HuVec2f *sprPos; //r29
    HuVec2f *numSprPos; //r28
    MDSPRITE_WORK *sprite; //r27
    MDCAMERA_WORK *camera; //r26
    HUSPRGRPID grpId; //r25
    
    HuVecF pos3D; //sp+0x18
    HuVecF pos2D; //sp+0xC
    MDMESS_WORK *mess; //sp+0x8
    
    camera = sys->camera;
    model = sys->model;
    sprite = sys->sprite;
    mess = sys->mess;
    grpId = sprite->grpId[MDSPR_GRP_HANDICAP];
    
    if(!sys->handicapInitF) {
        if(param == 0) {
            HuSprGrpPosSet(grpId, 0, 0);
            if(sys->gameType == 0) {
                for(i=0; i<4; i++) {
                    pos3D = *model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
                    Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
                    sprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i+4);
                    numSprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i);
                    sprPos->x = pos2D.x;
                    sprPos->y = pos2D.y+36;
                    numSprPos->x = pos2D.x+16;
                    numSprPos->y = pos2D.y+36;
                    HuSprDispOn(grpId, (int)i);
                    HuSprDispOn(grpId, (int)(i+4));
                }
            } else if(sys->gameType == 1) {
                for(i=0; i<2; i++) {
                    if(i == 0) {
                        pos3D.x = (model->posP[sys->chrSel[sys->teamPlayer[0][0]]+MDMODEL_CAP_CHR_00]->x+model->posP[sys->chrSel[sys->teamPlayer[0][1]]+MDMODEL_CAP_CHR_00]->x)/2;
                        pos3D.y = (model->posP[sys->chrSel[sys->teamPlayer[0][0]]+MDMODEL_CAP_CHR_00]->y+model->posP[sys->chrSel[sys->teamPlayer[0][1]]+MDMODEL_CAP_CHR_00]->y)/2;
                        pos3D.z = (model->posP[sys->chrSel[sys->teamPlayer[0][0]]+MDMODEL_CAP_CHR_00]->z+model->posP[sys->chrSel[sys->teamPlayer[0][1]]+MDMODEL_CAP_CHR_00]->z)/2;
                    } else if(i == 1) {
                        pos3D.x = (model->posP[sys->chrSel[sys->teamPlayer[1][0]]+MDMODEL_CAP_CHR_00]->x+model->posP[sys->chrSel[sys->teamPlayer[1][1]]+MDMODEL_CAP_CHR_00]->x)/2;
                        pos3D.y = (model->posP[sys->chrSel[sys->teamPlayer[1][0]]+MDMODEL_CAP_CHR_00]->y+model->posP[sys->chrSel[sys->teamPlayer[1][1]]+MDMODEL_CAP_CHR_00]->y)/2;
                        pos3D.z = (model->posP[sys->chrSel[sys->teamPlayer[1][0]]+MDMODEL_CAP_CHR_00]->z+model->posP[sys->chrSel[sys->teamPlayer[1][1]]+MDMODEL_CAP_CHR_00]->z)/2;
                    }
                    Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
                    sprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i+4);
                    numSprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i);
                    sprPos->x = pos2D.x;
                    sprPos->y = pos2D.y+32;
                    numSprPos->x = pos2D.x+16;
                    numSprPos->y = pos2D.y+32;
                    HuSprDispOn(grpId, (int)i);
                    HuSprDispOn(grpId, (int)(i+4));
                }
            }
        } else if(param == 1) {
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_HANDICAP, FALSE);
        }
        sys->handicapInitF = TRUE;
    }
    if(param == 2) {
        HuSprGrpPosSet(grpId, 0, 0);
        if(sys->gameType == 0) {
            for(i=0; i<4; i++) {
                pos3D = *model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
                Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i+4);
                numSprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i);
                sprPos->x = pos2D.x;
                sprPos->y = pos2D.y+36;
                numSprPos->x = pos2D.x+16;
                numSprPos->y = pos2D.y+36;
            }
        } else if(sys->gameType == 1) {
            for(i=0; i<2; i++) {
                if(i == 0) {
                    pos3D.x = (model->posP[sys->chrSel[sys->teamPlayer[0][0]]+MDMODEL_CAP_CHR_00]->x+model->posP[sys->chrSel[sys->teamPlayer[0][1]]+MDMODEL_CAP_CHR_00]->x)/2;
                    pos3D.y = (model->posP[sys->chrSel[sys->teamPlayer[0][0]]+MDMODEL_CAP_CHR_00]->y+model->posP[sys->chrSel[sys->teamPlayer[0][1]]+MDMODEL_CAP_CHR_00]->y)/2;
                    pos3D.z = (model->posP[sys->chrSel[sys->teamPlayer[0][0]]+MDMODEL_CAP_CHR_00]->z+model->posP[sys->chrSel[sys->teamPlayer[0][1]]+MDMODEL_CAP_CHR_00]->z)/2;
                } else if(i == 1) {
                    pos3D.x = (model->posP[sys->chrSel[sys->teamPlayer[1][0]]+MDMODEL_CAP_CHR_00]->x+model->posP[sys->chrSel[sys->teamPlayer[1][1]]+MDMODEL_CAP_CHR_00]->x)/2;
                    pos3D.y = (model->posP[sys->chrSel[sys->teamPlayer[1][0]]+MDMODEL_CAP_CHR_00]->y+model->posP[sys->chrSel[sys->teamPlayer[1][1]]+MDMODEL_CAP_CHR_00]->y)/2;
                    pos3D.z = (model->posP[sys->chrSel[sys->teamPlayer[1][0]]+MDMODEL_CAP_CHR_00]->z+model->posP[sys->chrSel[sys->teamPlayer[1][1]]+MDMODEL_CAP_CHR_00]->z)/2;
                }
                Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
                sprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i+4);
                numSprPos = MdSpritePosGet(sprite, MDSPR_GRP_HANDICAP, i);
                sprPos->x = pos2D.x;
                sprPos->y = pos2D.y+32;
                numSprPos->x = pos2D.x+16;
                numSprPos->y = pos2D.y+32;
            }
        }
    }
    return TRUE;
}

BOOL MdPartyProcDispSetup(int param)
{
    MDMODEL_WORK *model; //r31
    int i; //r30
    HuVecF *posP; //r29
    HuVecF *scaleP; //r28
    MDSPRITE_WORK *sprite; //r26
    MDPARTY_CHR *chr; //r25
    
    MDCAMERA_WORK *camera; //sp+0x8
    
    static float bss_144[6]; //bss+0x144
    static float moveTime[4]; //bss+0x134
    static float moveOfsTime[4]; //bss+0x124
    static float moveT[2]; //bss+0x11C
    static s8 moveFlag[4]; //bss+0x118
    static int startTime; //bss+0x114
    
    camera = sys->camera;
    model = sys->model;
    sprite = sys->sprite;
    chr = &sys->chr[0];
    if(!sys->dirtyF) {
        if(param == 0 || param == 1) {
            for(i=0; i<4; i++) {
                moveTime[i] = 0;
                moveOfsTime[i] = 0;
                moveFlag[i] = FALSE;
            }
            startTime = 0;
        } else if(param == 2) {
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_PLAYERNO, FALSE);
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_COMDIF, FALSE);
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_MAP_NAME, FALSE);
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_HANDICAP, FALSE);
            for(i=0; i<4; i++) {
                MdRingDispSet(i, FALSE);
            }
            for(i=0; i<4; i++) {
                Hu3DMotionShiftSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], model->motId[(sys->chrSel[i]*2)+MDMODEL_MOT_MARIO_IDLE], 0, 16, HU3D_MOTATTR_LOOP);
                chr[sys->chrSel[i]].mode = MDPARTY_CHR_SELECT_WAIT;
            }
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 30.0f);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, 100, -160, sys->procT);
        }
        if(sys->gameType == 1) {
            posP = model->posP[MDMODEL_VS];
            MdMathLerp(&posP->y, 75, -120, sys->procT);
        }
        MdPartyPlayerSprUpdate();
        MdProcCall(MDPARTY_PROC_HANDICAP, 2);
        startTime++;
        if(startTime >= 0) {
            moveFlag[0] = TRUE;
        }
        if(startTime >= 2) {
            moveFlag[1] = TRUE;
        }
        if(startTime >= 4) {
            moveFlag[2] = TRUE;
        }
        if(startTime >= 6) {
            moveFlag[3] = TRUE;
        }
        if(moveFlag[0]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[0], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[0], &moveT[1], 20.0f);
            posP = model->posP[sys->gameType+MDMODEL_CAP_BROYAL];
            scaleP = model->scaleP[sys->gameType+MDMODEL_CAP_BROYAL];
            MdMathLerp(&posP->x, -525.0f, -435.0f, moveT[0]);
            MdMathLerp(&posP->y, 365.0f, 330.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, -50.0f, 0.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 0.8f, 1.0f, moveT[0]);
        }
        if(moveFlag[1]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[1], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[1], &moveT[1], 20.0f);
            posP = model->posP[MDMODEL_CAP_TURN];
            scaleP = model->scaleP[MDMODEL_CAP_TURN];
            MdMathLerp(&posP->x, 525.0f, 435.0f, moveT[0]);
            MdMathLerp(&posP->y, 365.0f, 330.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, -50.0f, 0.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 0.8f, 1.0f, moveT[0]);
        }
        if(moveFlag[2]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[2], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[2], &moveT[1], 20.0f);
            posP = model->posP[sys->selPack+MDMODEL_CAP_PACK0];
            scaleP = model->scaleP[sys->selPack+MDMODEL_CAP_PACK0];
            MdMathLerp(&posP->x, -525.0f, -315.0f, moveT[0]);
            MdMathLerp(&posP->y, 180.0f, 125.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, -50.0f, 0.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 0.8f, 1.0f, moveT[0]);
        }
        if(moveFlag[3]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[3], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[3], &moveT[1], 20.0f);
            posP = model->posP[MDMODEL_CAP_BONUS];
            scaleP = model->scaleP[MDMODEL_CAP_BONUS];
            MdMathLerp(&posP->x, 525.0f, 315.0f, moveT[0]);
            MdMathLerp(&posP->y, 180.0f, 125.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, -50.0f, 0.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 0.8f, 1.0f, moveT[0]);
        }
        MdPartyModelBBoardSet(sys->gameType+MDMODEL_CAP_BROYAL, MDCAMERA_MAIN);
        MdPartyModelBBoardSet(sys->selPack+MDMODEL_CAP_PACK0, MDCAMERA_MAIN);
        MdPartyModelBBoardSet(MDMODEL_CAP_TURN, MDCAMERA_MAIN);
        MdPartyModelBBoardSet(MDMODEL_CAP_BONUS, MDCAMERA_MAIN);
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 30.0f);
        for(i=0; i<4; i++) {
            posP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&posP->y, -160, 100, sys->procT);
        }
        if(sys->gameType == 1) {
            posP = model->posP[MDMODEL_VS];
            MdMathLerp(&posP->y, -120, 75, sys->procT);
        }
        MdPartyPlayerSprUpdate();
        MdProcCall(MDPARTY_PROC_HANDICAP, 2);
        startTime++;
        if(startTime >= 6) {
            moveFlag[0] = TRUE;
        }
        if(startTime >= 4) {
            moveFlag[1] = TRUE;
        }
        if(startTime >= 2) {
            moveFlag[2] = TRUE;
        }
        if(startTime >= 0) {
            moveFlag[3] = TRUE;
        }
        if(moveFlag[0]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[0], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[0], &moveT[1], 20.0f);
            posP = model->posP[sys->gameType+MDMODEL_CAP_BROYAL];
            scaleP = model->scaleP[sys->gameType+MDMODEL_CAP_BROYAL];
            MdMathLerp(&posP->x, -435.0f,-525.0f,  moveT[0]);
            MdMathLerp(&posP->y, 330.0f,  365.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, 0.0f, -50.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 1.0f, 0.8f, moveT[0]);
        }
        if(moveFlag[1]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[1], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[1], &moveT[1], 20.0f);
            posP = model->posP[MDMODEL_CAP_TURN];
            scaleP = model->scaleP[MDMODEL_CAP_TURN];
            MdMathLerp(&posP->x, 435.0f, 525.0f, moveT[0]);
            MdMathLerp(&posP->y, 330.0f, 365.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, 0.0f, -50.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 1.0f, 0.8f, moveT[0]);
        }
        if(moveFlag[2]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[2], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[2], &moveT[1], 20.0f);
            posP = model->posP[sys->selPack+MDMODEL_CAP_PACK0];
            scaleP = model->scaleP[sys->selPack+MDMODEL_CAP_PACK0];
            MdMathLerp(&posP->x, -315.0f, -525.0f, moveT[0]);
            MdMathLerp(&posP->y, 125.0f, 180.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, 0.0f, -50.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 1.0f, 0.8f, moveT[0]);
        }
        if(moveFlag[3]) {
            MdMathOscillate(MDMATH_OSC_90, &moveTime[3], &moveT[0], 20.0f);
            MdMathOscillate(MDMATH_OSC_180, &moveOfsTime[3], &moveT[1], 20.0f);
            posP = model->posP[MDMODEL_CAP_BONUS];
            scaleP = model->scaleP[MDMODEL_CAP_BONUS];
            MdMathLerp(&posP->x, 315.0f, 525.0f, moveT[0]);
            MdMathLerp(&posP->y, 125.0f, 180.0f, moveT[0]);
            MdMathLerpOfs(&posP->z, 0.0f, -50.0f, moveT[0], moveT[1]*2);
            MdMathLerpScale(scaleP, 1.0f, 0.8f, moveT[0]);
        }
        MdPartyModelBBoardSet(sys->gameType+MDMODEL_CAP_BROYAL, MDCAMERA_MAIN);
        MdPartyModelBBoardSet(sys->selPack+MDMODEL_CAP_PACK0, MDCAMERA_MAIN);
        MdPartyModelBBoardSet(MDMODEL_CAP_TURN, MDCAMERA_MAIN);
        MdPartyModelBBoardSet(MDMODEL_CAP_BONUS, MDCAMERA_MAIN);
    } else if(param == 2) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 60.0f);
        for(i=0; i<4; i++) {
            scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
            MdMathLerp(&scaleP->x, 1, 0, sys->procT);
            MdMathLerp(&scaleP->y, 1, 0, sys->procT);
            MdMathLerp(&scaleP->z, 0.1f, 0, sys->procT);
        }
        scaleP = model->scaleP[sys->selMapNo+MDMODEL_MAP1];
        MdMathLerpScale(scaleP, 2.0f, 0.0f, sys->procT);
        scaleP = model->scaleP[sys->gameType+MDMODEL_CAP_BROYAL];
        MdMathLerpScale(scaleP, 0.8f, 0.0f, sys->procT);
        scaleP = model->scaleP[MDMODEL_CAP_TURN];
        MdMathLerpScale(scaleP, 0.8f, 0.0f, sys->procT);
        scaleP = model->scaleP[sys->selPack+MDMODEL_CAP_PACK0];
        MdMathLerpScale(scaleP, 0.8f, 0.0f, sys->procT);
        scaleP = model->scaleP[MDMODEL_CAP_BONUS];
        MdMathLerpScale(scaleP, 0.8f, 0.0f, sys->procT);
        if(sys->gameType == 1) {
            scaleP = model->scaleP[MDMODEL_VS];
            MdMathLerpScale(scaleP, 0.8f, 0.0f, sys->procT);
        }
    }
    if(sys->procTime >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcFinger(int param)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    
    HuVecF *posP[4]; //sp+0x2C
    HuVecF *rotP[4]; //sp+0x1C
    HuVecF *scaleP[4]; //sp+0xC
    MDCAMERA_WORK *camera; //sp+0x8
    
    camera = sys->camera;
    model = sys->model;
    for(i=0; i<4; i++) {
        posP[i] = model->posP[i+MDMODEL_FINGER_1P];
        rotP[i] = model->rotP[i+MDMODEL_FINGER_1P];
        scaleP[i] = model->scaleP[i+MDMODEL_FINGER_1P];
    }
    if(!sys->dirtyF) {
        if(param == 0) {
            for(i=0; i<4; i++) {
                if(sys->padEnable[i]) {
                    scaleP[i]->x = 0;
                    scaleP[i]->y = 0;
                    scaleP[i]->z = 0;
                    Hu3DModelDispOn(model->mdlId[i+MDMODEL_FINGER_1P]);
                }
            }
            MdPartyPlayerSprUpdate();
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0){ 
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->procTime, &sys->procT, 15.0f);
        for(i=0; i<4; i++) {
            if(sys->padEnable[i]) {
                MdMathLerpScale(scaleP[i], 0, 4, sys->procT);
            }
        }
    }
    if(param == 0) {
        if(sys->procTime >= 1) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

BOOL MdPartyProcMapEnter(int param)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    HuVecF *posP; //r29
    HuVecF *rotP; //r25
    MDSPRITE_WORK *sprite; //r24
    MDCAMERA_WORK *camera; //r22
    
    MDPARTY_STAR *star; //sp+0x3C
    MDMESS_WORK *mess; //sp+0x38
    HuVecF *scaleP; //sp+0x34
    static int mode; //bss+0x110
    static MDPARTY_EFFECT *effectP[4][2]; //bss+0xF0
    static HuVecF effectPos[4][2]; //bss+0x90
    static HuVecF effectRot[4][2]; //bss+0x30
    static int enterTime; //bss+0x2C
    
    camera = sys->camera;
    model = sys->model;
    sprite = sys->sprite;
    star = sys->star;
    mess = sys->mess;
    if(!sys->dirtyF) {
        if(param == 0) {
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_PLAYERNO, FALSE);
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_COMDIF, FALSE);
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_MAP_NAME, FALSE);
            MdSpriteGrpDispSet(sprite, MDSPR_GRP_HANDICAP, FALSE);
            for(i=0; i<4; i++) {
                MdRingDispSet(i, FALSE);
            }
            if(sys->gameType == 1) {
                Hu3DModelDispOff(model->mdlId[MDMODEL_VS]);
            }
            for(i=0; i<4; i++) {
                Hu3DModelDispOff(model->mdlId[sys->chrSel[i]+MDMODEL_CAP_CHR_00]);
            }
            Hu3DModelDispOff(model->mdlId[MDMODEL_STAGE_CHR]);
            for(i=0; i<4; i++) {
                Hu3DModelLayerSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], 4);
                Hu3DModelCameraSet(model->mdlId[sys->chrSel[i]+MDMODEL_MARIO], camera->camera[MDCAMERA_MAIN].bit);
                *model->posP[sys->chrSel[i]+MDMODEL_MARIO] = *model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
                model->posP[sys->chrSel[i]+MDMODEL_MARIO]->z = 0;
            }
            for(i=0; i<4; i++) {
                effectP[i][0] = ParticleDotCreate(sprite->animP[MDSPR_ANIM_EFFECT_DOT], 5, camera->camera[MDCAMERA_MAIN].bit);
                ParticleDotPosSet(effectP[i][0], model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00]);
                effectP[i][1] = ParticleGlowCreate(sprite->animP[MDSPR_ANIM_EFFECT_GLOW], 5, camera->camera[MDCAMERA_MAIN].bit);
                ParticleGlowPosSet(effectP[i][1], model->posP[sys->chrSel[i]+MDMODEL_MARIO]);
            }
            
            if(sys->gameType == 0) {
                for(i=0; i<4; i++) {
                    effectPos[i][0] = *model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                    effectRot[i][0] = *model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                }
            } else if(sys->gameType == 1) {
                for(i=0; i<4; i++) {
                    effectPos[sys->teamOrder[i]][0] = *model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                    effectRot[sys->teamOrder[i]][0] = *model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                }
            }
            effectPos[0][1].x = -300;
            effectPos[0][1].y = 50+effectPos[0][0].y;
            effectPos[0][1].z = 200+effectPos[0][0].z;
            effectPos[1][1].x = -100;
            effectPos[1][1].y = 50+effectPos[1][0].y;
            effectPos[1][1].z = 200+effectPos[1][0].z;
            effectPos[2][1].x = 100;
            effectPos[2][1].y = 50+effectPos[2][0].y;
            effectPos[2][1].z = 200+effectPos[2][0].z;
            effectPos[3][1].x = 300;
            effectPos[3][1].y = 50+effectPos[3][0].y;
            effectPos[3][1].z = 200+effectPos[3][0].z;
            
            effectRot[0][1].x = 0;
            effectRot[0][1].y = 540;
            effectRot[0][1].z = 0;
            effectRot[1][1].x = 0;
            effectRot[1][1].y = 540;
            effectRot[1][1].z = 0;
            effectRot[2][1].x = 0;
            effectRot[2][1].y = -540;
            effectRot[2][1].z = 0;
            effectRot[3][1].x = 0;
            effectRot[3][1].y = -540;
            effectRot[3][1].z = 0;
            MdPartyAudExec(MDPARTY_AUD_SE_BOARD_START);
            mode = 0;
        }
        sys->dirtyF = TRUE;
    }
    if(param == 0) {
        switch(mode) {
            case 0:
                mode = 1;
                break;
            
            case 1:
                MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 45.0f);
                MdMathOscillate(MDMATH_OSC_180, &sys->procOfsTime, &sys->procOfsT, 45.0f);
                if(sys->gameType == 0) {
                    float ofsTbl[4] = { 8, 4, 3, 6 };
                    for(i=0; i<4; i++) {
                        posP = model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                        rotP = model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                        scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_MARIO];
                        MdMathLerpOfs(&posP->x, effectPos[i][0].x, effectPos[i][1].x, sys->procT, sys->procOfsT*ofsTbl[i]*0.2f);
                        MdMathLerpOfs(&posP->y, effectPos[i][0].y, effectPos[i][1].y, sys->procT, sys->procOfsT*ofsTbl[i]*0.2f);
                        MdMathLerpOfs(&posP->z, effectPos[i][0].z, effectPos[i][1].z, sys->procT, sys->procOfsT*ofsTbl[i]*0.5f);
                        MdMathLerp(&rotP->y, effectRot[i][0].y, effectRot[i][1].y, sys->procT);
                        ParticleGlowPosSet(effectP[i][1], posP);
                    }
                } else if(sys->gameType == 1) {
                    float ofsTbl[4] = { 4, 2, 1.5f, 3 };
                    for(i=0; i<4; i++) {
                        posP = model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                        rotP = model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                        scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_MARIO];
                        MdMathLerpOfs(&posP->x, effectPos[sys->teamOrder[i]][0].x, effectPos[sys->teamOrder[i]][1].x, sys->procT, sys->procOfsT*ofsTbl[sys->teamOrder[i]]*0.2f);
                        MdMathLerpOfs(&posP->y, effectPos[sys->teamOrder[i]][0].y, effectPos[sys->teamOrder[i]][1].y, sys->procT, sys->procOfsT*ofsTbl[sys->teamOrder[i]]*0.2f);
                        MdMathLerpOfs(&posP->z, effectPos[sys->teamOrder[i]][0].z, effectPos[sys->teamOrder[i]][1].z, sys->procT, sys->procOfsT*ofsTbl[sys->teamOrder[i]]*0.5f);
                        MdMathLerp(&rotP->y, effectRot[sys->teamOrder[i]][0].y, effectRot[sys->teamOrder[i]][1].y, sys->procT);
                        ParticleGlowPosSet(effectP[i][1], posP);
                    }
                }
                if(sys->procTime >= 1) {
                    sys->procTime = 0;
                    sys->procOfsTime = 0;
                    sys->procT = 0;
                    sys->procOfsT = 0;
                    if(sys->gameType == 0) {
                        for(i=0; i<4; i++) {
                            effectPos[i][0] = *model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                            MdMathAngleClamp(&model->rotP[sys->chrSel[i]+MDMODEL_MARIO]->x);
                            MdMathAngleClamp(&model->rotP[sys->chrSel[i]+MDMODEL_MARIO]->y);
                            MdMathAngleClamp(&model->rotP[sys->chrSel[i]+MDMODEL_MARIO]->z);
                            effectRot[i][0] = *model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                        }
                    } else if(sys->gameType == 1) {
                        for(i=0; i<4; i++) {
                            effectPos[sys->teamOrder[i]][0] = *model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                            MdMathAngleClamp(&model->rotP[sys->chrSel[i]+MDMODEL_MARIO]->x);
                            MdMathAngleClamp(&model->rotP[sys->chrSel[i]+MDMODEL_MARIO]->y);
                            MdMathAngleClamp(&model->rotP[sys->chrSel[i]+MDMODEL_MARIO]->z);
                            effectRot[sys->teamOrder[i]][0] = *model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                        }
                    }
                    effectPos[0][1].x = -200;
                    effectPos[0][1].y = 250;
                    effectPos[0][1].z = -1000;
                    effectPos[1][1].x = -75;
                    effectPos[1][1].y = 250;
                    effectPos[1][1].z = -1000;
                    effectPos[2][1].x = 75;
                    effectPos[2][1].y = 250;
                    effectPos[2][1].z = -1000;
                    effectPos[3][1].x = 200;
                    effectPos[3][1].y = 250;
                    effectPos[3][1].z = -1000;
                    
                    effectRot[0][1].x = 90;
                    effectRot[0][1].y = 160;
                    effectRot[0][1].z = -10;
                    effectRot[1][1].x = 90;
                    effectRot[1][1].y = 175;
                    effectRot[1][1].z = -10;
                    effectRot[2][1].x = 90;
                    effectRot[2][1].y = -175;
                    effectRot[2][1].z = 10;
                    effectRot[3][1].x = 90;
                    effectRot[3][1].y = -160;
                    effectRot[3][1].z = 10;
                    MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                    star->motNext = MDPARTY_STAR_MOT_IDLE;
                    enterTime = 0;
                    mode++;
                }
                break;
                
            case 2:
                if(++enterTime >= 0) {
                    MdPartyAudExec(MDPARTY_AUD_SE_BOARD_ENTER1);
                    mode++;
                }
                break;
            
            case 3:
                MdProcCall(MDPARTY_PROC_CAMERA, 0);
                MdMathOscillate(MDMATH_OSC_90, &sys->procTime, &sys->procT, 60.0f);
                if(sys->gameType == 0) {
                    for(i=0; i<4; i++) {
                        posP = model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                        rotP = model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                        scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_MARIO];
                        MdMathLerp(&posP->x, effectPos[i][0].x, effectPos[i][1].x, sys->procT);
                        MdMathLerp(&posP->y, effectPos[i][0].y, effectPos[i][1].y, sys->procT);
                        MdMathLerp(&posP->z, effectPos[i][0].z, effectPos[i][1].z, sys->procT);
                        MdMathLerp(&rotP->x, effectRot[i][0].x, effectRot[i][1].x, sys->procT);
                        MdMathLerp(&rotP->y, effectRot[i][0].y, effectRot[i][1].y, sys->procT);
                        MdMathLerp(&rotP->z, effectRot[i][0].z, effectRot[i][1].z, sys->procT);
                        ParticleGlowPosSet(effectP[i][1], posP);
                    }
                } else if(sys->gameType == 1) {
                    for(i=0; i<4; i++) {
                        posP = model->posP[sys->chrSel[i]+MDMODEL_MARIO];
                        rotP = model->rotP[sys->chrSel[i]+MDMODEL_MARIO];
                        scaleP = model->scaleP[sys->chrSel[i]+MDMODEL_MARIO];
                        MdMathLerp(&posP->x, effectPos[sys->teamOrder[i]][0].x, effectPos[sys->teamOrder[i]][1].x, sys->procT);
                        MdMathLerp(&posP->y, effectPos[sys->teamOrder[i]][0].y, effectPos[sys->teamOrder[i]][1].y, sys->procT);
                        MdMathLerp(&posP->z, effectPos[sys->teamOrder[i]][0].z, effectPos[sys->teamOrder[i]][1].z, sys->procT);
                        MdMathLerp(&rotP->x, effectRot[sys->teamOrder[i]][0].x, effectRot[sys->teamOrder[i]][1].x, sys->procT);
                        MdMathLerp(&rotP->y, effectRot[sys->teamOrder[i]][0].y, effectRot[sys->teamOrder[i]][1].y, sys->procT);
                        MdMathLerp(&rotP->z, effectRot[sys->teamOrder[i]][0].z, effectRot[sys->teamOrder[i]][1].z, sys->procT);
                        ParticleGlowPosSet(effectP[i][1], posP);
                    }
                }
                if(sys->procTime >= 1) {
                    sys->procTime = 0;
                    sys->procOfsTime = 0;
                    sys->procT = 0;
                    sys->procOfsT = 0;
                    mode++;
                }
                break;
        }
        if(mode >= 2) {
            MdProcCall(MDPARTY_PROC_WIN, 1);
        }
    }
    if(param == 0 && mode == 3) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdPartyProcName(int param)
{
    HuVec2f *sprPos; //r31
    MDSPRITE_WORK *sprite; //r30
    HUSPRGRPID grpId; //r29
    
    static HuVecF pos[2] = { { 288, -128, 0 }, { 288, 64, 0 } };
    
    sprite = sys->sprite;
    grpId = sprite->grpId[MDSPR_GRP_NAME];
    if(!sys->nameInitF) {
        if(param == 0) {
            HuSprGrpPosSet(grpId, 0, 0);
            HuSprPosSet(grpId, 0, pos[1].x, pos[1].y);
            HuSprDispOn(grpId, 0);
        } else if(param == 1) {
            
        }
        sys->nameInitF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->nameTime, &sys->nameT, 15);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_NAME, 0);
        MdMathLerp(&sprPos->x, pos[0].x, pos[1].x, sys->nameT);
        MdMathLerp(&sprPos->y, pos[0].y, pos[1].y, sys->nameT);
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->nameTime, &sys->nameT, 15);
        sprPos = MdSpritePosGet(sprite, MDSPR_GRP_NAME, 0);
        MdMathLerp(&sprPos->x, pos[1].x, pos[0].x, sys->nameT);
        MdMathLerp(&sprPos->y, pos[1].y, pos[0].y, sys->nameT);
    }
    if(sys->nameTime >= 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void MdPartyPadSetup(void)
{
    int i; //r31
    u16 stat; //r30
    s8 padNo[4] = { 0, 0, 0, 0 };
    sys->maxPlayerNum = 0;
    for(i=0; i<4; i++) {
        sys->padNoConnect[i] = -1;
    }
    for(i=0; i<4; i++) {
        stat = HuPadStatGet(i);
        if(stat == 0) {
            sys->padNoConnect[sys->maxPlayerNum] = i;
            sys->padConnect[i] = TRUE;
            sys->maxPlayerNum++;
        } else {
            sys->padConnect[i] = FALSE;
        }
    }
    if(sys->padConnect[0] == FALSE) {
        for(i=0; i<4; i++) {
            sys->padNoConnect[i] = -1;
            sys->padConnect[i] = FALSE;
        }
        sys->padNoConnect[0] = 0;
        sys->padConnect[0] = TRUE;
        sys->maxPlayerNum = 1;
    }
    sys->playerNum = sys->maxPlayerNum-1;
}

void MdPartyPadSetupStory(void)
{
    int i; //r31
    s8 padNo[4] = { 0, -1, -1, -1 };
    sys->maxPlayerNum = 0;
    for(i=0; i<4; i++) {
        sys->padNoConnect[i] = -1;
    }
    for(i=0; i<4; i++) {
        if(GwPlayerConf[i].type == GW_TYPE_MAN) {
            padNo[i] = 0;
        } else if(GwPlayerConf[i].type == GW_TYPE_COM) {
            padNo[i] = -1;
        }
    }
    for(i=0; i<4; i++) {
        if(padNo[i] == 0) {
            sys->padNoConnect[sys->maxPlayerNum] = i;
            sys->padConnect[i] = TRUE;
            sys->maxPlayerNum++;
        } else {
            sys->padConnect[i] = FALSE;
        }
    }
    sys->playerNum = sys->maxPlayerNum-1;
}

void MdPartyPlayerSetupInit(void)
{
    MDSPRITE_WORK *sprite = sys->sprite; //r28
    MDMODEL_WORK *model = sys->model; //r29
    HUSPRGRPID grpId = sprite->grpId[MDSPR_GRP_PLAYERNO];
    int comNum = 0; //r30
    s8 padNo[4] = { -1, -1, -1, -1 }; //sp+0x8
    int i; //r31
    for(i=0; i<4; i++) {
        sys->padNoEnable[i] = sys->padNoConnect[i];
        if(i > sys->playerNum) {
            sys->padNoEnable[i] = -1;
        }
    }
    for(i=0; i<3; i++) {
        sys->comPlayerNo[i] = -1;
    }
    for(i=0; i<4; i++) {
        if(sys->padNoEnable[i] != -1) {
            padNo[sys->padNoEnable[i]] = 0;
        }
    }
    for(i=0; i<4; i++) {
        if(padNo[i] == -1) {
            sys->comPlayerNo[comNum] = i;
            comNum++;
            if(comNum >= 3) {
                break;
            }
        }
    }
    for(i=0; i<4; i++) {
        sys->padEnable[i] = FALSE;
    }
    for(i=0; i<4; i++) {
        if(sys->padNoEnable[i] != -1) {
            sys->padEnable[sys->padNoEnable[i]] = TRUE;
        }
    }
    for(i=0; i<4; i++) {
        if(sys->padEnable[i] == TRUE) {
            HuSprBankSet(grpId, (int)i, i);
        } else {
            HuSprBankSet(grpId, (int)i, 4);
        }
    }
    for(i=0; i<4; i++) {
        if(sys->padEnable[i] == TRUE) {
            Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_FINGER_1P+i], i);
        } else {
            Hu3DAnmNoSet(model->animId[MDMODEL_ANIM_FINGER_1P+i], 4);
        }
    }
}

void MdPartyPlayerCfgRead(void)
{
    int i;
    for(i=0; i<4; i++) {
        sys->chrSel[i] = GwPlayerConf[i].charNo;
        sys->comDif[i] = GwPlayerConf[i].dif;
    }
    sys->selComNo = 3-sys->playerNum;
    for(i=0; i<4; i++) {
        sys->chrSelEnd[i] = TRUE;
    }
    sys->selComNo = 3-sys->playerNum;
}

void MdPartyPlayerSprUpdate(void)
{
    int i; //r31
    HuVecF *fingerPosP; //r30
    MDMODEL_WORK *model; //r29
    MDSPRITE_WORK *sprite; //r28
    HuVecF *chrPosP; //r27
    MDCAMERA_WORK *camera; //r26
    HUSPRGRPID playerNoGrpId; //r25
    HUSPRGRPID comDifGrpId; //r24
    
    HuVecF pos3D; //sp+0x14
    HuVecF pos2D; //sp+0x8
    
    camera = sys->camera;
    model = sys->model;
    sprite = sys->sprite;
    playerNoGrpId = sprite->grpId[MDSPR_GRP_PLAYERNO];
    comDifGrpId = sprite->grpId[MDSPR_GRP_COMDIF];
    HuSprGrpPosSet(playerNoGrpId, 0, 0);
    HuSprGrpPosSet(comDifGrpId, 0, 0);
    for(i=0; i<4; i++) {
        fingerPosP = model->posP[i+MDMODEL_FINGER_1P];
        chrPosP = model->posP[sys->chrSel[i]+MDMODEL_CAP_CHR_00];
        *fingerPosP = *chrPosP;
        fingerPosP->x -= 60;
        fingerPosP->y += 105;
        fingerPosP->z += 30;
        pos3D = *chrPosP;
        Hu3D3Dto2D(&pos3D, camera->camera[MDCAMERA_MAIN].bit, &pos2D);
        HuSprPosSet(playerNoGrpId, (int)i, pos2D.x, pos2D.y-18);
        HuSprPosSet(comDifGrpId, (int)i, pos2D.x, pos2D.y-18);
    }
}

BOOL MdPartyChrSelCursorUpdate(int playerNo, BOOL comSelF)
{
    int i; //r31
    int padNo; //r30

    s8 selFlag[10]; //sp+0x8
    if(sys->chrSelEnd[playerNo] == TRUE) {
        return FALSE;
    }
    if(comSelF == FALSE) {
        padNo = playerNo;
    } else if(comSelF == TRUE) {
        padNo = 0;
    }
    if((sys->pad->DStkDown[padNo] & PAD_BUTTON_LEFT) ||
        (sys->pad->DStkDown[padNo] & PAD_BUTTON_RIGHT) ||
        (sys->pad->DStkDown[padNo] & PAD_BUTTON_UP) ||
        (sys->pad->DStkDown[padNo] & PAD_BUTTON_DOWN)) {
            for(i=0; i<10; i++) {
                selFlag[i] = -1;
            }
            for(i=0; i<4; i++) {
                if(comSelF == FALSE) {
                    if(i == playerNo) {
                        continue;
                    }
                    if(sys->padEnable[i] == FALSE) {
                        continue;
                    }
                } else if(comSelF == TRUE) {
                    if(i == playerNo) {
                        continue;
                    }
                    if(sys->chrSelEnd[i] == FALSE) {
                        continue;
                    }
                }
                selFlag[sys->chrSel[i]] = 0;
            }
            if(sys->pad->DStkDown[padNo] & PAD_BUTTON_LEFT) {
                for(i=0; i<4; i++) {
                    if(--sys->chrSel[playerNo] == -1) {
                        sys->chrSel[playerNo] = 9;
                    }
                    if(selFlag[sys->chrSel[playerNo]] == -1) {
                        break;
                    }
                }
            } else if(sys->pad->DStkDown[padNo] & PAD_BUTTON_RIGHT) {
                for(i=0; i<4; i++) {
                    sys->chrSel[playerNo] = ++sys->chrSel[playerNo] %10;
                    if(selFlag[sys->chrSel[playerNo]] == -1) {
                        break;
                    }
                }
            } else if(sys->pad->DStkDown[padNo] & PAD_BUTTON_UP) {
                static s8 fixPos[5][4] = {
                    { 0, 4, 1, 3 },
                    { 1, 0, 2, 4 },
                    { 2, 1, 3, 0 },
                    { 3, 2, 4, 1 },
                    { 4, 3, 0, 2 },
                };
                if(sys->chrSel[playerNo] < 5) {
                    for(i=0; i<4; i++) {
                        if(selFlag[fixPos[sys->chrSel[playerNo]][i]+5] == -1) {
                            sys->chrSel[playerNo] = fixPos[sys->chrSel[playerNo]][i]+5;
                            break;
                        }
                    }
                } else {
                    for(i=0; i<4; i++) {
                        if(selFlag[fixPos[sys->chrSel[playerNo]-5][i]] == -1) {
                            sys->chrSel[playerNo] = fixPos[sys->chrSel[playerNo]-5][i];
                            break;
                        }
                    }
                }
            } else if(sys->pad->DStkDown[padNo] & PAD_BUTTON_DOWN) {
                static s8 fixPos[5][4] = {
                    { 0, 1, 4, 2 },
                    { 1, 2, 0, 3 },
                    { 2, 3, 1, 4 },
                    { 3, 4, 2, 0 },
                    { 4, 0, 3, 1 },
                };
                if(sys->chrSel[playerNo] < 5) {
                    for(i=0; i<4; i++) {
                        if(selFlag[fixPos[sys->chrSel[playerNo]][i]+5] == -1) {
                            sys->chrSel[playerNo] = fixPos[sys->chrSel[playerNo]][i]+5;
                            break;
                        }
                    }
                } else {
                    for(i=0; i<4; i++) {
                        if(selFlag[fixPos[sys->chrSel[playerNo]-5][i]] == -1) {
                            sys->chrSel[playerNo] = fixPos[sys->chrSel[playerNo]-5][i];
                            break;
                        }
                    }
                }
            }
            return TRUE;
        } else {
            return FALSE;
        }
        
}

void MdPartyChrComInit(void)
{
    int i; //r31
    s8 selFlag[10];
    int temp = 0;
    for(i=0; i<10; i++) {
        selFlag[i] = -1;
    }
    for(i=0; i<4; i++) {
        if(sys->chrSelEnd[i] == TRUE) {
            selFlag[sys->chrSel[i]] = 0;
        }
    }
    for(i=0; i<4; i++) {
        if(selFlag[i] == -1) {
            sys->chrSel[sys->comPlayerNo[sys->selComNo]] = i;
            break;
        }
    }
}

void MdPartyTeamPlayerUpdate(void)
{
    int i; //r31
    int j; //r30
    int comNum; //r29
    
    static s8 teamPlayer[3][4] = {
        { 0, 1, 2, 3 },
        { 0, 2, 1, 3 },
        { 0, 3, 1, 2 },
    };
    static s8 teamOrder[3][4] = {
        { 0, 1, 2, 3 },
        { 0, 2, 1, 3 },
        { 0, 2, 3, 1 },
    };
    static s8 teamChr[45][2] = {
        { 0, 1 },
        { 0, 2 },
        { 0, 3 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
        { 0, 9 },
        
        { 1, 2 },
        { 1, 3 },
        { 1, 4 },
        { 1, 5 },
        { 1, 6 },
        { 1, 7 },
        { 1, 8 },
        { 1, 9 },
        
        { 2, 3 },
        { 2, 4 },
        { 2, 5 },
        { 2, 6 },
        { 2, 7 },
        { 2, 8 },
        { 2, 9 },
        
        { 3, 4 },
        { 3, 5 },
        { 3, 6 },
        { 3, 7 },
        { 3, 8 },
        { 3, 9 },
        
        { 4, 5 },
        { 4, 6 },
        { 4, 7 },
        { 4, 8 },
        { 4, 9 },
        
        { 5, 6 },
        { 5, 7 },
        { 5, 8 },
        { 5, 9 },
        
        { 6, 7 },
        { 6, 8 },
        { 6, 9 },
        
        { 7, 8 },
        { 7, 9 },
        
        { 8, 9 },
    };
    
    comNum = 0;
    for(i=0; i<2; i++) {
        sys->teamPlayer[0][i] = teamPlayer[sys->teamOrderNo][i];
        sys->teamPlayer[1][i] = teamPlayer[sys->teamOrderNo][i+2];
    }
    for(i=0; i<4; i++) {
        sys->teamOrder[i] = teamOrder[sys->teamOrderNo][i];
    }
    for(i=0; i<2; i++) {
        for(j=0; j<45; j++) {
            if((sys->chrSel[sys->teamPlayer[i][0]] == teamChr[j][0] &&
                sys->chrSel[sys->teamPlayer[i][1]] == teamChr[j][1]) ||
                (sys->chrSel[sys->teamPlayer[i][1]] == teamChr[j][0] &&
                sys->chrSel[sys->teamPlayer[i][0]] == teamChr[j][1])) {
                    sys->teamNo[i] = j;
                    break;
                }
        }
    }
    
    for(i=0; i<3; i++) {
        sys->teamComPlayerNo[i] = -1;
    }
    for(i=0; i<4; i++) {
        if(!sys->padEnable[teamPlayer[sys->teamOrderNo][i]]) {
            sys->teamComPlayerNo[comNum] = teamPlayer[sys->teamOrderNo][i];
            comNum++;
            if(comNum >= 3) {
                break;
            }
        }
    }
}

void MdPartyTeamOrderBackup(void)
{
    int i;
    for(i=0; i<4; i++) {
        sys->teamOrderBackup[i] = sys->teamOrder[i];
    }
}

void MdPartyModelBBoardSet(int mdlNo, int cameraNo)
{
    MDCAMERA *camera; //r31
    HuVecF *posP; //r30
    MDMODEL_WORK *model; //r29
    HuVecF *rotP; //r28
    MDCAMERA_WORK *cameraWork; //r27
    
    float rotY;
    HuVecF dir;
    
    cameraWork = sys->camera;
    camera = &cameraWork->camera[cameraNo];
    model = sys->model;
    posP = model->posP[mdlNo];
    rotP = model->rotP[mdlNo];
    dir.x = camera->pos.x-posP->x;
    dir.y = camera->pos.y-posP->y;
    dir.z = camera->pos.z-posP->z;
    rotY = HuAtan(dir.x, dir.z);
    rotP->y = rotY;
}

void MdPartySaveInit(void)
{
    int i;
    if(sys->gameType == 0) {
        GWTeamFSet(FALSE);
    } else if(sys->gameType == 1) {
        GWTeamFSet(TRUE);
    }
    if(sys->selBonus == 0) {
        GWBonusStarFSet(TRUE);
    } else if(sys->selBonus == 1) {
        GWBonusStarFSet(FALSE);
    }
    if(sys->selPack == 0) {
        GWMgPackSet(GW_MGPACK_ALL);
    } else if(sys->selPack == 1) {
        GWMgPackSet(GW_MGPACK_EASY);
    } else if(sys->selPack == 2) {
        GWMgPackSet(GW_MGPACK_ACTION);
    } else if(sys->selPack == 3) {
        GWMgPackSet(GW_MGPACK_SKILL);
    } else if(sys->selPack == 4) {
        GWMgPackSet(GW_MGPACK_GOOFY);
    }
    GwCommon.partyMgPack = GwSystem.mgPack;
    GwSystem.turnNo = 1;
    GwSystem.turnMax = (sys->selTurn*5)+10;
    GwSystem.boardNo = sys->selMapNo;
    if(sys->gameType == 0) {
        for(i=0; i<4; i++) {
            GwPlayer[i].handicap = sys->selHandicap[i];
        }
    } else if(sys->gameType == 1) {
        for(i=0; i<4; i++) {
            GwPlayer[i].handicap = 0;
        }
        GwPlayer[sys->teamPlayer[0][0]].handicap = sys->selHandicap[0];
        GwPlayer[sys->teamPlayer[1][0]].handicap = sys->selHandicap[1];
    }
    for(i=0; i<4; i++) {
        if(!sys->padEnable[i]) {
            GwPlayer[i].comF = TRUE;
        } else {
            GwPlayer[i].comF = FALSE;
        }
        GwPlayer[i].charNo = sys->chrSel[i];
        GwPlayer[i].dif = sys->comDif[i];
        if(sys->padNoConnect[i] != -1) {
            GwPlayer[i].padNo = sys->padNoConnect[i];
        } else {
            GwPlayer[i].padNo = -1;
        }
        if(sys->teamOrder[i] <= 1) {
            GwPlayer[i].grp = 0;
        } else {
            GwPlayer[i].grp = 1;
        }
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo = i;
        GwPlayerConf[i].dif = GwPlayer[i].dif;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].grp = GwPlayer[i].grp;
    }
    MBPartySaveInit(GwSystem.boardNo);
    MBPlayerSaveInit(GwSystem.teamF, GwSystem.bonusStarF, GwSystem.mgPack, GwSystem.turnMax, GwPlayer[0].handicap, GwPlayer[1].handicap, GwPlayer[2].handicap, GwPlayer[3].handicap);
}

void MdPartyAudExec(int cmd)
{
    int pan; //r30
    
    static int streamNo; //bss+0x28
    static int seNo; //bss+0x24
    switch(cmd) {
        case MDPARTY_AUD_MUS_START:
            streamNo = HuAudBGMPlay(MSM_STREAM_MODE);
            break;
            
        case MDPARTY_AUD_MUS_STOP:
            HuAudSStreamFadeOut(streamNo, 1000);
            break;
        
        case MDPARTY_AUD_SE_CMN_MOVE:
        case MDPARTY_AUD_SE_CMN_CONFIRM:
        case MDPARTY_AUD_SE_CMN_CHRSEL:
        case MDPARTY_AUD_SE_CMN_CANCEL:
        case MDPARTY_AUD_SE_MENU_MOVE:
        case MDPARTY_AUD_SE_BOARD_START:
        case MDPARTY_AUD_SE_BOARD_ENTER1:
            if(cmd == MDPARTY_AUD_SE_CMN_MOVE) {
                HuAudFXPlay(MSM_SE_CMN_01);
            } else if(cmd == MDPARTY_AUD_SE_CMN_CONFIRM) {
                HuAudFXPlay(MSM_SE_CMN_03);
            } else if(cmd == MDPARTY_AUD_SE_CMN_CHRSEL) {
                HuAudFXPlay(MSM_SE_CMN_02);
            } else if(cmd == MDPARTY_AUD_SE_CMN_CANCEL) {
                HuAudFXPlay(MSM_SE_CMN_04);
            } else if(cmd == MDPARTY_AUD_SE_MENU_MOVE) {
                HuAudFXPlay(MSM_SE_MENU_27);
            } else if(cmd == MDPARTY_AUD_SE_BOARD_START) {
                HuAudFXPlay(MSM_SE_MENU_28);
            } else if(cmd == MDPARTY_AUD_SE_BOARD_ENTER1) {
                seNo = HuAudFXPlay(MSM_SE_MENU_29);
            }
            break;
        
        case MDPARTY_AUD_SE_STAR_ENTER:
        case MDPARTY_AUD_SE_STAR_CONFIRM:
        {
            int pan = MdPartyFXPanGet(MDMODEL_STAR, MDCAMERA_FG);
            if(cmd == MDPARTY_AUD_SE_STAR_ENTER) {
                HuAudFXPlayPan(MSM_SE_GUIDE_49, pan);
            } else if(cmd == MDPARTY_AUD_SE_STAR_CONFIRM) {
                HuAudFXPlayPan(MSM_SE_GUIDE_50, pan);
            }
        }
            
            break;
        
        case MDPARTY_AUD_SE_CHR_MARIO:
        case MDPARTY_AUD_SE_CHR_LUIGI:
        case MDPARTY_AUD_SE_CHR_PEACH:
        case MDPARTY_AUD_SE_CHR_YOSHI:
        case MDPARTY_AUD_SE_CHR_WARIO:
        case MDPARTY_AUD_SE_CHR_DAISY:
        case MDPARTY_AUD_SE_CHR_WALUIGI:
        case MDPARTY_AUD_SE_CHR_KINOPIO:
        case MDPARTY_AUD_SE_CHR_TERESA:
        case MDPARTY_AUD_SE_CHR_MINIKOOPA:
        {
            static int charSe[10] = {
                MSM_SE_VOICE_MARIO+11,
                MSM_SE_VOICE_LUIGI+11,
                MSM_SE_VOICE_PEACH+11,
                MSM_SE_VOICE_YOSHI+11,
                MSM_SE_VOICE_WARIO+11,
                MSM_SE_VOICE_DAISY+11,
                MSM_SE_VOICE_WALUIGI+11,
                MSM_SE_VOICE_KINOPIO+11,
                MSM_SE_VOICE_TERESA+11,
                MSM_SE_VOICE_MINIKOOPAR+11
            };
            int pan = MdPartyFXPanGet((cmd-MDPARTY_AUD_SE_CHR_MARIO)+MDMODEL_CAP_CHR_00, MDCAMERA_MAIN);
            HuAudFXPlayPan(charSe[cmd-MDPARTY_AUD_SE_CHR_MARIO], pan);
        }
            break;
        
        case MDPARTY_AUD_SE_ALL_STOP:
            HuAudAllStop();
            break;
    }
}

int MdPartyFXPanGet(int mdlNo, int cameraNo)
{
    MDCAMERA *camera; //r31
    MDMODEL_WORK *model; //r30
    
    MDCAMERA_WORK *cameraWork; //r29
    HuVecF *posP; //r28
    int result; //r27
    
    HuVecF pos2D;
    
    cameraWork = sys->camera;
    camera = &cameraWork->camera[cameraNo];
    model = sys->model;
    posP = model->posP[mdlNo];
    Hu3D3Dto2D(posP, camera->bit, &pos2D);
    result = MSM_PAN_LEFT+((MSM_PAN_WIDTH/HU_DISP_WIDTH)*pos2D.x);
    return result;
}

void MdPosAdjustMode(void)
{
    HuVecF *pos; //r31
    HuVecF *rot; //r30
    HuVecF *scale; //r29
    MDDEBUGFONT_WORK *debugFont; //r28
    MDMODEL_WORK *model; //r27
    HUSPRGRPID grpId; //r26
    int i; //r25
    u32 attr; //r24
    MDSPRITE_WORK *sprite; //r23
    MDCAMERA_WORK *camera; //r22
    
    HuVecF pos2D; //sp+0x8
    
    static MDMODEL_DEBUG_INFO mdlInfo[MDMODEL_MAX] = {
        { "STAR", MDCAMERA_FG },
        { "STAGE", MDCAMERA_MAIN },
        { "STAGE_CHR", MDCAMERA_FG },
        { "MARIO", MDCAMERA_FG },
        { "LUIGI", MDCAMERA_FG },
        { "PEACH", MDCAMERA_FG },
        { "YOSHI", MDCAMERA_FG },
        { "WARIO", MDCAMERA_FG },
        { "DAISY", MDCAMERA_FG },
        { "WALUIGI", MDCAMERA_FG },
        { "KINOPIO", MDCAMERA_FG },
        { "TERESA", MDCAMERA_FG },
        { "MINIKOOPA", MDCAMERA_FG },
        { "CAP_MAN_1P", MDCAMERA_MAIN },
        { "CAP_MAN_2P", MDCAMERA_MAIN },
        { "CAP_MAN_3P", MDCAMERA_MAIN },
        { "CAP_MAN_4P", MDCAMERA_MAIN },
        { "CAP_COM_1P", MDCAMERA_MAIN },
        { "CAP_COM_2P", MDCAMERA_MAIN },
        { "CAP_COM_3P", MDCAMERA_MAIN },
        { "CAP_COM_4P", MDCAMERA_MAIN },
        { "CAP_BROYAL", MDCAMERA_MAIN },
        { "CAP_TMATCH", MDCAMERA_MAIN },
        { "CAP_CHR_00", MDCAMERA_MAIN },
        { "CAP_CHR_01", MDCAMERA_MAIN },
        { "CAP_CHR_02", MDCAMERA_MAIN },
        { "CAP_CHR_03", MDCAMERA_MAIN },
        { "CAP_CHR_04", MDCAMERA_MAIN },
        { "CAP_CHR_05", MDCAMERA_MAIN },
        { "CAP_CHR_06", MDCAMERA_MAIN },
        { "CAP_CHR_07", MDCAMERA_MAIN },
        { "CAP_CHR_08", MDCAMERA_MAIN },
        { "CAP_CHR_09", MDCAMERA_MAIN },
        { "VS", MDCAMERA_MAIN },
        { "MAP1", MDCAMERA_MAIN },
        { "MAP2", MDCAMERA_MAIN },
        { "MAP3", MDCAMERA_MAIN },
        { "MAP4", MDCAMERA_MAIN },
        { "MAP5", MDCAMERA_MAIN },
        { "MAP6", MDCAMERA_MAIN },
        { "MAP7", MDCAMERA_MAIN },
        { "CAP_PACK0", MDCAMERA_MAIN },
        { "CAP_PACK1", MDCAMERA_MAIN },
        { "CAP_PACK2", MDCAMERA_MAIN },
        { "CAP_PACK3", MDCAMERA_MAIN },
        { "CAP_PACK4", MDCAMERA_MAIN },
        { "CAP_TURN", MDCAMERA_MAIN },
        { "CAP_BONUS", MDCAMERA_MAIN },
        { "FINGER_1P", MDCAMERA_MAIN },
        { "FINGER_2P", MDCAMERA_MAIN },
        { "FINGER_3P", MDCAMERA_MAIN },
        { "FINGER_4P", MDCAMERA_MAIN },
        { "RING_1P", MDCAMERA_MAIN },
        { "RING_2P", MDCAMERA_MAIN },
        { "RING_3P", MDCAMERA_MAIN },
        { "RING_4P", MDCAMERA_MAIN },
        { "FONTBGFILTER", MDCAMERA_FG },
    };
    
    static MDMODEL_DEBUG_INFO *mdlInfoP;
    static HuVec2f sprPos;
    static float sprRot;
    static int menuSel;
    static int mdlSel;
    static int dispTimer;
    static BOOL showF;
    
    debugFont = sys->debugFont;
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    grpId = sprite->grpId[MDSPR_GRP_CURSOR_LR];
    if(debugFont->ctrl == MD_DEBUGFONT_CTRL_MODEL) {
        if(showF == FALSE) {
            if(HuPadBtn[0] & PAD_BUTTON_X) {
                if(++dispTimer >= 60) {
                    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_LR, TRUE);
                    dispTimer = 0;
                    showF = TRUE;
                }
            } else {
                dispTimer = 0;
            }
        } else if(showF == TRUE) {
            while(1) {
                if(HuPadBtn[0] & PAD_BUTTON_X) {
                    if(++dispTimer >= 60) {
                        MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_LR, FALSE);
                        dispTimer = 0;
                        showF = FALSE;
                        break;
                    }
                } else {
                    dispTimer = 0;
                }
                if(debugFont->ctrl == MD_DEBUGFONT_CTRL_MODEL) {
                    if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                        for(i=0; i<MDMODEL_MAX; i++) {
                            if(--mdlSel == -1) {
                                mdlSel = MDMODEL_MAX-1;
                            }
                            attr = Hu3DModelAttrGet(model->mdlId[mdlSel]);
                            if(!(attr & HU3D_ATTR_DISPOFF)) {
                                break;
                            }
                        }
                    } else if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                        for(i=0; i<MDMODEL_MAX; i++) {
                            mdlSel = ++mdlSel % MDMODEL_MAX;
                            attr = Hu3DModelAttrGet(model->mdlId[mdlSel]);
                            if(!(attr & HU3D_ATTR_DISPOFF)) {
                                break;
                            }
                        }
                    }
                    if(HuPadBtnDown[0] & PAD_BUTTON_DOWN) {
                        menuSel = ++menuSel % 3;
                    }
                    mdlInfoP = &mdlInfo[mdlSel];
                    pos = model->posP[mdlSel];
                    rot = model->rotP[mdlSel];
                    scale = model->scaleP[mdlSel];
                    attr = Hu3DModelAttrGet(model->mdlId[mdlSel]);
                    if(menuSel == 0) {
                        if(HuPadTrigL[0] <= 75) {
                            if(HuPadSubStkX[0] <= -29) {
                                pos->x -= 5;
                            } else if(HuPadSubStkX[0] >= 29) {
                                pos->x += 5;
                            }
                            if(HuPadStkY[0] <= -36) {
                                pos->y -= 5;
                            } else if(HuPadStkY[0] >= 36) {
                                pos->y += 5;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                pos->z += 5;
                            } else if(HuPadSubStkY[0] >= 29) {
                                pos->z -= 5;
                            }
                        } else if(HuPadBtn[0] & PAD_TRIGGER_L) {
                            if(HuPadSubStkX[0] <= -29) {
                                pos->x -= 0.5f;
                            } else if(HuPadSubStkX[0] >= 29) {
                                pos->x += 0.5f;
                            }
                            if(HuPadStkY[0] <= -36) {
                                pos->y -= 0.5f;
                            } else if(HuPadStkY[0] >= 36) {
                                pos->y += 0.5f;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                pos->z += 0.5f;
                            } else if(HuPadSubStkY[0] >= 29) {
                                pos->z -= 0.5f;
                            }
                        } else {
                            if(HuPadSubStkX[0] <= -29) {
                                pos->x -= 1.0f;
                            } else if(HuPadSubStkX[0] >= 29) {
                                pos->x += 1.0f;
                            }
                            if(HuPadStkY[0] <= -36) {
                                pos->y -= 1.0f;
                            } else if(HuPadStkY[0] >= 36) {
                                pos->y += 1.0f;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                pos->z += 1.0f;
                            } else if(HuPadSubStkY[0] >= 29) {
                                pos->z -= 1.0f;
                            }
                        }
                    } else if(menuSel == 1) {
                        if(HuPadTrigL[0] <= 75) {
                            if(HuPadSubStkX[0] <= -29) {
                                rot->x -= 10;
                            } else if(HuPadSubStkX[0] >= 29) {
                                rot->x += 10;
                            }
                            if(HuPadStkY[0] <= -36) {
                                rot->y -= 10;
                            } else if(HuPadStkY[0] >= 36) {
                                rot->y += 10;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                rot->z += 10;
                            } else if(HuPadSubStkY[0] >= 29) {
                                rot->z -= 10;
                            }
                        } else if(HuPadBtn[0] & PAD_TRIGGER_L) {
                            if(HuPadSubStkX[0] <= -29) {
                                rot->x -= 1;
                            } else if(HuPadSubStkX[0] >= 29) {
                                rot->x += 1;
                            }
                            if(HuPadStkY[0] <= -36) {
                                rot->y -= 1;
                            } else if(HuPadStkY[0] >= 36) {
                                rot->y += 1;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                rot->z += 1;
                            } else if(HuPadSubStkY[0] >= 29) {
                                rot->z -= 1;
                            }
                        } else {
                            if(HuPadSubStkX[0] <= -29) {
                                rot->x -= 5;
                            } else if(HuPadSubStkX[0] >= 29) {
                                rot->x += 5;
                            }
                            if(HuPadStkY[0] <= -36) {
                                rot->y -= 5;
                            } else if(HuPadStkY[0] >= 36) {
                                rot->y += 5;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                rot->z += 5;
                            } else if(HuPadSubStkY[0] >= 29) {
                                rot->z -= 5;
                            }
                        }
                    } else if(menuSel == 2) {
                        if(HuPadTrigL[0] <= 75) {
                            if(HuPadSubStkX[0] <= -29) {
                                scale->x -= 0.05f;
                                scale->y -= 0.05f;
                                scale->z -= 0.05f;
                            } else if(HuPadSubStkX[0] >= 29) {
                                scale->x += 0.05f;
                                scale->y += 0.05f;
                                scale->z += 0.05f;
                            }
                        } else if(HuPadBtn[0] & PAD_TRIGGER_L) {
                            if(HuPadSubStkX[0] <= -29) {
                                scale->x -= 0.01f;
                            } else if(HuPadSubStkX[0] >= 29) {
                                scale->x += 0.01f;
                            }
                            if(HuPadStkY[0] <= -36) {
                                scale->y -= 0.01f;
                            } else if(HuPadStkY[0] >= 36) {
                                scale->y += 0.01f;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                scale->z += 0.01f;
                            } else if(HuPadSubStkY[0] >= 29) {
                                scale->z -= 0.01f;
                            }
                        } else {
                            if(HuPadSubStkX[0] <= -29) {
                                scale->x -= 0.1f;
                            } else if(HuPadSubStkX[0] >= 29) {
                                scale->x += 0.1f;
                            }
                            if(HuPadStkY[0] <= -36) {
                                scale->y -= 0.1f;
                            } else if(HuPadStkY[0] >= 36) {
                                scale->y += 0.1f;
                            }
                            if(HuPadSubStkY[0] <= -29) {
                                scale->z += 0.1f;
                            } else if(HuPadSubStkY[0] >= 29) {
                                scale->z -= 0.1f;
                            }
                        }
                    }
                }
                HuSprPosSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_R), 32, 0);
                HuSprPosSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_L), -32, 0);
                sprRot += 2;
                MdMathAngleClamp(&sprRot);
                HuSprGrpZRotSet(grpId, sprRot);
                Hu3D3Dto2D(pos, camera->camera[mdlInfoP->cameraNo].bit, &pos2D);
                HuSprGrpPosSet(grpId, pos2D.x, pos2D.y);
                if(attr & HU3D_ATTR_DISPOFF) {
                    HuSprAttrSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_R), HUSPR_ATTR_ADDCOL);
                    HuSprAttrSet(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_L), HUSPR_ATTR_ADDCOL);
                    HuSprGrpTPLvlSet(grpId, 0.4f);
                } else {
                    HuSprAttrReset(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_R), HUSPR_ATTR_ADDCOL);
                    HuSprAttrReset(grpId, MDSPR_GRP_MEMBERNO(MDSPR_GRP_CURSOR_LR, MDSPR_CURSOR_LR_L), HUSPR_ATTR_ADDCOL);
                    HuSprGrpTPLvlSet(grpId, 1.0f);
                }
                MdDebugFontPrint(debugFont, "POS-ADJUST-MODE(%2d / %2d) -> %s", mdlSel, MDMODEL_MAX, mdlInfoP->name);
                if(debugFont->ctrl == MD_DEBUGFONT_CTRL_MODEL) {
                    if(menuSel == 0) {
                        MdDebugFontPrint(debugFont, "*  POS(%.1f / %.1f / %.1f)", pos->x, pos->y, pos->z);
                    } else {
                        MdDebugFontPrint(debugFont, "   POS(%.1f / %.1f / %.1f)", pos->x, pos->y, pos->z);
                    }
                    if(menuSel == 1) {
                        MdDebugFontPrint(debugFont, "*  ROT(%.1f / %.1f / %.1f)", rot->x, rot->y, rot->z);
                    } else {
                        MdDebugFontPrint(debugFont, "   ROT(%.1f / %.1f / %.1f)", rot->x, rot->y, rot->z);
                    }
                    if(menuSel == 2) {
                        MdDebugFontPrint(debugFont, "*SCALE(%.2f / %.2f / %.2f)", scale->x, scale->y, scale->z);
                    } else {
                        MdDebugFontPrint(debugFont, " SCALE(%.2f / %.2f / %.2f)", scale->x, scale->y, scale->z);
                    }
                }
                HuPrcVSleep();
            }
        }
    }
}


s8 capPlayerInitF;