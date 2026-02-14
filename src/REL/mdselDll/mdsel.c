#include "REL/mdselDll.h"
#include "game/saveload.h"
#include "game/wipe.h"

#include "REL/mdselDll/effect.c"
#include "messnum/mpmode.h"
#include "messnum/syshelp.h"

#define MDSEL_AUD_MUSSTART 0
#define MDSEL_AUD_MUSEND 1
#define MDSEL_AUD_SECMN_MOVE 2
#define MDSEL_AUD_SECMN_ACCEPT 3
#define MDSEL_AUD_SECMN_CANCEL 4
#define MDSEL_AUD_SE_ENTER 5
#define MDSEL_AUD_SE_2 6
#define MDSEL_AUD_SE_3 7
#define MDSEL_AUD_SE_4 8
#define MDSEL_AUD_SE_5 9
#define MDSEL_AUD_SE_6 10
#define MDSEL_AUD_SE_7 11
#define MDSEL_AUD_SECMN_COIN 12
#define MDSEL_AUD_SE_TERU_SPEAK 13
#define MDSEL_AUD_FX_STOP 14

#define MDSEL_STAR_NONE 0
#define MDSEL_STAR_RESET 1
#define MDSEL_STAR_ENTER 2
#define MDSEL_STAR_ENTER_IDLE 3
#define MDSEL_STAR_EXIT_START 4
#define MDSEL_STAR_EXIT 5
#define MDSEL_STAR_EXIT_IDLE 6

#define MDSEL_STAR_MOTNEXT_NONE 0
#define MDSEL_STAR_MOTNEXT_RESET 1
#define MDSEL_STAR_MOTNEXT_TALK 2
#define MDSEL_STAR_MOTNEXT_IDLE 3


#define MDSEL_MODE_MAX 6

#define MdSelStarExpand() MdProcWait(MDSEL_PROC_STAR_UPDATE, 0)
#define MdSelStarRotNext() MdProcWait(MDSEL_PROC_STAR_UPDATE, 1)
#define MdSelStarMoveBottom() MdProcWait(MDSEL_PROC_STAR_UPDATE, 2)
#define MdSelStarCapStart() MdProcWait(MDSEL_PROC_STAR_UPDATE, 3)
#define MdSelStarCapFocus() MdProcWait(MDSEL_PROC_STAR_UPDATE, 4)

#define MdSelReset() MdProcCall(MDSEL_PROC_RESET, 0)
#define MdSelWinOpen() MdProcWait(MDSEL_PROC_WIN_UPDATE, FALSE)
#define MdSelWinClose() MdProcWait(MDSEL_PROC_WIN_UPDATE, TRUE)

#define MdSelSprShow() MdProcWait(MDSEL_PROC_SPR_UPDATE, 0)
#define MdSelSprHide() MdProcWait(MDSEL_PROC_SPR_UPDATE, 1)
#define MdSelSprFlash() MdProcCall(MDSEL_PROC_SPR_UPDATE, 2)

BOOL MdSelBackupCheck(BOOL checkSave);

void MdSelBillboardUpdate(int mdlNo, int cameraNo);
void MdSelAudExec(int seNo);
int MdSelFXPanGet(int mdlNo, int cameraNo);

void MdSelFlowInit(void);
void MdSelFlowMain(void);

BOOL MdSelProcReset(int param);
BOOL MdSelProcCameraLerp(int param);
BOOL MdSelProcWinUpdate(int param);
BOOL MdSelProcStarUpdate(int param);
BOOL MdSelProcSprUpdate(int param);

char *lbl_1_data_144 = "26";

static int MdSelStarMdl[MDSEL_MODE_MAX] = {
    MDMODEL_STAR_MARU,
    MDMODEL_STAR_DARU,
    MDMODEL_STAR_NIRU,
    MDMODEL_STAR_CHORU,
    MDMODEL_STAR_NERU,
    MDMODEL_STAR_HARU
};
static u32 MdSelMes[MDSEL_MODE_MAX] = {
    MESS_MPMODE_MDOMAKE,
    MESS_MPMODE_MDOPTION,
    MESS_MPMODE_MDMINI,
    MESS_MPMODE_MDPARTY,
    MESS_MPMODE_MDSTORY,
    MESS_MPMODE_SDROOM
};

static OMOVL MdSelDll[MDSEL_MODE_MAX] = {
    DLL_mdomakedll,
    DLL_mdoptiondll,
    DLL_mdminidll,
    DLL_mdpartydll,
    DLL_mdstorydll,
    DLL_sdroomdll
};

static unsigned int MdSelDataDir[MDSEL_MODE_MAX] = {
    DATA_mdomake,
    DATA_mdoption,
    DATA_mdmini,
    DATA_mdparty,
    DATA_mdstory,
    DATA_sdroom
};

static int MdSelCapMdl[MDSEL_MODE_MAX] = {
    MDMODEL_CAP_MARU,
    MDMODEL_CAP_DARU,
    MDMODEL_CAP_NIRU,
    MDMODEL_CAP_CHORU,
    MDMODEL_CAP_NERU,
    MDMODEL_CAP_HARU
};

static s16 MdSelSprNo[MDSEL_MODE_MAX] = {
    MDSPR_GRP_MEMBERNO(MDSPR_GRP_MODE, MDSPR_MDOMAKE),
    MDSPR_GRP_MEMBERNO(MDSPR_GRP_MODE, MDSPR_MDOPTION),
    MDSPR_GRP_MEMBERNO(MDSPR_GRP_MODE, MDSPR_MDMINI),
    MDSPR_GRP_MEMBERNO(MDSPR_GRP_MODE, MDSPR_MDPARTY),
    MDSPR_GRP_MEMBERNO(MDSPR_GRP_MODE, MDSPR_MDSTORY),
    MDSPR_GRP_MEMBERNO(MDSPR_GRP_MODE, MDSPR_SDROOM),
};

static BOOL MdSelSprSlideF = TRUE;

MD_FUNC MdFlowTbl[MDSEL_FLOW_MAX] = {
    MdSelFlowInit,
    MdSelFlowMain
};

MD_PROC MdProcTbl[MDSEL_PROC_MAX] = {
    MdSelProcReset,
    MdSelProcCameraLerp,
    MdSelProcWinUpdate,
    MdSelProcStarUpdate,
    MdSelProcSprUpdate
};

SYS sysWork;
SYS *sys;
SYS *sysP;
static MDSEL_EFFECT *MdSelStarGlow[MDSEL_MODE_MAX];

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
    int i;
    static unsigned int freeDirTbl[18] = {
        DATA_board,
        DATA_storymode,
        DATA_w01,
        DATA_w02,
        DATA_w03,
        DATA_w04,
        DATA_w05,
        DATA_w06,
        DATA_w07,
        DATA_w10,
        DATA_w20,
        DATA_w01s,
        DATA_w02s,
        DATA_w03s,
        DATA_w04s,
        DATA_w05s,
        DATA_w06s,
        DATA_w07s,
    };
    
    sys = &sysWork;
    sysP = sys;
    sys->subMode = 0;
    sys->unk4 = 0;
    sys->flowTime = 0;
    sys->flowMode = MDSEL_FLOW_INIT;
    sys->ovlCallMode = MD_OVLCALL_GOTO;
    sys->nextDll = DLL_selmenuDLL;
    sys->exitF = FALSE;
    sys->skipWipeF = FALSE;
    sys->objman = omInitObjMan(512, 8192);
    sys->ovlHis = omOvlHisGet(0);
    sys->evtNo = sys->ovlHis->evtno;
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    _ClearFlag(FLAG_MG_PRACTICE);
    _ClearFlag(FLAG_INST_MGMODE);
    _ClearFlag(FLAG_INST_DECA);
    _ClearFlag(FLAG_INST_NOHISCHG);
    _ClearFlag(FLAGNUM(FLAG_GROUP_COMMON, 17));
    _ClearFlag(FLAG_BOARD_SAVEINIT);
    _ClearFlag(FLAG_MG_CIRCUIT);
    _ClearFlag(FLAG_BOARD_STAR_RESET);
    _ClearFlag(FLAG_MG_PAUSE_OFF);
    _SetFlag(FLAG_MGINST_ON);
    GWSubGameNoSet(0);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GwPlayerConf[i].padNo = i;
        GwPlayerConf[i].grp = i;
    }
    for(i=0; i<18; i++) {
        HuARDirFree(freeDirTbl[i]);
        HuDataDirClose(freeDirTbl[i]);
    }
    if(GwCommon.storyContinue == TRUE) {
        SLBoardLoadStory();
    }
    omGameSysInit(sys->objman);
    sys->obj[MDSEL_OBJ_DEBUGFONT] = omAddObj(sys->objman, 100, 0, 0, MdObjDebugFontCreate);
    sys->obj[MDSEL_OBJ_DEBUGFONT]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDDEBUGFONT_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDSEL_OBJ_DEBUGFONT]->data) {
        OSReport("sys->obj[MDSEL_OBJ_DEBUGFONT]->data = NULL...\n");
    }
    sys->debugFont = sys->obj[MDSEL_OBJ_DEBUGFONT]->data;
    sys->debugFont->createF = FALSE;
    sys->obj[MDSEL_OBJ_PAD] = omAddObj(sys->objman, 101, 0, 0, MdObjPadMain);
    sys->obj[MDSEL_OBJ_PAD]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDPAD_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDSEL_OBJ_PAD]->data) {
        OSReport("sys->obj[MDSEL_OBJ_PAD]->data = NULL...\n");
    }
    sys->pad = sys->obj[MDSEL_OBJ_PAD]->data;
    sys->obj[MDSEL_OBJ_CAMERA] = omAddObj(sys->objman, 102, 0, 0, MdObjCameraCreate);
    sys->obj[MDSEL_OBJ_CAMERA]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDCAMERA_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDSEL_OBJ_CAMERA]->data) {
        OSReport("sys->obj[MDSEL_OBJ_CAMERA]->data = NULL...\n");
    }
    sys->camera = sys->obj[MDSEL_OBJ_CAMERA]->data;
    sys->camera->createF = FALSE;
    sys->camera->debugFont = sys->debugFont;
    sys->obj[MDSEL_OBJ_LIGHT] = omAddObj(sys->objman, 103, 0, 0, MdObjLightCreate);
    sys->obj[MDSEL_OBJ_LIGHT]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDLIGHT_WORK), HU_MEMNUM_OVL);
    if(!sys->obj[MDSEL_OBJ_LIGHT]->data) {
        OSReport("sys->obj[MDSEL_OBJ_LIGHT]->data = NULL...\n");
    }
    sys->light = sys->obj[MDSEL_OBJ_LIGHT]->data;
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
    sys->star = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDSEL_STAR), HU_MEMNUM_OVL);
    if(!sys->star) {
        OSReport("sys->star = NULL...\n");
    }
    sys->mess = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDMESS), HU_MEMNUM_OVL);
    if(!sys->mess) {
        OSReport("sys->mess = NULL...\n");
    }
    MdMessCreate(sys->mess);
    sys->particle = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MDPARTICLE_WORK), HU_MEMNUM_OVL);
    if(!sys->particle) {
        OSReport("sys->particle = NULL...\n");
    }
    MdMathStdCurveInit();
    Hu3DBGColorSet(0, 0, 0);
    Hu3DNoSyncSet(FALSE);
    while(1) {
        if(sys->debugFont->createF == TRUE && sys->camera->createF == TRUE && sys->light->createF == TRUE) {
            break;
        }
        HuPrcVSleep();
    }
    sys->mainProc = HuPrcChildCreate(MdMain, 104, 12288, 0, sys->objman);
}

void MdMain(void)
{
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
    while(1) {
        switch(sys->loadState) {
            case MD_LOAD_STATE_NONE:
                break;

            case MD_LOAD_STATE_LOADING:
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
    MDSEL_STAR *star; //r31
    MDMODEL_WORK *model; //r30
    MDCAMERA_WORK *camera; //r29
    HuVecF *pos; //r28
    HuVecF *scale; //r27
    int i; //r26
    MDMESS *mess; //r25
    HuVecF *rot; //r24
    
    model = sys->model;
    camera = sys->camera;
    mess = sys->mess;
    star = sys->star;
    pos = model->posP[MDMODEL_STAR_TERU];
    rot = model->rotP[MDMODEL_STAR_TERU];
    scale = model->scaleP[MDMODEL_STAR_TERU];
    star->mode = MDSEL_STAR_NONE;
    star->motNext = MDSEL_STAR_MOTNEXT_RESET;
    
    while(1) {
        model = sys->model;
        camera = sys->camera;
        mess = sys->mess;
        star = sys->star;
        pos = model->posP[MDMODEL_STAR_TERU];
        rot = model->rotP[MDMODEL_STAR_TERU];
        scale = model->scaleP[MDMODEL_STAR_TERU];
        switch(star->mode) {
            case MDSEL_STAR_RESET:
                star->time = 0;
                star->timeY = 0;
                star->t = 0;
                star->yt = 0;
                star->pos3D[0].x = 0;
                star->pos3D[0].y = 0;
                star->pos3D[0].z = 0;
                Hu3D3Dto2D(&star->pos3D[0], camera->camera[MDCAMERA_MAIN].bit, &star->pos2D[0]);
                star->pos2D[0].z = 1500;
                star->pos2D[1].x = 64;
                star->pos2D[1].y = 300;
                star->pos2D[1].z = 1000;
                Hu3D2Dto3D(&star->pos2D[0], camera->camera[MDCAMERA_GUIDE].bit, &star->pos3D[0]);
                Hu3D2Dto3D(&star->pos2D[1], camera->camera[MDCAMERA_GUIDE].bit, &star->pos3D[1]);
                pos = &star->pos3D[0];
                scale->x = 0;
                scale->y = 0;
                scale->z = 0;
                Hu3DModelDispOn(model->mdlId[MDMODEL_STAR_TERU]);
                MdSelAudExec(MDSEL_AUD_SE_ENTER);
                star->mode = MDSEL_STAR_ENTER;
                break;
            
            case MDSEL_STAR_ENTER:
                MdMathOscillate(MDMATH_OSC_90, &star->time, &star->t, 30);
                MdMathOscillate(MDMATH_OSC_180, &star->timeY, &star->yt, 30);
                MdMathLerp(&pos->x, star->pos3D[0].x, star->pos3D[1].x, star->t);
                MdMathLerpOfs(&pos->y, star->pos3D[0].y, star->pos3D[1].y, star->t, -1*star->yt);
                MdMathLerp(&pos->z, star->pos3D[0].z, star->pos3D[1].z, star->t);
                MdMathLerpScale(scale, 0, 1, star->t);
                if(star->time >= 1) {
                    star->mode = MDSEL_STAR_ENTER_IDLE;
                }
                break;
            
            case MDSEL_STAR_EXIT_START:
                star->time = 0;
                star->t = 0;
                star->pos2D[0].x = 64;
                star->pos2D[0].y = 300;
                star->pos2D[0].z = 1000;
                star->pos2D[1].x = -100;
                star->pos2D[1].y = 300;
                star->pos2D[1].z = 1000;
                for(i=0; i<2; i++) {
                    Hu3D2Dto3D(&star->pos2D[i], camera->camera[MDCAMERA_GUIDE].bit, &star->pos3D[i]);
                }
                star->mode = MDSEL_STAR_EXIT;
                break;
            
            case MDSEL_STAR_EXIT:
                MdMathOscillate(MDMATH_OSC_90, &star->time, &star->t, 30);
                MdMathLerp(&pos->x, star->pos3D[0].x, star->pos3D[1].x, star->t);
                MdMathLerp(&pos->y, star->pos3D[0].y, star->pos3D[1].y, star->t);
                MdMathLerp(&pos->z, star->pos3D[0].z, star->pos3D[1].z, star->t);
                if(star->time >= 1) {
                    Hu3DModelDispOff(model->mdlId[MDMODEL_STAR_TERU]);
                    star->mode = MDSEL_STAR_EXIT_IDLE;
                }
                break;
            
            case MDSEL_STAR_NONE:
            case MDSEL_STAR_ENTER_IDLE:
            case MDSEL_STAR_EXIT_IDLE:
                break;
        }
        MdSelBillboardUpdate(MDMODEL_STAR_TERU, MDCAMERA_GUIDE);
        switch(star->motNext) {
            case MDSEL_STAR_MOTNEXT_NONE:
                break;
                
            case MDSEL_STAR_MOTNEXT_RESET:
                Hu3DModelAttrSet(model->mdlId[MDMODEL_STAR_TERU], HU3D_MOTATTR_LOOP);
                Hu3DModelAttrReset(model->mdlId[MDMODEL_STAR_TERU], HU3D_MOTATTR_PAUSE);
                Hu3DMotionSet(model->mdlId[MDMODEL_STAR_TERU], model->motId[MDMODEL_MOT_STAR_TERU]);
                star->motNext = MDSEL_STAR_MOTNEXT_NONE;
                break;
            
            case MDSEL_STAR_MOTNEXT_TALK:
                Hu3DMotionShiftSet(model->mdlId[MDMODEL_STAR_TERU], model->motId[MDMODEL_MOT_STAR_TERU_TALK], 0, 32, HU3D_MOTATTR_LOOP);
                star->motNext = MDSEL_STAR_MOTNEXT_NONE;
                break;
            
            case MDSEL_STAR_MOTNEXT_IDLE:
                Hu3DMotionShiftSet(model->mdlId[MDMODEL_STAR_TERU], model->motId[MDMODEL_MOT_STAR_TERU], 0, 32, HU3D_MOTATTR_LOOP);
                star->motNext = MDSEL_STAR_MOTNEXT_NONE;
                break;
        }
        HuPrcVSleep();
    }
}

static void MainWinUpdate(void)
{
    MDMESS *mess = sys->mess;
    while(1) {
        mess = sys->mess;
        MdMessMain(mess, MDMESS_WIN_MAIN);
        HuPrcVSleep();
    }
}

static void HelpWinUpdate(void)
{
    MDMESS *mess = sys->mess;
    while(1) {
        mess = sys->mess;
        MdMessMain(mess, MDMESS_WIN_HELP);
        HuPrcVSleep();
    }
}

void MdInit(void)
{
    MdSpriteCreate(sys->sprite);
    HuPrcVSleep();
    MdModelCreate(sys->model, sys->camera);
    HuPrcVSleep();
    HuDataDirClose(DATA_mdsel);
    HuPrcVSleep();
    if(sys->evtNo == 2) {
        BOOL res = MdSelBackupCheck(TRUE);
        if(res == TRUE) {
            SLSaveModeExec(FALSE);
        }
    } else if(sys->evtNo == 6) {
        SLSaveModeExec(TRUE);
    }
    HuPrcVSleep();
    MdParticleCreate(sys->particle);
    HuPrcVSleep();
    sys->starProc = HuPrcChildCreate(StarUpdate, 106, 12288, 0, sys->objman);
    sys->mainWinProc = HuPrcChildCreate(MainWinUpdate, 107, 4096, 0, sys->objman);
    sys->helpWinProc = HuPrcChildCreate(HelpWinUpdate, 108, 4096, 0, sys->objman);
    sys->loadState = MD_LOAD_STATE_NONE;
    sys->loadAsyncStat = 0;
    sys->loadProc = HuPrcChildCreate(LoadProc, 105, 12288, 0, sys->objman);
    HuPrcVSleep();
}

void MdClose(void)
{
    if(!sys->skipWipeF) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        WipeWait();
    }
    MdSelAudExec(MDSEL_AUD_FX_STOP);
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

void MdSelFlowInit(void)
{
    int i; //r31
    MDMODEL_WORK *model; //r30
    MDSPRITE_WORK *sprite; //r29
    
    MDCAMERA_WORK *camera; //r26
    MDMESS *mess; //sp+0x8
    
    camera = sys->camera;
    sprite = sys->sprite;
    model = sys->model;
    mess = sys->mess;
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_LR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_CURSOR_UDLR, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_WIN, FALSE);
    MdSpriteGrpDispSet(sprite, MDSPR_GRP_MODE, FALSE);
    Hu3DModelDispOn(model->mdlId[MDMODEL_STAGE]);
    for(i=0; i<MDMODEL_STAR_NUM; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_STAR_START]);
    }
    for(i=0; i<MDMODEL_CAP_NUM; i++) {
        Hu3DModelDispOff(model->mdlId[i+MDMODEL_CAP_START]);
    }
    Hu3DModelDispOn(model->mdlId[MDMODEL_PARTICLE]);
    Hu3DModelDispOn(model->mdlId[MDMODEL_FONTBGFILTER]);
    for(i=0; i<MDMODEL_STAR_NUM; i++) {
        model->posP[i+MDMODEL_STAR_START]->x = -300.0f+(i*100.0f);
        model->posP[i+MDMODEL_STAR_START]->y = 0;
        model->posP[i+MDMODEL_STAR_START]->z = 0;
    }
    for(i=0; i<MDSEL_MODE_MAX; i++) {
        MdSelStarGlow[i] = ParticleGlowCreate(sprite->animP[MDSPR_ANIM_EFF_GLOW], 1, camera->camera[MDCAMERA_MAIN].bit);
    }
    MdSelReset();
    HuPrcVSleep();
    MdSelSprSlideF = TRUE;
    sys->subMode = 0;
    sys->flowMode = MDSEL_FLOW_MAIN;
}

void MdSelFlowMain(void)
{
    MDMESS *mess; //r31
    MDSEL_STAR *star; //r30
    int choice; //r29
    MDMODEL_WORK *model; //sp+0x8
    
    model = sys->model;
    mess = sys->mess;
    star = sys->star;
    switch(sys->subMode) {
        case 0:
            MdSelReset();
            sys->selMode = 3;
            sys->subMode = 1;
            break;
        
        case 1:
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
            MdSelAudExec(MDSEL_AUD_MUSSTART);
            WipeWait();
            MdSelStarExpand();
            MdSelReset();
            while(star->mode != MDSEL_STAR_ENTER_IDLE) {
                HuPrcVSleep();
            }
            MdSelReset();
            MdSelWinOpen();
            MdSelReset();
            MdMessWinOpen(mess, MDMESS_WIN_MAIN, MDMESS_WIN_NOSPEAKER, 0, 0, 0, 32, 372, 512, 64, 0);
            MdMessWinOpen(mess, MDMESS_WIN_HELP, MDMESS_WIN_NOSPEAKER, 0, 0, 1, 64, 340, 448, 32, 0);
            MdMessWinSpeedSet(mess, MDMESS_WIN_HELP, 0);
            MdMessWinAttrSet(mess, MDMESS_WIN_MAIN, HUWIN_ATTR_ALIGN_CENTER);
            MdMessWinAttrSet(mess, MDMESS_WIN_HELP, HUWIN_ATTR_ALIGN_CENTER);
            MdMessWinPad1Set(mess, MDMESS_WIN_MAIN);
            star->motNext = MDSEL_STAR_MOTNEXT_TALK;
            if(sys->evtNo == 0) {
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_MPMODE_ENTER, MDMESS_LIST_END);
            } else {
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MESS_MPMODE_RETURN, MDMESS_LIST_END);
            }
            MdSelAudExec(MDSEL_AUD_SE_TERU_SPEAK);
            while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                HuPrcVSleep();
            }
            MdSelStarCapFocus();
            MdSelReset();
            MdSelSprShow();
            MdSelReset();
            MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MdSelMes[sys->selMode], MDMESS_LIST_END);
            MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_LIST_END);
            sys->subMode = 2;
            break;
        
        case 2:
            if((sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) || (sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT)) {
                sys->prevSelMode = sys->selMode;
                if(sys->pad->DStkDown[0] & PAD_BUTTON_RIGHT) {
                    sys->selMode = (++sys->selMode)%MDSEL_MODE_MAX;
                    sys->turnLeft = FALSE;
                } else if(sys->pad->DStkDown[0] & PAD_BUTTON_LEFT) {
                    if(--sys->selMode == -1) {
                        sys->selMode = MDSEL_MODE_MAX-1;
                    }
                    sys->turnLeft = TRUE;
                }
                MdSelAudExec(MDSEL_AUD_SECMN_MOVE);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MdSelMes[sys->selMode], MDMESS_LIST_END);
                MdSelStarRotNext();
                MdSelReset();
            } else if(HuPadBtnDown[0] & PAD_BUTTON_B) {
                MdSelAudExec(MDSEL_AUD_SECMN_CANCEL);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                MdMessWinMesSet(mess, MDMESS_WIN_MAIN, TRUE, MESS_MPMODE_EXIT, MDMESS_LIST_END);
                while(!MdMessWinCheck(mess, MDMESS_WIN_MAIN)) {
                    HuPrcVSleep();
                }
                choice = MdMessWinChoiceGet(mess, MDMESS_WIN_MAIN);
                if(choice == -1 || choice == 1) {
                    MdMessWinMesSet(mess, MDMESS_WIN_MAIN, FALSE, MdSelMes[sys->selMode], MDMESS_LIST_END);
                    MdMessWinMesSet(mess, MDMESS_WIN_HELP, FALSE, MESS_SYSHELP_MENU_SEL, MDMESS_LIST_END);
                } else if(choice == 0) {
                    MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                    star->motNext = MDSEL_STAR_MOTNEXT_IDLE;
                    sys->loadState = MD_LOAD_STATE_LOADING;
                    sys->loadDir = DATA_filesel;
                    MdSelSprHide();
                    MdSelReset();
                    MdSelWinClose();
                    MdSelReset();
                    WipeColorSet(0, 0, 0);
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
                    MdSelAudExec(MDSEL_AUD_MUSEND);
                    WipeWait();
                    while(sys->loadState == MD_LOAD_STATE_LOADING) {
                        HuPrcVSleep();
                    }
                    omOvlHisChg(0, sys->ovlHis->ovl, 0, sys->ovlHis->stat);
                    sys->ovlCallMode = MD_OVLCALL_RETURN;
                    sys->nextDll = DLL_fileseldll;
                    sys->skipWipeF = TRUE;
                    sys->exitF = TRUE;
                }
            } else if(HuPadBtnDown[0] & PAD_BUTTON_A) {
                MdSelAudExec(MDSEL_AUD_SECMN_ACCEPT);
                star->motNext = MDSEL_STAR_MOTNEXT_IDLE;
                MdMessWinHomeClear(mess, MDMESS_WIN_MAIN);
                MdMessWinHomeClear(mess, MDMESS_WIN_HELP);
                sys->loadState = MD_LOAD_STATE_LOADING;
                sys->loadDir = MdSelDataDir[sys->selMode];
                MdSelSprHide();
                MdSelReset();
                MdSelWinClose();
                MdSelReset();
                star->mode = MDSEL_STAR_EXIT_START;
                MdSelStarMoveBottom();
                MdSelReset();
                MdSelStarCapStart();
                WipeColorSet(255, 255, 255);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 90);
                MdSelAudExec(MDSEL_AUD_MUSEND);
                while(WipeCheckEnd()) {
                    MdProcCall(MDSEL_PROC_STAR_UPDATE, 3);
                    HuPrcVSleep();
                }
                while(sys->loadState == MD_LOAD_STATE_LOADING) {
                    HuPrcVSleep();
                }
                MdSelBackupCheck(FALSE);
                omOvlHisChg(0, sys->ovlHis->ovl, sys->selMode+1, sys->ovlHis->stat);
                sys->ovlCallMode = MD_OVLCALL_CALL;
                sys->nextDll = MdSelDll[sys->selMode];
                sys->skipWipeF = TRUE;
                sys->exitF = TRUE;
            }
            break;
    }
}

BOOL MdSelProcReset(int param)
{
    sys->starMoveF = FALSE;
    sys->unk7C = 0;
    sys->mdSprMoveF = FALSE;
    sys->unk84 = FALSE;
    sys->time = 0;
    sys->ofsTime = 0;
    sys->unk98 = 0;
    sys->unk9C = 0;
    sys->sprTime = 0;
    sys->t = 0;
    sys->ofsT = 0;
    sys->unkAC = 0;
    sys->unkB0 = 0;
    sys->sprT = 0;
    return FALSE;
}

typedef struct MdSelLerpData_s {
    HuVecF hvrA;
    HuVecF centerA;
    HuVecF hvrB;
    HuVecF centerB;
    int type;
    float step;
    float ofs;
} MDSEL_LERP_DATA;

BOOL MdSelProcCameraLerp(int param)
{
    MDSEL_LERP_DATA *lerp;
    MDCAMERA_WORK *cameraWork;
    MDCAMERA *camera;
    
    static MDSEL_LERP_DATA lerpData[4] = {
        {
            { -90, -12, 2500 },
            { 0, 400, 1750 },
            { -90, 0, 2500 },
            { 0, 200, -250 },
            MDMATH_OSC_90_REV,
            30,
            0.5f
        },
        {
            { -90, 0, 2500 },
            { 0, 200, -250 },
            { -90, 0, 2500 },
            { 0, 200, 0 },
            MDMATH_OSC_90,
            30,
            0
        },
        {
            { 90, 0, 2500 },
            { 0, 200, 0 },
            { 90, 0, 2500 },
            { 0, 260, 700 },
            MDMATH_OSC_90_REV,
            64,
            0
        },
        {
            { -90, 0, 2500 },
            { 0, 200, 0 },
            { -90, 0, 2500 },
            { 0, 200, -250 },
            MDMATH_OSC_90_REV,
            30,
            0
        }
    };
    cameraWork = sys->camera;
    camera = &cameraWork->camera[MDCAMERA_MAIN];
    lerp = &lerpData[param];
    MdMathOscillate(lerp->type, &sys->time, &sys->t, lerp->step);
    MdMathOscillate(MDMATH_OSC_180, &sys->ofsTime, &sys->ofsT, lerp->step);
    MdMathLerp(&camera->hvr.x, lerp->hvrA.x, lerp->hvrB.x, sys->t);
    MdMathLerp(&camera->hvr.y, lerp->hvrA.y, lerp->hvrB.y, sys->t);
    MdMathLerp(&camera->hvr.z, lerp->hvrA.z, lerp->hvrB.z, sys->t);
    MdMathLerp(&camera->center.x, lerp->centerA.x, lerp->centerB.x, sys->t);
    MdMathLerp(&camera->center.y, lerp->centerA.y, lerp->centerB.y, sys->t);
    MdMathLerpOfs(&camera->center.z, lerp->centerA.z, lerp->centerB.z, sys->t, sys->ofsT*lerp->ofs);
    if(sys->time >= 1.0f) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL MdSelProcWinUpdate(int param)
{
    MDSPRITE_WORK *sprite; //r31
    HuVec2F *bgScale; //r30
    HuVec2F *topPos; //r29
    HuVec2F *bottomPos; //r28
    HuVec2F *topScale; //r27
    HuVec2F *bottomScale; //r26
    
    HuVec2F *bgPos; //sp+0xC
    HUSPRGRPID grpId; //sp+0x8
    
    static int updateMode;
    static float openY;
    static float closeY;
    static float bgScaleY;
    
    sprite = sys->sprite;
    grpId = sprite->grpId[MDSPR_GRP_WIN];
    topPos = MdSpritePosGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_TOP));
    bottomPos = MdSpritePosGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_BOTTOM));
    bgPos = MdSpritePosGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_BG));
    topScale = MdSpriteScaleGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_TOP));
    bottomScale = MdSpriteScaleGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_FRAME_BOTTOM));
    bgScale = MdSpriteScaleGet(sprite, MDSPR_GRP_WIN, MDSPR_GRP_MEMBERNO(MDSPR_GRP_WIN, MDSPR_WIN_BG));
    if(!sys->starMoveF) {
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
        sys->starMoveF = TRUE;
    }
    if(param == FALSE) {
        if(updateMode == 0) {
            MdMathOscillate(MDMATH_OSC_90, &sys->time, &sys->t, 12);
            MdMathLerp(&topPos->y, 0, -2, sys->t);
            MdMathLerp(&bottomPos->y, 0, 2, sys->t);
            MdMathLerp(&topScale->x, 0, 72, sys->t);
            MdMathLerp(&bottomScale->x, 0, 72, sys->t);
            MdMathLerp(&bgScale->x, 0, 1, sys->t);
            if(sys->time >= 1) {
                sys->time = 0;
                updateMode = 1;
            }
        } else if(updateMode == 1) {
            MdMathOscillate(MDMATH_OSC_90, &sys->time, &sys->t, 15);
            MdMathLerp(&topPos->y, -2, -28, sys->t);
            MdMathLerp(&bottomPos->y, 2, 32, sys->t);
            MdMathLerp(&bgScale->y, 0, 8, sys->t);
            if(sys->time >= 1) {
                updateMode = 2;
            }
        }
    } else if(param == TRUE) {
        if(updateMode == 0) {
            MdMathOscillate(MDMATH_OSC_90, &sys->time, &sys->t, 15);
            MdMathLerp(&topPos->y, openY, -2, sys->t);
            MdMathLerp(&bottomPos->y, closeY, 2, sys->t);
            MdMathLerp(&bgScale->y, bgScaleY, 0, sys->t);
            if(sys->time >= 1) {
                sys->time = 0;
                updateMode = 1;
            }
            
        } else if(updateMode == 1) {
            MdMathOscillate(MDMATH_OSC_90, &sys->time, &sys->t, 12);
            MdMathLerp(&topPos->y, -2, 0, sys->t);
            MdMathLerp(&bottomPos->y, 2, 0, sys->t);
            MdMathLerp(&topScale->x, 72, 0, sys->t);
            MdMathLerp(&bottomScale->x, 72, 0, sys->t);
            MdMathLerp(&bgScale->x, 1, 0, sys->t);
            if(sys->time >= 1) {
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

typedef struct StarModeWork_s {
    s16 mode;
    s16 time;
    float radius;
    float angle;
    float startAngle;
    float endAngle;
    HuVecF startCircle;
    HuVecF endCircle;
} STAR_MODE_WORK;

BOOL MdSelProcStarUpdate(int param)
{
    static MDSEL_LERP_DATA cameraLerp = {
        { 90, 14, 1620 },
        { 0, -120, 0 },
        { 90, 0, 150 },
        { 0, 0, 0 },
        MDMATH_OSC_90_REV,
        120,
        0
    };
    static HuVecF modeCircleParam[MDSEL_MODE_MAX] = {
        { 30, 60, 120 },
        { 30, 55, 110 },
        { 30, 50, 100 },
        { 30, 45, 90 },
        { 30, 40, 80 },
        { 30, 35, 70 },
    };
    static STAR_MODE_WORK modeStar[MDSEL_MODE_MAX];
    static HuVecF pos3D[2];
    static HuVecF pos2D;
    static float rotYSpeed;
    static float cameraTime;
    static float cameraOfsTime;
    static float cameraT;
    static float cameraOfsT;
    static s8 starStopF;
    static HuVecF particlePos;
    
    STAR_MODE_WORK *starP; //r31
    MDMODEL_WORK *model; //r30
    int i; //r29
    HuVecF *pos; //r28
    MDSEL_LERP_DATA *lerpP; //r27
    HuVecF *rot; //r26
    HuVecF *scale; //r25
    MDCAMERA *camera; //r23
    MDCAMERA_WORK *cameraWork; //r20
    
    float motTime; //f31
    
    
    MDMESS *mess; //sp+0x10
    BOOL spC;
    float motSpeed; //sp+0x8
    
    cameraWork = sys->camera;
    camera = &cameraWork->camera[MDCAMERA_MAIN];
    model = sys->model;
    mess = sys->mess;
    if(!sys->starMoveF) {
        if(param == 0) {
            for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
                starP->mode = 0;
                starP->time = (i*15);
                starP->startCircle.x = 0;
                starP->startCircle.y = 0;
                starP->startCircle.z = 0;
                starP->endCircle.x = 0;
                starP->endCircle.y = 0;
                starP->endCircle.z = 0;
                starP->radius = 0;
                starP->angle = -90;
                starP->startAngle = starP->angle;
                starP->endAngle = starP->angle+(60.0f*(6-i));
                pos = model->posP[MdSelStarMdl[i]];
                rot = model->rotP[MdSelStarMdl[i]];
                scale = model->scaleP[MdSelStarMdl[i]];
                pos->x = 0;
                pos->y = 0;
                pos->z = 0;
                rot->x = 0;
                rot->y = 0;
                rot->z = 0;
                scale->x = 0;
                scale->y = 0;
                scale->z = 0;
                Hu3DModelDispOn(model->mdlId[MdSelStarMdl[i]]);
            }
            starStopF = FALSE;
        } else if(param == 1) {
            for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
                starP->mode = 0;
                starP->startCircle.x = 0;
                starP->startCircle.y = 0;
                starP->endCircle.x = 0;
                starP->endCircle.y = 0;
                starP->startAngle = starP->angle;
                if(sys->turnLeft == FALSE) {
                    starP->endAngle = starP->angle+60;
                } else if(sys->turnLeft == TRUE) {
                    starP->endAngle = starP->angle-60;
                }
            }
        } else if(param == 2) {
            for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
                starP->mode = 0;
                starP->startCircle.x = 0;
                starP->startCircle.y = 0;
                starP->startCircle.z = 0;
                starP->endCircle.x = 0;
                starP->endCircle.y = 0;
                starP->endCircle.z = 0;
            }
            pos = model->posP[MdSelStarMdl[sys->selMode]];
            pos2D.x = 288;
            pos2D.y = 336;
            pos2D.z = 900;
            pos3D[0] = *pos;
            Hu3D2Dto3D(&pos2D, camera->bit, &pos3D[1]);
            rotYSpeed = 0;
            cameraTime = 0;
            cameraOfsTime = 0;
            cameraT = 0;
            cameraOfsT = 0;
            MdSelAudExec(MDSEL_AUD_SE_2);
            MdSelAudExec(MDSEL_AUD_SE_3);
        } else if(param == 3) {
            starP = &modeStar[sys->selMode];
            starP->mode = 0;
            starP->time = 0;
            starP->startCircle.x = 0;
            starP->startCircle.y = 0;
            starP->startCircle.z = 0;
            starP->endCircle.x = 0;
            starP->endCircle.y = 0;
            starP->endCircle.z = 0;
            cameraTime = 0;
            cameraOfsTime = 0;
            cameraT = 0;
            cameraOfsT = 0;
        } else if(param == 4) {
            for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
                starP->mode = 0;
                starP->time = 0;
                starP->startCircle.x = 0;
                starP->startCircle.y = 0;
                starP->startCircle.z = 0;
                starP->endCircle.x = 0;
                starP->endCircle.y = 0;
                starP->endCircle.z = 0;
            }
        }
        sys->starMoveF = TRUE;
    }
    if(param == 0) {
        for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
            pos = model->posP[MdSelStarMdl[i]];
            rot = model->rotP[MdSelStarMdl[i]];
            scale = model->scaleP[MdSelStarMdl[i]];
            switch(starP->mode) {
                case 0:
                    if(--starP->time <= 0) {
                        MdSelAudExec(MDSEL_AUD_SE_ENTER);
                        starP->mode++;
                    }
                    break;
                
                case 1:
                    MdMathOscillate(MDMATH_OSC_90, &starP->startCircle.x, &starP->endCircle.x, modeCircleParam[i].x);
                    MdMathLerpScale(scale, 0, 1, starP->endCircle.x);
                    MdMathOscillate(MDMATH_OSC_90, &starP->startCircle.y, &starP->endCircle.y, modeCircleParam[i].y);
                    MdMathLerp(&starP->radius, 0, 300, starP->endCircle.y);
                    MdMathOscillate(MDMATH_OSC_90, &starP->startCircle.z, &starP->endCircle.z, modeCircleParam[i].z);
                    MdMathLerp(&starP->angle, starP->startAngle, starP->endAngle, starP->endCircle.z);
                    pos->x = starP->radius*HuCos(starP->angle);
                    pos->y = 0;
                    pos->z = starP->radius*HuSin(starP->angle);
                    if(starP->startCircle.z >= 1) {
                        MdMathAngleClamp(&starP->angle);
                        starP->mode++;
                    }
                    if(starP->startCircle.z < 0.8f) {
                        particlePos = *pos;
                        ParticleGlowPosSet(MdSelStarGlow[i], &particlePos);
                    }
                    break;
                
                case 2:
                    break;
            }
        }
        if(starStopF == FALSE && modeStar[0].startCircle.z >= 0.75f) {
            sys->star->mode = MDSEL_STAR_RESET;
            starStopF = TRUE;
        }
    } else if(param == 1) {
        for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
            pos = model->posP[MdSelStarMdl[i]];
            rot = model->rotP[MdSelStarMdl[i]];
            scale = model->scaleP[MdSelStarMdl[i]];
            switch(starP->mode) {
                case 0:
                    MdMathOscillate(MDMATH_OSC_90, &starP->startCircle.x, &starP->endCircle.x, 20);
                    MdMathOscillate(MDMATH_OSC_180, &starP->startCircle.y, &starP->endCircle.y, 20);
                    MdMathLerp(&starP->angle, starP->startAngle, starP->endAngle, starP->endCircle.x);
                    pos->x = starP->radius*HuCos(starP->angle);
                    pos->y = 0;
                    pos->z = starP->radius*HuSin(starP->angle);
                    if(i == sys->selMode) {
                        MdMathLerpScaleOfs(scale, 1, 2, starP->endCircle.x, starP->endCircle.y);
                    } else if(i == sys->prevSelMode) {
                        MdMathLerpScale(scale, 2, 1, starP->endCircle.x);
                    }
                    if(starP->startCircle.x >= 1) {
                        MdMathAngleClamp(&starP->angle);
                        starP->mode++;
                    }
                    break;
                
                case 1:
                    break;
            }
        }
        MdSelSprFlash();
    } else if(param == 2) {
        for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
            pos = model->posP[MdSelStarMdl[i]];
            rot = model->rotP[MdSelStarMdl[i]];
            scale = model->scaleP[MdSelStarMdl[i]];
            if(i == sys->selMode) {
                switch(starP->mode) {
                    case 0:
                        MdMathOscillate(MDMATH_OSC_90_REV, &starP->startCircle.x, &starP->endCircle.x, 60);
                        MdMathLerp(&pos->x, pos3D[0].x, pos3D[1].x, starP->endCircle.x);
                        MdMathLerp(&pos->y, pos3D[0].y, pos3D[1].y, starP->endCircle.x);
                        MdMathLerp(&pos->z, pos3D[0].z, pos3D[1].z, starP->endCircle.x);
                        MdMathOscillate(MDMATH_OSC_90_REV, &starP->startCircle.y, &starP->endCircle.y, 60);
                        MdMathLerp(&rotYSpeed, 0, 8, starP->endCircle.y);
                        rot->y += rotYSpeed;
                        if(starP->startCircle.x >= 1) {
                            starP->mode++;
                        }
                        break;
                    
                    case 1:
                        break;
                }
            } else {
                switch(starP->mode) {
                    case 0:
                        MdMathOscillate(MDMATH_OSC_90_REV, &starP->startCircle.x, &starP->endCircle.x, 60);
                        MdMathLerpScale(scale, 1, 0, starP->endCircle.x);
                        MdMathOscillate(MDMATH_OSC_90_REV, &starP->startCircle.y, &starP->endCircle.y, 60);
                        MdMathLerp(&starP->radius, 300, 1000, starP->endCircle.y);
                        pos->x = starP->radius*HuCos(starP->angle);
                        pos->y = 0;
                        pos->z = starP->radius*HuSin(starP->angle);
                        if(starP->startCircle.x >= 1) {
                            starP->mode++;
                        }
                        particlePos = *pos;
                        ParticleGlowPosSet(MdSelStarGlow[i], &particlePos);
                        break;
                    
                    case 1:
                        break;
                }
            }
        }
    } else if(param == 3) {
        spC = FALSE;
        motSpeed = 0;
        pos = model->posP[MdSelStarMdl[sys->selMode]];
        rot = model->rotP[MdSelStarMdl[sys->selMode]];
        scale = model->scaleP[MdSelStarMdl[sys->selMode]];
        starP = &modeStar[sys->selMode];
        switch(starP->mode) {
            case 0:
                rot->y += rotYSpeed;
                Hu3DModelDispOn(model->mdlId[MDMODEL_CAP_PARTICLE]);
                model->posP[MDMODEL_CAP_PARTICLE]->x = 0;
                model->posP[MDMODEL_CAP_PARTICLE]->y = 0;
                model->posP[MDMODEL_CAP_PARTICLE]->z = 0;
                Hu3DModelAttrSet(model->mdlId[MDMODEL_CAP_PARTICLE], HU3D_MOTATTR_LOOP);
                Hu3DModelAttrReset(model->mdlId[MDMODEL_CAP_PARTICLE], HU3D_MOTATTR_PAUSE);
                Hu3DMotionSpeedSet(model->mdlId[MDMODEL_CAP_PARTICLE], 1);
                Hu3DMotionTimeSet(model->mdlId[MDMODEL_CAP_PARTICLE], 0);
                MdSelAudExec(MDSEL_AUD_SE_4);
                MdSelAudExec(MDSEL_AUD_SE_5);
                starP->startCircle.y = 0;
                starP->endCircle.y = 0;
                starP->mode++;
                break;
            
            case 1:
                MdMathOscillate(MDMATH_OSC_90_REV, &starP->startCircle.y, &starP->endCircle.y, 104);
                MdMathLerp(&rotYSpeed, 8, 0, starP->endCircle.y);
                rot->y += rotYSpeed;
                if(rot->y >= 720) {
                    rot->y = 720;
                }
                MdMathOscillate(MDMATH_OSC_90_REV, &sys->time, &sys->t, 120);
                MdMathLerp(&motSpeed, 1, 2.5f, sys->t);
                Hu3DMotionSpeedSet(model->mdlId[MDMODEL_CAP_PARTICLE], motSpeed);
                if(sys->time >= 1) {
                    Hu3DModelDispOff(model->mdlId[MDMODEL_CAP_PARTICLE]);
                    Hu3DModelDispOn(model->mdlId[MdSelCapMdl[sys->selMode]]);
                    model->posP[MdSelCapMdl[sys->selMode]]->x = 0;
                    model->posP[MdSelCapMdl[sys->selMode]]->y = 0;
                    model->posP[MdSelCapMdl[sys->selMode]]->z = 0;
                    Hu3DModelAttrReset(model->mdlId[MdSelCapMdl[sys->selMode]], HU3D_MOTATTR_LOOP);
                    Hu3DModelAttrReset(model->mdlId[MdSelCapMdl[sys->selMode]], HU3D_MOTATTR_PAUSE);
                    Hu3DMotionSpeedSet(model->mdlId[MdSelCapMdl[sys->selMode]], 0.5f);
                    Hu3DMotionTimeSet(model->mdlId[MdSelCapMdl[sys->selMode]], 0);
                    MdSelAudExec(MDSEL_AUD_SE_6);
                    starP->time = 0;
                    starP->mode++;
                }
                
                break;
           
            case 2:
                motTime = Hu3DMotionTimeGet(model->mdlId[MdSelCapMdl[sys->selMode]]);
                if(motTime >= 20.0f) {
                    Hu3DMotionSpeedSet(model->mdlId[MdSelCapMdl[sys->selMode]], 1.5f);
                }
                if(++starP->time >= 30) {
                    MdSelAudExec(MDSEL_AUD_SE_7);
                    starP->mode++;
                }
                break;
           
            case 3:
                motTime = Hu3DMotionTimeGet(model->mdlId[MdSelCapMdl[sys->selMode]]);
                if(motTime >= 20.0f) {
                    Hu3DMotionSpeedSet(model->mdlId[MdSelCapMdl[sys->selMode]], 1.5f);
                }
                lerpP = &cameraLerp;
                MdMathOscillate(MDMATH_OSC_90_REV, &cameraTime, &cameraT, 90);
                MdMathOscillate(MDMATH_OSC_180, &cameraOfsTime, &cameraOfsT, 90);
                MdMathLerp(&camera->hvr.x, lerpP->hvrA.x, lerpP->hvrB.x, cameraT);
                MdMathLerp(&camera->hvr.y, lerpP->hvrA.y, lerpP->hvrB.y, cameraT);
                MdMathLerp(&camera->hvr.z, lerpP->hvrA.z, lerpP->hvrB.z, cameraT);
                MdMathLerp(&camera->center.x, lerpP->centerA.x, lerpP->centerB.x, cameraT);
                MdMathLerpOfs(&camera->center.y, lerpP->centerA.y, lerpP->centerB.y, cameraT, cameraOfsT*0.75f);
                MdMathLerp(&camera->center.z, lerpP->centerA.z, lerpP->centerB.z, cameraT);
                break;
        }
    } else if(param == 4) {
        for(starP=&modeStar[0], i=0; i<MDSEL_MODE_MAX; i++, starP++) {
            if(i == MDSEL_MODE_MAX/2) {
                pos = model->posP[MdSelStarMdl[i]];
                rot = model->rotP[MdSelStarMdl[i]];
                scale = model->scaleP[MdSelStarMdl[i]];
                switch(starP->mode) {
                    case 0:
                        MdMathOscillate(MDMATH_OSC_90, &starP->startCircle.x, &starP->endCircle.x, 20);
                        MdMathOscillate(MDMATH_OSC_180, &starP->startCircle.y, &starP->endCircle.y, 20);
                        MdMathLerpScaleOfs(scale, 1, 2, starP->endCircle.x, starP->endCircle.y);
                        if(starP->startCircle.x >= 1) {
                            starP->mode++;
                        }
                        break;
                    
                    case 1:
                        break;
                }
            }
            
        }
    }
    if(param == 0) {
        if(modeStar[MDSEL_MODE_MAX-1].mode == 2) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else if(param == 1) {
        if(modeStar[MDSEL_MODE_MAX-1].mode == 1) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else if(param == 2) {
        if(modeStar[sys->selMode].mode == 1) {
            for(i=0; i<MDSEL_MODE_MAX; i++) {
                if(i != sys->selMode) {
                    Hu3DModelDispOff(model->mdlId[MdSelStarMdl[i]]);
                }
            }
            return TRUE;
        } else {
            return FALSE;
        }
    } else if(param == 3) {
        if(modeStar[sys->selMode].mode == 3) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else if(param == 4) {
        if(modeStar[MDSEL_MODE_MAX/2].mode == 1) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

BOOL MdSelProcSprUpdate(int param)
{
    HuVec2F *pos; //r31
    int i; //r30
    MDSPRITE_WORK *sprite; //r29
    HUSPRGRPID grpId; //r28
    
    static HuVecF sprPos[2] = {
        { 288, -128, 0 },
        { 288, 64, 0 },
    };
    static int slideMode;
    sprite = sys->sprite;
    grpId = sprite->grpId[MDSPR_GRP_MODE];
    if(sys->mdSprMoveF == FALSE) {
        if(param == 0) {
            HuSprGrpPosSet(grpId, 0, 0);
            for(i=0; i<MDSEL_MODE_MAX; i++) {
                if(i == sys->selMode) {
                    HuSprDispOn(grpId, MdSelSprNo[sys->selMode]);
                } else {
                    HuSprDispOff(grpId, MdSelSprNo[i]);
                }
                HuSprPosSet(grpId, MdSelSprNo[i], sprPos[0].x, sprPos[0].y);
            }
        } else if(param != 1) {
            if(param == 2) {
                if(MdSelSprSlideF == FALSE) {
                    HuSprDispOff(grpId, MdSelSprNo[sys->prevSelMode]);
                    HuSprDispOn(grpId, MdSelSprNo[sys->selMode]);
                    pos = MdSpritePosGet(sprite, MDSPR_GRP_MODE, MdSelSprNo[sys->selMode]);
                    pos->x = sprPos[1].x;
                    pos->y = sprPos[1].y;
                    sys->sprTime = 1;
                } else if(MdSelSprSlideF == TRUE) {
                    slideMode = 0;
                }
            }
        }
        sys->mdSprMoveF = TRUE;
    }
    if(param == 0) {
        MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 15);
        for(i=0; i<MDSEL_MODE_MAX; i++) {
            pos = MdSpritePosGet(sprite, MDSPR_GRP_MODE, MdSelSprNo[i]);
            MdMathLerp(&pos->x, sprPos[0].x, sprPos[1].x, sys->sprT);
            MdMathLerp(&pos->y, sprPos[0].y, sprPos[1].y, sys->sprT);
        }
    } else if(param == 1) {
        MdMathOscillate(MDMATH_OSC_90_REV, &sys->sprTime, &sys->sprT, 15);
        for(i=0; i<MDSEL_MODE_MAX; i++) {
            pos = MdSpritePosGet(sprite, MDSPR_GRP_MODE, MdSelSprNo[i]);
            MdMathLerp(&pos->x, sprPos[1].x, sprPos[0].x, sys->sprT);
            MdMathLerp(&pos->y, sprPos[1].y, sprPos[0].y, sys->sprT);
        }
    } else if(param == 2) {
        if(MdSelSprSlideF && MdSelSprSlideF == TRUE) {
            switch(slideMode) {
                case 0:
                    MdMathOscillate(MDMATH_OSC_90_REV, &sys->sprTime, &sys->sprT, 7);
                    pos = MdSpritePosGet(sprite, MDSPR_GRP_MODE, MdSelSprNo[sys->prevSelMode]);
                    MdMathLerp(&pos->x, sprPos[1].x, sprPos[0].x, sys->sprT);
                    MdMathLerp(&pos->y, sprPos[1].y, sprPos[0].y, sys->sprT);
                    if(sys->sprTime >= 1) {
                        HuSprDispOff(grpId, MdSelSprNo[sys->prevSelMode]);
                        HuSprDispOn(grpId, MdSelSprNo[sys->selMode]);
                        pos = MdSpritePosGet(sprite, MDSPR_GRP_MODE, MdSelSprNo[sys->selMode]);
                        pos->x = sprPos[0].x;
                        pos->y = sprPos[0].y;
                        sys->sprTime = 0;
                        sys->sprT = 0;
                        slideMode = 1;
                    }
                    break;
                
                case 1:
                    MdMathOscillate(MDMATH_OSC_90, &sys->sprTime, &sys->sprT, 7);
                    pos = MdSpritePosGet(sprite, MDSPR_GRP_MODE, MdSelSprNo[sys->selMode]);
                    MdMathLerp(&pos->x, sprPos[0].x, sprPos[1].x, sys->sprT);
                    MdMathLerp(&pos->y, sprPos[0].y, sprPos[1].y, sys->sprT);
                    if(sys->sprTime >= 1) {
                        sys->sprTime = 0;
                        sys->sprT = 0;
                        slideMode = 2;
                    }
                    break;
                
                case 2:
                    break;
            }
        }
    }
    if(param == 0 || param == 1) {
        if(sys->sprTime >= 1) {
            return TRUE;
        }
    } else if(param == 2) {
        if(MdSelSprSlideF == FALSE) {
            if(sys->sprTime >= 1) {
                return TRUE;
            }
        } else if(MdSelSprSlideF == TRUE) {
            if(slideMode == 2) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void MdSelBillboardUpdate(int mdlNo, int cameraNo)
{
    MDCAMERA *camera; //r31
    HuVecF *pos; //r30
    MDMODEL_WORK *model; //r29
    HuVecF *rot; //r28
    MDCAMERA_WORK *cameraWork; //r27
    
    float rotY;
    HuVecF dir;
    
    cameraWork = sys->camera;
    camera = &cameraWork->camera[cameraNo];
    model = sys->model;
    pos = model->posP[mdlNo];
    rot = model->rotP[mdlNo];
    dir.x = camera->pos.x-pos->x;
    dir.y = camera->pos.y-pos->y;
    dir.z = camera->pos.z-pos->z;
    rotY = HuAtan(dir.x, dir.z);
    rot->y = rotY;
}

BOOL MdSelBackupCheck(BOOL checkSave)
{
    int result = 0;
    if(checkSave == FALSE) {
        memcpy(&GwCommonBackup, &GwCommon, sizeof(GwCommon));
    } else if(checkSave == TRUE) {
        result = memcmp(&GwCommonBackup, &GwCommon, sizeof(GwCommon));
        if(result != 0) {
            return TRUE;
        }
    }
    return FALSE;
}

void MdSelAudExec(int cmd)
{
    int pan;
    
    static int streamNo;
    
    switch(cmd) {
        case MDSEL_AUD_MUSSTART:
            streamNo = HuAudBGMPlay(MSM_STREAM_MODESEL);
            break;
        
        case MDSEL_AUD_MUSEND:
            HuAudSStreamFadeOut(streamNo, 1000);
            break;
        
        case MDSEL_AUD_SECMN_MOVE:
        case MDSEL_AUD_SECMN_ACCEPT:
        case MDSEL_AUD_SECMN_CANCEL:
        case MDSEL_AUD_SECMN_COIN:
            if(cmd == MDSEL_AUD_SECMN_MOVE) {
                HuAudFXPlay(MSM_SE_CMN_01);
            } else if(cmd == MDSEL_AUD_SECMN_ACCEPT) {
                HuAudFXPlay(MSM_SE_CMN_03);
            } else if(cmd == MDSEL_AUD_SECMN_CANCEL) {
                HuAudFXPlay(MSM_SE_CMN_04);
            } else if(cmd == MDSEL_AUD_SECMN_COIN) {
                HuAudFXPlay(MSM_SE_CMN_16);
            }
            break;
        
        case MDSEL_AUD_SE_ENTER:
        case MDSEL_AUD_SE_2:
        case MDSEL_AUD_SE_3:
        case MDSEL_AUD_SE_4:
        case MDSEL_AUD_SE_5:
        case MDSEL_AUD_SE_6:
        case MDSEL_AUD_SE_7:
            if(cmd == MDSEL_AUD_SE_ENTER) {
                HuAudFXPlay(MSM_SE_MENU_16);
            } else if(cmd == MDSEL_AUD_SE_2) {
                HuAudFXPlay(MSM_SE_MENU_19);
            } else if(cmd == MDSEL_AUD_SE_3) {
                HuAudFXPlay(MSM_SE_MENU_20);
            } else if(cmd == MDSEL_AUD_SE_4) {
                HuAudFXPlay(MSM_SE_MENU_21);
            } else if(cmd == MDSEL_AUD_SE_5) {
                HuAudFXPlay(MSM_SE_MENU_22);
            } else if(cmd == MDSEL_AUD_SE_6) {
                HuAudFXPlay(MSM_SE_MENU_23);
            } else if(cmd == MDSEL_AUD_SE_7) {
                HuAudFXPlay(MSM_SE_MENU_24);
            }
            
            break;
            
            
        case MDSEL_AUD_SE_TERU_SPEAK:
            pan = MdSelFXPanGet(MDMODEL_STAR_TERU, MDCAMERA_GUIDE);
            if(cmd == MDSEL_AUD_SE_TERU_SPEAK) {
                HuAudFXPlayPan(MSM_SE_GUIDE_56, pan);
            }
            break;
        
        case MDSEL_AUD_FX_STOP:
            HuAudAllStop();
            break;
    }
}

int MdSelFXPanGet(int mdlNo, int cameraNo)
{
    MDCAMERA *camera; //r31
    MDMODEL_WORK *model; //r30
    
    MDCAMERA_WORK *cameraWork; //r29
    HuVecF *pos3D; //r28
    int result; //r27
    
    HuVecF pos2D;
    
    cameraWork = sys->camera;
    camera = &cameraWork->camera[cameraNo];
    model = sys->model;
    pos3D = model->posP[mdlNo];
    Hu3D3Dto2D(pos3D, camera->bit, &pos2D);
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
        { "STAGE", MDCAMERA_MAIN },
        { "STAR_CHORU", MDCAMERA_MAIN },
        { "STAR_NERU", MDCAMERA_MAIN },
        { "STAR_TERU", MDCAMERA_GUIDE },
        { "STAR_NIRU", MDCAMERA_MAIN },
        { "STAR_HARU", MDCAMERA_MAIN },
        { "STAR_MARU", MDCAMERA_MAIN },
        { "STAR_DARU", MDCAMERA_MAIN },
        { "CAP_PARTICLE", MDCAMERA_MAIN },
        { "CAP_CHORU", MDCAMERA_MAIN },
        { "CAP_NERU", MDCAMERA_MAIN },
        { "CAP_NIRU", MDCAMERA_MAIN },
        { "CAP_HARU", MDCAMERA_MAIN },
        { "CAP_MARU", MDCAMERA_MAIN },
        { "CAP_DARU", MDCAMERA_MAIN },
        { "PARTICLE", MDCAMERA_MAIN },
        { "FONTBGFILTER", MDCAMERA_GUIDE },
    };
    
    static MDMODEL_DEBUG_INFO *mdlInfoP;
    static HuVec2F sprPos;
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