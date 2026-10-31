#ifndef M501_GAME_MASTER_H
#define M501_GAME_MASTER_H

#include "REL/mg_cpp/minigame_core.h"
#include "REL/mg_cpp/subthread.h"
#include "REL/mg_cpp/model.h"

namespace m501 {
    class CGameMaster : public CMinigameCore, public CSubThread {
    public:
        CGameMaster();
        ~CGameMaster();
    
    public:
        static const CModel &GetAisuModel();
    };
};

#endif