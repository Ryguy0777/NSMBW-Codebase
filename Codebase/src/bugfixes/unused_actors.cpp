#include <kamek.h>
#include <new/new_profile.hpp>

// assign unused (but otherwise functional) actors to their original sprite ids

// "Jumbo Ray Respawner 2" (actor 539, sprite 273)
kmWrite16(0x8030cde8, 0x020E);


// "Basic Platform - Water" (actor 591, sprite 486)
const char* WaterliftFileList [] = {"lift_han_wood", NULL};
const SpriteData WaterliftSpriteData = {fProfile::WATER_LIFT, 0, 0, 0 , 0, 0x10, 0x10, 0, 0, 0, 0, 8};
extern fProfile::fActorProfile_c g_profile_WATER_LIFT;
dCustomProfile_c WaterliftProfile(&g_profile_WATER_LIFT, "WATER_LIFT", SpriteId::WATER_LIFT, &WaterliftSpriteData, WaterliftFileList);

