#include <kamek.h>
#include <new/world_map_config.h>

// Remove "hint" messages

#ifdef REMOVE_MSG_CASTLE
kmWrite8(0x808FC98F, 0x01);
#endif

#ifdef REMOVE_MSG_HOUSE
kmWrite8(0x808FC36B, 0x01);
#endif

#ifdef REMOVE_MSG_GAMEOVER
kmWrite8(0x808FC72F, 0x01);
#endif