#pragma once

#include <types.h>
#include <new/sprite_ids.hpp>
#include <game/framework/f_profile.hpp>

struct SpriteData {
    u16 profileId;
    s32 xoffs;
    s32 yoffs;
    s32 spawnRangeOffset[2];
    s32 spawnRangeSize[2];
    u16 _1C;
    u16 _1E;
    u16 _20;
    u16 _22;
    /* Values:
    0x2: always spawn even if offscreen
    0x8: something with z-order
    0x10: grouped sprite */
    u16 flags;
};

class dCustomProfile_c {
public:
    // stage actor
    dCustomProfile_c(fProfile::fActorProfile_c *profile, const char* name, u16 spriteId, const SpriteData *spriteData, const char** files=NULL);
    // scenes, bases, etc
    dCustomProfile_c(fProfile::fBaseProfile_c *profile, const char* name, u16 profileId);

    fProfile::fActorProfile_c mProfile;
};

