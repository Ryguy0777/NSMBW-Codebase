#include <kamek.h>
#include <game/bases/d_s_stage.hpp>
#include <new/bases/d_msgbox_window.hpp>

// Add new "manager" actors to the stage creation function

kmBranchDefCpp(0x80924718, 0x8092471c, void, dScStage_c *this_) {
    // Create original managers
    this_->mainLevelSetup();

    // New managers
    // Message box window
    dMsgBoxWindow_c::m_instance = (dMsgBoxWindow_c *)fBase_c::createChild(fProfile::MSGBOX_WINDOW, this_, 0, 0);
}