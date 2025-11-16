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

// KOOPATLAS CONFIG (todo - write quick descriptions for all of these)
#ifdef KOOPATLAS_DEV_ENABLED
    #define KP_MUSIC_REPORT
    #define KP_MUSIC_DEBUGPRINT
    //#define KP_PATH_QUICK_UNLOCK
    //#define KP_CAMERA_REPORT

#endif