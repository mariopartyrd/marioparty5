#ifndef _GAME_MGDATA_H
#define _GAME_MGDATA_H

#include "dolphin.h"
#include "game/gamework.h"

#define MG_TYPE_4P 0
#define MG_TYPE_1VS3 1
#define MG_TYPE_2VS2 2
#define MG_TYPE_BATTLE 3
#define MG_TYPE_KUPA 4
#define MG_TYPE_LAST 5
#define MG_TYPE_KETTOU 6
#define MG_TYPE_SD 7
#define MG_TYPE_DONKEY 8
#define MG_TYPE_NONE 9
#define MG_TYPE_MAX MG_TYPE_NONE

#define MG_FLAG_SPECIAL (1 << 7)
#define MG_FLAG_NOSTORY (1 << 6)
#define MG_FLAG_GRPORDER (1 << 5)
#define MG_FLAG_4P (1 << 4)
#define MG_FLAG_EASY (1 << 3)
#define MG_FLAG_GOOFY (1 << 2)
#define MG_FLAG_SKILL (1 << 1)
#define MG_FLAG_ACTION (1 << 0)

#define MG_FLAG_PACKALL (MG_FLAG_EASY|MG_FLAG_GOOFY|MG_FLAG_SKILL|MG_FLAG_ACTION)

#define MG_RECORD_NONE -1

#define MgModeWorkGet() ((void *)mgModeWork)

typedef struct MgData_s {
	u16 ovl;
	u8 type;
	u8 flag;
	s8 recordNo;
	u32 nameMes;
	unsigned int dataDir;
	unsigned int instPic[3];
	char *movie[3];
	u32 instMes[2][5];
} MGDATA;

extern float sdWorkFloat[16];
extern int sdWorkInt[16];
extern MGDATA mgDataTbl[];
extern GWCOMMON GwCommonBackup;
extern u32 mgModeWork[64];

extern s16 mgMatchGameType;
extern s16 sdHealthLoss[GW_PLAYER_MAX];
extern s16 mgMatchMaxScore;
extern s16 lbl_80288A30;
extern int lbl_80288A2C;
extern int lbl_80288A28;
extern int mgSubMode;
extern int m562CtrlNo;
extern BOOL mgExitStartF;
extern BOOL mgPauseExitF;
extern BOOL mgInstExitF;
extern BOOL mgBoard2Force;

int MgSubModeGet(void);
int MgDecaScoreCalc(int gameNo, int mgScore);

#endif