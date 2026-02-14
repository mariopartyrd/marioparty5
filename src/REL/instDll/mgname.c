#include "REL/instDll.h"

#define NAME_SPR_MAX 100

static unsigned int FontFileTbl[198] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    INST_ANM_number_0,
    INST_ANM_number_1,
    INST_ANM_number_2,
    INST_ANM_number_3,
    INST_ANM_number_4,
    INST_ANM_number_5,
    INST_ANM_number_6,
    INST_ANM_number_7,
    INST_ANM_number_8,
    INST_ANM_number_9,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_punct_minus,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_letter_a_upper,
    INST_ANM_letter_b_upper,
    INST_ANM_letter_c_upper,
    INST_ANM_letter_d_upper,
    INST_ANM_letter_e_upper,
    INST_ANM_letter_f_upper,
    INST_ANM_letter_g_upper,
    INST_ANM_letter_h_upper,
    INST_ANM_letter_i_upper,
    INST_ANM_letter_j_upper,
    INST_ANM_letter_k_upper,
    INST_ANM_letter_l_upper,
    INST_ANM_letter_m_upper,
    INST_ANM_letter_n_upper,
    INST_ANM_letter_o_upper,
    INST_ANM_letter_p_upper,
    INST_ANM_letter_q_upper,
    INST_ANM_letter_r_upper,
    INST_ANM_letter_s_upper,
    INST_ANM_letter_t_upper,
    INST_ANM_letter_u_upper,
    INST_ANM_letter_v_upper,
    INST_ANM_letter_w_upper,
    INST_ANM_letter_x_upper,
    INST_ANM_letter_y_upper,
    INST_ANM_letter_z_upper,
    INST_ANM_letter_o_upper,
    INST_ANM_punct_apos,
    INST_ANM_letter_o_upper,
    INST_ANM_letter_o_upper,
    INST_ANM_letter_o_upper,
    INST_ANM_letter_o_upper,
    INST_ANM_letter_a_lower,
    INST_ANM_letter_b_lower,
    INST_ANM_letter_c_lower,
    INST_ANM_letter_d_lower,
    INST_ANM_letter_e_lower,
    INST_ANM_letter_f_lower,
    INST_ANM_letter_g_lower,
    INST_ANM_letter_h_lower,
    INST_ANM_letter_i_lower,
    INST_ANM_letter_j_lower,
    INST_ANM_letter_k_lower,
    INST_ANM_letter_l_lower,
    INST_ANM_letter_m_lower,
    INST_ANM_letter_n_lower,
    INST_ANM_letter_o_lower,
    INST_ANM_letter_p_lower,
    INST_ANM_letter_q_lower,
    INST_ANM_letter_r_lower,
    INST_ANM_letter_s_lower,
    INST_ANM_letter_t_lower,
    INST_ANM_letter_u_lower,
    INST_ANM_letter_v_lower,
    INST_ANM_letter_w_lower,
    INST_ANM_letter_x_lower,
    INST_ANM_letter_y_lower,
    INST_ANM_letter_z_lower,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_punct_amp,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_punct_minus,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_number_0,
    INST_ANM_punct_excl,
    INST_ANM_punct_hatena,
    INST_ANM_number_0,
    INST_ANM_number_0,
};

HUSPRGRPID MgNameCreate(u32 nameMes)
{
    char *mesP; //r31
    s16 width; //r30
    s16 *posY; //r29
    unsigned int *fileTbl; //r28
    s16 charNum; //r27
    s16 i; //r26
    unsigned int file; //r25
    s16 *posX; //r24
    ANIMDATA **animP; //r23
    HUSPRGRPID grpId; //r22
    HUSPRGRP *grpP; //r21
    HUSPRID sprId; //r20
    
    fileTbl = FontFileTbl;
    animP = HuMemDirectMalloc(HEAP_HEAP, sizeof(ANIMDATA *)*NAME_SPR_MAX);
    posX = HuMemDirectMalloc(HEAP_HEAP, sizeof(s16)*NAME_SPR_MAX);
    posY = HuMemDirectMalloc(HEAP_HEAP, sizeof(s16)*NAME_SPR_MAX);
    mesP = HuWinMesPtrGet(nameMes);
    for(width=0, charNum=0; *mesP; mesP++) {
        if(*mesP == 0x20 || *mesP == 0x10) {
            width += 12;
        } else if(*mesP >= 0x30 && *mesP != 0x80 && *mesP != 0x81) {
            if(mesP[1] == 0x80) {
                if(*mesP >= 0x96 && *mesP <= 0xA4) {
                    file = fileTbl[*mesP+0x6A];
                } else if(*mesP >= 0xAA && *mesP <= 0xAE) {
                    file = fileTbl[*mesP+0x65];
                } else if(*mesP >= 0xD6 && *mesP <= 0xE4) {
                    file = fileTbl[*mesP+0x43];
                } else if(*mesP >= 0xEA && *mesP <= 0xEE) {
                    file = fileTbl[*mesP+0x3E];
                }
            } else if(mesP[1] == 0x81) {
                if(*mesP >= 0xAA && *mesP <= 0xAE) {
                    file = fileTbl[*mesP+0x6A];
                } else if(*mesP >= 0xEA && *mesP <= 0xEE) {
                    file = fileTbl[*mesP+0x43];
                }
            } else {
                file = fileTbl[*mesP];
            }
            animP[charNum] = HuSprAnimDataRead(file);
            posX[charNum] = width;
            if(*mesP >= 0x61 && *mesP <= 0x7A) {
                posY[charNum] = 2;
                width += 18;
            } else if(*mesP == 0x3D) {
                posY[charNum] = 0;
                width += 18;
            } else if(*mesP == 0xC2 || *mesP == 0xC3) {
                posY[charNum] = 0;
                width += 18;
            } else if(*mesP == 0x5C) {
                posY[charNum] = 0;
                width += 8;
            } else if(*mesP >= 0x87 && *mesP <= 0x8F) {
                posY[charNum] = 4;
                width += 24;
            } else if(*mesP >= 0xC7 && *mesP <= 0xCF) {
                posY[charNum] = 4;
                width += 24;
            } else {
                posY[charNum] = 0;
                width += 28;
            }
            charNum++;
        }
    }
    grpId = HuSprGrpCreate(charNum);
    grpP = &HuSprGrpData[grpId];
    grpP->work[0] = width;
    width = (width/2)-14;
    for(i=0; i<charNum; i++) {
        sprId = HuSprCreate(animP[i], 0, 0);
        HuSprGrpMemberSet(grpId, i, sprId);
        HuSprPosSet(grpId, i, posX[i]-width, posY[i]);
        
    }
    HuMemDirectFree(animP);
    HuMemDirectFree(posX);
    HuMemDirectFree(posY);
    return grpId;
}