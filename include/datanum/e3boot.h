#ifndef DATANUM_E3BOOT_H
#define DATANUM_E3BOOT_H

#include "datadir_enum.h"

#define E3BOOT_PLAYER_FILE(charName) \
    E3BOOT_HSF_##charName, \
    E3BOOT_HSF_##charName##Mot1, \
    E3BOOT_HSF_##charName##Mot2, \
    E3BOOT_HSF_##charName##Mot3

enum {
    E3BOOT_DUMMY_FILE = DATANUM(DATA_e3boot, 0)-1,
    E3BOOT_ANM_hudson,
    E3BOOT_ANM_back,
    E3BOOT_ANM_logo,
    E3BOOT_ANM_copyright,
    E3BOOT_ANM_pressstart,
    E3BOOT_ANM_btnmg,
    E3BOOT_ANM_btnmovie,
    E3BOOT_ANM_fade,
    E3BOOT_HSF_starChorl,
    E3BOOT_HSF_starNerl,
    E3BOOT_HSF_starTerl,
    E3BOOT_HSF_starNirl,
    E3BOOT_HSF_starHarl,
    E3BOOT_HSF_starMarl,
    E3BOOT_HSF_starDarl,
    E3BOOT_PLAYER_FILE(mario),
    E3BOOT_PLAYER_FILE(luigi),
    E3BOOT_PLAYER_FILE(peach),
    E3BOOT_PLAYER_FILE(yoshi),
    E3BOOT_PLAYER_FILE(wario),
    E3BOOT_PLAYER_FILE(daisy),
    E3BOOT_PLAYER_FILE(waluigi),
    E3BOOT_PLAYER_FILE(kinopio)
};

#undef E3BOOT_PLAYER_FILE

#endif