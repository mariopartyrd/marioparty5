#ifndef _FILESELDLL_H
#define _FILESELDLL_H

#include "game/hu3d.h"
#include "game/window.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/card.h"
#include "game/saveload.h"
#include "game/process.h"

#include "datanum/filesel.h"

#define FILESEL_RESULT_CANCEL -1234
#define FILESEL_NULL_MES 0x450003

typedef struct FileselBox_s {
    BOOL validF;
    HU3DMODELID toueiMdlId;
    s16 charNo;
    HUSPRGRPID grpId;
    HUWINID winId;
    OSCalendarTime time;
    char name[17];
} FILESEL_BOX;

extern HU3DMODELID charToueiMdlId[];
extern HU3DMODELID ToueiMdlId[SAVE_BOX_MAX];
extern FILESEL_BOX fileselBox[SAVE_BOX_MAX];
extern HUWINID infoWinId;
extern HUWINID helpWinId;

void FileselSprUpdate(s16 boxNo);

void FileCommonInit(void);
s32 FileBoxInit(HUWINID srcWinId);

BOOL FileTestOpen(void);
s32 FileClear(HUWINID srcWinId);
s32 FileSaveMesOpen(HUWINID srcWinId, u32 mesId);
HUWINID FileCardMesOpen(u32 mesId, u32 insertMes1, u32 insertMes2, s16 posY);
void FileCardMesClose(HUWINID winId);
HUWINID FileStatusMesOpen(u32 mesId, u32 insertMes1, u32 insertMes2, s16 posY);
void FileStatusMesClose(HUWINID winId);

void NameEnterInit(void);
s32 NameEnterMain(s16 boxNo);
s16 NameEnterClose(HUWINID winId);

#endif