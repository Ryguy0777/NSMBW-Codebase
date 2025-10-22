#include <kamek.h>

#include <new/game_config.h>

// disable the super guide block if super guide is disabled
// or if we're using the NewerSMBW pregame (incompatible with super guide)
// note: the fileselect stars will stop shining after 8 deaths, if NEWER_PREGAME is enabling this patch

#if defined(NO_SUPER_GUIDE) || defined(NEWER_PREGAME)
kmWriteNop(0x80ac37f8);
#endif