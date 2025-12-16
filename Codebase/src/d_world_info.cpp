#include <new/game_config.h>
#include <new/bases/d_world_info.hpp>
#include <game/bases/d_dvd.hpp>

dDvd::loader_c s_worldInfoLoader;
bool s_worldInfoLoaded = false;

dWorldInfo_c dWorldInfo_c::m_instance;

bool dWorldInfo_c::loadWorldInfo() {
    if (s_worldInfoLoaded)
        return true;

    void *data = s_worldInfoLoader.request("/WorldInfo.bin", 0, nullptr);
    if (data) {
        dWorldInfo_c::m_instance.mpData = (header_s*)data;
        s_worldInfoLoaded = true;
        return true;
    }

    return false;
}