#include <kamek.h>

#include <new/game_config.h>

// Always show "Save Game" text
#ifdef DISABLE_QUICK_SAVE
kmWriteNop(0x8077AA7C);
#endif