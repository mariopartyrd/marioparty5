#ifndef MGCPP_OM_H
#define MGCPP_OM_H

#include "game/object.h"

class COMObj {
public:
    COMObj(OMOBJ *obj);
    virtual ~COMObj() {}

public:
    virtual void on_update(OMOBJ *obj) {}

public:
    static void ObjUpdate(OMOBJ *obj);
    static void ObjDelete(OMOBJ *obj);
    static void ObjDelete(OMOBJMAN *objman, OMOBJ *obj);
};

#endif