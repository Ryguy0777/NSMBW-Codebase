#include <new/bases/d_actor_spawn_render.hpp>
#include <nw4r/g3d/g3d_camera.h>
#include <nw4r/math/math_triangular.h>
#include <revolution/GX.h>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_cd.hpp>
#include <game/bases/d_actor.hpp>
#include <new/new_profile.hpp>
#include <new/bases/d_debug_config.hpp>

// Drawing helper functions
void DrawSpawnRange(float tlX, float tlY, float trX, float trY, float blX, float blY, float brX, float brY, u8 r, u8 g, u8 b) {

    // Setup drawing
    GXBegin(GX_LINES, GX_VTXFMT0, 8);

    // Draw top
    GXPosition3f32(tlX, tlY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);
    GXPosition3f32(trX, trY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);

    // Draw left
    GXPosition3f32(tlX, tlY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);
    GXPosition3f32(blX, blY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);

    // Draw right
    GXPosition3f32(trX, trY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);
    GXPosition3f32(brX, brY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);

    // Draw bottom
    GXPosition3f32(blX, blY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);
    GXPosition3f32(brX, brY, 9000.0f);
    GXColor4u8(r,g,b,0xFF);

    GXEnd();
}

dActorSpawnRender_c dActorSpawnRender_c::m_instance;

dActorSpawnRender_c::dActorSpawnRender_c() {
    create(nullptr, nullptr);
}

// Dummy function
void dActorSpawnRender_c::drawOpa() {
}

extern dActorData_c l_custom_actor_data_tbl[];

// Actual drawing
void dActorSpawnRender_c::drawXlu() {

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

    // Set line width to 2 pixels and point size to 4 pixels
    GXSetLineWidth(12, 0);
    GXSetPointSize(24, 0);

    // Load debug flags from config
    u32 flags = dDebugConfig_c::m_instance->mSpawnRangeDebugFlags;
    u32 mode = dDebugConfig_c::m_instance->mSpawnRangeModeFlags;

    dScStage_c *stage = dScStage_c::m_instance;
    u8 areaNo = stage->mCurrArea;
    dCdFile_c *file = dCd_c::m_instance->getFileP(stage->mCurrFile);

    int mapActorCount = file->mMapActorCountByArea[areaNo];
    if (mapActorCount == 0) {
        return;
    }

    sMapActorData *data = file->mMapActorsByArea[areaNo];
    if (data == nullptr) {
        return;
    }

    float actorCenterX, actorCenterY;
    float edgeDistX, edgeDistY;
    float tlX, tlY, trX, trY, blX, blY, brX, brY;

    for (u32 i = 0; i != mapActorCount; i++) {
        dActorData_c *mapActorInfo;
        if (data->mMapActorID < SPRITE_COUNT) {
            mapActorInfo = dActorData_c::getMapActorInfo(data->mMapActorID);
        } else {
            mapActorInfo = &l_custom_actor_data_tbl[data->mMapActorID - SPRITE_COUNT];
        }

        if (mapActorInfo->mFlags & ACTOR_CREATE_GLOBAL) {
            // Skip drawing if actor is spawned globally
        } else if (
            mode & (1 << SpawnRangeDisplayMode::Enemy) && !(mapActorInfo->mFlags & ACTOR_CREATE_MAPOBJ) ||
            mode & (1 << SpawnRangeDisplayMode::MapObj) && mapActorInfo->mFlags & ACTOR_CREATE_MAPOBJ
        ) {
            // Main spawn range, no margins
            // We need these for margins, so calculate them even if we're not going to draw them
            actorCenterX = data->mX + mapActorInfo->mSpawnRangeOffsetX;
            actorCenterY = -(data->mY - mapActorInfo->mSpawnRangeOffsetY);
            edgeDistX = mapActorInfo->mSpawnRangeHalfWidth;
            edgeDistY = mapActorInfo->mSpawnRangeHalfHeight;

            // Not entirely sure why it needs these extra values added/subtracted...
            tlY = actorCenterY + edgeDistY - 16;
            trY = actorCenterY + edgeDistY - 16;
            blY = actorCenterY - edgeDistY - 16;
            brY = actorCenterY - edgeDistY - 16;

            tlX = actorCenterX - edgeDistX + 8;
            trX = actorCenterX + edgeDistX + 8;
            blX = actorCenterX - edgeDistX + 8;
            brX = actorCenterX + edgeDistX + 8;
            
            if (flags & (1 << SpawnRangeDisplayFlags::NoMargins)) {
                DrawSpawnRange(tlX, tlY, trX, trY, blX, blY, brX, brY, 0xFF, 0, 0);
            }

            // Spawn range + margins
            if (flags & (1 << SpawnRangeDisplayFlags::Margins)) {
                // Don't draw if there's no margins
                if (
                    mapActorInfo->mSpawnMarginTop != 0 || mapActorInfo->mSpawnMarginBottom != 0 ||
                    mapActorInfo->mSpawnMarginLeft != 0 || mapActorInfo->mSpawnMarginRight != 0
                ) {
                    tlY = tlY + mapActorInfo->mSpawnMarginTop;
                    trY = trY + mapActorInfo->mSpawnMarginTop;
                    blY = blY - mapActorInfo->mSpawnMarginBottom;
                    brY = brY - mapActorInfo->mSpawnMarginBottom;

                    tlX = tlX - mapActorInfo->mSpawnMarginLeft;
                    trX = trX + mapActorInfo->mSpawnMarginRight;
                    blX = blX - mapActorInfo->mSpawnMarginLeft;
                    brX = brX + mapActorInfo->mSpawnMarginRight;

                    DrawSpawnRange(tlX, tlY, trX, trY, blX, blY, brX, brY, 0, 0xFF, 0);
                }
            }
        }

        // Cull distance
        if (flags & (1 << SpawnRangeDisplayFlags::VisibleArea)) {
            dActor_c *owner = nullptr;
            while (owner = (dActor_c*)fManager_c::searchBaseByGroupType(2, owner)) {
                actorCenterX = owner->mPos.x + owner->mVisibleAreaOffset.x;
                actorCenterY = owner->mPos.y + owner->mVisibleAreaOffset.y;
                edgeDistX = owner->mVisibleAreaSize.x/2;
                edgeDistY = owner->mVisibleAreaSize.y/2;

                tlY = actorCenterY + edgeDistY;
                trY = actorCenterY + edgeDistY;
                blY = actorCenterY - edgeDistY;
                brY = actorCenterY - edgeDistY;

                tlX = actorCenterX - edgeDistX;
                trX = actorCenterX + edgeDistX;
                blX = actorCenterX - edgeDistX;
                brX = actorCenterX + edgeDistX;

                DrawSpawnRange(tlX, tlY, trX, trY, blX, blY, brX, brY, 0, 0, 0xFF);
            }
        }

        data++;
    }
}