#pragma once

// The majority of these were ported from NSMBWer+'s worldmaphax.yaml

// Warp Cannon destinations (world num - 1)
#define CANNON_DEST_W1 4
#define CANNON_DEST_W2 4
#define CANNON_DEST_W3 5
#define CANNON_DEST_W4 5
#define CANNON_DEST_W5 7
#define CANNON_DEST_W6 7
#define CANNON_DEST_W7 10
#define CANNON_DEST_W8 10
#define CANNON_DEST_W9 10
#define CANNON_DEST_WA 10

// Ambush Removals
//#define REMOVE_W1_GOOMBA
//#define REMOVE_W2_ANTLION
//#define REMOVE_W3_BROS
//#define REMOVE_W4_FISH
//#define REMOVE_W5_PIRANHA
//#define REMOVE_W6_BULLET
//#define REMOVE_W7_LAKITU
//#define REMOVE_W8_BUBBLE

// Effect Removals
//#define REMOVE_EFF_W3_SNOW
//#define REMOVE_EFF_W5_POISON
//#define REMOVE_EFF_W8_LAVA
//#define REMOVE_EFF_W9_COMETS

//#define REMOVE_EFF_BOWSER_CASTLE_FIRE
//#define REMOVE_EFF_KINOPIO_SWEAT // Sweat from the Toad Rescue bubbles

// Koopaling/Airship Removals
//#define REMOVE_KOOPA_SHIP
//#define REMOVE_KOOPALINGS // Requires the Airship to be removed, or the game will crash

// Disable Menus
//#define REMOVE_ITEMS_MENU
//#define REMOVE_WORLD_SELECT
//#define REMOVE_MAP_VIEW

// Remove extra actors
//#define REMOVE_GHOST_HOUSE_BOO
//#define REMOVE_TOWER_CLEAR_FLAG
//#define REMOVE_CASTLE_CLEAR_FLAG
//#define REMOVE_BOWSER_CASTLE_CLEAR_FLAG

// Remove world-specific actors
//#define REMOVE_W2_PALM
//#define REMOVE_W4_PALM
//#define REMOVE_W5_WIGGLER
//#define REMOVE_W5_VINES
//#define REMOVE_W6_CLOUD
//#define REMOVE_W7_CLOUDS
//#define REMOVE_W7_VINES

//////////////////////////////////
// Others
//////////////////////////////////

// Completely remove the Toad Rescue mechanic
//#define REMOVE_TOAD_RESCUE

// Remove the Toad with the letter after 1-3
//#define REMOVE_LETTER_TOAD

// Hint messages shown after exiting certain courses
//#define REMOVE_MSG_CASTLE
//#define REMOVE_MSG_HOUSE
//#define REMOVE_MSG_GAMEOVER

//////////////////////////////////
// World Map resource lists
//////////////////////////////////

// Global resources
// These are loaded for every map
static const char *sc_GlobalResList[] = {
    "RouteInfo",
    "CollectionCoinCourseSort",
    "cobCastle",
    "cobCourse",
    "cobKinokoRed",
    "cobKinoko1up",
    "cobKinokoStar",
    "cobStart",
    "cobAnchor",
    "cobToride",
    "cobGhost",
    "cobCannon",
    "cobKoopaShip",
    "character_SV",
    "cobSurrender",
    "cobKinopio",
    // Start of new entries
    "mapModelCull",
};
