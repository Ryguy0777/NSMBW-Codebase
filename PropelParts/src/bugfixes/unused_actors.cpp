#include <kamek.h>
#include <new/new_profile.hpp>

// Assign unused (but otherwise functional) actors to their original sprite ids

// "Jumbo Ray Respawner 2" (actor 539, sprite 273)
kmWrite16(0x8030cde8, 0x020E);


// "Basic Platform - Water" (actor 591, sprite 486)
const char* l_WATER_LIFT_res [] = {"lift_han_wood", NULL};
const dActorData_c c_WATER_LIFT_actor_data = {fProfile::WATER_LIFT, 0, 0, 0, 0, 120, 120, 0, 0, 0, 0, ACTOR_CREATE_MAPOBJ};
dCustomProfile_c l_WATER_LIFT_profile(SpriteId::WATER_LIFT, &c_WATER_LIFT_actor_data, l_WATER_LIFT_res);

