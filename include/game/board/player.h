#ifndef _BOARD_PLAYER_H
#define _BOARD_PLAYER_H

#include "game/board/model.h"
#include "game/board/audio.h"
#include "game/board/sai.h"

#include "game/gamework.h"
#include "game/charman.h"
#include "game/object.h"
#include "game/pad.h"

#define MB_PLAYER_MOT_IDLE 1
#define MB_PLAYER_MOT_WALK 2
#define MB_PLAYER_MOT_RUN 3
#define MB_PLAYER_MOT_JUMP 4
#define MB_PLAYER_MOT_JUMPEND 5
#define MB_PLAYER_MOT_DIZZY 6
#define MB_PLAYER_MOT_WIN 7
#define MB_PLAYER_MOT_LOSE 8
#define MB_PLAYER_MOT_SHOCK 9
#define MB_PLAYER_MOT_FLATTEN 10
#define MB_PLAYER_MOT_JUMPSAI 11
#define MB_PLAYER_MOT_COINWIN 12
#define MB_PLAYER_MOT_COINLOSE 13
#define MB_PLAYER_MOT_CLIMB 14

#define MB_PLAYER_MOT_COUNT 14

#define MB_PLAYER_CAR_MOT_IDLE 1
#define MB_PLAYER_CAR_MOT_IDLE2 2
#define MB_PLAYER_CAR_MOT_JUMP 3
#define MB_PLAYER_CAR_MOT_WIN 4
#define MB_PLAYER_CAR_MOT_LOSE 5
#define MB_PLAYER_CAR_MOT_LOOK 6
#define MB_PLAYER_CAR_MOT_LOOKUP 7

#define MB_PLAYER_CAR_MOT_NUM 7

#define MB_PLAYER_MODE_DEFAULT 0
#define MB_PLAYER_MODE_ADDCAPSULE 1
#define MB_PLAYER_MODE_SAIEXEC 2
#define MB_PLAYER_MODE_WALK 3
#define MB_PLAYER_MODE_KETTOU_CHECK 4
#define MB_PLAYER_MODE_WALKEND 5
#define MB_PLAYER_MODE_WALKEND_EVENT 6
#define MB_PLAYER_MODE_ROT_PLAYER 7

#define MB_SAIMODE_NORMAL 0
#define MB_SAIMODE_KINOKO 1
#define MB_SAIMODE_SKINOKO 2
#define MB_SAIMODE_NKINOKO 3
#define MB_SAIMODE_KILLER 4

#define MBPlayerDispSet2(playerNo, dispF) MBModelDispSet(MBPlayerModelGet(playerNo), dispF)

typedef BOOL (*MBPLAYERTURNHOOK)(void);

typedef struct MBPlayerWork_s {
    MBMODELID modelId;
    s16 motNo;
    s8 unk4;
    s8 masuCorner;
    s16 masuNext;
    unsigned moveF : 1;
    unsigned playerNo : 2;
    unsigned moveEndF : 1;
    OMOBJ *rotateObj;
    OMOBJ *walkObj;
    OMOBJ *posFixObj;
    OMOBJ *walkNumObj;
    MBPLAYERTURNHOOK preTurnHook;
    MBPLAYERTURNHOOK postTurnHook;
    HUPROCESS *walkProc;
    HSFMATERIAL *matCopy;
} MBPLAYERWORK;

static inline int MBPlayerHandicapGet(int playerNo)
{
    return GwPlayer[playerNo].handicap;
}

static inline void MBPlayerHandicapSet(int playerNo, int handicap)
{
    GwPlayer[playerNo].handicap = handicap;
}

static inline int MBPlayerGrpGet(int playerNo)
{
    return GwPlayer[playerNo].grp;
}

void MBPlayerInit(BOOL noEventF);
void MBPlayerClose(void);
MBPLAYERWORK *MBPlayerWorkGet(int playerNo);
void MBPlayerPreTurnHookSet(int playerNo, MBPLAYERTURNHOOK hook);
void MBPlayerPostTurnHookSet(int playerNo, MBPLAYERTURNHOOK hook);
void MBTurnExecParty(int playerNo);
void MBTurnExecStory(int playerNo);
int MBPlayerSaiTypeGet(int saiMode);
void MBPlayerMasuWalkFrom(int playerNo, int masuId, BOOL waitF);
void MBPlayerMasuWalk(int playerNo, BOOL waitF);
void MBPlayerMasuWalkPos(int playerNo, HuVecF *pos, BOOL waitF);
void MBPlayerMasuWalkSpeed(int playerNo, int masuId, s16 maxTime, BOOL waitF);
void MBPlayerWalkExec(int playerNo, HuVecF *srcPos, HuVecF *dstPos, s16 maxTime, HuVecF *rot, BOOL waitF);
void MBPlayerWalkMain(int playerNo, HuVecF *srcPos, HuVecF *dstPos, u32 motId, float motSpeed, u32 attr, s16 maxTime, HuVecF *rot, BOOL waitF);
void MBPlayerRotateStart(int playerNo, s16 endAngle, s16 maxTime);
BOOL MBPlayerRotateCheck(int playerNo);
BOOL MBPlayerRotateCheckAll(void);
void MBPlayerSaiMotExec(int playerNo);
void MBWalkNumCreateColor(int playerNo, BOOL carF, int color);
void MBWalkNumCreate(int playerNo, BOOL carF);
void MBWalkNumKill(int playerNo);
void MBWalkNumDispSet(int playerNo, BOOL dispF);
void MBPlayerPosFix(int *masuIdTbl, BOOL snapF);
void MBPlayerPosFixWarp(int playerNo, int masuId, BOOL snapF);
void MBPlayerPosFixPlayer(int playerNo, int masuId, BOOL snapF);
BOOL MBPlayerPosFixCheck(void);
void MBPlayerPosFixEvent(int playerNo, int masuId, BOOL snapF);
void MBPlayerPosFixSet(int playerNo, int masuId);
void MBPlayerPosFixFlagSet(BOOL fixF);
void MBPlayerPosFixCurr(int masuId, BOOL snapF);
void MBPlayerPosFixOrderSet(void);
void MBPlayerEyeMarDarkSet(int playerNo, BOOL darkF);
void MBPlayerMatClone(int playerNo);
u32 MBPlayerNameMesGet(int playerNo);
char *MBPlayerNameGet(int playerNo);
void MBPlayerAmbSet(int playerNo, float ambR, float ambG, float ambB);
MBMODELID MBPlayerModelGet(int playerNo);
BOOL MBPlayerAllComCheck(void);
int MBPlayerGrpFind(int playerNo);
int MBPlayerGrpFindOther(int playerNo);
int MBPlayerGrpFindPlayer(int grpNo, int memberNo);
float MBPlayerWalkSpeedGet(void);
void MBPlayerLayerSet(int playerNo, int layer);
void MBPlayerCameraSet(int playerNo, u16 bit);
void MBPlayerUnkSpeedSet(int playerNo, float speed);
void MBPlayerStubValSet(int playerNo, BOOL value);
void MBPlayerPosReset(int playerNo);
void MBPlayerPosRestoreAll(void);
void MBPlayerMtxSet(int playerNo, Mtx *matrix);
void MBPlayerPosSetV(int playerNo, HuVecF *pos);
void MBPlayerPosSet(int playerNo, float posX, float posY, float posZ);
void MBPlayerPosGet(int playerNo, HuVecF *pos);
void MBPlayerRotSetV(int playerNo, HuVecF *rot);
void MBPlayerRotSet(int playerNo, float rotX, float rotY, float rotZ);
void MBPlayerRotGet(int playerNo, HuVecF *rot);
void MBPlayerRotYSet(int playerNo, float rotY);
float MBPlayerRotYGet(int playerNo);
void MBPlayerScaleSetV(int playerNo, HuVecF *scale);
void MBPlayerScaleSet(int playerNo, float scaleX, float scaleY, float scaleZ);
void MBPlayerScaleGet(int playerNo, HuVecF *scale);
void MBPlayerMotionNoSet(int playerNo, int motNo, u32 attr);
int MBPlayerMotionNoGet(int playerNo);
void MBPlayerMotionNoShiftSet(int playerNo, int motNo, float start, float end, u32 attr);
int MBPlayerMotionCreate(int playerNo, int dataNum);
void MBPlayerMotionKill(int playerNo, int motNo);
void MBPlayerMotionSpeedSet(int playerNo, float speed);
void MBPlayerMotionTimeSet(int playerNo, float time);
float MBPlayerMotionTimeGet(int playerNo);
float MBPlayerMotionMaxTimeGet(int playerNo);
void MBPlayerMotionStartEndSet(int playerNo, float start, float end);
void MBPlayerModelAttrSet(int playerNo, u32 modelAttr);
void MBPlayerModelAttrReset(int playerNo, u32 modelAttr);
void MBPlayerMotionVoiceOnSet(int playerNo, int motNo, BOOL voiceOn);
BOOL MBPlayerMotionEndCheck(int playerNo);
BOOL MBPlayerMotionEndCheckAll(void);
void MBPlayerMotionEndWait(int playerNo);
void MBPlayerMotIdleSet(int playerNo);
void MBPlayerCoinSet(int playerNo, int coinNum);
int MBPlayerCoinGet(int playerNo);
void MBPlayerCoinAdd(int playerNo, int coinNum);
void MBPlayerGrpCoinSet(int grp, int coinNum);
s16 MBPlayerGrpCoinGet(int grp);
int MBPlayerMaxCoinGet();
void MBPlayerStarSet(int playerNo, int starNum);
int MBPlayerStarGet(int playerNo);
void MBPlayerStarAdd(int playerNo, int starNum);
void MBPlayerGrpStarSet(int grp, int starNum);
s16 MBPlayerGrpStarGet(int grp);
int MBPlayerBestPathGet(void);
int MBPlayerRankGet(int playerNo);
s16 MBPlayerGrpRankGet(int grp);
void MBPlayerCapsuleUseSet(int capsuleNo);
int MBPlayerCapsuleUseGet(void);
int MBPlayerCapsuleMaxGet(void);
int MBPlayerCapsuleAdd(int playerNo, int capsuleNo);
int MBPlayerCapsuleRemove(int playerNo, int index);
int MBPlayerCapsuleFind(int playerNo, int capsuleNo);
s8 MBPlayerCapsuleGet(int playerNo, int index);
s8 MBPlayerGrpCapsuleGet(int grp, int index);
int MBPlayerCapsuleNumGet(int playerNo);
int MBPlayerGrpCapsuleNumGet(int grp);
BOOL MBPlayerAliveCheck(int playerNo);
BOOL MBPlayerDeadCheck(int playerNo);
int MBPlayerAliveGet(int *playerNo);
int MBPlayerAlivePlayerGet(int *playerNo);
int MBPlayerDeadGet(int *playerNo);
int MBPlayerDeadComGet(int *playerNo);
BOOL MBPlayerMoveCheck(int playerNo);
BOOL MBPlayerStoryLoseCheck(int playerNo);
void MBPlayerStoryAliveReset(int playerNo);
void MBPlayerStoryLoseExec(void);
BOOL MBPlayerKettouCheck(int playerNo, s16 masuId);
BOOL MBPlayerKettouStoryCheck(int playerNo);
BOOL MBPlayerStoryPlayerCheck(int playerNo);
BOOL MBPlayerStoryComCheck(int playerNo);
int MBPlayerStoryPlayerGet(void);
void MBPlayerWinLoseVoicePlay(int playerNo, int motNo, int seId);
int MBPlayerVoicePanPlay(int playerNo, s16 seId);
int MBPlayerVoicePlay(int playerNo, s16 seId);
void MBPlayerDispSet(int playerNo, BOOL dispF);
GXColor MBPlayerColorGet(int playerNo);
void MBPlayerBlackoutSet(BOOL value);
BOOL MBPlayerBlackoutGet(void);

void MBPlayerMasuCornerSet(int playerNo, s8 cornerNo);
s8 MBPlayerMasuCornerGet(int playerNo);

#endif