#include <kamek.h>
#include <propelparts/game_config.h>

// Remove the hardcoded static bg in 1-1
#ifdef NO_1_1_BG
kmWrite32(0x801180AC, 0x38600003);
#endif