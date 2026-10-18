#include "REL/mg_cpp/game_object.h"

CLinkList<CGameObject> CGameObject::s_objectList;

CGameObject::ObjPhase CGameObject::s_phase;

void CGameObject::SetGlobalPhase(CGameObject::ObjPhase phase)
{
    CGameObject *cur = s_objectList.Begin();
    CGameObject *end = s_objectList.End();
    s_phase = phase;
    while(cur != end) {
        cur->m_phaseBit |= 1 << phase;
        cur->set_phase(phase);
        cur = cur->next();
    }
}

bool CGameObject::IsAllStop()
{
    CGameObject *cur = s_objectList.Begin();
    CGameObject *end = s_objectList.End();
    while(cur != end) {
        CGameObject::ObjPhase phase = CGameObject::s_phase;
        if(cur->m_phaseBit & (1 << phase)) {
            return false;
        }
        cur = cur->next();
    }
    return true;
}

void CGameObject::UpdateAll()
{
    CGameObject *cur = s_objectList.Begin();
    CGameObject *end = s_objectList.End();
    while(cur != end) {
        cur->on_update();
        cur = cur->next();
    }
}

void CGameObject::on_update()
{
    
}

void CGameObject::Close()
{
    while(!s_objectList.IsEmpty()) {
        delete s_objectList.Last();
    }
}

CGameObject::ObjPhase CGameObject::GetGlobalPhase()
{
    return s_phase;
}