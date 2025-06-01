#ifndef _FILESELDLL_H
#define _FILESELDLL_H

#include "game/hu3d.h"
#include "game/window.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/card.h"
#include "game/saveload.h"

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
extern HU3DMODELID toueiMdlId[SAVE_BOX_MAX];
extern FILESEL_BOX fileselBox[SAVE_BOX_MAX];
extern HUWINID infoWinId;
extern HUWINID helpWinId;

s32 FileBoxInit(s16 boxNo);
void FileCommonInit(void);
BOOL FileTestOpen(void);
int FileClear(s16 boxNo);
int FileSaveMesOpen(s16 boxNo, u32 mesId);

void NameEnterInit(void);
int NameEnterMain(s16 boxNo);

#endif