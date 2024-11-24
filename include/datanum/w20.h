#ifndef DATANUM_WIN_H
#define DATANUM_WIN_H

#include "datadir_enum.h"

#define W20_MCCAR(charName) \
    W20_HSF_mcCar##charName, \
    W20_HSF_mcCar##charName##Idle, \
    W20_HSF_mcCar##charName##Idle2, \
    W20_HSF_mcCar##charName##Jump, \
    W20_HSF_mcCar##charName##Win, \
    W20_HSF_mcCar##charName##Lose, \
    W20_HSF_mcCar##charName##Look, \
    W20_HSF_mcCar##charName##LookUp

enum {
    W20_BIN_masuData = DATANUM(DATA_w20, 0),
    W20_HSF_map,
    W20_HSF_mapCol,
    W20_HSF_hikosen,
    W20_HSF_fusen1,
    W20_HSF_fusen2,
    W20_HSF_fusen3,
    W20_HSF_fusen4,
    W20_HSF_fusen5,
    W20_MCCAR(Mario),
    W20_MCCAR(Luigi),
    W20_MCCAR(Peach),
    W20_MCCAR(Yoshi),
    W20_MCCAR(Wario),
    W20_MCCAR(Daisy),
    W20_MCCAR(Waluigi),
    W20_MCCAR(Kinopio),
    W20_MCCAR(Teresa),
    W20_MCCAR(Minikoopa),
    W20_HSF_heiho,
    W20_HSF_heihoIdle,
    W20_HSF_heihoWin,
    W20_HSF_noko2,
    W20_HSF_noko2Jump,
    W20_HSF_pakkun,
    W20_HSF_pakkunIdle,
    W20_HSF_pakkunAngry,
    W20_ANM_turnFlag,
    W20_ANM_turnNum,
    W20_ANM_logo,
    W20_ANM_goal
};

#undef W20_MCCAR

#endif