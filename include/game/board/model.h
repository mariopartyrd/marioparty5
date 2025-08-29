#ifndef _BOARD_MODEL_H
#define _BOARD_MODEL_H

#include "game/charman.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "game/audio.h"

#define MB_MOT_MAX CHAR_MOT_MAX
#define MB_MODEL_MAX 512
#define MB_MODEL_NONE -1
#define MB_MOT_NONE -1
#define MB_MOT_DEFAULT 0

typedef s16 MBMODELID;

typedef struct MbModel_s {
	u8 disp : 1;
	u8 dispOld : 1;
	u8 unkFlag : 1;
	u8 unkFlag2 : 1;
	u8 motionOff : 1;
	s8 motNum;
	s8 charNo;
	u8 alpha;
	u8 layer;
	u16 cameraBit;
	MBMODELID mbId;
	HU3DMODELID modelId;
	HU3DMOTID motId[MB_MOT_MAX];
	s16 motNo;
	void *data;
	void *motData[MB_MOT_MAX];
	float unkSpeed;
	int dataNum;
	float motStart;
	float motEnd;
	HuVecF pos;
	HuVecF rot;
	HuVecF scale;
	HuVecF offset;
} MBMODEL;

typedef struct MbModelParam_s {
	int dataNum;
	u8 unkVal : 2;
	u8 dispF : 1;
	u8 linkF : 1;
	u8 motStartF : 1;
	u8 loopF : 1;
	s16 unkSpeed;
} MBMODELPARAM;

void MBModelInit(void);
void MBModelClose(void);
void MBModelManDummy(void);
void MBModelDispOffAll(BOOL dispOff);
void MBModelDispOffCamera(u16 cameraBit, BOOL dispOff);
void MBModelAmbSetAll(float r, float g, float b);
void MBModelCameraSetAll(u16 cameraBit);

MBMODELID MBModelCreate(int dataNum, int *motDataNum, BOOL link);
MBMODELID MBModelCreateChar(int charNo, int dataNum, int *motDataNum, BOOL link);
MBMODELID MBModelCreateParam(MBMODELPARAM *param, HuVecF *pos, HuVecF *rot);

void MBModelKill(MBMODELID modelId);

HU3DMODELID MBModelIdGet(MBMODELID modelId);
BOOL MBModelUnkFlagSet(MBMODELID modelId, BOOL flag);
BOOL MBModelZWriteSet(MBMODELID modelId, BOOL zWrite);
BOOL MBModelAmbSet(MBMODELID modelId, float r, float g, float b);

BOOL MBModelStubValSet(MBMODELID modelId, int value);
BOOL MBModelStubVal2Set(MBMODELID modelId, int value);
BOOL MBModelUnkSpeedSet(MBMODELID modelId, float speed);

int MBMotionCreate(MBMODELID modelId, int dataNum);
BOOL MBMotionKill(MBMODELID modelId, int motNo);
BOOL MBMotionStartEndSet(MBMODELID modelId, s16 start, s16 end);
BOOL MBMotionLoopSet(MBMODELID modelId, BOOL loopF);
BOOL MBMotionVoiceOnSet(MBMODELID modelId, int motNo, BOOL voiceOn);

BOOL MBMotionNoSet(MBMODELID modelId, int motNo, u32 attr);
int MBMotionShiftIDGet(MBMODELID modelId);
BOOL MBMotionShiftSet(MBMODELID modelId, int motNo, float start, float end, u32 attr);

#ifndef MBPAUSE_ATTRSET_HACK
BOOL MBModelAttrSet(MBMODELID modelId, u32 attr);
BOOL MBModelAttrReset(MBMODELID modelId, u32 attr);
#endif

BOOL MBMotionTimeSet(MBMODELID modelId, float time);
float MBMotionTimeGet(MBMODELID modelId);
float MBMotionMaxTimeGet(MBMODELID modelId);
float MBMotionSpeedGet(MBMODELID modelId);
BOOL MBMotionSpeedSet(MBMODELID modelId, float speed);
BOOL MBMotionEndCheck(MBMODELID modelId);
BOOL MBMotionShapeSet(MBMODELID modelId, int motNo, u32 attr);
//BOOL MBMotionShapeTimeSet(MBMODELID modelId, float time);
float MBMotionShapeTimeGet(MBMODELID modelId);
float MBMotionShapeMaxTimeGet(MBMODELID modelId);
BOOL MBMotionShapeSpeedSet(MBMODELID modelId, float speed);
float MBMotionShapeSpeedGet(MBMODELID modelId);
BOOL MBMotionShapeEndCheck(MBMODELID modelId);
BOOL MBMotionShapeStartEndSet(MBMODELID modelId, s16 start, s16 end);

BOOL MBModelAlphaSet(MBMODELID modelId, u8 alpha);
BOOL MBModelMtxSet(MBMODELID modelId, Mtx *mtx);
BOOL MBModelMtxGet(MBMODELID modelId, Mtx *mtx);

BOOL MBModelRotYSet(MBMODELID modelId, float rotY);
float MBModelRotYGet(MBMODELID modelId);

BOOL MBModelPosSetV(MBMODELID modelId, HuVecF *pos);
BOOL MBModelPosSet(MBMODELID modelId, float x, float y, float z);
BOOL MBModelRotSetV(MBMODELID modelId, HuVecF *rot);
BOOL MBModelRotSet(MBMODELID modelId, float x, float y, float z);
BOOL MBModelScaleSetV(MBMODELID modelId, HuVecF *scale);
BOOL MBModelScaleSet(MBMODELID modelId, float x, float y, float z);
void MBModelOffsetSet(MBMODELID modelId, float x, float y, float z);
void MBModelOffsetSetV(MBMODELID modelId, HuVecF *offset);

BOOL MBModelPosGet(MBMODELID modelId, HuVecF *pos);
BOOL MBModelRotGet(MBMODELID modelId, HuVecF *rot);
BOOL MBModelScaleGet(MBMODELID modelId, HuVecF *scale);
void MBModelOffsetGet(MBMODELID modelId, HuVecF *offset);

BOOL MBModelDispSet(MBMODELID modelId, BOOL dispF);
BOOL MBModelDispGet(MBMODELID modelId);
BOOL MBModelCameraSet(MBMODELID modelId, u16 cameraBit);
BOOL MBModelLayerSet(MBMODELID modelId, u8 layer);
int MBModelLayerGet(MBMODELID modelId);
void MBModelLayerSetAll(int layer);

BOOL MBModelHookSet(MBMODELID modelId, char *objName, MBMODELID hookMdlId);
BOOL MBModelHookReset(MBMODELID modelId);
BOOL MBModelHookObjReset(MBMODELID modelId, char *objName);

MBMODEL *MBModelGet(MBMODELID modelId);

void MBModelStub(void);
int MBModelSePlay(MBMODELID modelId, s16 seId);

#endif