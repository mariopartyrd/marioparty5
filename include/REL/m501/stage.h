#ifndef M501_STAGE_H
#define M501_STAGE_H

#include "REL/mg_cpp/game_object.h"

#include "REL/mg_cpp/singleton.h"

namespace m501 {
    class CStage : private CGameObject, public CSingleton<CStage> {
    public:
        CStage();
        ~CStage();
        void setAisuMotion();
    };
};

#endif