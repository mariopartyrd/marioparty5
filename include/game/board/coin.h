#ifndef _BOARD_COIN_H
#define _BOARD_COIN_H

#include "game/board/main.h"
#include "game/board/model.h"

#define MBCOIN_EFF_NONE -1
#define MBCOIN_DISP_NONE -1

#define MBCOIN_ATTR_NONE 0
#define MBCOIN_ATTR_IDLE (1 << 0)

typedef struct mbCoinEff_s MBCOINEFF;
typedef void (*MBCOINEFFHOOK)(MBCOINEFF *eff);

typedef struct mbCoinEffData_s {
    u8 dispF : 1;
    u8 hitF : 1;
    u8 timeEndF : 1;
    MBMODELID modelId;
    int no;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    HuVecF vel;
    s16 timeHit;
    s16 time;
    HU3DPARMANID parManId;
} MBCOINEFFDATA;

struct mbCoinEff_s {
    int effNo;
    int count;
    u32 attr;
    MBCOINEFFDATA *data;
    HUPROCESS *proc;
    MBCOINEFFHOOK hook;
    int work[4];
};


void MBCoinInit(void);
void MBCoinClose(void);
s16 MBCoinEffCreate(int count, MBCOINEFFHOOK hook);
void MBCoinEffKill(s16 effNo);
BOOL MBCoinEffCheck(s16 effNo);
MBCOINEFF *MBCoinEffGet(s16 effNo);
void MBCoinPosSetV(MBMODELID modelId, HuVecF *pos);
void MBCoinPosSet(MBMODELID modelId, float posX, float posY, float posZ);
void MBCoinPosGet(MBMODELID modelId, HuVecF *pos);
void MBCoinRotSetV(MBMODELID modelId, HuVecF *rot);
void MBCoinRotSet(MBMODELID modelId, float rotX, float rotY, float rotZ);
void MBCoinRotGet(MBMODELID modelId, HuVecF *rot);
void MBCoinScaleSetV(MBMODELID modelId, HuVecF *scale);
void MBCoinScaleSet(MBMODELID modelId, float scaleX, float scaleY, float scaleZ);
void MBCoinScaleGet(MBMODELID modelId, HuVecF *scale);
void MBCoinEffAttrSet(s16 effNo, u32 attr);
void MBCoinEffAttrReset(s16 effNo, u32 attr);
HU3DPARMANID MBCoinHitExec(MBMODELID modelId);
void MBCoinEffDispOn(s16 effNo, s16 coinNo);
HU3DPARMANID MBCoinParManCreate(void);
void MBCoinParManKill(HU3DPARMANID parManId);
s16 MBCoinDispCreate(HuVecF *pos, int coinNum, BOOL playSe);
s16 MBCoinDispCreateSe(HuVecF *pos, int coinNum);
void MBCoinDispKill(s16 no);
BOOL MBCoinDispKillCheck(s16 no);

int MBCoinAddProcExec(int playerNo, int coinNum, BOOL dispF, BOOL fastF);
int MBCoinAddDispExec(int playerNo, int coinNum, BOOL dispF);
int MBCoinAddExec(int playerNo, int coinNum);
void MBCoinAddAllProcExec(int coinNumP1, int coinNumP2, int coinNumP3, int coinNumP4, BOOL dispF, BOOL fastF);
void MBCoinAddAllDispExec(int coinNumP1, int coinNumP2, int coinNumP3, int coinNumP4, BOOL dispF);
void MBCoinAddAllExec(int coinNumP1, int coinNumP2, int coinNumP3, int coinNumP4);

#endif