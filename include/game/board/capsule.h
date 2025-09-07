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

void MBCapsuleOpeningAdd(int capsuleNum);
void MBCapsuleBowserAdd(void);
void MBKettouMgEndExec(int playerNo);
void MBCircuitKettouMgEndExec(int playerNo);
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
BOOL MBCapsuleExec(int playerNo, int capsuleNo, BOOL circuitF, BOOL walkF);
void MBSaiHiddenExec(int playerNo);
void MBKillerExec(int playerNo);
void MBKettouExec(int playerNo);

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
int MBBankCoinGet(void);
int MBCapsuleNextGet(int rank);
int MBCapsuleCodeGet(int capsuleNo);

#endif