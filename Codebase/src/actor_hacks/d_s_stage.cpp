#include <kamek.h>
#include <game/bases/d_s_stage.hpp>
#include <new/bases/d_msgbox_window.hpp>

// add new "manager" actors to the stage creation function

kmBranchDefCpp(0x80924718, 0x8092471c, void, dScStage_c *_this) {
    // create original managers
    _this->mainLevelSetup();
    // new actors

    // message box window
    dMsgBoxWindow_c::m_instance = (dMsgBoxWindow_c *)fBase_c::createChild(fProfile::MSGBOX_WINDOW, _this, 0, 0);
}