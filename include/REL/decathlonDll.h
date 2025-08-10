#ifndef _DECATHLONDLL_H
#define _DECATHLONDLL_H

#include "msm_se.h"
#include "msm_stream.h"
#include "msm.h"

#include "game/gamework.h"
#include "game/pad.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "game/hsfex.h"
#include "game/mgdata.h"
#include "game/flag.h"

#include "datanum/decathlon.h"
#include "messnum/mg_decathlon.h"
#include "messnum/syshelp.h"

#define DECAFLAG_MUS_FADEOUT (1 << 0)

#define DECAFLAG_WINBUSY (1 << 1)
#define DECAFLAG_SEE_INST (1 << 2)
#define DECAFLAG_EXIT (1 << 3)

#define DECAFLAG_INSTEXIT (1 << 4)
#define DECAFLAG_RESET (1 << 5)

#define DECAFLAG_CAMPATH_DONE (1 << 6)
#define DECAFLAG_PLAYERROT_DONE (1 << 7)
#define DECAFLAG_FADE_ENTER (1 << 8)
#define DECAFLAG_FADE_ENDING (1 << 9)
#define DECAFLAG_SCOREON (1 << 31)

#define DECA_MESSFLAG_INST (1 << 0)
#define DECA_MESSFLAG_BEGIN (1 << 1)
#define DECA_MESSFLAG_MGNO (1 << 2)
#define DECA_MESSFLAG_MGSTART (1 << 4)
#define DECA_MESSFLAG_RESULTSTART (1 << 5)
#define DECA_MESSFLAG_END (1 << 6)
#define DECA_MESSFLAG_MAXSCORE (1 << 7)
#define DECA_MESSFLAG_WINNER (1 << 8)
#define DECA_MESSFLAG_RESET (1 << 9)
#define DECA_MESSFLAG_EMOTION (1 << 10)
#define DECA_MESSFLAG_BTNWAIT (1 << 11)

enum {
    DECA_MDL_HATA,
    DECA_MDL_STAGE,
    DECA_MDL_CROWD,
    DECA_MDL_STAGE_NIGHT,
    DECA_MDL_STAGE_FB,
    DECA_MDL_KIKYUU,
    DECA_MDL_FUUSEN,
    DECA_MDL_HIKARI,
    DECA_MDL_MOON,
    DECA_MDL_HANABI,
    DECA_MDL_COVER,
    DECA_MDL_LIGHT1,
    DECA_MDL_LIGHT2,
    DECA_MDL_LIGHT3,
    DECA_MDL_LIGHT4,
    
    DECA_MDL_STAGE_NUM,
    
    DECA_MDL_PLAYER_START = DECA_MDL_STAGE_NUM,
    
    DECA_MDL_CAMERAMOT_START = DECA_MDL_PLAYER_START+GW_PLAYER_MAX,
    
    DECA_MDL_CAMERAMOT_FOCUS = DECA_MDL_CAMERAMOT_START,
    DECA_MDL_CAMERAMOT_INIT,
    DECA_MDL_CAMERAMOT_END_START,
    DECA_MDL_CAMERAMOT_END_EXIT,
    DECA_MDL_CAMERAMOT_ALT,
    DECA_MDL_CAMERAMOT_MG1,
    DECA_MDL_CAMERAMOT_MG2,
    DECA_MDL_CAMERAMOT_MG3,
    DECA_MDL_CAMERAMOT_MG4,
    DECA_MDL_CAMERAMOT_MG5,
    DECA_MDL_CAMERAMOT_MG6,
    DECA_MDL_CAMERAMOT_MG7,
    DECA_MDL_CAMERAMOT_MG8,
    DECA_MDL_CAMERAMOT_MG9,
    
    DECA_MDL_CAMERAMOT_END,
    
    DECA_MDL_LIGHT_MAIN = DECA_MDL_CAMERAMOT_END,
    
    DECA_MDL_MAX,
    DECA_MDL_CAMERAMOT_NUM = DECA_MDL_CAMERAMOT_END-DECA_MDL_CAMERAMOT_START,
    DECA_MDL_CAMERAMOT_NULL = -1,
    
};

#define DECA_SCORE_TIME 0
#define DECA_SCORE_DIST 1
#define DECA_SCORE_DIST_FRAC 2
#define DECA_SCORE_DIST_FOOT 3
#define DECA_SCORE_NUMBER 4
#define DECA_SCORE_NUMBER_CAP 5

#define DECA_SCOREMODE_INIT 0
#define DECA_SCOREMODE_WAIT 1
#define DECA_SCOREMODE_RECORD 2
#define DECA_SCOREMODE_MGSCORE 3
#define DECA_SCOREMODE_SCORE 4
#define DECA_SCOREMODE_ADD_BEGIN 5
#define DECA_SCOREMODE_ADD 6
#define DECA_SCOREMODE_NEWRANK_BEGIN 7
#define DECA_SCOREMODE_NEWRANK 8
#define DECA_SCOREMODE_STOP 9
#define DECA_SCOREMODE_STOP_IDLE 10
#define DECA_SCOREMODE_HIDE 11


typedef struct DecaWorkPlayer_s {
    HU3DMODELID mdlId;
    HU3DMOTID motId[5];
    s16 no;
    s16 charNo;
    s16 padNo;
    s8 mot;
    float rotEnd;
    float rot;
    HuVecF pos;
} DECAWORK_PLAYER;

typedef struct DecaWork_s {
    DECAWORK_PLAYER player[GW_PLAYER_MAX];
    HU3DMODELID guideMdlId;
    HuVecF guideRot;
    HuVecF guideScale;
    HuVecF guidePos;
    OMOBJMAN *objman;
    s16 cameraMotPrev;
    s16 cameraMot;
    HUWINID winId;
    int mesFlag;
    int flag;
    HU3DMODELID stageMdlId[DECA_MDL_MAX];
    s8 playerRotMode;
    float playerRotSpeed;
    int seNo;
    int streamNo;
    BOOL dirRead;
    int mgNoDir;
    HUPROCESS *enterProc;
    CAMMOTIONWORK camMot[29];
    int camMotNum;
    ANIMDATA *fbCopyAnim;
    float fbCopyX;
    float fbCopyY;
    float fbCopyW;
    float fbCopyH;
    int enterTimer;
} DECAWORK;

typedef struct DecaModeWork_s {
    u8 unkFlag;
    s8 mgNo;
    s8 gameEmotion;
    HuVecF unkVec[8];
    HuVecF startCamPos;
    HuVecF startCamUp;
    HuVecF startCamTarget;
    float startCamFov;
    HuVecF endCamPos;
    HuVecF endCamUp;
    HuVecF endCamTarget;
    float endCamFov;
} DECAMODEWORK;

typedef struct DecaMg_s {
    u16 ovl;
    s16 mgNo;
    s8 no;
    u32 mgIcon;
    int scoreType;
    s8 saveLessF;
} DECAMG;

typedef struct DecaStageMdl_s {
    int fileNum;
    int cameraBit;
    int layer;
    int attr;
    int motAttr;
} DECASTAGEMDL;

HUWINID DecaWinIdGet(void);

void DecaFlagSet(unsigned int flag);
void DecaFlagReset(unsigned int flag);
unsigned int DecaFlagGet(unsigned int flag);

void DecaUnkFlagSet(unsigned int flag);
void DecaUnkFlagReset(unsigned int flag);
unsigned int DecaUnkFlagGet(unsigned int flag);

void DecaMesFlagSet(unsigned int flag);
void DecaMesFlagReset(unsigned int flag);
unsigned int DecaMesFlagGet(unsigned int flag);

u32 DecaMgNameMesGet(void);
int DecaMgNoGet(void);
int DecaScoreMaxGet(void);
int DecaPlayerOrderGet(int *result);
int DecaPlayerRankGet(int no, int *outPlayer, int *outScore);
int DecaGameEmotionGet(void);
int DecaMgHighScoreGet(void);
int DecaMgScoreGet(int playerNo);
int DecaScoreGet(int playerNo);
void DecaMotionStartEndSet(int mdlId, float start, float end);
void DecaOvlEvtnoSet(int evtno);
void DecaCharSoundPlay(DECAWORK_PLAYER *player, int seNo);
void DecaProcessCreate(void (*func)(void));
void DecaCameraPathStart(void);
void DecaMgExit(int mgNo);
void DecaCameraMotionStart(int cameraMot);
void DecaWinMain(void);
void DecaWinMusicStart(void);
void DecaMain(void);
void DecaInit(void);
void DecaExit(void);
void DecaEvtOpening(void);
BOOL DecaEvtMgScore(void);
void DecaEvtMgNext(void);
void DecaEvtEnding(void);
void DecaEvtLast(void);
void DecaRecordSave(void);
void DecaStageReset(void);
void DecaPlayerRotateStart(int mode, float speed);
void DecaFbCopyCreate(void);
void DecaEnterEffectStart(int enterTimer);

void DecaScoreInit(void);
void DecaScoreStart(void);

void DecaScoreModeSet(int mode);
BOOL DecaScoreIdleCheck(void);

void DecaMessGameEmotion(void);
void DecaMessBtnWait(void);
void DecaMessInst(void);
void DecaMessBegin(void);
void DecaMessResultStart(void);
void DecaMessMgNo(void);
void DecaMessMgStart(void);
void DecaMessEnd(void);
void DecaMessMaxScore(void);
void DecaMessWinner(void);
void DecaMessReset(void);

extern DECAMG decaMgTbl[10];

extern DECAWORK decaWork;
extern DECAMODEWORK *decaModeWorkP;
extern DECAWORK *decaWorkP;

#endif