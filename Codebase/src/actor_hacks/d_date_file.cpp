#include <kamek.h>

#include <new/game_config.h>

// disable the disabling of the fileselect stars
// if the player has died more than 8 times in a level

#ifdef NO_SUPER_GUIDE
kmWriteNop(0x8077dd2c);
#endif