#include <kamek.h>
#include <new/world_map_config.h>

#ifdef REMOVE_CASTLE_CLEAR_FLAG
kmWriteNop(0x808CD044);
#endif