#ifndef _GAME_FLAG_H
#define _GAME_FLAG_H

#include "dolphin/types.h"

#define FLAGNUM(group, index) ((group)|(index))

enum {
    FLAG_GROUP_SAVE = (0 << 16),
    FLAG_GROUP_COMMON = (1 << 16),
    FLAG_GROUP_BOARD = (2 << 16),
    FLAG_GROUP_SYSTEM = (3 << 16),
};

enum {
    FLAG_STORY_MAP1_COMPLETE = FLAGNUM(FLAG_GROUP_SAVE, 2),
    FLAG_STORY_MAP2_COMPLETE = FLAGNUM(FLAG_GROUP_SAVE, 3),
    FLAG_STORY_MAP3_COMPLETE = FLAGNUM(FLAG_GROUP_SAVE, 4),
    FLAG_STORY_MAP4_COMPLETE = FLAGNUM(FLAG_GROUP_SAVE, 5),
    FLAG_STORY_MAP5_COMPLETE = FLAGNUM(FLAG_GROUP_SAVE, 6),
    FLAG_STORY_MAP6_COMPLETE = FLAGNUM(FLAG_GROUP_SAVE, 7),
    FLAG_BOARD_LAST5 = FLAGNUM(FLAG_GROUP_SAVE, 8),
    FLAG_MGINST_OFF = FLAGNUM(FLAG_GROUP_SAVE, 11),
};

enum {
    FLAG_BOARD_SAVEINIT = FLAGNUM(FLAG_GROUP_COMMON, 0),
    FLAG_BOARD_OPENING = FLAGNUM(FLAG_GROUP_COMMON, 1),
    FLAG_BOARD_MG = FLAGNUM(FLAG_GROUP_COMMON, 2),
    FLAG_BOARD_MGKUPA = FLAGNUM(FLAG_GROUP_COMMON, 3),
    FLAG_BOARD_MGVS = FLAGNUM(FLAG_GROUP_COMMON, 4),
    FLAG_BOARD_MGKETTOU = FLAGNUM(FLAG_GROUP_COMMON, 5),
    FLAG_BOARD_MGDONKEY = FLAGNUM(FLAG_GROUP_COMMON, 6),
    FLAG_MG_PAUSE_OFF = FLAGNUM(FLAG_GROUP_COMMON, 11),
    FLAG_BOARD_TUTORIAL = FLAGNUM(FLAG_GROUP_COMMON, 14),
    FLAG_MG_PRACTICE = FLAGNUM(FLAG_GROUP_COMMON, 15),
    FLAG_MG_CIRCUIT = FLAGNUM(FLAG_GROUP_COMMON, 16),
    FLAG_BOARD_TAUNTOFF = FLAGNUM(FLAG_GROUP_COMMON, 16),
    FLAG_BOARD_INIT = FLAGNUM(FLAG_GROUP_COMMON, 20),
    FLAG_BOARD_PAUSE = FLAGNUM(FLAG_GROUP_COMMON, 22),
    FLAG_BOARD_CAMERAMOT = FLAGNUM(FLAG_GROUP_COMMON, 25),
    FLAG_BOARD_STARMUS_RESET = FLAGNUM(FLAG_GROUP_COMMON, 27),
    FLAG_BOARD_STORYEND = FLAGNUM(FLAG_GROUP_COMMON, 28),
    FLAG_BOARD_PAUSE_ENABLE = FLAGNUM(FLAG_GROUP_COMMON, 30),
    FLAG_BOARD_EXIT = FLAGNUM(FLAG_GROUP_COMMON, 32),
    FLAG_BOARD_TURN_NOSTART = FLAGNUM(FLAG_GROUP_COMMON, 33),
    FLAG_BOARD_STAR_RESET = FLAGNUM(FLAG_GROUP_COMMON, 34),
};

enum {
    FLAG_BOARD_NOMG = FLAGNUM(FLAG_GROUP_BOARD, 0),
    FLAG_BOARD_DEBUG = FLAGNUM(FLAG_GROUP_BOARD, 1),
    FLAG_BOARD_SAI_SHORT = FLAGNUM(FLAG_GROUP_BOARD, 4),
};

enum {
    FLAG_DECA_INST = FLAGNUM(FLAG_GROUP_SYSTEM, 2)
};

s32 _CheckFlag(u32 flag);
void _SetFlag(u32 flag);
void _ClearFlag(u32 flag);
void _InitFlag(void);

#endif