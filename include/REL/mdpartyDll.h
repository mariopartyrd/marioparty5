#ifndef _MDPARTYDLL_H
#define _MDPARTYDLL_H

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


#define MD_OVLCALL_GOTO 0
#define MD_OVLCALL_CALL 1
#define MD_OVLCALL_RETURN 2

#define MDMESS_WIN_SPEAKER 0
#define MDMESS_WIN_NOSPEAKER 1
#define MDMESS_WIN_LIST_END -1

#define MDMATH_CURVE_SAMPLE_MAX 8

#define MDPARTY_TEXBUF_MIRROR 0
#define MDPARTY_TEXBUF_MAX 10

#define MDPARTY_TEXBUF_W 64
#define MDPARTY_TEXBUF_H 64

#define MDLIGHT_MAX 1

#define MdProcCall(idx, arg) MdProcTbl[idx](arg)
#define MdProcWait(idx, arg) do { \
    while(!MdProcCall(idx, arg)) { \
        HuPrcVSleep(); \
    } \
} while(0)

#define MDSPR_GRP_MEMBERNO(grp, spr) ((spr)-(grp##_START))

#define MDSPR_GRP_INFO_DEFINE(grp, posX, posY) { grp##_START, grp##_END, posX, posY }

enum {
    MD_LOAD_STATE_NONE,
    MD_LOAD_STATE_BOARD_LOAD,
    MD_LOAD_STATE_WAIT,
    MD_LOAD_STATE_CHAR_LOAD,
    MD_LOAD_STATE_LOAD,
    MD_LOAD_STATE_DONE
};

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
    MDCAMERA_BG, //0
    MDCAMERA_MAIN, //1
    MDCAMERA_FG, //2
    MDCAMERA_MAX //3
};

enum {
    MDCAMERA_FOCUS_TARGET, // 0
    MDCAMERA_FOCUS_POS, //1
    MDCAMERA_FOCUS_MAX //2
};

enum {
    MDMESS_WIN_MAIN, //0
    MDMESS_WIN_HELP, //1
    MDMESS_WIN_TEAM1, //2
    MDMESS_WIN_TEAM2, //3
    MDMESS_WIN_MAX //4
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
    MDPARTY_FLOW_INIT, //0
    MDPARTY_FLOW_START, //1
    MDPARTY_FLOW_PLAYER_NUM_SEL, //2
    MDPARTY_FLOW_CHR_SEL, //3
    MDPARTY_FLOW_TEAM_SEL, //4
    MDPARTY_FLOW_MAP_SEL, //5
    MDPARTY_FLOW_TURN_SEL, //6
    MDPARTY_FLOW_PACK_SEL, //7
    MDPARTY_FLOW_BONUS_SEL, //8
    MDPARTY_FLOW_HANDICAP, //9
    MDPARTY_FLOW_MAX //10
};

enum {
    MDPARTY_PROC_RESET, //0
    MDPARTY_PROC_CAMERA, //1
    MDPARTY_PROC_CURSOR, //2
    MDPARTY_PROC_WIN, //3
    MDPARTY_PROC_GAME_TYPE, //4
    MDPARTY_PROC_CAP_PLAYER, //5
    MDPARTY_PROC_CHR, //6
    MDPARTY_PROC_TEAM, //7
    MDPARTY_PROC_VS, //8
    MDPARTY_PROC_MAP, //9
    MDPARTY_PROC_MAP_NAME, //10
    MDPARTY_PROC_CAP_TURN, //11
    MDPARTY_PROC_CAP_PACK, //12
    MDPARTY_PROC_CAP_BONUS, //13
    MDPARTY_PROC_HANDICAP, //14
    MDPARTY_PROC_DISP_SETUP, //15
    MDPARTY_PROC_FINGER, //16
    MDPARTY_PROC_MAP_ENTER, //17
    MDPARTY_PROC_NAME, //18
    MDPARTY_PROC_MAX //19
};

enum {
    MDSPR_ANIM_WIN_FRAME, //0
    MDSPR_ANIM_WIN_BG, //1
    MDSPR_ANIM_CURSOR, //2
    MDSPR_ANIM_CURSOR_GLOW, //3
    MDSPR_ANIM_PLAYERNO, //4
    MDSPR_ANIM_COMDIF, //5
    MDSPR_ANIM_PLAYERNO_FINGER, //6
    MDSPR_ANIM_TURN, //7
    MDSPR_ANIM_BONUS, //8
    MDSPR_ANIM_TEAM_RED, //9
    MDSPR_ANIM_TEAM_BLUE, //10
    MDSPR_ANIM_MAP1_NAME, //11
    MDSPR_ANIM_MAP2_NAME, //12
    MDSPR_ANIM_MAP3_NAME, //13
    MDSPR_ANIM_MAP4_NAME, //14
    MDSPR_ANIM_MAP5_NAME, //15
    MDSPR_ANIM_MAP6_NAME, //16
    MDSPR_ANIM_MAP7_NAME, //17
    MDSPR_ANIM_HANDICAP_NUM, //18
    MDSPR_ANIM_HANDICAP, //19
    MDSPR_ANIM_EFFECT_DOT, //20
    MDSPR_ANIM_EFFECT_GLOW, //21
    MDSPR_ANIM_MODE_NAME, //22
    MDSPR_ANIM_MAX //23
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
    MDSPR_PLAYERNO_1P, //13
    MDSPR_PLAYERNO_2P, //14
    MDSPR_PLAYERNO_3P, //15
    MDSPR_PLAYERNO_4P, //16
    MDSPR_COMDIF_1P, //17
    MDSPR_COMDIF_2P, //18
    MDSPR_COMDIF_3P, //19
    MDSPR_COMDIF_4P, //20
    MDSPR_TEAM_RED, //21
    MDSPR_TEAM_BLUE, //22
    MDSPR_MAP1_NAME, //23
    MDSPR_MAP2_NAME, //24
    MDSPR_MAP3_NAME, //25
    MDSPR_MAP4_NAME, //26
    MDSPR_MAP5_NAME, //27
    MDSPR_MAP6_NAME, //28
    MDSPR_MAP7_NAME, //29
    MDSPR_HANDICAP_NUM_1P, //30
    MDSPR_HANDICAP_NUM_2P, //31
    MDSPR_HANDICAP_NUM_3P, //32
    MDSPR_HANDICAP_NUM_4P, //33
    MDSPR_HANDICAP_1P, //34
    MDSPR_HANDICAP_2P, //35
    MDSPR_HANDICAP_3P, //36
    MDSPR_HANDICAP_4P, //37
    MDSPR_MODE_NAME, //38
    MDSPR_MAX //39
};

enum {
    MDSPR_GRP_WIN, //0
    MDSPR_GRP_CURSOR_LR, //1
    MDSPR_GRP_CURSOR_UDLR, //2
    MDSPR_GRP_PLAYERNO, //3
    MDSPR_GRP_COMDIF, //4
    MDSPR_GRP_TEAM, //5
    MDSPR_GRP_MAP_NAME, //6
    MDSPR_GRP_HANDICAP, //7
    MDSPR_GRP_NAME, //8
    MDSPR_GRP_MAX //9
};

enum {
    MDSPR_GRP_WIN_START = MDSPR_WIN_FRAME_TOP,
    MDSPR_GRP_WIN_END = MDSPR_WIN_BG,
    MDSPR_GRP_CURSOR_LR_START = MDSPR_CURSOR_LR_R,
    MDSPR_GRP_CURSOR_LR_END = MDSPR_CURSOR_LR_L,
    MDSPR_GRP_CURSOR_UDLR_START = MDSPR_CURSOR_UDLR_R,
    MDSPR_GRP_CURSOR_UDLR_END = MDSPR_CURSOR_GLOW_UDLR_D,
    MDSPR_GRP_PLAYERNO_START = MDSPR_PLAYERNO_1P,
    MDSPR_GRP_PLAYERNO_END = MDSPR_PLAYERNO_4P,
    MDSPR_GRP_COMDIF_START = MDSPR_COMDIF_1P,
    MDSPR_GRP_COMDIF_END = MDSPR_COMDIF_4P,
    MDSPR_GRP_TEAM_START = MDSPR_TEAM_RED,
    MDSPR_GRP_TEAM_END = MDSPR_TEAM_BLUE,
    MDSPR_GRP_MAP_NAME_START = MDSPR_MAP1_NAME,
    MDSPR_GRP_MAP_NAME_END = MDSPR_MAP7_NAME,
    MDSPR_GRP_HANDICAP_START = MDSPR_HANDICAP_NUM_1P,
    MDSPR_GRP_HANDICAP_END = MDSPR_HANDICAP_4P,
    MDSPR_GRP_NAME_START = MDSPR_MODE_NAME,
    MDSPR_GRP_NAME_END = MDSPR_MODE_NAME,
};

enum {
    MDPARTY_OBJ_DEBUGFONT, // 0
    MDPARTY_OBJ_PAD, //1
    MDPARTY_OBJ_CAMERA, //2
    MDPARTY_OBJ_LIGHT, //3
    MDPARTY_OBJ_MAX //4
};

enum {
    MDMODEL_STAR, //0
    MDMODEL_STAGE, //1
    MDMODEL_STAGE_CHR, //2
    
    MDMODEL_MARIO, //3
    MDMODEL_LUIGI, //4
    MDMODEL_PEACH, //5
    MDMODEL_YOSHI, //6
    MDMODEL_WARIO, //7
    MDMODEL_DAISY, //8
    MDMODEL_WALUIGI, //9
    MDMODEL_KINOPIO, //10
    MDMODEL_TERESA, //11
    MDMODEL_MINIKOOPA, //12
    
    MDMODEL_CAP_MAN_1P, //13
    MDMODEL_CAP_MAN_2P, //14
    MDMODEL_CAP_MAN_3P, //15
    MDMODEL_CAP_MAN_4P, //16
    
    MDMODEL_CAP_COM_1P, //17
    MDMODEL_CAP_COM_2P, //18
    MDMODEL_CAP_COM_3P, //19
    MDMODEL_CAP_COM_4P, //20
    
    MDMODEL_CAP_BROYAL, //21
    MDMODEL_CAP_TMATCH, //22

    MDMODEL_CAP_CHR_00, //23
    MDMODEL_CAP_CHR_01, //24
    MDMODEL_CAP_CHR_02, //25
    MDMODEL_CAP_CHR_03, //26
    MDMODEL_CAP_CHR_04, //27
    MDMODEL_CAP_CHR_05, //28
    MDMODEL_CAP_CHR_06, //29
    MDMODEL_CAP_CHR_07, //30
    MDMODEL_CAP_CHR_08, //31
    MDMODEL_CAP_CHR_09, //32

    MDMODEL_VS, //33
    
    MDMODEL_MAP1, //34
    MDMODEL_MAP2, //35
    MDMODEL_MAP3, //36
    MDMODEL_MAP4, //37
    MDMODEL_MAP5, //38
    MDMODEL_MAP6, //39
    MDMODEL_MAP7, //40
    
    MDMODEL_CAP_PACK0, //41
    MDMODEL_CAP_PACK1, //42
    MDMODEL_CAP_PACK2, //43
    MDMODEL_CAP_PACK3, //44
    MDMODEL_CAP_PACK4, //45
    
    MDMODEL_CAP_TURN, //46
    MDMODEL_CAP_BONUS, //47
    
    MDMODEL_FINGER_1P, //48
    MDMODEL_FINGER_2P, //49
    MDMODEL_FINGER_3P, //50
    MDMODEL_FINGER_4P, //51
    
    MDMODEL_RING_1P, //52
    MDMODEL_RING_2P, //53
    MDMODEL_RING_3P, //54
    MDMODEL_RING_4P, //55
    
    MDMODEL_FONTBGFILTER, //56
    
    MDMODEL_MAX //57
};

enum {
    MDMODEL_ANIM_FINGER_1P, //0
    MDMODEL_ANIM_FINGER_2P, //1
    MDMODEL_ANIM_FINGER_3P, //2
    MDMODEL_ANIM_FINGER_4P, //3
    
    MDMODEL_ANIM_CAP_TURN, //4
    MDMODEL_ANIM_CAP_BONUS, //5
    
    MDMODEL_ANIM_MAX //6
};

enum {
    MDMODEL_MOT_STAR_IDLE, //0
    MDMODEL_MOT_STAR_TALK, //1
    MDMODEL_MOT_STAR_TALK_FAST, //2
    
    MDMODEL_MOT_MARIO_IDLE,//3
    MDMODEL_MOT_MARIO_WIN, //4
    
    MDMODEL_MOT_LUIGI_IDLE, //5
    MDMODEL_MOT_LUIGI_WIN, //6
    
    MDMODEL_MOT_PEACH_IDLE, //7
    MDMODEL_MOT_PEACH_WIN, //8
    
    MDMODEL_MOT_YOSHI_IDLE, //9
    MDMODEL_MOT_YOSHI_WIN, //10
    
    MDMODEL_MOT_WARIO_IDLE, //11
    MDMODEL_MOT_WARIO_WIN, //12
    
    MDMODEL_MOT_DAISY_IDLE, //13
    MDMODEL_MOT_DAISY_WIN, //14
    
    MDMODEL_MOT_WALUIGI_IDLE, //15
    MDMODEL_MOT_WALUIGI_WIN, //16
    
    MDMODEL_MOT_KINOPIO_IDLE, //17
    MDMODEL_MOT_KINOPIO_WIN, //18
    
    MDMODEL_MOT_TERESA_IDLE, //19
    MDMODEL_MOT_TERESA_WIN, //20
    
    MDMODEL_MOT_MINIKOOPA_IDLE, //21
    MDMODEL_MOT_MINIKOOPA_WIN, //22
    
    MDMODEL_MOT_MAX //23
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
    HUSPRID sprId[MDSPR_MAX]; //0x5C
    HUSPRGRPID grpId[MDSPR_GRP_MAX]; //0xAA
} MDSPRITE_WORK;

typedef struct MdModelDebugInfo_s {
    char *name; //0x00
    int cameraNo; //0x04
} MDMODEL_DEBUG_INFO;

typedef struct MdModelWork_s {
    HU3DMODELID mdlId[MDMODEL_MAX]; //0x00
    HU3DMOTID motId[MDMODEL_MOT_MAX]; //0x72
    HU3DANIMID animId[MDMODEL_ANIM_MAX]; //0xA0
    HuVecF *posP[MDMODEL_MAX]; //0xAC
    HuVecF *rotP[MDMODEL_MAX]; //0x190
    HuVecF *scaleP[MDMODEL_MAX]; //0x274
} MDMODEL_WORK;


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
} MDMESS_WORK;

typedef struct MdSprInfo_s {
    s16 animNo; //0x00
    s16 pri; //0x02
    HuVec2f pos; //0x04
    HuVec2f scale; //0xC
    float tpLvl; //0x14
} MDSPR_INFO;

typedef struct MdSprGrpInfo_s {
    s16 start; //0x00
    s16 end; //0x02
    float posX; //0x04
    float posY; //0x08
} MDSPR_GRP_INFO;

typedef struct MdTexBufUnit_s {
    void *addr; //0x00
    GXTexObj texObj; //0x04
    s16 posX; //0x24
    s16 posY; //0x26
    u16 w; //0x28
    u16 h; //0x2A
    GXTexFmt texFmt; //0x2C
} MDTEXBUF_UNIT;

typedef struct MdTexBufWork_s {
    MDTEXBUF_UNIT unit[MDPARTY_TEXBUF_MAX]; //0x00
    int unk1E0;
} MDTEXBUF_WORK;

typedef struct MdRing_s {
    BOOL disp; //0x00
    int mode; //0x04
    GXColor color; //0x08
    GXColor endColor; //0x0C
    int unk10;
    float time; //0x14
    float ofsTime; //0x18
    float t; //0x1C
    float ofsT; //0x20
    int mdlNo; //0x24
    HuVecF **posP; //0x28
    HuVecF **rotP; //0x2C
    HuVecF **scaleP; //0x30
    int baseMdlNo; //0x34
    HuVecF baseOfs; //0x38
} MDRING;

typedef struct MdRingWork_s {
    MDRING ring[4]; //0x00
    float scale; //0x110
} MDRING_WORK;

typedef struct MdPartyPlayerSpr_s {
    int mode; //0x00
    float time; //0x04
    float ofsTime; //0x08
    float t; //0x0C
    float ofsT; //0x10
} MDPARTY_PLAYERSPR;

typedef struct MdPartyChar_s {
    int mode; //0x04
    float time; //0x04
    float ofsTime; //0x08
    float t; //0x0C
    float ofsT; //0x10
    HuVecF origPos; //0x14
    HuVecF pos; //0x20
    HuVecF origRot; //0x2C
    HuVecF rot; //0x38
} MDPARTY_CHR;

typedef struct MdPartyStar_s {
    s16 unk0;
    s16 mode; //0x02
    s16 motNext; //0x04
} MDPARTY_STAR;

typedef struct Sys_s {
    int subMode; //0x00
    int unk4;
    int flowTime; //0x08
    int flowMode; //0x0C
    int ovlCallMode; //0x10
    OMOVL nextDll; //0x14
    s8 exitF; //0x18
    s8 skipWipeF; //0x19
    s8 gameType; //0x1A
    s8 playerNum; //0x1B
    s8 prevPlayerNum; //0x1C
    s8 maxPlayerNum; //0x1D
    s8 padConnect[4]; //0x1E
    s8 padEnable[4]; //0x22
    s8 padNoConnect[4]; //0x26
    s8 padNoEnable[4]; //0x2A
    s8 chrSel[4]; //0x2E
    s8 chrSelEnd[4]; //0x32
    s8 comDif[4]; //0x36
    s8 comPlayerNo[3]; //0x3A
    s8 selComNo; //0x3D
    s8 teamOrderNo; //0x3E
    s8 teamPlayer[2][2]; //0x3F
    s8 teamOrder[4]; //0x43
    s8 teamOrderBackup[4]; //0x47
    s8 teamNo[2]; //0x4B
    s8 teamComPlayerNo[3]; //0x4D
    s8 selMapNo; //0x50
    s8 prevSelMapNo; //0x51
    float selMapAngle; //0x54
    float unk58;
    float selMapRadius; //0x5C
    s8 selTurn; //0x60
    s8 selPack; //0x61
    s8 selBonus; //0x62
    s8 selHandicap[4]; //0x63
    s8 selHandicapPlayer; //0x67
    OMOBJMAN *objman; //0x68
    HUPROCESS *mainProc; //0x6C
    HUPROCESS *loadProc; //0x70
    HUPROCESS *starProc; //0x74
    HUPROCESS *playerSprProc; //0x78
    HUPROCESS *chrProc; //0x7C
    HUPROCESS *ringProc; //0x80
    HUPROCESS *mainWinProc; //0x84
    HUPROCESS *textWinProc; //0x88
    OMOBJ *obj[MDPARTY_OBJ_MAX]; //0x8C
    OMOVLHIS *ovlHis; //0x9C
    s32 evtNo; //0xA0
    s16 loadState; //0xA4
    s16 loadAsyncStat; //0xA6
    unsigned int loadDir; //0xA8
    s16 loadProcReady; //0xAC
    MDDEBUGFONT_WORK *debugFont; //0xB0
    MDPAD_WORK *pad; //0xB4
    MDCAMERA_WORK *camera; //0xB8
    MDLIGHT_WORK *light; //0xBC
    MDSPRITE_WORK *sprite; //0xC0
    MDMODEL_WORK *model; //0xC4
    MDTEXBUF_WORK *texbuf; //0xC8
    MDPARTY_STAR *star; //0xCC
    MDPARTY_PLAYERSPR playerSpr[4]; //0xD0
    MDPARTY_CHR chr[10]; //0x120
    MDMESS_WORK *mess; //0x3C8
    MDRING_WORK *ring; //0x3CC
    BOOL dirtyF; //0x3D0
    BOOL teamInitF; //0x3D4
    BOOL winInitF; //0x3D8
    BOOL cursorInitF; //0x3DC
    BOOL handicapInitF; //0x3E0
    BOOL nameInitF; //0x3E4
    BOOL unk3E8;
    int cursorDir; //0x3EC
    int cursorMode; //0x3F0
    float unk3F4;
    float procTime; //0x3F8
    float procOfsTime; //0x3FC
    float cameraTime; //0x400
    float cameraTotalTime; //0x404
    float sprTime; //0x408
    float nameTime; //0x40C
    float procT; //0x410
    float procOfsT; //0x414
    float cameraT; //0x418
    float cameraTotalT; //0x41C
    float sprT; //0x420
    float nameT; //0x424
    float curveTime[8]; //0x428
    float curveSlope[25]; //0x448
} SYS;

extern SYS *sys;
extern SYS *sysP;
extern SYS sysWork;
extern MD_FUNC MdFlowTbl[MDPARTY_FLOW_MAX];
extern MD_PROC MdProcTbl[MDPARTY_PROC_MAX];

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

void MdMessCreate(MDMESS_WORK *mess);
void MdMessKill(MDMESS_WORK *mess);
void MdMessMain(MDMESS_WORK *mess, int winNo);
void MdMessWinOpen(MDMESS_WORK *mess, int winNo, s16 type, s16 frame, s16 drawNo, s16 pri, float posX, float posY, s16 w, s16 h, float bgTPLvl);
void MdMessWinClose(MDMESS_WORK *mess, int winNo);
void MdMessWinKill(MDMESS_WORK *mess, int winNo);
void MdMessWinPosSet(MDMESS_WORK *mess, int winNo, float posX, float posY);
void MdMessWinScaleSet(MDMESS_WORK *mess, int winNo, float scaleX, float scaleY);
void MdMessWinCenterPosSet(MDMESS_WORK *mess, int winNo, float posX, float posY);

void MdMessWinSpeedSet(MDMESS_WORK *mess, int winNo, s16 speed);
void MdMessWinAttrSet(MDMESS_WORK *mess, int winNo, u32 attr);
void MdMessWinAttrReset(MDMESS_WORK *mess, int winNo, u32 attr);
void MdMessWinPad1Set(MDMESS_WORK *mess, int winNo);
void MdMessWinMesSet(MDMESS_WORK *mess, int winNo, BOOL choice, ...);
BOOL MdMessWinCheck(MDMESS_WORK *mess, int winNo);
s16 MdMessWinChoiceNowGet(MDMESS_WORK *mess, int winNo);
void MdMessWinChoiceSet(MDMESS_WORK *mess, int winNo, s16 choice);
int MdMessWinChoiceGet(MDMESS_WORK *mess, int winNo);
void MdMessWinHomeClear(MDMESS_WORK *mess, int winNo);
s16 MdMessWinStatGet(MDMESS_WORK *mess, int winNo);

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
void MdMathCurveGet(float *out, int sampleNum, float *sampleT, float *sample, float *sampleSlope, float t);
void MdMathStdCurveInit(void);

void MdSpriteCreate(MDSPRITE_WORK *sprite);
HuVec2f *MdSpritePosGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo);
HuVec2f *MdSpriteScaleGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo);
float *MdSpriteAlphaGet(MDSPRITE_WORK *sprite, int grpNo, int memberNo);

void MdSpriteGrpDispSet(MDSPRITE_WORK *sprite, int grpNo, BOOL dispF);
HuVec2f *MdSpriteGrpPosGet(MDSPRITE_WORK *sprite, int grpNo);
HuVec2f *MdSpriteGrpScaleGet(MDSPRITE_WORK *sprite, int grpNo);


void MdModelCreate(MDMODEL_WORK *model, MDCAMERA_WORK *camera);
void MdTexBufInit(void);
void MdTexBufObjPosSet(MDTEXBUF_UNIT *unit, HU3DMODELID modelId, char *objName, u16 cameraBit, HuVecF *ofsP);
void MdTexBufPosSet(MDTEXBUF_UNIT *unit, HuVecF *posP, u16 cameraBit);

void MdModelAnimInit(void);

void MdRingCreate(void);
void MdRingMain(void);
void MdRingDispSet(int no, BOOL dispF);
void MdRingBaseOfsSet(int no, float x, float y, float z);
void MdRingModeSet(int no, int mode);
void MdRingBaseMdlSet(int no, int mdlNo);
void MdRingColorReset(int no);
//void MdRingScaleSet(int no, float scale);

void MdTexBufLayerHook(s16 layerNo);

#endif