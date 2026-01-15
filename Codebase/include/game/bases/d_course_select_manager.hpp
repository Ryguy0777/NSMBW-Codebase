#pragma once
#include <game/bases/d_base.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dCourseSelectManager_c : public dBase_c {
public:
    void saveGame(bool isQuickSave);
    void changeToNormalState();

    sFStateMgr_c<dCourseSelectManager_c, sStateMethodUsr_FI_c> mStateMgr;

    static dCourseSelectManager_c *m_instance;

    STATE_FUNC_DECLARE(dCourseSelectManager_c, CollectionCoinWait);
};