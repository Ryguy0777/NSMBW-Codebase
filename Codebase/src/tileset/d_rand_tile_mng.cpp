// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/course/tileset/rand.cpp
#include <kamek.h>
#include <game/bases/d_bg.hpp>
#include <new/bases/d_rand_tile_mng.hpp>
#include <game/bases/d_res_mng.hpp>

// Extern for ASM call
extern "C" {
bool DoRandTile(dBgUnit_c *unit, dBGRender *render);
}

// Static instance
dRandTileMng_c *dRandTileMng_c::m_instance;

// Build function
dRandTileMng_c *dRandTileMng_c::build(int area) {

    // If an instance is already initialized, return it
    if (dRandTileMng_c::m_instance != NULL) {
        return dRandTileMng_c::m_instance;
    }

    // Create class and return it
    return new dRandTileMng_c(area);
}

// Constructor
dRandTileMng_c::dRandTileMng_c(int area) {

    // Process every tileset
    for (int slot = 0; slot < 4; slot++) {

        // Get tileset name
        char *tilesetName = dBgGlobal_c::ms_pInstance->getEnvironment(area, slot);

        // If tileset name is valid, get data
        RandTileBin_s *data = NULL;
        if (tilesetName[0] != '\0') {
            data = (RandTileBin_s *)dResMng_c::m_instance->getResSilently(tilesetName, RANDDATA).ptr();
        }

        // Set it
        mRandData[slot] = data;
    }

    // Set static instance
    dRandTileMng_c::m_instance = this;
}

// Destructor
dRandTileMng_c::~dRandTileMng_c() {

    // Reset static instance
    dRandTileMng_c::m_instance = NULL;
}

// Get entry from data given a tile
RandTileBinEntry_s *GetTileFromData(RandTileBin_s *data, u8 tileNum) {

    // Process every entry
    for (int i = 0; i < data->mNumEntries; i++) {
        RandTileBinEntry_s *currEntry = &data->mEntries[i];

        // If tile matches, return the entry
        if (currEntry->mTileNum == tileNum)
            return currEntry;
    }

    // No entry found, return none
    return NULL;
}

// Main random function
bool DoRandTile(dBgUnit_c* unit, dBGRender* render) {

    // Get tile and slot
    u16 tileNum = render->mCurrTile & 0x3FF;
    int slot = tileNum >> 8;

    // Get random data for slot
    RandTileBin_s *data = dRandTileMng_c::m_instance->mRandData[slot];

    // If no data is found, fall back to Nintendo's code
    if (data == NULL || data->mVersion != RANDSPECVERSION) {
        return false;
    }

    // Find the entry for this tile
    RandTileBinEntry_s *entry = GetTileFromData(data, tileNum & 0xFF);

    // If no tile is found, still skip Nintendo's code and render original tile
    if (entry == NULL) {
        return true;
    }

    // Initialize pointer
    u16* tilePtr = NULL;

    // Handle special tiles first
    switch(entry->mSpecialType) {
        case RandTileBinEntry_s::SPECIAL_RAND_VDOUBLEBOTTOM:
            // Get pointer to top tile
            tilePtr = unit->GetBuffPos(render->mCurrX*16, (render->mCurrY-1) * 16, NULL, false);

            // If pointer is not null, set the new tile
            if (tilePtr != NULL)
                render->mCurrTile = (*tilePtr + 16) & 0xFF;
            return true;

        case RandTileBinEntry_s::SPECIAL_RAND_HDOUBLERIGHT:
            // Get pointer to left tile
            tilePtr = unit->GetBuffPos((render->mCurrX-1) * 16, render->mCurrY*16, NULL, false);

            // If pointer is not null, set the new tile
            if (tilePtr != NULL)
                render->mCurrTile = (*tilePtr + 1) & 0xFF;
            return true;
        default:
            break;
    }

    // Get a pointer to the random choices list
    u16* tileArray = (u16*)((u32)data + entry->mTileArrayOffs);

    // Handle regular tiles with the game's original functions (setting slot to 0 in order to insert it manually later)
    switch(entry->mNormalType) {
        case RandTileBinEntry_s::NORMAL_RAND_HORI:
            unit->randomiseHorz(render, tileArray, entry->mArrayLen, 0);
            break;
        case RandTileBinEntry_s::NORMAL_RAND_VERT:
            unit->randomiseVert(render, tileArray, entry->mArrayLen, 0);
            break;
        case RandTileBinEntry_s::NORMAL_RAND_BOTH:
            unit->randomiseHorzVert(render, tileArray, entry->mArrayLen, 0);
            break;
        default:
            return true;
    }

    // Reload tile and add slot to it
    render->mCurrTile |= slot * 0x100;
    return true;
}

kmCallDefAsm(0x80086B60) {
    // Let me free
    nofralloc

    // Push stack manually
    stwu r1, -0x10(r1)
    mflr r0
    stw r0, 0x14(r1)

    // Modified original instruction
    mr r3, r29
    mr r4, r30
    bl DoRandTile

    // Restore r0 and reload LR
    lhz r0, 0xC10(r30)
    lwz r12, 0x14(r1)

    // If return is false, fall back to original call
    cmpwi r3, 0
    beq+ end

    // Else skip it
    addi r12, r12, 0x108

    // Restore r3
    end:
    mr r3, r29

    // Pop stack manually and return
    mtlr r12
    addi r1, r1, 0x10
    blr
}
