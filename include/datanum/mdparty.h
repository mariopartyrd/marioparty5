#ifndef DATANUM_MDPARTY_H
#define DATANUM_MDPARTY_H

#include "datadir_enum.h"

#define MDPARTY_CHAR(charName) \
    MDPARTY_HSF_##charName, \
    MDPARTY_HSF_##charName##_idle, \
    MDPARTY_HSF_##charName##_win

enum {
    MDPARTY_DUMMY_FILE = DATANUM(DATA_mdparty, 0)-1,
    MDPARTY_CHAR(mario), //0
    MDPARTY_CHAR(luigi), //3
    MDPARTY_CHAR(peach), //6
    MDPARTY_CHAR(yoshi), //9
    MDPARTY_CHAR(wario), //12
    MDPARTY_CHAR(daisy), //15
    MDPARTY_CHAR(waluigi), //18
    MDPARTY_CHAR(kinopio), //21
    MDPARTY_CHAR(teresa), //24
    MDPARTY_CHAR(minikoopa), //27
    MDPARTY_HSF_star, //30
    MDPARTY_HSF_stage, //31
    MDPARTY_HSF_stage_chr, //32
    MDPARTY_HSF_cap_broyal, //33
    MDPARTY_HSF_cap_tmatch, //34
    MDPARTY_HSF_cap_chr, //35
    MDPARTY_HSF_map1, //36
    MDPARTY_HSF_map2, //37
    MDPARTY_HSF_map3, //38
    MDPARTY_HSF_map4, //39
    MDPARTY_HSF_map5, //40
    MDPARTY_HSF_map6, //41
    MDPARTY_HSF_map7, //42
    MDPARTY_HSF_cap_bonus, //43
    MDPARTY_HSF_finger, //44
    MDPARTY_HSF_ring, //45
    MDPARTY_HSF_cap_man, //46
    MDPARTY_HSF_cap_com, //47
    MDPARTY_HSF_cap_pack0, //48
    MDPARTY_HSF_cap_pack1, //49
    MDPARTY_HSF_cap_pack2, //50
    MDPARTY_HSF_cap_pack3, //51
    MDPARTY_HSF_cap_pack4, //52
    MDPARTY_HSF_cap_turn, //53
    MDPARTY_HSF_vs, //54
    MDPARTY_HSF_star_idle, //55
    MDPARTY_HSF_star_talk, //56
    MDPARTY_HSF_star_talk_fast, //57
    MDPARTY_ANM_cursor, //58
    MDPARTY_ANM_cursor_glow, //59
    MDPARTY_ANM_win_frame, //60
    MDPARTY_ANM_win_bg, //61
    MDPARTY_ANM_handicap_num, //62
    MDPARTY_ANM_handicap, //63
    MDPARTY_ANM_effect_dot, //64
    MDPARTY_ANM_effect_glow, //65
    MDPARTY_ANM_team_red, //66
    MDPARTY_ANM_team_blue, //67
    MDPARTY_ANM_playerno_finger, //68
    MDPARTY_ANM_playerno, //69
    MDPARTY_ANM_comdif, //70
    MDPARTY_ANM_turn, //71
    MDPARTY_ANM_bonus, //72
    MDPARTY_ANM_map1_name, //73
    MDPARTY_ANM_map2_name, //74
    MDPARTY_ANM_map3_name, //75
    MDPARTY_ANM_map4_name, //76
    MDPARTY_ANM_map5_name, //77
    MDPARTY_ANM_map6_name, //78
    MDPARTY_ANM_map7_name, //79
    MDPARTY_ANM_mode_name, //80
};

#undef MDPARTY_CHAR

#endif