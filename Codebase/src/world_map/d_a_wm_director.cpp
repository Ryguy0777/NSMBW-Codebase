#include <kamek.h>
#include <new/world_map_config.h>

#include <game/bases/d_wm_effect_proc.hpp>
#include <new/bases/d_wm_cull_render.hpp>

// Remove "hint" messages

#ifdef REMOVE_MSG_CASTLE
kmWrite8(0x808FC98F, 0x01);
#endif

#ifdef REMOVE_MSG_HOUSE
kmWrite8(0x808FC36B, 0x01);
#endif

#ifdef REMOVE_MSG_GAMEOVER
kmWrite8(0x808FC72F, 0x01);
#endif

// Schedule the culling vizualizer
kmBranchDefCpp(0x808FB1B4, 0x808FB1CC, void, dWmEffectProc_c *effectProc) {
    if (effectProc != nullptr) {
        effectProc->entry();
    }

    dWmCullRender_c::m_instance.entry();
}