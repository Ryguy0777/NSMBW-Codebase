#include <kamek.h>

// Fix the bug where fence climbing enemies will spawn unlimited coins
// If frozen during a wire fence flip

extern "C" void dontFlipNetEnemy(void);
kmBranchDefAsm(0x80044994, 0x80044998) {
    nofralloc

    lwz r0, 0x490(r3) // Load dIceMng_c.mIsFrozen
    cmpwi r0, 0 // Check if we're not frozen
    beq notFrozen
    b dontFlipNetEnemy

    notFrozen:
    lwzu r12, 0x394(r3) // Original instruction
    blr
}