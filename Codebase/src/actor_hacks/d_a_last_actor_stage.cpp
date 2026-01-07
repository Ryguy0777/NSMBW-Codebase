#include <kamek.h>
#include <new/bases/d_collision_render.hpp>

// Schedule various debugging renderers for drawing
kmBranchDefCpp(0x80830BD8, NULL, int, void) {
    dCollisionRender_c::m_instance.entry();
    return 1;
}