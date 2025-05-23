#ifndef DATANUM_BOARD_H
#define DATANUM_BOARD_H

#include "datadir_enum.h"

enum {
    BOARD_HSF_yaji = DATANUM(DATA_board, 0),
    BOARD_HSF_confetti = DATANUM(DATA_board, 1),
    BOARD_HSF_coin = DATANUM(DATA_board, 2),
    BOARD_HSF_coinMot = DATANUM(DATA_board, 3),
    BOARD_HSF_star = DATANUM(DATA_board, 4),
    BOARD_HSF_starMot = DATANUM(DATA_board, 5),
    BOARD_HSF_mapCapsule = DATANUM(DATA_board, 6),
    BOARD_HSF_dossun = DATANUM(DATA_board, 7),
    BOARD_HSF_dossunIdle = DATANUM(DATA_board, 8),
    BOARD_HSF_battan = DATANUM(DATA_board, 9),
    BOARD_HSF_battanIdle = DATANUM(DATA_board, 10),
    BOARD_HSF_battanMove = DATANUM(DATA_board, 11),
    BOARD_HSF_num0 = DATANUM(DATA_board, 12),
    BOARD_HSF_num1 = DATANUM(DATA_board, 13),
    BOARD_HSF_num2 = DATANUM(DATA_board, 14),
    BOARD_HSF_num3 = DATANUM(DATA_board, 15),
    BOARD_HSF_num4 = DATANUM(DATA_board, 16),
    BOARD_HSF_num5 = DATANUM(DATA_board, 17),
    BOARD_HSF_num6 = DATANUM(DATA_board, 18),
    BOARD_HSF_num7 = DATANUM(DATA_board, 19),
    BOARD_HSF_num8 = DATANUM(DATA_board, 20),
    BOARD_HSF_num9 = DATANUM(DATA_board, 21),
    BOARD_HSF_numPlus = DATANUM(DATA_board, 22),
    BOARD_HSF_numMinus = DATANUM(DATA_board, 23),
    BOARD_HSF_sai = DATANUM(DATA_board, 24),
    BOARD_HSF_sai1 = DATANUM(DATA_board, 25),
    BOARD_HSF_sai2 = DATANUM(DATA_board, 26),
    BOARD_HSF_sai3 = DATANUM(DATA_board, 27),
    BOARD_HSF_sai4 = DATANUM(DATA_board, 28),
    BOARD_HSF_sai5 = DATANUM(DATA_board, 29),
    BOARD_HSF_sai7 = DATANUM(DATA_board, 30),
    BOARD_HSF_sai8 = DATANUM(DATA_board, 31),
    BOARD_HSF_sai9 = DATANUM(DATA_board, 32),
    BOARD_HSF_capMachine = DATANUM(DATA_board, 33),
    BOARD_HSF_capMachineMouth = DATANUM(DATA_board, 34),
    BOARD_HSF_capMachineKakimaze = DATANUM(DATA_board, 35),
    BOARD_HSF_capsel = DATANUM(DATA_board, 36),
    BOARD_ANM_masu = DATANUM(DATA_board, 37),
    BOARD_ANM_masuLight = DATANUM(DATA_board, 38),
    BOARD_HSF_masuLightNext = DATANUM(DATA_board, 39),
    BOARD_ANM_masuCapsule = DATANUM(DATA_board, 40),
    BOARD_ANM_masuMapView = DATANUM(DATA_board, 41),
    BOARD_ANM_cursor = DATANUM(DATA_board, 42),
    BOARD_ANM_statusNum = DATANUM(DATA_board, 43),
    BOARD_ANM_statusCom = DATANUM(DATA_board, 44),
    BOARD_ANM_statusStar = DATANUM(DATA_board, 45),
    BOARD_ANM_statusCoin = DATANUM(DATA_board, 46),
    BOARD_ANM_statusRank = DATANUM(DATA_board, 47),
    BOARD_ANM_statusRankTeamRed = DATANUM(DATA_board, 48),
    BOARD_ANM_statusRankTeamBlue = DATANUM(DATA_board, 49),
    BOARD_ANM_statusBack = DATANUM(DATA_board, 50),
    BOARD_ANM_statusBorder = DATANUM(DATA_board, 51),
    BOARD_ANM_statusTeamBack = DATANUM(DATA_board, 52),
    BOARD_ANM_statusTeamBorder = DATANUM(DATA_board, 53),
    BOARD_ANM_statusCapsule = DATANUM(DATA_board, 54),
    BOARD_ANM_statusSaiMode = DATANUM(DATA_board, 55),
    BOARD_ANM_statusKaoMario = DATANUM(DATA_board, 56),
    BOARD_ANM_statusKaoLuigi = DATANUM(DATA_board, 57),
    BOARD_ANM_statusKaoPeach = DATANUM(DATA_board, 58),
    BOARD_ANM_statusKaoYoshi = DATANUM(DATA_board, 59),
    BOARD_ANM_statusKaoWario = DATANUM(DATA_board, 60),
    BOARD_ANM_statusKaoDaisy = DATANUM(DATA_board, 61),
    BOARD_ANM_statusKaoWaluigi = DATANUM(DATA_board, 62),
    BOARD_ANM_statusKaoKinopio = DATANUM(DATA_board, 63),
    BOARD_ANM_statusKaoTeresa = DATANUM(DATA_board, 64),
    BOARD_ANM_statusKaoMinikoopa = DATANUM(DATA_board, 65),
    BOARD_ANM_statusKaoMinikoopaR = DATANUM(DATA_board, 66),
    BOARD_ANM_statusKaoMinikoopaG = DATANUM(DATA_board, 67),
    BOARD_ANM_statusKaoMinikoopaB = DATANUM(DATA_board, 68),
    BOARD_ANM_mgCallVs = DATANUM(DATA_board, 69),
    BOARD_ANM_mgCall1Vs3 = DATANUM(DATA_board, 70),
    BOARD_ANM_mgCall2Vs2 = DATANUM(DATA_board, 71),
    BOARD_ANM_mgCall4P = DATANUM(DATA_board, 72),
    BOARD_ANM_mgCallKettou = DATANUM(DATA_board, 73),
    BOARD_ANM_mgCallList = DATANUM(DATA_board, 74),
    BOARD_ANM_mgCallListCoin = DATANUM(DATA_board, 75),
    BOARD_ANM_logoMap1 = DATANUM(DATA_board, 76),
    BOARD_ANM_logoMap2 = DATANUM(DATA_board, 77),
    BOARD_ANM_logoMap3 = DATANUM(DATA_board, 78),
    BOARD_ANM_logoMap4 = DATANUM(DATA_board, 79),
    BOARD_ANM_logoMap5 = DATANUM(DATA_board, 80),
    BOARD_ANM_logoMap6 = DATANUM(DATA_board, 81),
    BOARD_ANM_logoMap7 = DATANUM(DATA_board, 82),
    BOARD_ANM_rouletteBorder = DATANUM(DATA_board, 83),
    BOARD_ANM_rouletteSeg2 = DATANUM(DATA_board, 84),
    BOARD_ANM_rouletteSeg3 = DATANUM(DATA_board, 85),
    BOARD_ANM_rouletteCursorShadow = DATANUM(DATA_board, 86),
    BOARD_ANM_rouletteCursor = DATANUM(DATA_board, 87),
    BOARD_ANM_rouletteMario = DATANUM(DATA_board, 88),
    BOARD_ANM_rouletteLuigi = DATANUM(DATA_board, 89),
    BOARD_ANM_roulettePeach = DATANUM(DATA_board, 90),
    BOARD_ANM_rouletteYoshi = DATANUM(DATA_board, 91),
    BOARD_ANM_rouletteWario = DATANUM(DATA_board, 92),
    BOARD_ANM_rouletteDaisy = DATANUM(DATA_board, 93),
    BOARD_ANM_rouletteWaluigi = DATANUM(DATA_board, 94),
    BOARD_ANM_rouletteKinopio = DATANUM(DATA_board, 95),
    BOARD_ANM_rouletteTeresa = DATANUM(DATA_board, 96),
    BOARD_ANM_rouletteMinikoopa = DATANUM(DATA_board, 97),
    BOARD_ANM_rouletteMinikoopaR = DATANUM(DATA_board, 98),
    BOARD_ANM_rouletteMinikoopaG = DATANUM(DATA_board, 99),
    BOARD_ANM_rouletteMinikoopaB = DATANUM(DATA_board, 100),
    BOARD_ANM_telopMario = DATANUM(DATA_board, 101),
    BOARD_ANM_telopLuigi = DATANUM(DATA_board, 102),
    BOARD_ANM_telopPeach = DATANUM(DATA_board, 103),
    BOARD_ANM_telopYoshi = DATANUM(DATA_board, 104),
    BOARD_ANM_telopWario = DATANUM(DATA_board, 105),
    BOARD_ANM_telopDaisy = DATANUM(DATA_board, 106),
    BOARD_ANM_telopWaluigi = DATANUM(DATA_board, 107),
    BOARD_ANM_telopKinopio = DATANUM(DATA_board, 108),
    BOARD_ANM_telopTeresa = DATANUM(DATA_board, 109),
    BOARD_ANM_telopMinikoopa = DATANUM(DATA_board, 110),
    BOARD_ANM_telopMinikoopaR = DATANUM(DATA_board, 111),
    BOARD_ANM_telopMinikoopaG = DATANUM(DATA_board, 112),
    BOARD_ANM_telopMinikoopaB = DATANUM(DATA_board, 113),
    BOARD_ANM_telopLast = DATANUM(DATA_board, 114),
    BOARD_ANM_telopTurn = DATANUM(DATA_board, 115),
    BOARD_ANM_telopTurnMulti = DATANUM(DATA_board, 116),
    BOARD_ANM_telopTurnNum = DATANUM(DATA_board, 117),
    BOARD_HSF_effLight = DATANUM(DATA_board, 118),
    BOARD_HSF_effLightRing = DATANUM(DATA_board, 119),
    BOARD_HSF_effSpeed = DATANUM(DATA_board, 120),
    BOARD_HSF_effFire = DATANUM(DATA_board, 121),
    BOARD_HSF_effThunder = DATANUM(DATA_board, 122),
    BOARD_HSF_effExplode = DATANUM(DATA_board, 123),
    BOARD_ANM_effGrad = DATANUM(DATA_board, 124),
    BOARD_ANM_effBubble = DATANUM(DATA_board, 125),
    BOARD_ANM_effBubbleMask = DATANUM(DATA_board, 126),
    BOARD_ANM_effBubbleBump = DATANUM(DATA_board, 127),
    BOARD_ANM_effPuff = DATANUM(DATA_board, 128),
    BOARD_ANM_effPuffDie = DATANUM(DATA_board, 129),
    BOARD_ANM_effExplode = DATANUM(DATA_board, 130),
    BOARD_ANM_effSmoke = DATANUM(DATA_board, 131),
    BOARD_ANM_effFire = DATANUM(DATA_board, 132),
    BOARD_ANM_effTri = DATANUM(DATA_board, 133),
    BOARD_ANM_effCloud = DATANUM(DATA_board, 134),
    BOARD_ANM_effCoin = DATANUM(DATA_board, 135),
    BOARD_ANM_mapViewYaji = DATANUM(DATA_board, 136),
    BOARD_ANM_mapViewYajiLine = DATANUM(DATA_board, 137),
    BOARD_ANM_mapViewPath = DATANUM(DATA_board, 138),
    BOARD_ANM_mapViewStar = DATANUM(DATA_board, 139),
    BOARD_ANM_telopKoopaEvBonus = DATANUM(DATA_board, 140),
    BOARD_ANM_telopKoopaEvCoinSwap = DATANUM(DATA_board, 141),
    BOARD_ANM_telopKoopaEvShuffle = DATANUM(DATA_board, 142),
    BOARD_ANM_telopKoopaEvMg = DATANUM(DATA_board, 143),
    BOARD_ANM_telopDonkeyEvRoulette = DATANUM(DATA_board, 144),
    BOARD_ANM_telopDonkeyEvBonus = DATANUM(DATA_board, 145),
    BOARD_ANM_telopDonkeyEvMg = DATANUM(DATA_board, 146),
    BOARD_ANM_telopComNumMinikoopa = DATANUM(DATA_board, 147),
    BOARD_ANM_telopComNumRemain = DATANUM(DATA_board, 148),
    BOARD_ANM_telopComNum = DATANUM(DATA_board, 149),
    BOARD_ANM_telopStoryClear = DATANUM(DATA_board, 150),
    BOARD_ANM_telopStoryMgVs = DATANUM(DATA_board, 151),
};

#endif