#pragma once

#include <types.h>
#include <game/bases/d_panel_obj_list.hpp>

class dPanelObjMgr_c {
public:
    dPanelObjMgr_c();
    ~dPanelObjMgr_c();

    void addPanelObjList(dPanelObjList_c *);
    void removePanelObjList(dPanelObjList_c *);

    u32 mCount;
    dPanelObjList_c *mpFirst, *mpLast;
};