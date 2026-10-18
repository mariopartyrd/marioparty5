#ifndef MGCPP_LINKNODE_H
#define MGCPP_LINKNODE_H

class CLinkNodeBase;

typedef bool (*CLinkCompare)(CLinkNodeBase *a, CLinkNodeBase *b);

class CLinkNodeBase {
public:
    CLinkNodeBase();
    ~CLinkNodeBase();
    void removeLink();
    void insertFront(CLinkNodeBase *link);
    void insertBack(CLinkNodeBase *link);
    void copyLink(CLinkNodeBase *link);
    bool isInvalid()
    {
        return m_next == this && m_next == m_prev;
    }
    CLinkNodeBase *getNextNode()
    {
        return m_next;
    }
    
    CLinkNodeBase *getPrevNode()
    {
        return m_prev;
    }
    
    void sortList(CLinkCompare cmpFunc);
    static CLinkNodeBase *addSorted(CLinkCompare cmpFunc, CLinkNodeBase *node);
    static CLinkNodeBase *getLast(CLinkNodeBase *node);
    static CLinkNodeBase *getSortPos(CLinkCompare cmpFunc, CLinkNodeBase *a, CLinkNodeBase *b);

public:
    CLinkNodeBase *m_next;
    CLinkNodeBase *m_prev;
};

template <typename T>
class CLinkNode : private CLinkNodeBase {
public:
    CLinkNode() {}
    ~CLinkNode() {}

public:
    T *next()
    {
        return (T *)m_next;
    }
};
    
template <typename T>
class CLinkList : private CLinkNodeBase {
public:
    CLinkList() {}
    ~CLinkList() {}
public:
    T *Begin()
    {
        return (T *)getNextNode();
    }
    
    T *Last()
    {
        return (T *)getPrevNode();
    }
    
    T *End()
    {
        return (T *)this;
    }
    
    bool IsEmpty()
    {
        return m_next == this && m_next == m_prev;
    }
};

#endif