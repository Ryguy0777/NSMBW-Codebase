#include <kamek.h>
#include <game/bases/d_s_stage.hpp>
#include <new/bases/d_msgbox_window.hpp>

#include <new/world_map_config.h>

// Add new "manager" actors to the stage creation function

kmBranchDefCpp(0x80924718, 0x8092471c, void, dScStage_c *this_) {
    // Create original managers
    this_->mainLevelSetup();

    // New managers
    // Message box window
    dMsgBoxWindow_c::m_instance = (dMsgBoxWindow_c *)fBase_c::createChild(fProfile::MSGBOX_WINDOW, this_, 0, 0);
}

// Cannon destinations
kmWrite32(0x802F4CF0, CANNON_DEST_W1);
kmWrite32(0x802F4CF4, CANNON_DEST_W2);
kmWrite32(0x802F4CF8, CANNON_DEST_W3);
kmWrite32(0x802F4CFC, CANNON_DEST_W4);
kmWrite32(0x802F4D00, CANNON_DEST_W5);
kmWrite32(0x802F4D04, CANNON_DEST_W6);
kmWrite32(0x802F4D08, CANNON_DEST_W7);
kmWrite32(0x802F4D0C, CANNON_DEST_W8);
kmWrite32(0x802F4D10, CANNON_DEST_W9);
kmWrite32(0x802F4D14, CANNON_DEST_WA);