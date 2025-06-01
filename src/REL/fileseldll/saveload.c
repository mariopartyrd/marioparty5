#include "REL/fileseldll.h"
#include "game/pad.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/main.h"
#include "game/sreset.h"

#include "messnum/mpsystem.h"
#include "datanum/win.h"

#define FILECARD_FLAG_NOSAVE (1 << 0)
#define FILECARD_FLAG_FORMAT (1 << 1)
#define FILECARD_FLAG_RETRY (1 << 2)
#define FILECARD_FLAG_MENU (1 << 3)
#define FILECARD_FLAG_CANCEL (1 << 4)
#define FILECARD_FLAG_ALL 0x1F
#define FILECARD_CHOICE_MAX 4

#define FILECARD_MESSID_NONE -1

#define FILE_MESS_NOCARD 0
#define FILE_MESS_FATALERROR 1
#define FILE_MESS_NOENT 2
#define FILE_MESS_INSSPACE 3
#define FILE_MESS_FULL 4
#define FILE_MESS_BROKEN 5
#define FILE_MESS_FORMATERROR 6
#define FILE_MESS_WRONGDEVICE 7
#define FILE_MESS_INVALID 8
#define FILE_MESS_SERIAL_INVALID 9
#define FILE_MESS_NOSAVE_CHOICE 10
#define FILE_MESS_REINSERT 11
#define FILE_MESS_FORMAT_UNMOUNT 12

static HUSPRGRPID choiceGrpId = HUSPR_GRP_NONE;
static u32 fileMesId = FILECARD_MESSID_NONE;

s16 FileMessOut(s16 messNo);
int FileCardChoice(s16 flag, float posY);

s32 FileCardLoad(void);
s32 FileCheckCardSpace(void);
s32 FileCardErrorExec(s32 result, HUWINID winId);



void FileCommonInit(void)
{
    GwCommon = GwCommonOrig;
    GWSdCommonInit();
}

s32 FileBoxInit(HUWINID srcWinId)
{
    BOOL invalidF = FALSE;
    s16 i;
    HUWINID winId;
    s32 result;
    u16 *checksum;
    
    int slotStat[SAVE_BOX_MAX];
    OSTime time;
    UnMountCnt = 0;
    if(srcWinId == HUWIN_NONE) {
        winId = HuWinExCreateFrame(-10000, 130, 478, 94, HUWIN_SPEAKER_NONE, 2);
    } else {
        winId = srcWinId;
    }
    SLWinIdSet(winId);
    SLCurSlotNoSet(0);
    while(1) {
        result = FileCardLoad();
        if(result == CARD_RESULT_NOFILE) {
            for(i=0; i<SAVE_BOX_MAX; i++) {
                fileselBox[i].validF = FALSE;
            }
            result = FileCheckCardSpace();
            if(result == CARD_RESULT_READY) {
                break;
            } else {
                goto cardError;
            }
            
        } else if(result != CARD_RESULT_READY) {
            cardError:
            result = FileCardErrorExec(result, winId);
            if(result == CARD_RESULT_NOCARD || result == CARD_RESULT_NOFILE || result == CARD_RESULT_IOERROR) {
                continue;
            }
            if(result != FILESEL_RESULT_CANCEL) {
                goto cleanup;
            }
            HuWinExOpen(winId);
            HuWinMesSet(winId, MESS_MPSYSTEM_TITLE_RETURN);
            HuWinMesWait(winId);
            result = HuWinChoiceGet(winId, 0);
            HuWinExClose(winId);
            if(result == 0) {
                result = CARD_RESULT_WRONGDEVICE;
                goto cleanup;
            } else {
                continue;
            }
        } else if(!strncmp(&saveBuf[curSlotNo][0], "ERASE", 5)) {
            time = OSGetTime();
            SLSaveDataMake(TRUE, &time);
            for(i=0; i<SAVE_BOX_MAX; i++) {
                SLSaveEmptyMark(curSlotNo, i);
                fileselBox[i].validF = FALSE;
            }
            break;
        } else {
            invalidF = FALSE;
            for(i=0; i<SAVE_BOX_MAX; i++) {
                slotStat[i] = 0;
            }
            checksum = (u16 *)&saveBuf[curSlotNo][SAVE_ICONBANNER_CHECKSUM_OFS];
            if(*checksum != SLCheckSumGet(0, SAVE_ICONBANNER_SIZE)) {
                OSReport("IconBanner Area Broken!\n");
                time = OSGetTime();
                SLSaveDataMake(FALSE, &time);
                invalidF = TRUE;
            }
            for(i=0; i<SAVE_BOX_MAX; i++) {
                SLCurBoxNoSet(i);
                if(!strncmp(&saveBuf[curSlotNo][SLBoxDataOffsetGet(i)], "SAVE", 4) ||
                !strncmp(&saveBuf[curSlotNo][SLBoxDataOffsetGet(i+SAVE_BOXNO_BACKUP)], "SAVE", 4)) {
                    if(!SLCheckSumCheck()) {
                        OSReport("Box%d Broken!\n", i);
                        SLBoxBackupLoad(i);
                        if(!SLCheckSumCheck()) {
                            SLSaveEmptyMark(curSlotNo, i);
                            slotStat[i] = 2;
                        } else {
                            slotStat[i] = 1;
                        }
                    }
                } else {
                    if(strncmp(&saveBuf[curSlotNo][SLBoxDataOffsetGet(i)], "EMPT", 4) &&
                        strncmp(&saveBuf[curSlotNo][SLBoxDataOffsetGet(i+SAVE_BOXNO_BACKUP)], "EMPT", 4)) {
                        slotStat[i] = 3;
                        SLSaveEmptyMark(curSlotNo, i);
                    } else if(!strncmp(&saveBuf[curSlotNo][SLBoxDataOffsetGet(i)], "EMPT", 4) ||
                        !strncmp(&saveBuf[curSlotNo][SLBoxDataOffsetGet(i+SAVE_BOXNO_BACKUP)], "EMPT", 4)){
                        SLSaveEmptyMark(curSlotNo, i);
                    }
                }
                SLCommonLoad();
                if(!strncmp(GwCommon.magic, "SAVE", 4)) {
                    fileselBox[i].validF = TRUE;
                    SLBoardLoadStory();
                    if(GwCommon.storyContinue) {
                        fileselBox[i].charNo = GwPlayer[0].charNo;
                    } else {
                        fileselBox[i].charNo = 7;
                    }
                    OSTicksToCalendarTime(GwCommon.time, &fileselBox[i].time);
                    memcpy(fileselBox[i].name, GwCommon.name, 17);
                } else {
                    fileselBox[i].validF = FALSE;
                }
                GWLanguageSet(HUWIN_LANG_ENGLISH);
            }
            if(invalidF || slotStat[0] || slotStat[1] || slotStat[2]) {
                HuWinExOpen(winId);
                if(slotStat[0] != 2 && slotStat[0] != 3 &&
                    slotStat[1] != 2 && slotStat[1] != 3 &&
                    slotStat[2] != 2 && slotStat[2] != 3) {
                        HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
                        HuWinMesSet(winId, MESS_MPSYSTEM_SAVE_SLOT_REPAIR);
                        HuWinMesWait(winId);
                    } else {
                        for(i=0; i<SAVE_BOX_MAX; i++) {
                            char fileStr[8];
                            sprintf(fileStr, "%d", i+1);
                            if(slotStat[i] == 1) {
                                HuWinInsertMesSet(winId, MESSNUM_PTR(fileStr), 0);
                                HuWinMesSet(winId, MESS_MPSYSTEM_SAVE_REPAIR);
                                HuWinMesWait(winId);
                            } else if(slotStat[i] == 2) {
                                HuWinInsertMesSet(winId, MESSNUM_PTR(fileStr), 0);
                                HuWinMesSet(winId, MESS_MPSYSTEM_SAVE_BROKEN);
                                HuWinMesWait(winId);
                            } else if(slotStat[i] == 3) {
                                HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
                                HuWinInsertMesSet(winId, MESSNUM_PTR(fileStr), 1);
                                HuWinMesSet(winId, MESS_MPSYSTEM_INVALID_MAGIC);
                                HuWinMesWait(winId);
                            }
                        }
                    }
            } else {
                break;
            }
            HuWinExClose(winId);
            result = FileSaveMesOpen(HUWIN_NONE, MESS_MPSYSTEM_MES_SAVE_SAVE);
        }
        if(UnMountCnt) {
            result = CARD_RESULT_NOFILE;
            UnMountCnt = 0;
        }
        if(result == FILESEL_RESULT_CANCEL) {
            HuWinExOpen(winId);
            HuWinMesSet(winId, MESS_MPSYSTEM_TITLE_RETURN);
            HuWinMesWait(winId);
            result = HuWinChoiceGet(winId, 0);
            HuWinExClose(winId);
            if(result != 0) {
                continue;
            } else {
                result = CARD_RESULT_WRONGDEVICE;
                break;
            }
        } else {
            if(result == CARD_RESULT_BUSY) {
                goto cleanup;
            }
            if(result != CARD_RESULT_NOFILE && result != CARD_RESULT_IOERROR) {
                break;
            }
        }
    }
    result = CARD_RESULT_READY;
    for(i=0; i<3; i++) {
        FileselSprUpdate(i);
    }
    cleanup:
    if(srcWinId == HUWIN_NONE) {
        HuWinExClose(winId);
        SLWinIdSet(HUWIN_NONE);
        HuWinKill(winId);
    }
    return result;
}

s32 FileCardMount(s16 slotNo);


s32 FileCheckCardSpace(void)
{
    s32 result = FileCardMount(curSlotNo);
    u32 byteNotUsed, fileNotUsed;
    if(result < 0) {
        return result;
    }
    result = HuCardSectorSizeGet(curSlotNo);
    if(result < 0 && result != SAVE_SECTOR_SIZE) {
        FileMessOut(FILE_MESS_INVALID);
        return CARD_RESULT_FATAL_ERROR;
    }
    result = HuCardFreeSpaceGet(curSlotNo, &byteNotUsed, &fileNotUsed);
    if(fileNotUsed == 0 && byteNotUsed < SAVE_BUF_SIZE) {
        FileMessOut(FILE_MESS_FULL);
        return CARD_RESULT_INSSPACE;
    }
    if(fileNotUsed == 0) {
        FileMessOut(FILE_MESS_NOENT);
        return CARD_RESULT_INSSPACE;
    }
    if(byteNotUsed < SAVE_BUF_SIZE) {
        FileMessOut(FILE_MESS_INSSPACE);
        return CARD_RESULT_INSSPACE;
    }
    return CARD_RESULT_READY;
}

s32 FileCardWarning(HUWINID srcWinId)
{
    s32 result;
    int winId;
    
    if(srcWinId == HUWIN_NONE) {
        winId = HuWinExCreateFrame(-10000, 130, 478, 94, HUWIN_SPEAKER_NONE, 2);
    } else {
        winId = srcWinId;
    }
    SLWinIdSet(winId);
    
    if((result = HuCardMount(curSlotNo)) == CARD_RESULT_READY && 
        (result = HuCardOpen(curSlotNo, SLSaveFileName, &curFileInfo)) == CARD_RESULT_READY) {
        result = CARD_RESULT_READY;
    } else  {
        if((result = FileCheckCardSpace()) == CARD_RESULT_READY) {
            result = CARD_RESULT_READY;
        } else {
            result = FileCardErrorExec(result, winId);
        }
    }
    if(srcWinId == HUWIN_NONE) {
        SLWinIdSet(HUWIN_NONE);
        HuWinKill(winId);
    }
    return result;
}


BOOL FileTestOpen(void)
{
    s32 result = HuCardMount(curSlotNo);
    if(result == CARD_RESULT_READY) {
        result = HuCardOpen(curSlotNo, SLSaveFileName, &curFileInfo);
        if(result == CARD_RESULT_READY) {
            return TRUE;
        }
    }
    return FALSE;
}

s32 FileCardFormat(s16 slotNo);

s32 FileCardErrorExec(s32 result, HUWINID winId)
{
    int choice;
    if(UnMountCnt) {
        HuWinExOpen(winId);
        HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
        HuWinMesSet(winId, MESS_MPSYSTEM_CARD_REMOVE);
        HuWinMesWait(winId);
        HuWinExClose(winId);
        UnMountCnt = 0;
        return CARD_RESULT_NOFILE;
    }
    UnMountCnt = 0;
    HuWinExOpen(winId);
    HuWinMesSet(winId, MESS_MPSYSTEM_NOSAVE);
    HuWinMesWait(winId);
    if(result == CARD_RESULT_BROKEN) {
        repeatBroken:
        choice = FileCardChoice((FILECARD_FLAG_ALL & ~FILECARD_FLAG_MENU), 256);
        if(UnMountCnt && choice == FILECARD_FLAG_FORMAT) {
            HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_REMOVE);
            HuWinMesWait(winId);
            HuWinExClose(winId);
            UnMountCnt = 0;
            return CARD_RESULT_NOFILE;
        } else if(choice == FILESEL_RESULT_CANCEL) {
            HuWinExClose(winId);
            UnMountCnt = 0;
            return FILESEL_RESULT_CANCEL;
        } else if(choice == FILECARD_FLAG_FORMAT) {
            HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_FORMAT);
            HuWinMesWait(winId);
            choice = HuWinChoiceGet(winId, 1);
            if(choice == 0) {
                choice = FileCardFormat(curSlotNo);
                HuWinExClose(winId);
                return CARD_RESULT_NOCARD;
            } else if(UnMountCnt) {
                HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
                HuWinMesSet(winId, MESS_MPSYSTEM_CARD_REMOVE);
                HuWinMesWait(winId);
                HuWinExClose(winId);
                UnMountCnt = 0;
                return CARD_RESULT_NOFILE;
            } else {
                FileMessOut(FILE_MESS_BROKEN);
                HuWinMesSet(winId, MESS_MPSYSTEM_NOSAVE);
                HuWinMesWait(winId);
                goto repeatBroken;
            }
        } else if(choice == FILECARD_FLAG_NOSAVE) {
            HuWinExClose(winId);
            UnMountCnt = 0;
            return CARD_RESULT_BUSY;
        } else {
            HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_INSERT);
            HuWinMesWait(winId);
            while(!(HuPadBtnDown[0] & PAD_BUTTON_A)) {
                HuPrcVSleep();
            }
            HuAudFXPlay(MSM_SE_CMN_02);
            HuWinExClose(winId);
            UnMountCnt = 0;
            return CARD_RESULT_IOERROR;
        }
    } else if(result == CARD_RESULT_INSSPACE) {
        repeatInsSpace:
        choice = FileCardChoice((FILECARD_FLAG_ALL & ~FILECARD_FLAG_FORMAT), 256);
        if(choice == FILESEL_RESULT_CANCEL) {
            HuWinExClose(winId);
            UnMountCnt = 0;
            return FILESEL_RESULT_CANCEL;
        } else if(choice == FILECARD_FLAG_NOSAVE) {
            HuWinExClose(winId);
            UnMountCnt = 0;
            return CARD_RESULT_BUSY; 
        } else if(choice == FILECARD_FLAG_MENU) {
            HuWinMesSet(winId, MESS_MPSYSTEM_GOTO_IPL_CARD_WARN);
            HuWinMesWait(winId);
            choice = HuWinChoiceGet(winId, 1);
            if(choice != 0) {
                HuWinMesSet(winId, MESS_MPSYSTEM_NOSAVE);
                HuWinMesWait(winId);
                UnMountCnt = 0;
                goto repeatInsSpace;
            } else {
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                HuAudSStreamAllFadeOut(1000);
                WipeWait();
                HuSRDisableF = FALSE;
                SR_ExecResetMenu = TRUE;
                while(1) {
                    HuPrcVSleep();
                }
            }
        } else {
            HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_INSERT);
            HuWinMesWait(winId);
            while(!(HuPadBtnDown[0] & PAD_BUTTON_A)) {
                HuPrcVSleep();
            }
            HuAudFXPlay(MSM_SE_CMN_02);
            HuWinExClose(winId);
            UnMountCnt = 0;
            return CARD_RESULT_IOERROR;
        }
    } else {
        choice = FileCardChoice((FILECARD_FLAG_ALL & ~(FILECARD_FLAG_FORMAT|FILECARD_FLAG_MENU)), 256);
        if(choice == FILESEL_RESULT_CANCEL) {
            HuWinExClose(winId);
            UnMountCnt = 0;
            return FILESEL_RESULT_CANCEL;
        } else if(choice == FILECARD_FLAG_NOSAVE) {
            HuWinExClose(winId);
            UnMountCnt = 0;
            return CARD_RESULT_BUSY; 
        } else {
            HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_INSERT);
            HuWinMesWait(winId);
            while(!(HuPadBtnDown[0] & PAD_BUTTON_A)) {
                HuPrcVSleep();
            }
            HuAudFXPlay(MSM_SE_CMN_02);
            HuWinExClose(winId);
            UnMountCnt = 0;
            return CARD_RESULT_IOERROR;
        }
    }
}

s32 FileCardOpen(char *name)
{
    s32 result;
    if(!SLSaveFlagGet()) {
        return CARD_RESULT_READY;
    }
    result = FileCardMount(curSlotNo);
    if(result < 0) {
        return result;
    }
    result = HuCardOpen(curSlotNo, name, &curFileInfo);
    if(result == CARD_RESULT_NOFILE) {
        return CARD_RESULT_NOFILE;
    }
    if(result == CARD_RESULT_WRONGDEVICE) {
        FileMessOut(FILE_MESS_WRONGDEVICE);
        return CARD_RESULT_FATAL_ERROR;
    }
    if(result == CARD_RESULT_FATAL_ERROR) {
        FileMessOut(FILE_MESS_FATALERROR);
        return CARD_RESULT_FATAL_ERROR;
    }
    if(result == CARD_RESULT_NOCARD) {
        FileMessOut(FILE_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    }
    if(result == CARD_RESULT_BROKEN) {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result > 0 && result != SAVE_SECTOR_SIZE) {
            FileMessOut(FILE_MESS_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            UnMountCnt = 0;
            FileMessOut(FILE_MESS_BROKEN);
            return CARD_RESULT_BROKEN;
        }
    }
    return CARD_RESULT_READY;
}

s32 FileCardRead(s32 length, void *addr)
{
    s32 result;
    if(!SLSaveFlagGet()) {
        return CARD_RESULT_READY;
    }
    SLSerialNoGet();
    result = HuCardRead(&curFileInfo, addr, length, 0);
    if(result == CARD_RESULT_NOCARD) {
        FileMessOut(FILE_MESS_NOCARD);
    } else if(result < 0) {
        FileMessOut(FILE_MESS_FATALERROR);
    }
    return result;
}

s32 FileCardClose(void)
{
    s32 result;
    if(!SLSaveFlagGet()) {
        return CARD_RESULT_READY;
    }
    result = HuCardClose(&curFileInfo);
    return result;
}

s32 FileCardMount(s16 slotNo)
{
    s32 result;
    result = HuCardMount(curSlotNo);
    if(result == CARD_RESULT_WRONGDEVICE) {
        FileMessOut(FILE_MESS_WRONGDEVICE);
        return result;
    }
    if(result == CARD_RESULT_FATAL_ERROR) {
        FileMessOut(FILE_MESS_FATALERROR);
        return CARD_RESULT_FATAL_ERROR;
    }
    if(result == CARD_RESULT_NOCARD) {
        FileMessOut(FILE_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    }
    if(result == CARD_RESULT_BROKEN) {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result > 0 && result != SAVE_SECTOR_SIZE) {
            FileMessOut(FILE_MESS_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            UnMountCnt = 0;
            FileMessOut(FILE_MESS_BROKEN);
            return CARD_RESULT_BROKEN;
        }
    }
    result = HuCardSectorSizeGet(curSlotNo);
    if(result < 0) {
        FileMessOut(FILE_MESS_FATALERROR);
        return result;
    } else {
        if(result != SAVE_SECTOR_SIZE) {
            FileMessOut(FILE_MESS_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        }
        return CARD_RESULT_READY;
    }
    (void)result;
}

s32 FileCardFormat(s16 slotNo)
{
    s16 result;
    HUWINID winId;
    OSTime time;
    if(UnMountCnt & (1 << curSlotNo)) {
        FileMessOut(FILE_MESS_FORMAT_UNMOUNT);
        UnMountCnt = 0;
        return CARD_RESULT_READY;
    }
    winId = FileCardMesOpen(MESS_MPSYSTEM_CARD_FORMAT_WARN, MESS_MPSYSTEM_CARD_SLOTA+slotNo, FILECARD_MESSID_NONE, 80);
    HuPrcSleep(30);
    if(UnMountCnt & (1 << curSlotNo)) {
        FileCardMesClose(winId);
        FileMessOut(FILE_MESS_FORMAT_UNMOUNT);
        UnMountCnt = 0;
        return CARD_RESULT_READY;
    }
    result = HuCardFormat(curSlotNo);
    SLSerialNo[curSlotNo] = 0;
    if(result < 0) {
        FileCardMesClose(winId);
    }
    if(result == CARD_RESULT_FATAL_ERROR) {
        FileMessOut(FILE_MESS_FORMATERROR);
        FileMessOut(FILE_MESS_FATALERROR);
        return CARD_RESULT_FATAL_ERROR;
    }
    if(result == CARD_RESULT_NOCARD) {
        FileMessOut(FILE_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    }
    if(result == CARD_RESULT_WRONGDEVICE) {
        FileMessOut(FILE_MESS_WRONGDEVICE);
        return result;
    }
    SLSerialNoGet();
    FileCardMesClose(winId);
    SLCurBoxNoSet(0);
    time = OSGetTime();
    SLSaveDataMake(FALSE, &time);
    SLCheckSumBoxAllSet();
    return result;
}

s32 FileCardLoad(void)
{
    s32 result = FileCardOpen(SLSaveFileName);
    if(result >= 0) {
        result = FileCardRead(SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
        FileCardClose();
        if(result >= 0) {
            u16 *checksum = (u16 *)&saveBuf[curSlotNo][SAVE_BOX_SIZE];
            u16 checksumVal = SLCheckSumGet(0, SAVE_BOX_SIZE);
            *checksum == checksumVal;
        }
    }
    return result;
}

s32 FileCardCopy(char *name, s32 length, void *addr)
{
    s32 result;
    int winId;
    int statusWinId;
    void *eraseBuf;
    
    u32 byteNotUsed;
    u32 fileNotUsed;
    if(!SLSaveFlagGet()) {
        return CARD_RESULT_READY;
    }
    SLCheckSumBoxAllSet();
    SLSaveBackup();
    result = FileCardMount(curSlotNo);
    if(result < 0) {
        return result;
    }
    result = HuCardSectorSizeGet(curSlotNo);
    if(result < 0 && result != SAVE_SECTOR_SIZE) {
        FileMessOut(FILE_MESS_INVALID);
        return CARD_RESULT_FATAL_ERROR;
    }
    result = HuCardFreeSpaceGet(curSlotNo, &byteNotUsed, &fileNotUsed);
    if(fileNotUsed == 0 && length > byteNotUsed) {
        FileMessOut(FILE_MESS_FULL);
        return CARD_RESULT_INSSPACE;
    }
    if(fileNotUsed == 0) {
        FileMessOut(FILE_MESS_NOENT);
        return CARD_RESULT_INSSPACE;
    }
    if(length > byteNotUsed) {
        FileMessOut(FILE_MESS_INSSPACE);
        return CARD_RESULT_INSSPACE;
    }
    winId = FileCardMesOpen(MESS_MPSYSTEM_CARD_CREATEFILE, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, FILECARD_MESSID_NONE, 130);
    statusWinId = FileStatusMesOpen(MESS_MPSYSTEM_MES_SAVE_SAVE, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, FILECARD_MESSID_NONE, 80);
    result = HuCardCreate(curSlotNo, name, length, &curFileInfo);
    if(result < 0) {
        FileCardMesClose(winId);
        FileCardMesClose(statusWinId);
    }
    if(result == CARD_RESULT_NOCARD) {
        FileMessOut(FILE_MESS_NOCARD);
        return result;
    }
    if(result == CARD_RESULT_BROKEN) {
        FileMessOut(FILE_MESS_BROKEN);
        return result;
    }
    if(result < 0) {
        FileMessOut(FILE_MESS_FATALERROR);
        return result;
    }
    SLSerialNoGet();
    eraseBuf = HuMemDirectMalloc(HEAP_HEAP, length);
    memset(eraseBuf, length, 0);
    memcpy(eraseBuf, SLEraseStr, 6);
    result = HuCardWrite(&curFileInfo, eraseBuf, length, 0);
    if(result == CARD_RESULT_READY) {
        result = HuCardWrite(&curFileInfo, addr, length, 0);
    }
    HuMemDirectFree(eraseBuf);
    if(result < 0) {
        FileCardMesClose(winId);
        FileCardMesClose(statusWinId);
    }
    if(result == CARD_RESULT_NOCARD) {
        FileMessOut(FILE_MESS_NOCARD);
        return result;
    }
    if(result == CARD_RESULT_BROKEN) {
        FileMessOut(FILE_MESS_BROKEN);
        return result;
    }
    if(result < 0) {
        FileMessOut(FILE_MESS_FATALERROR);
        return result;
    }
    result = SLStatSet(TRUE);
    FileCardMesClose(winId);
    FileCardMesClose(statusWinId);
    if(result < 0) {
        return result;
    }
    return CARD_RESULT_READY;
}

s32 FileClear(HUWINID srcWinId)
{
    s32 result;
    int winId;
    if(srcWinId == HUWIN_NONE) {
        winId = HuWinExCreateFrame(-10000, 130, 478, 94, HUWIN_SPEAKER_NONE, 2);
    } else {
        winId = srcWinId;
    }
    SLWinIdSet(winId);
    repeatCopy:
    result = FileCardCopy(SLSaveFileName, SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
    if(result == CARD_RESULT_READY) {
        result = CARD_RESULT_READY;
    } else {
        result = FileCardErrorExec(result, winId);
        if(result == CARD_RESULT_NOCARD) {
            goto repeatCopy;
        }
    }
    if(srcWinId == HUWIN_NONE) {
        SLWinIdSet(HUWIN_NONE);
        HuWinKill(winId);
    }
    return result;
}

s32 FileCardWrite(s32 length, void *addr)
{
    s32 result;
    int winId;
    if(!SLSaveFlagGet()) {
        return CARD_RESULT_READY;
    }
    if(fileMesId != FILECARD_MESSID_NONE) {
        winId = FileStatusMesOpen(fileMesId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, FILECARD_MESSID_NONE, 80);
    }
    HuPrcSleep(60);
    SLSerialNoGet();
    result = HuCardWrite(&curFileInfo, addr, length, 0);
    if(fileMesId != FILECARD_MESSID_NONE) {
        FileStatusMesClose(winId);
    }
    if(result == 0) {
        result = SLStatSet(TRUE);
    }
    return result;
}

s32 FileSave(void)
{
    s32 result;
    SLCheckSumBoxAllSet();
    SLSaveBackup();
    result = FileCardOpen(SLSaveFileName);
    if(result == CARD_RESULT_NOFILE) {
        if(!SLSerialNoCheck()) {
            FileMessOut(FILE_MESS_SERIAL_INVALID);
        } else {
            result = FileCardCopy(SLSaveFileName, SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
            if(result >= 0) {
                SLSerialNoGet();
            }
        }
    } else if(result >= 0) {
        if(!SLSerialNoCheck()) {
            FileMessOut(FILE_MESS_SERIAL_INVALID);
        } else {
            int winId = FileCardMesOpen(MESS_MPSYSTEM_MES_CARD_WRITE, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, FILECARD_MESSID_NONE, 80);
            result = FileCardWrite(SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
            FileCardMesClose(winId);
            if(result == CARD_RESULT_NOCARD) {
                FileMessOut(FILE_MESS_NOCARD);
            } else if(result == CARD_RESULT_WRONGDEVICE) {
                FileMessOut(FILE_MESS_WRONGDEVICE);
            } else if(result == CARD_RESULT_BROKEN) {
                result = HuCardSectorSizeGet(curSlotNo);
                if(result > 0 && result != SAVE_SECTOR_SIZE) {
                    FileMessOut(FILE_MESS_INVALID);
                } else {
                    FileMessOut(FILE_MESS_BROKEN);
                    return CARD_RESULT_BROKEN;
                }
            } else if(result < 0) {
                FileMessOut(FILE_MESS_FATALERROR);
            }
        }
    }
    FileCardClose();
    return result;
}

s32 FileSaveMesOpen(HUWINID srcWinId, u32 mesId)
{
    s32 result;
    int winId;
    if(srcWinId == HUWIN_NONE) {
        winId = HuWinExCreateFrame(-10000, 130, 478, 94, HUWIN_SPEAKER_NONE, 2);
    } else {
        winId = srcWinId;
    }
    SLWinIdSet(winId);
    fileMesId = mesId;
    repeatSave:
    result = FileSave();
    if(SLSerialNoCheck()) {
        if(result == CARD_RESULT_READY) {
            result = CARD_RESULT_READY;
        } else {
            goto cardError;
        }
    } else if(result == CARD_RESULT_READY) {
        result = CARD_RESULT_IOERROR;
    } else {
        cardError:
        result = FileCardErrorExec(result, winId);
        if(result == CARD_RESULT_NOCARD) {
            goto repeatSave;
        }
    }
    if(srcWinId == HUWIN_NONE) {
        SLWinIdSet(HUWIN_NONE);
        HuWinKill(winId);
    }
    fileMesId = FILECARD_MESSID_NONE;
    return result;
}

HUWINID FileStatusMesOpen(u32 mesId, u32 insertMes1, u32 insertMes2, s16 posY)
{
    HUWINID winId;
    HuVec2F size;
    if(insertMes1 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMes1, 0);
    }
    if(insertMes2 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMes2, 1);
    }
    HuWinMesMaxSizeGet(1, &size, mesId);
    winId = HuWinExCreateFrame(-10000, posY, size.x, size.y, HUWIN_SPEAKER_NONE, 2);
    HuWinExOpen(winId);
    if(insertMes1 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMes1, 0);
    }
    if(insertMes2 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMes2, 1);
    }
    HuWinMesSet(winId, mesId);
    HuWinMesWait(winId);
    return winId;
}

void FileStatusMesClose(HUWINID winId)
{
    if(winId >= 0) {
        HuWinExClose(winId);
        HuWinExKill(winId);
    }
}

HUWINID FileCardMesOpen(u32 mesId, u32 insertMes1, u32 insertMes2, s16 posY)
{
    HUWINID winId;
    HuVec2F size;
    if(SLWinId == HUWIN_NONE) {
        HuWinInit(1);
    }
    if(insertMes1 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMes1, 0);
    }
    if(insertMes2 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMes2, 1);
    }
    HuWinMesMaxSizeGet(1, &size, mesId);
    if(SLWinId == HUWIN_NONE) {
        winId = HuWinExCreateFrame(-10000, posY, size.x, size.y, HUWIN_SPEAKER_NONE, 2);
    } else {
        winId = SLWinId;
    }
    
    HuWinExOpen(winId);
    if(insertMes1 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMes1, 0);
    }
    if(insertMes2 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMes2, 1);
    }
    HuWinMesSet(winId, mesId);
    HuWinMesWait(winId);
    return winId;
}

void FileCardMesClose(HUWINID winId)
{
    if(SLWinId != winId && winId >= 0) {
        HuWinExClose(winId);
        HuWinExKill(winId);
    }
}


//#define FILE_MESS_NOCARD 0
//#define FILE_MESS_FATALERROR 1
//#define FILE_MESS_NOENT 2
//#define FILE_MESS_INSSPACE 3
//#define FILE_MESS_FULL 4
//#define FILE_MESS_BROKEN 5
//#define FILE_MESS_FORMATERROR 6
//#define FILE_MESS_WRONGDEVICE 7
//#define FILE_MESS_INVALID 8
//#define FILE_MESS_SERIAL_INVALID 9
//#define FILE_MESS_NOSAVE_CHOICE 10
//#define FILE_MESS_REINSERT 11
//#define FILE_MESS_FORMAT_UNMOUNT 12

s16 FileMessOut(s16 messNo)
{
    HUWINID winId;
    u32 mesId;
    s16 choice = -1;
    u32 insertMes = 0;
    BOOL choiceF = FALSE;
    
    HUWIN *winP;
    
    if(SLWinId == HUWIN_NONE) {
        HuWinInit(1);
    }
    switch(messNo) {
        case FILE_MESS_NOCARD:
            mesId = MESS_MPSYSTEM_CARD_NOCARD;
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            UnMountCnt = 0;
            break;
        
        case FILE_MESS_FATALERROR:
            mesId = MESS_MPSYSTEM_CARD_FATAL_ERROR;
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            break;
       
        case FILE_MESS_NOENT:
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            mesId = MESS_MPSYSTEM_CARD_NOENT;
            break;
       
        case FILE_MESS_INSSPACE:
            mesId = MESS_MPSYSTEM_CARD_INSSPACE;
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            break;
       
        case FILE_MESS_FULL:
            mesId = MESS_MPSYSTEM_CARD_FULL;
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            break;
       
        case FILE_MESS_BROKEN:
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            mesId = MESS_MPSYSTEM_CARD_FORMAT_CHOICE;
            break;
       
        case FILE_MESS_FORMATERROR:
            mesId = MESS_MPSYSTEM_CARD_FORMAT_ERROR;
            break;
       
        case FILE_MESS_WRONGDEVICE:
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            mesId = MESS_MPSYSTEM_CARD_WRONGDEVICE;
            break;
       
        case FILE_MESS_INVALID:
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            mesId = MESS_MPSYSTEM_CARD_INVALID;
            break;
       
        case FILE_MESS_SERIAL_INVALID:
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            mesId = MESS_MPSYSTEM_CARD_SERIAL_INVALID;
            break;
       
        case FILE_MESS_NOSAVE_CHOICE:
            mesId = MESS_MPSYSTEM_NOSAVE_CHOICE;
            choiceF = TRUE;
            break;
        
        case FILE_MESS_REINSERT:
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            mesId = MESS_MPSYSTEM_CARD_REINSERT;
            break;
       
        case FILE_MESS_FORMAT_UNMOUNT:
            HuWinInsertMesSizeGet(MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            insertMes = MESS_MPSYSTEM_CARD_SLOTA+curSlotNo;
            mesId = MESS_MPSYSTEM_CARD_FORMAT_UNMOUNT;
            break;
    }
    if(SLWinId == HUWIN_NONE) {
        HuVec2F size;
        size.x = 478;
        size.y = 94;
        winId = HuWinExCreateFrame(-10000, 130, size.x, size.y, HUWIN_SPEAKER_NONE, 2);
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
    HuWinMesSet(winId, mesId);
    HuWinMesWait(winId);
    if(choiceF) {
        choice = HuWinChoiceGet(winId, 1);
        if(messNo == FILE_MESS_BROKEN && choice == 0) {
            HuWinInsertMesSet(winId, MESS_MPSYSTEM_CARD_SLOTA+curSlotNo, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_FORMAT);
            HuWinMesWait(winId);
            choice = HuWinChoiceGet(winId, 1);
        }
    }
    if(messNo == FILE_MESS_REINSERT) {
        while(!(HuPadBtnDown[0] & PAD_BUTTON_A)) {
            HuPrcVSleep();
        }
        HuAudFXPlay(MSM_SE_CMN_02);
    }
    if(SLWinId == HUWIN_NONE) {
        HuWinExClose(winId);
        HuWinExKill(winId);
    }
    return choice;
}

int FileCardChoice(s16 flag, float posY)
{
    s16 i;
    s16 choice = 0;
    HUSPRGRPID grpId;
    s16 choiceNum;
    s16 j;
    s16 diff;
    s16 prevChoice;
    s16 cancelF = FALSE;
    HUSPRID sprId;
    ANIMDATA *animP;
    float scale;
    float space;
    
    s16 choiceNo[FILECARD_CHOICE_MAX];
    s16 choiceFlag[FILECARD_CHOICE_MAX];
    
    posY += 16;
    
    for(i=choiceNum=0; i<FILECARD_CHOICE_MAX; i++) {
        if(flag & (1 << i)) {
            choiceNo[choiceNum] = i;
            choiceFlag[choiceNum] = (1 << i);
            choiceNum++;
        }
    }
    grpId = HuSprGrpCreate(choiceNum);
    choiceGrpId = grpId;
    animP = HuSprAnimDataRead(WIN_ANM_cardChoice);
    space = 120;
    for(i=0; i<choiceNum; i++) {
        sprId = HuSprCreate(animP, 10, 0);
        HuSprGrpMemberSet(grpId, i, sprId);
        HuSprPosSet(grpId, i, ((576-(choiceNum*space))/2)+(i*space)+(space/2), posY);
        HuSprBankSet(grpId, i, choiceNo[i]*2);
        HuSprScaleSet(grpId, i, 0.7f, 0.7f);
        HuSprAttrSet(grpId, i, HUSPR_ATTR_LINEAR);
        HuSprPriSet(grpId, choice, 100);
    }
    for(i=1; i<=10; i++) {
        scale = 0.7*(i/10.0);
        for(j=0; j<choiceNum; j++) {
            HuSprScaleSet(grpId, j, scale, scale);
        }
        HuPrcVSleep();
    }
    for(i=1; i<=5; i++) {
        scale = 0.7+(0.3*(i/5.0));
        HuSprScaleSet(grpId, choice, scale, scale);
        HuPrcVSleep();
    }
    HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARDCHOICE_1+choiceNo[choice]);
    while(1) {
        diff = 0;
        if(HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            diff = -1;
        } else if(HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            diff = 1;
        }
        if(diff) {
            prevChoice = choice;
            choice += diff;
            if(choice < 0) {
                choice = choiceNum-1;
            } else if(choice >= choiceNum) {
                choice = 0;
            }
            HuAudFXPlay(MSM_SE_CMN_01);
            for(i=1; i<=5; i++) {
                if(i == 3) {
                    HuSprPriSet(grpId, choice, 95);
                    HuSprPriSet(grpId, prevChoice, 100);
                    HuWinMesSet(infoWinId, MESS_MPSYSTEM_CARDCHOICE_1+choiceNo[choice]);
                }
                scale = 0.7+(0.3*(i/5.0));
                HuSprScaleSet(grpId, choice, scale, scale);
                scale = 1.0-(0.3*(i/5.0));
                HuSprScaleSet(grpId, prevChoice, scale, scale);
                HuPrcVSleep();
            }
        }
        if(HuPadBtnDown[0] & PAD_BUTTON_A) {
            HuAudFXPlay(MSM_SE_CMN_03);
            HuSprBankSet(grpId, choice, (choiceNo[choice]*2)+1);
            break;
        }
        if((flag & FILECARD_FLAG_CANCEL) && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
            HuAudFXPlay(MSM_SE_CMN_04);
            cancelF = TRUE;
            break;
        }
        HuPrcVSleep();
    }
    HuWinHomeClear(infoWinId);
    HuPrcSleep(5);
    HuSprGrpKill(grpId);
    if(cancelF) {
        return FILESEL_RESULT_CANCEL;
    }
    return choiceFlag[choice];
}