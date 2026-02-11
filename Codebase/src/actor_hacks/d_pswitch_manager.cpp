#include <kamek.h>
#include <game/bases/d_pswitch_manager.hpp>

// dPSwManager_c hacks to allow for the ! Switch to activate switch blocks

// Patch dPSwManager_c::ProcMain to check for all switch types
kmWrite8(0x800d884b, 7);

// dPSwManager_c::getTimer
kmBranchDefCpp(0x800d88e0, NULL, int, dPSwManager_c *_self, dPSwManager_c::SwType_e type) {
    if (type > dPSwManager_c::HN_SWICH) {
        // Use the ! Switch timer for switch block switches
        return _self->mSwitchTimers[dPSwManager_c::Q_SWICH];
    } else {
        return _self->mSwitchTimers[type];
    }
}

// dPSwManager_c::setTimer
kmBranchDefCpp(0x800d8930, NULL, void, dPSwManager_c *_self, dPSwManager_c::SwType_e type, int time) {
    if (type > dPSwManager_c::HN_SWICH) {
        // Use the ! Switch timer for switch block switches
        _self->mSwitchTimers[dPSwManager_c::Q_SWICH] = time;
    } else {
        _self->mSwitchTimers[type] = time;
    }
}