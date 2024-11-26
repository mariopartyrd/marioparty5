#ifndef _BOARD_MAIN_H
#define _BOARD_MAIN_H

#include "game/object.h"
#include "game/process.h"
#include "game/gamework.h"
#include "game/data.h"
#include "datanum/board.h"

#define MBNO_WORLD_1 0
#define MBNO_WORLD_2 1
#define MBNO_WORLD_3 2
#define MBNO_WORLD_4 3
#define MBNO_WORLD_5 4
#define MBNO_WORLD_6 5
#define MBNO_WORLD_7 6
#define MBNO_TUTORIAL 7
#define MBNO_CIRCUIT 8
#define MBNO_MAX 9

#define MB_LAST5_EFF_NONE 0
#define MB_LAST5_EFF_COINMUL 1
#define MB_LAST5_EFF_RANDCAPSULE 2
#define MB_LAST5_EFF_REDBOWSER 3
#define MB_LAST5_EFF_5STAR 4

typedef void (*MBCREATEHOOK)(void);
typedef void (*MBKILLHOOK)(void);
typedef void (*MBLIGHTHOOK)(void);
typedef void (*MBTURNHOOK)(void);

extern OMOBJMAN *mbObjMan;
extern HUPROCESS *mbMainProcess;

static inline int MBBoardNoGet(void)
{
	return GwSystem.boardNo;
}

#define MBAddObj(prio, mdlcnt, mtncnt, objFunc) omAddObj(mbObjMan, prio, mdlcnt, mtncnt, objFunc)
#define MBDelObj(obj) omDelObj(HuPrcCurrentGet(), obj)

#define MBPrcCreate(func, prio, stackSize) HuPrcChildCreate(func, prio, stackSize, 0, mbMainProcess)
#define MBPrcDestructorSet(proc, func) HuPrcDestructorSet2(proc, func)

//Translate data numbers in board.bin
#define MBDATANUM(dataNum) (dataNum)

void MBPauseWatchProcCreate(void);
void MBPauseWatchProcStop(void);
BOOL MBKillCheck(void);
HUPROCESS *MBPauseWatchProcGet(void);
void MBObjectSetup(int boardNo, MBCREATEHOOK createHook, MBKILLHOOK killHook);
void MBOvlGoto(OMOVL ovl);
BOOL MBEventMgCheck(void);
void MBDataClose(void);
void MBLightHookSet(MBLIGHTHOOK setHook, MBLIGHTHOOK resetHook);
void MBLightSet(void);
void MBLightReset(void);
void MBPauseFlagSet(void);
void MBPauseFlagClear(void);
BOOL MBPauseFlagCheck(void);
s32 MBDataDirReadAsync(int dataNum);
void MBDataAsyncWait(s32 statId);
void MBPreTurnHookSet(MBTURNHOOK turnHook);
void MBPostTurnHookSet(MBTURNHOOK turnHook);
void fn_8008D448(int value);
void fn_8008D450(int value);
int fn_8008D458(void);
int fn_8008D460(void);
void MBPartySaveInit(int boardNo);
void MBStorySaveInit(int mgPack, int storyDif);
void MBPlayerSaveInit(BOOL teamF, BOOL bonusStarF, int mgPack, int turnMax, int handicapP1, int handicapP2, int handicapP3, int handicapP4);
float MBVecDistXZ(HuVecF *a, HuVecF *b);
BOOL MBVecDistXZCheck(HuVecF *a, HuVecF *b, float maxDist);
float MBAngleWrap(float angle);
void MBAngleWrapV(HuVecF *angle);
BOOL MBAngleAdd(float *dest, float angle, float speed);
BOOL MBAngleMoveTo(float *dest, float angle, float speed);
BOOL MBVecMagCheck(HuVecF *a, HuVecF *b, float dist);
void MBMtxLookAtCalc(Mtx dest, HuVecF *eye, HuVecF *up, HuVecF *target);
void MB3Dto2D(HuVecF *src, HuVecF *dst);
void MB2Dto3D(HuVecF *src, HuVecF *dst);
float MBBezierCalc(float a, float b, float c, float t);
void MBBezierCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *dst, float t);
void MBBezierCalcVList(HuVecF *src, HuVecF *dst, float t);
float MBBezierCalcSlope(float a, float b, float c, float t);
void MBBezierCalcSlopeV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *dst, float t);
float MBHermiteCalc(float a, float b, float c, float d, float t);
void MBHermiteCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, HuVecF *dst, float t);
float MBHermiteCalcSlope(float a, float b, float c, float d, float t);
float MBAngleLerp(float a, float b, float t);
float MBAngleLerpSin(float a, float b, float t);
float MBAngleLerpCos(float a, float b, float t);
void MBPointDepthScale(HuVecF *src, float scale, HuVecF *dst);

#endif