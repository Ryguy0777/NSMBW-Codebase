#include <kamek.h>
#include <new/world_map_config.h>

// Remove airship actor
#ifdef REMOVE_KOOPA_SHIP
kmWrite32(0x808DBEE0, 0x4E800020);
#endif