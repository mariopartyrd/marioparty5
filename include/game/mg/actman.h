#ifndef _MG_ACTMAN_H
#define _MG_ACTMAN_H

#include "game/object.h"
#include "game/hu3d.h"
#include "game/pad.h"
#include "game/charman.h"
#include "game/mg/colman.h"

typedef struct MgPlayer_s MGPLAYER;
typedef struct MgActor_s MGACTOR;

typedef void (*MGPLAYER_MODE_FUNC)(MGPLAYER *playerP);
typedef void (*MGACTOR_CORRECT_HOOK)(MGACTOR *actorP, int param);

#define MGPLAYER_ATTR_COMSTK (1 << 0)
#define MGPLAYER_ATTR_HEADJUMP_OFF (1 << 1)
#define MGPLAYER_ATTR_STUNOFF (1 << 2)

#define MGPLAYER_ATTR_ANGLELOCK (1 << 8)
#define MGPLAYER_ATTR_MOVEOFF (1 << 9)
#define MGPLAYER_ATTR_PAUSE (1 << 10)
#define MGPLAYER_ATTR_VIBKILL (1 << 11)

#define MGPLAYER_VIBATTR_SQUISH (1 << 0)
#define MGPLAYER_VIBATTR_HEADJUMP (1 << 1)
#define MGPLAYER_VIBATTR_SQUISH_FAST (1 << 2)
#define MGPLAYER_VIBATTR_HIT_SRC (1 << 3)
#define MGPLAYER_VIBATTR_KNOCKBACK_SRC (1 << 4)
#define MGPLAYER_VIBATTR_SQUISH_HARD_SRC (1 << 5)
#define MGPLAYER_VIBATTR_HIT (1 << 6)
#define MGPLAYER_VIBATTR_KNOCKBACK (1 << 7)
#define MGPLAYER_VIBATTR_SQUISH_HARD (1 << 8)

#define MGPLAYER_MODE_ATTR_SQUISH (1 << 0)
#define MGPLAYER_MODE_ATTR_SQUISH_HARD (1 << 1)
#define MGPLAYER_MODE_ATTR_HIPDROP_OFF (1 << 2)
#define MGPLAYER_MODE_ATTR_STKLOCK (1 << 3)
#define MGPLAYER_MODE_ATTR_HEADJUMP (MGPLAYER_MODE_ATTR_HIPDROP_OFF|MGPLAYER_MODE_ATTR_STKLOCK)
#define MGPLAYER_MODE_ATTR_AIR (1 << 4)

#define MGPLAYER_ACTFLAG_WALK (1 << 0)
#define MGPLAYER_ACTFLAG_JUMP (1 << 1)
#define MGPLAYER_ACTFLAG_PUNCH (1 << 2)
#define MGPLAYER_ACTFLAG_KICK (1 << 3)
#define MGPLAYER_ACTFLAG_HIPDROP (1 << 4)
#define MGPLAYER_ACTFLAG_STUN (1 << 5)

#define MGPLAYER_MODE_WALK 0
#define MGPLAYER_MODE_JUMP 1
#define MGPLAYER_MODE_FALL 2
#define MGPLAYER_MODE_PUNCH 3
#define MGPLAYER_MODE_KICK 4
#define MGPLAYER_MODE_HIPDROP 5
#define MGPLAYER_MODE_HIT 6
#define MGPLAYER_MODE_KNOCKBACK 7
#define MGPLAYER_MODE_SQUISH 8
#define MGPLAYER_MODE_SQUISH_HARD 9

#define MGPLAYER_MODE_MAX 18

#define MGPLAYER_STUN_NONE 0
#define MGPLAYER_STUN_HIT 1
#define MGPLAYER_STUN_KNOCKBACK 2
#define MGPLAYER_STUN_SQUISH 3
#define MGPLAYER_STUN_SQUISH_HARD 4

#define MGPLAYER_STUN_BLINK 5
#define MGPLAYER_STUN_FREEZE 6

struct MgActor_s {
    s8 no;
    int mdlId;
    u8 attr;
    int param;
    int type;
    HuVecF oldPos;
    HuVecF vel;
    HuVecF pos;
    HuVecF moveDir;
    HuVecF colNorm;
    HuVecF forceA;
    HuVecF forceB;
    float rotY;
    float gravity;
    float velY;
    float terminalVelY;
    u16 colMesh;
    HSFOBJECT *colObj;
    s16 colFace;
    u32 colGroundAttr;
    int correctHookParam;
    MGACTOR_CORRECT_HOOK correctHook; 
};

struct MgPlayer_s {
    s16 playerNo;
    s16 charNo;
    s8 padNo;
    s8 camBit;
    OMOBJ *omObj;
    u32 actionFlag;
    u16 motNo;
    s8 stunType;
    float stunAngle;
    float stunTime;
    float squishTime;
    u16 attr;
    u16 modeAttr;
    u16 vibAttr;
    s16 subMode;
    int mode;
    float timer;
    float squishOldHeight;
    s16 work[4];
    MGACTOR *actor;
    MGPLAYER_MODE_FUNC modeFunc[MGPLAYER_MODE_MAX];
};

typedef struct MgActorParam_s {
    float height;
    float radius;
    int param;
    int type;
    COL_NARROW_HOOK narrowHook;
    u32 attr;
    int correctHookParam;
    MGACTOR_CORRECT_HOOK correctHook; 
} MGACTOR_PARAM;

typedef struct MgActorColMapPoly_s {
    HuVecF pos;
    u32 code;
    int mdlNo;
    HSFOBJECT *obj;
    int triNo;
} MGACTOR_COLMAP_POLY;

BOOL MgPlayerComStkOn(MGPLAYER *playerP);
void MgPlayerComStkOff(MGPLAYER *playerP);
BOOL MgPlayerVecChase(MGPLAYER *playerP, HuVecF *pos, float velZ, float radius);
BOOL MgActorVecChase(MGACTOR *actorP, HuVecF *pos, float velZ, float radius);
void MgPlayerVibrateCreate(MGPLAYER *playerP);
void MgPlayerVibrateKill(MGPLAYER *playerP);
BOOL MgPlayerModeLandSet(MGPLAYER *playerP, int mode);
BOOL MgPlayerModeSet(MGPLAYER *playerP, int mode);

void MgActorInit(void);
void MgActorClose(void);
OMOBJMAN *MgActorObjectSetup(void);
void MgPlayerPadSet(MGPLAYER *playerP, int stkX, int stkY, int btnDown, int btn);
void MgActorColMapInit(HU3DMODELID *mdlId, s16 mdlNum, int bodyMax);
void MgActorColAttrSet(MGACTOR *actorP, u32 mask);
void MgActorColAttrReset(MGACTOR *actorP, u32 mask);
u32 MgActorColAttrGet(MGACTOR *actorP, u32 mask);
void MgPlayerModeFuncSet(MGPLAYER *playerP, MGPLAYER_MODE_FUNC *funcTbl);
MGPLAYER *MgPlayerCreate(s16 playerNo, MGACTOR_PARAM *param, s16 model, s16 camBit, u32 actionFlag, unsigned int *motDataNum);
MGPLAYER *MgPlayerCreateJumpAlt(s16 playerNo, MGACTOR_PARAM *param, s16 model, s16 camBit, u32 actionFlag, unsigned int *motDataNum, unsigned int flag);
MGACTOR *MgActorCreate(MGACTOR_PARAM *param, int mdlId);
void MgActorColMapMaskSet(int mdlNo, u32 mask);
u32 MgActorColMapMaskGet(int mdlNo);
void MgActorColCylReset(void);
void MgActorColCylSet(void);

void MgActorKill(MGACTOR *actorP);

void MgActorExec(void);

BOOL MgActorColMapPolyGet(HuVecF *pos1, HuVecF *pos2, u32 mask, MGACTOR_COLMAP_POLY *outPoly);
void MgActorPosUpdate(MGACTOR *actorP, HuVecF *pos);
void MgActorPosSet(MGACTOR *actorP, HuVecF *pos);

void MgPlayerPauseOn(MGPLAYER *playerP);
void MgPlayerPauseOff(MGPLAYER *playerP, HuVecF *pos);
void MgActorPauseOn(MGACTOR *actorP);
void MgActorPauseOff(MGACTOR *actorP, HuVecF *pos);

void MgActorColAttrParamSet(COL_ATTRPARAM *param, u32 polyAttr);
void MgActorColAttrParamGet(COL_ATTRPARAM *param, u32 polyAttr);

u16 MgPlayerAttrCheck(MGPLAYER *playerP, u16 attr);
void MgPlayerAttrReset(MGPLAYER *playerP, u16 attr);
void MgPlayerAttrSet(MGPLAYER *playerP, u16 attr);

u16 MgPlayerVibAttrCheck(MGPLAYER *playerP, u16 attr);
void MgPlayerVibAttrReset(MGPLAYER *playerP, u16 attr);
void MgPlayerVibAttrSet(MGPLAYER *playerP, u16 attr);

u16 MgPlayerModeAttrCheck(MGPLAYER *playerP, u16 attr);
void MgPlayerModeAttrReset(MGPLAYER *playerP, u16 attr);
void MgPlayerModeAttrSet(MGPLAYER *playerP, u16 attr);

BOOL MgPlayerStunSet(MGPLAYER *playerP, int stunType, float angle, int maxTime);

#endif