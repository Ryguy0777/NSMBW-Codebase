#include <kamek.h>
#include <new/world_map_config.h>

// Remove hardcoded effects

#ifdef REMOVE_EFF_W3_SNOW
kmWriteNop(0x80321978);
kmWriteNop(0x80321988);
#endif

#ifdef REMOVE_EFF_W5_POISON
kmWriteNop(0x803219CC);
#endif

#ifdef REMOVE_EFF_W8_LAVA
kmWriteNop(0x80321998);
kmWriteNop(0x803219A8);
#endif

#ifdef REMOVE_EFF_W9_COMETS
kmWriteNop(0x803219E0);
#endif

#ifdef REMOVE_EFF_BOWSER_CASTLE_FIRE
kmWriteNop(0x80321AA4);
#endif

#ifdef REMOVE_EFF_KINOPIO_SWEAT
kmWriteNop(0x80321A08);
#endif