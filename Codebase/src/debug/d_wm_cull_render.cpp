#include <kamek.h>
#include <new/bases/d_wm_cull_render.hpp>

#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_wm_actor.hpp>
#include <game/bases/d_game_com.hpp>
#include <new/bases/d_debug_config.hpp>
#include <new/gx_helper.hpp>

dWmCullRender_c dWmCullRender_c::m_instance;

dWmCullRender_c::dWmCullRender_c() {
    create(nullptr, nullptr);
}

// Dummy
void dWmCullRender_c::drawOpa() {

}

void dWmCullRender_c::drawXlu() {
    // Setup copied from 0x801717C0
    // Set current vertex descriptor to enable position and color0, both provided directly
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    // Position has 3 elements (x,y,z), each of type f32
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

    // Color 0 has 4 components (r, g, b, a), each of type u8
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // Disable culling
    GXSetCullMode(GX_CULL_NONE);

    // Enable color channel 0 and set vertex color as source
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, false, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(0);

    // Enable one TEV stage
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Set blend mode
    GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_NOOP);

    // Load camera matrix
    nw4r::math::MTX34 mtx;
    m3d::getCurrentCamera().GetCameraMtx(&mtx);
    GXLoadPosMtxImm(mtx, 0);

    // Set line width to 3 pixels and point size to 6 pixels
    GXSetLineWidth(18, 0);
    GXSetPointSize(36, 0);

    // Load debug flags from config
    u32 flags = dDebugConfig_c::m_instance->mMapCullingDebugFlags;

    // Draw all dWmActor_c culling spheres
    if (flags & (1 << WmCullDisplayFlags::MapActors)) {

        for (int profName = fProfile::WM_IBARA; profName < fProfile::WORLD_SELECT; profName++) {
            dWmActor_c *pActor = nullptr;

            while (pActor = (dWmActor_c*)fManager_c::searchBaseByProfName(profName, pActor), pActor != nullptr) {
                mSphere_c *sphere = pActor->getCullSphere();
                // No need to vizualize spheres that won't ever be culled
                if (sphere->mRadius != 0.0f) {

                    // Make random color
                    u32 uptr = (u32)pActor;
                    u8 r = (uptr >> 16) & 0xFF;
                    u8 g = (uptr >> 8) & 0xFF;
                    u8 b = uptr & 0xFF;
                    u8 a = 0xFF;

                    float centerX = sphere->mCenter.x;
                    float centerY = sphere->mCenter.y;
                    float radius = sphere->mRadius;

                    DrawCircle(centerX, centerY, radius, radius, sphere->mCenter.z, r, g, b, a);
                }
            }
        }
    }

    // Draw all culling spheres for the map model
    if (flags & (1 << WmCullDisplayFlags::MapModel)) {
        daWmMap_c *pMap = daWmMap_c::m_instance;

        for (int i = 0; i < 30; i++) {
            // No node to cull here
            if (pMap->mCullNodeIdx[i] < 0) {
                continue;
            }

            mVec3_c nodePos;
            pMap->GetNodePos(pMap->mCullNodeIdx[i], nodePos);
            float radius = pMap->mpCullData->mNodes[i].mRadius;

            // Make random color
            u32 uptr = (u32)pMap;
            u8 r = (uptr >> 16) & 0xFF;
            u8 g = (uptr >> 8) & 0xFF;
            u8 b = uptr & 0xFF;
            u8 a = 0xFF;

            DrawCircle(nodePos.x, nodePos.y, radius, radius, nodePos.z, r, g, b, a);
        }
    }
}