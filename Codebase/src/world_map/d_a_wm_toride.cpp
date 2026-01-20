#include <kamek.h>
#include <new/world_map_config.h>

#ifdef REMOVE_TOWER_CLEAR_FLAG
kmWriteNop(0x808F2C68);
#endif