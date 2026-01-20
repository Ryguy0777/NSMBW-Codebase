#include <kamek.h>
#include <new/world_map_config.h>

#ifdef REMOVE_BOWSER_CASTLE_CLEAR_FLAG
kmWriteNop(0x808FF348);
#endif