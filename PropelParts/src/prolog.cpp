#include <kamek.h>
#include <propelparts/game_config.h>
#include <revolution/OS/OSError.h>
#include <propelparts/lib/revolution/PAD.h>
#include <propelparts/bases/d_custom_controller.hpp>
#include <propelparts/bases/d_debug_config.hpp>
#include <propelparts/bases/d_level_info.hpp>
#include <propelparts/bases/d_world_info.hpp>

// Prolog hook
// This is the equivalent of the prolog.S file from NewerSMBW
// Anything that needs to be done on game boot should be placed here
// As it runs right after kamek hooks are patched in
kmBranchDefCpp(0x8015bd74, NULL, bool, bool ret) {
    OSReport("PropelParts - Hacks by Ryguy\n");
    PADInit();
    dCustomController_c::makeInstance();
    ret &= dDebugConfig_c::setupConfig();
#ifdef LEVEL_INFO_ENABLED
    ret &= dLevelInfo_c::loadLevelInfo();
#endif
    ret &= dWorldInfo_c::loadWorldInfo();
    return ret;
}