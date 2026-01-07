#pragma once

#include <types.h>
#include <new/sprite_ids.hpp>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_data.hpp>

// All the neccessary code for profile/actor expansion
// Check d_a_spawner.cpp for a usage example

typedef u16 SpriteName;

class dCustomProfile_c {
public:
    // Stage actor with sprite
    dCustomProfile_c(fProfile::fActorProfile_c *profile, const char* name, SpriteName spriteId, const dActorData_c *dActorData_c, const char** files=NULL);
    // Stage actor without sprite
    dCustomProfile_c(fProfile::fActorProfile_c *profile, const char* name, ProfileName profileId);
    // Non-stage actors (scenes, bases, etc)
    dCustomProfile_c(fProfile::fBaseProfile_c *profile, const char* name, ProfileName profileId);

    fProfile::fActorProfile_c mProfile;
};

