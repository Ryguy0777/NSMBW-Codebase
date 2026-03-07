// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/course/tileset/profoverride.h
// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/course/tileset/profoverride.cpp
#include <kamek.h>
#include <game/bases/d_bg_actor_mng.hpp>
#include <game/bases/d_a_unit_rail.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_s_stage.hpp>

#define PROFDATA "BG_prof/prof_overrides.bin"
#define PROFSPECVERSION 1

struct ProfsBinEntry_s {
    u8 mTileNum;         // Relative tile number
    // 1 byte padding
    u16 mProfileName;
    u8 mRailColorIndex;
    u8 mRailTexSrtIndex;
    // 2 bytes padding
    mVec3_c mOffset;
    mVec2_c mScale;
    u32 mParam;
};

struct ProfsBin_s {
    u8 mVersion;
    u8 mProfileIDVersion;    // UNUSED
    u8 mNumEntries;          // Number of profile overrides
    u8 mNumColors;           // Number of rail vertex color entries
    u8 mNumScales;           // Number of rail material scale entries
    // 3 bytes padding
    ProfsBinEntry_s mEntries[];
    // 8-bit rgba color table follows
    // mVec2_c scale table follows
};

// Externs for ASM call
extern "C" {
void DoObjOverride(dBgActorManager_c *mng, char *tileNames);
void DestroyOverrides();
GXColor GetRailColor(daUnitRail_c *rail, GXColor originalColor);
mVec2_c *GetRailTexScale(mVec2_c *originalScales, daUnitRail_c *rail);
}

// External data
extern dBgActorManager_c::BgObjName_t *l_rail_list[5];
extern GXColor RailColors[5];
extern mVec2_c RailScales[5];

void ParseObjOverride(ProfsBin_s *data, u32 entryCount, dBgActorManager_c::BgObjName_t *buffer, int slot) {

    // Parse entries
    for (int i = 0; i < entryCount; i++) {

        // Get current entry
        ProfsBinEntry_s *currSrc = &data->mEntries[i];
        dBgActorManager_c::BgObjName_t *currDest = &buffer[i];

        // Copy the tile number
        currDest->mTileNum = (0x100 * slot) | currSrc->mTileNum;

        // Copy the profile number based on version
        u16 profileId = currSrc->mProfileName;
        #if GAME_REVISION >= GAME_REVISION_C
        if (profileId > fProfile::LOGO_SCREEN) {
            profileId += 2;
        }
        if (profileId > fProfile::MULTI_COURSE_SELECT) {
            profileId += 2;
        }
        #elif GAME_REVISION >= GAME_REVISION_K
        if (profileId > fProfile::MULTI_COURSE_SELECT) {
            profileId += 2;
        }
        #endif
        currDest->mProfileId = profileId;

        // Copy the rest using memcpy
        memcpy(&currDest->mOffset.x, &currSrc->mOffset.x, 0x18);

        // Special code for rail actor
        if (profileId == fProfile::UNIT_RAIL) {
            // Copy tile number in the settings to allow later identification
            currDest->mParam |= currDest->mTileNum << 16;

            // Store color indexes
            currDest->m_06 = currSrc->mRailColorIndex;
            currDest->m_07 = currSrc->mRailTexSrtIndex;
        }
    }
}

void DoObjOverride(dBgActorManager_c *mng, char *tileNames) {

    // Store pointers to the overrides and their lengths on the stack
    ProfsBin_s *files[4] = {NULL, NULL, NULL, NULL};
    int lengths[4] = {0, 0, 0, 0};

    // Set up type to 0 and count to 1 (to account for dummy final entry)
    int type = 0;
    u32 count = 1;

    // Initial parsing
    for (int i = 0; i < 4; i++) {

        // Get tileset name
        const char *currTileName = &tileNames[32*i];

        // If not set, go to next tileset
        if (currTileName[0] == '\0')
            continue;

        // Get override file
        ProfsBin_s *currFile = (ProfsBin_s*)dResMng_c::m_instance->getResSilently(currTileName, PROFDATA).ptr();

        // Check if file was found and that the version matches
        if (currFile != NULL && currFile->mVersion == PROFSPECVERSION) {

            // Store pointer and entry count
            files[i] = currFile;
            lengths[i] = currFile->mNumEntries;

        // File not found or invalid, replicate donut lifts
        } else if (i == 0)
            lengths[i] = 1;

        // File not found or invalid, replicate rails
        else if (i == 3) {
            if (!strcmp(currTileName, "Pa3_rail")) {
                type = 1;
                lengths[i] = 26;
            } else if (!strcmp(currTileName, "Pa3_rail_white")) {
                type = 2;
                lengths[i] = 26;
            } else if (!strcmp(currTileName, "Pa3_daishizen")) {
                type = 3;
                lengths[i] = 13;
            } else if (!strcmp(currTileName, "Pa3_MG_house_ami_rail")) {
                type = 4;
                lengths[i] = 19;
            }
        }

        // Increase count for later
        count += lengths[i];
    }

    // Set type for rail colors
    mng->mType = type;

    // Allocate buffer
    dBgActorManager_c::BgObjName_t *buffer = new dBgActorManager_c::BgObjName_t[count];

    // Counter to keep track of parsed entries
    int parsedCount = 0;

    // Parse the entries
    for (int i = 0; i < 4; i++) {

        // Failsafe for empty files
        if (lengths[i]) {

            // If file was found, let the parser deal with it
            if (files[i] != NULL) {
                ParseObjOverride(files[i], lengths[i], &buffer[parsedCount], i);

            // Else if slot is Pa0, copy the donut lifts override
            } else if (i == 0)
                memcpy(&buffer[parsedCount], &l_rail_list[0][0], sizeof(dBgActorManager_c::BgObjName_t));

            // Else if slot is Pa3 and type is set, copy the corresponding rail overrides
            else if (i == 3 && type != 0)
                memcpy(&buffer[parsedCount], &l_rail_list[type][1], lengths[i] * sizeof(dBgActorManager_c::BgObjName_t));
            }

        // Increase counter
        parsedCount += lengths[i];

    }

    // Set dummy final entry
    buffer[parsedCount].mProfileId = fProfile::DUMMY_ACTOR;

    // Store array to static address
    l_pRailList = buffer;
}

void DestroyOverrides() {
    // Delete allocated overrides
    delete l_pRailList;
}

// Helper function
dBgActorManager_c::BgObjName_t *GetBgObjFromRail(daUnitRail_c *rail) {

    // Grab tilenum from settings
    u16 tileNum = rail->mParam >> 16;

    // Get bgActorObj entry by tileNum
    dBgActorManager_c::BgObjName_t *currEntry = &l_pRailList[0];
    while(currEntry->mProfileId != fProfile::DUMMY_ACTOR) {
        if (currEntry->mProfileId == fProfile::UNIT_RAIL && currEntry->mParam >> 16 == tileNum)
            break;
        currEntry++;
    }

    return currEntry;
}

GXColor GetRailColor(daUnitRail_c *rail, GXColor originalColor) {

    // Get bgObj from rail
    dBgActorManager_c::BgObjName_t *bgObj = GetBgObjFromRail(rail);

    // Check rail settings
    u8 colorIndex = (bgObj->m_06);

    if (colorIndex == 0) {
        return originalColor;
    }

    else if (colorIndex < 5) {
        return RailColors[colorIndex];
    }

    // Get tileset slot
    u8 slot = rail->mParam >> 24;

    // Get override file (failsaves not needed)
    char *tilesetName = dBgGlobal_c::ms_pInstance->getEnvironment(dScStage_c::m_instance->mCurrArea, slot);
    ProfsBin_s *bin = (ProfsBin_s *)dResMng_c::m_instance->getResSilently(tilesetName, PROFDATA).ptr();

    // Go to color table
    GXColor *table = (GXColor*)((u32)&bin->mEntries + (bin->mNumEntries * sizeof(ProfsBinEntry_s)));

    // Get the color according to the index given
    return table[colorIndex - 5];

}

mVec2_c *GetRailTexScale(mVec2_c *originalScales, daUnitRail_c *rail) {

    // Get bgObj from rail
    dBgActorManager_c::BgObjName_t *bgObj = GetBgObjFromRail(rail);

    // Check rail settings
    u8 texSrtIndex = (bgObj->m_07);

    if (texSrtIndex == 0) {
        return originalScales;
    }

    else if (texSrtIndex < 5) {
        return &RailScales[texSrtIndex];
    }

    // Get tileset slot
    u8 slot = rail->mParam >> 24;

    // Get override file (failsaves not needed)
    char *tilesetName = dBgGlobal_c::ms_pInstance->getEnvironment(dScStage_c::m_instance->mCurrArea, slot);
    ProfsBin_s *bin = (ProfsBin_s *)dResMng_c::m_instance->getResSilently(tilesetName, PROFDATA).ptr();

    // Go to scale table
    mVec2_c* table = (mVec2_c*)((u32)&bin->mEntries + (bin->mNumEntries * sizeof(ProfsBinEntry_s)) + bin->mNumColors * sizeof(GXColor));

    // Get the scale according to the index given
    return &table[texSrtIndex - 5];
}

// Force rails to be loaded by default
kmWrite32(0x8091FCC4, 0x48000058);

kmBranchDefAsm(0x8007E30C, 0x8007E3AC) {

    // No stack saving necessary here
    nofralloc

    // If there are no zones, bail
    cmpwi r3, 0
    beq end

    // Setup call + modified original instruction
    lwz r4, 0x8(r3)
    mr r3, r29
    bl DoObjOverride

    // Return later to skip all the Nintendo fluff
    end:
    blr
}

kmBranchDefAsm(0x808B2A40, 0x808B2A44) {
    // Let me free
    nofralloc

    // Call C++ function
    addi r4, r6, 4
    bl GetRailColor

    // Move color to r0
    mr r0, r3

    // Restore registers and return
    mr r3, r29
    li r4, 1
    addi r5, r1, 0xC
    blr
}

kmCallDefAsm(0x808B2AE0) {
    // Let me free
    nofralloc

    // Original instruction
    add r3, r3, r0

    // Push stack manually
    stwu r1, -0x10(r1)
    mflr r0
    stw r0, 0x14(r1)

    // Save r5
    stw r5, 0xC(r1)

    // Call CPP function
    addi r3, r3, 0x2C
    mr r4, r29
    bl GetRailTexScale

    // Subtract from offset to fit the original instructions
    subi r3, r3, 0x2C

    // Restore r5
    lwz r5, 0xC(r1)

    // Pop stack manually and return
    lwz r0, 0x14(r1)
    mtlr r0
    addi r1, r1, 0x10
    blr
}
