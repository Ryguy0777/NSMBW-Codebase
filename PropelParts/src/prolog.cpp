#include <kamek.h>
#include <revolution/OS/OSError.h>
#include <new/lib/revolution/PAD.h>
#include <new/bases/d_custom_controller.hpp>
#include <new/bases/d_debug_config.hpp>

// Prolog hook
// This is the equivalent of the prolog.S file from NewerSMBW
// Anything that needs to be done on game boot should be placed here
// As it runs right after kamek hooks are patched in
kmBranchDefCpp(0x8015bd74, NULL, bool, bool ret) {
    OSReport("PropelParts - Hacks by Ryguy\n");
    PADInit();
    dCustomController_c::makeInstance();
    ret &= dDebugConfig_c::setupConfig();
    return ret;
}