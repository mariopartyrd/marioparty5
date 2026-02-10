#ifndef DATANUM_MDSEL_H
#define DATANUM_MDSEL_H

#include "datadir_enum.h"

enum {
    MDSEL_HSF_stage = DATANUM(DATA_mdsel, 0),
    MDSEL_HSF_star_choru,
    MDSEL_HSF_star_neru,
    MDSEL_HSF_star_teru,
    MDSEL_HSF_star_niru,
    MDSEL_HSF_star_haru,
    MDSEL_HSF_star_maru,
    MDSEL_HSF_star_daru,
    MDSEL_HSF_cap_particle,
    MDSEL_HSF_cap_choru,
    MDSEL_HSF_cap_neru,
    MDSEL_HSF_cap_niru,
    MDSEL_HSF_cap_haru,
    MDSEL_HSF_cap_maru,
    MDSEL_HSF_cap_daru,
    MDSEL_HSF_star_choru_mot,
    MDSEL_HSF_star_neru_mot,
    MDSEL_HSF_star_teru_mot,
    MDSEL_HSF_star_teru_mot_talk,
    MDSEL_HSF_star_niru_mot,
    MDSEL_HSF_star_haru_mot,
    MDSEL_HSF_star_maru_mot,
    MDSEL_HSF_star_daru_mot,
    MDSEL_ANM_win_frame,
    MDSEL_ANM_win_bg,
    MDSEL_ANM_cursor,
    MDSEL_ANM_cursor_glow,
    MDSEL_ANM_eff_glow,
    MDSEL_ANM_mdparty,
    MDSEL_ANM_mdstory,
    MDSEL_ANM_mdmini,
    MDSEL_ANM_sdroom,
    MDSEL_ANM_mdomake,
    MDSEL_ANM_mdoption
};

#undef FILESEL_CHAR

#endif