#ifndef DATANUM_GUIDE_H
#define DATANUM_GUIDE_H

#include "datadir_enum.h"

#define GUIDE_FILE(charName) \
    GUIDE_HSF_##charName, \
    GUIDE_HSF_##charName##Mot1, \
    GUIDE_HSF_##charName##Mot2, \
    GUIDE_HSF_##charName##Mot3
    
enum {
    GUIDE_DUMMY_FILE = DATANUM(DATA_guide, 0)-1,
    GUIDE_FILE(chorl),
    GUIDE_FILE(nerl),
    GUIDE_FILE(terl),
    GUIDE_FILE(nirl),
    GUIDE_FILE(harl),
    GUIDE_FILE(marl),
    GUIDE_FILE(darl),
};

#undef GUIDE_FILE

#endif