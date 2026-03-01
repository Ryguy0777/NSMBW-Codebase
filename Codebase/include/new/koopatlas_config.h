#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED

// DEBUGGING MODULES
    #define KP_MAP_REPORT           // Prints general map info (layouts, texmaps, general data) to the log
    #define KP_SPAMMY_DEBUG         // Prints map setup and path info to the log, this "spams" the log

    //#define KP_PATH_MNG_REPORT      // Prints occasional info from the Path Manager
    //#define KP_UNLOCK_DEBUG         // Prints debugging info about path unlock conditions

    #define KP_PATH_QUICK_UNLOCK    // Debug tool to quickly unlock paths -- TODO: Remove me(?)
    //#define KP_CHK_SET_DEFAULT_NODE // When loading into a map, hold -, +, and 1 to be set on path node 0

    #define KP_MUSIC_REPORT         // Prints music related info to the log
    //#define KP_MUSIC_DEBUGPRINT     // Prints music info to the Exception Handler (highly recommend disabling for mods)

    //#define KP_CAMERA_REPORT        // Prints camera info to the log
    //#define KP_CAMERA_DEBUG         // Prints camera info + screen variables to the log (more verbose than the above)

    //#define KP_MAP_TILE_REPORT      // Prints tile info to the log
    //#define KP_MAP_BOUND_REPORT     // Prints rendering bound info to the log
    //#define KP_MAP_DOODAD_REPORT    // Prints doodad info to the log

// TOGGLABLE FEATURES
    // Enables a debug menu which can be displayed by pressing (UNDETERMINED) at any time
    //#define KP_ENABLE_DEBUG_MENU

    // Skips drawing the topmost layer in Map 0 after the first show of the map
    // Used in Newer for the initial fade-in clouds in W1
    #define KP_SKIP_TOP_LAYER_W1

    // Hides the completion stars on the HUD if the LevelInfo world is 15 (F)
    // Intended for Newer's scrapped "Cutland" world
    #define KP_HIDE_STARS_W15

    // Plays the "invalid choice" sound when trying to enter completed one-time courses
    #define KP_PLAY_INVALID_SND

#endif