#ifndef _MG_COLMAN_H
#define _MG_COLMAN_H

#include <dolphin.h>
#include "game/hu3d.h"

#define COLMAP_ACTOR_MAX 256

typedef struct ColMapNarrowParam_s {
    int paramA;
    int paramB;
    int type;
    HuVecF point;
    HuVecF normPos;
} COLMAP_NARROW_PARAM;

typedef struct ColMapActor_s COLMAP_ACTOR;

typedef int (*COLMAP_NARROW_HOOK)(COLMAP_NARROW_PARAM *a, COLMAP_NARROW_PARAM *b);
typedef int (*COLMAP_ACTORCOL_HOOK)(COLMAP_ACTOR *actor, void *user);


typedef struct ColMapAttrParam_s {
    int type;
    float speed;
    float yDeviate;
    float maxDot;
    u32 attr;
} COLMAP_ATTR_PARAM;


typedef struct ColMapActorParam_s {
    float height;
    float radius;
    int paramA;
    int paramB;
    int type;
    u32 mask;
    COLMAP_NARROW_HOOK narrowHook;
    COLMAP_NARROW_HOOK narrowHook2;
    COLMAP_ACTORCOL_HOOK actorColHook;
    void *user;
    u32 attr;
} COLMAP_ACTOR_PARAM;

typedef struct ColMapColPoint_s {
    HuVecF colOfs;
    HuVecF normal;
    u32 polyAttr;
    u16 meshNo;
    HSFOBJECT *obj;
    s16 faceNo;
} COLMAP_COLPOINT;

struct ColMapActor_s {
    COLMAP_ACTOR_PARAM param;
    u32 groundAttr;
    HuVecF pos[2];
    s16 colPointNum;
    COLMAP_COLPOINT colPoint[5];
    HuVecF oldPos;
    HuVecF moveDir;
    u32 paramAttr;
    float unk130;
    float unk134;
    u32 colBit[(COLMAP_ACTOR_MAX+31)/32];
};

COLMAP_ACTOR *ColMapActorGet(int no);
void ColMapClear(void);
void ColMapInit(HU3DMODELID *mdlId, s16 mdlNum, int actorMax);
void ColMapMaskSet(int mdlNo, u32 mask);
u32 ColMapMaskGet(int mdlNo);
BOOL ColMapAltColReset(void);
BOOL ColMapAltColSet(void);
void ColMapKill(void);
BOOL ColMapInitCheck(void);
void ColMapDirtyClear(void);
void ColMapAttrParamSet(COLMAP_ATTR_PARAM *param, u32 polyAttr);
void ColMapAttrParamGet(COLMAP_ATTR_PARAM *param, u32 polyAttr);
void ColMapActorPosSet(HuVecF *pos, int no);
void ColMapActorPosGet(HuVecF *pos, int no);
void ColMapActorParamSet(COLMAP_ACTOR_PARAM *param, int no);
COLMAP_ACTOR *ColMapActorGetSafe(int no);
BOOL ColMapPolyGet(HuVecF *pos1, HuVecF *pos2, u32 mask, HuVecF *outPos, u32 *outCode, int *outMdlNo, HSFOBJECT **outObj, int *outTriNo);

#endif