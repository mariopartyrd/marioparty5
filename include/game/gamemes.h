#ifndef _GAME_GAMEMES_H
#define _GAME_GAMEMES_H

#include "humath.h"
#include "game/sprite.h"
#include "game/object.h"
#include "game/data.h"
#include "datanum/gamemes.h"
#include "messnum/mginstsys.h"

#include "stdarg.h"

#define GMES_ID_NONE -1

#define GMES_MAX 8

#define GMES_STRMAX 16

#define GMES_MES_NULL 0
#define GMES_MES_TIMER 1
#define GMES_MES_MG 2
#define GMES_MES_MG_2VS2 3
#define GMES_MES_MG_WINNER 4
#define GMES_MES_MG_1VS3 5
#define GMES_MES_MG_BATTLE 6
#define GMES_MES_MG_4P 7
#define GMES_MES_MG_KUPA 8
#define GMES_MES_MG_LAST 9
#define GMES_MES_MG_KETTOU 10
#define GMES_MES_MG_SD 11
#define GMES_MES_MG_DONKEY 12
#define GMES_MES_MG_DRAW 13
#define GMES_MES_MG_RECORD 14

#define GMES_STAT_NONE 0
#define GMES_STAT_EXIST (1 << 0)
#define GMES_STAT_TIMEEND (1 << 2)
#define GMES_STAT_KILL (1 << 3)
#define GMES_STAT_FXPLAY (1 << 4)

#define GMES_DISP_NONE 0
#define GMES_DISP_UPDATE 1
#define GMES_DISP_SET 2
#define GMES_DISP_HIDE 3

#define GMES_SUBMODE_NULL -1
#define GMES_SUBMODE_NONE 0

#define GMES_STR_NONE -1

#define GMES_MG_TYPE_START 0
#define GMES_MG_TYPE_FINISH 1
#define GMES_MG_TYPE_DRAW 2

#define GMES_MG_WINNER_NONE -1

#define GMesTimerCreate(time) GMesCreate(GMES_MES_TIMER, time, -1, -1)
#define GMesTimerPosCreate(time, x, y) GMesCreate(GMES_MES_TIMER, time, x, y)
#define GMesTimerValueSet(gMesId, value) GMesDispSet(gMesId, GMES_DISP_UPDATE, value)
#define GMesTimerEnd(gMesId) GMesDispSet(gMesId, GMES_DISP_SET, -1)

#define GMesMgStartCreate() GMesCreate(GMES_MES_MG, GMES_MG_TYPE_START)
#define GMesMgFinishCreate() GMesCreate(GMES_MES_MG, GMES_MG_TYPE_FINISH)
#define GMesMgDrawCreate() GMesCreate(GMES_MES_MG, GMES_MG_TYPE_DRAW)
#define GMesFXPlayCheck(gMesId) (GMesStatGet(gMesId) & GMES_STAT_FXPLAY)
#define GMesMgStartTypeCreate(type) GMesCreate(type, GMES_MG_TYPE_START)
#define GMesMgFinishTypeCreate(type) GMesCreate(type, GMES_MG_TYPE_FINISH)
#define GMesMgDrawTypeCreate(type) GMesCreate(type, GMES_MG_TYPE_DRAW)

#define GMesMgWinnerCreate(charNo1, charNo2, charNo3, charNo4) GMesCreate(GMES_MES_MG_WINNER, 3, charNo1, charNo2, charNo3, charNo4)


#define GMesRecordCreate(val) GMesCreate(GMES_MES_MG_RECORD, val)


typedef s16 GMESID;

typedef struct GameMes_s GAMEMES;

typedef BOOL (*GAMEMESEXEC)(GAMEMES *gMes);
typedef BOOL (*GAMEMESCREATE)(GAMEMES *gMes, va_list args);

typedef struct GameMes_s {
	GAMEMESEXEC exec;
	void *buf;
	HuVec2F pos;
	HuVec2F scale;
	float unk18;
	float angle;
	float winScale;
	float unk24;
	s16 time;
	s16 timeMax;
	s16 timerVal;
	s16 subMode;
	s16 work;
	s16 charNum;
	s16 dispMode;
	s16 dispValue;
	s16 mesMode;
	HUSPRGRPID strGId[GMES_STRMAX];
	HUSPRID strSprId[GMES_STRMAX];
	u8 mesNo;
	u8 stat;
	GXColor color;
} GAMEMES;

extern GAMEMES gMesData[GMES_MAX];

void GMesInit(void);
void *GMesDataRead(unsigned int dataNum);
void GMesExec(void);

GMESID GMesCreate(s16 mesNo, ...);
u8 GMesStatGet(GMESID id);
void GMesPosSet(GMESID id, float posX, float posY);
void GMesDispSet(GMESID id, s16 mode, s16 value);
void GMesKill(GMESID id);
void GMesClose(void);
BOOL GMesKillCheck(void);
void GMesStub(void);
void GMesSprKill(GAMEMES *gMes);
int GMesStrCreate(GAMEMES *gMes, char *str, s16 flag);
int GMesStrCopy(GAMEMES *gMes, s16 id);
void GMesPauseCreate(void);
void GMesPauseCancel(void);
void GMesPauseEnable(BOOL enable);
void GMesPracticeCreate(void);
void GMesStartSePlay(void);
void GMesExitCheck(OMOBJ *obj);
GMESID GMesStrWinCreate(GAMEMES *gMes, u32 winMessId);

void MgScoreWinInit(void);
HUSPRGRPID MgScoreWinCreate(s16 sizeX, s16 sizeY);
void MgScoreWinColorSet(HUSPRGRPID gid, u8 r, u8 g, u8 b);
void MgScoreWinTPLvlSet(HUSPRGRPID gid, float tpLvl);
void MgScoreWinPosSet(HUSPRGRPID gid, float posX, float posY);
void MgScoreWinDispSet(HUSPRGRPID gid, BOOL disp);

BOOL GMesTimerInit(GAMEMES *gMes, va_list args);
BOOL GMesTimerExec(GAMEMES *gMes);
BOOL GMesMgInit(GAMEMES *gMes, va_list args);
BOOL GMesMg4PExec(GAMEMES *gMes);
BOOL GMesMg2Vs2Exec(GAMEMES *gMes);
BOOL GMesMgWinnerInit(GAMEMES *gMes, va_list args);
BOOL GMesMgWinnerExec(GAMEMES *gMes);
BOOL GMesMg1Vs3Init(GAMEMES *gMes, va_list args);
BOOL GMesMg1Vs3Exec(GAMEMES *gMes);
BOOL GMesMgBattleInit(GAMEMES *gMes, va_list args);
BOOL GMesMgBattleExec(GAMEMES *gMes);
BOOL GMesMgKupaInit(GAMEMES *gMes, va_list args);
BOOL GMesMgKupaExec(GAMEMES *gMes);
BOOL GMesMgLastExec(GAMEMES *gMes);
BOOL GMesMgKettouExec(GAMEMES *gMes);
BOOL GMesMgSdInit(GAMEMES *gMes, va_list args);
BOOL GMesMgSdExec(GAMEMES *gMes);
BOOL GMesMgDonkeyExec(GAMEMES *gMes);
BOOL GMesMgDrawInit(GAMEMES *gMes, va_list args);
BOOL GMesMgDrawExec(GAMEMES *gMes);
BOOL GMesMgRecordInit(GAMEMES *gMes, va_list args);
BOOL GMesMgRecordExec(GAMEMES *gMes);

extern GAMEMES gMesData[GMES_MAX];
extern s16 gMesTime;
extern u8 gMesCloseF;
extern unsigned int gMesVWait;
extern int gMesLanguageNo;
extern OMOVL gMesOvlPrev;

#endif
