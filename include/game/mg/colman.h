#ifndef _MG_COLMAN_H
#define _MG_COLMAN_H

#include <dolphin.h>
#include "game/hu3d.h"

#define COLBODY_MAX 256
#define COLBODY_ATTR_ACTIVE (1 << 0)
#define COLBODY_ATTR_MESHCOL_OFF (1 << 1)
#define COLBODY_ATTR_BODYCOL_OFF (1 << 2)
#define COLBODY_ATTR_COL_OFF (1 << 4)

#define COLBODY_ATTR_RESET (1 << 24)

typedef struct ColNarrowParam_s {
    int paramA;
    int paramB;
    int type;
    HuVecF point;
    HuVecF normPos;
    int colResult;
} COL_NARROW_PARAM;

typedef struct ColBody_s COLBODY;

typedef int (*COL_NARROW_HOOK)(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);
typedef void (*COL_CORRECT_HOOK)(COLBODY *body, void *user);


typedef struct ColAttrParam_s {
    int type;
    float speed;
    float yDeviate;
    float maxDot;
    u32 attr;
} COL_ATTRPARAM;


typedef struct ColBodyParam_s {
    float height;
    float radius;
    int paramA;
    int paramB;
    int type;
    u32 mask;
    COL_NARROW_HOOK narrowHook;
    COL_NARROW_HOOK narrowHook2;
    COL_CORRECT_HOOK colCorrectHook;
    void *user;
    u32 attr;
} COLBODY_PARAM;

typedef struct ColBodyPoint_s {
    HuVecF colOfs;
    HuVecF normal;
    u32 polyAttr;
    u16 meshNo;
    HSFOBJECT *obj;
    s16 faceNo;
} COLBODY_POINT;

struct ColBody_s {
    COLBODY_PARAM param;
    u32 groundAttr;
    HuVecF pos[2];
    s16 colPointNum;
    COLBODY_POINT colPoint[5];
    HuVecF oldPos;
    HuVecF moveDir;
    u32 paramAttr;
    float colT;
    float oldColT;
    s32 colBit[(COLBODY_MAX+31)/32];
};

COLBODY *ColBodyGetRaw(int no);
void ColMapClear(void);
void ColMapInit(HU3DMODELID *mdlId, s16 mdlNum, int actorMax);
void ColMapMaskSet(int mdlNo, u32 mask);
u32 ColMapMaskGet(int mdlNo);
BOOL ColCylReset(void);
BOOL ColCylSet(void);
void ColMapKill(void);
BOOL ColMapInitCheck(void);
void ColDirtyClear(void);
void ColAttrParamSet(COL_ATTRPARAM *param, u32 polyAttr);
void ColAttrParamGet(COL_ATTRPARAM *param, u32 polyAttr);
void ColBodyPosSet(HuVecF *pos, int no);
void ColBodyPosGet(HuVecF *pos, int no);
void ColBodyParamSet(COLBODY_PARAM *param, int no);
COLBODY *ColBodyGet(int no);
BOOL ColPolyGet(HuVecF *pos1, HuVecF *pos2, u32 mask, HuVecF *outPos, u32 *outCode, int *outMdlNo, HSFOBJECT **outObj, int *outTriNo);
void ColBodyExec(void);

#endif