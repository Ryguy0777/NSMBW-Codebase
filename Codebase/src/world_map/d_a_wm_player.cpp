#include <kamek.h>
#include <new/world_map_config.h>

#ifdef REMOVE_ITEMS_MENU
kmWriteNop(0x80904298);
#endif

#ifdef REMOVE_WORLD_SELECT
kmWriteNop(0x809042E4);
#endif

#ifdef REMOVE_MAP_VIEW
kmWriteNop(0x809041E8);
#endif

// Don't create WM_KINOPIO

#ifdef REMOVE_LETTER_TOAD
kmWriteNop(0x80903DCC);
#endif