#ifndef _BOARD_CAPSULE_H
#define _BOARD_CAPSULE_H

#include "dolphin/types.h"

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
#define CAPSULE_NULL -1

void MBCapsuleOpeningAdd(s32 capsuleNum);
void MBCapsuleBowserAdd(void);
void MBKettouMgEndExec(s32 playerNo);
void MBCircuitKettouMgEndExec(s32 playerNo);
void MBDonkeyMgEndExec(s32 playerNo);
void MBKupaMgEndExec(s32 playerNo);
void MBCircuitKupaMgEndExec(s32 playerNo);
void MBVsMgEndExec(s32 playerNo);
void MBLast5Exec(void);
void MBKupaExec(s32 playerNo);
void MBDonkeyExec(s32 playerNo);

void MB3MiracleGetExec(s32 playerNo);
void MBCapMachineCreateAll(void);
void MBCapMachineExec(int playerNo);
int MBCapsuleMasuPatNoGet(int capsuleNo);
BOOL MBCapsuleExec(s32 playerNo, int capsuleNo, BOOL circuitF, BOOL walkF);
void MBSaiHiddenExec(s32 playerNo);

#endif