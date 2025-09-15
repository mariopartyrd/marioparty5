#ifndef _BOARD_CAPSULE_H
#define _BOARD_CAPSULE_H

#include "dolphin/types.h"
#include "game/object.h"
#include "game/board/model.h"

#define CAPSULE_KINOKO 0
#define CAPSULE_SKINOKO 1
#define CAPSULE_NKINOKO 2
#define CAPSULE_DOKAN 3
#define CAPSULE_JANGO 4
#define CAPSULE_BOBLE 5
#define CAPSULE_HANACHAN 6
#define CAPSULE_HAMMERBRO 10
#define CAPSULE_COINBLOCK 11
#define CAPSULE_TOGEZO 12
#define CAPSULE_PATAPATA 13
#define CAPSULE_KILLER 14
#define CAPSULE_KURIBO 15
#define CAPSULE_BOMHEI 16
#define CAPSULE_BANK 17
#define CAPSULE_KAMEKKU 20
#define CAPSULE_THROWMAN 21
#define CAPSULE_PAKKUN 22
#define CAPSULE_KOKAMEKKU 23
#define CAPSULE_UKKI 24
#define CAPSULE_JUGEM 25
#define CAPSULE_TUMUJIKUN 30
#define CAPSULE_DUEL 31
#define CAPSULE_WANWAN 32
#define CAPSULE_HONE 33
#define CAPSULE_KOOPA 34
#define CAPSULE_CHANCE 35
#define CAPSULE_MIRACLE 36
#define CAPSULE_DONKEY 37
#define CAPSULE_VS 38
#define CAPSULE_DEBUGCAM 40
#define CAPSULE_DEBUGWARP 41
#define CAPSULE_DEBUGSETPOS 42
#define CAPSULE_MAX 43
#define CAPSULE_NULL -1
#define CAPSULE_INVALID -99

#define CAPSULE_MASUPAT_KINOKO 0
#define CAPSULE_MASUPAT_MOVE 1
#define CAPSULE_MASUPAT_COIN 2
#define CAPSULE_MASUPAT_WARN 3
#define CAPSULE_MASUPAT_BOMHEI 4
#define CAPSULE_MASUPAT_BANK 5
#define CAPSULE_MASUPAT_TUMUJIKUN 6
#define CAPSULE_MASUPAT_KETTOU 7
#define CAPSULE_MASUPAT_WANWAN 8
#define CAPSULE_MASUPAT_CHANCE 9
#define CAPSULE_MASUPAT_NONE 10

#define CAPSULE_COLOR_GREEN 0
#define CAPSULE_COLOR_YELLOW 1
#define CAPSULE_COLOR_RED 2
#define CAPSULE_COLOR_BLUE 3
#define CAPSULE_COLOR_MAX 4

#define CAPSULE_OBJ_MAX 128

#define MBCapsuleEffRandF() ((frand() & 0x7F)*(1.0f/127.0f))
#define MBCapsuleEffRand(max) ((frand() & 0x7FFF)%max)

typedef void (*CAPSULE_EFF_HOOK)(int capsuleNo, int type, int mdlId, BOOL flag1, BOOL flag2, BOOL flag3);

void MBCapsuleOpeningAdd(int capsuleNum);
void MBCapsuleBowserAdd(void);
void MBKettouMgEndExec(int playerNo);
BOOL MBCircuitKettouMgEndExec(int playerNo);
void MBDonkeyMgEndExec(int playerNo);
void MBKupaMgEndExec(int playerNo);
void MBCircuitKupaMgEndExec(int playerNo);
void MBVsMgEndExec(int playerNo);
void MBLast5Exec(void);
void MBKupaExec(int playerNo);
void MBDonkeyExec(int playerNo);
void MBCapsuleKupaGetExec(int playerNo);

void MB3MiracleGetExec(int playerNo);
void MBCapMachineCreateAll(void);
void MBCapMachineExec(int playerNo);
int MBCapsuleMasuPatNoGet(int capsuleNo);
BOOL MBCapsuleExec(int playerNo, int capsuleNo, BOOL circuitF, BOOL masuF);
void MBSaiHiddenExec(int playerNo);
void MBKillerExec(int playerNo);
void MBKettouExec(int playerNo);
void MBKillerEventExec(int playerNo);
BOOL MBKillerEventCheck(int playerNo);
BOOL MBKillerEventCheckAll(void);

void MBKillerMultiExec(int *playerNo);

int MBCapsuleSelExec(void);
int MBCapsuleColorNoGet(int capsuleNo);

void MBCapsuleSelStoryExec(void);
void MBCapsuleAddStory(void);

int MBCapsuleColCreate(int dataNum);

void MBCapMachineTutorialExec(void);

void MBDonkeyOpeningExec(void);

unsigned int MBCapsuleMdlGet(int capsuleNo);
int MBCapsuleCostGet(int capsuleNo);

BOOL MBCapsuleUseExec(int playerNo, int capsuleNo);
int MBComCapsuleSelGet(int playerNo);
float MBCapsuleSelModelUpdate(HuVecF *posStart, HuVecF *posEnd, MBMODELID modelId, float weight);
int MBCapsuleWinCreate(int capsuleNo);

void MBCapMachineObjCreate(int masuId);

int MBCapsuleObjCreate(int capsuleNo, BOOL linkF);
void MBCapsuleObjPosSet(int id, float posX, float posY, float posZ);
void MBCapsuleObjPosSetV(int id, HuVecF *pos);
void MBCapsuleObjRotSet(int id, float rotX, float rotY, float rotZ);
void MBCapsuleObjRotSetV(int id, HuVecF *rot);

void MBCapsuleObjScaleSet(int id, float scaleX, float scaleY, float scaleZ);
void MBCapsuleObjScaleSetV(int id, HuVecF *scale);

void MBCapsuleObjKill(int id);
void MBCapsuleObjLayerSet(int id, u8 layer);
void MBCapsuleObjMtxGet(int id, Mtx *mtx);
void MBCapsuleObjMtxSet(int id, Mtx *mtx);
void MBCapsuleObjDispSet(int id, BOOL dispF);
u8 MBCapsuleObjLayerGet(int id);
//void MBCapsuleObjAttrSet(int id, u32 attr);
//void MBCapsuleObjAttrReset(int id, u32 attr);
void MBCapsuleObjAlphaSet(int id, u8 alpha);

void MBCapsuleMasuSet(int masuId, int capsuleNo);

u32 MBCapsuleMesGet(int capsuleNo);

int MBCapsuleListGet(int *capsuleNo);
BOOL MBCapsuleColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out);
void MBCapMachineInit(void);
void MBCapsuleObjInit(void);
void MBCapMachineClose(void);
void MBCapsuleThrowAutoExec(int masuId, int capsuleNo, HuVecF *posA, HuVecF *posB, HuVecF *masuPos);
BOOL MBCapsuleThrowAutoCheck(void);
void MBCapsulePosSetExec(void);

BOOL MBCapsuleNoUseCheck(int capsuleNo);
BOOL MBCapsulePosAutoSelCheck(int playerNo, int capsuleNo);

void MBMasuBomheiSet(int masuId);
void MBMasuBomheiNumSet(int masuId, int num);
int MBMasuBomheiNumGet(int masuId);

int MBBankCoinGet(void);
void MBBankCoinAdd(int coin);
int MBCapsuleNextGet(int rank);
int MBCapsuleCodeGet(int capsuleNo);

void MBCapsuleEffHookSet(CAPSULE_EFF_HOOK effHook);

BOOL MBCapsuleStatusDispCheck(int playerNo);


void MBCapsuleKinokoExec(void);
void MBCapsuleKinokoDestroy(void);
void MBCapsuleSKinokoExec(void);
void MBCapsuleSKinokoDestroy(void);
void MBCapsuleNKinokoExec(void);
void MBCapsuleNKinokoDestroy(void);
void MBCapsuleDokanExec(void);
void MBCapsuleDokanDestroy(void);
void MBCapsuleJangoExec(void);
void MBCapsuleJangoDestroy(void);
void MBCapsuleBobleExec(void);
void MBCapsuleBobleDestroy(void);
void MBCapsuleHanachanExec(void);
void MBCapsuleHanachanDestroy(void);

void MBCapsuleHammerBroExec(void);
void MBCapsuleHammerBroDestroy(void);
void MBCapsuleCoinBlockExec(void);
void MBCapsuleCoinBlockDestroy(void);
void MBCapsuleTogezoExec(void);
void MBCapsuleTogezoDestroy(void);
void MBCapsulePatapataExec(void);
void MBCapsulePatapataDestroy(void);
void MBCapsuleKillerExec(void);
void MBCapsuleKillerDestroy(void);
void MBCapsuleKuriboExec(void);
void MBCapsuleKuriboDestroy(void);
void MBCapsuleBomheiExec(void);
void MBCapsuleBomheiDestroy(void);
void MBCapsuleBankExec(void);
void MBCapsuleBankDestroy(void);

void MBCapsuleKamekkuExec(void);
void MBCapsuleKamekkuDestroy(void);
void MBCapsuleThrowmanExec(void);
void MBCapsuleThrowmanDestroy(void);
void MBCapsulePakkunExec(void);
void MBCapsulePakkunDestroy(void);
void MBCapsuleKokamekkuExec(void);
void MBCapsuleKokamekkuDestroy(void);
void MBCapsuleUkkiExec(void);
void MBCapsuleUkkiDestroy(void);
void MBCapsuleJugemExec(void);
void MBCapsuleJugemDestroy(void);

void MBCapsuleTumujikunExec(void);
void MBCapsuleTumujikunDestroy(void);
void MBCapsuleKettouExec(void);
void MBCapsuleKettouDestroy(void);
void MBCapsuleWanwanExec(void);
void MBCapsuleWanwanDestroy(void);
void MBCapsuleHoneExec(void);
void MBCapsuleHoneDestroy(void);
void MBCapsuleKoopaExec(void);
void MBCapsuleKoopaDestroy(void);
void MBCapsuleChanceExec(void);
void MBCapsuleChanceDestroy(void);
void MBCapsuleMiracleExec(void);
void MBCapsuleMiracleDestroy(void);
void MBCapsuleDonkeyExec(void);
void MBCapsuleDonkeyDestroy(void);
void MBCapsuleVsExec(void);
void MBCapsuleVsDestroy(void);

void MBCapsuleDebugCamExec(void);
void MBCapsuleDebugCamDestroy(void);
void MBCapsuleDebugWarpExec(void);
void MBCapsuleDebugWarpDestroy(void);
void MBCapsuleDebugPosSetExec(void);
void MBCapsuleDebugPosSetDestroy(void);

void MBCapsuleKillerMoveP1Exec(void);
void MBCapsuleKillerMoveP1Destroy(void);
void MBCapsuleKillerMoveP2Exec(void);
void MBCapsuleKillerMoveP2Destroy(void);
void MBCapsuleKillerMoveP3Exec(void);
void MBCapsuleKillerMoveP3Destroy(void);
void MBCapsuleKillerMoveP4Exec(void);
void MBCapsuleKillerMoveP4Destroy(void);
void MBCapsuleKillerMoveExec(int playerNo, BOOL flag);

void MBCapsuleStatusPosMoveWait(BOOL dispF, BOOL waitF);
void MBCapsuleStatusInSet(int leftPlayer, int rightPlayer, BOOL waitF);
void MBCapsuleStatusOutSet(int leftPlayer, int rightPlayer, BOOL waitF);


void MBCapsuleCameraViewPlayerSet(int playerNo);
void MBCapsuleCameraViewSet(int playerNo, int viewNo, BOOL masuF);
void MBCapsuleCameraViewNoSet(int playerNo, int viewNo);
BOOL MBCapsuleTeamCheck(int player1, int player2);
void MBCapsuleDataDirRead(int capsuleNo);
void MBPlayerMoveInit(void);
void MBPlayerMoveObjCreate(void);
void MBPlayerMoveHitCreate(int playerNo, BOOL shockF, BOOL dizzyF);
void MBPlayerMoveEjectCreate(int playerNo, BOOL dizzyF);
BOOL MBPlayerMoveObjCheck(int playerNo);

int MBCapsuleNoGet(void);
int MBKettouCoinLoseGet(void);
int MBKettouOppCoinLoseGet(void);
int MBKettouCoinLoseGet2(void);
int MBKettouOppCoinLoseGet2(void);
void MBCapsuleEffHookStoryCall(int evtType, int type, int mdlId);
void MBBankCoinReset(void);
int MBCapsuleStarCreate(void);
void MBCapsuleStarKill(int mdlId);
void MBCapsuleStarPosSet(int mdlId, float x, float y, float z);
void MBCapsuleStarRotSet(int mdlId, float x, float y, float z);
void MBCapsuleStarScaleSet(int mdlId, float x, float y, float z);
void MBCapsuleStarObjExec(OMOBJ *obj);
s16 MBCapsulePlayerMotionCreate(int playerNo, int fileNum);
int MBCapsuleModelCreate(int fileNum, int *motTbl, BOOL linkF, int delay, BOOL closeDir);
void MBCapsuleModelKill(int mdlId);
s16 MBCapsuleSprCreate(unsigned int dataNum, s16 prio, s16 bank);
void MBCapsuleSprClose(s16 sprId);
void *MBCapsuleMalloc(int size);

void MBCapsuleExplodeEffCreate(int no);
void MBCapsuleGlowEffCreate(void);
int MBCapsuleExplodeEffAdd(int no, HuVecF pos, HuVecF rot, float size, float speed, float animSpeed, GXColor color);
int MBCapsuleExplodeEffLineAdd(int no, HuVecF pos, HuVecF rot, float size, float speed, float radius, float animSpeed, GXColor color);
int MBCapsuleExplodeEffCheck(int no);

void MBCapsuleDustExplodeAdd(int no, HuVecF pos);
void MBCapsuleDustCloudAdd(int no, HuVecF pos);

int MBCapsuleGlowEffAdd(HuVecF pos, HuVecF vel, float scale, float fadeSpeed, float rotSpeed, float gravity, GXColor color);
void MBCapsuleHanachanGlowEffAdd(HuVecF pos, float scale, float fadeSpeed, float radiusX, float radiusY, float radiusZ, int mode);

int MBCapsuleGlowEffCheck(void);
void MBCapsuleExplodeEffKill(int no);
void MBCapsuleGlowEffKill(void);
void MBCapsuleGlowEffBlendModeSet(int blendMode);
void MBCapsuleNKinokoEffCreate(void);
void MBCapsuleNKinokoEffKill(void);
int MBCapsuleNKinokoEffAdd(HuVecF pos, HuVecF vel, float scale, float rotSpeed, int maxTime, GXColor color);
int MBCapsulePlayerAliveFind(int playerNo);
void MBCapsulePlayerMotSet(int playerNo, int motNo, u32 attr, BOOL shiftF);
void MBCapsulePlayerIdleWait(void);
BOOL MBCapsulePlayerMotShiftCheck(int playerNo);
void MBCapsuleModelMotSet(int mdlId, int motNo, u32 attr, BOOL shiftF);
BOOL MBCapsulePlayerMasuCheck(int playerNo, HuVecF *a, HuVecF *b, HuVecF *out);
void MBCapsuleTimingHookCreate(HU3DMODELID modelId, HU3DMOTID motId, BOOL lagF);
void MBCapsuleBezierGetV(float t, float *a, float *b, float *c, float *out);
void MBCapsuleBezierNormGetV(float t, float *a, float *b, float *c, float *out);
void MBCapsuleHermiteGetV(float t, HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, HuVecF *out);
float MBCapsuleAngleLerp(float a, float b, float t);
float MBCapsuleAngleWrap(float a, float b);
float MBCapsuleAngleSumLerp(float t, float a, float b);

void MBCapsuleLoseEffCreate(void);
void MBCapsuleLoseEffKill(void);
int MBCapsuleLoseEffAdd(HuVecF *pos, HuVecF *vel, float scale, int maxTime, int capsuleNo);
void MBCapsuleLoseEffAddMulti(int playerNo, int max, float scale);
int MBCapsuleLoseEffNumGet(void);

s16 MBCapsuleMasuNextRegularGet(s16 masuId, HuVecF *pos);
s16 MBCapsuleMasuNextGet(s16 masuId, HuVecF *pos);
void MBPlayerMoveObjKill(int playerNo);

void MBCapsuleRingEffCreate(void);
void MBCapsuleRingEffKill(void);
void MBCapsuleRayEffCreate(void);
void MBCapsuleRayEffKill(void);
void MBCapsuleHanachanRingCreate(void);
void MBCapsuleHanachanRingKill(void);
int MBCapsuleHanachanRingAdd(HuVecF *pos, HuVecF *dir, int inTime, int holdTime, int outTime, GXColor color);
void MBCapsuleHanachanRingClear(void);
void MBCapsuleChoiceSet(int choice);
int MBCapsuleRingEffAdd(HuVecF pos, HuVecF rot, HuVecF vel, int inTime, int rotTime, int no, GXColor color);
int MBCapsuleRayEffAdd(HuVecF pos, HuVecF rot, HuVecF moveDir, int speed, GXColor color);

void MBCapsuleCoinEffCreate(void);
void MBCapsuleCoinEffKill(void);
int MBCapsuleCoinEffNumGet(void);
int MBCapsuleCoinEffAdd(HuVecF *pos, HuVecF *rot, float scale, float gravity, int maxTime, int mode);
int MBCapsuleCoinEffMaxYSet(int id, float maxY);

void MBCapsuleModelMtxSet(MBMODELID mdlId, Mtx *matrix);
int MBCapsuleCoinDispExec(int playerNo, int coinNum, BOOL winMotF, BOOL waitF);

OMOBJ *MBCapsuleSaiHiddenCreate(int playerNo, int mdlId, int effDelay, BOOL rotF, BOOL skipF);
void MBCapsuleSaiHiddenStartSet(OMOBJ *obj, BOOL startF);
void MBCapsuleSaiHiddenKill(OMOBJ *obj);
BOOL MBCapsuleSaiHiddenKillCheck(OMOBJ *obj);
BOOL MBCapsuleSaiHiddenInCheck(OMOBJ *obj);
BOOL MBCapsuleSaiHiddenHitCheck(OMOBJ *obj, HuVecF *pos);
void MBCapsuleSeDelayPlay(int seId, int delay);

void MBCapsuleCoinManCreate(void);
void MBCapsuleCoinManKill(void);
int MBCapsuleCoinManAdd(BOOL downF, int playerNo, int num);
int MBCapsuleCoinManNumGet(void);
 
#endif