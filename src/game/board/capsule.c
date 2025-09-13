#include "game/esprite.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/main.h"

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
#include "game/board/effect.h"

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

static HU3DMODELID CapsuleEffCreate(ANIMDATA *anim, s16 num);

#define CAPSULE_EFF_ATTR_NONE 0
#define CAPSULE_EFF_ATTR_COUNTER_RESET (1 << 0)
#define CAPSULE_EFF_ATTR_COUNTER_UPDATE (1 << 1)

#define CAPSULE_EFF_DISPATTR_NONE 0
#define CAPSULE_EFF_DISPATTR_ZBUF_OFF (1 << 0)
#define CAPSULE_EFF_DISPATTR_NOANIM (1 << 1)
#define CAPSULE_EFF_DISPATTR_CAMERA_ROT (1 << 2)
#define CAPSULE_EFF_DISPATTR_ROT3D (1 << 3)
#define CAPSULE_EFF_DISPATTR_ALL 15

typedef struct CapsuleEffect_s CAPSULE_EFFECT;
typedef void (*CAPSULE_EFFHOOK)(HU3DMODEL *modelP, CAPSULE_EFFECT *effP, Mtx *matrix);

typedef struct CapsuleEffData_s {
    s16 time;
    s16 work;
    s16 mode;
    s16 cameraBit;
    HuVecF vel;
    float baseAlpha;
    float tpLvl;
    float speed;
    float unk20;
    float gravity;
    float rotSpeed;
    float animTime;
    float animSpeed;
    float scale;
    HuVecF rot;
    HuVecF pos;
    GXColor color;
    int no;
} CAPSULE_EFFDATA;

struct CapsuleEffect_s {
    s16 mode;
    s16 time;
    HuVecF vel;
    s16 work[8];
    u8 blendMode;
    u8 attr;
    u8 dispAttr;
    u8 unk23;
    HU3DMODELID modelId;
    s16 num;
    u32 count;
    u32 prevCounter;
    u32 prevCount;
    u32 dlSize;
    ANIMDATA *anim;
    CAPSULE_EFFDATA *data;
    HuVecF *vertex;
    HuVec2F *st;
    void *dl;
    CAPSULE_EFFHOOK hook;
    HU3DMODEL *hookMdlP;
};

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
static CAPSULE_YAJI_WORK *capsuleYajiWork;
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
static int RingEffAdd(HuVecF pos, HuVecF rot, HuVecF vel, int inTime, int rotTime, int no, int mode, GXColor color);
static void GlowEffClose(void);
static void RingEffClose(void);
static int GlowEffAdd(HuVecF pos, HuVecF vel, float scale, float fadeSpeed, float rotSpeed, float gravity, GXColor color); 
static int GlowEffNumGet(void);
static int RingEffNumGet(void);

static void CapsuleUseExec(void)
{
    int i;
    int mdlId;
    int coinDispId;
    
    HuVecF playerPos;
    HuVecF mdlPos;
    HuVecF coinDispPos;
    HuVecF effectVel;
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
    static HuVecF ringRot = { -90, 0, 20 };
    
    
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
    
    RingEffAdd(mdlPos, ringRot, coinDispPos, 18, 60, 0, 1, color);
    MBAudFXPlay(MSM_SE_BOARD_30);
    for(i=0; i<18.0f; i++) {
        HuPrcVSleep();
    }
    coinDispPos.x = 1.5f;
    coinDispPos.y = 3.0f;
    coinDispPos.z = 200;
    color.r = (MBCapsuleEffRandF()*64)+192;
    color.g = (MBCapsuleEffRandF()*64)+192;
    color.b = (MBCapsuleEffRandF()*64)+192;
    color.a = 255;
    RingEffAdd(mdlPos, ringRot, coinDispPos, 18, 36, 2, 1, color);
    coinDispPos.x = 1.5f;
    coinDispPos.y = 3.0f;
    coinDispPos.z = 300;
    color.r = (MBCapsuleEffRandF()*64)+192;
    color.g = (MBCapsuleEffRandF()*64)+192;
    color.b = (MBCapsuleEffRandF()*64)+192;
    color.a = 255;
    RingEffAdd(mdlPos, ringRot, coinDispPos, 12, 30, 1, 1, color);
    angle = 0;
    for(i=0; i<128; i++) {
        angle += (MBCapsuleEffRandF()+1)*10;
        coinDispPos.x = (75*(-0.5f+MBCapsuleEffRandF()))+mdlPos.x;
        coinDispPos.y = (75*(-0.5f+MBCapsuleEffRandF()))+mdlPos.y;
        coinDispPos.z = 30.000002f+mdlPos.z;
        radius = (MBCapsuleEffRandF()+1.0f)*5;
        effectVel.x = radius*HuSin(angle);
        effectVel.y = radius*HuCos(angle);
        effectVel.z = 0;
        intensity = MBCapsuleEffRandF();
        color.r = (intensity*63)+192;
        color.g = (intensity*63)+192;
        color.b = (intensity*63)+192;
        color.a = (MBCapsuleEffRandF()*63)+192;
        GlowEffAdd(coinDispPos, effectVel, ((MBCapsuleEffRandF()*0.1f)+0.3f)*100, (MBCapsuleEffRandF()+1.0f)*0.016666668f, 0, 0, color);
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
    } while(GlowEffNumGet() && RingEffNumGet());
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

BOOL MBCapsulePlayerMasuCheck(int playerNo, HuVecF *a, HuVecF *b, HuVecF *out)
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
static int ExplodeEffNumGet(void);

static void ThrowEffInit(int capsuleNo);
static void ThrowEffClose(void);
static void RingEffDispAttrSet(int dispAttr);
static void DomeEffInit(void);
static void RayEffInit(void);
static void DomeEffClose(void);
static void RayEffClose(void);

static void CapsuleThrowEffCreate(float *x, float *y, float *z, float yOfs, int masuId);
static BOOL CapsuleThrowParamSet(HuVecF *pos, int *maxTime);
static void CapsuleThrowEffKill(void);
static void CapsuleBezierParamSet(float *x, float *y, float *z, int num);
static void CapsuleCurveCalc(float t, float *x, float *y, float *z, HuVecF *out, int num);
static void ThrowEffStart(HuVecF *pos, int capsuleNo);
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
    HuVecF vel;
    HuVecF playerPos;
    HuVecF playerPosOrig;
    float x[3];
    float y[3];
    float z[3];
    HuVecF throwOut;

    GXColor color;
    float magTemp;
    static HuVecF ringData[2] = { { -90, 0, 0 }, { 0, 10, 60 } };
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
    RingEffDispAttrSet(CAPSULE_EFF_DISPATTR_ALL);
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
    x[0] = work->playerPos.x;
    y[0] = work->playerPos.y;
    z[0] = work->playerPos.z;
    x[1] = work->playerPos.x;
    y[1] = work->playerPos.y;
    z[1] = work->playerPos.z;
    x[2] = work->masuPos.x;
    y[2] = work->masuPos.y;
    z[2] = work->masuPos.z;
    CapsuleThrowEffCreate(x, y, z, work->yOfs, work->masuId);
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
    x[1] = pos.x;
    y[1] = pos.y;
    z[1] = pos.z;
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
    ThrowEffStart(&work->playerPos, work->capsuleNo);
    seNo = MBAudFXPlay(MSM_SE_BOARD_26);
    do {
        work->time++;
        t = (float)work->time/(float)work->maxTime;
        VECSubtract(&work->masuPos, &work->playerPos, &dir);
        VECScale(&dir, &dir, t);
        VECAdd(&work->playerPos, &dir, &effPos);
        mdl2Pos = effPos;
        CapsuleCurveCalc(t, x, y, z, &throwOut, 3);
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
        pos.x = effPos.x+(((0.5f-MBCapsuleEffRandF())*100)*0.75f);
        pos.y = effPos.y+(((0.5f-MBCapsuleEffRandF())*100)*0.75f);
        pos.z = effPos.z+(((0.5f-MBCapsuleEffRandF())*100)*0.75f);
        vel.x = vel.y = vel.z = 0;
        color = colTbl[MBCapsuleColorNoGet(work->capsuleNo)];
        color.a = (MBCapsuleEffRandF()*63)+192;
        GlowEffAdd(pos, vel, ((MBCapsuleEffRandF()*0.025f)+0.1f)*100, (MBCapsuleEffRandF()+1)*0.016666668f, 0, 0.025f, color);
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
    RingEffAdd(work->masuPos, ringData[0], ringData[1], 6, 18, 0, 2, color);
    if(masuP != NULL) {
        DomeEffMdlUpdate(&work->masuPos, &masuP->rot);
    } else {
        DomeEffMdlUpdate(&work->masuPos, NULL);
    }
    work->time = 0;
    do {
        work->time++;
        HuPrcVSleep();
    } while(work->time < 6.0f || ExplodeEffNumGet() > 0);
    MBCapsuleMasuSet(work->masuId, work->capsuleNo);
    work->time = 0;
    do {
        work->time++;
        HuPrcVSleep();
    } while(work->time < 84.0f || ExplodeEffNumGet() > 0);
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
    float x[3];
    float y[3];
    float z[3];
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


static void CapsuleThrowEffCreate(float *x, float *y, float *z, float yOfs, int masuId)
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
    work->startPos.x = x[0];
    work->startPos.y = y[0];
    work->startPos.z = z[0];
    work->endPos.x = x[2];
    work->endPos.y = y[2];
    work->endPos.z = z[2];
    work->pos.x = work->pos.y = work->pos.z = 0;
    work->yOfs = yOfs;
    work->tick = 0;
    work->x[0] = x[0];
    work->y[0] = y[0];
    work->z[0] = z[0];
    work->x[1] = x[1];
    work->y[1] = y[1];
    work->z[1] = z[1];
    work->x[2] = x[2];
    work->y[2] = y[2];
    work->z[2] = z[2];
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
        work->x[1] = pos.x;
        work->y[1] = pos.y;
        work->z[1] = pos.z;
        CapsuleBezierParamSet(work->x, work->y, work->z, 3);
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
        CapsuleCurveCalc(t, work->x, work->y, work->z, &out, 3);
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
            work->x[1] = pos.x;
            work->y[1] = pos.y;
            work->z[1] = pos.z;
            CapsuleBezierParamSet(work->x, work->y, work->z, 3);
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
        pos->x = work->x[1];
        pos->y = work->y[1];
        pos->z = work->z[1];
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
    HuVecF glowVel;
    GXColor color;
    float magTemp;
    static HuVecF ringData[2] = { { -90, 0, 0 }, { 0, 10, 60 } };
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
    RingEffDispAttrSet(CAPSULE_EFF_DISPATTR_ALL);
    DomeEffInit();
    RayEffInit();
    work->capsuleObjMdlId = MBCapsuleObjCreate(work->capsuleNo, FALSE);
    MBCapsuleObjPosSetV(work->capsuleObjMdlId, &work->playerPos);
    MBCapsuleObjLayerSet(work->capsuleObjMdlId, 2);
    seNo = MBAudFXPlay(MSM_SE_BOARD_26);
    ThrowEffStart(&work->playerPos, work->capsuleNo);
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
        glowPos.x = effPos.x+(((0.5f-MBCapsuleEffRandF())*100)*0.75f);
        glowPos.y = effPos.y+(((0.5f-MBCapsuleEffRandF())*100)*0.75f);
        glowPos.z = effPos.z+(((0.5f-MBCapsuleEffRandF())*100)*0.75f);
        glowVel.x = glowVel.y = glowVel.z = 0;
        color = colTbl[MBCapsuleColorNoGet(work->capsuleNo)];
        color.a = (MBCapsuleEffRandF()*63)+192;
        GlowEffAdd(glowPos, glowVel, ((MBCapsuleEffRandF()*0.025f)+0.1f)*100, (MBCapsuleEffRandF()+1)*0.016666668f, 0, 0.025f, color);
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
    RingEffAdd(work->masuPos, ringData[0], ringData[1], 6, 18, 0, 2, color);
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
    } while(work->time < 6.0f || ExplodeEffNumGet() > 0);
    MBCapsuleMasuSet(work->masuId, work->capsuleNo);
    work->time = 0;
    do {
        work->time++;
        HuPrcVSleep();
    } while(work->time < 84.0f || ExplodeEffNumGet() > 0);
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
static void CapsuleYajiObjSet(int masuId, float rot);
static int CapsuleMasuMesCreate(int mes);
static float CameraXZAngleGet(float angle);
static float AngleSumWrap(float start, float end);
static void CapsuleYajiShrinkSet(void);
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
                    CapsuleYajiObjSet(masuId, yajiAngle[i]);
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
        CapsuleYajiShrinkSet();
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
            masuLast = pathTbl[MBCapsuleEffRand(num)];
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
            HuPrcSleep((MBCapsuleEffRandF()*60.0f)*0.3f);
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
            MBAudFXPlay(MSM_SE_BOARD_24);
            capsuleSelResult = masuId;
            goto done;
        } else {
            if(sePlayF) {
                MBAudFXPlay(MSM_SE_BOARD_23);
            }
            masuId = pathTbl[selPos];
            HuPrcVSleep();
            CapsuleYajiShrinkSet();
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
        } while(ExplodeEffNumGet());
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
    int inIdx;
    int outIdx;
    int num;
    int *chanceTbl;
    int gamePart;
    int totalChance;
    int temp;
    int turnSeg;
    int code;
    int chance;
    
    int list[CAPSULE_MAX-4];
    int outList[CAPSULE_MAX-4];
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
            inIdx = i%num;
            outIdx = MBCapsuleEffRand(num);
            if(inIdx != outIdx) {
                temp = list[inIdx];
                list[inIdx] = list[outIdx];
                list[outIdx] = temp;
            }
        }
        return list[MBCapsuleEffRand(num)];
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
        chance = MBCapsuleEffRandF()*totalChance;
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
                outList[idx] = list[i];
                idx++;
            }
        }
        if(idx <= 0) {
            return MBCapsuleNextGet(-1);
        }
        for(i=0; i<idx*5; i++) {
            inIdx = i%idx;
            outIdx = MBCapsuleEffRand(idx);
            if(inIdx != outIdx) {
                temp = outList[inIdx];
                outList[inIdx] = outList[outIdx];
                outList[outIdx] = temp;
            }
        }
        i = MBCapsuleEffRand(idx);
        return outList[i];
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
    chance = MBCapsuleEffRandF()*chanceTotal;
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
    chance = MBCapsuleEffRandF();
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

static void CapsuleYajiExec(OMOBJ *obj);

static void CapsuleYajiCreate(void)
{
    CAPSULE_YAJI_WORK *work;
    int i;
    work = capsuleYajiWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPSULE_YAJI_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(CAPSULE_YAJI_WORK));
    for(i=0; i<10; i++) {
        work->mdlId[i] = MBModelCreate(BOARD_HSF_yaji, NULL, TRUE);
        MBModelLayerSet(work->mdlId[i], 6);
        MBModelDispSet(work->mdlId[i], FALSE);
    }
    capsuleYajiObj = MBAddObj(32768, 0, 0, CapsuleYajiExec);
    capsuleYajiObj->work[0] = 0;
}

static void CapsuleYajiObjSet(int masuId, float rot)
{
    CAPSULE_YAJI_WORK *work = capsuleYajiWork;
    int i;
    HuVecF masuPos;
    HuVecF pos;
    if(capsuleYajiWork == NULL) {
        return;
    }
    MBMasuPosGet(MASU_LAYER_DEFAULT, masuId, &masuPos);
    for(i=0; i<10; i++) {
        if(MBModelDispGet(work->mdlId[i]) != TRUE) {
            int zOfs = 0;
            MBModelRotSet(work->mdlId[i], 0, rot, 0);
            pos.x = masuPos.x+(150*HuSin(rot));
            pos.y = masuPos.y+100;
            pos.z = (masuPos.z+(150*HuCos(rot)))+zOfs;
            MBModelPosSetV(work->mdlId[i], &pos);
            MBModelScaleSet(work->mdlId[i], 0, 0, 0);
            MBModelDispSet(work->mdlId[i], TRUE);
            MBModelLayerSet(work->mdlId[i], 3);
            MBModelAttrSet(work->mdlId[i], HU3D_ATTR_ZCMP_OFF);
            break;
        }
    }
    capsuleYajiObj->work[0] = 0;
    capsuleYajiObj->work[1] = 1;
}

static void CapsuleYajiExec(OMOBJ *obj)
{
    CAPSULE_YAJI_WORK *work = capsuleYajiWork;
    int i;
    HuVecF scale;
    if(MBKillCheck() || capsuleYajiObj == NULL) {
        for(i=0; i<10; i++) {
            MBModelKill(work->mdlId[i]);
        }
        HuMemDirectFree(capsuleYajiWork);
        capsuleYajiWork = NULL;
        omDelObj(mbObjMan, obj);
        capsuleYajiObj = NULL;
        return;
    }
    if(capsuleYajiObj->work[1]) {
        capsuleYajiObj->work[1]--;
        return;
    }
    for(i=0; i<10; i++) {
        if(MBModelDispGet(work->mdlId[i]) != FALSE) {
            MBModelScaleGet(work->mdlId[i], &scale);
            if(capsuleYajiObj->work[0] == FALSE) {
                if(scale.z < 2) {
                    scale.x = scale.y = scale.z += 0.3;
                } else {
                    scale.x = scale.y = scale.z = 2;
                }
            } else {
                if((scale.z -= 0.3f) < 0) {
                    MBModelDispSet(work->mdlId[i], FALSE);
                }
                scale.x = scale.y = scale.z;
            }
            MBModelScaleSetV(work->mdlId[i], &scale);
        }
    }
}

static void CapsuleYajiShrinkSet(void)
{
    CAPSULE_YAJI_WORK *work = capsuleYajiWork;
    if(capsuleYajiWork != NULL) {
        capsuleYajiObj->work[0] = TRUE;
    }
}

static void CapsuleYajiKill(void)
{
    capsuleYajiObj = NULL;
}

typedef struct ExplodeEffWork_s {
    int mdlId;
    int effNum;
    ANIMDATA *animP;
} EXPLODE_EFF_WORK;

static void ExplodeEffExec(OMOBJ *obj);

static void ExplodeEffInit(void)
{
    EXPLODE_EFF_WORK *work;
    OMOBJ *obj;
    CAPSULE_EFFECT *effP;
    HU3DMODEL *modelP;
    ANIMDATA *animP;
    int mdlId;
    obj = explodeEffObj = MBAddObj(32768, 0, 0, ExplodeEffExec);
    work = obj->data =HuMemDirectMallocNum(HEAP_HEAP, sizeof(EXPLODE_EFF_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(EXPLODE_EFF_WORK));
    work->animP = animP = HuSprAnimRead(HuDataReadNum(CAPSULE_ANM_effExplode, HU_MEMNUM_OVL));
    work->mdlId = mdlId = CapsuleEffCreate(animP, 64);
    Hu3DModelLayerSet(work->mdlId, 3);
    work->effNum = 0;
    modelP = &Hu3DData[work->mdlId];
    effP = modelP->hookData;
    effP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
}

static void ExplodeEffExec(OMOBJ *obj)
{
    CAPSULE_EFFDATA *effDataP;
    EXPLODE_EFF_WORK *work;
    CAPSULE_EFFECT *effP;
    int i;
    HU3DMODEL *modelP;
    work = obj->data;
    if(MBKillCheck() || explodeEffObj == NULL) {
        Hu3DModelKill(work->mdlId);
        work->mdlId = MB_MODEL_NONE;
        HuSprAnimKill(work->animP);
        work->animP = NULL;
        omDelObj(mbObjMan, obj);
        explodeEffObj = NULL;
        return;
    }
    if(work->effNum <= 0) {
        Hu3DModelDispOff(work->mdlId);
    } else {
        Hu3DModelDispOn(work->mdlId);
        modelP = &Hu3DData[work->mdlId];
        effP = modelP->hookData;
        effDataP = effP->data;
        effP->unk23 = 0;
        for(i=0; i<effP->num; i++, effDataP++) {
            if(effDataP->scale <= 0) {
                continue;
            }
            effDataP->pos.x += effDataP->vel.x;
            effDataP->pos.y += effDataP->vel.y;
            effDataP->pos.z += effDataP->vel.z;
            effDataP->rot.z += effDataP->speed;
            if(effDataP->rot.z >= 360) {
                effDataP->rot.z -= 360;
            }
            effDataP->no = effDataP->animTime;
            effDataP->animTime += effDataP->animSpeed;
            if(effDataP->no >= 16) {
                effDataP->no = 0;
                effDataP->time = 0;
                effDataP->scale = 0;
                work->effNum--;
            }
        }
    }
}

static void ExplodeEffClose(void)
{
    explodeEffObj = NULL;
}

static int ExplodeEffNumGet(void)
{
    OMOBJ *obj = explodeEffObj;
    EXPLODE_EFF_WORK *work;
    if(explodeEffObj == NULL) {
        return 0;
    } else {
        work = obj->data;
        return work->effNum;
    }
}

static int ExplodeEffAddData(HuVecF pos, HuVecF vel, float size, float speed, float radius, float animSpeed, GXColor color)
{
    CAPSULE_EFFDATA *effDataP;
    CAPSULE_EFFECT *effP;
    int i;
    EXPLODE_EFF_WORK *work;
    OMOBJ *obj;
    HU3DMODEL *modelP;
    
    
    obj = explodeEffObj;
    work = obj->data;
    modelP = &Hu3DData[work->mdlId];
    effP = modelP->hookData;
    effDataP=effP->data;
    for(i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->scale <= 0) {
            break;
        }
    }
    if(i >= effP->num) {
        return -1;
    }
    effDataP->time = effDataP->work = 0;
    effDataP->pos.x = pos.x;
    effDataP->pos.y = pos.y;
    effDataP->pos.z = pos.z;
    effDataP->vel.x = vel.x;
    effDataP->vel.y = vel.y;
    effDataP->vel.z = vel.z;
    effDataP->speed = speed;
    effDataP->scale = size;
    effDataP->color = color;
    effDataP->rot.z = 0;
    effDataP->no = 0;
    effDataP->time = 0;
    effDataP->animTime = 0;
    effDataP->animSpeed = animSpeed;
    work->effNum++;
    return i;
}

static int ExplodeEffAdd(HuVecF pos, HuVecF vel, float size, float speed, float radius, float animSpeed, GXColor color)
{
    HuVecF finalPos;
    HuVecF dir;
    
    float dist;
    int out1, out2;
    
    dir.x = vel.z;
    dir.z = vel.x;
    dir.y = 0;
    VECNormalize(&dir, &dir);
    dist = 0.5f*radius;
    
    finalPos.x = pos.x+(dir.x*dist);
    finalPos.y = pos.y+(dir.y*dist);
    finalPos.z = pos.z+(dir.z*dist);
    
    out1 = ExplodeEffAddData(finalPos, vel, size, speed, radius, animSpeed, color);
    
    finalPos.x = pos.x-(dir.x*dist);
    finalPos.y = pos.y-(dir.y*dist);
    finalPos.z = pos.z-(dir.z*dist);
    out2 = ExplodeEffAddData(finalPos, vel, size, -speed, radius, animSpeed, color);
    return (out1 << 16)|out2;
}

static void ExplodeEffAddAll(HuVecF pos)
{
    int i;
    for(i=0; i<32; i++) {
        HuVecF effPos;
        HuVecF effVel;
        GXColor effColor;
        float angle;
        float dist;
        angle = i*11.25f;
        dist = (MBCapsuleEffRandF()*100)*0.33f;
        effPos.x = pos.x+(dist*HuCos(angle));
        effPos.y = pos.y+(dist*HuSin(angle))+100;
        effPos.z = pos.z+50;
        dist = ((MBCapsuleEffRandF()*0.04f)+0.005f)*100;
        effVel.x = dist*HuCos(angle);
        effVel.y = dist*HuSin(angle);
        effVel.z = 0;
        dist = MBCapsuleEffRandF();
        
        effColor.r = 192+(32*dist);
        effColor.g = 192+(32*dist);
        effColor.b = 192+(32*dist);
        effColor.a = (MBCapsuleEffRandF()*63)+192;
        ExplodeEffAdd(effPos, effVel, ((MBCapsuleEffRandF()*0.5f)+1)*100, 0.5f, ((MBCapsuleEffRandF()*0.5f)+0.5f)*100, (MBCapsuleEffRandF()*0.66f)+0.33f, effColor);
    }
}

typedef struct GlowEffWork_s {
    int mdlId;
    int effNum;
    ANIMDATA *animP;
} GLOW_EFF_WORK;

static void GlowEffExec(OMOBJ *obj);

static void GlowEffInit(void)
{
    GLOW_EFF_WORK *work;
    OMOBJ *obj;
    CAPSULE_EFFECT *effP;
    HU3DMODEL *modelP;
    ANIMDATA *animP;
    int mdlId;
    obj = glowEffObj = MBAddObj(32768, 0, 0, GlowEffExec);
    work = obj->data =HuMemDirectMallocNum(HEAP_HEAP, sizeof(GLOW_EFF_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(GLOW_EFF_WORK));
    work->animP = animP = HuSprAnimRead(HuDataReadNum(CAPSULE_ANM_effCoin, HU_MEMNUM_OVL));
    work->mdlId = mdlId = CapsuleEffCreate(animP, 256);
    Hu3DModelLayerSet(work->mdlId, 3);
    work->effNum = 0;
    modelP = &Hu3DData[work->mdlId];
    effP = modelP->hookData;
    effP->blendMode = HU3D_PARTICLE_BLEND_ADDCOL;
    effP->dispAttr = CAPSULE_EFF_DISPATTR_NOANIM;
}

static void GlowEffExec(OMOBJ *obj)
{
    CAPSULE_EFFDATA *effDataP;
    GLOW_EFF_WORK *work;
    CAPSULE_EFFECT *effP;
    int i;
    HU3DMODEL *modelP;
    work = obj->data;
    if(MBKillCheck() || glowEffObj == NULL) {
        Hu3DModelKill(work->mdlId);
        work->mdlId = MB_MODEL_NONE;
        HuSprAnimKill(work->animP);
        work->animP = NULL;
        omDelObj(mbObjMan, obj);
        glowEffObj = NULL;
        return;
    }
    if(work->effNum <= 0) {
        Hu3DModelDispOff(work->mdlId);
    } else {
        Hu3DModelDispOn(work->mdlId);
        modelP = &Hu3DData[work->mdlId];
        effP = modelP->hookData;
        effDataP = effP->data;
        effP->unk23 = 0;
        for(i=0; i<effP->num; i++, effDataP++) {
            if(effDataP->scale <= 0) {
                continue;
            }
            effDataP->pos.x += effDataP->vel.x;
            effDataP->pos.y += effDataP->vel.y;
            effDataP->pos.z += effDataP->vel.z;
            if(effDataP->gravity) {
                effDataP->vel.y -= effDataP->gravity;
            }
            effDataP->color.a = effDataP->baseAlpha*effDataP->tpLvl;
            effDataP->tpLvl -= effDataP->speed;
            effDataP->rot.z += effDataP->rotSpeed;
            if(effDataP->tpLvl <= 0) {
                effDataP->no = 0;
                effDataP->time = 0;
                effDataP->scale = 0;
                work->effNum--;
            }
        }
    }
}

static void GlowEffClose(void)
{
    glowEffObj = NULL;
}

static int GlowEffNumGet(void)
{
    OMOBJ *obj = glowEffObj;
    GLOW_EFF_WORK *work;
    if(glowEffObj == NULL) {
        return 0;
    } else {
        work = obj->data;
        return work->effNum;
    }
}

static int GlowEffAdd(HuVecF pos, HuVecF vel, float scale, float fadeSpeed, float rotSpeed, float gravity, GXColor color)
{
    CAPSULE_EFFDATA *effDataP;
    CAPSULE_EFFECT *effP;
    int i;
    GLOW_EFF_WORK *work;
    OMOBJ *obj;
    HU3DMODEL *modelP;
    if(glowEffObj == NULL) {
        return -1;
    }
    obj = glowEffObj;
    work = obj->data;
    modelP = &Hu3DData[work->mdlId];
    effP = modelP->hookData;
    effDataP=effP->data;
    for(i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->scale <= 0) {
            break;
        }
    }
    if(i >= effP->num) {
        return -1;
    }
    effDataP->time = effDataP->work = 0;
    effDataP->pos.x = pos.x;
    effDataP->pos.y = pos.y;
    effDataP->pos.z = pos.z;
    effDataP->vel.x = vel.x;
    effDataP->vel.y = vel.y;
    effDataP->vel.z = vel.z;
    effDataP->baseAlpha = color.a;
    effDataP->tpLvl = 1;
    effDataP->speed = fadeSpeed;
    effDataP->gravity = gravity;
    effDataP->rotSpeed = rotSpeed;
    effDataP->scale = scale;
    effDataP->color = color;
    effDataP->rot.z = MBCapsuleEffRandF()*360;
    effDataP->no = 0;
    effDataP->time = 0;
    work->effNum++;
    return i;
}

typedef struct RingEffWork_s {
    int mdlId[3];
    int effNum;
    ANIMDATA *animP[3];
} RING_EFF_WORK;

static void RingEffExec(OMOBJ *obj);

static void RingEffInit(void)
{
    RING_EFF_WORK *work;
    int i;
    CAPSULE_EFFDATA *effDataP;
    CAPSULE_EFFECT *effP;
    OMOBJ *obj;
    HU3DMODEL *modelP;
    int mdlId;
    ANIMDATA *animP;
    static int ringFileTbl[3] = {
        CAPSULE_ANM_effSun,
        CAPSULE_ANM_effLens,
        CAPSULE_ANM_effLensDark
    };
    
    obj = ringEffObj = MBAddObj(32768, 0, 0, RingEffExec);
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(RING_EFF_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(RING_EFF_WORK));
    for(i=0; i<3; i++) {
        work->animP[i] = animP = HuSprAnimRead(HuDataReadNum(ringFileTbl[i], HU_MEMNUM_OVL));
        work->mdlId[i] = mdlId = CapsuleEffCreate(animP, 32);
        Hu3DModelLayerSet(mdlId, 3);
        work->effNum = 0;
        modelP = &Hu3DData[mdlId];
        effP = modelP->hookData;
        effP->blendMode = HU3D_PARTICLE_BLEND_ADDCOL;
        effP->dispAttr = CAPSULE_EFF_DISPATTR_ALL & ~CAPSULE_EFF_DISPATTR_ZBUF_OFF;
    }
}

static void RingEffExec(OMOBJ *obj)
{
    CAPSULE_EFFDATA *effDataP;
    RING_EFF_WORK *work;
    int i;
    CAPSULE_EFFECT *effP;
    int j;
    HU3DMODEL *modelP;
    float t;
    
    work = obj->data;
    if(MBKillCheck() || ringEffObj == NULL) {
        for(i=0; i<3; i++) {
            Hu3DModelKill(work->mdlId[i]);
            work->mdlId[i] = MB_MODEL_NONE;
        }
        for(i=0; i<3; i++) {
            HuSprAnimKill(work->animP[i]);
            work->animP[i] = NULL;
        }
        omDelObj(mbObjMan, obj);
        ringEffObj = NULL;
        return;
    }
    for(i=0; i<3; i++) {
        if(work->effNum <= 0) {
            Hu3DModelDispOff(work->mdlId[i]);
        } else {
            Hu3DModelDispOn(work->mdlId[i]);
            modelP = &Hu3DData[work->mdlId[i]];
            effP = modelP->hookData;
            effDataP = effP->data;
            effP->unk23 = 0;
            for(j=0; j<effP->num; j++, effDataP++) {
                if(effDataP->scale <= 0) {
                    continue;
                }
                switch(effDataP->time) {
                    case 0:
                        t = (++effDataP->work)/effDataP->baseAlpha;
                        switch(effDataP->mode) {
                            case 0:
                                (void)t;
                                break;
                                
                            case 1:
                                t = HuSin(t*90);
                                break;
                            
                            case 2:
                                t = HuCos((1-t)*90);
                                break;
                        }
                        effDataP->scale = effDataP->vel.z*(effDataP->vel.x+(t*(1-effDataP->vel.x)));
                        effDataP->color.a = effDataP->speed*t;
                        if(t >= 1) {
                            effDataP->scale = effDataP->vel.z;
                            effDataP->color.a = effDataP->speed;
                            effDataP->time++;
                            effDataP->work = 0;
                        }
                        break;
                    
                    case 1:
                        t = (++effDataP->work)/effDataP->tpLvl;
                        switch(effDataP->mode) {
                            case 0:
                                (void)t;
                                break;
                                
                            case 1:
                                t = HuSin(t*90);
                                break;
                            
                            case 2:
                                t = HuCos((1-t)*90);
                                break;
                        }
                        effDataP->scale = effDataP->vel.z*(1+(t*(effDataP->vel.y-1)));
                        effDataP->color.a = effDataP->speed*(1-t);
                        if(t >= 1) {
                            effDataP->scale = 0;
                            work->effNum--;
                        }
                        break;
                }
            }
        }
    }
}

static void RingEffClose(void)
{
    ringEffObj = NULL;
}

static int RingEffNumGet(void)
{
    OMOBJ *obj = ringEffObj;
    RING_EFF_WORK *work;
    if(ringEffObj == NULL) {
        return 0;
    } else {
        work = obj->data;
        return work->effNum;
    }
}

static void RingEffDispAttrSet(int dispAttr)
{
    int i;
    OMOBJ *obj;
    RING_EFF_WORK *work;
    CAPSULE_EFFECT *effP;
    HU3DMODEL *modelP;
    obj = ringEffObj;
    if(ringEffObj == NULL) {
        return;
    } else {
        work = obj->data;
        for(i=0; i<3; i++) {
            modelP = &Hu3DData[work->mdlId[i]];
            effP = modelP->hookData;
            effP->dispAttr = dispAttr;
        }
    }
}

static int RingEffAdd(HuVecF pos, HuVecF rot, HuVecF vel, int inTime, int rotTime, int no, int mode, GXColor color)
{
    CAPSULE_EFFDATA *effDataP;
    CAPSULE_EFFECT *effP;
    int i;
    RING_EFF_WORK *work;
    OMOBJ *obj;
    HU3DMODEL *modelP;
    if(ringEffObj == NULL) {
        return -1;
    }
    obj = ringEffObj;
    work = obj->data;
    modelP = &Hu3DData[work->mdlId[no%3]];
    effP = modelP->hookData;
    effDataP = effP->data;
    for(i=0; i<effP->num; i++, effDataP++) {
        if(effDataP->scale <= 0) {
            break;
        }
    }
    if(i >= effP->num) {
        return -1;
    }
    effDataP->time = effDataP->work = 0;
    effDataP->pos.x = pos.x;
    effDataP->pos.y = pos.y;
    effDataP->pos.z = pos.z;
    effDataP->vel.x = vel.x;
    effDataP->vel.y = vel.y;
    effDataP->vel.z = vel.z;
    effDataP->baseAlpha = inTime;
    effDataP->tpLvl = rotTime;
    effDataP->speed = color.a;
    effDataP->scale = vel.z;
    effDataP->color = color;
    effDataP->rot.x = rot.x;
    effDataP->rot.y = rot.y;
    effDataP->rot.z = rot.z;
    effDataP->no = 0;
    effDataP->time = 0;
    effDataP->mode = mode;
    work->effNum++;
    return i;
}

typedef struct DomeEffData_s {
    BOOL flag;
    float scale;
    float angle;
    float angleSpeed;
    float scaleSpeed;
    int delay;
    HuVecF prevVel;
    HuVecF prevPos[3];
    HuVecF accel;
    HuVecF vel;
    HuVecF pos[3];
    HuVec2F uv[3];
    GXColor color;
} DOME_EFF_DATA;

typedef struct DomeEffect_s {
    int mdlId;
    int mode;
    int time;
    int num;
    int vtxNum;
    int segNum;
    HuVecF *vtx;
    HuVec2F *st;
    DOME_EFF_DATA *data;
    ANIMDATA *animP;
    GXColor color;
    u32 dlSize;
    void *dl;
} DOME_EFFECT;

static void DomeEffExec(OMOBJ *obj);
static void DomeEffDraw(HU3DMODEL *modelP, Mtx *mtx);

static void DomeEffInit(void)
{
    DOME_EFF_DATA *effDataP;
    DOME_EFFECT *effP;
    int vtxTri;
    int i;
    HU3DMODEL *modelP;
    int k;
    int j;
    void *dlBuf;
    OMOBJ *obj;
    void *dlBegin;
    
    float yOfs;
    float y;
    float ofsX;
    float ofsZ;
    float scale;
    float scaleX;
    float scaleZ;
    
    HuVecF vtxPos;
    float scaleTbl[3];
    static HuVecF vtxTbl[2][3] = {
        {
            { 0, 0, 0 },
            { 1, 0, 0 },
            { 0, 0, 1 },
        },
        {
            { 0, 0, 1 },
            { 1, 0, 0 },
            { 1, 0, 1 },
        },
    };
    static GXColor color = { 255, 255, 128, 64 };
    
    obj = domeEffObj = MBAddObj(32768, 0, 0, DomeEffExec);
    effP = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(DOME_EFFECT), HU_MEMNUM_OVL);
    memset(effP, 0, sizeof(DOME_EFFECT));
    effP->mdlId = Hu3DHookFuncCreate(DomeEffDraw);
    Hu3DModelCameraSet(effP->mdlId, HU3D_CAM0);
    Hu3DModelLayerSet(effP->mdlId, 2);
    modelP = &Hu3DData[effP->mdlId];
    modelP->hookData = effP;
    effP->mode = 0;
    effP->time = 0;
    effP->num = 1152;
    effP->vtxNum = effP->num*3;
    effDataP = effP->data = HuMemDirectMallocNum(HEAP_MODEL, effP->num*sizeof(DOME_EFF_DATA), modelP->mallocNo);
    memset(effDataP, 0, effP->num*sizeof(DOME_EFF_DATA));
    effP->vtx = HuMemDirectMallocNum(HEAP_MODEL, effP->vtxNum*sizeof(HuVecF), modelP->mallocNo);
    memset(effP->vtx, 0, effP->vtxNum*sizeof(HuVecF));
    effP->st = HuMemDirectMallocNum(HEAP_MODEL, effP->vtxNum*sizeof(HuVec2F), modelP->mallocNo);
    memset(effP->st, 0, effP->vtxNum*sizeof(HuVec2F));
    effP->animP = HuSprAnimRead(HuDataReadNum(CAPSULE_ANM_effCrack, HU_MEMNUM_OVL));
    effP->color.r = 0;
    effP->color.g = 0;
    effP->color.b = 0;
    effP->color.a = 0;
    scaleX = 8.333334f;
    scaleZ = 8.333334f;
    ofsX = -100;
    ofsZ = -100;
    for(i=0; i<24; i++) {
        for(j=0; j<24; j++) {
            for(k=0; k<2; effDataP++, k++) {
                effDataP->flag = TRUE;
                effDataP->scale = 1;
                effDataP->angle = 0;
                if(frand() & 0x1) {
                    effDataP->angleSpeed = (MBCapsuleEffRandF()*2)+5;
                } else {
                    effDataP->angleSpeed = -((MBCapsuleEffRandF()*2)+5);
                }
                effDataP->scaleSpeed = (MBCapsuleEffRandF()*0.035f)+0.035f;
                effDataP->delay = frand() & 0xF;
                for(vtxTri=0; vtxTri<3; vtxTri++) {
                    effDataP->pos[vtxTri].x = ofsX+(scaleX*vtxTbl[k][vtxTri].x);
                    effDataP->pos[vtxTri].z = ofsZ+(scaleZ*vtxTbl[k][vtxTri].z);
                    effDataP->pos[vtxTri].y = 0;
                    scale = HuMagPoint2D(effDataP->pos[vtxTri].x, effDataP->pos[vtxTri].z)/100;
                    scaleTbl[vtxTri] = scale;
                    if(scaleTbl[vtxTri] > 1) {
                        scale = 1;
                        VECNormalize(&effDataP->pos[vtxTri], &effDataP->pos[vtxTri]);
                        VECScale(&effDataP->pos[vtxTri], &effDataP->pos[vtxTri], 100);
                    }
                    yOfs = 100;
                    y = 100*scale;
                    effDataP->pos[vtxTri].y = sqrtf((yOfs*yOfs)-(y*y));
                    yOfs = 100;
                    effDataP->uv[vtxTri].x = (effDataP->pos[vtxTri].x+yOfs)/200;
                    effDataP->uv[vtxTri].y = (effDataP->pos[vtxTri].z+yOfs)/200;
                }
                vtxPos = effDataP->pos[0];
                VECAdd(&vtxPos, &effDataP->pos[1], &vtxPos);
                VECAdd(&vtxPos, &effDataP->pos[2], &vtxPos);
                VECScale(&vtxPos, &effDataP->vel, 0.333333f);
                yOfs = HuMagPoint2D(effDataP->vel.x, effDataP->vel.z)/100;
                effDataP->delay = (int)(yOfs*yOfs*30)+(frand() & 0x7);
                VECNormalize(&effDataP->vel, &effDataP->accel);
                VECScale(&effDataP->accel, &effDataP->accel, ((MBCapsuleEffRandF()*0.5f)+0.5f)*10);
                effDataP->prevVel = effDataP->vel;
                for(vtxTri=0; vtxTri<3; vtxTri++) {
                    VECSubtract(&effDataP->pos[vtxTri], &effDataP->vel, &effDataP->pos[vtxTri]);
                    effDataP->prevPos[vtxTri] = effDataP->pos[vtxTri];
                }
                effDataP->color = color;
                if(scaleTbl[0] > 1 && scaleTbl[1] > 1 && scaleTbl[2] > 1) {
                    effDataP->flag = FALSE;
                    effDataP->scale = 0;
                }
            }
            ofsX += scaleX;
        }
        ofsX = -100;
        ofsZ += scaleZ;
    }
    dlBegin = dlBuf = HuMemDirectMallocNum(HEAP_MODEL, 0x10000, modelP->mallocNo);
    DCFlushRange(dlBuf, 0x10000);
    GXBeginDisplayList(dlBegin, 0x10000);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, effP->vtxNum);
    for(i=0; i<effP->vtxNum/3; i++) {
        GXPosition1x16(i*3);
        GXColor1x16(i);
        GXTexCoord1x16(i*3);
        
        GXPosition1x16((i*3)+1);
        GXColor1x16(i);
        GXTexCoord1x16((i*3)+1);
        
        GXPosition1x16((i*3)+2);
        GXColor1x16(i);
        GXTexCoord1x16((i*3)+2);
    }
    GXEnd();
    effP->dlSize = GXEndDisplayList();
    effP->dlSize > 0x10000;
    effP->dl = HuMemDirectMallocNum(HEAP_MODEL, effP->dlSize, modelP->mallocNo);
    memcpy(effP->dl, dlBuf, effP->dlSize);
    DCFlushRange(effP->dl, effP->dlSize);
    HuMemDirectFree(dlBuf);
}

static void RayEffAdd(float angleY, float angleX, float radiusMin, float fadeTime);
static void RayEffMdlUpdate(float x, float y, float z);
static void RayEffModeSet(int mode);

static void DomeEffExec(OMOBJ *obj)
{
    DOME_EFFECT *effP;
    DOME_EFF_DATA *effDataP;
    int i;
    int j;
    int no;
    float scale;
    
    HuVecF ringPos;
    HuVecF ringVel;
    static GXColor ringCol[3] = {
        { 255, 128, 128, 255 },
        { 128, 128, 255, 255 },
        { 255, 128, 255, 255 },
    };
    static HuVecF ringRot[3] = {
        { -90, 0, 30 },
        { -90, 0, -30 },
        { -90, 0, 0 },
    };
    static float ringVelZ[3] = {
        84,
        78,
        80
    };
    static int ringNo[3] = {
        1,
        2,
        1
    };
    
    no = -1;
    effP = obj->data;
    
    if(MBKillCheck() || domeEffObj == NULL) {
        HuSprAnimKill(effP->animP);
        Hu3DModelKill(effP->mdlId);
        omDelObj(mbObjMan, obj);
        domeEffObj = NULL;
        return;
    }
    effDataP = effP->data;
    if(effP->mode >= 2 && effP->mode <= 4 && effP->segNum < 16) {
        RayEffAdd(effP->segNum*22, (MBCapsuleEffRandF()*20)+30, ((MBCapsuleEffRandF()*0.25f)+0.3f)*300, 50);
        RayEffMdlUpdate(obj->trans.x, obj->trans.y, obj->trans.z);
        effP->segNum++;
    }
    switch(effP->mode) {
        case 0:
            break;
            
        case 1:
            for(i=0, effDataP=effP->data; i<effP->num; i++, effDataP++) {
                effDataP->color.a =64;
                effDataP->vel.y = 0;
                for(j=0; j<3; j++) {
                    effDataP->pos[j].y = 0;
                }
            }
            Hu3DModelScaleSet(effP->mdlId, 0, 1, 0);
            no = 0;
            effP->time = 0;
            effP->mode++;
            break;
            
        case 2:
            scale = (++effP->time)/6.0f;
            if(scale >= 1) {
                scale = 1;
            }
            Hu3DModelScaleSet(effP->mdlId, scale, 1, scale);
            if(effP->time == 3) {
                no = 1;
            }
            if(effP->time >= 6.0f) {
                RayEffModeSet(1);
                effP->time = 0;
                effP->mode++;
            }
            break;
        
        case 3:
            scale = (++effP->time)/6.0f;
            if(scale >= 1) {
                scale = 1;
            }
            for(i=0, effDataP=effP->data; i<effP->num; i++, effDataP++) {
                effDataP->accel.x *= 0.33f;
                effDataP->accel.z *= 0.33f;
                effDataP->color.a = 64+(scale*191);
                effDataP->vel.y = effDataP->prevVel.y*scale;
                for(j=0; j<3; j++) {
                    effDataP->pos[j].y = effDataP->prevPos[j].y*scale;
                }
            }
            effP->color.r = effP->color.g = effP->color.b = scale*64;
            effP->color.a = scale*255;
            if(effP->time >= 6.0f) {
                RayEffModeSet(2);
                effP->time = 0;
                effP->mode++;
            }
            break;
            
        case 4:
            if(effP->color.a > 20) {
                effP->color.a -= 20;
            } else {
                effP->color.a = 0;
            }
            for(i=0, effDataP=effP->data; i<effP->num; i++, effDataP++) {
                if(effDataP->flag) {
                    if(effDataP->delay) {
                        effDataP->delay--;
                        effDataP->accel.y = (931.0/3000.0);
                    } else {
                        effDataP->vel.y += effDataP->accel.y;
                        effDataP->accel.y += (931.0/3000.0);
                        effDataP->angle += effDataP->angleSpeed;
                        if((effDataP->scale -= effDataP->scaleSpeed) <= 0 || effDataP->vel.y < 0){
                            effDataP->flag = FALSE;
                        } else if(effDataP->scale > 1) {
                            effDataP->color.a = 255;
                        } else {
                            effDataP->color.a = effDataP->scale*255;
                        }
                    }
                }
            }
            break;
            
        case 5:
            break;
    }
    if(no >= 0) {
        ringPos.x = obj->trans.x;
        ringPos.y = obj->trans.y;
        ringPos.z = obj->trans.z;
        ringVel.x = 1;
        ringVel.y = 8.5f;
        ringVel.z = ringVelZ[no];
        RingEffAdd(ringPos, ringRot[no], ringVel, 1, 30, ringNo[no], 0, ringCol[no]);
    }
}

static void DomeEffClose(void)
{
    domeEffObj = NULL;
}

static void DomeEffMdlUpdate(HuVecF *pos, HuVecF *rot)
{
    OMOBJ *obj;
    DOME_EFFECT *effP;
    obj = domeEffObj;
    if(domeEffObj != NULL) {
        effP = obj->data;
        effP->mode = 1;
        if(pos != NULL) {
            obj->trans.x = pos->x;
            obj->trans.y = pos->y;
            obj->trans.z = pos->z;
        } else {
            obj->trans.x = obj->trans.y = obj->trans.z = 0;
        }
        if(rot != NULL) {
            obj->rot.x = rot->x;
            obj->rot.y = rot->y;
            obj->rot.z = rot->z;
        } else {
            obj->rot.x = obj->rot.y = obj->rot.z = 0;
        }
        Hu3DModelPosSet(effP->mdlId, obj->trans.x, obj->trans.y, obj->trans.z);
        Hu3DModelRotSet(effP->mdlId, obj->rot.x, obj->rot.y, obj->rot.z);
    }
}

static void DomeEffDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    DOME_EFFECT *effP;
    DOME_EFF_DATA *effDataP;
    int j;
    HuVecF *pos;
    HuVec2F *st;
    int i;
    float s;
    float c;
    HuVecF outPos;
    static GXColor matColor = { 255, 255, 255, 255 };
    static GXColor ambColor = { 255, 255, 255, 255 };
    
    
    effP = modelP->hookData;
    effDataP = effP->data;
    if(effP->mode == 0) {
        return;
    }
    GXLoadPosMtxImm(*mtx, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    if(shadowModelDrawF) {
        return;
    }
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetTevColor(GX_ALPHA1, effP->color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_C2);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_A2);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetChanAmbColor(GX_COLOR0A0, ambColor);
    GXSetChanMatColor(GX_COLOR0A0, matColor);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_CLAMP, GX_AF_NONE);
    HuSprTexLoad(effP->animP, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_NONE);

    for(pos=effP->vtx, st=effP->st, i=0; i<effP->num; i++, effDataP++){
        if(effDataP->flag) {
            if(effDataP->angle == 0) {
                for(j=0; j<3; j++) {
                    VECScale(&effDataP->pos[j], &outPos, effDataP->scale);
                    VECAdd(&outPos, &effDataP->vel, pos);
                    pos++;
                    *st = effDataP->uv[j];
                    st++;
                }
            } else {
                s = HuSin(effDataP->angle);
                c = HuCos(effDataP->angle);
                for(j=0; j<3; j++) {
                    VECScale(&effDataP->pos[j], &outPos, effDataP->scale);
                    pos->x = (effDataP->vel.x+effDataP->pos[j].x*c)-(effDataP->pos[j].y*s);
                    pos->y = (effDataP->vel.y+effDataP->pos[j].x*s)+(effDataP->pos[j].y*c);
                    pos->z = effDataP->vel.z+effDataP->pos[j].z;
                    pos++;
                    *st = effDataP->uv[j];
                    st++;
                }
            }
        } else {
            for(j=0; j<3; j++) {
                pos->x = pos->y = pos->z = 0;
                pos++;
            }
            st += 3;
        }
    }
    effDataP = effP->data;
    DCFlushRangeNoSync(effP->vtx, effP->vtxNum*sizeof(HuVecF));
    DCFlushRangeNoSync(effP->st, effP->vtxNum*sizeof(HuVec2F));
    DCFlushRangeNoSync(effP->data, effP->num*sizeof(DOME_EFF_DATA));
    PPCSync();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, effP->vtx, sizeof(HuVecF));
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetArray(GX_VA_CLR0, &effP->data->color, sizeof(DOME_EFF_DATA));
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, effP->st, sizeof(HuVec2F));
    GXCallDisplayList(effP->dl, effP->dlSize);
}

typedef struct RayEffData_s {
    BOOL flag;
    float time;
    float radiusMin;
    float fadeTime;
    HuVecF rot;
    HuVecF dir;
    HuVecF pos[4];
    HuVec2F st[4];
    GXColor color;
} RAY_EFF_DATA;

typedef struct RayEffect_s {
    int mdlId;
    int mode;
    int time;
    int num;
    int vtxNum;
    HuVecF *vtx;
    HuVec2F *st;
    RAY_EFF_DATA *data;
    ANIMDATA *animP;
    u32 dlSize;
    void *dl;
} RAY_EFFECT;

static void RayEffExec(OMOBJ *obj);
static void RayEffDraw(HU3DMODEL *modelP, Mtx *mtx);

static void RayEffInit(void)
{
    RAY_EFFECT *effP;
    RAY_EFF_DATA *effDataP;
    int j;
    HU3DMODEL *modelP;
    int k;
    int i;
    void *dlBuf;
    RAY_EFFECT *effFirstP;
    OMOBJ *obj;
    void *dlBegin;
    static GXColor colTbl[2] = {
        { 0, 255, 0, 255 },
        { 64, 255, 64, 255 },
    };
    obj = rayEffObj = MBAddObj(32768, 0, 0, RayEffExec);
    effFirstP = effP  = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(RAY_EFFECT)*2, HU_MEMNUM_OVL);
    memset(effP, 0, sizeof(RAY_EFFECT)*2);
    for(i=0; i<2; i++, effP++) {
        effP->mdlId = Hu3DHookFuncCreate(RayEffDraw);
        Hu3DModelCameraSet(effP->mdlId, HU3D_CAM0);
        Hu3DModelLayerSet(effP->mdlId, 3);
        modelP = &Hu3DData[effP->mdlId];
        modelP->hookData = effP;
        effP->mode = 0;
        effP->time = 0;
        effP->num = 16;
        effP->vtxNum = effP->num*4;
        effDataP = effP->data = HuMemDirectMallocNum(HEAP_MODEL, effP->num*sizeof(RAY_EFF_DATA), modelP->mallocNo);
        memset(effP->data, 0, effP->num*sizeof(RAY_EFF_DATA));
        effP->vtx = HuMemDirectMallocNum(HEAP_MODEL, effP->vtxNum*sizeof(HuVecF), modelP->mallocNo);
        memset(effP->vtx, 0, effP->vtxNum*sizeof(HuVecF));
        effP->st = HuMemDirectMallocNum(HEAP_MODEL, effP->vtxNum*sizeof(HuVec2F), modelP->mallocNo);
        memset(effP->st, 0, effP->vtxNum*sizeof(HuVec2F));
        if(i == 0) {
            effP->animP = HuSprAnimRead(HuDataReadNum(CAPSULE_ANM_effRay, HU_MEMNUM_OVL));
        } else {
            effP->animP = effFirstP->animP;
        }
        for(j=0; j<effP->num; j++, effDataP++) {
            effDataP->flag = FALSE;
            effDataP->time = 1;
            effDataP->radiusMin = 0;
            effDataP->fadeTime = 0;
            effDataP->rot.x = effDataP->rot.y = effDataP->rot.z = 0;
            effDataP->dir.x = effDataP->dir.y = effDataP->dir.z = 0;
            for(k=0; k<4; k++) {
                effDataP->pos[k].x = effDataP->pos[k].y = effDataP->pos[k].z = 0;
                effDataP->st[k].x = effDataP->st[k].y = 0;
            }
            effDataP->color = colTbl[i];
        }
        dlBegin = dlBuf = HuMemDirectMallocNum(HEAP_MODEL, 0x10000, modelP->mallocNo);
        DCFlushRange(dlBuf, 0x10000);
        GXBeginDisplayList(dlBegin, 0x10000);
        GXBegin(GX_QUADS, GX_VTXFMT0, effP->vtxNum);
        for(j=0; j<effP->vtxNum/4; j++) {
            GXPosition1x16(j*4);
            GXColor1x16(j);
            GXTexCoord1x16(j*4);
            
            GXPosition1x16((j*4)+1);
            GXColor1x16(j);
            GXTexCoord1x16((j*4)+1);
            
            GXPosition1x16((j*4)+2);
            GXColor1x16(j);
            GXTexCoord1x16((j*4)+2);
            
            GXPosition1x16((j*4)+3);
            GXColor1x16(j);
            GXTexCoord1x16((j*4)+3);
        }
        GXEnd();
        effP->dlSize = GXEndDisplayList();
        effP->dlSize > 0x10000;
        effP->dl = HuMemDirectMallocNum(HEAP_MODEL, effP->dlSize, modelP->mallocNo);
        memcpy(effP->dl, dlBuf, effP->dlSize);
        DCFlushRange(effP->dl, effP->dlSize);
        HuMemDirectFree(dlBuf);
    }
}

static void RayEffExec(OMOBJ *obj)
{
    RAY_EFFECT *effP;
    RAY_EFF_DATA *effDataP;
    int i;
    int j;
    effP = obj->data;
    if(MBKillCheck() || rayEffObj == NULL) {
        HuSprAnimKill(effP->animP);
        for(i=0; i<2; i++, effP++) {
            Hu3DModelKill(effP->mdlId);
        }
        HuMemDirectFree(obj->data);
        obj->data = NULL;
        omDelObj(mbObjMan, obj);
        rayEffObj = NULL;
        return;
    }
    for(j=0; j<2; j++, effP++) {
        for(effDataP=effP->data, i=0; i<effP->num; i++, effDataP++) {
            if(effDataP->flag) {
                if(effDataP->time < 1) {
                    effDataP->time += 0.05f;
                }
                if(effP->mode >= 2) {
                    effDataP->fadeTime--;
                    if(effDataP->fadeTime <= 0.1) {
                        effDataP->flag = FALSE;
                    } else if(effDataP->fadeTime < 50) {
                        effDataP->color.a = 255*(effDataP->fadeTime/50);
                    }
                }
            }
        }
        if(effP->mode >= 1) {
            if(j == 0) {
                obj->rot.y += 2;
            }
            if(j == 0) {
                Hu3DModelRotSet(effP->mdlId, 0, obj->rot.y, 0);
            } else {
                Hu3DModelRotSet(effP->mdlId, 0, -obj->rot.y, 0);
            }
        }
    }
}

static void RayEffClose(void)
{
    rayEffObj = NULL;
}

static void RayEffAdd(float angleY, float angleX, float radiusMin, float fadeTime)
{
    RAY_EFF_DATA *effDataP;
    RAY_EFFECT *effP;
    int j;
    int i;
    OMOBJ *obj;
    obj = rayEffObj;
    if(rayEffObj == NULL) {
        return;
    }
    for(effP=obj->data, i=0; i<2; i++, effP++) {
        for(effDataP=effP->data, j=0; j<effP->num; j++, effDataP++){
            if(!effDataP->flag) {
                break;
            }
        }
        if(j >= effP->num) {
            continue;
        }
        effDataP->flag = TRUE;
        effDataP->time = 0;
        effDataP->radiusMin = radiusMin;
        effDataP->fadeTime = fadeTime;
        effDataP->color.r += (u8)(MBCapsuleEffRandF()*64);
        effDataP->color.b += (u8)(MBCapsuleEffRandF()*64);
        effDataP->rot.x = angleX;
        effDataP->rot.y = angleY;
        effDataP->rot.z = 0;
        effDataP->dir.x = HuCos(effDataP->rot.y)*HuCos(effDataP->rot.x);
        effDataP->dir.y = HuSin(effDataP->rot.x);
        effDataP->dir.z = HuSin(effDataP->rot.y)*HuCos(effDataP->rot.x);
        VECNormalize(&effDataP->dir, &effDataP->dir);
    }
}

static void RayEffMdlUpdate(float x, float y, float z)
{
    RAY_EFFECT *effP;
    int i;
    OMOBJ *obj;
    RAY_EFF_DATA *effDataP;
    obj = rayEffObj;
    if(rayEffObj == NULL) {
        return;
    }
    effP = obj->data;
    effDataP = effP->data;
    for(i=0; i<2; i++, effP++) {
        Hu3DModelPosSet(effP->mdlId, x, y, z);
    }
}

static void RayEffModeSet(int mode)
{
    RAY_EFFECT *effP;
    int i;
    OMOBJ *obj;
    RAY_EFF_DATA *effDataP;
    obj = rayEffObj;
    if(rayEffObj == NULL) {
        return;
    }
    effP = obj->data;
    effDataP = effP->data;
    for(i=0; i<2; i++, effP++) {
        effP->mode = mode;
    }
}

static void RayEffDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    RAY_EFFECT *effP;
    RAY_EFF_DATA *effDataP;
    HuVecF *pos;
    HuVec2F *st;
    int i;
    int j;
    
    HuVecF basePos;
    HuVecF ofs;
    
    static GXColor matColor = { 255, 255, 255, 255 };
    static GXColor ambColor = { 255, 255, 255, 255 };
    
    effP = modelP->hookData;
    effDataP = effP->data;
    GXLoadPosMtxImm(*mtx, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    if(shadowModelDrawF) {
        return;
    }
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetChanAmbColor(GX_COLOR0A0, ambColor);
    GXSetChanMatColor(GX_COLOR0A0, matColor);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_CLAMP, GX_AF_NONE);
    HuSprTexLoad(effP->animP, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_NONE);
    for(pos=effP->vtx, st=effP->st, i=0; i<effP->num; i++, effDataP++){
        if(effDataP->flag) {
            float s = HuSin(effDataP->rot.y+90);
            float c = HuCos(effDataP->rot.y+90);
            ofs.x = 120.00001f*effDataP->dir.x;
            ofs.y = 0;
            ofs.z = 120.00001f*effDataP->dir.z;
            pos[0].x = ofs.x-(c*effDataP->fadeTime);
            pos[0].y = 0;
            pos[0].z = ofs.z-(s*effDataP->fadeTime);
            
            pos[1].x = ofs.x+(c*effDataP->fadeTime);
            pos[1].y = 0;
            pos[1].z = ofs.z+(s*effDataP->fadeTime);
            VECScale(&effDataP->dir, &basePos, effDataP->time*effDataP->radiusMin);
            VECAdd(&basePos, &ofs, &basePos);
            
            pos[2].x = basePos.x+(c*effDataP->fadeTime*0.5f);
            pos[2].y = basePos.y;
            pos[2].z = basePos.z+(s*effDataP->fadeTime*0.5f);
            
            pos[3].x = basePos.x-(c*effDataP->fadeTime*0.5f);
            pos[3].y = basePos.y;
            pos[3].z = basePos.z-(s*effDataP->fadeTime*0.5f);
            
            st[0].x = 1;
            st[0].y = 1;
            
            st[1].x = 0;
            st[1].y = 1;
            
            st[2].x = 0;
            st[2].y = 0;
            
            st[3].x = 1;
            st[3].y = 0;
            
            pos += 4;
            st += 4;
        } else {
            for(j=0; j<4; j++) {
                pos->x = pos->y = pos->z = 0;
                pos++;
            }
            st += 4;
        }
    }
    effDataP = effP->data;
    DCFlushRangeNoSync(effP->vtx, effP->vtxNum*sizeof(HuVecF));
    DCFlushRangeNoSync(effP->st, effP->vtxNum*sizeof(HuVec2F));
    DCFlushRangeNoSync(effP->data, effP->num*sizeof(RAY_EFF_DATA));
    PPCSync();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, effP->vtx, sizeof(HuVecF));
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetArray(GX_VA_CLR0, &effP->data->color, sizeof(RAY_EFF_DATA));
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, effP->st, sizeof(HuVec2F));
    GXCallDisplayList(effP->dl, effP->dlSize);
}

typedef struct ThrowEffWork_s {
    HuVecF prevPos[12];
} THROW_EFF_WORK;

static void ThrowEffExec(OMOBJ *obj);

static void ThrowEffInit(int capsuleNo)
{
    OMOBJ *obj;
    int i;
    MBEFFECTDATA *effDataP;
    THROW_EFF_WORK *work;
    MBEFFECT *effP;
    HU3DMODEL *modelP;
    
    capsuleNo &= 0xFF;
    
    obj = throwEffObj = MBAddObj(32768, 13, 0, ThrowEffExec);
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(THROW_EFF_WORK), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(THROW_EFF_WORK));
    for(i=0; i<12; i++) {
        obj->mdlId[i] = MB_MODEL_NONE;
        work->prevPos[i].x = work->prevPos[i].y = work->prevPos[i].z = 0;
    }
    obj->mdlId[12] = MBEffCreate(HuSprAnimRead(HuDataReadNum(CAPSULE_ANM_effDot, HU_MEMNUM_OVL)), 24);
    Hu3DModelCameraSet(obj->mdlId[12], HU3D_CAM0);
    Hu3DModelLayerSet(obj->mdlId[12], 3);
    modelP = &Hu3DData[obj->mdlId[12]];
    effP = modelP->hookData;
    effDataP = effP->data;
    effP->blendMode = MB_EFFECT_BLEND_ADDCOL;
    for(i=0; i<effP->num; i++, effDataP++) {
        effDataP->scale = 0;
        effDataP->pos.x = effDataP->pos.y = effDataP->pos.z = 0;
    }
    obj->stat |= OM_STAT_MODELPAUSE;
    obj->work[0] = 0;
    obj->work[1] = 0;
}

typedef struct ThrowTrailPoint_s {
    HuVecF start;
    HuVecF end;
    float mag;
    float totalMag;
} THROW_TRAIL_POINT;

static void ThrowEffExec(OMOBJ *obj)
{
    int i; //r31
    THROW_TRAIL_POINT *trailPoint; //r29
    MBEFFECTDATA *effDataP; //r28
    THROW_EFF_WORK *work; //r27
    int pointNo; //r24
    MBEFFECT *effP; //r23
    int trailPointNo; //r19
    
    float totalMag; //f31
    float nextDist; //f30
    float scale; //f29
    
    THROW_TRAIL_POINT trailData[12]; //sp+0xD8
    HuVecF pointHis[12]; //sp+0x48
    HuVecF dir; //sp+0x3C
    HU3DMODEL *modelP; //sp+0x38
    int j; //sp+0x34
    int nextTrailPoint; //sp+0x30
    
    work = obj->data;
    if(MBKillCheck() || throwEffObj == NULL) {
        for(i=0; i<12; i++) {
            if(obj->mdlId[i] != MB_MODEL_NONE) {
                MBCapsuleObjKill(obj->mdlId[i]);
            }
            obj->mdlId[i] = MB_MODEL_NONE;
        }
        MBEffKill(obj->mdlId[12]);
        obj->mdlId[12] = HU3D_MODELID_NONE;
        omDelObj(mbObjMan, obj);
        throwEffObj = NULL;
        return;
    }
    if(!obj->work[0]) {
        return;
    }
    pointNo = obj->work[1];
    work->prevPos[pointNo].x = obj->trans.x;
    work->prevPos[pointNo].y = obj->trans.y;
    work->prevPos[pointNo].z = obj->trans.z;
    for(trailPoint=trailData, totalMag=0, i=0; i<11; i++, trailPoint++) {
        if((trailPointNo = i+obj->work[1]) >= 12) {
            trailPointNo -= 12;
        }
        if((nextTrailPoint = trailPointNo+1) >= 12) {
            nextTrailPoint -= 12;
        }
        VECSubtract(&work->prevPos[trailPointNo], &work->prevPos[nextTrailPoint], &dir);
        trailPoint->start = work->prevPos[trailPointNo];
        trailPoint->end = work->prevPos[nextTrailPoint];
        trailPoint->mag = VECMag(&dir);
        trailPoint->totalMag = totalMag;
        totalMag += trailPoint->mag;
    }
    for(i=0; i<12; i++) {
        if(totalMag <= 0) {
            nextDist = 0;
        } else if(totalMag <= 120) {
            nextDist = (totalMag/12)*i;
        } else {
            nextDist = 10.0f*i;
        }
        for(j=0, trailPoint=trailData; j<11; j++, trailPoint++) {
            if(nextDist < trailPoint->totalMag+trailPoint->mag) {
                break;
            }
        }
        if(j < 11){
            nextDist -= trailPoint->totalMag;
            if(trailPoint->mag > 0) {
                scale = nextDist/trailPoint->mag;
                VECSubtract(&trailPoint->end, &trailPoint->start, &dir);
                VECScale(&dir, &dir, scale);
                VECAdd(&trailPoint->start, &dir, &pointHis[i]);
            } else {
                pointHis[i] = trailPoint->start;
            }
        } else {
            pointHis[i] = trailData[10].end;
        }
    }
    for(i=0; i<12; i++) {
        if(obj->mdlId[i] != MB_MODEL_NONE) {
            MBCapsuleObjPosSetV(obj->mdlId[i], &pointHis[i]);
        }
    }
    modelP = &Hu3DData[obj->mdlId[12]];
    effP = modelP->hookData;
    effDataP = effP->data;
    work = obj->data;
    for(i=0; i<effP->num/2; i++, effDataP++){ 
        effDataP->pos.x = pointHis[i].x;
        effDataP->pos.y = pointHis[i].y;
        effDataP->pos.z = pointHis[i].z;
    }
    for(i=0; i<effP->num/2; i++, effDataP++){ 
        effDataP->pos.x = pointHis[i].x;
        effDataP->pos.y = pointHis[i].y;
        effDataP->pos.z = pointHis[i].z;
    }
    pointNo--;
    if(pointNo < 0) {
        pointNo = 11;
    }
    obj->work[1] = pointNo;
}

static void ThrowEffClose(void)
{
    throwEffObj = NULL;
}

static void ThrowEffStart(HuVecF *pos, int capsuleNo)
{
    MBEFFECTDATA *effDataP;
    int i;
    OMOBJ *obj;
    MBEFFECT *effP;
    THROW_EFF_WORK *work;
    HU3DMODEL *modelP;
    float size;
    static GXColor colTbl[4][2] = {
        {
            { 128, 255, 128, 255 },
            { 192, 255, 192, 255 },
        },
        {
            { 255, 255, 128, 255 },
            { 255, 255, 192, 255 },
        },
        {
            { 255, 128, 128, 255 },
            { 255, 192, 192, 255 },
        },
        {
            { 128, 255, 255, 255 },
            { 192, 255, 255, 255 },
        }
    };
    GXColor color;
    
    obj = throwEffObj;
    if(throwEffObj == NULL) {
        return;
    }
    work = obj->data;
    for(i=0; i<12; i++) {
        work->prevPos[i] = *pos;
    }
    modelP = &Hu3DData[obj->mdlId[12]];
    effP = modelP->hookData;
    effDataP = effP->data;
    work = obj->data;
    color = colTbl[MBCapsuleColorNoGet(capsuleNo)][0];
    for(i=0; i<effP->num/2; i++, effDataP++) {
        size = i/12.0f;
        effDataP->scale = 100*(0.9f+(size*-0.49999997f));
        effDataP->pos.x = pos->x;
        effDataP->pos.y = pos->y;
        effDataP->pos.z = pos->z;
        effDataP->color.r = color.r;
        effDataP->color.g = color.g;
        effDataP->color.b = color.b;
        effDataP->color.a = 32*(1-size);
    }
    color = colTbl[MBCapsuleColorNoGet(capsuleNo)][1];
    for(i=0; i<effP->num/2; i++, effDataP++) {
        size = i/12.0f;
        effDataP->scale = 100*(0.9f+(size*-0.49999997f))*0.75f;
        effDataP->pos.x = pos->x;
        effDataP->pos.y = pos->y;
        effDataP->pos.z = pos->z;
        effDataP->color.r = color.r;
        effDataP->color.g = color.g;
        effDataP->color.b = color.b;
        effDataP->color.a = 64*(1-size);
    }
    obj->work[0] = TRUE;
}

static void ThrowEffPosSet(HuVecF *pos)
{
    OMOBJ *obj = throwEffObj;
    if(throwEffObj != NULL) {
        obj->trans.x = pos->x;
        obj->trans.y = pos->y;
        obj->trans.z = pos->z;
    }
}

static void CapsuleEffDraw(HU3DMODEL *modelP, Mtx *mtx);

static HU3DMODELID CapsuleEffCreate(ANIMDATA *anim, s16 num)
{
    CAPSULE_EFFECT *effP;
    CAPSULE_EFFDATA *effDataP;
    s16 i;
    HuVec2F *st;
    HU3DMODEL *modelP;
    HuVecF *vtx;
    HU3DMODELID modelId;
    void *dlBuf;
    void *dlBegin;
    modelId = Hu3DHookFuncCreate(CapsuleEffDraw);
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    modelP = &Hu3DData[modelId];
    modelP->hookData = effP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(CAPSULE_EFFECT), modelP->mallocNo);
    effP->anim = anim;
    effP->num = num;
    effP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
    effP->dispAttr = CAPSULE_EFF_DISPATTR_NONE;
    effP->hook = NULL;
    effP->hookMdlP = NULL;
    effP->count = 0;
    effP->attr = CAPSULE_EFF_ATTR_NONE;
    effP->unk23 = 0;
    effP->prevCount = 0;
    effP->mode = effP->time = 0;
    effP->data = effDataP = HuMemDirectMallocNum(HEAP_MODEL, num*sizeof(CAPSULE_EFFDATA), modelP->mallocNo);
    memset(effDataP, 0, num*sizeof(CAPSULE_EFFDATA));
    for(i=0; i<num; i++, effDataP++) {
        effDataP->scale = 0;
        effDataP->rot.x = effDataP->rot.y = effDataP->rot.z = 0;
        effDataP->animTime = 0;
        effDataP->animSpeed = 1;
        effDataP->pos.x = ((frand() & 0x7F)-64)*20;
        effDataP->pos.y = ((frand() & 0x7F)-64)*30;
        effDataP->pos.z = ((frand() & 0x7F)-64)*20;
        effDataP->color.r = effDataP->color.g = effDataP->color.b = effDataP->color.a = 255;
        effDataP->no = 0;
    }
    effP->vertex = vtx = HuMemDirectMallocNum(HEAP_MODEL, num*sizeof(HuVecF)*4, modelP->mallocNo);
    for(i=0; i<num*4; i++, vtx++) {
        vtx->x = vtx->y = vtx->z = 0;
    }
    effP->st = st = HuMemDirectMallocNum(HEAP_MODEL, num*sizeof(HuVec2F)*4, modelP->mallocNo);
    for(i=0; i<num; i++) {
        st->x = 0;
        st->y = 0;
        st++;
        
        st->x = 1;
        st->y = 0;
        st++;
        
        st->x = 1;
        st->y = 1;
        st++;
        
        st->x = 0;
        st->y = 1;
        st++;
    }
    dlBegin = dlBuf = HuMemDirectMallocNum(HEAP_MODEL, 0x10000, modelP->mallocNo);
    DCFlushRange(dlBuf, 0x10000);
    GXBeginDisplayList(dlBegin, 0x10000);
    GXBegin(GX_QUADS, GX_VTXFMT0, num*4);
    for(i=0; i<num; i++) {
        GXPosition1x16(i*4);
        GXColor1x16(i);
        GXTexCoord1x16(i*4);
        
        GXPosition1x16((i*4)+1);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+1);
        
        GXPosition1x16((i*4)+2);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+2);
        
        GXPosition1x16((i*4)+3);
        GXColor1x16(i);
        GXTexCoord1x16((i*4)+3);
    }
    GXEnd();
    effP->dlSize = GXEndDisplayList();
    effP->dl = HuMemDirectMallocNum(HEAP_MODEL, effP->dlSize, modelP->mallocNo);
    memcpy(effP->dl, dlBuf, effP->dlSize);
    DCFlushRange(effP->dl, effP->dlSize);
    HuMemDirectFree(dlBuf);
    return modelId;
}

static HuVecF basePos[] = {
    { -0.5f,  0.5f, 0.0f },
    {  0.5f,  0.5f, 0.0f },
    {  0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f }
};

static HuVec2F baseST[] = {
    { 0.0f, 0.0f },
    { 0.25f, 0.0f },
    { 0.25f, 0.25f },
    { 0.0f, 0.25f },
};


static void CapsuleEffDraw(HU3DMODEL *modelP, Mtx *mtx)
{
    CAPSULE_EFFECT *effP;
    CAPSULE_EFFDATA *effDataP;
    HuVecF *vtx;
    HuVec2F *st;
    HuVecF *scaleVtxP;
    s16 i;
    s16 j;
    HuVecF *initVtxP;
    s16 bmpFmt;
    s16 row;
    s16 col;
    CAPSULE_EFFHOOK hook;
    
    Mtx mtxInv;
    Mtx mtxPos;
    Mtx mtxRotZ;
    HuVecF scaleVtx[4];
    HuVecF finalVtx[4];
    HuVecF initVtx[4];
    ROMtx basePosMtx;
    
    effP = modelP->hookData;
    if(effP->prevCounter != GlobalCounter || shadowModelDrawF) {
        if(effP->hookMdlP && effP->hookMdlP != modelP){
            CapsuleEffDraw(effP->hookMdlP, mtx);
        }
        GXLoadPosMtxImm(*mtx, GX_PNMTX0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        if(shadowModelDrawF) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        } else {
            bmpFmt = (effP->anim->bmp->dataFmt & 0xF);
            if(bmpFmt == ANIM_BMP_I8 || bmpFmt == ANIM_BMP_I4) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_ZERO);
            } else {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
            }
            if(effP->dispAttr & CAPSULE_EFF_DISPATTR_ZBUF_OFF) {
                GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
            } else if(modelP->attr & HU3D_ATTR_ZWRITE_OFF) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            } else {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
            }
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        HuSprTexLoad(effP->anim, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(GX_FALSE);
        switch (effP->blendMode) {
            case HU3D_PARTICLE_BLEND_NORMAL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_ADDCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_INVCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
                break;
        }
        if(HmfInverseMtxF3X3(*mtx, mtxInv) == FALSE) {
            PSMTXIdentity(mtxInv);
        }
        PSMTXReorder(mtxInv, basePosMtx);
        if(effP->hook) {
            hook = effP->hook;
            hook(modelP, effP, mtx);
        }
        effDataP = effP->data;
        vtx = effP->vertex;
        st = effP->st;
        if(effP->dispAttr & CAPSULE_EFF_DISPATTR_CAMERA_ROT) {
            MTXIdentity(mtxInv);
            MTXIdentity(*(Mtx *)(&basePosMtx));
            initVtx[0] = basePos[0];
            initVtx[1] = basePos[1];
            initVtx[2] = basePos[2];
            initVtx[3] = basePos[3];
        } else {
            PSMTXROMultVecArray(basePosMtx, &basePos[0], initVtx, 4);
        }
        for(i=0; i<effP->num; i++, effDataP++) {
            if(!effDataP->scale) {
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
            } else if(effP->dispAttr & CAPSULE_EFF_DISPATTR_ROT3D) {
                VECScale(&basePos[0], &scaleVtx[0], effDataP->scale);
                VECScale(&basePos[1], &scaleVtx[1], effDataP->scale);
                VECScale(&basePos[2], &scaleVtx[2], effDataP->scale);
                VECScale(&basePos[3], &scaleVtx[3], effDataP->scale);
                mtxRot(mtxPos, effDataP->rot.x, effDataP->rot.y, effDataP->rot.z);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);
            } else if(!effDataP->rot.z) {
                scaleVtxP = scaleVtx;
                initVtxP = initVtx;
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
            } else {
                VECScale(&basePos[0], &scaleVtx[0], effDataP->scale);
                VECScale(&basePos[1], &scaleVtx[1], effDataP->scale);
                VECScale(&basePos[2], &scaleVtx[2], effDataP->scale);
                VECScale(&basePos[3], &scaleVtx[3], effDataP->scale);
                MTXRotRad(mtxRotZ, 'Z', effDataP->rot.z);
                PSMTXConcat(mtxInv, mtxRotZ, mtxPos);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);

            }
        }
        effDataP = effP->data;
        st = effP->st;
        if(!(effP->dispAttr & CAPSULE_EFF_DISPATTR_NOANIM)) {
            for(i=0; i<effP->num; i++, effDataP++) {
                row = effDataP->no & 0x3;
                col = (effDataP->no >> 2) & 0x3;
                for(j=0; j<4; j++, st++) {
                    st->x = (0.25f*row)+baseST[j].x;
                    st->y = (0.25f*col)+baseST[j].y;
                }
            }
        } else {
            for(i=0; i<effP->num; i++, effDataP++) {
                for(j=0; j<4; j++, st++) {
                    st->x = 4*baseST[j].x;
                    st->y = 4*baseST[j].y;
                }
            }
        }
        DCFlushRangeNoSync(effP->vertex, effP->num*sizeof(HuVecF)*4);
        DCFlushRangeNoSync(effP->st, effP->num*sizeof(HuVec2F)*4);
        DCFlushRangeNoSync(effP->data, effP->num*sizeof(CAPSULE_EFFDATA));
        PPCSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetArray(GX_VA_POS, effP->vertex, sizeof(HuVecF));
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetArray(GX_VA_CLR0, &effP->data->color, sizeof(CAPSULE_EFFDATA));
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetArray(GX_VA_TEX0, effP->st, sizeof(HuVec2F));
        GXCallDisplayList(effP->dl, effP->dlSize);
        if(shadowModelDrawF == FALSE) {
            if(!(effP->attr & CAPSULE_EFF_ATTR_COUNTER_UPDATE)) {
                effP->count++;
            }
            if(effP->prevCount != 0 && effP->prevCount <= effP->count) {
                if(effP->attr & CAPSULE_EFF_ATTR_COUNTER_RESET) {
                    effP->count = 0;
                }
                effP->count = effP->prevCount;
            }
            effP->prevCounter = GlobalCounter;
        }

    }
    
}

static float AngleSumWrap(float start, float end)
{
    float result;
    if(start >= 360) {
        start -= 360;
    } else if(start < 0) {
        start += 360;
    }
    if(end >= 360) {
        end -= 360;
    } else if(end < 0) {
        end += 360;
    }
    result = start-end;
    if(result <= -180.0f) {
        result += 360.0f;
    } else if(result >= 180.0f){
        result -= 360.0f;
    }
    return result;
}

static float CameraXZAngleGet(float angle)
{
    MBCAMERA *cameraP = MBCameraGet();
    Mtx rot;
    HuVecF dir;
    mtxRot(rot, cameraP->rot.x, cameraP->rot.y, cameraP->rot.z);
    dir.x = HuSin(angle);
    dir.y = 0;
    dir.z = HuCos(angle);
    MTXMultVec(rot, &dir, &dir);
    return HuAtan(dir.x, dir.z);
}

static void ComNoUseComKeyHook(void);

static void ComNoUseChoiceSet(int choice)
{
    comNoUseChoice = choice;
    MBTopWinComKeyHookSet(ComNoUseComKeyHook);
}

static void ComNoUseComKeyHook(void)
{
    
    int comKey[GW_PLAYER_MAX];
    int i;
    int padNo;
    int key;
    s16 delay;
    int playerNo;
    comKey[0] = comKey[1] = comKey[2] = comKey[3] = 0;
    playerNo = GwSystem.turnPlayerNo;
    padNo = GwPlayer[playerNo].padNo;
    delay = GWComKeyDelayGet();
    if(comNoUseChoice >= 0) {
        key = PAD_BUTTON_RIGHT;
    } else {
        key = PAD_BUTTON_LEFT;
    }
    comKey[padNo] = key;
    for(i=0; i<abs(comNoUseChoice); i++) {
        comKey[padNo] = key;
        HuWinComKeyWait(comKey[0], comKey[1], comKey[2], comKey[3], delay);
    }
    comKey[padNo] = PAD_BUTTON_A;
    HuWinComKeyWait(comKey[0], comKey[1], comKey[2], comKey[3], delay);
}

static float CalcBezier(float a, float b, float c, float t)
{
    float temp = 1.0-t;
    float result =(a*(temp*temp))+(2.0*(b*(temp*t)))+(c*(t*t));
    return result;
}

static void ApplyBezier(float t, float *a, float *b, float *c, float *out)
{
    int i;
    for(i=0; i<3; i++) {
        *out++ = CalcBezier(*a++, *b++, *c++, t);
    }
}

static float capsuleTime[8];
static float capsuleBezierX[8];
static float capsuleBezierY[8];
static float capsuleBezierZ[8];

static void CalcThrowCameraParam(float *pos, float *paramOut, float *paramIn, int num)
{
    int i;
    float delta[8];
    float speed[8];
    
    
    paramOut[0] = 0;
    paramOut[num-1] = 0;
    for(i=0; i<num-1; i++) {
        delta[i] = paramIn[i+1]-paramIn[i];
        speed[i+1] = (pos[i+1]-pos[i])/delta[i];
    }
    paramOut[1] = (speed[2]-speed[1])-(delta[0]*paramOut[0]);
    speed[1] = 2*(paramIn[2]-paramIn[0]);
    for(i=1; i<num-2; i++) {
        double t = delta[i]/speed[i];
        paramOut[i+1] = (speed[i+2]-speed[i+1])-(paramOut[i]*t);
        speed[i+1] = (2*(paramIn[i+2]-paramIn[i]))-(delta[i]*t);
    }
    paramOut[num-2] -= delta[num-2]*paramOut[num-1];
    for(i=num-2; i>0; i--) {
        paramOut[i] = (paramOut[i]-(delta[i]*paramOut[i+1]))/speed[i];
    }
}

static void CapsuleBezierParamSet(float *x, float *y, float *z, int num)
{
    int i;
    float dx;
    float dy;
    capsuleTime[0] = 0;
    for(i=1; i<num; i++) {
        dx = x[i]-x[i-1];
        dy = y[i]-y[i-1];
        capsuleTime[i] = capsuleTime[i-1]+sqrt((dx*dx)+(dy*dy));
    }
    for(i=1; i<num; i++) {
        capsuleTime[i] /= capsuleTime[num-1];
    }
    CalcThrowCameraParam(x, capsuleBezierX, capsuleTime, num);
    CalcThrowCameraParam(y, capsuleBezierY, capsuleTime, num);
    CalcThrowCameraParam(z, capsuleBezierZ, capsuleTime, num);
}

static float GetCamBezier(float t, float *pos, float *param, float *time, int num)
{
    int i;
    int max;
    int idx;
    float timeFrac;
    float dt;
    
    for(i=0, max=num-1; i<max;) {
        idx = (i+max)/2;
        if(time[idx] < t) {
            i = idx+1;
        } else{
            max = idx;
        }
    }
    if(i > 0) {
        i--;
    }
    dt = time[i+1]-time[i];
    timeFrac = t-time[i];
    return pos[i] + (timeFrac * ((timeFrac * ((3.0f * param[i]) + ((timeFrac * (param[i+1] - param[i])) / dt))) + (((pos[i+1] - pos[i]) / dt) - (dt * ((2 * param[i]) + param[i+1])))));
}

static void CapsuleCurveCalc(float t, float *x, float *y, float *z, HuVecF *out, int num)
{
    out->x = GetCamBezier(t, x, capsuleBezierX, capsuleTime, num);
    out->y = GetCamBezier(t, y, capsuleBezierY, capsuleTime, num);
    out->z = GetCamBezier(t, z, capsuleBezierZ, capsuleTime, num);
}

static void CapsuleColBegin(void)
{
    int boardNo = MBBoardNoGet();
    if(capsuleColMdl != MB_MODEL_NONE) {
        capsuleColMdlId = MBModelIdGet(capsuleColMdl);
    }
}

static BOOL CapsuleColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    HSFFACE *faceP; //r30
    HSFDATA *hsfP; //r29
    HSFBUFFER *faceBufP; //r28
    HSFBUFFER *vtxBufP; //r27
    BOOL quadF; //r25
    int i; //r24
    int j; //r23
    HU3DMODEL *modelP; //r22
    
    float dot; //f31
    float dotEdge; //f30
    float dotA; //f29
    float dotB; //f28
    float invDotA; //f27
    float scale; //f26
    
    
    HuVecF faceVtx[4]; //sp+0x7C
    HuVecF faceNorm; //sp+0x70
    HuVecF faceCA; //sp+0x64
    HuVecF faceBA; //sp+0x58
    HuVecF faceBAQuad; //sp+0x4C
    HuVecF faceCAQuad; //sp+0x40
    HuVecF faceNormQuad; //sp+0x34
    HuVecF dir; //sp+0x28
    HuVecF outPos; //sp+0x1C
    HuVecF dotVec; //sp+0x10
    
    float mag; //sp+0xC
    int temp; //sp+0x8
    
    quadF = FALSE;
    temp = 0;
    if(capsuleColMdlId == HU3D_MODELID_NONE) {
        return FALSE;
    }
    modelP = &Hu3DData[capsuleColMdlId];
    hsfP = modelP->hsf;
    faceBufP = hsfP->face;
    vtxBufP = hsfP->vertex;
    VECSubtract(posA, posB, &dir);
    VECNormalize(&dir, &dir);
    mag = VECMag(&dir);
    for(i=0; i<hsfP->faceNum; i++, faceBufP++) {
        faceP = faceBufP->data;
        for(j=0; j<faceBufP->count; j++, faceP++) {
            switch(faceP->type & HSF_FACE_MASK) {
                case 0:
                    quadF = TRUE;
                    break;
                
                case 1:
                    quadF = TRUE;
                    break;

                case HSF_FACE_TRI:
                    quadF = FALSE;
                    faceVtx[0] = *(((HuVecF *)vtxBufP->data)+faceP->indices[0][0]);
                    faceVtx[1] = *(((HuVecF *)vtxBufP->data)+faceP->indices[1][0]);
                    faceVtx[2] = *(((HuVecF *)vtxBufP->data)+faceP->indices[2][0]);
                    break;
                
                case HSF_FACE_QUAD:
                    quadF = TRUE;
                    break;
                    
                default:
                    break;
            }
            if(!quadF) {
                VECSubtract(&faceVtx[1], &faceVtx[0], &faceBAQuad);
                VECSubtract(&faceVtx[2], &faceVtx[0], &faceCAQuad);
                VECCrossProduct(&faceBAQuad, &faceCAQuad, &faceNormQuad);
                VECNormalize(&faceNormQuad, &faceNormQuad);
                dot = -VECDotProduct(&faceNormQuad, &faceVtx[0]);
                dotA = (faceNormQuad.x*posA->x)+(faceNormQuad.y*posA->y)+(faceNormQuad.z*posA->z)+dot;
                dotB = (faceNormQuad.x*posB->x)+(faceNormQuad.y*posB->y)+(faceNormQuad.z*posB->z)+dot;
                if(dotA*dotB > 0) {
                    continue;
                }
                invDotA = -((faceNormQuad.x*posA->x)+(faceNormQuad.y*posA->y)+(faceNormQuad.z*posA->z)+dot);
                dotEdge = (faceNormQuad.x*dir.x)+(faceNormQuad.y*dir.y)+(faceNormQuad.z*dir.z);
                if(dotEdge != 0.0f) { 
                    scale = invDotA/dotEdge;
                    VECScale(&dir, &outPos, scale);
                    VECAdd(&outPos, posA, &outPos);
                    *out = outPos;
                    VECSubtract(&faceVtx[1], &faceVtx[0], &faceBA);
                    VECSubtract(&outPos, &faceVtx[0], &faceCA);
                    VECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.x = VECDotProduct(&faceNormQuad, &faceNorm);
                    VECSubtract(&faceVtx[2], &faceVtx[1], &faceBA);
                    VECSubtract(&outPos, &faceVtx[1], &faceCA);
                    VECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.y = VECDotProduct(&faceNormQuad, &faceNorm);
                    VECSubtract(&faceVtx[0], &faceVtx[2], &faceBA);
                    VECSubtract(&outPos, &faceVtx[2], &faceCA);
                    VECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.z = VECDotProduct(&faceNormQuad, &faceNorm);
                    if(dotVec.x > 0 && dotVec.y > 0 && dotVec.z > 0) {
                        return TRUE;
                    }
                    if(dotVec.x < 0 && dotVec.y < 0 && dotVec.z < 0) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

static void CapsuleColEnd(void)
{
    capsuleColMdlId = HU3D_MODELID_NONE;
}