#include <kamek.h>

#include <propelparts/game_config.h>

// Disable the super guide block if super guide is disabled
// Or if we're using the NewerSMBW pregame (incompatible with super guide)
// Note: the file select stars will stop shining after 8 deaths, if NEWER_PREGAME is enabling this patch

#if defined(NO_SUPER_GUIDE) || defined(NEWER_PREGAME)
kmWriteNop(0x80ac37f8);
#endif