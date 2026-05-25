#include <humath.h>
#include <math.h>

#include "game/frand.h"
#include "game/hsfex.h"

#include "REL/m507dll.h"

#ifndef __MWERKS__
#include "game/audio.h"
#endif

void fn_1_6A34(float var_f30, Vec *arg0, Vec *arg1, Vec *arg2)
{
    float var_f31;

    if (1.0f < var_f30) {
        var_f30 = 1.0f;
    }
    var_f31 = HuSin(90.0f * var_f30) * HuSin(90.0f * var_f30);
    arg2->x = arg0->x + (var_f31 * (arg1->x - arg0->x));
    arg2->y = arg0->y + (var_f31 * (arg1->y - arg0->y));
    arg2->z = arg0->z + (var_f31 * (arg1->z - arg0->z));
}

void fn_1_6B90(Vec *arg0, Vec *arg1, Vec *arg2)
{
    Hu3DCameraPosSetV(1, arg0, arg1, arg2);
    lbl_1_bss_1318 = *arg0;
    lbl_1_bss_1300 = *arg2;
}

s32 fn_1_6C1C(float var_f27, Vec *arg0)
{
    Vec sp18;
    Vec spC;
    float var_f31;
    float var_f30;
    float var_f29;
    float var_f28;

    var_f31 = var_f27;
    if (1.0f < var_f31) {
        var_f31 = 1.0f;
    }
    var_f29 = HuSin(90.0f * var_f31) * HuSin(90.0f * var_f31);
    sp18.x = lbl_1_bss_1318.x + (var_f29 * (lbl_1_bss_130C.x - lbl_1_bss_1318.x));
    sp18.y = lbl_1_bss_1318.y + (var_f29 * (lbl_1_bss_130C.y - lbl_1_bss_1318.y));
    sp18.z = lbl_1_bss_1318.z + (var_f29 * (lbl_1_bss_130C.z - lbl_1_bss_1318.z));
    var_f30 = var_f27;
    if (1.0f < var_f30) {
        var_f30 = 1.0f;
    }
    var_f28 = HuSin(90.0f * var_f30) * HuSin(90.0f * var_f30);
    spC.x = lbl_1_bss_1300.x + (var_f28 * (lbl_1_bss_12F4.x - lbl_1_bss_1300.x));
    spC.y = lbl_1_bss_1300.y + (var_f28 * (lbl_1_bss_12F4.y - lbl_1_bss_1300.y));
    spC.z = lbl_1_bss_1300.z + (var_f28 * (lbl_1_bss_12F4.z - lbl_1_bss_1300.z));
    Hu3DCameraPosSetV(1, &sp18, arg0, &spC);
    if (1.0f <= var_f27) {
        return 1;
    }
    lbl_1_bss_1224 = 1;
    return 0;
}

s32 fn_1_6F50(float var_f27, Vec *arg0)
{
    Vec sp18;
    Vec spC;
    s32 i;
    float var_f31;
    float var_f30;
    float var_f29;
    float var_f28;

    for (i = 0; i < 4; i++) {
        var_f31 = var_f27;
        if (1.0f < var_f31) {
            var_f31 = 1.0f;
        }
        var_f29 = HuSin(90.0f * var_f31) * HuSin(90.0f * var_f31);
        sp18.x = lbl_1_bss_12C4[i].x + (var_f29 * (lbl_1_bss_1294[i].x - lbl_1_bss_12C4[i].x));
        sp18.y = lbl_1_bss_12C4[i].y + (var_f29 * (lbl_1_bss_1294[i].y - lbl_1_bss_12C4[i].y));
        sp18.z = lbl_1_bss_12C4[i].z + (var_f29 * (lbl_1_bss_1294[i].z - lbl_1_bss_12C4[i].z));
        var_f30 = var_f27;
        if (1.0f < var_f30) {
            var_f30 = 1.0f;
        }
        var_f28 = HuSin(90.0f * var_f30) * HuSin(90.0f * var_f30);
        spC.x = lbl_1_bss_1264[i].x + (var_f28 * (lbl_1_bss_1234[i].x - lbl_1_bss_1264[i].x));
        spC.y = lbl_1_bss_1264[i].y + (var_f28 * (lbl_1_bss_1234[i].y - lbl_1_bss_1264[i].y));
        spC.z = lbl_1_bss_1264[i].z + (var_f28 * (lbl_1_bss_1234[i].z - lbl_1_bss_1264[i].z));
        Hu3DCameraPosSetV(1 << i, &sp18, arg0, &spC);
    }
    if (1.0f <= var_f27) {
        return 1;
    }
    lbl_1_bss_1224 = 1;
    return 0;
}

float fn_1_7334(float var_f29, float arg1, float arg2)
{
    float var_f31;
    float var_f30;

    var_f31 = fmod(arg1 - var_f29, 360.0);
    if (0.0f > var_f31) {
        var_f31 += 360.0f;
    }
    if (180.0f < var_f31) {
        var_f31 -= 360.0f;
    }
    var_f30 = fmod(var_f29 + (var_f31 * arg2), 360.0);
    if (0.0f > var_f30) {
        var_f30 += 360.0f;
    }
    return var_f30;
}

void fn_1_7438(s32 arg0)
{
    if (lbl_1_bss_1228 >= 0) {
        if (arg0 < 0) {
            arg0 = 1000;
        }
        HuAudSeqFadeOut(lbl_1_bss_1228, arg0);
    }
    lbl_1_bss_1228 = -1;
}

void fn_1_74A4(s16 arg0, char *arg1, Vec *arg2)
{
    Mtx sp10;

    Hu3DModelObjMtxGet(arg0, arg1, sp10);
    arg2->x = sp10[0][3];
    arg2->y = sp10[1][3];
    arg2->z = sp10[2][3];
}

u32 fn_1_74FC(u32 arg0, u32 arg1)
{
    return arg0 + frandmod(arg1 - arg0);
}

s32 fn_1_7538(Vec *arg0, Vec *arg1, float arg2, float arg3)
{
    if ((((arg1->x - arg0->x) * (arg1->x - arg0->x)) + ((arg1->z - arg0->z) * (arg1->z - arg0->z))) < ((arg2 + arg3) * (arg2 + arg3))) {
        return 1;
    }
    return 0;
}

s32 fn_1_7598(Vec *arg0, s32 arg1, s32 arg2)
{
    Vec sp10;
    s32 var_r31;

    Hu3D3Dto2D(arg0, 1, &sp10);
    var_r31 = arg1 + ((sp10.x / 576.0f) * (arg2 - arg1));
    return var_r31;
}

s32 fn_1_765C(Vec *arg0, s32 arg1, s32 arg2)
{
    Vec sp10;
    s32 var_r31;

    Hu3D3Dto2D(arg0, 1, &sp10);
    var_r31 = arg1 + ((sp10.y / 480.0f) * (arg2 - arg1));
    return var_r31;
}
