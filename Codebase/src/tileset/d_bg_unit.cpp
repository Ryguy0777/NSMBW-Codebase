// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/course/tileset/common.cpp
#include <kamek.h>
#include <game/bases/d_bg.hpp>
#include <new/bases/d_grass_bin_mng.hpp>
#include <game/bases/d_res_mng.hpp>
#include <new/bases/d_rand_tile_mng.hpp>

// Preload the data files so we can reuse them anytime in the level
dBgUnit_c *LoadBins(dBgUnit_c *unit, EGG::Heap *heap, int area, int layer) {

    // Only do this operation in the first layer
    if (layer == 0) {

        // Process each tileset
        for (int slot = 0; slot < 4; slot++) {

            // Get tileset name (this has already been set up so it is safe to call)
            const char* tileName = dBgGlobal_c::ms_pInstance->getEnvironment(area, slot);

            // If tileset is null, skip
            if (tileName[0] == '\0') {
                continue;
            }

            // Get the grass file
            GrassBin_s *grass = (GrassBin_s *)dResMng_c::m_instance->getResSilently(tileName, GRASSDATA).ptr();

            // If the file was found and is valid, create the class
            if (grass != NULL && grass->mVersion == GRASSSPECVERSION && dGrassBinMng_c::m_instance == NULL) {
                dGrassBinMng_c::build(grass, slot);
            }

            // Create the random class
            dRandTileMng_c::build(area);
        }
    }

    // Original function call
    return dBgUnit_c::create(unit, heap, area, layer);
}

// Preload the data files so we can reuse them anytime in the level - hooks
kmCall(0x80081718, LoadBins);
kmCall(0x800817C4, LoadBins);

// Hijack the function end to destroy the data files
kmBranchDefCpp(0x80154E84, NULL, int, void) {
    delete dGrassBinMng_c::m_instance;
    delete dRandTileMng_c::m_instance;

    return 1;
}