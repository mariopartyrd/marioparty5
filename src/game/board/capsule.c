#include "game/esprite.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/com.h"
#include "game/board/masu.h"
#include "game/board/model.h"
#include "game/board/player.h"
#include "game/board/pad.h"
#include "game/board/pause.h"
#include "game/board/status.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/board/audio.h"
#include "game/board/coin.h"

#include "datanum/capsule.h"
#include "messnum/capsule_ex99.h"
#include "messnum/capsule_ex98.h"

typedef struct CapsuleData_s {
    unsigned int mdlFile;
    unsigned int objMdlFile;
    unsigned int mesNo;
    int masuPatNo;
    int colorNo;
    int cost;
    s8 code;
    s8 listFlag;
} CAPSULE_DATA;

typedef struct CapsuleComCharData_s {
    s8 code1;
    s8 code2;
    s8 forceRejectF : 1;
} CAPSULE_COMCHARDATA;

typedef struct CapsuleComData_s {
    CAPSULE_COMCHARDATA charData[10];
    s8 minCoin;
} CAPSULE_COMDATA;

static BOOL comCapsuleEndF[GW_PLAYER_MAX] = {};
static int jumpMotTbl[CHARNO_MAX] = {
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, luigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, peach),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, yoshi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, wario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, donkey),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, daisy),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, waluigi),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, mario),
    CHARMOT_DATANUM(CHARMOT_HSF_c000m1_303, mario),
};

static CAPSULE_DATA capsuleTbl[CAPSULE_MAX] = {
    {
        CAPSULE_HSF_selKinoko,
        CAPSULE_HSF_capsuleGreen,
        MESS_CAPSULE_EX98_KINOKO,
        CAPSULE_MASUPAT_KINOKO,
        CAPSULE_COLOR_GREEN,
        5,
        'A',
        FALSE
    },
    {
        CAPSULE_HSF_selSKinoko,
        CAPSULE_HSF_capsuleGreen,
        MESS_CAPSULE_EX98_SKINOKO,
        CAPSULE_MASUPAT_KINOKO,
        CAPSULE_COLOR_GREEN,
        10,
        'B',
        FALSE
    },
    {
        CAPSULE_HSF_selNKinoko,
        CAPSULE_HSF_capsuleGreen,
        MESS_CAPSULE_EX98_NKINOKO,
        CAPSULE_MASUPAT_KINOKO,
        CAPSULE_COLOR_GREEN,
        5,
        'A',
        FALSE
    },
    {
        CAPSULE_HSF_selDokan,
        CAPSULE_HSF_capsuleGreen,
        MESS_CAPSULE_EX98_DOKAN,
        CAPSULE_MASUPAT_MOVE,
        CAPSULE_COLOR_GREEN,
        10,
        'A',
        FALSE
    },
    {
        CAPSULE_HSF_selJango,
        CAPSULE_HSF_capsuleGreen,
        MESS_CAPSULE_EX98_JANGO,
        CAPSULE_MASUPAT_MOVE,
        CAPSULE_COLOR_GREEN,
        10,
        'B',
        FALSE
    },
    {
        CAPSULE_HSF_selBoble,
        CAPSULE_HSF_capsuleGreen,
        MESS_CAPSULE_EX98_BOBLE,
        CAPSULE_MASUPAT_MOVE,
        CAPSULE_COLOR_GREEN,
        10,
        'C',
        FALSE
    },
    {
        CAPSULE_HSF_selHanachan,
        CAPSULE_HSF_capsuleGreen,
        MESS_CAPSULE_EX98_HANACHAN,
        CAPSULE_MASUPAT_MOVE,
        CAPSULE_COLOR_GREEN,
        20,
        'E',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        CAPSULE_HSF_selHammerbro,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_HAMMERBRO,
        CAPSULE_MASUPAT_COIN,
        CAPSULE_COLOR_YELLOW,
        5,
        'A',
        FALSE
    },
    {
        CAPSULE_HSF_selCoinblock,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_COINBLOCK,
        CAPSULE_MASUPAT_COIN,
        CAPSULE_COLOR_YELLOW,
        5,
        'A',
        FALSE
    },
    {
        CAPSULE_HSF_selTogezo,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_TOGEZO,
        CAPSULE_MASUPAT_COIN,
        CAPSULE_COLOR_YELLOW,
        5,
        'B',
        FALSE
    },
    {
        CAPSULE_HSF_selPatapata,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_PATAPATA,
        CAPSULE_MASUPAT_COIN,
        CAPSULE_COLOR_YELLOW,
        10,
        'D',
        FALSE
    },
    {
        CAPSULE_HSF_selKiller,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_KILLER,
        CAPSULE_MASUPAT_COIN,
        CAPSULE_COLOR_YELLOW,
        10,
        'E',
        TRUE
    },
    {
        CAPSULE_HSF_selKuribo,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_KURIBO,
        CAPSULE_MASUPAT_COIN,
        CAPSULE_COLOR_YELLOW,
        5,
        'C',
        FALSE
    },
    {
        CAPSULE_HSF_selBomhei,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_BOMHEI,
        CAPSULE_MASUPAT_BOMHEI,
        CAPSULE_COLOR_YELLOW,
        0,
        'B',
        TRUE
    },
    {
        CAPSULE_HSF_selBank,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_BANK,
        CAPSULE_MASUPAT_BANK,
        CAPSULE_COLOR_YELLOW,
        10,
        'A',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        CAPSULE_HSF_selKamekku,
        CAPSULE_HSF_capsuleRed,
        MESS_CAPSULE_EX98_KAMEKKU,
        CAPSULE_MASUPAT_WARN,
        CAPSULE_COLOR_RED,
        10,
        'B',
        FALSE
    },
    {
        CAPSULE_HSF_selThrowman,
        CAPSULE_HSF_capsuleRed,
        MESS_CAPSULE_EX98_THROWMAN,
        CAPSULE_MASUPAT_WARN,
        CAPSULE_COLOR_RED,
        5,
        'C',
        FALSE
    },
    {
        CAPSULE_HSF_selPakkun,
        CAPSULE_HSF_capsuleYellow,
        MESS_CAPSULE_EX98_PAKKUN,
        CAPSULE_MASUPAT_COIN,
        CAPSULE_COLOR_YELLOW,
        5,
        'B',
        FALSE
    },
    {
        CAPSULE_HSF_selKokamekku,
        CAPSULE_HSF_capsuleRed,
        MESS_CAPSULE_EX98_KOKAMEKKU,
        CAPSULE_MASUPAT_WARN,
        CAPSULE_COLOR_RED,
        10,
        'C',
        FALSE
    },
    {
        CAPSULE_HSF_selUkki,
        CAPSULE_HSF_capsuleRed,
        MESS_CAPSULE_EX98_UKKI,
        CAPSULE_MASUPAT_WARN,
        CAPSULE_COLOR_RED,
        10,
        'C',
        FALSE
    },
    {
        CAPSULE_HSF_selJugem,
        CAPSULE_HSF_capsuleRed,
        MESS_CAPSULE_EX98_JUGEM,
        CAPSULE_MASUPAT_WARN,
        CAPSULE_COLOR_RED,
        10,
        'B',
        FALSE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        CAPSULE_HSF_selTumujikun,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_TUMUJIKUN,
        CAPSULE_MASUPAT_TUMUJIKUN,
        CAPSULE_COLOR_BLUE,
        15,
        'D',
        TRUE
    },
    {
        CAPSULE_HSF_selDuel,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_KETTOU,
        CAPSULE_MASUPAT_KETTOU,
        CAPSULE_COLOR_BLUE,
        10,
        'D',
        FALSE
    },
    {
        CAPSULE_HSF_selWanwan,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_WANWAN,
        CAPSULE_MASUPAT_WANWAN,
        CAPSULE_COLOR_BLUE,
        15,
        'D',
        TRUE
    },
    {
        CAPSULE_HSF_selHone,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_HONE,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_BLUE,
        -1,
        'C',
        TRUE
    },
    {
        CAPSULE_HSF_selKoopa,
        CAPSULE_HSF_capsuleKoopa,
        MESS_CAPSULE_EX98_KOOPA,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_BLUE,
        -1,
        'C',
        FALSE
    },
    {
        CAPSULE_HSF_selChance,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_CHANCE,
        CAPSULE_MASUPAT_CHANCE,
        CAPSULE_COLOR_BLUE,
        20,
        'E',
        TRUE
    },
    {
        CAPSULE_HSF_selMiracle,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_MIRACLE,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_BLUE,
        -1,
        'E',
        TRUE
    },
    {
        CAPSULE_HSF_selMiracle,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_DONKEY,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_BLUE,
        -1,
        'Z',
        TRUE
    },
    {
        CAPSULE_HSF_selMiracle,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_VS,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_BLUE,
        -1,
        'Z',
        TRUE
    },
    {
        0,
        0,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        CAPSULE_HSF_selMiracle,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        CAPSULE_HSF_selMiracle,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
    {
        CAPSULE_HSF_selMiracle,
        CAPSULE_HSF_capsuleBlue,
        MESS_CAPSULE_EX98_ERROR,
        CAPSULE_MASUPAT_NONE,
        CAPSULE_COLOR_GREEN,
        0,
        'Z',
        TRUE
    },
};

static CAPSULE_COMDATA capsuleComData[CAPSULE_MAX] = {
    {
        {
            { 'A', 'a' },
            { 'A', 'a' },
            { 'B', 'b' },
            { 'A', 'a' },
            { 'D', 'c' },
            { 'A', 'a' },
            { 'C', 'c' },
            { 'A', 'a' },
            { 'B', 'a' },
            { 'C', 'b' },
        },
        0
    },
    {
        {
            { 'A', 'a' },
            { 'A', 'a' },
            { 'B', 'b' },
            { 'A', 'b' },
            { 'C', 'b' },
            { 'A', 'a' },
            { 'C', 'b' },
            { 'A', 'a' },
            { 'B', 'c' },
            { 'C', 'a' },
        },
        0
    },
    {
        {
            { 'B', 'd' },
            { 'D', 'd' },
            { 'C', 'd' },
            { 'B', 'c' },
            { 'A', 'c' },
            { 'C', 'c' },
            { 'A', 'c' },
            { 'C', 'd' },
            { 'C', 'c' },
            { 'B', 'c' },
        },
        0
    },
    {
        {
            { 'D', 'a' },
            { 'D', 'a' },
            { 'C', 'a' },
            { 'A', 'a' },
            { 'B', 'a' },
            { 'C', 'b' },
            { 'B', 'a' },
            { 'D', 'a', TRUE },
            { 'A', 'a' },
            { 'D', 'b' },
        },
        30
    },
    {
        {
            { 'A', 'a' },
            { 'A', 'a' },
            { 'B', 'd' },
            { 'A', 'a' },
            { 'B', 'a' },
            { 'C', 'b' },
            { 'B', 'b' },
            { 'B', 'c' },
            { 'A', 'c' },
            { 'C', 'b' },
        },
        0
    },
    {
        {
            { 'A', 'b' },
            { 'A', 'b' },
            { 'C', 'c' },
            { 'B', 'a' },
            { 'A', 'c' },
            { 'C', 'c' },
            { 'C', 'b' },
            { 'C', 'c' },
            { 'C', 'c' },
            { 'C', 'c' },
        },
        0
    },
    {
        {
            { 'A', 'b' },
            { 'B', 'a', TRUE },
            { 'B', 'a', TRUE },
            { 'A', 'b' },
            { 'B', 'a' },
            { 'A', 'b' },
            { 'C', 'a' },
            { 'D', 'a', TRUE },
            { 'C', 'b' },
            { 'B', 'a' },
        },
        40
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'C', 'd' },
            { 'A', 'd' },
            { 'B', 'd' },
            { 'C', 'd' },
            { 'A', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
        },
        0
    },
    {
        {
            { 'A', 'b' },
            { 'C', 'a' },
            { 'A', 'a' },
            { 'B', 'd' },
            { 'A', 'b' },
            { 'C', 'b' },
            { 'B', 'c' },
            { 'A', 'a' },
            { 'A', 'a' },
            { 'B', 'c' },
        },
        0
    },
    {
        {
            { 'C', 'c' },
            { 'B', 'a' },
            { 'A', 'a' },
            { 'A', 'c' },
            { 'A', 'a' },
            { 'B', 'b' },
            { 'C', 'c' },
            { 'B', 'a' },
            { 'A', 'c' },
            { 'A', 'a' },
        },
        0
    },
    {
        {
            { 'C', 'c' },
            { 'B', 'c' },
            { 'A', 'a' },
            { 'B', 'b' },
            { 'C', 'c' },
            { 'B', 'b' },
            { 'A', 'a' },
            { 'D', 'a' },
            { 'B', 'c' },
            { 'C', 'c' },
        },
        0
    },
    {
        {
            { 'B', 'b' },
            { 'C', 'a' },
            { 'C', 'c' },
            { 'B', 'b' },
            { 'B', 'a' },
            { 'C', 'b' },
            { 'A', 'a' },
            { 'C', 'c' },
            { 'B', 'c' },
            { 'A', 'a' },
        },
        0
    },
    {
        {
            { 'C', 'b' },
            { 'B', 'c' },
            { 'A', 'a', TRUE },
            { 'B', 'b' },
            { 'C', 'c' },
            { 'C', 'b' },
            { 'A', 'a' },
            { 'B', 'a', TRUE },
            { 'A', 'a' },
            { 'B', 'c' },
        },
        -10
    },
    {
        {
            { 'C', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
            { 'C', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
            { 'C', 'd' },
        },
        0
    },
    {
        {
            { 'B', 'c' },
            { 'C', 'b' },
            { 'A', 'c' },
            { 'B', 'c' },
            { 'C', 'c' },
            { 'B', 'c' },
            { 'B', 'b' },
            { 'C', 'c' },
            { 'A', 'c' },
            { 'B', 'c' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'B', 'c' },
            { 'C', 'a' },
            { 'C', 'c' },
            { 'B', 'b' },
            { 'C', 'b' },
            { 'A', 'a' },
            { 'A', 'a' },
            { 'C', 'c' },
            { 'A', 'a' },
            { 'B', 'b' },
        },
        0
    },
    {
        {
            { 'C', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
            { 'A', 'd' },
            { 'B', 'd' },
            { 'C', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
        },
        0
    },
    {
        {
            { 'B', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
            { 'C', 'd' },
            { 'C', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
            { 'B', 'd' },
            { 'A', 'd' },
        },
        0
    },
    {
        {
            { 'B', 'd' },
            { 'C', 'c' },
            { 'C', 'd' },
            { 'C', 'b' },
            { 'A', 'a' },
            { 'A', 'a' },
            { 'C', 'a' },
            { 'C', 'd' },
            { 'B', 'a' },
            { 'A', 'b' },
        },
        0
    },
    {
        {
            { 'C', 'c' },
            { 'B', 'b' },
            { 'B', 'b' },
            { 'B', 'c' },
            { 'A', 'b' },
            { 'B', 'c' },
            { 'B', 'a' },
            { 'A', 'c' },
            { 'A', 'b' },
            { 'A', 'b' },
        },
        0
    },
    {
        {
            { 'C', 'c' },
            { 'B', 'c' },
            { 'B', 'b' },
            { 'B', 'b' },
            { 'A', 'a' },
            { 'A', 'a' },
            { 'B', 'b' },
            { 'A', 'c' },
            { 'C', 'b' },
            { 'A', 'a' },
        },
        0
    },
     {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
     {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
     {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
     {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'D', 'a' },
            { 'D', 'a' },
            { 'D', 'a' },
            { 'A', 'c' },
            { 'C', 'b' },
            { 'B', 'd' },
            { 'A', 'a' },
            { 'D', 'a' },
            { 'A', 'a' },
            { 'B', 'b' },
        },
        0
    },
    {
        {
            { 'A', 'a' },
            { 'B', 'c' },
            { 'C', 'c' },
            { 'B', 'b' },
            { 'A', 'a' },
            { 'C', 'c' },
            { 'A', 'a' },
            { 'D', 'a' },
            { 'B', 'c' },
            { 'A', 'c' },
        },
        0
    },
    {
        {
            { 'B', 'c' },
            { 'A', 'b' },
            { 'A', 'b' },
            { 'B', 'c' },
            { 'B', 'a' },
            { 'A', 'c' },
            { 'B', 'b' },
            { 'A', 'b' },
            { 'C', 'c' },
            { 'B', 'b' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'B', 'c' },
            { 'B', 'c' },
            { 'A', 'a' },
            { 'A', 'b' },
            { 'B', 'b' },
            { 'B', 'c' },
            { 'A', 'b' },
            { 'B', 'a' },
            { 'C', 'a' },
            { 'C', 'a' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
    {
        {
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
            { 'E', 'd' },
        },
        0
    },
};

static u32 capsuleMesTbl[CAPSULE_MAX] = {
    MESS_CAPSULE_EX99_KINOKO,
    MESS_CAPSULE_EX99_SKINOKO,
    MESS_CAPSULE_EX99_NKINOKO,
    MESS_CAPSULE_EX99_DOKAN,
    MESS_CAPSULE_EX99_JANGO,
    MESS_CAPSULE_EX99_BOBLE,
    MESS_CAPSULE_EX99_HANACHAN,
    0,
    0,
    0,
    MESS_CAPSULE_EX99_HAMMERBRO,
    MESS_CAPSULE_EX99_COINBLOCK,
    MESS_CAPSULE_EX99_TOGEZO,
    MESS_CAPSULE_EX99_PATAPATA,
    MESS_CAPSULE_EX99_KILLER,
    MESS_CAPSULE_EX99_KURIBO,
    MESS_CAPSULE_EX99_BOMHEI,
    MESS_CAPSULE_EX99_BANK,
    0,
    0,
    MESS_CAPSULE_EX99_KAMEKKU,
    MESS_CAPSULE_EX99_THROWMAN,
    MESS_CAPSULE_EX99_PAKKUN,
    MESS_CAPSULE_EX99_KOKAMEKKU,
    MESS_CAPSULE_EX99_UKKI,
    MESS_CAPSULE_EX99_LAKITU,
    0,
    0,
    0,
    0,
    MESS_CAPSULE_EX99_TUMUJIKUN,
    MESS_CAPSULE_EX99_KETTOU,
    MESS_CAPSULE_EX99_WANWAN,
    MESS_CAPSULE_EX99_HONE,
    MESS_CAPSULE_EX99_KOOPA,
    MESS_CAPSULE_EX99_CHANCE,
    MESS_CAPSULE_EX99_MIRACLE,
    MESS_CAPSULE_EX99_DONKEY,
    MESS_CAPSULE_EX99_VS,
    0,
    MESS_CAPSULE_EX98_ERROR,
    MESS_CAPSULE_EX98_ERROR,
    MESS_CAPSULE_EX98_ERROR
};

static u32 capsuleStoryMesTbl[CAPSULE_MAX] = {
    MESS_CAPSULE_EX99_KINOKO,
    MESS_CAPSULE_EX99_SKINOKO,
    MESS_CAPSULE_EX99_NKINOKO,
    MESS_CAPSULE_EX99_DOKAN,
    MESS_CAPSULE_EX99_JANGO,
    MESS_CAPSULE_EX99_BOBLE,
    MESS_CAPSULE_EX99_HANACHAN,
    0,
    0,
    0,
    MESS_CAPSULE_EX99_HAMMERBRO_STORY,
    MESS_CAPSULE_EX99_COINBLOCK,
    MESS_CAPSULE_EX99_TOGEZO_STORY,
    MESS_CAPSULE_EX99_PATAPATA,
    MESS_CAPSULE_EX99_KILLER_STORY,
    MESS_CAPSULE_EX99_KURIBO,
    MESS_CAPSULE_EX99_BOMHEI,
    MESS_CAPSULE_EX99_BANK,
    0,
    0,
    MESS_CAPSULE_EX99_KAMEKKU,
    MESS_CAPSULE_EX99_THROWMAN,
    MESS_CAPSULE_EX99_PAKKUN,
    MESS_CAPSULE_EX99_KOKAMEKKU,
    MESS_CAPSULE_EX99_UKKI,
    MESS_CAPSULE_EX99_LAKITU,
    0,
    0,
    0,
    0,
    MESS_CAPSULE_EX99_TUMUJIKUN,
    MESS_CAPSULE_EX99_KETTOU,
    MESS_CAPSULE_EX99_WANWAN,
    MESS_CAPSULE_EX99_HONE,
    MESS_CAPSULE_EX99_KOOPA,
    MESS_CAPSULE_EX99_CHANCE,
    MESS_CAPSULE_EX99_MIRACLE,
    MESS_CAPSULE_EX99_DONKEY,
    MESS_CAPSULE_EX99_VS,
    0,
    MESS_CAPSULE_EX98_ERROR,
    MESS_CAPSULE_EX98_ERROR,
    MESS_CAPSULE_EX98_ERROR
};

typedef struct CapsuleYajiWork_s {
    int mdlId[10];
} CAPSULE_YAJI_WORK;

typedef struct CapsulePosSelWork_s {
    int playerNo;
    int masuId;
    int capsuleNo;
    int cursorSprId;
    int batsuSprId;
    int capsuleMdlId1;
    int capsuleMdlId2;
    int capsuleSelWinId;
    int capsuleMasuWinId;
    int unk24;
    s16 *masuIdTbl;
    s16 *capsuleMasuTbl;
    s16 *masuIdTblPath;
} CAPSULE_POSSEL_WORK;

typedef struct CapsuleThrowWork_s {
    int playerNo;
    int masuId;
    int capsuleNo;
    int capsuleMdlId1;
    int capsuleMdlId2;
    int capsuleObjMdlId;
    int jumpMotId;
    int time;
    int maxTime;
    float yOfs;
    HuVecF playerPos;
    HuVecF masuPos;
    HuVecF upPos;
} CAPSULE_THROW_WORK;

static BOOL noRemoveF;
static BOOL listStartF;
static int comNoUseChoice;
static OMOBJ *throwEffObj;
static OMOBJ *rayEffObj;
static OMOBJ *domeEffObj;
static OMOBJ *ringEffObj;
static OMOBJ *glowEffObj;
static OMOBJ *explodeEffObj;
static OMOBJ *capsuleYajiObj;
static OMOBJ *capsuleYajiWork;
static HUPROCESS *capsuleUseProc;
static BOOL capsuleMasuEndF;
static CAPSULE_POSSEL_WORK *capsulePosSelWork;
static HUPROCESS *capsulePosSelProc;
static CAPSULE_THROW_WORK *capsuleThrowWork;
static HUPROCESS *capsuleThrowProc;

static int capsuleSelMasu = MASU_NULL;
static int capsuleSelResult = MASU_NULL;
static int capsuleUsePlayerNo = -1;
static int capsuleUseNo = CAPSULE_NULL;
static int capsuleMdl = MB_MODEL_NONE;
static int capsuleColMdl = MB_MODEL_NONE;
static int capsuleColMdlId = HU3D_MODELID_NONE;

static BOOL CapsuleMasuCheck(int masuId);
static BOOL CapsuleMasuVisitCheck(int masuId);


void MBCapsulePosSelStart(int playerNo, int capsuleNo);
BOOL MBCapsulePosSelCheck(void);
int MBCapsulePosSelResultGet(void);
void MBCapsuleThrowStart(int playerNo, int masuId, int capsuleNo);
BOOL MBCapsuleThrowCheck(void);

static BOOL CapsuleNoUseProcCreate(int playerNo, int capsuleNo);
static void CapsuleUseStart(int playerNo, int capsuleNo);
static BOOL CapsuleUseCheck(void);

BOOL MBCapsuleUseExec(int playerNo, int capsuleNo)
{
    capsuleNo &= 0xFF;
    capsuleSelMasu = MASU_NULL;
    noRemoveF = FALSE;
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        int result = MBTutorialExec(TUTORIAL_INST_SAI_CAPSULE_SET);
        if(!result) {
            capsuleSelMasu = GwPlayer[playerNo].masuId;
        } else {
            MASU *masuP;
            int i;
            int tries;
            capsuleSelMasu = GwPlayer[playerNo].masuId;
            tries = 4;
            do {
                for(masuP=MBMasuGet(MASU_LAYER_DEFAULT, capsuleSelMasu), i=0; i<masuP->linkNum; i++) {
                    if(!(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & (MBBranchFlagGet()|(MASU_FLAG_BLOCKL|MASU_FLAG_BLOCKR)))) {
                        break;
                    }
                }
                if(i < masuP->linkNum) {
                    capsuleSelMasu = masuP->linkTbl[i];
                }
                if(CapsuleMasuVisitCheck(capsuleSelMasu)) {
                    tries--;
                }
                if(tries == 0 && !CapsuleMasuCheck(capsuleSelMasu)) {
                    tries++;
                }
            } while(tries);
        }
    }
    if(MBPlayerCapsuleFind(playerNo, capsuleNo) == -1) {
        return TRUE;
    } else {
        int result;
        MBCapsulePosSelStart(playerNo, capsuleNo);
        while(!MBCapsulePosSelCheck()) {
            HuPrcVSleep();
        }
        result = MBCapsulePosSelResultGet();
        if(result != MASU_NULL) {
            if(result == -2) {
                return CapsuleNoUseProcCreate(playerNo, capsuleNo);
            } else if(result == GwPlayer[playerNo].masuId) {
                CapsuleUseStart(playerNo, capsuleNo);
                while(!CapsuleUseCheck()) {
                    HuPrcVSleep();
                }
                MBCapsuleExec(playerNo, capsuleNo, TRUE, FALSE);
            } else {
                MBCapsuleThrowStart(playerNo, result, capsuleNo);
                while(!MBCapsuleThrowCheck()) {
                    HuPrcVSleep();
                }
            }
            MB3MiracleGetExec(playerNo);
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

void MBCapsuleStub(void)
{
    
}

BOOL MBCapsuleStub0(void)
{
    return FALSE;
}

static void CapsuleUseExec(void);
static void CapsuleUseDestroy(void);

static void CapsuleUseStart(int playerNo, int capsuleNo)
{
    capsuleNo &= 0xFF;
    capsuleUsePlayerNo = playerNo;
    capsuleUseNo = capsuleNo;
    capsuleUseProc = MBPrcCreate(CapsuleUseExec, 8196, 24576);
    MBPrcDestructorSet(capsuleUseProc, CapsuleUseDestroy);
}

static BOOL CapsuleUseCheck(void)
{
    if(capsuleUseProc != NULL) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void GlowEffInit(void);
static void RingEffInit(void);
static int RingEffAdd(HuVecF pos, HuVecF ofs, HuVecF scale, int inTime, int rotTime, int no, int mode, GXColor color);
static void GlowEffClose(void);
static void RingEffClose(void);
static int GlowEffAdd(HuVecF pos, HuVecF ofs, float scale, float fadeSpeed, float rotSpeed, float gravity, GXColor color); 
static BOOL GlowEffCheck(void);
static BOOL RingEffCheck(void);

#define frand7f() ((frand() & 0x7F)*(1.0f/127.0f))

static void CapsuleUseExec(void)
{
    int i;
    int mdlId;
    int coinDispId;
    
    HuVecF playerPos;
    HuVecF mdlPos;
    HuVecF coinDispPos;
    HuVecF effectOfs;
    GXColor color;
    
    float intensity;
    float angle;
    float radius;
    
    static GXColor colTbl[CAPSULE_COLOR_MAX] = {
        { 192, 255, 192, 255 },
        { 255, 255, 192, 255 },
        { 255, 192, 192, 255 },
        { 192, 255, 255, 255 },
    };
    static HuVecF ringOfs = { -90, 0, 20 };
    
    
    GlowEffInit();
    RingEffInit();
    if(capsuleMdl != MB_MODEL_NONE) {
        mdlId = capsuleMdl;
    } else {
        mdlId = MBModelCreate(MBCapsuleMdlGet(capsuleUseNo), NULL, FALSE);
    }
    MBPlayerPosGet(capsuleUsePlayerNo, &playerPos);
    mdlPos = playerPos;
    mdlPos.y += 250;
    MBModelPosSet(mdlId, mdlPos.x, mdlPos.y, mdlPos.z);
    MBModelScaleSet(mdlId, 1.2f, 1.2f, 1.2f);
    MBModelLayerSet(mdlId, 2);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL)|| !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    }
    WipeWait();
    MBStatusDispFocusSet(capsuleUsePlayerNo, TRUE);
    while(!MBStatusMoveCheck(capsuleUsePlayerNo)){
        HuPrcVSleep();
    }
    if(MBCapsuleCostGet(capsuleUseNo) > 0) {
        MBCoinAddExec(capsuleUsePlayerNo, -MBCapsuleCostGet(capsuleUseNo));
        coinDispPos.x = playerPos.x;
        coinDispPos.y = playerPos.y+250;
        coinDispPos.z = playerPos.z+50;
        coinDispId = MBCoinDispCreateSe(&coinDispPos, -MBCapsuleCostGet(capsuleUseNo));
        do {
            HuPrcVSleep();
        } while(!MBCoinDispKillCheck(coinDispId));
    }
    coinDispPos.x = 0.75f;
    coinDispPos.y = 1.2f;
    coinDispPos.z = 300;
    color = colTbl[MBCapsuleColorNoGet(capsuleUseNo)];
    
    RingEffAdd(mdlPos, ringOfs, coinDispPos, 18, 60, 0, 1, color);
    MBAudFXPlay(MSM_SE_BOARD_30);
    for(i=0; i<18.0f; i++) {
        HuPrcVSleep();
    }
    coinDispPos.x = 1.5f;
    coinDispPos.y = 3.0f;
    coinDispPos.z = 200;
    color.r = (frand7f()*64)+192;
    color.g = (frand7f()*64)+192;
    color.b = (frand7f()*64)+192;
    color.a = 255;
    RingEffAdd(mdlPos, ringOfs, coinDispPos, 18, 36, 2, 1, color);
    coinDispPos.x = 1.5f;
    coinDispPos.y = 3.0f;
    coinDispPos.z = 300;
    color.r = (frand7f()*64)+192;
    color.g = (frand7f()*64)+192;
    color.b = (frand7f()*64)+192;
    color.a = 255;
    RingEffAdd(mdlPos, ringOfs, coinDispPos, 12, 30, 1, 1, color);
    angle = 0;
    for(i=0; i<128; i++) {
        angle += (frand7f()+1)*10;
        coinDispPos.x = (75*(-0.5f+frand7f()))+mdlPos.x;
        coinDispPos.y = (75*(-0.5f+frand7f()))+mdlPos.y;
        coinDispPos.z = 30.000002f+mdlPos.z;
        radius = (frand7f()+1.0f)*5;
        effectOfs.x = radius*HuSin(angle);
        effectOfs.y = radius*HuCos(angle);
        effectOfs.z = 0;
        intensity = frand7f();
        color.r = (intensity*63)+192;
        color.g = (intensity*63)+192;
        color.b = (intensity*63)+192;
        color.a = (frand7f()*63)+192;
        GlowEffAdd(coinDispPos, effectOfs, ((frand7f()*0.1f)+0.3f)*100, (frand7f()+1.0f)*0.016666668f, 0, 0, color);
    }
    for(i=0; i<30.0f; i++) {
        intensity = i/30.0f;
        MBModelAlphaSet(mdlId, (1-intensity)*255);
        HuPrcVSleep();
    }
    MBModelDispSet(mdlId, FALSE);
    capsuleMdl = MB_MODEL_NONE;
    if(!noRemoveF) {
        MBPlayerCapsuleUseSet(capsuleUseNo);
        i = MBPlayerCapsuleFind(capsuleUsePlayerNo, capsuleUseNo);
        if(i != -1) {
            MBPlayerCapsuleRemove(capsuleUsePlayerNo, i);
        }
    }
    do {
        HuPrcVSleep();
    } while(GlowEffCheck() && RingEffCheck());
    if(mdlId != MB_MODEL_NONE) {
        MBModelKill(mdlId);
    }
    
    HuPrcEnd();
}

static void CapsuleUseDestroy(void)
{
    GlowEffClose();
    RingEffClose();
    capsuleUseProc = NULL;
}

static void CapsuleThrowProcExec(void);
static void CapsuleThrowDestroy(void);

void MBCapsuleThrowStart(int playerNo, int masuId, int capsuleNo)
{
    CAPSULE_THROW_WORK *work;
    capsuleNo &= 0xFF;
    if(capsuleThrowWork == NULL) {
        capsuleThrowWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULE_THROW_WORK), HU_MEMNUM_OVL);
    }
    capsuleThrowWork == NULL;
    memset(capsuleThrowWork, 0, sizeof(CAPSULE_THROW_WORK));
    work = capsuleThrowWork;
    work->playerNo = playerNo;
    work->capsuleNo = capsuleNo;
    work->masuId = masuId;
    work->capsuleMdlId1 = MB_MODEL_NONE;
    work->capsuleMdlId2 = MB_MODEL_NONE;
    work->capsuleObjMdlId = MB_MODEL_NONE;
    work->time = 0;
    capsuleThrowProc = MBPrcCreate(CapsuleThrowProcExec, 8196, 24576);
    MBPrcDestructorSet(capsuleThrowProc, CapsuleThrowDestroy);
}

BOOL MBCapsuleThrowCheck(void)
{
    if(capsuleThrowProc) {
        return FALSE;
    } else {
        return TRUE;
    }
}

int MBCapsuleColCreate(int dataNum)
{
    int mdlId;
    if(dataNum != -1) {
        mdlId = MBModelCreate(dataNum, NULL, FALSE);
        MBModelDispSet(mdlId, FALSE);
        capsuleColMdl = mdlId;
    } else {
        capsuleColMdl = MB_MODEL_NONE;
    }
    return mdlId;
}

static void CapsuleColBegin(void);
static BOOL CapsuleColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out);
static void CapsuleColEnd(void);

BOOL MBCapsuleColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    BOOL result;
    CapsuleColBegin();
    result = CapsuleColCheck(posA, posB, out);
    CapsuleColEnd();
    return result;
}

BOOL MBCapsulePlayerMasuCheck(int playerNo)
{
    int i;
    BOOL result;
    for(i=0, result=FALSE; i<GW_PLAYER_MAX && !result; i++) {
        if(i != playerNo && MBPlayerAliveCheck(i)) {
            HuVecF posSelf;
            HuVecF pos;
            HuVecF dir;
            MBPlayerPosGet(playerNo, &posSelf);
            MBPlayerPosGet(i, &pos);
            VECSubtract(&posSelf, &pos, &dir);
            if(fabs(dir.x) < 300 && pos.z > posSelf.z && pos.z < posSelf.z+600) {
                result = TRUE;
            }
        }
    }
    return result;
}

static void ExplodeEffInit(void);
static void ExplodeEffClose(void);
static int ExplodeEffCheck(void);

static void ThrowEffInit(int capsuleNo);
static void ThrowEffClose(void);
static void RingEffNumSet(int num);
static void DomeEffInit(void);
static void RayEffInit(void);
static void DomeEffClose(void);
static void RayEffClose(void);

static void CapsuleThrowEffCreate(float *throwX, float *throwY, float *throwZ, float yOfs, int masuId);
static BOOL CapsuleThrowParamSet(HuVecF *pos, int *maxTime);
static void CapsuleThrowEffKill(void);
static void CapsuleThrowCameraSet(float *throwX, float *throwY, float *throwZ, int num);
static void CapsuleThrowCameraCalc(float t, float *throwX, float *throwY, float *throwZ, HuVecF *out, int num);
static void ThrowEffUpdate(HuVecF *pos, int capsuleNo);
static void ThrowEffPosSet(HuVecF *pos);
static void DomeEffMdlUpdate(HuVecF *pos, HuVecF *rot);
static void CapsuleThrowWin(int masuId, int capsuleNo);

static void CapsuleThrowProcExec(void)
{
    CAPSULE_THROW_WORK *work = capsuleThrowWork;
    int seNo;
    MASU *masuP;
    int num;
    int maxTime;
    
    float t;
    float time;
    float scale;
    float mag;
    
    Mtx rotMtx;
    Mtx objMtx;
    HuVecF effPos;
    HuVecF mdl2Pos;
    HuVecF dir;
    HuVecF dirOrig;
    HuVecF pos;
    HuVecF ofs;
    HuVecF playerPos;
    HuVecF playerPosOrig;
    float throwX[3];
    float throwY[3];
    float throwZ[3];
    HuVecF throwOut;

    GXColor color;
    float magTemp;
    static HuVecF ringOfs[2] = { { -90, 0, 0 }, { 0, 10, 60 } };
    static GXColor colTbl[CAPSULE_COLOR_MAX] = {
        { 192, 255, 192, 255 },
        { 255, 255, 192, 255 },
        { 255, 192, 192, 255 },
        { 192, 255, 255, 255 },
    };
    
    ExplodeEffInit();
    GlowEffInit();
    ThrowEffInit(work->capsuleNo);
    RingEffInit();
    RingEffNumSet(15);
    DomeEffInit();
    RayEffInit();
    CapsuleColBegin();
    if(capsuleMdl != MB_MODEL_NONE) {
        work->capsuleMdlId1 = capsuleMdl;
    } else {
        work->capsuleMdlId1 = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, FALSE);
    }
    MBPlayerPosGet(work->playerNo, &pos);
    MBModelPosSet(work->capsuleMdlId1, pos.x, pos.y+250, pos.z);
    MBModelScaleSet(work->capsuleMdlId1, 1.2f, 1.2f, 1.2f);
    MBModelLayerSet(work->capsuleMdlId1, 2);
    work->capsuleMdlId2 = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, FALSE);
    MBModelDispSet(work->capsuleMdlId2, FALSE);
    MBPlayerPosGet(work->playerNo, &work->playerPos);
    work->playerPos.y += 250;
    MBMasuPosGet(MASU_LAYER_DEFAULT, work->masuId, &work->masuPos);
    work->masuPos.y += 20;
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, work->masuId);
    VECSubtract(&work->masuPos, &work->playerPos, &dir);
    mag = VECMag(&dir);
    work->yOfs = 100+(0.1f*mag);
    if(work->yOfs > 700) {
        work->yOfs = 700;
    }
    playerPosOrig = work->playerPos;
    playerPos = playerPosOrig;
    throwX[0] = work->playerPos.x;
    throwY[0] = work->playerPos.y;
    throwZ[0] = work->playerPos.z;
    throwX[1] = work->playerPos.x;
    throwY[1] = work->playerPos.y;
    throwZ[1] = work->playerPos.z;
    throwX[2] = work->masuPos.x;
    throwY[2] = work->masuPos.y;
    throwZ[2] = work->masuPos.z;
    CapsuleThrowEffCreate(throwX, throwY, throwZ, work->yOfs, work->masuId);
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL)|| !MBTutorialExitFlagGet()) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSSCOPY, 15);
    }
    WipeWait();
    maxTime = 30;
    MBPlayerPosGet(work->playerNo, &pos);
    MBAudFXPlay(MSM_SE_BOARD_36);
    for(time=0; time<maxTime; time++) {
        t = 1-(time/maxTime);
        MBModelPosSet(work->capsuleMdlId1, pos.x, pos.y+(100+(150*t)), pos.z);
        scale = 1.2f*HuSin(t*90);
        MBModelScaleSet(work->capsuleMdlId1, scale, scale, scale);
        HuPrcVSleep();
    }
    do {
        HuPrcVSleep();
    } while(CapsuleThrowParamSet(&pos, &work->maxTime) == FALSE);
    CapsuleThrowEffKill();
    CapsuleColEnd();
    throwX[1] = pos.x;
    throwY[1] = pos.y;
    throwZ[1] = pos.z;
    work->time = 0;
    MBPlayerRotateStart(work->playerNo, HuAtan(dir.x, dir.z), 15);
    while(!MBPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    work->jumpMotId = MBPlayerMotionCreate(work->playerNo, CHARMOT_HSF_c000m1_303);
    MBPlayerMotionNoShiftSet(work->playerNo, work->jumpMotId, 0, 8, HU3D_MOTATTR_NONE);
    MBAudFXPlay(MSM_SE_BOARD_25);
    for(time=MBPlayerMotionTimeGet(work->playerNo); !MBPlayerMotionEndCheck(work->playerNo);) {
        HuPrcVSleep();
    }
    if(work->capsuleMdlId1 != MB_MODEL_NONE) {
        MBModelDispSet(work->capsuleMdlId1, FALSE);
    }
    capsuleMdl = MB_MODEL_NONE;
    work->capsuleObjMdlId = MBCapsuleObjCreate(work->capsuleNo, FALSE);
    MBPlayerPosGet(work->playerNo, &pos);
    MBCapsuleObjPosSet(work->capsuleObjMdlId, pos.x, pos.y+100, pos.z);
    MBCapsuleObjScaleSet(work->capsuleObjMdlId, 1, 1, 1);
    MBCapsuleObjLayerSet(work->capsuleObjMdlId, 2);
    ThrowEffUpdate(&work->playerPos, work->capsuleNo);
    seNo = MBAudFXPlay(MSM_SE_BOARD_26);
    do {
        work->time++;
        t = (float)work->time/(float)work->maxTime;
        VECSubtract(&work->masuPos, &work->playerPos, &dir);
        VECScale(&dir, &dir, t);
        VECAdd(&work->playerPos, &dir, &effPos);
        mdl2Pos = effPos;
        CapsuleThrowCameraCalc(t, throwX, throwY, throwZ, &throwOut, 3);
        effPos.x = throwOut.x;
        effPos.y = throwOut.y;
        effPos.z = throwOut.z;
        VECSubtract(&effPos, &mdl2Pos, &pos);
        VECScale(&pos, &pos, 0.75f);
        VECAdd(&mdl2Pos, &pos, &mdl2Pos);
        MBCapsuleObjPosSet(work->capsuleObjMdlId, effPos.x, effPos.y, effPos.z);
        MBCapsuleObjScaleSet(work->capsuleObjMdlId, 0.5f, 0.5f, 0.5f);
        VECSubtract(&work->masuPos, &work->playerPos, &dir);
        dirOrig = dir;
        dir.x = dirOrig.z;
        dir.y = 0;
        dir.z = -dirOrig.x;
        if((magTemp=VECMag(&dir)) <= 0) {
            dir.z = 1;
        }
        MTXRotAxisRad(rotMtx, &dir, 0.1f);
        MBCapsuleObjMtxGet(work->capsuleObjMdlId, &objMtx);
        MTXConcat(rotMtx, objMtx, objMtx);
        MBCapsuleObjMtxSet(work->capsuleObjMdlId, &objMtx);
        ThrowEffPosSet(&effPos);
        MBModelPosSet(work->capsuleMdlId2, mdl2Pos.x, mdl2Pos.y, mdl2Pos.z);
        MBModelDispSet(work->capsuleMdlId2, FALSE);
        MBCameraFocusSet(work->capsuleMdlId2);
        pos.x = effPos.x+(((0.5f-frand7f())*100)*0.75f);
        pos.y = effPos.y+(((0.5f-frand7f())*100)*0.75f);
        pos.z = effPos.z+(((0.5f-frand7f())*100)*0.75f);
        ofs.x = ofs.y = ofs.z = 0;
        color = colTbl[MBCapsuleColorNoGet(work->capsuleNo)];
        color.a = (frand7f()*63)+192;
        GlowEffAdd(pos, ofs, ((frand7f()*0.025f)+0.1f)*100, (frand7f()+1)*0.016666668f, 0, 0.025f, color);
        HuPrcVSleep();
    } while(work->time < work->maxTime);
    if(seNo != MSM_SENO_NONE) {
        MBAudFXStop(seNo);
    }
    seNo = MSM_SENO_NONE;
    MBAudFXPlay(MSM_SE_BOARD_27);
    effPos = work->masuPos;
    effPos.y -= 10000;
    ThrowEffPosSet(&effPos);
    MBModelDispSet(work->capsuleMdlId1, FALSE);
    MBModelDispSet(work->capsuleMdlId2, FALSE);
    MBCapsuleObjDispSet(work->capsuleObjMdlId, FALSE);
    color = colTbl[MBCapsuleColorNoGet(work->capsuleNo)];
    RingEffAdd(work->masuPos, ringOfs[0], ringOfs[1], 6, 18, 0, 2, color);
    if(masuP != NULL) {
        DomeEffMdlUpdate(&work->masuPos, &masuP->rot);
    } else {
        DomeEffMdlUpdate(&work->masuPos, NULL);
    }
    work->time = 0;
    do {
        work->time++;
        HuPrcVSleep();
    } while(work->time < 6.0f || ExplodeEffCheck() > 0);
    MBCapsuleMasuSet(work->masuId, work->capsuleNo);
    work->time = 0;
    do {
        work->time++;
        HuPrcVSleep();
    } while(work->time < 84.0f || ExplodeEffCheck() > 0);
    CapsuleThrowWin(work->masuId, work->capsuleNo);
    MBPlayerMotIdleSet(work->playerNo);
    MBPlayerRotateStart(work->playerNo, 0, 15);
    while(!MBPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    MBCameraSkipSet(FALSE);
    MBCameraFocusPlayerSet(work->playerNo);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    if(!noRemoveF) {
        MBPlayerCapsuleUseSet(work->capsuleNo);
        num = MBPlayerCapsuleFind(work->playerNo, work->capsuleNo);
        if(num != -1) {
            MBPlayerCapsuleRemove(work->playerNo, num);
        }
    }
    if(work->jumpMotId != MB_MOT_NONE && !MBKillCheck()) {
        MBPlayerMotionKill(work->playerNo, work->jumpMotId);
    }
    work->jumpMotId = MB_MOT_NONE;
    HuPrcEnd();
}

static void CapsuleThrowDestroy(void)
{
    CAPSULE_THROW_WORK *work = capsuleThrowWork;
    CapsuleThrowEffKill();
    CapsuleColEnd();
    ExplodeEffClose();
    GlowEffClose();
    ThrowEffClose();
    RingEffClose();
    DomeEffClose();
    RayEffClose();
    MBModelKill(work->capsuleMdlId1);
    MBModelKill(work->capsuleMdlId2);
    MBCapsuleObjKill(work->capsuleObjMdlId);
    if(capsuleThrowWork != NULL) {
        HuMemDirectFree(capsuleThrowWork);
    }
    capsuleThrowWork = NULL;
    capsuleThrowProc = NULL;
}

typedef struct CapsuleThrowEffWork_s {
    int mdlId;
    int maxTime;
    BOOL stopF;
    int minNo;
    int no;
    BOOL initF;
    HuVecF startPos;
    HuVecF endPos;
    HuVecF pos;
    float yOfs;
    u32 tick;
    float throwX[3];
    float throwY[3];
    float throwZ[3];
} CAPSULE_THROWEFF_WORK;

typedef struct CapsuleThrowData_s {
    float speed;
    float yOfs;
    float radius;
    int dir;
} CAPSULE_THROW_DATA;

static CAPSULE_THROW_DATA capsuleThrowData[48] = {
    {
        0.5f,
        0,
        0,
        0
    },
    {
        0.2f,
        0,
        0,
        0
    },
    {
        0.8f,
        0,
        0,
        0
    },
    {
        0.5f,
        500,
        0,
        0
    },
    {
        0.2f,
        500,
        0,
        0
    },
    {
        0.8f,
        500,
        0,
        0
    },
    {
        1.0f,
        500,
        0,
        0
    },
    {
        1.2f,
        500,
        0,
        0
    },
    {
        0.2f,
        0,
        300,
        0
    },
    {
        0.2f,
        0,
        -300,
        0
    },
    {
        0,
        0,
        300,
        0
    },
    {
        0,
        0,
        -300,
        0
    },
    {
        0.8f,
        0,
        300,
        0
    },
    {
        0.8f,
        0,
        -300,
        0
    },
    {
        1.0f,
        0,
        300,
        0
    },
    {
        1.0f,
        0,
        -300,
        0
    },
    {
        0.2f,
        0,
        800,
        0
    },
    {
        0.2f,
        0,
        -800,
        0
    },
    {
        0,
        0,
        800,
        0
    },
    {
        0,
        0,
        -800,
        0
    },
    {
        0.8f,
        0,
        800,
        0
    },
    {
        0.8f,
        0,
        -800,
        0
    },
    {
        1.0f,
        0,
        800,
        0
    },
    {
        1.0f,
        0,
        -800,
        0
    },
    {
        0.2f,
        500,
        300,
        0
    },
    {
        0.2f,
        500,
        -300,
        0
    },
    {
        0,
        500,
        300,
        0
    },
    {
        0,
        500,
        -300,
        0
    },
    {
        0.8f,
        500,
        300,
        0
    },
    {
        0.8f,
        500,
        -300,
        0
    },
    {
        1.0f,
        500,
        300,
        0
    },
    {
        1.0f,
        500,
        -300,
        0
    },
    {
        0.2f,
        500,
        800,
        0
    },
    {
        0.2f,
        500,
        -800,
        0
    },
    {
        0,
        500,
        800,
        0
    },
    {
        0,
        500,
        -800,
        0
    },
    {
        0.8f,
        500,
        800,
        0
    },
    {
        0.8f,
        500,
        -800,
        0
    },
    {
        1.0f,
        500,
        800,
        0
    },
    {
        1.0f,
        500,
        -800,
        0
    },
    {
        0.5f,
        500,
        0,
        1
    },
    {
        0.5f,
        500,
        0,
        -1
    },
    {
        0.7f,
        500,
        0,
        1
    },
    {
        0.7f,
        500,
        0,
        -1
    },
    {
        0.3f,
        500,
        0,
        1
    },
    {
        0.3f,
        500,
        0,
        -1
    },
    {
        0.7f,
        500,
        0,
        1
    },
    {
        0.7f,
        500,
        0,
        -1
    },
};

static int capsuleThrowMdl = HU3D_MODELID_NONE;


static void CapsuleThrowEffDraw(HU3DMODEL *modelP, Mtx *mtx);


static void CapsuleThrowEffCreate(float *throwX, float *throwY, float *throwZ, float yOfs, int masuId)
{
    CAPSULE_THROWEFF_WORK *work;
    HU3DMODEL *modelP;
    capsuleThrowMdl = Hu3DHookFuncCreate(CapsuleThrowEffDraw);
    Hu3DModelCameraSet(capsuleThrowMdl, HU3D_CAM0);
    modelP = &Hu3DData[capsuleThrowMdl];
    modelP->hookData = work = HuMemDirectMallocNum(HEAP_MODEL, sizeof(CAPSULE_THROWEFF_WORK), modelP->mallocNo);
    memset(work, 0, sizeof(CAPSULE_THROWEFF_WORK));
    Hu3DModelLayerSet(capsuleThrowMdl, 7);
    work->mdlId = capsuleThrowMdl;
    work->maxTime = 0;
    work->stopF = FALSE;
    work->minNo = 0;
    if(MBBoardNoGet() == 6 && GWPartyFGet() == TRUE && GwPlayer[GwSystem.turnPlayerNo].masuId == 63 && masuId == 55) {
        work->minNo = 40;
    }
    work->no = work->minNo;
    work->initF = FALSE;
    work->startPos.x = throwX[0];
    work->startPos.y = throwY[0];
    work->startPos.z = throwZ[0];
    work->endPos.x = throwX[2];
    work->endPos.y = throwY[2];
    work->endPos.z = throwZ[2];
    work->pos.x = work->pos.y = work->pos.z = 0;
    work->yOfs = yOfs;
    work->tick = 0;
    work->throwX[0] = throwX[0];
    work->throwY[0] = throwY[0];
    work->throwZ[0] = throwZ[0];
    work->throwX[1] = throwX[1];
    work->throwY[1] = throwY[1];
    work->throwZ[1] = throwZ[1];
    work->throwX[2] = throwX[2];
    work->throwY[2] = throwY[2];
    work->throwZ[2] = throwZ[2];
}


static void CapsuleThrowEffDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    CAPSULE_THROWEFF_WORK *work = modelP->hookData;
    int boardNo;
    int i;
    u32 tick;
    HuVecF pos;
    HuVecF dir;
    HuVecF out;
    float mag;
    float t;
    float angle;
    static int tickTbl[MBNO_MAX+1] = {
        160000,
        120000,
        60000,
        90000,
        120000,
        120000,
        90000,
        120000,
        120000,
        120000
    };
    if(work->stopF) {
        return;
    }
    if(!work->initF) {
        VECSubtract(&work->endPos, &work->startPos, &pos);
        VECScale(&pos, &pos, capsuleThrowData[work->no].speed);
        VECAdd(&pos, &work->startPos, &pos);
        if(pos.y < work->startPos.y) {
            pos.y = work->startPos.y;
        } else if(pos.y < work->endPos.y) {
            pos.y = work->endPos.y;
        }
        pos.y += work->yOfs+capsuleThrowData[work->no].yOfs;
        if(0.0f != capsuleThrowData[work->no].radius) {
            VECSubtract(&work->endPos, &work->startPos, &dir);
            VECNormalize(&dir, &dir);
            angle = HuAtan(dir.x, dir.z)+90;
            dir.x = HuSin(angle)*capsuleThrowData[work->no].radius;
            dir.z = HuCos(angle)*capsuleThrowData[work->no].radius;
            pos.x += dir.x;
            pos.z += dir.z;
        } else {
            if(0.0f != capsuleThrowData[work->no].dir) {
                VECSubtract(&work->endPos, &work->startPos, &dir);
                mag = VECMag(&dir);
                VECNormalize(&dir, &dir);
                t = dir.z;
                dir.z = (dir.x*mag)*capsuleThrowData[work->no].dir;
                dir.x = (t*mag)*capsuleThrowData[work->no].dir;
                pos.x += dir.x;
                pos.z += dir.z;
            }
        }
        work->throwX[1] = pos.x;
        work->throwY[1] = pos.y;
        work->throwZ[1] = pos.z;
        CapsuleThrowCameraSet(work->throwX, work->throwY, work->throwZ, 3);
        VECSubtract(&pos, &work->startPos, &dir);
        mag = VECMag(&dir);
        VECSubtract(&work->endPos, &pos, &dir);
        mag += VECMag(&dir);
        work->maxTime = mag/25;
        if(work->maxTime < 45.0f) {
            work->maxTime = 45;
        }
        work->pos = work->startPos;
        work->initF = TRUE;
    }
    work->tick = OSGetTick();
    boardNo = MBBoardNoGet();
    if(boardNo < 0) {
        boardNo = 0;
    } else if(boardNo > MBNO_MAX) {
        boardNo = MBNO_MAX;
    }
    for(i=0; work->initF<(0.5f*work->maxTime); i++) {
        t = work->initF/(0.5f*work->maxTime);
        CapsuleThrowCameraCalc(t, work->throwX, work->throwY, work->throwZ, &out, 3);
        pos.x = out.x;
        pos.y = out.y;
        pos.z = out.z;
        if(CapsuleColCheck(&work->pos, &pos, &dir)) {
           break; 
        }
        work->pos = pos;
        work->initF++;
        tick = OSGetTick();
        if((tick-work->tick) > tickTbl[boardNo]){
            return;
        }
    }
    if(work->initF >= (0.5f*work->maxTime)) {
        work->stopF = TRUE;
        return;
    } else {
        work->initF = FALSE;
        work->no++;
        if(work->no >= 48) {
            work->no = 0;
            VECSubtract(&work->endPos, &work->startPos, &pos);
            VECScale(&pos, &pos, capsuleThrowData[work->no].speed);
            VECAdd(&pos, &work->startPos, &pos);
            if(pos.y < work->startPos.y) {
                pos.y = work->startPos.y;
            } else if(pos.y < work->endPos.y) {
                pos.y = work->endPos.y;
            }
            pos.y += work->yOfs+capsuleThrowData[work->no].yOfs;
            work->throwX[1] = pos.x;
            work->throwY[1] = pos.y;
            work->throwZ[1] = pos.z;
            CapsuleThrowCameraSet(work->throwX, work->throwY, work->throwZ, 3);
            VECSubtract(&pos, &work->startPos, &dir);
            mag = VECMag(&dir);
            VECSubtract(&work->endPos, &pos, &dir);
            mag += VECMag(&dir);
            work->maxTime = mag/25;
            if(work->maxTime < 45.0f) {
                work->maxTime = 45;
            }
            work->stopF = TRUE;
        }
    }
}

static BOOL CapsuleThrowParamSet(HuVecF *pos, int *maxTime)
{
    CAPSULE_THROWEFF_WORK *work;
    HU3DMODEL *modelP;
    if(capsuleThrowMdl == HU3D_MODELID_NONE) {
        return FALSE;
    }
    modelP = &Hu3DData[capsuleThrowMdl];
    work = modelP->hookData;
    if(!work->stopF) {
        return FALSE;
    } else {
        pos->x = work->throwX[1];
        pos->y = work->throwY[1];
        pos->z = work->throwZ[1];
        *maxTime = work->maxTime;
        return TRUE;
    }
}

static void CapsuleThrowEffKill(void)
{
    if(capsuleThrowMdl != HU3D_MODELID_NONE) {
        Hu3DModelKill(capsuleThrowMdl);
    }
    capsuleThrowMdl = HU3D_MODELID_NONE;
}

static void CapsuleThrowAutoExec(void);
static void CapsuleThrowAutoKill(void);

void MBCapsuleThrowAutoExec(int masuId, int capsuleNo, HuVecF *posA, HuVecF *posB, HuVecF *masuPos)
{
    CAPSULE_THROW_WORK *work;
    capsuleNo &= 0xFF;
    if(capsuleThrowWork == NULL) {
        capsuleThrowWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULE_THROW_WORK), HU_MEMNUM_OVL);
    }
    capsuleThrowWork == NULL;
    memset(capsuleThrowWork, 0, sizeof(CAPSULE_THROW_WORK));
    work = capsuleThrowWork;
    work->playerNo = -1;
    work->capsuleNo = capsuleNo;
    work->masuId = masuId;
    work->capsuleObjMdlId = MB_MODEL_NONE;
    work->time = 0;
    work->maxTime = 19;
    work->masuPos = *masuPos;
    work->playerPos.x = work->masuPos.x;
    work->playerPos.y = work->masuPos.y+1000;
    work->playerPos.z = work->masuPos.z;
    work->upPos.x = work->masuPos.x;
    work->upPos.y = work->masuPos.y+500;
    work->upPos.z = work->masuPos.z;
    capsuleThrowProc = MBPrcCreate(CapsuleThrowAutoExec, 8196, 24576);
    MBPrcDestructorSet(capsuleThrowProc, CapsuleThrowAutoKill);
    do {
        HuPrcVSleep();
    } while(!MBCapsuleThrowAutoCheck());
}

BOOL MBCapsuleThrowAutoCheck(void)
{
    if(capsuleThrowProc) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void ApplyBezier(float t, float *a, float *b, float *c, float *out);

static void CapsuleThrowAutoExec(void)
{
    CAPSULE_THROW_WORK *work = capsuleThrowWork;
    MASU *masuP;
    int seNo;
    
    float time;
    
    Mtx rotMtx;
    Mtx objMtx;
    HuVecF effPos;
    HuVecF dir;
    HuVecF dirOrig;
    HuVecF glowPos;
    HuVecF glowOfs;
    GXColor color;
    float magTemp;
    static HuVecF ringOfs[2] = { { -90, 0, 0 }, { 0, 10, 60 } };
    static GXColor colTbl[CAPSULE_COLOR_MAX] = {
        { 192, 255, 192, 255 },
        { 255, 255, 192, 255 },
        { 255, 192, 192, 255 },
        { 192, 255, 255, 255 },
    };
    
    ExplodeEffInit();
    GlowEffInit();
    ThrowEffInit(work->capsuleNo);
    RingEffInit();
    RingEffNumSet(15);
    DomeEffInit();
    RayEffInit();
    work->capsuleObjMdlId = MBCapsuleObjCreate(work->capsuleNo, FALSE);
    MBCapsuleObjPosSetV(work->capsuleObjMdlId, &work->playerPos);
    MBCapsuleObjLayerSet(work->capsuleObjMdlId, 2);
    seNo = MBAudFXPlay(MSM_SE_BOARD_26);
    ThrowEffUpdate(&work->playerPos, work->capsuleNo);
    do  {
        work->time++;
        time = (float)work->time/(float)work->maxTime;
        ApplyBezier((time*0.5f)+0.5f, (float *)&work->playerPos, (float *)&work->upPos, (float *)&work->masuPos, (float *)&effPos);
        MBCapsuleObjPosSet(work->capsuleObjMdlId, effPos.x, effPos.y, effPos.z);
        MBCapsuleObjScaleSet(work->capsuleObjMdlId, 0.5f, 0.5f, 0.5f);
        VECSubtract(&work->masuPos, &work->playerPos, &dir);
        dirOrig = dir;
        dir.x = dirOrig.z;
        dir.y = 0;
        dir.z = -dirOrig.x;
        if((magTemp=VECMag(&dir)) <= 0) {
            dir.z = 1;
        }
        MTXRotAxisRad(rotMtx, &dir, 0.1f);
        MBCapsuleObjMtxGet(work->capsuleObjMdlId, &objMtx);
        MTXConcat(rotMtx, objMtx, objMtx);
        MBCapsuleObjMtxSet(work->capsuleObjMdlId, &objMtx);
        ThrowEffPosSet(&effPos);
        glowPos.x = effPos.x+(((0.5f-frand7f())*100)*0.75f);
        glowPos.y = effPos.y+(((0.5f-frand7f())*100)*0.75f);
        glowPos.z = effPos.z+(((0.5f-frand7f())*100)*0.75f);
        glowOfs.x = glowOfs.y = glowOfs.z = 0;
        color = colTbl[MBCapsuleColorNoGet(work->capsuleNo)];
        color.a = (frand7f()*63)+192;
        GlowEffAdd(glowPos, glowOfs, ((frand7f()*0.025f)+0.1f)*100, (frand7f()+1)*0.016666668f, 0, 0.025f, color);
        HuPrcVSleep();
    } while(work->time < work->maxTime);
    if(seNo != MSM_SENO_NONE){
        MBAudFXStop(seNo);
    }
    MBAudFXPlay(MSM_SE_BOARD_27);
    effPos = work->masuPos;
    effPos.y -= 50;
    ThrowEffPosSet(&effPos);
    MBCapsuleObjDispSet(work->capsuleObjMdlId, FALSE);
    color = colTbl[MBCapsuleColorNoGet(work->capsuleNo)];
    RingEffAdd(work->masuPos, ringOfs[0], ringOfs[1], 6, 18, 0, 2, color);
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, work->masuId);
    if(masuP != NULL) {
        DomeEffMdlUpdate(&work->masuPos, &masuP->rot);
    } else {
        DomeEffMdlUpdate(&work->masuPos, NULL);
    }
    work->time = 0;
    do {
        work->time++;
        HuPrcVSleep();
    } while(work->time < 6.0f || ExplodeEffCheck() > 0);
    MBCapsuleMasuSet(work->masuId, work->capsuleNo);
    work->time = 0;
    do {
        work->time++;
        HuPrcVSleep();
    } while(work->time < 84.0f || ExplodeEffCheck() > 0);
    HuPrcEnd();
}

static void CapsuleThrowAutoKill(void)
{
    CAPSULE_THROW_WORK *work = capsuleThrowWork;
    ExplodeEffClose();
    GlowEffClose();
    ThrowEffClose();
    RingEffClose();
    DomeEffClose();
    RayEffClose();
    MBCapsuleObjKill(work->capsuleObjMdlId);
    if(capsuleThrowWork != NULL) {
        HuMemDirectFree(capsuleThrowWork);
    }
    capsuleThrowWork = NULL;
    capsuleThrowProc = NULL;
}

static void CapsulePosSelExec(void);
static void CapsulePosAutoSelExec(void);
static void CapsulePosSelKill(void);

void MBCapsulePosSelStart(int playerNo, int capsuleNo)
{
    CAPSULE_POSSEL_WORK *work;
    capsuleNo &= 0xFF;
    switch(capsuleNo) {
        case CAPSULE_HONE:
        case CAPSULE_KOOPA:
        case CAPSULE_MIRACLE:
        case CAPSULE_DONKEY:
        case CAPSULE_VS:
            capsuleSelResult = -2;
            capsulePosSelProc = NULL;
            return;
    }
    if(capsulePosSelWork == NULL) {
        capsulePosSelWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULE_POSSEL_WORK), HU_MEMNUM_OVL);
    }
    capsulePosSelWork == NULL;
    memset(capsulePosSelWork, 0, sizeof(CAPSULE_POSSEL_WORK));
    work = capsulePosSelWork;
    work->playerNo = playerNo;
    work->masuId = MASU_NULL;
    work->capsuleNo = capsuleNo;
    capsuleSelResult = MASU_NULL;
    comCapsuleEndF[playerNo] = FALSE;
    MBCameraSkipSet(TRUE);
    if(!GwPlayer[playerNo].comF && capsuleSelMasu == MASU_NULL) {
        capsulePosSelProc = MBPrcCreate(CapsulePosSelExec, 8196, 24576);
    } else {
        capsulePosSelProc = MBPrcCreate(CapsulePosAutoSelExec, 8196, 24576);
    }
    MBPrcDestructorSet(capsulePosSelProc, CapsulePosSelKill);
}

BOOL MBCapsulePosSelCheck(void)
{
    if(capsulePosSelProc) {
        return FALSE;
    } else {
        return TRUE;
    }
}

int MBCapsulePosSelResultGet(void)
{
    return capsuleSelResult;
}

static void CapsuleMasuListGet(s16 *tbl, s16 masuId, s16 maxNum);

void MBCapsulePosSetExec(void)
{
    s16 *tbl;
    int i;
    int j;
    int num;
    int max;
    max = 0;
    tbl = HuMemDirectMallocNum(HEAP_HEAP, MASU_MAX*sizeof(s16), HU_MEMNUM_OVL);
    for(i=1; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        memset(tbl, 0, MASU_MAX*sizeof(s16));
        listStartF = TRUE;
        CapsuleMasuListGet(tbl, i, 10);
        for(j=0, num=1; j<MASU_MAX; j++) {
            if(tbl[j] & 0x1) {
                num++;
            }
        }
        if(num > max) {
            max = num;
        }
    }
    HuMemDirectFree(tbl);
}

static int cursorSprFile = BOARD_ANM_cursor;

static void CapsuleYajiCreate(void);
static void CapsuleYajiRotSet(int masuId, float rot);
static int CapsuleMasuMesCreate(int mes);
static float CameraXZAngleGet(float angle);
static float AngleSumWrap(float start, float end);
static void CapsuleYajiGrowSet(void);
static void CapsuleYajiKill(void);

static void CapsulePosSelExec(void)
{
    CAPSULE_POSSEL_WORK *work; //r29
    int i; //r28
    int masuId; //r27
    s16 *tbl; //r26
    int num; //r25
    int capsuleMes; //r24
    int linkNum; //r23
    s16 angle; //r22
    MASU *masuP; //r21
    int linkCount; //r20
    BOOL enablePlaceF; //r19
    GWPLAYER *playerP;//r18
    int padNo; //r17
    
    float t; //f30
    float scale; //f29
    float angleEnd; //f28
    float angleNext; //f27
    float angleMax; //f26
    float angleStk; //f25
    float zoom; //f24
    
    float yajiAngle[10]; //sp+0x120
    float masuAngle[10]; //sp+0xF8
    s16 angleTbl[10]; //sp+0xE4
    s16 linkTbl[10]; //sp+0xD0
    HuVecF cursorPos; //sp+0xC4
    HuVecF playerPos; //sp+0xB8
    HuVecF masuPos2; //sp+0xAC
    HuVecF mdl2Pos; //sp+0xA0
    HuVecF cameraOfs; //sp+0x94
    HuVecF stkDir; //sp+0x88
    HuVecF masuDir; //sp+0x7C
    HuVecF masuPos; //sp+0x70
    HuVec2F winPos; //sp+0x68
    int prevMasuId; //sp+0x48
    int masuMax; //sp+0x44
    int validLinkNum; //sp+0x40
    int maxTime; //sp+0x3C
    BOOL cursorDispF; //sp+0x38
    int capsuleMesPrev; //sp+0x34
    BOOL modelDispF; //sp+0x30
    int moveNum; //sp+0x2C
    MASU *masuNextP; //sp+0x28
    int masuNum; //sp+0x24
    MBCAMERA *cameraP; //sp+0x20
    int lastMasuId; //sp+0x1C
    s16 dir; //sp+0xA
    
    work = capsulePosSelWork;
    moveNum = -1;
    memset(angleTbl, 0, sizeof(angleTbl));
    memset(linkTbl, 0, sizeof(linkTbl));
    playerP = GWPlayerGet(work->playerNo);
    masuNum = MBMasuNumGet(MASU_LAYER_DEFAULT);
    padNo = GwPlayer[work->playerNo].padNo;
    masuId = prevMasuId = playerP->masuId;
    masuMax = 10;
    linkCount = 0;
    validLinkNum = 0;
    dir = 1;
    work->masuIdTbl = tbl = HuMemDirectMallocNum(HEAP_HEAP, MASU_MAX*sizeof(s16), HU_MEMNUM_OVL);
    memset(tbl, 0, MASU_MAX*sizeof(s16));
    listStartF = TRUE;
    CapsuleMasuListGet(tbl, masuId, 10);
    if(MBCapsuleNoUseCheck(work->capsuleNo) && MBPlayerCoinGet(work->playerNo) >= MBCapsuleCostGet(work->capsuleNo)) {
        tbl[masuId] = 1;
    } else {
        tbl[masuId] = 0x8000;
    }
    work->cursorSprId = espEntry(cursorSprFile, 2000, 0);
    espDispOn(work->cursorSprId);
    espDrawNoSet(work->cursorSprId, 0);
    espAttrSet(work->cursorSprId, HUSPR_ATTR_NOANIM);
    espPosSet(work->cursorSprId, 0, 0);
    espBankSet(work->cursorSprId, 0);
    work->batsuSprId = espEntry(CAPSULE_ANM_batsu, 1900, 0);
    espDispOff(work->batsuSprId);
    espDrawNoSet(work->batsuSprId, 0);
    espAttrSet(work->batsuSprId, HUSPR_ATTR_NOANIM);
    espPosSet(work->batsuSprId, 0, 0);
    espBankSet(work->batsuSprId, 0);
    work->capsuleMdlId1 = capsuleMdl = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, TRUE);
    MBModelDispSet(work->capsuleMdlId1, FALSE);
    MBPlayerPosGet(work->playerNo, &playerPos);
    MBModelPosSet(work->capsuleMdlId1, playerPos.x, playerPos.y+250, playerPos.z);
    MBModelScaleSet(work->capsuleMdlId1, 1.2f, 1.2f, 1.2f);
    MBModelLayerSet(work->capsuleMdlId1, 2);
    MBModelAttrSet(work->capsuleMdlId1, HU3D_MOTATTR_LOOP);
    cameraP = MBCameraGet();
    zoom = cameraP->zoom;
    cameraOfs = cameraP->offset;
    work->capsuleMdlId2 = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, TRUE);
    MBModelDispSet(work->capsuleMdlId2, FALSE);
    MBPlayerPosGet(work->playerNo, &playerPos);
    MBModelPosSetV(work->capsuleMdlId2, &playerPos);
    MBCameraModelViewSet(work->capsuleMdlId2, NULL, NULL, 3200, -1, 15);
    modelDispF = TRUE;
    work->capsuleSelWinId = MBWinCreateHelp(MESS_CAPSULE_EX99_CAPSULESEL);
    MBTopWinPosGet(&winPos);
    MBTopWinPosSet(winPos.x, winPos.y-32);
    work->capsuleMasuWinId = MBWIN_NONE;
    capsuleMes = -1;
    capsuleMesPrev = capsuleMes;
    CapsuleYajiCreate();
    while(1) {
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
        MBMasuEffClear();
        for(i=1; i<=masuNum; i++) {
            if(tbl[i] & 0x1) {
                MBMasuEffSet(i, MASU_EFF_CAPSULESEL);
            }
        }
        if(tbl[masuId] & 0x1) {
            MBMasuEffSet(masuId, MASU_EFF_CAPSULE);
            enablePlaceF = TRUE;
        } else {
            enablePlaceF = FALSE;
        }
        for(i=linkNum=0; i<masuP->linkNum; i++) {
            if(tbl[masuP->linkTbl[i]]) {
                linkTbl[linkNum++] = masuP->linkTbl[i];
            }
        }
        linkCount = linkNum;
        linkNum += MBMasuLinkTblGet(MASU_LAYER_DEFAULT, masuId, &linkTbl[linkNum]);
        for(i=0, validLinkNum=0; i<linkNum; i++) {
            if(i < linkCount) {
                continue;
            }
            if(tbl[linkTbl[i]] == 0) {
                linkTbl[i] = -1;
            } else {
                validLinkNum++;
            }
        }
        if(!enablePlaceF && linkCount == 1 && validLinkNum == 1 && masuId != playerP->masuId) {
            cursorDispF = TRUE;
        } else {
            cursorDispF = FALSE;
        }
        if(!cursorDispF) {
            if(masuId == playerP->masuId) {
                if(!MBCapsuleNoUseCheck(work->capsuleNo)) {
                    capsuleMes = 3;
                } else {
                    if(MBPlayerCoinGet(work->playerNo) < MBCapsuleCostGet(work->capsuleNo)) {
                        capsuleMes = 4;
                    } else {
                        capsuleMes = 1;
                    }
                }
            } else {
                capsuleMes = 0;
            }
            if(capsuleMes != capsuleMesPrev) {
                if(work->capsuleMasuWinId != MBWIN_NONE) {
                    MBWinKill(work->capsuleMasuWinId);
                }
                work->capsuleMasuWinId = MBWIN_NONE;
                work->capsuleMasuWinId = CapsuleMasuMesCreate(capsuleMes);
                capsuleMesPrev = capsuleMes;
            }
        }
        for(i=0; i<linkNum; i++) {
            if(linkTbl[i] != MASU_NULL) {
                masuNextP = MBMasuGet(MASU_LAYER_DEFAULT, linkTbl[i]);
                if(!masuNextP) {
                    linkTbl[i] = -1;
                } else {
                    VECSubtract(&masuNextP->pos, &masuPos, &masuDir);
                    angleEnd = 90-HuAtan(masuDir.z, masuDir.x);
                    OSf32tos16(&angleEnd, &angle);
                    if(angle < 0) {
                        angle += 360;
                    }
                    if(angle > 360) {
                        angle -= 360;
                    }
                    yajiAngle[i] = angle;
                    masuAngle[i] = CameraXZAngleGet(angle);
                    angle = ((angle+22)/45)*45;
                    angleTbl[i] = angle;
                }
            }
        }
        if(cursorDispF) {
            espDispOn(work->cursorSprId);
        } else if(!enablePlaceF && linkCount >= 2) {
            for(i=0; i<linkNum; i++) {
                if(linkTbl[i] != MASU_NULL) {
                    CapsuleYajiRotSet(masuId, yajiAngle[i]);
                }
            }
            espDispOff(work->cursorSprId);
        } else {
            espDispOn(work->cursorSprId);
            if(!enablePlaceF) {
                espDispOn(work->batsuSprId);
            }
        }
        if(enablePlaceF) {
            MBAudFXPlay(MSM_SE_BOARD_23);
        }
        cursorMove:
        Hu3D3Dto2D(&masuPos, HU3D_CAM0, &cursorPos);
        cursorPos.x += -32;
        cursorPos.y += -16;
        espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
        espPosSet(work->batsuSprId, cursorPos.x, cursorPos.y);
        stkDir.x = MBPadStkXGet(padNo);
        stkDir.y = 0;
        stkDir.z = MBPadStkYGet(padNo);
        if(modelDispF) {
            MBModelDispSet(work->capsuleMdlId1, TRUE);
            num = 0;
            do {
                Hu3D3Dto2D(&masuPos, HU3D_CAM0, &cursorPos);
                cursorPos.x += -32;
                cursorPos.y += -16;
                espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
                espPosSet(work->batsuSprId, cursorPos.x, cursorPos.y);
                num++;
                t = num/18.0f;
                if(t > 1) {
                    t = 1;
                }
                MBPlayerPosGet(work->playerNo, &playerPos);
                playerPos.y += (150*HuSin(t*90))+100;
                MBModelPosSetV(work->capsuleMdlId1, &playerPos);
                scale = 1.2f*HuSin(t*90);
                MBModelScaleSet(work->capsuleMdlId1, scale, scale, scale);
                HuPrcVSleep();
            } while(!MBCameraMotionCheck() || t < 1);
            modelDispF = FALSE;
        }
        if(cursorDispF) {
            if(dir > 0) {
                for(i=0; i<linkCount; i++) {
                    if(linkTbl[i] != -1) {
                        masuId = linkTbl[i];
                        break;
                    }
                }
                moveNum = 0;
            } else {
                for(i=linkCount; i<linkNum; i++) {
                    if(linkTbl[i] != -1) {
                        masuId = linkTbl[i];
                        break;
                    }
                }
            }
        } else {
            if(HuPadBtnDown[padNo] & PAD_BUTTON_A) {
                if(enablePlaceF) {
                    MBAudFXPlay(MSM_SE_BOARD_24);
                    capsuleSelResult = masuId;
                    goto exit;
                }
            } else if(HuPadBtnDown[padNo] & PAD_BUTTON_B) {
                capsuleSelResult = MASU_NULL;
                goto exit;
            } else {
                if(fabs(stkDir.x) >= 8 || fabs(stkDir.z) >= 8) {
                    angleStk = HuAtan(MBPadStkXGet(padNo), -MBPadStkYGet(padNo));
                    angleNext = 180;
                    angleMax = 180;
                    lastMasuId = -1;
                    for(i=0; i<linkNum; i++) {
                        if(linkTbl[i] == MASU_NULL) {
                            continue;
                        }
                        angleNext = AngleSumWrap(masuAngle[i], angleStk);
                        if(fabs(angleNext) < 45 && fabs(angleNext) < angleMax) {
                            lastMasuId = linkTbl[i];
                            (void)angleMax;
                        }
                    }
                    if(lastMasuId != -1) {
                        masuId = lastMasuId;
                        moveNum = 0;
                    }
                    if(linkCount <= 0 && masuId == prevMasuId) {
                        masuNextP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
                        for(i=0; i<masuNextP->linkNum; i++) {
                            if((MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & MBBranchFlagGet()) == 0) {
                                MBMasuPosGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i], &masuDir);
                                VECSubtract(&masuDir, &masuPos, &masuDir);
                                angleEnd = CameraXZAngleGet(HuAtan(masuDir.x, masuDir.z));
                                if(fabs(AngleSumWrap(angleEnd, angleStk)) <= 45) {
                                    capsuleMes = 2;
                                    if(capsuleMes != capsuleMesPrev) {
                                        if(work->capsuleMasuWinId != MBWIN_NONE) {
                                            MBWinKill(work->capsuleMasuWinId);
                                        }
                                        work->capsuleMasuWinId = MBWIN_NONE;
                                        work->capsuleMasuWinId = CapsuleMasuMesCreate(capsuleMes);
                                        capsuleMesPrev = capsuleMes;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        HuPrcVSleep();
        if(masuId == prevMasuId) {
            goto cursorMove;
        }
        CapsuleYajiGrowSet();
        espDispOff(work->batsuSprId);
        playerPos = masuPos;
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos2);
        VECSubtract(&masuPos2, &playerPos, &mdl2Pos);
        t = VECMag(&mdl2Pos);
        for(maxTime=t/20, num=1; num<maxTime; num++) {
            t = (float)num/maxTime;
            VECSubtract(&masuPos2, &playerPos, &mdl2Pos);
            VECScale(&mdl2Pos, &mdl2Pos, t);
            VECAdd(&playerPos, &mdl2Pos, &mdl2Pos);
            Hu3D3Dto2D(&mdl2Pos, HU3D_CAM0, &cursorPos);
            cursorPos.x += -32;
            cursorPos.y += -16;
            espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
            espPosSet(work->batsuSprId, cursorPos.x, cursorPos.y);
            MBModelPosSet(work->capsuleMdlId2, mdl2Pos.x, mdl2Pos.y, mdl2Pos.z);
            HuPrcVSleep();
        }
        for(num=0; num<linkCount; num++) {
            if(masuId == linkTbl[num]) {
                break;
            }
        }
        if(num < linkCount) {
            if(CapsuleMasuVisitCheck(masuId)) {
                masuMax--;
            }
            dir = 1;
        } else {
            if(CapsuleMasuVisitCheck(prevMasuId)) {
                masuMax++;
            }
            dir = -1;
        }
        prevMasuId = masuId;
    }
    exit:
    HuMemDirectFree(tbl);
    work->masuIdTbl = NULL;
    espDispOff(work->cursorSprId);
    espDispOff(work->batsuSprId);
    if(work->capsuleSelWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleSelWinId);
    }
    work->capsuleSelWinId = MBWIN_NONE;
    if(work->capsuleMasuWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleMasuWinId);
    }
    work->capsuleMasuWinId = MBWIN_NONE;
    if(capsuleSelResult != MASU_NULL) {
        WipeColorSet(255, 255, 255);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL)|| !MBTutorialExitFlagGet()) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        }
        WipeWait();
    } else {
        if(work->capsuleMdlId1 != MB_MODEL_NONE) {
            MBModelKill(work->capsuleMdlId1);
        }
        work->capsuleMdlId1 = MB_MODEL_NONE;
        capsuleMdl = MB_MODEL_NONE;
    }
    MBCameraSkipSet(FALSE);
    //TODO: Missing move in US Version
    MBCameraModelViewSet(MBPlayerModelGet(work->playerNo), NULL, &cameraOfs, zoom, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBMasuEffClear();
    
    HuPrcEnd();
}
static void CapsuleMasuPathGet(s16 *pathTbl, s16 *tbl, s16 masuId, s16 masuLast, s16 startNum);

static void CapsulePosAutoSelExec(void)
{
    CAPSULE_POSSEL_WORK *work; //r31
    s16 *pathTbl; //r30
    s16 num; //r29
    s16 i; //r28
    int masuId; //r27
    s16 *tbl; //r26
    s16 masuLast; //r25
    GWPLAYER *playerP; //r24
    int time; //r23
    int capsuleMes; //r22
    s16 selPos; //r21
    int j; //r20
    MBCAMERA *cameraP; //r19
    BOOL sePlayF; //r18
    int maxTime; //r17
    
    float t; //f31
    float scale; //f30
    float zoom; //f29
    
    s16 angleTbl[10]; //sp+0xA4
    s16 linkTbl[10]; //sp+0x90
    HuVecF cursorPos; //sp+0x84
    HuVecF playerPos; //sp+0x78
    HuVecF masuPos2; //sp+0x6C
    HuVecF mdl2Pos; //sp+0x60
    HuVecF cameraOfs; //sp+0x54
    HuVecF masuPos; //sp+0x48
    HuVec2F winPos; //sp+0x40
    int prevMasuId; //sp+0x3C
    int masuMax; //sp+0x38
    int linkCount; //sp+0x34
    int validLinkNum; //sp+0x30
    int capsuleMesPrev; //sp+0x2C
    BOOL modelDispF; //sp+0x28
    int moveNum; //sp+0x24
    MASU *masuP; //sp+0x20
    int masuNum; //sp+0x1C
    int padNo; //sp+0x18
    s16 dir; //sp+0x8
    
    work = capsulePosSelWork;
    moveNum = -1;
    memset(angleTbl, 0, sizeof(angleTbl));
    memset(linkTbl, 0, sizeof(linkTbl));
    playerP = GWPlayerGet(work->playerNo);
    masuNum = MBMasuNumGet(MASU_LAYER_DEFAULT);
    padNo = GwPlayer[work->playerNo].padNo;
    masuId = prevMasuId = playerP->masuId;
    masuMax = 10;
    linkCount = 0;
    validLinkNum = 0;
    dir = 1;
    work->masuIdTbl = tbl = HuMemDirectMallocNum(HEAP_HEAP, MASU_MAX*sizeof(s16), HU_MEMNUM_OVL);
    memset(tbl, 0, MASU_MAX*sizeof(s16));
    listStartF = TRUE;
    CapsuleMasuListGet(tbl, masuId, 10);
    if(MBCapsuleNoUseCheck(work->capsuleNo) && MBPlayerCoinGet(work->playerNo) >= MBCapsuleCostGet(work->capsuleNo)) {
        tbl[masuId] = 1;
    } else {
        tbl[masuId] = 0x8000;
    }
    if(MBCapsulePosAutoSelCheck(work->playerNo, work->capsuleNo)) {
        masuLast = masuId;
    } else {
        work->capsuleMasuTbl = pathTbl = HuMemDirectMallocNum(HEAP_HEAP, MASU_MAX*sizeof(s16), HU_MEMNUM_OVL);
        memset(pathTbl, 0, MASU_MAX*sizeof(s16));
        for(i=1, num=0; i<MASU_MAX; i++) {
            if(i != masuId && (tbl[i] & 0x1) && MBMasuCapsuleGet(MASU_LAYER_DEFAULT, i) == CAPSULE_NULL) {
                pathTbl[num] = i;
                num++;
            }
        }
        if(num <= 0) {
            for(i=1, num=0; i<MASU_MAX; i++) {
                if(i != masuId && (tbl[i] & 0x1)) {
                    pathTbl[num] = i;
                    num++;
                }
            }
        }
        if(num <= 0) {
            masuLast = MASU_NULL;
        } else if(num <= 1) {
            masuLast = pathTbl[0];
        } else {
            masuLast = pathTbl[(frand() & 0x7FFF) % num];
        }
        HuMemDirectFree(work->capsuleMasuTbl);
        work->capsuleMasuTbl = NULL;
    }
    
    if(capsuleSelMasu != MASU_NULL) {
        tbl[capsuleSelMasu] = 1;
        masuLast = capsuleSelMasu;
    }
    work->cursorSprId = espEntry(cursorSprFile, 2000, 0);
    espDispOn(work->cursorSprId);
    espDrawNoSet(work->cursorSprId, 0);
    espAttrSet(work->cursorSprId, HUSPR_ATTR_NOANIM);
    espPosSet(work->cursorSprId, 0, 0);
    espBankSet(work->cursorSprId, 0);
    work->batsuSprId = espEntry(CAPSULE_ANM_batsu, 1900, 0);
    espDispOff(work->batsuSprId);
    espDrawNoSet(work->batsuSprId, 0);
    espAttrSet(work->batsuSprId, HUSPR_ATTR_NOANIM);
    espPosSet(work->batsuSprId, 0, 0);
    espBankSet(work->batsuSprId, 0);
    work->capsuleMdlId1 = capsuleMdl = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, TRUE);
    MBModelDispSet(work->capsuleMdlId1, TRUE);
    MBPlayerPosGet(work->playerNo, &playerPos);
    MBModelPosSet(work->capsuleMdlId1, playerPos.x, playerPos.y+250, playerPos.z);
    MBModelScaleSet(work->capsuleMdlId1, 1.2f, 1.2f, 1.2f);
    MBModelLayerSet(work->capsuleMdlId1, 2);
    MBModelAttrSet(work->capsuleMdlId1, HU3D_MOTATTR_LOOP);
    cameraP = MBCameraGet();
    zoom = cameraP->zoom;
    cameraOfs = cameraP->offset;
    work->capsuleMdlId2 = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, TRUE);
    MBModelDispSet(work->capsuleMdlId2, FALSE);
    MBPlayerPosGet(work->playerNo, &playerPos);
    MBModelPosSetV(work->capsuleMdlId2, &playerPos);
    MBCameraModelViewSet(work->capsuleMdlId2, NULL, NULL, 3200, -1, 15);
    modelDispF = TRUE;
    
    if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        work->capsuleSelWinId = MBWinCreateHelp(MESS_CAPSULE_EX99_CAPSULESEL);
        MBTopWinPosGet(&winPos);
        MBTopWinPosSet(winPos.x, winPos.y-32);
    } else {
        work->capsuleSelWinId = MBWIN_NONE;
    }
    work->capsuleMasuWinId = MBWIN_NONE;
    capsuleMes = -1;
    capsuleMesPrev = capsuleMes;
    CapsuleYajiCreate();
    work->masuIdTblPath = pathTbl = HuMemDirectMallocNum(HEAP_HEAP, MASU_MAX*sizeof(s16), HU_MEMNUM_OVL);
    memset(pathTbl, 0, MASU_MAX*sizeof(s16));
    if(masuLast != playerP->masuId && masuLast != MASU_NULL) {
        capsuleMasuEndF = FALSE;
        CapsuleMasuPathGet(pathTbl, tbl, masuId, masuLast, 0);
        for(num=0; num<MASU_MAX; num++) {
            if(masuLast == pathTbl[num]) {
                break;
            }
        }
        if(num < MASU_MAX) {
            pathTbl[num+1] = masuLast;
            pathTbl[num+2] = masuLast;
            num += 2;
        } else {
            comCapsuleEndF[work->playerNo] = TRUE;
            capsuleSelResult = masuId = MASU_NULL;
            goto done;
        }
    } else {
        pathTbl[0] = masuId;
        pathTbl[1] = playerP->masuId;
        pathTbl[2] = playerP->masuId;
        num = 2;
    }
    for(selPos=1; selPos<num; selPos++) {
        if(masuLast == MASU_NULL) {
            comCapsuleEndF[work->playerNo] = TRUE;
            capsuleSelResult = masuId = MASU_NULL;
            goto done;
        }
        masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
        MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
        MBMasuEffClear();
        for(j=1; j<=masuNum; j++) {
            if(tbl[j] & 0x1) {
                MBMasuEffSet(j, MASU_EFF_CAPSULESEL);
            }
        }
        if(tbl[masuId] & 0x1) {
            MBMasuEffSet(masuId, MASU_EFF_CAPSULE);
            sePlayF = TRUE;
        } else {
            sePlayF = FALSE;
        }
        espDispOn(work->cursorSprId);
        espDispOff(work->batsuSprId);
        if(modelDispF) {
            MBModelDispSet(work->capsuleMdlId1, TRUE);
            time = 0;
            do {
                Hu3D3Dto2D(&masuPos, HU3D_CAM0, &cursorPos);
                cursorPos.x += -32;
                cursorPos.y += -16;
                espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
                espPosSet(work->batsuSprId, cursorPos.x, cursorPos.y);
                time++;
                t = time/18.0f;
                if(t > 1) {
                    t = 1;
                }
                MBPlayerPosGet(work->playerNo, &playerPos);
                playerPos.y += (150*HuSin(t*90))+100;
                MBModelPosSetV(work->capsuleMdlId1, &playerPos);
                scale = 1.2f*HuSin(t*90);
                MBModelScaleSet(work->capsuleMdlId1, scale, scale, scale);
                HuPrcVSleep();
            } while(!MBCameraMotionCheck() || t < 1);
            modelDispF = FALSE;
        }
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
            if(masuId == playerP->masuId) {
                if(!MBCapsuleNoUseCheck(work->capsuleNo)) {
                    capsuleMes = 3;
                } else {
                    if(MBPlayerCoinGet(work->playerNo) < MBCapsuleCostGet(work->capsuleNo)) {
                        capsuleMes = 4;
                    } else {
                        capsuleMes = 1;
                    }
                }
            } else {
                capsuleMes = 0;
            }
            if(capsuleMes != capsuleMesPrev) {
                if(work->capsuleMasuWinId != MBWIN_NONE) {
                    MBWinKill(work->capsuleMasuWinId);
                }
                work->capsuleMasuWinId = MBWIN_NONE;
                work->capsuleMasuWinId = CapsuleMasuMesCreate(capsuleMes);
                capsuleMesPrev = capsuleMes;
            }
        }
        if(selPos == 0) {
            HuPrcSleep((frand7f()*60.0f)*0.3f);
        }
        if(masuId == masuLast) {
            if(sePlayF) {
                MBAudFXPlay(MSM_SE_BOARD_23);
            }
            for(i=0; i<15.0 || !MBCameraMotionCheck(); i++) {
                MBMasuPosGet(MASU_LAYER_DEFAULT, masuLast, &mdl2Pos);
                MBModelPosSetV(work->capsuleMdlId2, &mdl2Pos);
                Hu3D3Dto2D(&mdl2Pos, HU3D_CAM0, &cursorPos);
                cursorPos.x += -32;
                cursorPos.y += -16;
                espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
                HuPrcVSleep();
            }
            MBAudFXPlay(MSM_SE_BOARD_23);
            capsuleSelResult = masuId;
            goto done;
        } else {
            if(sePlayF) {
                MBAudFXPlay(MSM_SE_BOARD_23);
            }
            masuId = pathTbl[selPos];
            HuPrcVSleep();
            CapsuleYajiGrowSet();
            espDispOff(work->batsuSprId);
            playerPos = masuPos;
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos2);
            VECSubtract(&masuPos2, &playerPos, &mdl2Pos);
            t = VECMag(&mdl2Pos);
            for(maxTime=t/20, time=1; time<maxTime; time++) {
                t = (float)time/maxTime;
                VECSubtract(&masuPos2, &playerPos, &mdl2Pos);
                VECScale(&mdl2Pos, &mdl2Pos, t);
                VECAdd(&playerPos, &mdl2Pos, &mdl2Pos);
                Hu3D3Dto2D(&mdl2Pos, HU3D_CAM0, &cursorPos);
                cursorPos.x += -32;
                cursorPos.y += -16;
                espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
                espPosSet(work->batsuSprId, cursorPos.x, cursorPos.y);
                MBModelPosSet(work->capsuleMdlId2, mdl2Pos.x, mdl2Pos.y, mdl2Pos.z);
                HuPrcVSleep();
            }
            prevMasuId = masuId;
        }
    }
    if(selPos >= num) {
        capsuleSelResult = masuLast;
    }
    done:
    if(work->capsuleSelWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleSelWinId);
    }
    work->capsuleSelWinId = MBWIN_NONE;
    if(work->capsuleMasuWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleMasuWinId);
    }
    work->capsuleMasuWinId = MBWIN_NONE;
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        for(i=0; i<15.0f || !MBCameraMotionCheck(); i++) {
            MBMasuPosGet(MASU_LAYER_DEFAULT, masuLast, &mdl2Pos);
            MBModelPosSetV(work->capsuleMdlId2, &mdl2Pos);
            Hu3D3Dto2D(&mdl2Pos, HU3D_CAM0, &cursorPos);
            cursorPos.x += -32;
            cursorPos.y += -16;
            espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
            HuPrcVSleep();
        }
        MBTutorialExec(TUTORIAL_INST_SAI_CAPSULE_USE);
    }
    HuMemDirectFree(work->masuIdTbl);
    work->masuIdTbl = NULL;
    HuMemDirectFree(work->masuIdTblPath);
    work->masuIdTblPath = NULL;
    espDispOff(work->cursorSprId);
    espDispOff(work->batsuSprId);
    if(capsuleSelResult != MASU_NULL) {
        WipeColorSet(255, 255, 255);
        if(!_CheckFlag(FLAG_BOARD_TUTORIAL)|| !MBTutorialExitFlagGet()) {
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSSCOPY, 1);
        }
        WipeWait();
    } else {
        if(work->capsuleMdlId1 != MB_MODEL_NONE) {
            MBModelKill(work->capsuleMdlId1);
        }
        work->capsuleMdlId1 = MB_MODEL_NONE;
        capsuleMdl = MB_MODEL_NONE;
    }
    MBCameraSkipSet(FALSE);
    //TODO: Missing move in US Version
    MBCameraModelViewSet(MBPlayerModelGet(work->playerNo), NULL, &cameraOfs, zoom, -1, 1);
    MBCameraMotionWait();
    MBCameraSkipSet(TRUE);
    MBMasuEffClear();
    
    HuPrcEnd();
}

static void CapsulePosSelKill(void)
{
    CAPSULE_POSSEL_WORK *work = capsulePosSelWork;
    CapsuleYajiKill();
    if(work->cursorSprId != -1) {
        espKill(work->cursorSprId);
    }
    work->cursorSprId = -1;
    if(work->batsuSprId != -1) {
        espKill(work->batsuSprId);
    }
    work->batsuSprId = -1;
    if(work->capsuleMdlId2 != MB_MODEL_NONE) {
        MBModelKill(work->capsuleMdlId2);
    }
    work->capsuleMdlId2 = MB_MODEL_NONE;
    if(work->capsuleSelWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleSelWinId);
    }
    work->capsuleSelWinId = MBWIN_NONE;
    if(work->capsuleMasuWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleMasuWinId);
    }
    work->capsuleMasuWinId = MBWIN_NONE;
    if(work->masuIdTbl != NULL) {
        HuMemDirectFree(work->masuIdTbl);
    }
    work->masuIdTbl = NULL;
    if(work->capsuleMasuTbl != NULL) {
        HuMemDirectFree(work->capsuleMasuTbl);
    }
    work->capsuleMasuTbl = NULL;
    if(work->masuIdTblPath != NULL) {
        HuMemDirectFree(work->masuIdTblPath);
    }
    work->masuIdTblPath = NULL;
    if(capsulePosSelWork != NULL) {
        HuMemDirectFree(capsulePosSelWork);
    }
    capsulePosSelWork = NULL;
    capsulePosSelProc = NULL;
}

static BOOL CapsuleMasuCheck(int masuId)
{
    int type = MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId);
    u32 flag = MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId);
    if((type == MASU_TYPE_BLUE || type == MASU_TYPE_RED) && !(flag & MBBranchFlagGet())) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static BOOL CapsuleMasuVisitCheck(int masuId)
{
    int type = MBMasuTypeGet(MASU_LAYER_DEFAULT, masuId);
    u32 flag = MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId);
    if(type == MASU_TYPE_NONE || type == MASU_TYPE_STAR) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void CapsuleMasuListGet(s16 *tbl, s16 masuId, s16 maxNum)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    int i;
    if(CapsuleMasuCheck(masuId)) {
        tbl[masuId] = 1;
    } else {
        tbl[masuId] = 0x8000;
    }
    if(CapsuleMasuVisitCheck(masuId) && !listStartF) {
        maxNum--;
    }
    listStartF = FALSE;
    if(maxNum > 0) {
        for(i=0; i<masuP->linkNum; i++) {
            if((MBMasuFlagGet(MASU_LAYER_DEFAULT, masuP->linkTbl[i]) & (MBBranchFlagGet()|(MASU_FLAG_BLOCKL|MASU_FLAG_BLOCKR)))) {
                continue;
            }
            if(i || !(MBMasuFlagGet(MASU_LAYER_DEFAULT, masuId) & (MASU_FLAG_BATTAN|MASU_FLAG_DOSSUN))) {
                CapsuleMasuListGet(tbl, masuP->linkTbl[i], maxNum);
            }
        }
    }
}

static void CapsuleMasuPathGet(s16 *pathTbl, s16 *tbl, s16 masuId, s16 masuLast, s16 startNum)
{
    MASU *masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    int i;
    pathTbl[startNum] = masuId;
    startNum++;
    if(masuId == masuLast) {
        capsuleMasuEndF = TRUE;
    } else {
        for(i=0; i<masuP->linkNum; i++) {
            if(tbl[masuP->linkTbl[i]]) {
                CapsuleMasuPathGet(pathTbl, tbl, masuP->linkTbl[i], masuLast, startNum);
                if(capsuleMasuEndF) {
                    break;
                }
            }
        }
    }
}

static int CapsuleMasuMesCreate(int mes)
{
    int winId;
    switch(mes) {
        case 0:
            winId = MBWinCreate(MBWIN_TYPE_CAPSULE, MESS_CAPSULE_EX99_MASU_SELECT, HUWIN_SPEAKER_NONE);
            break;
        
        case 1:
            winId = MBWinCreate(MBWIN_TYPE_CAPSULE, MESS_CAPSULE_EX99_SELF_USE, HUWIN_SPEAKER_NONE);
            break;
        
        case 2:
            winId = MBWinCreate(MBWIN_TYPE_CAPSULE, MESS_CAPSULE_EX99_THROW_DIST_MAX, HUWIN_SPEAKER_NONE);
            break;
        
        case 3:
            winId = MBWinCreate(MBWIN_TYPE_CAPSULE, MESS_CAPSULE_EX99_SELF_FAIL, HUWIN_SPEAKER_NONE);
            break;
        
        default:
            winId = MBWinCreate(MBWIN_TYPE_CAPSULE, MESS_CAPSULE_EX99_SELF_COST_FAIL, HUWIN_SPEAKER_NONE);
            break;
    }
    MBWinMesSpeedSet(winId, 0);
    MBWinPause(winId);
    return winId;;
}

static BOOL CapsuleNoUseProcCheck(void);
static void CapsuleNoUseProcExec(void);
static void CapsuleNoUseProcDestroy(void);

static BOOL CapsuleNoUseProcCreate(int playerNo, int capsuleNo)
{
    CAPSULE_POSSEL_WORK *work;
    capsuleNo &= 0xFF;
    if(capsulePosSelWork == NULL) {
        capsulePosSelWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULE_POSSEL_WORK), HU_MEMNUM_OVL);
    }
    capsulePosSelWork == NULL;
    memset(capsulePosSelWork, 0, sizeof(CAPSULE_POSSEL_WORK));
    work = capsulePosSelWork;
    work->playerNo = playerNo;
    work->masuId = MASU_NULL;
    work->capsuleNo = capsuleNo;
    capsuleSelResult = MASU_NULL;
    comCapsuleEndF[playerNo] = FALSE;
    MBCameraSkipSet(TRUE);
    capsulePosSelProc = MBPrcCreate(CapsuleNoUseProcExec, 8196, 24576);
    MBPrcDestructorSet(capsulePosSelProc, CapsuleNoUseProcDestroy);
    do {
        HuPrcVSleep();
    } while(!CapsuleNoUseProcCheck());
    if(capsuleSelResult == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static BOOL CapsuleNoUseProcCheck(void)
{
    if(capsulePosSelProc) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void ComNoUseChoiceSet(int choice);
static void ExplodeEffAddAll(HuVecF pos);

static void CapsuleNoUseProcExec(void)
{
    CAPSULE_POSSEL_WORK *work; //r31
    int time; //r30
    MBCAMERA *cameraP; //r29
    int masuId; //r28
    int capsuleIdx; // r27
    GWPLAYER *playerP; //r26
    int padNo; //r25
    float t; //f31
    float scale; //f30
    
    HuVecF cursorPos; //sp+0x54
    HuVecF playerPos; //sp+0x48
    HuVecF cameraOfs; //sp+0x3C
    HuVecF stkDir; //sp+0x30
    HuVecF masuPos; //sp+0x24
    int masuPrev; //sp+0x14
    float zoom; //sp+0x10
    MASU *masuP; //sp+0xC
    int masuNum; //sp+0x8
    
    work = capsulePosSelWork;
    playerP = GWPlayerGet(work->playerNo);
    masuNum = MBMasuNumGet(MASU_LAYER_DEFAULT);
    padNo = GwPlayer[work->playerNo].padNo;
    masuId = playerP->masuId;
    masuPrev = masuId;
    
    work->cursorSprId = espEntry(cursorSprFile, 2000, 0);
    espDispOn(work->cursorSprId);
    espDrawNoSet(work->cursorSprId, 0);
    espAttrSet(work->cursorSprId, HUSPR_ATTR_NOANIM);
    espPosSet(work->cursorSprId, 0, 0);
    espBankSet(work->cursorSprId, 0);
    work->batsuSprId = espEntry(CAPSULE_ANM_batsu, 1900, 0);
    espDispOff(work->batsuSprId);
    espDrawNoSet(work->batsuSprId, 0);
    espAttrSet(work->batsuSprId, HUSPR_ATTR_NOANIM);
    espPosSet(work->batsuSprId, 0, 0);
    espBankSet(work->batsuSprId, 0);
    work->capsuleMdlId1 = capsuleMdl = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, TRUE);
    MBModelDispSet(work->capsuleMdlId1, FALSE);
    MBPlayerPosGet(work->playerNo, &playerPos);
    MBModelPosSet(work->capsuleMdlId1, playerPos.x, playerPos.y+250, playerPos.z);
    MBModelScaleSet(work->capsuleMdlId1, 1.2f, 1.2f, 1.2f);
    MBModelLayerSet(work->capsuleMdlId1, 2);
    MBModelAttrSet(work->capsuleMdlId1, HU3D_MOTATTR_LOOP);
    cameraP = MBCameraGet();
    zoom = cameraP->zoom;
    cameraOfs = cameraP->offset;
    work->capsuleMdlId2 = MBModelCreate(MBCapsuleMdlGet(work->capsuleNo), NULL, TRUE);
    MBModelDispSet(work->capsuleMdlId2, FALSE);
    MBPlayerPosGet(work->playerNo, &playerPos);
    MBModelPosSetV(work->capsuleMdlId2, &playerPos);
    work->capsuleSelWinId = MBWIN_NONE;
    work->capsuleMasuWinId = MBWIN_NONE;
    CapsuleYajiCreate();
    masuP = MBMasuGet(MASU_LAYER_DEFAULT, masuId);
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
    espDispOn(work->cursorSprId);
    espDispOff(work->batsuSprId);
    Hu3D3Dto2D(&masuPos, HU3D_CAM0, &cursorPos);
    cursorPos.x += -32;
    cursorPos.y += -16;
    espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
    espPosSet(work->batsuSprId, cursorPos.x, cursorPos.y);
    stkDir.x = MBPadStkXGet(padNo);
    stkDir.y = 0;
    stkDir.z = MBPadStkYGet(padNo);
    MBModelDispSet(work->capsuleMdlId1, TRUE);
    time = 0;
    do {
        Hu3D3Dto2D(&masuPos, HU3D_CAM0, &cursorPos);
        cursorPos.x += -32;
        cursorPos.y += -16;
        espPosSet(work->cursorSprId, cursorPos.x, cursorPos.y);
        espPosSet(work->batsuSprId, cursorPos.x, cursorPos.y);
        time++;
        t = time/18.0f;
        if(t > 1) {
            t = 1;
        }
        MBPlayerPosGet(work->playerNo, &playerPos);
        playerPos.y += (150*HuSin(t*90))+100;
        MBModelPosSetV(work->capsuleMdlId1, &playerPos);
        scale = 1.2f*HuSin(t*90);
        MBModelScaleSet(work->capsuleMdlId1, scale, scale, scale);
        HuPrcVSleep();
    } while(!MBCameraMotionCheck() || t < 1);
    MBWinCreateChoice(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX99_TOSS, HUWIN_SPEAKER_NONE, 1);\
    MBTopWinInsertMesSet(MBCapsuleMesGet(work->capsuleNo), 0);
    MBTopWinAttrReset(HUWIN_ATTR_NOCANCEL);
    if(GwPlayer[work->playerNo].comF) {
        ComNoUseChoiceSet(-1);
    }
    MBTopWinWait();
    MBTopWinKill();
    capsuleSelResult = MBWinLastChoiceGet();
    if(capsuleSelResult == 0) {
        ExplodeEffInit();
        HuPrcVSleep();
        MBAudFXPlay(MSM_SE_BOARD_129);
        MBModelDispSet(work->capsuleMdlId1, FALSE);
        ExplodeEffAddAll(playerPos);
        MBPlayerCapsuleUseSet(work->playerNo);
        capsuleIdx = MBPlayerCapsuleFind(work->playerNo, work->capsuleNo);
        if(capsuleIdx != -1) {
            MBPlayerCapsuleRemove(work->playerNo, capsuleIdx);
        }
        do {
            HuPrcVSleep();
        } while(ExplodeEffCheck());
        ExplodeEffClose();
        HuPrcVSleep();
    } else {
        for(time=0; time<=18.0f; time++) {
            t = 1-(time/18.0f);
            if(t < 0) {
                t = 0;
            }
            MBPlayerPosGet(work->playerNo, &playerPos);
            playerPos.y += (150*HuSin(t*90))+100;
            MBModelPosSetV(work->capsuleMdlId1, &playerPos);
            scale = 1.2f*HuSin(t*90);
            MBModelScaleSet(work->capsuleMdlId1, scale, scale, scale);
            HuPrcVSleep();
        }
    }
    for(time=0; time<6.0f; time++) {
        t = 1-(time/6.0f);
        espScaleSet(work->cursorSprId, t, t);
        espScaleSet(work->batsuSprId, t, t);
        HuPrcVSleep();
    }
    espDispOn(work->cursorSprId);
    espDispOff(work->batsuSprId);
    espDispOff(work->cursorSprId);
    espDispOff(work->batsuSprId);
    if(work->capsuleSelWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleSelWinId);
    }
    work->capsuleSelWinId = MBWIN_NONE;
    if(work->capsuleMasuWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleMasuWinId);
    }
    work->capsuleMasuWinId = MBWIN_NONE;
    if(work->capsuleMdlId1 != MB_MODEL_NONE) {
        MBModelKill(work->capsuleMdlId1);
    }
    work->capsuleMdlId1 = MB_MODEL_NONE;
    capsuleMdl = MB_MODEL_NONE;
    MBMasuEffClear();
    HuPrcEnd();
}

static void CapsuleNoUseProcDestroy(void)
{
    CAPSULE_POSSEL_WORK *work = capsulePosSelWork;
    CapsuleYajiKill();
    if(work->cursorSprId != -1) {
        espKill(work->cursorSprId);
    }
    work->cursorSprId = -1;
    if(work->batsuSprId != -1) {
        espKill(work->batsuSprId);
    }
    work->batsuSprId = -1;
    if(work->capsuleMdlId1 != MB_MODEL_NONE) {
        MBModelKill(work->capsuleMdlId1);
    }
    work->capsuleMdlId1 = MB_MODEL_NONE;
    if(work->capsuleMdlId2 != MB_MODEL_NONE) {
        MBModelKill(work->capsuleMdlId2);
    }
    work->capsuleMdlId2 = MB_MODEL_NONE;
    if(work->capsuleSelWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleSelWinId);
    }
    work->capsuleSelWinId = MBWIN_NONE;
    if(work->capsuleMasuWinId != MBWIN_NONE) {
        MBWinKill(work->capsuleMasuWinId);
    }
    work->capsuleMasuWinId = MBWIN_NONE;
    if(work->masuIdTbl != NULL) {
        HuMemDirectFree(work->masuIdTbl);
    }
    work->masuIdTbl = NULL;
    if(work->capsuleMasuTbl != NULL) {
        HuMemDirectFree(work->capsuleMasuTbl);
    }
    work->capsuleMasuTbl = NULL;
    if(work->masuIdTblPath != NULL) {
        HuMemDirectFree(work->masuIdTblPath);
    }
    work->masuIdTblPath = NULL;
    if(capsulePosSelWork != NULL) {
        HuMemDirectFree(capsulePosSelWork);
    }
    capsulePosSelWork = NULL;
    capsulePosSelProc = NULL;
}

void MBCapsuleMasuSet(int masuId, int capsuleNo)
{
    capsuleNo &= 0xFF;
    if(capsuleNo > CAPSULE_MIRACLE) {
        return;
    }
    switch(capsuleNo) {
        case CAPSULE_HONE:
            return;
        
        case CAPSULE_KOOPA:
            return;
        
        case CAPSULE_MIRACLE:
            return;
        
        case CAPSULE_CHANCE:
        default:
            MBMasuCapsuleSet(MASU_LAYER_DEFAULT, masuId, capsuleNo);
            if(capsuleNo != CAPSULE_BOMHEI) {
                (void)capsuleNo;
                return;
            } else {
                MBMasuBomheiSet(masuId);
            }
    }
}

static void CapsuleThrowWin(int masuId, int capsuleNo)
{
    capsuleNo &= 0xFF;
    switch(capsuleNo) {
        case CAPSULE_HONE:
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX99_EVENT_NULL, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTopWinKill();
            break;
        
        case CAPSULE_KOOPA:
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX99_EVENT_NULL, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTopWinKill();
            break;
        
        case CAPSULE_CHANCE:
            break;
            
        case CAPSULE_MIRACLE:
            MBWinCreate(MBWIN_TYPE_TALKEVENT, MESS_CAPSULE_EX99_EVENT_NULL, HUWIN_SPEAKER_NONE);
            MBTopWinWait();
            MBTopWinKill();
            break;
    }
}

unsigned int MBCapsuleMdlGet(int capsuleNo)
{
    capsuleNo &= 0xFF;
    return capsuleTbl[capsuleNo].mdlFile;
}

unsigned int MBCapsuleObjMdlGet(int capsuleNo)
{
    capsuleNo &= 0xFF;
    return capsuleTbl[capsuleNo].objMdlFile;
}

u32 MBCapsuleMesGet(int capsuleNo)
{
    capsuleNo &= 0xFF;
    return capsuleTbl[capsuleNo].mesNo;
}

int MBCapsuleMasuPatNoGet(int capsuleNo)
{
    capsuleNo &= 0xFF;
    return capsuleTbl[capsuleNo].masuPatNo;
}

int MBCapsuleColorNoGet(int capsuleNo)
{
    capsuleNo &= 0xFF;
    return capsuleTbl[capsuleNo].colorNo;
}

int MBCapsuleCostGet(int capsuleNo)
{
    capsuleNo &= 0xFF;
    return capsuleTbl[capsuleNo].cost;
}

BOOL MBCapsuleNoUseCheck(int capsuleNo)
{
    capsuleNo &= 0xFF;
    if(capsuleTbl[capsuleNo].cost != -1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int CapsuleMasuListSearch(int masuId)
{
    s16 *tbl = HuMemDirectMallocNum(HEAP_HEAP, MASU_MAX*sizeof(s16), HU_MEMNUM_OVL);
    s16 i;
    s16 num;
    memset(tbl, 0, MASU_MAX*sizeof(s16));
    listStartF = TRUE;
    CapsuleMasuListGet(tbl, masuId, 10);
    tbl[masuId] = 0x8000;
    for(i=0, num=0; i<MASU_MAX; i++) {
        if(tbl[i] & 0x1) {
            num++;
        }
    }
    HuMemDirectFree(tbl);
    return num;
}

int MBCapsuleListGet(int *capsuleNo)
{
    int i;
    int num;
    int redNum;
    int bankNum;
    
    for(i=1, redNum=0; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        if(MBMasuTypeGet(MASU_LAYER_DEFAULT, i) == MASU_TYPE_RED) {
            redNum++;
        }
    }
    for(i=1, bankNum=0; i<=MBMasuNumGet(MASU_LAYER_DEFAULT); i++) {
        if(MBMasuCapsuleGet(MASU_LAYER_DEFAULT, i) == CAPSULE_BANK) {
            bankNum++;
        }
    }
    for(i=0, num=0; i<CAPSULE_MAX-4; i++) {
        if((GWPartyFGet() != FALSE || !capsuleTbl[i].listFlag)
            && (!GWTeamFGet() || i != CAPSULE_MIRACLE)
            && (redNum > 0 || i != CAPSULE_KOOPA)
            && (MBBankCoinGet() <= 0 || bankNum < 1 || i != CAPSULE_BANK)
            && (GwSystem.turnMax-GwSystem.turnNo >= 5 || i != CAPSULE_DUEL)
            && MBCapsuleMdlGet(i) != 0
            && i <= CAPSULE_MIRACLE) {
            capsuleNo[num] = i;
            num++;
        }
    }
    return num;
}

typedef struct CapsuleTurnData_s {
    s8 code;
    int chance;
} CAPSULE_TURNDATA;

int MBCapsuleNextGet(int rank)
{
    int i;
    int idx;
    int wrapIdx;
    int swapIdx;
    int num;
    int *chanceTbl;
    int gamePart;
    int totalChance;
    int temp;
    int turnSeg;
    int code;
    int chance;
    
    int list[CAPSULE_MAX-4];
    int listBackup[CAPSULE_MAX-4];
    CAPSULE_TURNDATA turnData[5];
    
    static int chanceData[3][4][5] = {
        {
            {
                70,
                15,
                15
            },
            {
                70,
                20,
                10
            },
            {
                70,
                20,
                0,
                10
            },
            {
                50,
                30,
                0,
                20,
            },
        },
        {
            {
                40,
                30,
                30
            },
            {
                30,
                30,
                20,
                20
            },
            {
                10,
                40,
                10,
                30,
                10
            },
            {
                10,
                40,
                0,
                30,
                20
            },
        },
        {
            {
                30,
                30,
                30,
                10
            },
            {
                20,
                20,
                20,
                30,
                10
            },
            {
                10,
                30,
                0,
                40,
                20
            },
            {
                10,
                20,
                0,
                40,
                30
            }
        }
    };
    static int maxTurnTbl[9] = {
        10,
        15,
        20,
        25,
        30,
        35,
        40,
        45,
        50
    };
    static int turnTbl[9][2] = {
        { 3, 6 },
        { 5, 10 },
        { 5, 15 },
        { 8, 16 },
        { 10, 20 },
        { 10, 20 },
        { 13, 26 },
        { 15, 30 },
        { 15, 35 },
    };
    num = MBCapsuleListGet(list);
    if(rank < 0) {
        for(i=0; i<num*5; i++) {
            wrapIdx = i%num;
            swapIdx = (frand()&0x7FFF)%num;
            if(wrapIdx != swapIdx) {
                temp = list[wrapIdx];
                list[wrapIdx] = list[swapIdx];
                list[swapIdx] = temp;
            }
        }
        return list[(frand()&0x7FFF)%num];
    } else {
        for(i=0; i<8u; i++) {
            if(GwSystem.turnMax <= maxTurnTbl[i]) {
                break;
            }
        }
        if(i < 9u) {
            turnSeg = i;
            for(i=0; i<2; i++) {
                if(GwSystem.turnNo < turnTbl[turnSeg][i]) {
                    break;
                }
            }
            gamePart = i;
        } else {
            if(GwSystem.turnNo <= (GwSystem.turnMax/3)) {
                gamePart = 0;
            } else if(GwSystem.turnNo <= (((GwSystem.turnMax)/3)*2)) {
                gamePart = 1;
            } else {
                gamePart = 2;
            }
        }
        if(rank > 3) {
            rank = 3;
        }
        chanceTbl = &chanceData[gamePart][rank][0];
        for(i=0, totalChance=0, idx=0; i<5; i++) {
            if(chanceTbl[i] > 0) {
                turnData[idx].code = i+'A';
                totalChance += chanceTbl[i];
                turnData[idx].chance = totalChance;
                idx++;
            }
        }
        chance = frand7f()*totalChance;
        for(i=0; i<idx; i++) {
            if(chance < turnData[i].chance) {
                break;
            }
        }
        if(i >= idx) {
            i = 0;
        }
        code = turnData[i].code;
        for(i=0, idx=0; i<num; i++) {
            if(code == capsuleTbl[list[i]].code) {
                listBackup[idx] = list[i];
                idx++;
            }
        }
        if(idx <= 0) {
            return MBCapsuleNextGet(-1);
        }
        for(i=0; i<idx*5; i++) {
            wrapIdx = i%idx;
            swapIdx = (frand()&0x7FFF)%idx;
            if(wrapIdx != swapIdx) {
                temp = listBackup[wrapIdx];
                listBackup[wrapIdx] = listBackup[swapIdx];
                listBackup[swapIdx] = temp;
            }
        }
        i = (frand()&0x7FFF)%idx;
        return listBackup[i];
    }
}

typedef struct CapsuleComSel_s {
    int capsuleNo;
    int chance;
} CAPSULE_COMSEL;

static BOOL CapsuleComUseCheck(int playerNo, int capsuleNo);

int MBComCapsuleSelGet(int playerNo)
{
    CAPSULE_COMSEL *selTblP;
    int i;
    int chance;
    int j;
    int selMax;
    int chanceTotal;
    int charNo;
    int capsuleNum;
    int capsuleNo;
    CAPSULE_COMDATA *comData;
    int pathLen;
    CAPSULE_COMSEL selTbl[9];
    
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return 0;
    }
    if(GWTeamFGet() && !GwPlayer[MBPlayerGrpFind(playerNo)].comF) {
        return -1;
    }
    if(comCapsuleEndF[playerNo]) {
        comCapsuleEndF[playerNo] = FALSE;
        return -1;
    }
    charNo = GwPlayer[playerNo].charNo;
    if(charNo > CHARNO_MINIKOOPA) {
        charNo = CHARNO_MINIKOOPA;
    }
    capsuleNum = MBPlayerCapsuleNumGet(playerNo);
    if(capsuleNum <= 0) {
        return -1;
    }
    selTblP = selTbl;
    selTblP->capsuleNo = CAPSULE_NULL;
    selMax = 1;
    chanceTotal = 30;
    selTblP->chance = chanceTotal;
    selTblP++;
    for(i=0; i<capsuleNum; i++) {
        capsuleNo = MBPlayerCapsuleGet(playerNo, i);
        if(capsuleNo == CAPSULE_NULL) {
            continue;
        }
        comData = &capsuleComData[capsuleNo];
        switch(comData->charData[charNo].code1) {
            case 'A':
                chance = 50;
                break;
            
            case 'B':
                chance = 30;
                break;
                
            case 'C':
                chance = 10;
                break;
            
            case 'D':
                for(j=0; j<GW_PLAYER_MAX; j++) {
                    if(j == playerNo) {
                        continue;
                    }
                    if(!GWTeamFGet() || MBPlayerGrpGet(playerNo) != MBPlayerGrpGet(j)) {
                        if(MBPlayerAliveCheck(j)) {
                            pathLen = MBMasuPathTypeLenGet(GwPlayer[j].masuId, MASU_TYPE_STAR);
                            if(pathLen <= 10) {
                                break;
                            }
                        }
                    }
                }
                if(GWPartyFGet() != FALSE && j < 4 && MBPlayerBestPathGet() != playerNo) {
                    chance = 50;
                } else {
                    chance = 20;
                }
                break;
            
            default:
                if(capsuleNum >= MBPlayerCapsuleMaxGet()) {
                    chance = 30;
                } else {
                    chance = 5;
                }
                break;
        }
        selTblP->capsuleNo = capsuleNo;
        chanceTotal += chance;
        selTblP->chance = chanceTotal;
        selMax++;
        selTblP++;
    }
    chance = frand7f()*chanceTotal;
    for(i=0, selTblP=selTbl; i<selMax; i++, selTblP++) {
        if(chance < selTblP->chance) {
            break;
        }
    }
    if(i >= selMax || selTblP->capsuleNo == CAPSULE_NULL) {
        return -1;
    }
    if(CapsuleMasuListSearch(GwPlayer[playerNo].masuId) <= 0 && !CapsuleComUseCheck(playerNo, selTblP->capsuleNo)) {
        return -1;
    }
    if(i >= selMax) {
        return -1;
    }
    return MBPlayerCapsuleFind(playerNo, selTblP->capsuleNo);
}


BOOL MBCapsulePosAutoSelCheck(int playerNo, int capsuleNo)
{
    CAPSULE_COMDATA *comData;
    int charNo;
    float chance;
    charNo = GwPlayer[playerNo].charNo;
    if(charNo > CHARNO_MINIKOOPA) {
        charNo = CHARNO_MINIKOOPA;
    }
    comData = &capsuleComData[capsuleNo];
    if(!CapsuleComUseCheck(playerNo, capsuleNo)) {
        return FALSE;
    }
    if(comData->charData[charNo].code2 == 'd') {
        return FALSE;
    }
    if(CapsuleMasuListSearch(GwPlayer[playerNo].masuId) <= 0) {
        return TRUE;
    }
    chance = frand7f();
    switch(comData->charData[charNo].code2) {
        case 'a':
            if(chance < 0.9f) {
                return TRUE;
            }
            break;
        
        case 'b':
            if(chance < 0.5f) {
                return TRUE;
            }
            break;
        
        case 'c':
            if(chance < 0.25f) {
                return TRUE;
            }
            break;
        
        default:
            if(chance < 0.0f) {
                return TRUE;
            }
            break;
    }
    return FALSE;
}

static BOOL CapsuleComUseCheck(int playerNo, int capsuleNo)
{
    CAPSULE_COMDATA *comData;
    int charNo;
    BOOL validF;
    validF = FALSE;
    charNo = GwPlayer[playerNo].charNo;
    if(charNo > CHARNO_MINIKOOPA) {
        charNo = CHARNO_MINIKOOPA;
    }
    comData = &capsuleComData[capsuleNo];
    if(!MBCapsuleNoUseCheck(capsuleNo)) {
        return FALSE;
    }
    if(MBPlayerCoinGet(playerNo) < MBCapsuleCostGet(capsuleNo)) {
        return FALSE;
    }
    if(capsuleNo != CAPSULE_KURIBO && MBPlayerCoinGet(playerNo) < MBCapsuleCostGet(capsuleNo)+10) {
        return FALSE;
    }
    if(capsuleNo == CAPSULE_BANK && MBBankCoinGet() <= 5) {
        return FALSE;
    }
    if(comData->charData[charNo].code2 == 'd') {
        return FALSE;
    }
    if(comData->minCoin > 0) {
        if(MBPlayerCoinGet(playerNo) >= comData->minCoin) {
            validF = TRUE;
        } else if(comData->minCoin < 0) {
            if(MBPlayerCoinGet(playerNo) <= -comData->minCoin) {
                validF = TRUE;
            }
        }
    }
    if(comData->charData[charNo].forceRejectF && !validF) {
        return FALSE;
    } else {
        return TRUE;
    }
    
}

int MBCapsuleCodeGet(int capsuleNo)
{
    capsuleNo &= 0xFF;
    return capsuleTbl[capsuleNo].code;
}

int MBCapsuleWinCreate(int capsuleNo)
{
    static char coinStr[16];
    int winId;
    capsuleNo &= 0xFF;
    if(GWPartyFGet() != FALSE) {
        winId = MBWinCreate(MBWIN_TYPE_CAPSULE, capsuleMesTbl[capsuleNo], HUWIN_SPEAKER_NONE);
    } else {
        winId = MBWinCreate(MBWIN_TYPE_CAPSULE, capsuleStoryMesTbl[capsuleNo], HUWIN_SPEAKER_NONE);
    }
    sprintf(coinStr, "%d", MBCapsuleCostGet(capsuleNo));
    MBTopWinInsertMesSet(MESSNUM_PTR(coinStr), 0);
    MBWinMesSpeedSet(winId, 0);
    MBWinPause(winId);
    return winId;
}

typedef struct CapsuleObj_s {
    s16 flag;
    MBMODELID mdlId;
    MBMODELID mdlId2;
    u8 layer;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
} CAPSULE_OBJ;

static CAPSULE_OBJ capsuleObj[CAPSULE_OBJ_MAX];

void MBCapsuleObjInit(void)
{
    CAPSULE_OBJ *obj = capsuleObj;
    memset(obj, 0, sizeof(CAPSULE_OBJ)*CAPSULE_OBJ_MAX);
}

int MBCapsuleObjCreate(int capsuleNo, BOOL linkF)
{
    CAPSULE_OBJ *obj;
    int i;
    unsigned int file;
    for(obj=capsuleObj, capsuleNo &= 0xFF, i=0; i<CAPSULE_OBJ_MAX; i++, obj++) {
        if(!obj->flag) {
            break;
        }
    }
    if(i >= CAPSULE_OBJ_MAX) {
        return MB_MODEL_NONE;
    }
    file = MBCapsuleObjMdlGet(capsuleNo);
    obj->flag = TRUE;
    obj->mdlId = MBModelCreate(file, NULL, linkF);
    obj->layer = 0;
    obj->pos.x = obj->pos.y = obj->pos.z = 0;
    obj->rot.x = obj->rot.y = obj->rot.z = 0;
    obj->scale.x = obj->scale.y = obj->scale.z = 1;
    MBCapsuleObjLayerSet(obj->mdlId, obj->layer);
    return obj->mdlId;
}

static int CapsuleObjSearch(int id)
{
    CAPSULE_OBJ *obj;
    int i;
    for(obj=capsuleObj, i=0; i<CAPSULE_OBJ_MAX; i++, obj++) {
        if(obj->flag && obj->mdlId == id){
            break;
        }
    }
    if(i < CAPSULE_OBJ_MAX) {
        return i;
    } else {
        return -1;
    }
}

void MBCapsuleObjKill(int id)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        if(obj->flag) {
            MBModelKill(obj->mdlId);
            obj->mdlId = MB_MODEL_NONE;
            obj->mdlId2 = MB_MODEL_NONE;
            obj->flag = FALSE;
        }
    }
}

void MBCapsuleObjPosSet(int id, float posX, float posY, float posZ)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        obj->pos.x = posX;
        obj->pos.y = posY;
        obj->pos.z = posZ;
        MBModelPosSetV(obj->mdlId, &obj->pos);
    }
}

void MBCapsuleObjRotSet(int id, float rotX, float rotY, float rotZ)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        obj->rot.x = rotX;
        obj->rot.y = rotY;
        obj->rot.z = rotZ;
        MBModelRotSetV(obj->mdlId, &obj->rot);
    }
}

void MBCapsuleObjScaleSet(int id, float scaleX, float scaleY, float scaleZ)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        obj->scale.x = scaleX;
        obj->scale.y = scaleY;
        obj->scale.z = scaleZ;
        MBModelScaleSetV(obj->mdlId, &obj->scale);
    }
}

void MBCapsuleObjPosSetV(int id, HuVecF *pos)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBCapsuleObjPosSet(id, pos->x, pos->y, pos->z);
    }
}

void MBCapsuleObjRotSetV(int id, HuVecF *rot)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBCapsuleObjRotSet(id, rot->x, rot->y, rot->z);
    }
}

void MBCapsuleObjScaleSetV(int id, HuVecF *scale)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBCapsuleObjScaleSet(id, scale->x, scale->y, scale->z);
    }
}

void MBCapsuleObjPosGet(int id, HuVecF *pos)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        *pos = obj->pos;
    }
}

void MBCapsuleObjRotGet(int id, HuVecF *rot)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        *rot = obj->rot;
    }
}

void MBCapsuleObjScaleGet(int id, HuVecF *scale)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        *scale = obj->scale;
    }
}

void MBCapsuleObjLayerSet(int id, u8 layer)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        obj->layer = layer;
        MBModelLayerSet(obj->mdlId, obj->layer);
    }
}

u8 MBCapsuleObjLayerGet(int id)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) == -1) {
        return 0;
    }
    obj = &capsuleObj[idx];
    return obj->layer;
}

void MBCapsuleObjAttrSet(int id, u32 attr)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBModelAttrSet(obj->mdlId, attr);
    }
}

void MBCapsuleObjAttrReset(int id, u32 attr)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBModelAttrReset(obj->mdlId, attr);
    }
}

void MBCapsuleObjDispSet(int id, BOOL dispF)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBModelDispSet(obj->mdlId, dispF);
    }
}

void MBCapsuleObjAlphaSet(int id, u8 alpha)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBModelAlphaSet(obj->mdlId, alpha);
    }
}

void MBCapsuleObjMtxSet(int id, Mtx *mtx)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBModelMtxSet(obj->mdlId, mtx);
    }
}

void MBCapsuleObjMtxGet(int id, Mtx *mtx)
{
    CAPSULE_OBJ *obj;
    int idx;
    if((idx=CapsuleObjSearch(id)) != -1) {
        obj = &capsuleObj[idx];
        MBModelMtxGet(obj->mdlId, mtx);
    }
}

static void CapsuleYajiCreate(void)
{
    
}

static void CapsuleYajiRotSet(int masuId, float rot)
{
    
}

static void CapsuleYajiGrowSet(void)
{
    
}

static void CapsuleYajiKill(void)
{
    
}

static void ExplodeEffInit(void)
{
    
}

static void ExplodeEffClose(void)
{
    
}

static void ExplodeEffAddAll(HuVecF pos)
{
    
}


static int ExplodeEffCheck(void)
{
    
}

static void GlowEffInit(void)
{
    
}

static void GlowEffClose(void)
{
    
}

static BOOL GlowEffCheck(void)
{
    
}

static int GlowEffAdd(HuVecF pos, HuVecF ofs, float scale, float fadeSpeed, float rotSpeed, float gravity, GXColor color)
{
    
}

static void RingEffInit(void)
{
    
}

static void RingEffClose(void)
{
    
}

static BOOL RingEffCheck(void)
{
    
}

static void RingEffNumSet(int num)
{
    
}

static int RingEffAdd(HuVecF pos, HuVecF ofs, HuVecF scale, int inTime, int rotTime, int no, int mode, GXColor color)
{
    
}

static void DomeEffInit(void)
{
    
}

static void DomeEffClose(void)
{
    
}

static void DomeEffMdlUpdate(HuVecF *pos, HuVecF *rot)
{

}

static void RayEffInit(void)
{
    
}

static void RayEffClose(void)
{
    
}

static void ThrowEffInit(int capsuleNo)
{
    
}

static void ThrowEffClose(void)
{
    
}

static void ThrowEffUpdate(HuVecF *pos, int capsuleNo)
{
    
}

static void ThrowEffPosSet(HuVecF *pos)
{
    
}

static float AngleSumWrap(float start, float end)
{
    
}

static float CameraXZAngleGet(float angle)
{
    
}

static void ComNoUseChoiceSet(int choice)
{
    
}

static void ApplyBezier(float t, float *a, float *b, float *c, float *out)
{
    
}

static void CapsuleThrowCameraSet(float *throwX, float *throwY, float *throwZ, int num)
{
    
}

static void CapsuleThrowCameraCalc(float t, float *throwX, float *throwY, float *throwZ, HuVecF *out, int num)
{
    
}

static void CapsuleColBegin(void)
{
    
}

static BOOL CapsuleColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    
}

static void CapsuleColEnd(void)
{
    
}