#include <kamek.h>

#include <new/game_config.h>

// Use regular save behavior
#ifdef DISABLE_QUICK_SAVE
kmWriteNop(0x8092FD08);
#endif