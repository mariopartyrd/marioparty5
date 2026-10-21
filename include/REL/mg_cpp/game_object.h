#ifndef MGCPP_GAME_OBJECT_H
#define MGCPP_GAME_OBJECT_H

#include "REL/mg_cpp/assert.h"
#include "REL/mg_cpp/linknode.h"

class CGameObject : private CLinkNode<CGameObject> {
public:
    CGameObject() {}
    virtual ~CGameObject() {
        
    }
    
public:
    enum ObjPhase {
        PHASE_INIT,
        PHASE_START,
        PHASE_GAME,
        PHASE_FINISH,
        PHASE_CLOSE
    };

    virtual void set_phase(ObjPhase phase) = 0;
    virtual void on_update() {}

public:
    static void SetGlobalPhase(ObjPhase phase);
    static bool IsAllStop();
    static void UpdateAll();
    static void Close();
    static ObjPhase GetGlobalPhase();
    
    
private:
    int m_phaseBit;
    
public:
    static CLinkList<CGameObject> s_objectList;
    static ObjPhase s_phase;
};


#endif