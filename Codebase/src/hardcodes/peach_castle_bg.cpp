#include <kamek.h>
#include <new/game_config.h>

// remove the hardcoded static bg in 1-1
#ifdef NO_1_1_BG
kmWrite32(0x801180AC, 0x38600003);
#endif