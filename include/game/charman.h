#ifndef _GAME_CHARMAN_H
#define _GAME_CHARMAN_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/armem.h"

#include "msm_se.h"
#include "datanum/char.h"
#include "datanum/charmot.h"


#define CHARVOICEID(no) (MSM_SE_VOICE_MARIO+(no))
#define CHARSEID(no) (MSM_SE_MARIO+(no))

#define CHARNO_NONE -1
#define CHARNO_MARIO 0
#define CHARNO_LUIGI 1
#define CHARNO_PEACH 2
#define CHARNO_YOSHI 3
#define CHARNO_WARIO 4
#define CHARNO_DAISY 5
#define CHARNO_WALUIGI 6
#define CHARNO_KINOPIO 7
#define CHARNO_TERESA 8
#define CHARNO_MINIKOOPA 9
#define CHARNO_MINIKOOPAR 10
#define CHARNO_MINIKOOPAG 11
#define CHARNO_MINIKOOPAB 12
#define CHARNO_MAX 13

#define CHAR_MODEL0 (1 << 0)
#define CHAR_MODEL1 (1 << 1)
#define CHAR_MODEL2 (1 << 2)
#define CHAR_MODEL3 (1 << 3)
#define CHAR_MODEL_MAX 4

#define CHAR_MOT_MAX 32

#define CHAR_NPC_MAX 14
#define CHAR_NPC_NO_BASE 14
#define CHAR_NPC_NONE -1

#define CHAR_MOTNO(dataNum) FILENUM(dataNum)

#define CHARMOT_DATANUM(dataNum, charName) (DATANUM(DATA_##charName##mot, FILENUM(dataNum)))
#define CHAR_DATANUM(dataNum, charName) (DATANUM(DATA_##charName, FILENUM(dataNum)))

void CharInit(void);
AMEM_PTR CharMotionAMemPGet(s16 charNo);
void CharMotionInit(s16 charNo);
void CharMotionInitAsync(s16 charNo);
void CharMotionClose(s16 charNo);
void CharDataClose(s16 charNo);
void CharClose(s16 charNo);
HU3DMODELID CharModelCreate(s16 charNo, s16 model);

HU3DMOTID CharMotionCreate(s16 charNo, unsigned int dataNum);
void CharMotionNoSet(s16 charNo, HU3DMOTID motId, unsigned int motNo);
void CharMotionKill(s16 charNo, unsigned int motId);
void CharMotionDataClose(s16 charNo);
void CharModelDataClose(s16 charNo);
void CharModelKill(s16 charNo);
void CharMotionSet(s16 charNo, HU3DMOTID motId);

char **CharModelEyeBmpGet(s16 charNo, s16 model);
char *CharModelItemHookGet(s16 charNo, s16 model, s16 hookNo);

void CharMotionTimeSet(s16 charNo, float time);
float CharMotionTimeGet(s16 charNo);
float CharMotionMaxTimeGet(s16 charNo);
BOOL CharMotionEndCheck(s16 charNo);
HU3DMOTID CharMotionShiftIDGet(s16 charNo);
void CharMotionShiftSet(s16 charNo, HU3DMOTID motId, float start, float end, u32 attr);
float CharMotionShiftTimeGet(s16 charNo);
void CharMotionSpeedSet(s16 charNo, float speed);
void CharMotionStartEndSet(s16 charNo, float start, float end);
void CharModelAttrSet(s16 charNo, u32 attr);
void CharModelAttrReset(s16 charNo, u32 attr);
float CharModelHeightGet(s16 charNo);
HU3DMODELID CharModelMotListCreate(s16 charNo, s16 model, unsigned int *motDataNum, HU3DMOTID *motId);
s32 CharModelTimingHookNoGet(s16 charNo);
void CharModelHookDustCreate(s16 charNo, char *objName);
void CharEffectSmokeCreate(s16 cameraBit, HuVecF *pos);
void CharEffectCoinGlowCreate(s16 cameraBit, HuVecF *pos);
void CharModelHitCreate(s16 charNo);
void CharEffectHitCreate(s16 cameraBit, HuVecF *pos, HuVecF *rot);
void CharModelShoeHitCreate(s16 charNo);
void CharEffectShoeHitCreate(s16 cameraBit, HuVecF *pos, HuVecF *rot);
void CharEffectLayerSet(s16 layerNo);
void CharMotionVoiceOnSet(s16 charNo, s16 motNo, BOOL voiceOn);
void CharModelVoicePanAutoSet(s16 charNo, BOOL voicePanAuto);
void CharModelFxFlagSet(s16 charNo, BOOL fxFlag);
void CharMotionUpdateSet(s16 charNo, unsigned int dataNum, BOOL updateF);
s32 CharNpcDustSet(HU3DMODELID modelId, HU3DMOTID motId, s16 type, s16 npcNo);
s32 CharNpcDustVoiceOffSet(HU3DMODELID modelId, HU3DMOTID motId, s16 type);
void CharModelStepFxSet(s16 charNo, s16 stepFx);

void CharModelLandDustCreate(s16 charNo, HuVecF *pos);
void CharEffectDustCreate(s16 cameraBit, float scale, HuVecF *pos);
void CharEffectCryCreate(s16 cameraBit, HuVecF *pos, HuVecF *offset, float scale);
void CharModelCryCreate(s16 charNo, float yOfs, float ofsY);

unsigned int CharModelFileNumGet(s16 charNo, s16 model);
unsigned int CharMotionFileNumGet(s16 charNo, u16 motId);
void CharModelVoicePanSet(s16 charNo, s16 pan);
void CharModelVoiceVolSet(s16 charNo, s16 vol);

void CharWinLoseVoicePlay(s16 charNo, unsigned int motId, s16 seId);
void CharLoseVoicePlay(s16 charNo1, s16 charNo2, s16 charNo3, s16 charNo4);

s16 CharMotionTotalTimeGet(s16 charNo, int motNo);
s16 CharMotionExtraTimeGet(s16 charNo, int motNo);

extern unsigned int charDataDirTbl[CHARNO_MAX][6];

#endif
