#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/printfunc.h"

static void SeqModeInit(s16 mode, s16 frameNo);
static void SeqModeFadeIn(s16 mode, s16 frameNo);
static void SeqModeMain(s16 mode, s16 frameNo);
static void SeqModePreWin(s16 mode, s16 frameNo);
static void SeqModeWin(s16 mode, s16 frameNo);
static void PlayerObjExec(OMOBJ *obj);

static OMOBJMAN *objman;
static HU3DMODELID charMdlId;
static HU3DMOTID charMotId[5];
static s16 charNo;
static s16 padNo;
static s16 jumpNum;

static s16 jumpTimer = -1;
static MGSEQ_PARAM seqParam = {
    10,
    MGSEQ_TIMER_BOTTOM,
    SeqModeInit,
    SeqModeFadeIn,
    NULL,
    SeqModeMain,
    NULL,
    SeqModePreWin,
    SeqModeWin,
    NULL,
    NULL
};

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
    HU3DLIGHTID lightId;
    OSReport("******* Sequence Manager Example ObjectSetup *********\n");
    objman = omInitObjMan(50, 8192);
    omGameSysInit(objman);
    CRot.x = -20;
    CRot.y = 0;
    CRot.z = 0;
    Center.x = 0;
    Center.y = 150;
    Center.z = 0;
    CZoom = 500;
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45, 20, 10000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    omAddObj(objman, 32730, 0, 0, omOutView);
    lightId = Hu3DGLightCreate(0, 1000, 1000, 0, -1, -1, 255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    omAddObj(objman, 32730, 0, 0, PlayerObjExec);
    MgSeqCreate(&seqParam);
}


static void SeqModeInit(s16 mode, s16 frameNo)
{
    static unsigned int motFileTbl[] = {
        CHARMOT_HSF_c000m1_300,
        CHARMOT_HSF_c000m1_303,
        CHARMOT_HSF_c000m1_304,
        CHARMOT_HSF_c000m1_306,
        CHARMOT_HSF_c000m1_307,
        0
    };
    
    padNo = GwPlayerConf[0].padNo;
    charNo = GwPlayerConf[0].charNo;
    charMdlId = CharModelMotListCreate(charNo, CHAR_MODEL0, motFileTbl, charMotId);
    Hu3DModelPosSet(charMdlId, 0, 0, 0);
    CharMotionSet(charNo, charMotId[0]);
    Hu3DModelAttrSet(charMdlId, HU3D_MOTATTR_LOOP);
    MgSeqModeNext();
}

static void SeqModeFadeIn(s16 mode, s16 frameNo)
{
    CZoom = ((1-HuSin((frameNo/60.0)*90))*5000)+500;
    if(frameNo >= 60) {
        MgSeqModeNext();
    }
}

static void SeqModeMain(s16 mode, s16 frameNo)
{
    if(jumpNum >= 10) {
        MgSeqWinnerSet1(0);
        MgSeqRecordSet(10);
        MgSeqModeNext();
    }
}

static void SeqModePreWin(s16 mode, s16 frameNo)
{
    if(frameNo > 60) {
        if(jumpNum < 10) {
            MgSeqModeNext();
        }
        return;
    }
    CRot.x = -20*HuCos((frameNo/60.0)*90);
    Center.y = 150-(50*HuSin((frameNo/60.0)*90));
    CZoom = 500-(HuSin((frameNo/60.0)*90)*200); 
}

static void SeqModeWin(s16 mode, s16 frameNo)
{
    if(frameNo == 0) {
        if(jumpNum >= 10) {
            CharMotionShiftSet(charNo, charMotId[3], 0, 8, HU3D_MOTATTR_NONE);
            HuAudSStreamPlay(MSM_STREAM_TEST);
        } else {
            CharMotionShiftSet(charNo, charMotId[4], 0, 8, HU3D_MOTATTR_NONE);
            HuAudSStreamPlay(MSM_STREAM_MODESEL);
        }
    }
}

static void PlayerObjExec(OMOBJ *obj)
{
    print8(8, 24, 2.0f, "%d", jumpNum);
    if(jumpTimer == -1 && MgSeqModeGet() == MGSEQ_MODE_MAIN && (HuPadBtnDown[padNo] & PAD_BUTTON_A)) {
        jumpNum++;
        jumpTimer = 0;
        CharMotionShiftSet(charNo, charMotId[1], 0, 8, HU3D_MOTATTR_NONE);
    }
    if(jumpTimer >= 0) {
        Hu3DModelPosSet(charMdlId, 0, 150*HuSin((jumpTimer/30.0)*180.0), 0);
        jumpTimer++;
        if(jumpTimer == 28) {
            CharMotionShiftSet(charNo, charMotId[2], 0, 8, HU3D_MOTATTR_NONE);
        } else if(jumpTimer > 30) {
            CharMotionShiftSet(charNo, charMotId[0], 0, 8, HU3D_MOTATTR_NONE);
            jumpTimer = -1;
        }
    }
}