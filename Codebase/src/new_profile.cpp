#include <kamek.h>
#include <new/new_profile.hpp>
#include <revolution/OS/OSError.h>

#define DEBUG_PROFILE_CALL

// sprites

extern SpriteData spriteDataTable[SPRITE_COUNT];
SpriteData customSpriteDataTable[SpriteId::TOTAL_SPRITE_COUNT - SPRITE_COUNT];

extern const char** spriteFiles[SPRITE_COUNT];
const char** customSpriteFiles[SpriteId::TOTAL_SPRITE_COUNT - SPRITE_COUNT];

// profiles

extern fProfile::fProfilePtr_c* profileList[fProfile::PROFILE_COUNT];
fProfile::fProfilePtr_c* customProfileList[fProfile::TOTAL_ACTOR_COUNT - fProfile::PROFILE_COUNT];

extern const char* profileNames[fProfile::PROFILE_COUNT];
const char* customProfileNames[fProfile::TOTAL_ACTOR_COUNT - fProfile::PROFILE_COUNT];

// custom profile ctors

// stage actor with sprite
dCustomProfile_c::dCustomProfile_c(fProfile::fActorProfile_c *profile, const char* name, SpriteName spriteId, const SpriteData *spriteData, const char** files) {
#ifdef DEBUG_PROFILE_CALL
    OSReport("Profile Setter: Sprite, %p, %s, %d, %d, %p, %p\n", profile, name, spriteData->profileId, spriteId, spriteData, files);
#endif
    // store profile in the class
    mProfile.mpClassInit = profile->mpClassInit;
    mProfile.mExecuteOrder = profile->mExecuteOrder;
    mProfile.mDrawOrder = profile->mDrawOrder;
    mProfile.mActorProperties = profile->mActorProperties;

    // store sprite data
    ProfileName profileId = spriteData->profileId;
    if (spriteId < SPRITE_COUNT) {
        spriteDataTable[spriteId] = *spriteData;
        spriteFiles[spriteId] = files;
    } else {
        customSpriteDataTable[spriteId - SPRITE_COUNT] = *spriteData;
        customSpriteFiles[spriteId - SPRITE_COUNT] = files;
    }

    // store profile data
    if (profileId < fProfile::PROFILE_COUNT) {
        profileList[profileId] = (fProfile::fProfilePtr_c*)&mProfile;
        profileNames[profileId] = name;
    } else {
        customProfileList[profileId - fProfile::PROFILE_COUNT] = (fProfile::fProfilePtr_c*)&mProfile;
        customProfileNames[profileId - fProfile::PROFILE_COUNT] = name;
    }
}

// stage actor without sprite
dCustomProfile_c::dCustomProfile_c(fProfile::fActorProfile_c *profile, const char* name, ProfileName profileId) {
#ifdef DEBUG_PROFILE_CALL
    OSReport("Profile Setter: Actor, %p, %s, %d\n", profile, name, profileId);
#endif
    // store profile in the class
    mProfile.mpClassInit = profile->mpClassInit;
    mProfile.mExecuteOrder = profile->mExecuteOrder;
    mProfile.mDrawOrder = profile->mDrawOrder;
    mProfile.mActorProperties = profile->mActorProperties;

    // store profile data
    if (profileId < fProfile::PROFILE_COUNT) {
        profileList[profileId] = (fProfile::fProfilePtr_c*)&mProfile;
        profileNames[profileId] = name;
    } else {
        customProfileList[profileId - fProfile::PROFILE_COUNT]= (fProfile::fProfilePtr_c*)&mProfile;
        customProfileNames[profileId - fProfile::PROFILE_COUNT] = name;
    }
}

// non-stage actors
dCustomProfile_c::dCustomProfile_c(fProfile::fBaseProfile_c *profile, const char* name, ProfileName profileId) {
#ifdef DEBUG_PROFILE_CALL
    OSReport("Profile Setter: Base, %p, %s, %d\n", profile, name, profileId);
#endif
    // store profile in the class
    mProfile.mpClassInit = profile->mpClassInit;
    mProfile.mExecuteOrder = profile->mExecuteOrder;
    mProfile.mDrawOrder = profile->mDrawOrder;

    // store profile data
    if (profileId < fProfile::PROFILE_COUNT) {
        profileList[profileId] = (fProfile::fProfilePtr_c*)&mProfile;
        profileNames[profileId] = name;
    } else {
        customProfileList[profileId - fProfile::PROFILE_COUNT]= (fProfile::fProfilePtr_c*)&mProfile;
        customProfileNames[profileId - fProfile::PROFILE_COUNT] = name;
    }
}

#ifdef IS_GAME_VERSION_DYNAMIC
#error Dynamic compilation is unsupported for this patch.
#endif

// Profile count for ASM snippets
#if GAME_REVISION == GAME_REVISION_C
    #define PROFCNT 754
#elif GAME_REVISION >= GAME_REVISION_K
    #define PROFCNT 752
#else
    #define PROFCNT 750
#endif

/* Spritedata List hooks */

kmCallDefAsm(0x80068440) {
    // Move r30 to r4 to preserve it, as we are inside a loop
    mr r4, r30

    // Check if original sprite (using cr7 because cr0 is in use)
    cmpwi cr7, r0, SPRITE_COUNT*0x28
    blt+ cr7, notCustom

    // Subtract using sub rather than subi because r0
    li r12, SPRITE_COUNT*0x28
    sub r0, r0, r12

    // Override table address
    lis r4, customSpriteDataTable@h
    ori r4, r4, customSpriteDataTable@l

    // Modified original instruction
    notCustom:
    add r28, r4, r0
    blr
}

kmCallDefAsm(0x80068E18) {
    // Check if original sprite
    cmpwi r0, SPRITE_COUNT*0x28
    blt+ notCustom

    // Subtract using sub rather than subi because r0
    li r12, SPRITE_COUNT*0x28
    sub r0, r0, r12

    // Override table address
    lis r9, customSpriteDataTable@h
    ori r9, r9, customSpriteDataTable@l

    // Original instruction
    notCustom:
    lhzx r0, r9, r0
    blr
}

kmCallDefAsm(0x80068F50) {
    // Check if original sprite
    cmpwi r0, SPRITE_COUNT*0x28;
    blt+ notCustom

    // Subtract using sub rather than subi because r0
    li r12, SPRITE_COUNT*0x28
    sub r0, r0, r12

    // Override table address
    lis r3, customSpriteDataTable@h
    ori r3, r3, customSpriteDataTable@l

    // Original instruction
    notCustom:
    lhzx r3, r3, r0
    blr
}

kmCallDefAsm(0x807FC8D8) {
    // Check if original sprite
    cmpwi r0, SPRITE_COUNT*0x28;
    blt+ notCustom

    // Subtract using sub rather than subi because r0
    li r12, SPRITE_COUNT*0x28
    sub r0, r0, r12

    // Override table address
    lis r7, customSpriteDataTable@h
    ori r7, r7, customSpriteDataTable@l

    // Original instruction
    notCustom:
    lhzx r31, r7, r0
    blr
}

kmCallDefAsm(0x8006894C) {
    // Check if original sprite (using cr7 because cr0 is in use)
    cmpwi cr7, r4, SPRITE_COUNT*0x28;
    blt+ cr7, notCustom

    // Subtract
    subi r4, r4, SPRITE_COUNT*0x28

    // Override table address
    lis r0, customSpriteDataTable@h
    ori r0, r0, customSpriteDataTable@l

    // Original instruction
    notCustom:
    add r31, r0, r4
    blr
}

/* Profile List hooks */

kmCallDefAsm(0x8006C7C8) {
    // Check if original sprite
    cmpwi r0, PROFCNT*4;
    blt+ notCustom

    // Subtract
    li r12, PROFCNT*4
    sub r0, r0, r12

    // Override table address
    lis r4, customProfileList@h
    ori r4, r4, customProfileList@l

    // Original instruction
    notCustom:
    lwzx r4, r4, r0
    blr
}

kmCallDefAsm(0x80161CF4) {
    // Check if original sprite
    cmpwi r0, PROFCNT*4;
    blt+ notCustom

    // Subtract
    li r12, PROFCNT*4
    sub r0, r0, r12

    // Override table address
    lis r3, customProfileList@h
    ori r3, r3, customProfileList@l

    // Original instruction
    notCustom:
    lwzx r3, r3, r0
    blr
}

kmCallDefAsm(0x80162BC8) {
    // Check if original sprite
    cmpwi r31, PROFCNT*4;
    blt+ notCustom

    // Subtract
    subi r31, r31, PROFCNT*4

    // Override table address
    lis r7, customProfileList@h
    ori r7, r7, customProfileList@l

    // Original instruction
    notCustom:
    lwzx r0, r7, r31
    blr
}

// This simply modifies the lwzx instruction to match the previous one, as the registers are untouched from the previous hook
kmWrite8(0x80162BE5, 0x67);

/* File List hook */

kmCallDefAsm(0x8091FD3C) {
    // Move r25 to r21 to preserve it
    mr r21, r25

    // Check if original sprite
    cmpwi r0, SPRITE_COUNT*4;
    blt+ notCustom

    // Subtract using sub rather than subi because r0
    li r12, SPRITE_COUNT*4
    sub r0, r0, r12

    // Override table address
    lis r21, customSpriteFiles@h
    ori r21, r21, customSpriteFiles@l

    // Modified original instruction
    notCustom:
    lwzx r21, r21, r0
    blr
}

/* Profile Names hook */

kmBranchDefCpp(0x801018CC, NULL, const char*, u16 profileId, const char** array) {
    if (profileId < fProfile::PROFILE_COUNT)
        return array[profileId];
    return customProfileNames[profileId - fProfile::PROFILE_COUNT];
}