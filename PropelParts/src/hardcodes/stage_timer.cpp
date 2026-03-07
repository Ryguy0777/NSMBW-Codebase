#include <kamek.h>
#include <propelparts/game_config.h>

// Removes the timer from the stage.

#ifdef NO_TIMER
kmWriteNop(0x800e3abc);

kmWriteNop(0x80158f28);
kmWriteNop(0x80158e0c);
kmWriteNop(0x8015910c);
kmWriteNop(0x8015915c);
#endif