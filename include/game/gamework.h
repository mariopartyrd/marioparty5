#ifndef _GAMEWORK_H
#define _GAMEWORK_H

#include "dolphin.h"
#include "game/flag.h"

//HACK: to prevent prototype errors
extern void HuPadRumbleAllStop(void);

#define GW_PLAYER_MAX 4

#define GW_TYPE_MAN 0
#define GW_TYPE_COM 1

#define GW_DIF_EASY 0
#define GW_DIF_NORMAL 1
#define GW_DIF_HARD 2
#define GW_DIF_VERYHARD 3

#define GW_STAT_LEN 52
#define GW_SD_BACKUP_MAX 12

#define GW_MGNO_BASE 501
#define GW_MGNO_NONE 65535

#define GW_LANGUAGE_JPN 0
#define GW_LANGUAGE_ENG 1
#define GW_LANGUAGE_GER 2
#define GW_LANGUAGE_FRA 3
#define GW_LANGUAGE_ITA 4
#define GW_LANGUAGE_SPA 5

typedef struct GwPlayerConf_s {
    s16 charNo;
    s16 padNo;
    s16 dif;
    s16 grp;
    s16 type;
} GWPLAYERCONF;

typedef struct GwPlayer_s {
    u16 dif : 2;
    u16 comF : 1;
    u16 charNo : 4;
    u16 field3 : 2;
    u16 dead : 1;
    u16 rollType : 6;
    u16 grp : 1;
    u16 field6 : 1;
    s8 handicap;
    s8 padNo;
    s8 capsule[3];
    u16 color : 2;
    u16 moveF : 1;
    u16 jumpF : 1;
    u16 dispLightF : 1;
    u16 size : 2;
    u16 diceCnt : 2;
    u16 rank : 2;
    s8 walkNum;
    s16 masuNo;
    s16 masuNoPrev;
    s16 masuNoNext;
    s16 capsuleUse;
    s8 blueMasuNum;
    s8 redMasuNum;
    s8 donkeyMasuNum;
    s8 hatenaMasuNum;
    s8 kupaMasuNum;
    s8 m444MasuNum;
    s8 battleMasuNum;
    s8 kinokoMasuNum;
    s8 warpMasuNum;
    s8 starMasuNum;
    s8 vsMasuNum;
    s16 coin;
    s16 coinTotalMg;
    s16 coinTotal;
    s16 coinMax;
    s16 coinBattle;
    s16 mgCoin;
    s16 mgCoinBonus;
    s32 mgScore;
    s16 star;
    s16 starMax;
    s16 starStat[GW_STAT_LEN];
    s16 coinStat[GW_STAT_LEN];
} GWPLAYER;

typedef struct GwSystem_s {
    u8 partyF : 1;
    u8 teamF : 1;
    u8 storyDif;
    u16 bonusStarF : 1;
    u16 mgInstF : 1;
    u16 mgComF : 1;
    u16 mgPack : 3;
    u16 messSpeed : 2;
    u16 saveMode : 2;
    u8 turnNo;
    u8 turnMax;
    u8 starFlag;
    u8 starTotal;
    u8 starPos : 3;
    u8 boardNo : 5;
    s8 last5Effect;
    s8 turnPlayerNo;
    u8 unk[3];
    s16 saiHiddenPos;
    u32 mbSaveWork[8];
    u8 messDelay;
    u8 mgEvent : 4;
    s8 playerMode;
    u16 mgNo;
    s16 subGameNo;
    u16 bankCoin;
    s16 masuCapsule[256];
    u8 flag[8][16];
} GWSYSTEM;

typedef struct GwDecaScore_s {
    s8 charNo;
    u16 mgScore[10];
    u16 finalScore;
} GWDECASCORE;

typedef struct GwCommon_s {
    char magic[4];
    u16 unk4;
    u8 languageNo;
    u8 soundOutputMode;
    s8 rumbleF;
    u16 totalStar;
    OSTime time;
    char name[17];
    u32 mgUnlock[4];
    u32 mgRecord[12];
    u16 charPlayNum[10][10];
    u16 playNum[10];
    u16 boardMaxStar[10][10];
    u16 boardMaxCoin[10][10];
    s8 storyBoardNo[5];
    s8 storyNo;
    s8 storyRank[5];
    u16 storyPlayNum[10];
    u16 charStoryPlayNum[10][10];
    s8 charStoryRank[10][10];
    u8 storyContinue : 1;
    u8 partyContinue : 1;
    u8 w07Unlock : 1;
    u8 veryHardUnlock : 1;
    u8 m562VeryHardUnlock : 1;
    u8 unk424_2 : 1;
    u8 unk424_3 : 1;
    u8 unk424_4 : 1;
    u8 unk425_1 : 1;
    u8 unk425_2 : 1;
    u8 storyMgInstF : 1;
    u8 storyMgComF : 1;
    u8 storyMgPack : 3;
    u8 storyMessSpeed : 2;
    u8 storySaveMode : 2;
    u8 partyMgInstF : 1;
    u8 partyMgComF : 1;
    u8 partyMgPack : 3;
    u8 partyMessSpeed : 2;
    u8 partySaveMode : 2;
    s32 sdPoint[4];
    u8 unkSdFlag : 1;
    u8 unkSdFlag2 : 1;
    u16 sdVsPlayCnt;
    u16 sdVsWinCnt[3][2];
    GWDECASCORE decaScore[10];
    u16 decaHighScore[10];
    u8 unk54C[5];
} GWCOMMON;

typedef struct GwSdMachine_s {
    s8 charNo;
    char name[17];
    u8 partNo[4];
} GWSDMACHINE;

typedef struct GwSdPicture_s {
    GWSDMACHINE machine;
    u8 type;
    u8 diff;
    u16 number;
} GWSDPICTURE;

typedef struct GwSdCommon_s {
    u8 unk0;
    u8 unk1;
    u8 unk2[10];
    u16 partFlag[4];
    u16 picNo;
    GWSDMACHINE machine[2];
    GWSDPICTURE picture[10];
    u16 battleEasyWinF : 1;
    u16 battleNormalWinF : 1;
    u16 battleHardWinF : 1;
    u16 flagEasyWinF : 1;
    u16 flagNormalWinF : 1;
    u16 flagHardWinF : 1;
    u16 rabbitEasyWinF : 1;
    u16 rabbitNormalWinF : 1;
    u16 rabbitHardWinF : 1;
    u16 extCharSelF : 1;
    u16 unkFlag : 1;
    u16 newPart1F : 1;
    u16 newPart2F : 1;
    u16 newPart3F : 1;
    u16 prevPoint;
} GWSDCOMMON;

typedef struct GwSdBackup_s {
    s8 slotNo;
    s8 boxNo;
    s8 garageNo;
    GWSDMACHINE machine;
} GWSDBACKUP;

typedef struct GwSdBattle_s {
    GWSDMACHINE machine[2];
    s8 gameType;
    s8 stageNo;
    s8 stageType1;
    s8 stageType2;
    s8 winnerNo;
    s8 padNo[2];
} GWSDBATTLE;

typedef struct GwCardCommon_s {
    u8 boardSize;
    s8 pedestalF[GW_PLAYER_MAX];
    s8 starNum[GW_PLAYER_MAX];
    s8 ztarNum[GW_PLAYER_MAX];
} GWCARDCOMMON;

extern GWCARDCOMMON GwCardCommon;
extern GWSDBATTLE GwSdBattle;
extern GWSDBACKUP GwSdBackup[GW_SD_BACKUP_MAX];
extern GWDECASCORE GwMgDecaScore[GW_PLAYER_MAX];
extern GWSDCOMMON GwSdCommon;
extern GWCOMMON GwCommonOrig;
extern GWCOMMON GwCommon;
extern GWSYSTEM GwSystem;
extern GWPLAYER GwPlayer[GW_PLAYER_MAX];
extern GWPLAYERCONF GwPlayerConf[GW_PLAYER_MAX];

extern int GwSdBackupNo;

static inline BOOL GWPartyFGet(void)
{
    return GwSystem.partyF;
}

static inline void GWPartyFSet(BOOL flag)
{
    GwSystem.partyF = flag;
}

static inline BOOL GWTeamFGet(void)
{
    return GwSystem.teamF;
}

static inline void GWTeamFSet(BOOL flag)
{
    GwSystem.teamF = flag;
}

static inline void GWMgCoinBonusSet(s32 playerNo, s16 mgCoin)
{
    if(_CheckFlag(FLAG_MG_PRACTICE)) {
        return;
    }
    GwPlayer[playerNo].mgCoinBonus = mgCoin;
}

static inline void GWMgCoinSet(s32 playerNo, s16 mgCoin)
{
    GwPlayer[playerNo].mgCoin = mgCoin;
}

static inline void GWMgScoreSet(s32 playerNo, u32 mgScore)
{
    GwPlayer[playerNo].mgScore = mgScore;
}

static inline u32 GWMgScoreGet(s32 playerNo)
{
    return GwPlayer[playerNo].mgScore;
}

static inline void GWBonusStarFSet(BOOL flag)
{
    GwSystem.bonusStarF = flag;
}

static inline BOOL GWBonusStarFGet(void)
{
    return GwSystem.bonusStarF;
}

static inline void GWMgInstFSet(BOOL flag)
{
    GwSystem.mgInstF = flag;
}

static inline BOOL GWMgInstFGet(void)
{
    return GwSystem.mgInstF;
}

static inline void GWMgComFSet(BOOL flag)
{
    GwSystem.mgComF = flag;
}

static inline BOOL GWMgComFGet(void)
{
    return GwSystem.mgComF;
}

static inline s16 GWMgCoinBonusGet(s32 playerNo)
{
    return GwPlayer[playerNo].mgCoinBonus;
}

static inline s16 GWMgCoinGet(s32 playerNo)
{
    return GwPlayer[playerNo].mgCoin;
}

static inline s32 GWMessSpeedGet(void)
{
    if (GwSystem.messSpeed == 3) {
        GwSystem.messSpeed = 1;
    }
    return GwSystem.messSpeed;
}

static inline void GWMessSpeedSet(s32 value)
{
	GwSystem.messSpeed = value;
	switch(value) {
		case 0:
			GwSystem.messDelay = 16;
			break;
			
		case 2:
			GwSystem.messDelay = 48;
			break;
			
		default:
			GwSystem.messDelay = 32;
			break;
	}
}

static inline s32 GWMgPackGet(void)
{
    if (GwSystem.mgPack == 5) {
        GwSystem.mgPack = 0;
    }
    return GwSystem.mgPack;
}

static inline void GWMgPackSet(s32 value)
{
	GwSystem.mgPack = value;
}


static inline void GWSaveModeSet(s32 value)
{
    GwSystem.saveMode = value;
}

static inline s32 GWSaveModeGet(void)
{
    if (GwSystem.saveMode == 3) {
		GWSaveModeSet(1);
    }
    return GwSystem.saveMode;
}

static inline void GWLanguageSet(s32 no)
{
    GwCommon.languageNo = no;
}

static inline s32 GWLanguageGet(void)
{
    return GwCommon.languageNo;
}

static inline s32 GWRumbleFGet(void)
{
    return GwCommon.rumbleF;
}

static inline void GWRumbleFSet(s32 value)
{
    GwCommon.rumbleF = value;
	if(value == 0) {
		HuPadRumbleAllStop();
	}
}

#define GWMgCoinBonusAdd(player, value) GWMgCoinBonusSet((player), GWMgCoinBonusGet((player))+(value))
#define GWMgCoinAdd(player, value) GWMgCoinSet((player), GWMgCoinGet((player))+(value))

#endif
