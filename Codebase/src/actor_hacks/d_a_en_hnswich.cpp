#include <kamek.h>

// Add support for colored ! Switches that fill in associated switch blocks
// Nybbles 12.0-12.3 in Reggie are now used as a "switch type" 

// Originally, the switch will place itself upside down if 
// Nybble 12 is greater than 1, we change this to just check if nybble 12.4 is 1
kmWrite32(0x80a198d0, 0x550007fe);

// Change the "activation type" for the switch manager when a ! Switch is pressed
kmBranchDefAsm(0x80a1b0c8, 0x80a1b0cc) {
    // Load mParam of this (r30) into r12
    lwz r12, 0x4(r30)
    // mParam >> 1 & 7
    rlwinm r4, r12, 31, 29, 31
    // Add 1 to differentiate it from P Switch
    addi r4, r4, 1
    blr
}