// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/course/tileset/anim.h
// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/course/tileset/anim.cpp
#include <kamek.h>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_res_mng.hpp>

#define ANIMDATA "BG_anim/anim_overrides.bin"
#define ANIMTILE "BG_anim/anim_%02x.bin"
#define ANIMSPECVERSION 1

struct AnimsBinEntry_s {
    u16 mDelayOffs;  // Pointer to null-terminated u8 array; offset is relative to start of file
    u8 mTileNum;     // Relative tileset number
    bool mReverse;   // Reverse animation if true
};

struct AnimsBin_s {
    u8 mVersion;
    u8 mNumEntries;
    AnimsBinEntry_s mEntries[];
    // Delay arrays follow
};

// Process animation data
void ProcessAnimsBin(dBgTexMng_c *mng, AnimsBin_s *animData, int slot, char* tileName) {
    char buffer[32];

    for (int i = 0; i < animData->mNumEntries; i++) {
        // Get relevant anim entry and framesheet
        AnimsBinEntry_s *currAnim = &animData->mEntries[i];
        u8* currAnimDelays = (u8*)((u32)animData + currAnim->mDelayOffs);

        // Print name to buffer
        snprintf(buffer, sizeof(buffer), ANIMTILE, currAnim->mTileNum);

        // Setup the animated tile (function takes care of everything)
        mng->setupAnimTile(slot, 0x100 * slot | currAnim->mTileNum, buffer, currAnimDelays, currAnim->mReverse);
    }
}

// Expand tileset allocation (original value = 0x400; is this even necessary?)
kmWrite16(0x80087546, 0x1000);

// Actual animtiles code
kmCallDefCpp(0x80087840, void, dBgTexMng_c* mng, int slot, u16 tileNum, char* name, u8* pFrameTimes, bool reverse) {

    // Original call that we hijacked
    mng->setupAnimTile(slot, tileNum, name, pFrameTimes, reverse);

    // Get current area
    int area = dScStage_c::m_instance->mCurrArea;

    // Process every slot
    for (slot = 0; slot < 4; slot++) {
        // Get the tileset name
        char* tileName = dBgGlobal_c::ms_pInstance->getEnvironment(area, slot);

        // If tileset isn't set, skip
        if (tileName[0] == '\0') {
            continue;
        }

        // Get anims.bin from relevant file
        AnimsBin_s *animData = (AnimsBin_s *)dResMng_c::m_instance->getRes(tileName, ANIMDATA).ptr();

        // If file was not found or the version mismatches, skip
        if (animData == NULL || animData->mVersion != ANIMSPECVERSION) {
            continue;
        }

        // Process anims file
        ProcessAnimsBin(mng, animData, slot, tileName);
    }
}

