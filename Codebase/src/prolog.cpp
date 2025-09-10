#include <kamek.h>
#include <lib/revolution/OS/OSError.h>

// prolog hook
// this is the equivalent of the prolog.S file from NewerSMBW
// anything that needs to be done on game boot should be placed here
// as it runs right after kamek hooks are patched in
kmBranchDefCpp(0x8015bd74, NULL, bool, bool ret) {
    OSReport("NSMBW Custom Codebase - Hacks by Ryguy\n");
    return ret;
}