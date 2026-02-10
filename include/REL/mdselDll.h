#ifndef _MDSELDLL_H
#define _MDSELDLL_H

#include "msm_se.h"
#include "msm_stream.h"
#include "msm.h"
#include "game/audio.h"

#include "game/object.h"
#include "game/printfunc.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/pad.h"
#include "game/disp.h"

#define MDMATH_OSC_0 0
#define MDMATH_OSC_90 1
#define MDMATH_OSC_90_REV 2
#define MDMATH_OSC_180 3
#define MDMATH_OSC_360 4
#define MDMATH_OSC_540 5
#define MDMATH_OSC_720 6

#define MD_LOAD_STATE_NONE 0
#define MD_LOAD_STATE_LOADING 1
#define MD_LOAD_STATE_DONE 2

#define MD_OVLCALL_GOTO 0
#define MD_OVLCALL_CALL 1
#define MD_OVLCALL_RETURN 2

#define MDMESS_WIN_SPEAKER 0
#define MDMESS_WIN_NOSPEAKER 1
#define MDMESS_LIST_END -1

#define MDMATH_CURVE_SAMPLE_MAX 8

#define MDLIGHT_MAX 1
#define MDPARTICLE_MAX 768

#define MdProcCall(idx, arg) MdProcTbl[idx](arg)
#define MdProcWait(idx, arg) do { \
    while(!MdProcCall(idx, arg)) { \
        HuPrcVSleep(); \
    } \
} while(0)

#define MDSPR_GRP_MEMBERNO(grp, spr) ((spr)-(grp##_START))

#define MDSPR_GRP_INFO_DEFINE(grp, posX, posY) { grp##_START, grp##_END, posX, posY }
enum {
    MDMATH_CURVE_STD_BOUNCE, //0
    MDMATH_CURVE_STD_LINE, //1
    MDMATH_CURVE_STD_MAX // 2
};

enum {
    MD_DEBUGFONT_CTRL_NOP, //0
    MD_DEBUGFONT_CTRL_CAMERA, //1
    MD_DEBUGFONT_CTRL_LIGHT, //2
    MD_DEBUGFONT_CTRL_MODEL, //3
    MD_DEBUGFONT_CTRL_MAX //4
};

enum {
    MDCAMERA_MAIN, //0
    MDCAMERA_GUIDE, //1
    MDCAMERA_MAX //2
};

enum {
    MDCAMERA_FOCUS_TARGET, // 0
    MDCAMERA_FOCUS_POS, //1
    MDCAMERA_FOCUS_MAX //2
};

enum {
    MDMESS_WIN_MAIN, //0
    MDMESS_WIN_HELP, //1
    MDMESS_WIN_MAX //2
};

enum {
    MDLIGHT_TYPE_NOP, //0
    MDLIGHT_TYPE_G_DIRECT, //1
    MDLIGHT_TYPE_L_DIRECT, //2
    MDLIGHT_TYPE_G_POINT, //3
    MDLIGHT_TYPE_L_POINT, //4
    MDLIGHT_TYPE_G_SPOT, //5
    MDLIGHT_TYPE_L_SPOT, //6
    MDLIGHT_TYPE_MAX
};

typedef void (*MD_FUNC)(void);
typedef BOOL (*MD_PROC)(int param);


enum {
    MDSEL_FLOW_INIT, //0
    MDSEL_FLOW_MAIN, //1
    MDSEL_FLOW_MAX //2
};

enum {
    MDSEL_PROC_RESET, //0
    MDSEL_PROC_CAMERA_LERP, //1
    MDSEL_PROC_WIN_UPDATE, //2
    MDSEL_PROC_STAR_UPDATE, //3
    MDSEL_PROC_SPR_UPDATE, //4
    MDSEL_PROC_MAX //5
};

enum {
    MDSPR_ANIM_WIN_FRAME, //0
    MDSPR_ANIM_WIN_BG, //1
    MDSPR_ANIM_CURSOR, //2
    MDSPR_ANIM_CURSOR_GLOW, //3
    MDSPR_ANIM_EFF_GLOW, //4
    MDSPR_ANIM_MDPARTY, //5
    MDSPR_ANIM_MDSTORY, //6
    MDSPR_ANIM_MDMINI, //7
    MDSPR_ANIM_SDROOM, //8
    MDSPR_ANIM_MDOMAKE, //9
    MDSPR_ANIM_MDOPTION, //10
    MDSPR_ANIM_MAX
};

enum {
    MDSPR_WIN_FRAME_TOP, //0
    MDSPR_WIN_FRAME_BOTTOM, //1
    MDSPR_WIN_BG, //2
    MDSPR_CURSOR_LR_R, //3
    MDSPR_CURSOR_LR_L, //4
    MDSPR_CURSOR_UDLR_R, //5
    MDSPR_CURSOR_UDLR_L, //6
    MDSPR_CURSOR_UDLR_U, //7
    MDSPR_CURSOR_UDLR_D, //8
    MDSPR_CURSOR_GLOW_UDLR_R, //9
    MDSPR_CURSOR_GLOW_UDLR_L, //10
    MDSPR_CURSOR_GLOW_UDLR_U, //11
    MDSPR_CURSOR_GLOW_UDLR_D, //12
    MDSPR_MDPARTY, //13
    MDSPR_MDSTORY, //14
    MDSPR_MDMINI, //15
    MDSPR_SDROOM, //16
    MDSPR_MDOMAKE, //17
    MDSPR_MDOPTION, //18
    MDSPR_MAX //19
};

enum {
    MDSPR_GRP_WIN, //0
    MDSPR_GRP_CURSOR_LR, //1
    MDSPR_GRP_CURSOR_UDLR, //2
    MDSPR_GRP_MODE, //3
    MDSPR_GRP_MAX //4
};

enum {
    MDSPR_GRP_WIN_START = MDSPR_WIN_FRAME_TOP,
    MDSPR_GRP_WIN_END = MDSPR_WIN_BG,
    MDSPR_GRP_CURSOR_LR_START = MDSPR_CURSOR_LR_R,
    MDSPR_GRP_CURSOR_LR_END = MDSPR_CURSOR_LR_L,
    MDSPR_GRP_CURSOR_UDLR_START = MDSPR_CURSOR_UDLR_R,
    MDSPR_GRP_CURSOR_UDLR_END = MDSPR_CURSOR_GLOW_UDLR_D,
    MDSPR_GRP_MODE_START = MDSPR_MDPARTY,
    MDSPR_GRP_MODE_END = MDSPR_MDOPTION,
};

enum {
    MDSEL_OBJ_DEBUGFONT, // 0
    MDSEL_OBJ_PAD, //1
    MDSEL_OBJ_CAMERA, //2
    MDSEL_OBJ_LIGHT, //3
    MDSEL_OBJ_MAX //4
};

enum {
    MDMODEL_STAGE, //0
    
    MDMODEL_STAR_START,
    MDMODEL_STAR_CHORU = MDMODEL_STAR_START, //1
    MDMODEL_STAR_NERU, //2
    MDMODEL_STAR_TERU, //3
    MDMODEL_STAR_NIRU, //4
    MDMODEL_STAR_HARU, //5
    MDMODEL_STAR_MARU, //6
    MDMODEL_STAR_DARU, //7
    MDMODEL_STAR_END,
    MDMODEL_STAR_NUM = MDMODEL_STAR_END-MDMODEL_STAR_START,
    
    MDMODEL_CAP_START = MDMODEL_STAR_END,
    MDMODEL_CAP_PARTICLE = MDMODEL_CAP_START, //8
    MDMODEL_CAP_CHORU, //9
    MDMODEL_CAP_NERU, //10
    MDMODEL_CAP_NIRU, //11
    MDMODEL_CAP_HARU, //12
    MDMODEL_CAP_MARU, //13
    MDMODEL_CAP_DARU, //14
    MDMODEL_CAP_END,
    MDMODEL_CAP_NUM = MDMODEL_CAP_END-MDMODEL_CAP_START,
    
    MDMODEL_PARTICLE = MDMODEL_CAP_END, //15
    MDMODEL_FONTBGFILTER, //16
    MDMODEL_MAX //17
};

enum {
    MDMODEL_MOT_STAR_CHORU, //0
    MDMODEL_MOT_STAR_NERU, //1
    MDMODEL_MOT_STAR_TERU, //2
    MDMODEL_MOT_STAR_TERU_TALK, //3
    MDMODEL_MOT_STAR_NIRU, //4
    MDMODEL_MOT_STAR_HARU, //5
    MDMODEL_MOT_STAR_MARU, //6
    MDMODEL_MOT_STAR_DARU, //7
    MDMODEL_MOT_MAX
};

typedef struct MdDebugFontWork_s {
    BOOL createF; //0x00
    int ctrl; //0x04
    BOOL printF; //0x08
    int color; //0x0C
    s16 posX; //0x10
    s16 posY; //0x12
    s16 ofsY; //0x14
    s16 rowOfsY; //0x16
    s16 rowH; //0x18
    s16 maxLen; //0x1A
    float size; //0x1C
} MDDEBUGFONT_WORK;

typedef struct MdPadWork_s {
    u16 DStkDown[4];
    u16 DStk[4];
} MDPAD_WORK;

typedef struct MdCameraTypeInfo_s {
    int bit;
    int focusType;
} MDCAMERA_TYPE_INFO;

typedef struct MdCameraRect_s {
    float x;
    float y;
    float w;
    float h;
} MDCAMERA_RECT;

typedef struct MdCameraParam_s {
    HuVecF hvr;
    HuVecF center;
} MDCAMERA_PARAM;

typedef struct MdCamera_s {
    u16 bit; //0x00
    int focusType; //0x04
    HuVecF pos; //0x08
    HuVecF target; //0x14
    HuVecF cross; //0x20
    HuVecF up; //0x2C
    HuVecF dir; //0x38
    HuVecF hvr; //0x44
    float unk50;
    HuVecF center; //0x54
    HuVecF unk60[3];
    float fovy; //0x84
} MDCAMERA;

typedef struct MdCameraWork_s {
    BOOL createF; //0x00
    MDDEBUGFONT_WORK *debugFont; //0x04
    MDCAMERA camera[MDCAMERA_MAX]; //0x08
    int selCamera;
} MDCAMERA_WORK;

typedef struct MdLight_s {
    s16 type; //0x00
    HU3DLIGHTID gLightId; //0x02
    HU3DLLIGHTID lLightId; //0x04
    HuVecF hvr; //0x08
    HuVecF center; //0x14
    HuVecF pos; //0x20
    HuVecF target; //0x2C
    HuVecF lightPos; //0x38
    HuVecF lightDir; //0x44
    GXColor rgb; //0x50
} MDLIGHT;

typedef struct MdLightWork_s {
    BOOL createF; //0x00
    int lightSel; //0x04
    MDDEBUGFONT_WORK *debugFont; //0x08
    MDLIGHT light[MDLIGHT_MAX]; //0x0C
} MDLIGHT_WORK;

typedef struct MdSpriteWork_s {
    ANIMDATA *animP[MDSPR_ANIM_MAX]; //0x00
    HUSPRID sprId[MDSPR_MAX]; //0x2C
    HUSPRGRPID grpId[MDSPR_GRP_MAX]; //0x52
} MDSPRITE_WORK;

typedef struct MdModelDebugInfo_s {
    char *name; //0x00
    int cameraNo; //0x04
} MDMODEL_DEBUG_INFO;

typedef struct MdModelWork_s {
    HU3DMODELID mdlId[MDMODEL_MAX]; //0x00
    HU3DMOTID motId[MDMODEL_MOT_MAX]; //0x22
    HuVecF *posP[MDMODEL_MAX]; //0x34
    HuVecF *rotP[MDMODEL_MAX]; //0x78
    HuVecF *scaleP[MDMODEL_MAX]; //0xBC
} MDMODEL_WORK;

typedef struct MdSelStar_s {
    s16 unk0;
    s16 mode; //0x02
    s16 motNext; //0x04
    HuVecF unk8;
    HuVecF pos2D[2]; //0x14
    HuVecF pos3D[2]; //0x2C
    float time; //0x44
    float timeY; //0x48
    float t; //0x4C
    float yt; //0x50
} MDSEL_STAR;

typedef struct MdMessWin_s {
    s16 mode;
    HUWINID winId; //0x02
    s16 choice; //0x04
    s16 unk6;
    s16 type; //0x08
    s16 unkA;
} MDMESS_WIN;

typedef struct MdMess_s {
    MDMESS_WIN win[MDMESS_WIN_MAX];
} MDMESS;

typedef struct MdSprInfo_s {
    s16 animNo; //0x00
    s16 pri; //0x02
    HuVec2F pos; //0x04
    HuVec2F scale; //0xC
    float tpLvl; //0x14
} MDSPR_INFO;

typedef struct MdSprGrpInfo_s {
    s16 start; //0x00
    s16 end; //0x02
    float posX; //0x04
    float posY; //0x08
} MDSPR_GRP_INFO;

typedef struct MdParticleWork_s MDPARTICLE_WORK;
typedef struct MdParticleBuffer_s MDPARTICLE_BUFFER;
typedef void (*MDPARTICLE_UPDATE_HOOK)(MDPARTICLE_BUFFER *spawner);


typedef struct MdParticle_s {
    int mode; //0x00
    int delay; //0x04
    float angle; //0x08
    float radius; //0x0C
    float tpLvl; //0x10
    float radiusSpeed; //0x14
    float angleSpeed; //0x18
    float velY; //0x1C
    float scale; //0x20
    float zRot; //0x24
    HuVecF pos; //0x28
    GXColor color; //0x34
} MDPARTICLE;

typedef struct MdParticleParam_s {
    float baseAlpha; //0x00
    float fadeInSpeed2; //0x04
    float fadeOutSpeed1; //0x08
    float fadeInSpeed1; //0x0C
    float fadeOutSpeed2; //0x10
    float unk14;
    float scaleSpeed1; //0x18
    float scaleSpeed2; //0x1C
    float unk20;
    GXColor color; //0x24
} MDPARTICLE_PARAM;

struct MdParticleBuffer_s {
    BOOL pauseF; //0x00
    ANIMDATA *animP; //0x04
    HU3DMODELID mdlId; //0x08
    s16 num; //0x0A
    MDPARTICLE *particle; //0x0C
    MDPARTICLE_PARAM *param; //0x10
    HU3DPARTICLE *hookWork; //0x14
    HU3DPARTICLEDATA *dataBuf; //0x18
    HU3DPARTICLEHOOK hook; //0x1C
    MDPARTICLE_UPDATE_HOOK updateHook; //0x20
};

struct MdParticleWork_s {
    MDPARTICLE data[MDPARTICLE_MAX]; //0x0
    MDPARTICLE_BUFFER buffer[2]; //0xA800
    MDPARTICLE_PARAM param; //0xA848
};

typedef struct Sys_s {
    int subMode; //0x00
    int unk4;
    int flowTime; //0x08
    int flowMode; //0x0C
    int ovlCallMode; //0x10
    OMOVL nextDll; //0x14
    s8 exitF; //0x18
    s8 skipWipeF; //0x19
    s8 selMode; //0x1A
    s8 prevSelMode; //0x1B
    OMOBJMAN *objman; //0x1C
    HUPROCESS *mainProc; //0x20
    HUPROCESS *loadProc; //0x24
    HUPROCESS *starProc; //0x28
    HUPROCESS *mainWinProc; //0x2C
    HUPROCESS *helpWinProc; //0x30
    OMOBJ *obj[MDSEL_OBJ_MAX]; //0x34
    OMOVLHIS *ovlHis; //0x44
    s32 evtNo; //0x48
    s16 loadState; //0x4C
    s16 loadAsyncStat; //0x4E
    unsigned int loadDir; //0x50
    MDDEBUGFONT_WORK *debugFont; //0x54
    MDPAD_WORK *pad; //0x58
    MDCAMERA_WORK *camera; //0x5C
    MDLIGHT_WORK *light; //0x60
    MDSPRITE_WORK *sprite; //0x64
    MDMODEL_WORK *model; //0x68
    MDSEL_STAR *star; //0x6C
    MDMESS *mess; //0x70
    MDPARTICLE_WORK *particle; //0x74
    BOOL starMoveF; //0x78
    int unk7C;
    BOOL mdSprMoveF; //0x80
    BOOL unk84; //0x84
    BOOL turnLeft; //0x88
    int unk8C;
    float time; //0x90
    float ofsTime;
    float unk98;
    float unk9C;
    float sprTime; //0xA0
    float t; //0xA4
    float ofsT;
    float unkAC;
    float unkB0;
    float sprT; //0xB4
} SYS;

extern SYS *sys;
extern SYS *sysP;
extern SYS sysWork;
extern MD_FUNC MdFlowTbl[MDSEL_FLOW_MAX];
extern MD_PROC MdProcTbl[MDSEL_PROC_MAX];

void MdMain(void);
void MdInit(void);
void MdClose(void);

void MdObjDebugFontCreate(OMOBJ *obj);
void MdObjDebugFontMain(OMOBJ *obj);
void MdObjPadMain(OMOBJ *obj);
void MdObjCameraCreate(OMOBJ *obj);
void MdObjCameraMain(OMOBJ *obj);
void MdObjLightCreate(OMOBJ *obj);
void MdObjLightMain(OMOBJ *obj);

void MdDebugFontCreate(MDDEBUGFONT_WORK *debugFont);
void MdDebugFontUpdate(MDDEBUGFONT_WORK *debugFont);
void MdDebugFontPrint(MDDEBUGFONT_WORK *debugFont, const char *str, ...);

void MdCameraCreate(MDCAMERA_WORK *cameraWork);
void MdCameraDebug(MDCAMERA_WORK *cameraWork);
void MdCameraUpdate(MDCAMERA_WORK *cameraWork);
void MdCameraPrint(MDCAMERA_WORK *cameraWork);
void MdCameraPosSet(MDCAMERA *camera, float h, float v, float r, float cx, float cy, float cz);

void MdLightCreate(MDLIGHT_WORK *lightWork);
void MdLightUpdate(MDLIGHT_WORK *lightWork);
void MdLightUpdateData(MDLIGHT *light);
void MdLightDebug(MDLIGHT_WORK *lightWork);
void MdLightPrint(MDLIGHT_WORK *lightWork);

void MdMessCreate(MDMESS *mess);
void MdMessKill(MDMESS *mess);
void MdMessMain(MDMESS *mess, int winNo);
void MdMessWinOpen(MDMESS *mess, int winNo, s16 type, s16 frame, s16 drawNo, s16 pri, float posX, float posY, s16 w, s16 h, float bgTPLvl);
void MdMessWinClose(MDMESS *mess, int winNo);
void MdMessWinKill(MDMESS *mess, int winNo);
void MdMessWinPosSet(MDMESS *mess, int winNo, float posX, float posY);

void MdMessWinSpeedSet(MDMESS *mess, int winNo, s16 speed);
void MdMessWinAttrSet(MDMESS *mess, int winNo, u32 attr);
void MdMessWinAttrReset(MDMESS *mess, int winNo, u32 attr);
void MdMessWinPad1Set(MDMESS *mess, int winNo);
void MdMessWinMesSet(MDMESS *mess, int winNo, BOOL choice, ...);
BOOL MdMessWinCheck(MDMESS *mess, int winNo);
s16 MdMessWinChoiceNowGet(MDMESS *mess, int winNo);
void MdMessWinChoiceSet(MDMESS *mess, int winNo, s16 choice);
int MdMessWinChoiceGet(MDMESS *mess, int winNo);
void MdMessWinHomeClear(MDMESS *mess, int winNo);
s16 MdMessWinStatGet(MDMESS *mess, int winNo);

Mtx *MdMathMatrixDir(HuVecF *start, HuVecF *end);
void MdMathMatrixRot(Mtx *matrix, HuVecF *rot);

void MdMathClamp(float *val, float min, float max);

void MdMathAngleClamp(float *angle);


void MdMathOscillate(int type, float *in, float *out, float t);
void MdMathLerp(float *out, float a, float b, float t);
void MdMathLerpOfs(float *out, float a, float b, float t, float ofs);
void MdMathLerpColor(u8 *out, float a, float b, float t);
void MdMathLerpScale(HuVecF *out, float a, float b, float t);
void MdMathLerpScaleOfs(HuVecF *out, float a, float b, float t, float ofs);
void MdMathStdCurveGet(int type, float *in, float *out, float t);

void MdMathCurveInit(int sampleNum, float *sampleT, float *sample, float *sampleSlope);
void MdMathCurveSample(float *out, int sampleNum, float *sampleT, float *sample, float *sampleSlope, float t);
void MdMathStdCurveInit(void);

void MdSpriteCreate(MDSPRITE_WORK *sprite);
HuVec2F *MdSpritePosGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo);
HuVec2F *MdSpriteScaleGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo);
float *MdSpriteAlphaGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo);

void MdSpriteGrpDispSet(MDSPRITE_WORK *sprite, int grpNo, BOOL dispF);
HuVec2F *MdSpriteGrpPosGet(MDSPRITE_WORK *sprite, int grpNo);
HuVec2F *MdSpriteGrpScaleGet(MDSPRITE_WORK *sprite, int grpNo);


void MdModelCreate(MDMODEL_WORK *model, MDCAMERA_WORK *camera);
void MdParticleCreate(MDPARTICLE_WORK *particle);
void MdParticleDataUpdate(int bufIdx);

#endif