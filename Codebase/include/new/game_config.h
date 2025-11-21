#pragma once

// defines to toggle certain code modules should be defined here.

//#define PLAYER_MODEL_HACKS

#define CONTROLLER_EXPANSION_ENABLED

#define NO_1_1_BG

//#define NO_2_CASTLE_MAZE

#define NO_SUPER_GUIDE

//#define PLAYER_1_CAN_CHANGE_CHARACTER

//#define NO_LIFE_DECREMENT

//#define NO_TIMER

//#define DISABLE_QUICK_SAVE

#define LEVEL_INFO_ENABLED
#define LEVEL_INFO_UTILS_ENABLED

// These require LevelInfo and the utils to be enabled
//#define NEWER_PREGAME
//#define NEWER_FILE_SELECT
//#define NEWER_PAUSE_MENU

//#define KOOPATLAS_ENABLED

#define KOOPATLAS_DEV_ENABLED // Temporary, this is used to enable ONLY ported portions of KP

// KOOPATLAS CONFIG
#ifdef KOOPATLAS_DEV_ENABLED

// DEBUGGING
    #define KP_MUSIC_REPORT         // Prints music related info to the log
    #define KP_MUSIC_DEBUGPRINT     // Writes music to the Exception Handler
    //#define KP_PATH_QUICK_UNLOCK    // Debug tool to quickly unlock paths
    //#define KP_CAMERA_REPORT        // Prints camera info to the log
    //#define KP_CAMERA_DEBUG         // Prints camera + screen variables to the log
    #define KP_MAP_REPORT           // Prints general map info (layouts, texmaps, general data) to the log
    #define KP_SPAMMY_DEBUG         // Prints map setup and path info to the log, this "spams" the log

// TOGGLABLE FEATURES

#endif