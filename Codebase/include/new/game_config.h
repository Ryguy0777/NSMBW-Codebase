#pragma once

// defines to toggle certain code modules should be defined here.

//#define PLAYER_MODEL_HACKS

#define CONTROLLER_EXPANSION_ENABLED

#define NO_1_1_BG

//#define NO_2_CASTLE_MAZE

//#define NEWER_PREGAME

#define NO_SUPER_GUIDE

//#define PLAYER_1_CAN_CHANGE_CHARACTER

//#define NO_LIFE_DECREMENT

//#define NO_TIMER

//#define NEWER_FILE_SELECT

#define DISABLE_QUICK_SAVE

//#define KOOPATLAS_ENABLED

//#define KOOPATLAS_DEV_ENABLED // Temporary, this is used to enable ONLY ported portions of KP

// Koopatlas Config
#ifdef KOOPATLAS_ENABLED

#define KP_MUSIC_REPORT
#define KP_MUSIC_DEBUGPRINT
//#define KP_PATH_DEBUG

#endif