#pragma once
#include <game/bases/d_base.hpp>

/// @unofficial
class dCourseSelectManager_c : public dBase_c {
public:
    void saveGame(bool isQuickSave);

    static dCourseSelectManager_c *m_instance;
};