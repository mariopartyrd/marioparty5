#include "messnum/mpsystem.h"
#include "messnum/sdg.h"
#include "datanum/win.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/hu3d.h"
#include "game/wipe.h"

#include "game/window.h"
#include "game/saveload.h"
#include "game/card.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/sprite.h"

#define SAVE_BOX_COMMON_OFS 0
#define SAVE_BOX_BOARD_SYSTEM_OFS (sizeof(GWCOMMON))
#define SAVE_BOX_BOARD_PLAYER_OFS (SAVE_BOX_BOARD_SYSTEM_OFS+sizeof(GWSYSTEM))
#define SAVE_BOX_STORY_SYSTEM_OFS (SAVE_BOX_BOARD_PLAYER_OFS+(GW_PLAYER_MAX*sizeof(GWPLAYER)))
#define SAVE_BOX_STORY_PLAYER_OFS (SAVE_BOX_STORY_SYSTEM_OFS+sizeof(GWSYSTEM))
#define SAVE_BOX_SDCOMMON_OFS (SAVE_BOX_STORY_PLAYER_OFS+(GW_PLAYER_MAX*sizeof(GWPLAYER)))

#define SL_CUR_SLOT_MESS (MESS_MPSYSTEM_CARD_SLOTA+curSlotNo)

#define SL_MESSID_NONE -1

#define SL_MESS_NOCARD 0
#define SL_MESS_FATAL_ERROR 1
#define SL_MESS_CARD_NOENT 2
#define SL_MESS_CARD_INSSPACE 3
#define SL_MESS_CARD_FULL 4
#define SL_MESS_FORMAT_CHOICE 5
#define SL_MESS_FORMAT_ERROR 6
#define SL_MESS_WRONGDEVICE 7
#define SL_MESS_CARD_INVALID 8
#define SL_MESS_SERIAL_INVALID 9
#define SL_MESS_NOSAVE_CHOICE 10
#define SL_MESS_CARD_REINSERT 11
#define SL_MESS_CARD_FORMAT_UNMOUNT 12

u64 SLSerialNo[2] = {};
static u32 boxDataOfs[SAVE_BOX_MAX*2] = {
    SAVE_BOX_OFS(0),
    SAVE_BOX_OFS(1),
    SAVE_BOX_OFS(2),
    SAVE_BOXBACKUP_OFS(0),
    SAVE_BOXBACKUP_OFS(1),
    SAVE_BOXBACKUP_OFS(2)
};

static u8 commentTbl[2][CARD_COMMENT_SIZE/2] = {
    "Mario Party 5",
    "00/00/0000"
};

BOOL SaveEnableF = TRUE;
static int SLCurWinId = HUWIN_NONE;
char SLSaveFileName[] = "MARIPA5";
char SLEraseStr[] = "ERASE";
HUWINID SLWinId = HUWIN_NONE;

u8 ATTRIBUTE_ALIGN(32) saveBuf[2][SAVE_BUF_SIZE];
CARDFileInfo curFileInfo;

BOOL saveExecF;
u8 curBoxNo;
s16 curSlotNo;

static HUWINID SLMessWinCreate(s32 mesNum, s32 insertMesNum1, s32 insertMesNum2, s16 posY);
static void SLMessWinKill(HUWINID winId);

void SLWinClose(void)
{
    SLWinId = HUWIN_NONE;
}

s32 SLFileOpen(char *fileName)
{
    s32 result;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    mount:
    result = SLCardMount(curSlotNo);
    if(result < 0) {
        return result;
    }
    result = HuCardOpen(curSlotNo, fileName, &curFileInfo);
    if(result == CARD_RESULT_NOFILE) {
        return CARD_RESULT_NOFILE;
    } else if(result == CARD_RESULT_WRONGDEVICE) {
        SLMessOut(SL_MESS_WRONGDEVICE);
        return CARD_RESULT_FATAL_ERROR;
    } else if(result == CARD_RESULT_FATAL_ERROR) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    } else if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    } else if(result == CARD_RESULT_BROKEN) {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result > 0 && result != SAVE_SECTOR_SIZE) {
            SLMessOut(SL_MESS_CARD_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            UnMountCnt = 0;
            result = SLMessOut(SL_MESS_FORMAT_CHOICE);
            if(result == 0) {
                result = SLFormat(curSlotNo);
                if(result == CARD_RESULT_READY) {
                    goto mount;
                } else {
                    return result;
                }
            } else {
                return CARD_RESULT_NOFILE;
            }
        }
    }
    return CARD_RESULT_READY;
}

s32 SLFileCreate(char *fileName, u32 size, void *addr)
{
    s32 result;
    u32 byteNotUsed;
    u32 filesNotUsed;
    void *eraseBuf;
    int winId;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    result = SLCardMount(curSlotNo);
    if(result < 0) {
        return result;
    }
    result = HuCardSectorSizeGet(curSlotNo);
    if(result < 0 && result != SAVE_SECTOR_SIZE) {
        SLMessOut(SL_MESS_CARD_INVALID);
        return CARD_RESULT_FATAL_ERROR;
    }
    result = HuCardFreeSpaceGet(curSlotNo, &byteNotUsed, &filesNotUsed);
    if(filesNotUsed == 0 && size > byteNotUsed) {
        SLMessOut(SL_MESS_CARD_FULL);
        return CARD_RESULT_INSSPACE;
    }
    if(filesNotUsed == 0) {
        SLMessOut(SL_MESS_CARD_NOENT);
        return CARD_RESULT_INSSPACE;
    }
    if(size > byteNotUsed) {
        SLMessOut(SL_MESS_CARD_INSSPACE);
        return CARD_RESULT_INSSPACE;
    }
    winId = SLMessWinCreate(MESS_MPSYSTEM_CARD_CREATEFILE, SL_CUR_SLOT_MESS, SL_MESSID_NONE, 200);
    HuSRDisableF = TRUE;
    result = HuCardCreate(curSlotNo, fileName, size, &curFileInfo);
    if(result < 0) {
        SLMessWinKill(winId);
        HuSRDisableF = FALSE;
    }
    if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    }
    if(result < 0) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    }
    eraseBuf = HuMemDirectMalloc(HEAP_HEAP, size);
    memset(eraseBuf, size, 0);
    memcpy(eraseBuf, SLEraseStr, sizeof(SLEraseStr));
    result = HuCardWrite(&curFileInfo, eraseBuf, size, 0);
    if(result == CARD_RESULT_READY) {
        result = HuCardWrite(&curFileInfo, addr, size, 0);
    }
    HuMemDirectFree(eraseBuf);
    HuSRDisableF = FALSE;
    if(result < 0) {
        SLMessWinKill(winId);
    }
    if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    }
    if(result < 0) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    }
    result = SLStatSet(FALSE);
    SLMessWinKill(winId);
    if(result < 0) {
        return result;
    } else {
        return CARD_RESULT_READY;
    }
}

s32 SLFileWrite(s32 length, void *addr)
{
    int winId;
    s32 result;
    HuVec2F size;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    HuWinInit(1);
    HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
    HuWinMesMaxSizeGet(1, &size, MESS_MPSYSTEM_MES_CARD_WRITE);
    winId = HuWinExCreateFrame(-10000, 200, size.x, size.y, HUWIN_SPEAKER_NONE, 2);
    HuWinExOpen(winId);
    HuWinInsertMesSet(winId, SL_CUR_SLOT_MESS, 0);
    HuWinMesSet(winId, MESS_MPSYSTEM_MES_CARD_WRITE);
    HuWinMesWait(winId);
    HuPrcSleep(60);
    SLSerialNoGet();
    HuSRDisableF = TRUE;
    result = HuCardWrite(&curFileInfo, addr, length, 0);
    HuSRDisableF = FALSE;
    if(result == CARD_RESULT_READY) {
        result = SLStatSet(FALSE);
    }
    HuWinExClose(winId);
    HuWinExKill(winId);
    (void)winId;
    (void)winId;
    return result;
}

s32 SLFileRead(s32 length, void *addr)
{
    s32 result;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    SLSerialNoGet();
    result = HuCardRead(&curFileInfo, addr, length, 0);
    if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
    } else if(result < 0) {
        SLMessOut(SL_MESS_FATAL_ERROR);
    }
    return result;
}

s32 SLFileClose(void)
{
    s32 result;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    result = HuCardClose(&curFileInfo);
    return result;
}

void SLCurSlotNoSet(s16 slotNo)
{
    curSlotNo = slotNo;
}

s16 SLCurSlotNoGet(void)
{
    return curSlotNo;
}

void SLCurBoxNoSet(s16 boxNo)
{
    curBoxNo = boxNo;
}

s16 SLCurBoxNoGet(void)
{
    return curBoxNo;
}

void SLSaveFlagSet(BOOL saveFlag)
{
    if(saveFlag == FALSE) {
        GwCommon.partyContinue = 0;
        GwCommon.storyContinue = 0;
    }
    SaveEnableF = saveFlag;
}

BOOL SLSaveFlagGet(void)
{
    return SaveEnableF;
}

void SLSaveEmptyMark(s16 slotNo, s16 boxNo)
{
    memcpy(&saveBuf[slotNo][SLBoxDataOffsetGet(boxNo)], "EMPT", 4);
    memcpy(&saveBuf[slotNo][SLBoxDataOffsetGet(boxNo+SAVE_BOXNO_BACKUP)], "EMPT", 4);
}

void SLSaveDataSlotMake(s16 slotNo, BOOL eraseF, OSTime *saveTime)
{
    u8 *buf = &saveBuf[slotNo][0];
    ANIMDATA *anim;
    u16 checkSum;
    if(eraseF) {
        memset(buf, 0xFF, SAVE_BUF_SIZE);
    }
    memcpy(buf, &commentTbl[0][0], CARD_COMMENT_SIZE/2);
    memcpy(buf+SAVE_COMMENT_DATE_OFS, &commentTbl[1][0], CARD_COMMENT_SIZE/2);
    anim = HuSprAnimDataRead(WIN_ANM_saveBanner);
    memcpy(buf+SAVE_BANNER_OFS, anim->bmp->data, CARD_BANNER_WIDTH*CARD_BANNER_HEIGHT);
    memcpy(buf+SAVE_BANNER_TLUT_OFS, anim->bmp->palData, 512);
    anim = HuSprAnimDataRead(WIN_ANM_saveIcon1+frandmod(4));
    memcpy(buf+SAVE_ICON_OFS, anim->bmp->data, CARD_ICON_WIDTH*CARD_ICON_HEIGHT*4);
    memcpy(buf+SAVE_ICON_TLUT_OFS, anim->bmp->palData, 512);
    SLSaveDataInfoSet(slotNo, saveTime);
    checkSum = SLCheckSumSlotGet(slotNo, 0, SAVE_MAIN_CHECKSUM_OFS);
    saveBuf[slotNo][SAVE_MAIN_CHECKSUM_OFS] = checkSum >> 8;
    saveBuf[slotNo][SAVE_MAIN_CHECKSUM_OFS+1] = checkSum & 0xFF;
}

void SLSaveDataMake(BOOL eraseF, OSTime *saveTime)
{
    SLSaveDataSlotMake(curSlotNo, eraseF, saveTime);
}

void SLSaveDataInfoSet(s16 slotNo, OSTime *saveTime)
{
    OSCalendarTime calendarTime;
    s16 digit;
    u16 year;
    u16 checkSum;
    OSTicksToCalendarTime(*saveTime, &calendarTime);
    digit = (calendarTime.mon+1)/10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS] = digit+'0';
    digit = (calendarTime.mon+1)%10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+1] = digit+'0';
    digit = (calendarTime.mday)/10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+3] = digit+'0';
    digit = (calendarTime.mday)%10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+4] = digit+'0';
    year = calendarTime.year;
    digit = year/1000;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+6] = digit+'0';
    year -= digit*1000;
    digit = year/100;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+7] = digit+'0';
    year -= digit*100;
    digit = year/10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+8] = digit+'0';
    year -= digit*10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+9] = year+'0';
    checkSum = SLCheckSumGet(0, SAVE_MAIN_CHECKSUM_OFS);
    saveBuf[slotNo][SAVE_MAIN_CHECKSUM_OFS] = checkSum >> 8;
    saveBuf[slotNo][SAVE_MAIN_CHECKSUM_OFS+1] = checkSum & 0xFF;
}

void SLCommonSet(void)
{
    u32 boxOfs = boxDataOfs[curBoxNo];
    OSTime time = OSGetTime();
    GwCommon.time = time;
    memcpy(&GwCommon.magic[0], "SAVE", 4);
    memcpy(&saveBuf[curSlotNo][boxOfs+SAVE_BOX_COMMON_OFS], &GwCommon, sizeof(GWCOMMON));
    SLSaveDataInfoSet(curSlotNo, &time);
}

void SLCommonSaveCopy(GWCOMMON *commonP, s16 slotNo, s16 boxNo)
{
    u32 boxOfs = boxDataOfs[boxNo];
    memcpy(&saveBuf[slotNo][boxOfs+SAVE_BOX_COMMON_OFS], commonP, sizeof(GWCOMMON));
}

void SLBoardSave(void)
{
    u32 boxOfs = boxDataOfs[curBoxNo];
    s16 i;
    memcpy(&saveBuf[curSlotNo][boxOfs+SAVE_BOX_BOARD_SYSTEM_OFS], &GwSystem, sizeof(GWSYSTEM));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        memcpy(&saveBuf[curSlotNo][boxOfs+SAVE_BOX_BOARD_PLAYER_OFS+(i*sizeof(GWPLAYER))], &GwPlayer[i], sizeof(GWPLAYER));
    }
    (void)i;
    (void)i;
    (void)i;
}

void SLBoardSaveStory(void)
{
    u32 boxOfs = boxDataOfs[curBoxNo];
    s16 i;
    memcpy(&saveBuf[curSlotNo][boxOfs+SAVE_BOX_STORY_SYSTEM_OFS], &GwSystem, sizeof(GWSYSTEM));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        memcpy(&saveBuf[curSlotNo][boxOfs+SAVE_BOX_STORY_PLAYER_OFS+(i*sizeof(GWPLAYER))], &GwPlayer[i], sizeof(GWPLAYER));
    }
    (void)i;
    (void)i;
    (void)i;
}

s32 SLSave(void)
{
    s32 result;
    SLCheckSumBoxAllSet();
    SLSaveBackup();
    repeat:
    result = SLFileOpen(SLSaveFileName);
    if(result == CARD_RESULT_NOFILE) {
        if(!SLSerialNoCheck()) {
            SLMessOut(SL_MESS_SERIAL_INVALID);
            goto savefail;
        } else {
            SLCurWinId = SLMessWinCreate(MESS_MPSYSTEM_MES_SAVE_SAVE, SL_CUR_SLOT_MESS, -1, 150);
            result = SLFileCreate(SLSaveFileName, SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
            SLMessWinKill(SLCurWinId);
            SLCurWinId = HUWIN_NONE;
            if(result < 0) {
                goto savefail;
            }
            SLSerialNoGet();
        }
    } else if(result == CARD_RESULT_NOCARD) {
        result = SLMessOut(SL_MESS_NOSAVE_CHOICE);
        if(result != 0) {
            SLMessOut(SL_MESS_CARD_REINSERT);
            goto repeat;
        }
        SLSaveFlagSet(FALSE);
        goto unmount;
    } else if(result < 0) {
        goto savefail;
    } else {
        if(!SLSerialNoCheck()) {
            SLMessOut(SL_MESS_SERIAL_INVALID);
            goto savefail;
        }
        SLCurWinId = SLMessWinCreate(MESS_MPSYSTEM_MES_SAVE_SAVE, SL_CUR_SLOT_MESS, -1, 150);
        result = SLFileWrite(SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
        SLMessWinKill(SLCurWinId);
        SLCurWinId = HUWIN_NONE;
        if(result == CARD_RESULT_NOCARD) {
            SLMessOut(SL_MESS_NOCARD);
        } else if(result == CARD_RESULT_WRONGDEVICE) {
            SLMessOut(SL_MESS_WRONGDEVICE);
        } else if(result == CARD_RESULT_BROKEN) {
            result = HuCardSectorSizeGet(curSlotNo);
            if(result > 0 && result != SAVE_SECTOR_SIZE) {
                SLMessOut(SL_MESS_CARD_INVALID);
                goto savefail;
            } else {
                UnMountCnt = 0;
                result = SLMessOut(SL_MESS_FORMAT_CHOICE);
                if(result == 0) {
                    result = SLFormat(curSlotNo);
                    if(result == CARD_RESULT_READY) {
                        goto repeat;
                    } else {
                        return result;
                    }
                } else {
                    result = CARD_RESULT_BROKEN;
                }
            }
        } else if(result < 0) {
            SLMessOut(SL_MESS_FATAL_ERROR);
        }
    }
    SLFileClose();
    if(result >= 0) {
        HuCardUnMount(curSlotNo);
        return TRUE;
    }
    savefail:
    result = SLMessOut(SL_MESS_NOSAVE_CHOICE);
    if(result != 0) {
        SLMessOut(SL_MESS_CARD_REINSERT);
        goto repeat;
    }
    SLSaveFlagSet(FALSE);
    unmount:
    HuCardUnMount(curSlotNo);
    return FALSE;
}

s32 SLLoad(void)
{
    s32 result = SLFileOpen(SLSaveFileName);
    if(result >= 0) {
        result = SLFileRead(SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
        SLFileClose();
        if(result >= 0) {
            u16 *checkSumBuf = ((u16 *)&saveBuf[curSlotNo][SAVE_BOX_SIZE]);
            u16 checkSum = SLCheckSumGet(0, SAVE_BOX_SIZE);
            *checkSumBuf == checkSum;
        }
    }
    HuCardUnMount(curSlotNo);
    return FALSE;
}

void SLCommonLoad(void)
{
    u32 boxOfs = boxDataOfs[curBoxNo];
    memcpy(&GwCommon, &saveBuf[curSlotNo][boxOfs+SAVE_BOX_COMMON_OFS], sizeof(GWCOMMON));
}

void SLCommonLoadCopy(GWCOMMON *commonP, s16 slotNo, s16 boxNo)
{
    u32 boxOfs = boxDataOfs[boxNo];
    memcpy(commonP, &saveBuf[slotNo][boxOfs+SAVE_BOX_COMMON_OFS], sizeof(GWCOMMON));
}

void SLBoardLoad(void)
{
    u32 boxOfs = boxDataOfs[curBoxNo];
    s16 i;
    memcpy(&GwSystem, &saveBuf[curSlotNo][boxOfs+SAVE_BOX_BOARD_SYSTEM_OFS], sizeof(GWSYSTEM));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        memcpy(&GwPlayer[i], &saveBuf[curSlotNo][boxOfs+SAVE_BOX_BOARD_PLAYER_OFS+(i*sizeof(GWPLAYER))], sizeof(GWPLAYER));
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].dif = GwPlayer[i].dif;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].grp = GwPlayer[i].grp;
    }
}

void SLBoardLoadStory(void)
{
    u32 boxOfs = boxDataOfs[curBoxNo];
    GWCOMMON *commonP = HuMemDirectMalloc(HEAP_HEAP, sizeof(GWCOMMON));
    s16 i;
    memcpy(commonP, &saveBuf[curSlotNo][boxOfs+SAVE_BOX_COMMON_OFS], sizeof(GWCOMMON));
    for(i=0; i<5; i++) {
        GwCommon.storyBoardNo[i] = commonP->storyBoardNo[i];
        GwCommon.storyRank[i] = commonP->storyRank[i];
    }
    GwCommon.storyNo = commonP->storyNo;
    GwCommon.storyMgPack = commonP->storyMgPack;
    HuMemDirectFree(commonP);
    memcpy(&GwSystem, &saveBuf[curSlotNo][boxOfs+SAVE_BOX_STORY_SYSTEM_OFS], sizeof(GWSYSTEM));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        memcpy(&GwPlayer[i], &saveBuf[curSlotNo][boxOfs+SAVE_BOX_STORY_PLAYER_OFS+(i*sizeof(GWPLAYER))], sizeof(GWPLAYER));
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].dif = GwPlayer[i].dif;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].grp = 0;
    }
}

void SLSdLoad(s16 slotNo, s16 boxNo, GWSDCOMMON *sdCommonP)
{
    u32 boxOfs = boxDataOfs[boxNo];
    memcpy(sdCommonP, &saveBuf[slotNo][boxOfs+SAVE_BOX_SDCOMMON_OFS], sizeof(GWSDCOMMON));
}

void SLSdSave(s16 slotNo, s16 boxNo, GWSDCOMMON *sdCommonP)
{
    u32 boxOfs = boxDataOfs[boxNo];
    memcpy(&saveBuf[slotNo][boxOfs+SAVE_BOX_SDCOMMON_OFS], sdCommonP, sizeof(GWSDCOMMON));
}

s32 SLSerialNoGet(void)
{
    return CARDGetSerialNo(curSlotNo, &SLSerialNo[curSlotNo]);
}

BOOL SLSerialNoCheck(void)
{
    u64 serialNo;
    if(SLSerialNo[curSlotNo] == 0) {
        return TRUE;
    } else {
        s32 result = CARDGetSerialNo(curSlotNo, &serialNo);
        if(result < 0) {
            return TRUE;
        }
        if(SLSerialNo[curSlotNo] != serialNo) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

BOOL SLCheckSumBoxSlotCheck(s16 slotNo, s16 boxNo)
{
    u32 boxOfs = boxDataOfs[boxNo];
    u16 *checkSumBuf = (u16 *)&saveBuf[slotNo][boxOfs+SAVE_BOX_SIZE-2];
    u16 checkSum = SLCheckSumSlotGet(slotNo, boxOfs, SAVE_BOX_SIZE-2);
    if(*checkSumBuf == checkSum) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL SLCheckSumCheck(s16 slotNo, s16 boxNo)
{
    return SLCheckSumBoxSlotCheck(curSlotNo, curBoxNo);
}

u16 SLCheckSumSlotGet(s16 slotNo, u32 ofs, u32 size)
{
    u32 value;
    u32 i;
    for(i=value=0; i<size; i++) {
        value = value+saveBuf[slotNo][ofs+i];
    }
    value = ~value;
    (void)value;
    (void)value;
    (void)value;
    (void)i;
    (void)i;
    return ((u16)value) & 0xFFFF;
}

u16 SLCheckSumGet(u32 ofs, u32 size)
{
    return SLCheckSumSlotGet(curSlotNo, ofs, size);
}

void SLCheckSumBoxSet(void)
{
    u32 boxOfs = boxDataOfs[curBoxNo];
    u16 checkSum = SLCheckSumGet(boxOfs, SAVE_BOX_SIZE-2);
    saveBuf[curSlotNo][boxOfs+SAVE_BOX_SIZE-2] = (checkSum >> 8) & 0xFF;
    saveBuf[curSlotNo][boxOfs+SAVE_BOX_SIZE-1] = checkSum & 0xFF;
}

void SLCheckSumBoxAllSet(void)
{
    s16 oldBoxNo = curBoxNo;
    for(curBoxNo=0; curBoxNo<SAVE_BOX_MAX; curBoxNo++) {
        SLCheckSumBoxSet();
    }
    curBoxNo = oldBoxNo;
}

void SLSaveBackup(void)
{
    memcpy(&saveBuf[curSlotNo][SAVE_BOXBACKUP_OFS(0)], &saveBuf[curSlotNo][SAVE_BOX_OFS(0)], SAVE_BOX_SIZE*SAVE_BOX_MAX);
}

void SLBoxBackupSlotLoad(s16 slotNo, s16 boxNo)
{
    memcpy(&saveBuf[slotNo][boxDataOfs[boxNo]], &saveBuf[slotNo][boxDataOfs[boxNo+SAVE_BOXNO_BACKUP]], SAVE_BOX_SIZE);
}

void SLBoxBackupLoad(s16 boxNo)
{
    SLBoxBackupSlotLoad(curSlotNo, boxNo);
}

u32 SLBoxDataOffsetGet(s16 boxNo)
{
    return boxDataOfs[boxNo];
}

s32 SLStatSet(BOOL errorOutF)
{
    CARDStat stat;
    s32 fileNo = curFileInfo.fileNo;
    s32 result = CARDGetStatus(curSlotNo, fileNo, &stat);
    if(result == CARD_RESULT_NOCARD) {
        if(errorOutF) {
            SLMessOut(SL_MESS_NOCARD);
        }
        return CARD_RESULT_NOCARD;
    } else if(result < 0) {
        if(errorOutF) {
            SLMessOut(SL_MESS_FATAL_ERROR);
        }
        return CARD_RESULT_FATAL_ERROR;
    }
    CARDSetCommentAddress(&stat, 0);
    CARDSetIconAddress(&stat, 64);
    CARDSetBannerFormat(&stat, CARD_STAT_BANNER_C8);
    CARDSetIconFormat(&stat, 0, CARD_STAT_ICON_C8);
    CARDSetIconFormat(&stat, 1, CARD_STAT_ICON_C8);
    CARDSetIconFormat(&stat, 2, CARD_STAT_ICON_C8);
    CARDSetIconFormat(&stat, 3, CARD_STAT_ICON_C8);
    CARDSetIconSpeed(&stat, 0, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 1, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 2, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 3, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 4, CARD_STAT_SPEED_END);
    CARDSetIconAnim(&stat, CARD_STAT_ANIM_LOOP);
    result = CARDSetStatus(curSlotNo, fileNo, &stat);
    if(result == CARD_RESULT_NOCARD) {
        if(errorOutF) {
            SLMessOut(SL_MESS_NOCARD);
        }
        return CARD_RESULT_NOCARD;
    } else if(result < 0) {
        if(errorOutF) {
            SLMessOut(SL_MESS_FATAL_ERROR);
        }
        return CARD_RESULT_FATAL_ERROR;
    }
    return result;
}

s32 SLCardMount(s16 slotNo)
{
    s32 result;
    mount:
    result = HuCardMount(curSlotNo);
    if(result == CARD_RESULT_WRONGDEVICE) {
        SLMessOut(SL_MESS_WRONGDEVICE);
        return result;
    } else if(result == CARD_RESULT_FATAL_ERROR) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    } else if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    } else if(result == CARD_RESULT_BROKEN) {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result > 0 && result != SAVE_SECTOR_SIZE) {
            SLMessOut(SL_MESS_CARD_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            UnMountCnt = 0;
            result = SLMessOut(SL_MESS_FORMAT_CHOICE);
            if(result == 0) {
                result = SLFormat(curSlotNo);
                if(result == CARD_RESULT_READY) {
                    goto mount;
                } else {
                    return result;
                }
            } else {
                return CARD_RESULT_FATAL_ERROR;
            }
        }
    } else {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result < 0) {
            SLMessOut(SL_MESS_FATAL_ERROR);
            return result;
        } else if(result != SAVE_SECTOR_SIZE) {
            SLMessOut(SL_MESS_CARD_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            return CARD_RESULT_READY;
        }
    }
}

s32 SLFormat(s16 slotNo)
{
    s16 result;
    if(UnMountCnt & (1 << curSlotNo)) {
        SLMessOut(SL_MESS_CARD_FORMAT_UNMOUNT);
        UnMountCnt = 0;
        return CARD_RESULT_READY;
    } else {
        HUWINID winId = SLMessWinCreate(MESS_MPSYSTEM_CARD_FORMAT_WARN, SL_CUR_SLOT_MESS, SL_MESSID_NONE, 200);
        HuPrcSleep(30);
        (void)winId;
        (void)winId;
        if(UnMountCnt & (1 << curSlotNo)) {
            SLMessWinKill(winId);
            SLMessOut(SL_MESS_CARD_FORMAT_UNMOUNT);
            UnMountCnt = 0;
            return CARD_RESULT_READY;
        } else {
            result = HuCardFormat(curSlotNo);
            SLSerialNo[curSlotNo] = 0;
            if(result < 0) {
                SLMessWinKill(winId);
            }
            if(result == CARD_RESULT_FATAL_ERROR) {
                SLMessOut(SL_MESS_FORMAT_ERROR);
                SLMessOut(SL_MESS_FATAL_ERROR);
                return CARD_RESULT_FATAL_ERROR;
            } else if(result == CARD_RESULT_NOCARD) {
                SLMessOut(SL_MESS_NOCARD);
                return CARD_RESULT_NOCARD;
            } else if(result == CARD_RESULT_WRONGDEVICE) {
                SLMessOut(SL_MESS_WRONGDEVICE);
                return result;
            } else {
                OSTime time;
                SLSerialNoGet();
                SLMessWinKill(winId);
                time = OSGetTime();
                SLSaveDataMake(FALSE, &time);
                SLCheckSumBoxAllSet();
                return result;
            }
        }
    }
    
    (void)result;
    (void)result;
}

void SLWinIdSet(HUWINID winId)
{
    SLWinId = winId;
}

static HUWINID SLMessWinCreate(s32 mesNum, s32 insertMesNum1, s32 insertMesNum2, s16 posY)
{
    HUWINID winId;
    HuVec2F size;
    if(SLWinId == HUWIN_NONE) {
        HuWinInit(1);
    }
    if(insertMesNum1 != SL_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMesNum1, 0);
    }
    if(insertMesNum2 != SL_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMesNum2, 1);
    }
    HuWinMesMaxSizeGet(1, &size, mesNum);
    if(SLWinId == HUWIN_NONE) {
        winId = HuWinExCreateFrame(-10000, posY, size.x, size.y, HUWIN_SPEAKER_NONE, 2);
        HuWinExOpen(winId);
    } else {
        winId = SLWinId;
    }
    if(insertMesNum1 != SL_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMesNum1, 0);
    }
    if(insertMesNum2 != SL_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMesNum2, 1);
    }
    HuWinMesSet(winId, mesNum);
    HuWinMesWait(winId);
    (void)winId;
    (void)insertMesNum1;
    (void)insertMesNum2;
    return winId;
}

static void SLMessWinKill(HUWINID winId)
{
    if(SLWinId == winId) {
        return;
    }
    if(winId < 0) {
        return;
    }
    HuWinExClose(winId);
    HuWinExKill(winId);
}

s16 SLMessOut(s16 messId)
{
    u32 mess;
    HUWINID winId;
    s16 choiceNo = -1;
    u32 insertMes = 0;
    BOOL choiceF = FALSE;

    HUWIN *winP;
    if(SLWinId == HUWIN_NONE) {
        HuWinInit(1);
    }
    SLMessWinKill(SLCurWinId);
    SLCurWinId = HUWIN_NONE;
    switch(messId) {
        case SL_MESS_NOCARD:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_NOCARD;
            break;
        
        case SL_MESS_FATAL_ERROR:
            mess = MESS_MPSYSTEM_CARD_FATAL_ERROR;
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            break;
        
        case SL_MESS_CARD_NOENT:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_NOENT;
            break;
        
        case SL_MESS_CARD_INSSPACE:
            mess = MESS_MPSYSTEM_CARD_INSSPACE;
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            break;
        
        case SL_MESS_CARD_FULL:
            mess = MESS_MPSYSTEM_CARD_FULL;
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            break;
        
        case SL_MESS_FORMAT_CHOICE:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_FORMAT_CHOICE;
            choiceF = TRUE;
            break;
        
        case SL_MESS_FORMAT_ERROR:
            mess = MESS_MPSYSTEM_CARD_FORMAT_ERROR;
            break;
        
        case SL_MESS_WRONGDEVICE:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_WRONGDEVICE;
            break;
        
        case SL_MESS_CARD_INVALID:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_INVALID;
            break;
        
        case SL_MESS_SERIAL_INVALID:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_SERIAL_INVALID;
            break;
        
        case SL_MESS_NOSAVE_CHOICE:
            mess = MESS_MPSYSTEM_NOSAVE_CHOICE;
            choiceF = TRUE;
            break;
        
        case SL_MESS_CARD_REINSERT:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_REINSERT;
            break;
        
        case SL_MESS_CARD_FORMAT_UNMOUNT:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertMes = SL_CUR_SLOT_MESS;
            mess = MESS_MPSYSTEM_CARD_FORMAT_UNMOUNT;
            break;
    }
    if(SLWinId == HUWIN_NONE) {
        HuVec2F size;
        size.x = 478;
        size.y = 94;
        winId = HuWinExCreateFrame(-10000, 200, size.x, size.y, HUWIN_SPEAKER_NONE, 2);
    } else {
        winId = SLWinId;
    }
    winP = &winData[winId];
    winP->padMask = HUWIN_PLAYER_1;
    if(insertMes) {
        HuWinInsertMesSet(winId, insertMes, 0);
    }
    HuWinAttrSet(winId, HUWIN_ATTR_NOCANCEL);
    HuWinExOpen(winId);
    HuWinMesSet(winId, mess);
    HuWinMesWait(winId);
    if(choiceF) {
        if(messId == SL_MESS_FORMAT_CHOICE) {
            HuWinInsertMesSet(winId, SL_CUR_SLOT_MESS, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_FORMAT);
            HuWinMesWait(winId);
        }
        choiceNo = HuWinChoiceGet(winId, -1);
    }
    if(messId == SL_MESS_CARD_REINSERT) {
        while(!(HuPadBtnDown[0] & PAD_BUTTON_A)) {
            HuPrcVSleep();
        }
        HuAudFXPlay(MSM_SE_CMN_02);
    }
    if(SLWinId == HUWIN_NONE) {
        HuWinExClose(winId);
        HuWinExKill(winId);
    }
    return choiceNo;
}

void SLSaveBoardTurnExec(void)
{
    s16 sprId;
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    HuWinComKeyReset();
    if(!SaveEnableF || GWSaveModeGet() == 1) {
        return;
    }
    HuPrcVSleep();
    sprId = espEntry(WIN_ANM_saveBgBoard, 5000, 0);
    espPosSet(sprId, 288.0, 240.0);
    espAttrReset(sprId, HUSPR_ATTR_DISPOFF);
    Hu3DBGColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 20);
    WipeWait();
    if(GWSaveModeGet() == 2) {
        GWSaveModeSet(1);
    }
    if(GWPartyFGet() == TRUE) {
        GwCommon.partyContinue = TRUE;
        SLBoardSave();
    } else {
        GwCommon.storyContinue = TRUE;
        SLBoardSaveStory();
    }
    SLCommonSet();
    if(SLSave()) {
        saveExecF = TRUE;
    } else {
        GWSaveModeSet(1);
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 20);
    WipeWait();
}

void SLSaveBoardEndExec(void)
{
    s16 sprId;
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    HuWinComKeyReset();
    if(!SaveEnableF) {
        return;
    }
    HuPrcVSleep();
    sprId = espEntry(WIN_ANM_saveBgBoard, 5000, 0);
    espPosSet(sprId, 288.0, 240.0);
    espAttrReset(sprId, HUSPR_ATTR_DISPOFF);
    Hu3DBGColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 20);
    WipeWait();

    if(GWPartyFGet() == TRUE) {
        GwCommon.partyContinue = FALSE;
        SLBoardSave();
    } else {
        GwCommon.storyContinue = FALSE;
        SLBoardSaveStory();
    }
    SLCommonSet();
    if(SLSave()) {
        saveExecF = TRUE;
    } else {
        GWSaveModeSet(1);
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 20);
    WipeWait();
}

void SLSaveStoryExec(void)
{
    s16 sprId;
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    HuWinComKeyReset();
    if(!SaveEnableF) {
        return;
    }
    HuPrcVSleep();
    sprId = espEntry(WIN_ANM_saveBgStory, 5000, 0);
    espPosSet(sprId, 288.0, 240.0);
    espAttrReset(sprId, HUSPR_ATTR_DISPOFF);
    Hu3DBGColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 20);
    WipeWait();
    GwCommon.storyContinue = TRUE;
    SLBoardSaveStory();
    SLCommonSet();
    if(SLSave()) {
        saveExecF = TRUE;
    } else {
        GWSaveModeSet(1);
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 20);
    WipeWait();
}

void SLSaveStaffExec(void)
{
    s16 sprId;
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    HuWinComKeyReset();
    if(!SaveEnableF) {
        return;
    }
    HuPrcVSleep();
    sprId = espEntry(WIN_ANM_saveBgStory, 5000, 0);
    espPosSet(sprId, 288.0, 240.0);
    espAttrReset(sprId, HUSPR_ATTR_DISPOFF);
    Hu3DBGColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 20);
    WipeWait();
    GwCommon.storyContinue = FALSE;
    if(GwPlayerConf[0].dif == GW_DIF_HARD) {
        GwCommon.veryHardUnlock = TRUE;
    }
    if(GwPlayerConf[0].dif == GW_DIF_VERYHARD) {
        GwCommon.m562VeryHardUnlock = TRUE;
    }
    GwCommon.w07Unlock = TRUE;
    SLCommonSet();
    if(SLSave()) {
        saveExecF = TRUE;
    } else {
        GWSaveModeSet(1);
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 20);
    WipeWait();
}

void SLSaveModeExec(s16 sdModeF)
{
    s16 sprId;

    HuWinComKeyReset();
    if(!SaveEnableF) {
        return;
    }
    HuPrcVSleep();
    if(!sdModeF) {
        sprId = espEntry(WIN_ANM_saveBgMode, 5000, 0);
    } else {
        sprId = espEntry(WIN_ANM_saveBgSd, 5000, 0);
    }
    espPosSet(sprId, 288.0, 240.0);
    espAttrReset(sprId, HUSPR_ATTR_DISPOFF);
    Hu3DBGColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 20);
    WipeWait();
    SLCommonSet();
    if(SLSave()) {
        saveExecF = TRUE;
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 20);
    WipeWait();
    espKill(sprId);
    (void)sprId;
    (void)sprId;
    (void)sprId;
}

void SLSaveSdExec(void)
{
    if(!SaveEnableF) {
        return;
    } else {
        s16 slotNo = curSlotNo;
        int boxNo = curBoxNo;
        s16 sprId;
        HuWinComKeyReset();
        HuPrcVSleep();
        sprId = espEntry(WIN_ANM_saveBgSd, 5000, 0);
        espPosSet(sprId, 288.0, 240.0);
        espAttrReset(sprId, HUSPR_ATTR_DISPOFF);
        Hu3DBGColorSet(0, 0, 0);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREVTYPE, 20);
        WipeWait();
        SLCommonSet();
        SLSdSave(0, SLCurBoxNoGet(), &GwSdCommon);
        if(SLSave()) {
            saveExecF = TRUE;
        }
        if(SaveEnableF) {
            curSlotNo = 1;
            curBoxNo = 0;
            SLSaveSdBExec();
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITEFADE, 20);
        WipeWait();
        espKill(sprId);
        curSlotNo = slotNo;
        curBoxNo = boxNo;
    }
}

s32 SLSaveSdBExec(void)
{
    s32 result;
    SLCheckSumBoxAllSet();
    SLSaveBackup();
    repeat:
    result = SLFileOpen(SLSaveFileName);
    if(result == CARD_RESULT_NOFILE) {
        if(!SLSerialNoCheck()) {
            SLMessOut(SL_MESS_SERIAL_INVALID);
            goto savefail;
        } else {
            SLCurWinId = SLMessWinCreate(MESS_MPSYSTEM_MES_SAVE_SAVE, SL_CUR_SLOT_MESS, -1, 150);
            result = SLFileCreate(SLSaveFileName, SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
            SLMessWinKill(SLCurWinId);
            SLCurWinId = HUWIN_NONE;
            if(result < 0) {
                goto savefail;
            }
            SLSerialNoGet();
        }
    } else if(result == CARD_RESULT_NOCARD) {
        goto savefail;
    } else if(result < 0) {
        goto savefail;
    } else {
        if(!SLSerialNoCheck()) {
            SLMessOut(SL_MESS_SERIAL_INVALID);
            goto savefail;
        }
        SLCurWinId = SLMessWinCreate(MESS_MPSYSTEM_MES_SAVE_SAVE, SL_CUR_SLOT_MESS, -1, 150);
        result = SLFileWrite(SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
        SLMessWinKill(SLCurWinId);
        SLCurWinId = HUWIN_NONE;
        if(result == CARD_RESULT_NOCARD) {
            SLMessOut(SL_MESS_NOCARD);
        } else if(result == CARD_RESULT_WRONGDEVICE) {
            SLMessOut(SL_MESS_WRONGDEVICE);
        } else if(result == CARD_RESULT_BROKEN) {
            result = HuCardSectorSizeGet(curSlotNo);
            if(result > 0 && result != SAVE_SECTOR_SIZE) {
                SLMessOut(SL_MESS_CARD_INVALID);
                goto savefail;
            } else {
                UnMountCnt = 0;
                result = SLMessOut(SL_MESS_FORMAT_CHOICE);
                if(result == 0) {
                    result = SLFormat(curSlotNo);
                    if(result == CARD_RESULT_READY) {
                        goto repeat;
                    } else {
                        return result;
                    }
                } else {
                    result = CARD_RESULT_BROKEN;
                }
            }
        } else if(result < 0) {
            SLMessOut(SL_MESS_FATAL_ERROR);
        }
    }
    SLFileClose();
    if(result >= 0) {
        HuCardUnMount(curSlotNo);
        return TRUE;
    }
    savefail:
    {
        int winId = HuWinExCreateFrame(-10000, 200, 478, 94, HUWIN_SPEAKER_NONE, 2);
        HUWIN *winP;
        HuWinAttrSet(winId, HUWIN_ATTR_NOCANCEL);
        winP = &winData[winId];
        winP->padMask = HUWIN_PLAYER_1;
        HuWinExOpen(winId);
        HuWinInsertMesSet(winId, SL_CUR_SLOT_MESS, 0);
        HuWinMesSet(winId, MESS_SDG_FILE_SAVEFAIL);
        HuWinMesWait(winId);
        result = HuWinChoiceGet(winId, -1);
        if(result != 0) {
            HuWinInsertMesSet(winId, SL_CUR_SLOT_MESS, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_REINSERT);
            HuWinMesWait(winId);
            while(!(HuPadBtnDown[0] & PAD_BUTTON_A)) {
                HuPrcVSleep();
            }
            HuAudFXPlay(MSM_SE_CMN_02);
            HuWinExClose(winId);
            goto repeat;
        } else {
            HuWinExClose(winId);
            HuCardUnMount(curSlotNo);
            return FALSE;
        }
    }
    
}
