#include <kamek.h>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_s_world_map_static.hpp>

// Load new World Map resources
kmBranchDefCpp(0x80926D64, 0x80926D68, bool) {
    dResMng_c::m_instance->setRes("WorldMap", "mapModelCull", nullptr);

    return true;
}

// Delete new resources
kmBranchDefCpp(0x80927A9C, 0x80927AA0, bool) {
    dResMng_c::m_instance->deleteRes("mapModelCull");

    return true;
}