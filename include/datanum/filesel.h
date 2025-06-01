#ifndef DATANUM_FILESEL_H
#define DATANUM_FILESEL_H

#include "datadir_enum.h"

#define FILESEL_CHAR(charName) \
    FILESEL_HSF_##charName, \
    FILESEL_HSF_##charName##Idle, \
    FILESEL_HSF_##charName##Run

enum {
    FILESEL_DUMMY_FILE = DATANUM(DATA_filesel, 0)-1,
    FILESEL_HSF_touei,
    FILESEL_HSF_stage,
    FILESEL_ANM_keyboardMain,
    FILESEL_ANM_keyboardNumSpc,
    FILESEL_ANM_fileNum,
    FILESEL_ANM_fileBack,
    FILESEL_ANM_fileCopy,
    FILESEL_ANM_ring,
    FILESEL_ANM_ringEllipse,
    FILESEL_ANM_back,
    FILESEL_ANM_num,
    FILESEL_ANM_circleIn,
    FILESEL_ANM_fade,
    FILESEL_CHAR(mario),
    FILESEL_CHAR(luigi),
    FILESEL_CHAR(peach),
    FILESEL_CHAR(yoshi),
    FILESEL_CHAR(wario),
    FILESEL_CHAR(daisy),
    FILESEL_CHAR(waluigi),
    FILESEL_CHAR(kinopio),
    FILESEL_CHAR(teresa),
    FILESEL_CHAR(minikoopa),
    FILESEL_HSF_star,
    FILESEL_HSF_starMot,
};

#undef FILESEL_CHAR

#endif