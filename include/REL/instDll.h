#ifndef _INSTDLL_H
#define _INSTDLL_H

#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "game/audio.h"
#include "game/window.h"
#include "game/wipe.h"
#include "game/pad.h"
#include "game/thpmain.h"
#include "game/data.h"
#include "game/charman.h"

#include "datanum/inst.h"
#include "messnum/mginstsys.h"

#define STAGE_MOT_MAX 256

enum {
    STAGE_MODEL_NONE = -1,
    STAGE_MODEL_STAGE1 = 0,
    STAGE_MODEL_STAGE2, //1
    STAGE_MODEL_MGMOVIE, //2
    STAGE_MODEL_TOUEI1, //3
    STAGE_MODEL_TOUEI2, //4
    STAGE_MODEL_TOUEI3, //5
    STAGE_MODEL_TOUEI4, //6
    STAGE_MODEL_TOUEI5, //7
    STAGE_MODEL_GUIDE, //8
    STAGE_MODEL_SORA, //9
    STAGE_MODEL_MAX = 256
};

typedef struct StageModel_s {
    unsigned int dataNum;
    s16 motMdl;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    s16 cameraBit;
} STAGE_MODEL;

#define INST_CHARMDL_NPC GW_PLAYER_MAX

#define INST_CHARMDL_MAX (INST_CHARMDL_NPC+1)

extern OMOBJMAN *objman;
extern HU3DMODELID CharMdlId[INST_CHARMDL_MAX];
extern HU3DMOTID CharMotId[INST_CHARMDL_MAX][5];
extern s16 CharNo[GW_PLAYER_MAX];
extern BOOL CameraDebugF;
extern s16 InstMode;

extern STAGE_MODEL StageMdlTbl[];
extern HU3DMODELID StageMdlId[];
extern GXColor ToueiFogColor;

void StageModelCreate(STAGE_MODEL *model);
void ToueiDispInit(char *movie);
HUSPRGRPID MgNameCreate(u32 nameMes);

#endif