#ifndef M501_PARTICLE_H
#define M501_PARTICLE_H

#include "humath.h"

namespace m501 {
    class CParticle {
    public:
        static void createFallParticle(HuVecF *pos, int type);
        static void createTakeParticle(HuVecF *pos, int type);
    };
};

#endif