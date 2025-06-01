#ifndef _GAME_SAVELOAD_H
#define _GAME_SAVELOAD_H

#include "dolphin.h"
#include "game/gamework.h"
#include "game/window.h"

#define SAVE_ROUNDUP(value, x) (((value) + (x-1)) & ~(x-1))

#define SAVE_SECTOR_SIZE 8192

#define SAVE_COMMENT_OFS 0
#define SAVE_BANNER_OFS (SAVE_COMMENT_OFS+CARD_COMMENT_SIZE)
#define SAVE_COMMENT_DATE_OFS (CARD_COMMENT_SIZE/2)

#define SAVE_BANNER_TLUT_OFS (SAVE_BANNER_OFS+(CARD_BANNER_WIDTH*CARD_BANNER_HEIGHT))
#define SAVE_ICON_OFS (SAVE_BANNER_TLUT_OFS+512)
#define SAVE_ICON_TLUT_OFS (SAVE_ICON_OFS+(CARD_ICON_WIDTH*CARD_ICON_HEIGHT*4))
#define SAVE_ICONBANNER_CHECKSUM_OFS (SAVE_ICON_TLUT_OFS+512)
#define SAVE_ICONBANNER_SIZE SAVE_ICONBANNER_CHECKSUM_OFS

#define SAVE_BOX_SIZE (SAVE_ROUNDUP((sizeof(GWCOMMON)+(2*sizeof(GWSYSTEM))+(2*GW_PLAYER_MAX*sizeof(GWPLAYER))+sizeof(GWSDCOMMON)), 4)+2)

#define SAVE_BOX_DATA_OFS (SAVE_ICONBANNER_CHECKSUM_OFS+2)
#define SAVE_BOX_OFS(boxNo) (SAVE_BOX_DATA_OFS+(SAVE_BOX_SIZE*(boxNo)))

#define SAVE_BOX_MAX 3
#define SAVE_BOXNO_BACKUP SAVE_BOX_MAX

#define SAVE_BOXBACKUP_DATA_OFS SAVE_ROUNDUP(SAVE_BOX_OFS(SAVE_BOX_MAX), SAVE_SECTOR_SIZE)
#define SAVE_BOXBACKUP_OFS(boxNo) (SAVE_BOXBACKUP_DATA_OFS+(SAVE_BOX_SIZE*(boxNo)))

#define SAVE_BUF_SIZE SAVE_ROUNDUP(SAVE_BOXBACKUP_OFS(SAVE_BOX_MAX), SAVE_SECTOR_SIZE)


extern u8 ATTRIBUTE_ALIGN(32) saveBuf[2][SAVE_BUF_SIZE];
extern u64 SLSerialNo[2];
extern BOOL SaveEnableF;
extern char SLSaveFileName[];
extern char SLEraseStr[];
extern HUWINID SLWinId;
extern CARDFileInfo curFileInfo;
extern BOOL saveExecF;
extern u8 curBoxNo;
extern s16 curSlotNo;

void SLCurSlotNoSet(s16 slotNo);
s16 SLCurSlotNoGet(void);
s16 SLCurBoxNoGet(void);
void SLSaveFlagSet(BOOL saveFlag);
BOOL SLSaveFlagGet(void);
void SLSaveEmptyMark(s16 slotNo, s16 boxNo);
void SLSaveDataSlotMake(s16 slotNo, BOOL eraseF, OSTime *saveTime);
void SLSaveDataMake(BOOL eraseF, OSTime *saveTime);
void SLSaveDataInfoSet(s16 slotNo, OSTime *saveTime);
void SLCommonSet(void);
void SLCommonSaveCopy(GWCOMMON *commonP, s16 slotNo, s16 boxNo);
void SLBoardSave(void);
void SLBoardSaveStory(void);

s32 SLSave(void);
s32 SLLoad(void);
void SLCommonLoad(void);
void SLCommonLoadCopy(GWCOMMON *commonP, s16 slotNo, s16 boxNo);
void SLBoardLoad(void);
void SLBoardLoadStory(void);
void SLSdLoad(s16 slotNo, s16 boxNo, GWSDCOMMON *sdCommonP);
void SLSdSave(s16 slotNo, s16 boxNo, GWSDCOMMON *sdCommonP);

s32 SLSerialNoGet(void);
BOOL SLSerialNoCheck(void);
BOOL SLCheckSumBoxSlotCheck(s16 slotNo, s16 boxNo);
BOOL SLCheckSumCheck(void);

u16 SLCheckSumSlotGet(s16 slotNo, u32 begin, u32 size);
u16 SLCheckSumGet(u32 begin, u32 size);
void SLCheckSumBoxSet(void);
void SLCheckSumBoxAllSet(void);
void SLSaveBackup(void);
void SLBoxBackupSlotLoad(s16 slotNo, s16 boxNo);
void SLBoxBackupLoad(s16 boxNo);

u32 SLBoxDataOffsetGet(s16 boxNo);
s32 SLStatSet(BOOL errorOutF);
s32 SLCardMount(s16 slotNo);
s32 SLFormat(s16 slotNo);
void SLWinIdSet(HUWINID winId);
s16 SLMessOut(s16 messId);

void SLSaveBoardTurnExec(void);
void SLSaveBoardEndExec(void);
void SLSaveStoryExec(void);
void SLSaveStaffExec(void);
void SLSaveModeExec(s16 sdModeF);
s32 SLSaveSdBExec(void);

#endif
