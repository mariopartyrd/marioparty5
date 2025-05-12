#include "dolphin.h"

void HuSubVecF(Vec* out, Vec* in, Vec* sub) {
    out->x = in->x - sub->x;
    out->y = in->y - sub->y;
    out->z = in->z - sub->z;
}