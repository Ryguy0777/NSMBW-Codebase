#include <kamek.h>
#include <new/game_config.h>

// Remove the life counter decrement on death, for mods that may prefer "infinite" lives

#ifdef NO_LIFE_DECREMENT
kmWriteNop(0x8013d9f8);
#endif