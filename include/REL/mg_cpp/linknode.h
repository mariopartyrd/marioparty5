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
    CLinkNodeBase *getNextNode() const
    {
        return m_next;
    }
    
    CLinkNodeBase *getPrevNode() const
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
    T *next() const
    {
        return (T *)m_next;
    }
    
    void removeLink()
    {
        CLinkNodeBase::removeLink();
    }
};
    
template <typename T>
class CLinkList : private CLinkNodeBase {
public:
    CLinkList() {}
    ~CLinkList() {}
public:
    T *Begin() const
    {
        return (T *)getNextNode();
    }
    
    T *Last()
    {
        return (T *)getPrevNode();
    }
    
    T *End() const
    {
        return (T *)this;
    }
    
    void Copy(CLinkList *other)
    {
        copyLink(other);
    }
    
    bool IsEmpty() const
    {
        return m_next == this && m_next == m_prev;
    }
    
    void InsertBack(T *node)
    {
        ((CLinkNodeBase *)node)->insertBack(this);
    }
};

#endif