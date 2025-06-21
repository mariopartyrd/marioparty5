#ifndef _BOARD_MASU_H
#define _BOARD_MASU_H

#include "game/hu3d.h"
#include "game/board/capsule.h"

#define MASU_TYPE_NONE 0
#define MASU_TYPE_BLUE 1
#define MASU_TYPE_RED 2
#define MASU_TYPE_KUPA 3
#define MASU_TYPE_HATENA 4
#define MASU_TYPE_STAR 5
#define MASU_TYPE_BLANKHATENA 6
#define MASU_TYPE_VS 7
#define MASU_TYPE_DONKEY 8
#define MASU_TYPE_MAX 9
#define MASU_TYPE_NULL -1

#define MASU_LAYER_DEFAULT 0
#define MASU_LAYER_MAX 1

#define MASU_NULL -1
#define MASU_MAX 256
#define MASU_STAR_MAX 8
#define MASU_DONKEY_MAX 8

#define MASU_LINK_MAX 5

#define MASU_FLAG_BIT(bit) (1U << (bit))
#define MASU_FLAG_BIT_MULTI(bitStart, bitNum) (((1U << bitNum)-1) << (bitStart))
#define MASU_FLAG_BITFIELD(bitStart, bitNum, value) (((unsigned int)(value) & ((1 << bitNum)-1)) << (bitStart))

#define MASU_FLAG_NONE 0
#define MASU_FLAG_JUMPTO MASU_FLAG_BIT(16)
#define MASU_FLAG_JUMPFROM MASU_FLAG_BIT(17)
#define MASU_FLAG_CLIMBTO MASU_FLAG_BIT(18)
#define MASU_FLAG_CLIMBFROM MASU_FLAG_BIT(19)
#define MASU_FLAG_CAPMACHINE MASU_FLAG_BIT(24)
#define MASU_FLAG_BLOCKL MASU_FLAG_BIT(25)
#define MASU_FLAG_BLOCKR MASU_FLAG_BIT(26)
#define MASU_FLAG_START MASU_FLAG_BIT(31)

#define MASU_FLAG_BATTAN MASU_FLAG_BIT(29)
#define MASU_FLAG_DOSSUN MASU_FLAG_BIT(30)

#define MASU_EFF_NONE 0
#define MASU_EFF_WALK 1
#define MASU_EFF_CAPSULE 2
#define MASU_EFF_CAPSULESEL 3

typedef struct Masu_s {
    u8 useMtxF : 1;
    u8 hasStarF : 1;
    HuVecF pos;
    u32 flag;
    HuVecF scale;
    HuVecF rot;
    u16 type;
    s16 capsuleNo;
    Mtx matrix;
    s16 effNo;
    u16 linkNum;
    u16 linkTbl[MASU_LINK_MAX];
} MASU;

typedef int (*MASUPLAYERHOOK)(int playerNo);
typedef int (*MASUPATHCHECKHOOK)(s16 id, u32 flag, s16 *linkTbl, BOOL endF);

void MBMasuCreate(int dataNum);
void MBMasuKill(void);
BOOL MBMasuDataRead(int layer, int dataNum);
BOOL MBMasuWalkExec(int playerNo, int masuId);
BOOL MBMasuWalkEndExec(int playerNo, int masuId);
void MBMasuWalkEndEventExec(int playerNo, int masuId);
void MBMasuColorSet(int playerNo);
int MBMasuNumGet(int layer);
MASU *MBMasuGet(int layer, int id);
u32 MBMasuFlagGet(int layer, int id);
int MBMasuTypeGet(int layer, int id);
void MBMasuTypeSet(int layer, int id, int type);
int MBMasuCapsuleGet(int layer, int id);
void MBMasuCapsuleSet(int layer, int id, int capsuleNo);
void MBMasuDispMaskSet(int layer, u32 bit);
void MBMasuDispMaskReset(int layer, u32 bit);
BOOL MBMasuPosGet(int layer, int id, HuVecF *pos);
void MBMasuCornerPosGet(int id, int cornerNo, HuVecF *pos);
void MBMasuCornerRotPosGet(int id, int cornerNo, HuVecF *pos);
BOOL MBMasuRotGet(int layer, int id, HuVecF *rot);
int MBMasuFlagFind(int layer, u32 flag, u32 mask);
int MBMasuFlagMatchFind(int layer, u32 flag);
int MBMasuFlagPosGet(int layer, u32 flag, HuVecF *pos);
int MBMasuFlagFindLink(int layer, int masuId, u32 flag, u32 mask);
int MBMasuFlagMatchFindLink(int layer, int masuId, u32 flag);
int MBMasuLinkTblGet(int layer, int id, s16 *linkTbl);
int MBMasuTypeLinkGet(int layer, int id, u16 type);
int MBMasuCapsuleFind(int layer, int capsuleNo, int *list);
void MBMasuWalkPostHookSet(MASUPLAYERHOOK hook);
void MBMasuWalkPreHookSet(MASUPLAYERHOOK hook);
BOOL MBMasuWalkPostExec(int playerNo);
BOOL MBMasuWalkPreExec(int playerNo);
void MBMasuHatenaHookSet(MASUPLAYERHOOK hook);
void MBMasuTypeChange(u16 oldType, u16 newType);
void MBMasuDispSet(BOOL dispF);
void MBMasuCameraSet(u16 cameraBit);
void MBMasuSaiHiddenSet(void);
void MBMasuEffSet(int masuId, int effNo);
void MBMasuEffClear(void);
void MBMasuPlayerLightSet(int playerNo, BOOL dispLightF);
void MBMasuMtxSet(int masuId, Mtx matrix);
void MBMasuMtxGet(int masuId, Mtx matrix);
int MBMasuStarGet(int starNo);
BOOL MBMasuStarCheck(MASU *masuP);
BOOL MBMasuIdStarCheck(int masuId);
void MBMasuCapsuleClear(void);
void MBMasuPathCheckHookSet(MASUPATHCHECKHOOK hook);
int MBMasuPathCheck(s16 masuId, s16 *linkTbl, BOOL endF);
int MBMasuPathNoLoopCheck(s16 masuId, s16 *linkTbl);
int MBMasuPathTypeWrapLenGet(s16 masuId, s16 type, BOOL endF);
int MBMasuPathTypeLenGet(s16 masuId, s16 type);
int MBMasuPathFlagLenGet(s16 masuId, u32 flag, u32 mask);
int MBMasuPathFlagMatchLenGet(s16 masuId, u32 mask);
s16 MBMasuPathMasuGet(s16 masuId, s16 type);
s16 MBMasuPathMasuWrapGet(s16 masuId, s16 type, BOOL endF);
int MBMasuPathWrapLenGet(s16 masuId, s16 masuIdEnd, BOOL endF);
int MBMasuPathLenGet(s16 masuId, s16 masuIdEnd);
u32 MBMasuEventFlagGet(u32 flag, u32 mask);
u32 MBMasuDispMaskGet(void);
BOOL MBMasuDispGet(void);
s16 MBMasuCarTargetGet(s16 masuId, s16 playerNo);
s16 MBMasuTargetGet(s16 masuId);
s16 MBMasuCarNextGet(s16 masuId);
s16 MBMasuCarPrevGet(s16 masuId);
s16 MBMasuDonkeySet(s16 no);
s16 MBMasuDonkeyGet(s16 no);
s16 MBMasuDonkeyNumGet(void);

#endif