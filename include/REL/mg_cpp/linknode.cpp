#include "REL/mg_cpp/linknode.h"

CLinkNodeBase::CLinkNodeBase()
{
    m_next = this;
    m_prev = this;
}

CLinkNodeBase::~CLinkNodeBase()
{
    m_next->m_prev = m_prev;
    m_prev->m_next = m_next;
}

void CLinkNodeBase::removeLink()
{
    m_next->m_prev = m_prev;
    m_prev->m_next = m_next;
    m_next = this;
    m_prev = this;
}

void CLinkNodeBase::insertFront(CLinkNodeBase *link)
{
    CLinkNodeBase *newLink = link;
    m_prev = newLink;
    m_next = newLink->m_next;
    newLink->m_next->m_prev = this;
    newLink->m_next = this;
}

void CLinkNodeBase::insertBack(CLinkNodeBase *link)
{
    CLinkNodeBase *newLink = link;
    m_next = newLink;
    m_prev = newLink->m_prev;
    newLink->m_prev->m_next = this;
    newLink->m_prev = this;
}

void CLinkNodeBase::copyLink(CLinkNodeBase *link)
{
    if(!isInvalid()) {
        CLinkNodeBase *newLink = link;
        CLinkNodeBase *next = m_next;
        CLinkNodeBase *prev = m_prev;
        m_next = this;
        m_prev = this;
        next->m_prev = newLink->m_prev;
        prev->m_next = newLink;
        newLink->m_prev->m_next = next;
        newLink->m_prev = prev;
    }
}

void CLinkNodeBase::sortList(CLinkCompare cmpFunc)
{
    if(!isInvalid()) {
        m_prev->m_next = nullptr;
        CLinkNodeBase *cur = CLinkNodeBase::addSorted(cmpFunc, m_next);
        CLinkNodeBase *prev;
        m_next = cur;
        for(prev=this; cur; cur=cur->m_next) {
            cur->m_prev = prev;
            prev = cur;
        }
        m_prev = prev;
        prev->m_next = this;
    }
}

CLinkNodeBase *CLinkNodeBase::addSorted(CLinkCompare cmpFunc, CLinkNodeBase *node)
{
    if(node == nullptr) {
        return nullptr;
    }
    if(node->m_next == nullptr) {
        return node;
    }
    CLinkNodeBase *a = node;
    CLinkNodeBase *b = getLast(a);
    a = addSorted(cmpFunc, a);
    b = addSorted(cmpFunc, b);
    //Cast required to match
    (void)a;
    return getSortPos(cmpFunc, a, b);
}

CLinkNodeBase *CLinkNodeBase::getLast(CLinkNodeBase *node)
{
    CLinkNodeBase *cur;
    cur = node->m_next;
    if(cur) {
        cur = cur->m_next;
    }
    while(cur) {
        node = node->m_next;
        cur = cur->m_next;
        if(cur) {
            cur = cur->m_next;
        }
        
    }
    cur = node->m_next;
    node->m_next = nullptr;
    return cur;
}

CLinkNodeBase *CLinkNodeBase::getSortPos(CLinkCompare cmpFunc, CLinkNodeBase *a, CLinkNodeBase *b)
{
    //Must be array of 2 pointers to not call CLinkNodeBase constructor
    CLinkNodeBase *pos[2];
    CLinkNodeBase **pPos;
    
    pPos = &pos[0];
    while(a && b) {
        if(cmpFunc(a, b)) {
            *pPos = a;
            pPos = &a->m_next;
            a = a->m_next;
        } else {
            *pPos = b;
            pPos = &b->m_next;
            b = b->m_next;
        }
    }
    if(a == nullptr) {
        *pPos = b;
    } else {
        *pPos = a;
    }
    return pos[0];
}