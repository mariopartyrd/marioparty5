#include "game/gamework.h"
#include "game/flag.h"
#include "game/charman.h"
#include "game/saveload.h"

GWCARDCOMMON GwCardCommon;
GWSDBATTLE GwSdBattle;
GWSDBACKUP GwSdBackup[GW_SD_BACKUP_MAX];
GWDECASCORE GwMgDecaScore[GW_PLAYER_MAX];
GWSDCOMMON GwSdCommon;
GWCOMMON GwCommonOrig;
GWCOMMON GwCommon;
GWSYSTEM GwSystem;
GWPLAYER GwPlayer[GW_PLAYER_MAX];
GWPLAYERCONF GwPlayerConf[GW_PLAYER_MAX];

int GwSdBackupNo;

void GWCommonInit(void);
void GWSdCommonInit(void);

void GWInit(void)
{
    s32 i, j;
    GWCommonInit();
    _InitFlag();
    memset(&GwPlayerConf[0], 0, sizeof(GwPlayerConf));
    memset(&GwPlayer[0], 0, sizeof(GwPlayer));
    memset(&GwSystem, 0, sizeof(GwSystem));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        GWPLAYERCONF *conf = &GwPlayerConf[i];
        conf->charNo = i;
        conf->padNo = i;
        conf->dif = 0;
        conf->grp = i;
        if(HuPadStatGet(i) == 0) {
            conf->type = GW_TYPE_MAN;
        } else {
            conf->type = GW_TYPE_COM;
        }
    }
    GWLanguageSet(1);
    GWRumbleFSet(TRUE);
    GWMgInstFSet(TRUE);
    GWMgComFSet(TRUE);
    GWMessSpeedSet(1);
    GWSaveModeSet(0);
    GWPartyFSet(TRUE);
    for(i=0; i<10; i++) {
        GwCommon.storyPlayNum[i] = 0;
    }
    for(i=0; i<10; i++) {
        for(j=0; j<10; j++) {
            GwCommon.charStoryPlayNum[i][j] = 0;
            GwCommon.charStoryRank[i][j] = -1;
        }
    }
    for(i=0; i<5; i++) {
        GwCommon.storyBoardNo[i] = -1;
    }
    GwCommon.storyNo = -1;
    GWSdCommonInit();
}

void GWCommonInit(void)
{
    GWCOMMON *commonP = &GwCommonOrig;
    s32 boardNo, charNo, i, j;
    memset(commonP, 0, sizeof(GWCOMMON));
    commonP->magic[0] = 0;
    commonP->unk4 = 0;
    commonP->languageNo = 1;
    commonP->soundOutputMode = 1;
    commonP->rumbleF = TRUE;
    commonP->totalStar = 0;
    commonP->time = 0;
    commonP->mgUnlock[0] = 0;
    commonP->mgUnlock[1] = 0;
    commonP->mgRecord[0] = 90;
    commonP->mgRecord[1] = 15;
    commonP->mgRecord[2] = 3600;
    commonP->mgRecord[3] = 2700;
    commonP->mgRecord[4] = 900;
    commonP->mgRecord[5] = 9000;
    commonP->mgRecord[6] = 30;
    commonP->mgRecord[7] = 200;
    commonP->mgRecord[8] = 5000;
    commonP->mgRecord[9] = 5000;
    commonP->mgRecord[10] = 5000;
    commonP->mgRecord[11] = 5000;
    for(boardNo=0; boardNo<10; boardNo++) {
        for(charNo=0; charNo<13; charNo++) {
            commonP->charPlayNum[boardNo][charNo] = 0;
        }
        commonP->playNum[boardNo] = 0;
        for(i=0; i<13; i++) {
            commonP->boardMaxStar[boardNo][i] = 0;
            commonP->boardMaxCoin[boardNo][i] = 0;
        }
    }
    commonP->storyContinue = 0;
    commonP->partyContinue = 0;
    commonP->w07Unlock = 0;
    commonP->veryHardUnlock = 0;
    commonP->m562VeryHardUnlock = 0;
    commonP->unk424_3 = 0;
    commonP->storyMgInstF = commonP->partyMgInstF = TRUE;
    commonP->storyMgComF = commonP->partyMgComF = TRUE;
    commonP->storyMgPack = commonP->partyMgPack = 0;
    commonP->storyMessSpeed = commonP->partyMessSpeed = 1;
    commonP->storySaveMode = commonP->partySaveMode = 0;
    for(i=0; i<4; i++) {
        commonP->sdPoint[i] = 0;
    }
    for(i=0; i<10; i++) {
        commonP->decaScore[i].charNo = CHARNO_NONE;
    }
    commonP->decaHighScore[0] = 2404;
    commonP->decaHighScore[1] = 1876;
    commonP->decaHighScore[2] = 31;
    commonP->decaHighScore[3] = 120;
    commonP->decaHighScore[4] = 4100;
    commonP->decaHighScore[5] = 17;
    commonP->decaHighScore[6] = 3157;
    commonP->decaHighScore[7] = 837;
    commonP->decaHighScore[8] = 1948;
    commonP->decaHighScore[9] = 8523;
    memcpy(&GwCommon, &GwCommonOrig, sizeof(GWCOMMON));
    GWRumbleFSet(TRUE);
    GWMgInstFSet(TRUE);
    GWMgComFSet(TRUE);
    GWMessSpeedSet(1);
    GWSaveModeSet(0);
    for(i=0; i<10; i++) {
        commonP->storyPlayNum[i] = 0;
    }
    for(i=0; i<10; i++) {
        for(j=0; j<10; j++) {
            commonP->charStoryPlayNum[i][j] = 0;
            commonP->charStoryRank[i][j] = -1;
        }
    }
    for(i=0; i<5; i++) {
        commonP->storyBoardNo[i] = -1;
    }
    commonP->storyNo = -1;
}

s32 GWComKeyDelayGet(void)
{
    if(GwSystem.messDelay > 48) {
        GWMessSpeedSet(1);
    }
    return GwSystem.messDelay;
}

void GWMgRecordSet(s32 id, u32 value)
{
    if(_CheckFlag(FLAG_MG_PRACTICE)) {
        return;
    }
    GwCommon.mgRecord[id] = value;
}

u32 GWMgRecordGet(s32 id)
{
    return GwCommon.mgRecord[id];
}

void GWCharColorGet(s32 charNo, GXColor *colorP)
{
    GXColor color[13] = {
        { 227, 67, 67, 255 },
        { 68, 67, 227, 255 },
        { 241, 158, 220, 255 },
        { 67, 228, 68, 255 },
        { 138, 60, 180, 255 },
        { 146, 85, 55, 255 },
        { 227, 228, 68, 255 },
        { 40, 40, 40, 255 }
    };
    *colorP = color[charNo];
}

void GWPlayNumSet(s32 boardNo, u8 num)
{
    if(num > 99) {
        num = 99;
    }
    GwCommon.playNum[boardNo] = num;
}

void GWPlayNumAdd(s32 boardNo, u8 num)
{
    num += GwCommon.playNum[boardNo];
    if(num > 99) {
        num = 99;
    }
    GwCommon.playNum[boardNo] = num;
}

u16 GWPlayNumGet(s32 boardNo)
{
    return GwCommon.playNum[boardNo];
}

void GWBoardMaxStarSet(s32 boardNo, s32 value, u8 charNo)
{
    GwCommon.boardMaxStar[boardNo][charNo] = value;
}

u16 GWBoardMaxStarGet(s32 boardNo, u8 charNo)
{
    return GwCommon.boardMaxStar[boardNo][charNo];
}

void GWBoardMaxCoinSet(s32 boardNo, s32 value, u8 charNo)
{
    GwCommon.boardMaxCoin[boardNo][charNo] = value;
}

u16 GWBoardMaxCoinGet(s32 boardNo, u8 charNo)
{
    return GwCommon.boardMaxCoin[boardNo][charNo];
}

s32 GWCharPlayNumInc(s32 charNo, s32 boardNo)
{
    s32 playNum = GwCommon.charPlayNum[boardNo][charNo]+1;
    if(playNum > 99) {
        playNum = 99;
    }
    GwCommon.charPlayNum[boardNo][charNo] = playNum;
    return playNum;
}

s32 GWCharPlayNumGet(s32 charNo, s32 boardNo)
{
    return GwCommon.charPlayNum[boardNo][charNo];
}

void GWCharPlayNumSet(s32 charNo, s32 boardNo, s32 value)
{
    GwCommon.charPlayNum[boardNo][charNo] = value;
}

BOOL GWMgUnlockGet(s32 mgNo)
{
    s32 word;
    s32 bit;
    mgNo -= GW_MGNO_BASE;
    word = mgNo >> 5;
    bit = mgNo % 32;
    if(GwCommon.mgUnlock[word] & (1 << bit)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void GWMgUnlockSet(s32 mgNo)
{
    s32 word;
    s32 bit;
    mgNo -= GW_MGNO_BASE;
    word = mgNo >> 5;
    bit = mgNo % 32;
    GwCommon.mgUnlock[word] |= (1 << bit);
}

BOOL GWMgCustomGet(s32 mgNo)
{
    
}

void GWMgCustomSet(s32 mgNo)
{
    
}

void GWMgCustomReset(s32 mgNo)
{
    
}

s16 GWPlayerCoinGet(int playerNo)
{
    return GwPlayer[playerNo].coin;
}

void GWPlayerCoinSet(int playerNo, s16 coin)
{
    if(_CheckFlag(FLAG_MG_PRACTICE)) {
        return;
    }
    if(coin < 0) {
        coin = 0;
    }
    if(coin > 999) {
        coin = 999;
    }
    if(coin > GwPlayer[playerNo].coinMax) {
        GwPlayer[playerNo].coinMax = coin;
    }
    GwPlayer[playerNo].coin = coin;
}

void GWPlayerCoinAdd(int playerNo, s16 coin)
{
    GWPlayerCoinSet(playerNo, coin+GwPlayer[playerNo].coin);
}

void GWPlayerStarSet(int playerNo, s16 star)
{
    if(star < 0) {
        star = 0;
    }
    if(star > 999) {
        star = 999;
    }
    if(star > GwPlayer[playerNo].starMax) {
        GwPlayer[playerNo].starMax = star;
    }
    GwPlayer[playerNo].star = star;
}

void GWPlayerStarAdd(int playerNo, s16 star)
{
    GWPlayerStarSet(playerNo, star+GwPlayer[playerNo].star);
}

s16 GWPlayerStarGet(int playerNo)
{
    return GwPlayer[playerNo].star;
}

void GWTotalStarSet(s16 totalStar)
{
    if(totalStar < 0) {
        totalStar = 0;
    }
    if(totalStar > 10000) {
        totalStar = 10000;
    }
    GwCommon.totalStar = totalStar;
}

void GWTotalStarAdd(s16 totalStar)
{
    GWTotalStarSet(GwCommon.totalStar+totalStar);
}

u16 GWTotalStarGet(void)
{
    return GwCommon.totalStar;
}

void GWSdCommonInit(void)
{
    s16 i;
    GwSdCommon.unk0 = 255;
    GwSdCommon.unk1 = 0;
    GwSdCommon.partFlag[0] = 0;
    GwSdCommon.partFlag[1] = 0;
    GwSdCommon.partFlag[2] = 0;
    GwSdCommon.partFlag[3] = 0;
    GwSdCommon.picNo = 0;
    GwSdCommon.machine[0].charNo = CHARNO_NONE;
    GwSdCommon.machine[1].charNo = CHARNO_NONE;
    for(i=0; i<10; i++) {
        GwSdCommon.picture[i].number = -1;
    }
    GwSdCommon.prevPoint = 0;
    //This is all 1 assignment
    GwSdCommon.battleEasyWinF = GwSdCommon.battleNormalWinF = GwSdCommon.battleHardWinF =
    GwSdCommon.flagEasyWinF = GwSdCommon.flagNormalWinF = GwSdCommon.flagHardWinF =
    GwSdCommon.rabbitEasyWinF = GwSdCommon.rabbitNormalWinF = GwSdCommon.rabbitHardWinF = 
    GwSdCommon.extCharSelF = GwSdCommon.unkFlag = GwSdCommon.newPart1F = GwSdCommon.newPart2F = GwSdCommon.newPart3F = 0;
}


void GWSaveCompleteSet(void)
{
    s16 i;
    for(i=0; i<4; i++) {
        GwCommon.mgUnlock[i] = -1;
    }
    for(i=0; i<10; i++) {
        GwCommon.playNum[i] = 1;
    }
    for(i=0; i<10; i++) {
        GwCommon.storyPlayNum[i] = 1;
    }
    GwCommon.w07Unlock = 1;
    GwCommon.veryHardUnlock = 1;
    GwCommon.m562VeryHardUnlock = 1;
    for(i=0; i<4; i++) {
        GwCommon.sdPoint[i] = 100;
    }
    for(i=0; i<4; i++) {
        GwSdCommon.partFlag[i] = -1;
    }
    //This is all 1 assignment
    GwSdCommon.battleEasyWinF = GwSdCommon.battleNormalWinF = GwSdCommon.battleHardWinF =
    GwSdCommon.flagEasyWinF = GwSdCommon.flagNormalWinF = GwSdCommon.flagHardWinF =
    GwSdCommon.rabbitEasyWinF = GwSdCommon.rabbitNormalWinF = GwSdCommon.rabbitHardWinF = 
    GwSdCommon.extCharSelF = 1;
    SLSdSave(curSlotNo, curBoxNo, &GwSdCommon);
}