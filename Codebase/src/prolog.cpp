#include <kamek.h>
#include <revolution/OS/OSError.h>
#include <revolution/PAD.h>
#include <new/bases/d_custom_controller.hpp>
#include <new/bases/d_debug_config.hpp>

// prolog hook
// this is the equivalent of the prolog.S file from NewerSMBW
// anything that needs to be done on game boot should be placed here
// as it runs right after kamek hooks are patched in
kmBranchDefCpp(0x8015bd74, NULL, bool, bool ret) {
    OSReport("NSMBW Custom Codebase - Hacks by Ryguy\n");
    PADInit();
    dCustomController_c::makeInstance();
    dDebugConfig_c::setupConfig();
    return ret;
}