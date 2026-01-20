#include <kamek.h>
#include <new/world_map_config.h>

#ifdef REMOVE_TOAD_RESCUE

// dWmLib::IsKinopioHelpStart() always return false
kmBranchDefAsm(0x800FCB60, NULL) {
    li r3, 0x0
    blr
}

// Skip WM_KINOBALLOON creation
kmWrite32(0x808E1A5C, 0x48000110);
kmWrite32(0x808E1C1C, 0x48000050);

// Remove hiding Toad Balloons
kmWrite32(0x808D8720, 0x4E800020);

// Skip something with Toad Balloon sounds
kmWrite32(0x80905424, 0x38600001);

// Skip Toad Rescue course searching
kmWrite32(0x809273C0, 0x48000038);
kmWriteNop(0x80927464);
#endif

// Prevent Kinopio Help Demo from ever playing
#ifdef REMOVE_LETTER_TOAD
kmWrite32(0x800FCB04, 0x4E800020);
#endif