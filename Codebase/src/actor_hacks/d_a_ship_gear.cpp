#include <kamek.h>

// Stop the airship wheels if nybble 5 is 1

kmBranchDefAsm(0x808A20E0, 0x808A20E4) {
    sth r0, 0x51C(r3) // Replaced insn

    lwz r8, 0x4(r3)
    rlwinm r7, r8, 0x4, 0x1C, 0x1F // m_param >> 0x1C
    cmpwi r7, 0x1
    bne _over

    // Set mRotateSpeed to 0
    li r0, 0x0
    sth r0, 0x51C(r3)

_over:
    blr
}
