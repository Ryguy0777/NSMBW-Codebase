#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <game/bases/d_res_mng.hpp>
#include <game/cLib/c_counter.hpp>
#include <lib/MSL/math.h>

#include <new/bases/koopatlas/d_kp_map.hpp>
#include <new/bases/koopatlas/d_kp_camera.hpp>

const int dKPMap_c::sc_effectGroups[] = {1,0,4,5,7,8,9,10,11,12,13,14};
const int dKPMap_c::sc_effectPrios[] = {141,142,143,129,144,145,146,147,148,149,150,151};

// TODO: Instance is needed for pathmanager usage, remove that requirement asp
dKPMap_c *dKPMap_c::m_instance = nullptr;

dKPMap_c *dKPMap_c_classInit() {
    dKPMap_c *c = new dKPMap_c;

    dKPMap_c::m_instance = c;
    return c;
}

// Replace WM_MAP actor
kmWritePointer(0x80984710, &dKPMap_c_classInit);

dKPMap_c::dKPMap_c() {
    mDispLaunchStar = false;
}

int dKPMap_c::create() {
    mRender.mAllocator.attach(mHeap::g_gameHeaps[0], 0x20);
    mRender.create(&mRender.mAllocator, nullptr);

    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    nw4r::g3d::ResFile rf(dScKoopatlas_c::m_instance->mMapData.mBgLoader.GetBuffer());
    rf.CheckRevision();
    rf.Init();
    rf.Bind(rf);

    PSMTXTrans(mBgMtx, 0.0f, 0.0f, -500.0f);

    nw4r::g3d::ResMdl modelRes = rf.GetResMdl("KpBG");
    nw4r::g3d::ResAnmTexSrt anmRes = rf.GetResAnmTexSrt("KpBG");

    mBgModel.create(modelRes, &mAllocator, 0x32C, 1, nullptr);
    mBgSrtAnm.create(modelRes, anmRes, &mAllocator, nullptr, 1);
    mBgSrtAnm.setAnm(mBgModel, anmRes, 0, m3d::FORWARD_LOOP);
    mBgModel.setAnm(mBgSrtAnm, 0.0f);

    for (int i = 0; i < EFFECT_PROC_COUNT; i++) {
        mEffectProcs[i].create(&mAllocator, -1, sc_effectPrios[i], sc_effectGroups[i]);
    }

    bool lsRotate = false;
    switch (dScKoopatlas_c::m_instance->mCurrentMapID) {
        case 5: // Starry Sky
            mLaunchStarPos.x = 5424.0f;
            mLaunchStarPos.y = -4416.0f;
            mDispLaunchStar = true;
            break;
        /*case 11: // Sky City
            mLaunchStarPos.x = 3216.0f;
            mLaunchStarPos.y = -3120.0f;
            mDispLaunchStar = true;
            break;*/
        case 6: // Planet Koopa
            lsRotate = true;
            mLaunchStarPos.x = 2268.0f;
            mLaunchStarPos.y = -3420.0f;
            mDispLaunchStar = true;
            break;
    }

    if (mDispLaunchStar) {
        nw4r::g3d::ResFile lsRes = dResMng_c::m_instance->getRes("StarRing", "g3d/StarRing.brres");
        nw4r::g3d::ResMdl lsMdl = lsRes.GetResMdl("StarRing");
        mLaunchStarModel.create(lsMdl, &mAllocator, 0x224, 1, nullptr);

        nw4r::g3d::ResAnmChr lsAnm = lsRes.GetResAnmChr("StarRing_wait");
        mLaunchStarAnm.create(lsMdl, lsAnm, &mAllocator, nullptr);

        mLaunchStarAnm.setAnm(mLaunchStarModel, lsAnm, m3d::FORWARD_LOOP);
        mLaunchStarModel.setAnm(mLaunchStarAnm);

        PSMTXTrans(mLaunchStarMtx, mLaunchStarPos.x, mLaunchStarPos.y, 1000.0f);
        mAng angY = (lsRotate) ? 0x6200 : -0x5C00;
        mLaunchStarMtx.ZXYrotM(0x2000, angY, 0);
    }

    mAllocator.adjustFrmHeap();
    return true;
}

int dKPMap_c::doDelete() {
    return true;
}

int dKPMap_c::execute() {
    mBgSrtAnm.play();

    mBgModel.setLocalMtx(&mBgMtx);
    mBgModel.setScale(1.0f, 1.0f, 1.0f);
    mBgModel.calc(false);

    //spawnMapEffects();

    if (mDispLaunchStar) {
        mLaunchStarAnm.play();
        mLaunchStarModel.play();
        mLaunchStarModel.setLocalMtx(&mLaunchStarMtx);
        mLaunchStarModel.setScale(0.035f, 0.035f, 0.035f);
        mLaunchStarModel.calc(false);
    }
    return true;
}

int dKPMap_c::draw() {
    mRender.entry();
    mBgModel.entry();

    if (mDispLaunchStar) {
        mLaunchStarModel.entry();
    }

    for (int i = 0; i < EFFECT_PROC_COUNT; i++) {
        mEffectProcs[i].entry();
    }

    dKPMapData_c *dataCls = &dScKoopatlas_c::m_instance->mMapData;
    if (dataCls->mpPathLayer) {
        renderPathLayer(dataCls->mpPathLayer);
    }
    return true;
}

void dKPMap_c::spinLaunchStar() {
    nw4r::g3d::ResFile lsRes = dResMng_c::m_instance->getRes("StarRing", "g3d/StarRing.brres");
    nw4r::g3d::ResAnmChr lsAnm = lsRes.GetResAnmChr("StarRing_shot");
    mLaunchStarAnm.setAnm(mLaunchStarModel, lsAnm, m3d::FORWARD_LOOP);
    mLaunchStarModel.setAnm(mLaunchStarAnm);
}

// TODO: Try to find a way to dehardcode this, if possible
/*void dKPMap_c::spawnMapEffects() {
    int mapID = dScKoopatlas_c::m_instance->mCurrentMapID;

    const S16Vec efRot = {0x1800, 0, 0};

    // Fullmap
    if (mapID == 1) {
        // Mushroom Peaks torches
        static const VEC3 torchPos[6] = {
            {8402.0f, -5528.0f, 7000.0f}, // Big Tower
            {8444.0f, -5524.0f, 7000.0f}, // Tower
            {8358.0f, -5524.0f, 7000.0f}, // Tower
            {8420.0f, -5534.0f, 7000.0f}, // Tower
            {8380.0f, -5534.0f, 7000.0f}, // Tower
            {7804.0f, -5064.0f, 7000.0f}, // Castle
        };
        const VEC3 reallyBigScale = {1.6f, 1.6f, 1.6f};
        const VEC3 bigScale = {1.2f, 1.2f, 1.2f};
        const VEC3 smallScale = {0.25f, 0.25f, 0.25f};
        for (int i = 0; i < 6; i++) {
            const VEC3 *whichScale = &smallScale;
            if (i == 0)
                whichScale = &bigScale;
            else if (i == 5)
                whichScale = &reallyBigScale;
            effects[i].spawn("Wm_cs_torch", 0, &torchPos[i], &efRot, whichScale);
        }

        // Mountain Snow
        const VEC3 efPos = {6000.0f, -5250.0f, 7000.0f};
        effects[6].spawn("Wm_cs_snow_b", 0, &efPos, &efRot, 0);
    }

    // Mountain Backside
    else if (mapID == 3) {
        VEC3 efPos = {3930.0f, -2700.0f, 7000.0f};
        effects[0].spawn("Wm_cs_snow_a", 0, &efPos, &efRot, 0);
        efPos.y -= 700.0f;
        effects[1].spawn("Wm_cs_snow_a", 0, &efPos, &efRot, 0);
    }

    // Freezeflame Volcano
    else if (mapID == 4) {
        const VEC3 efPos = {2200.0f, -2000.0f, 7000.0f};
        effects[0].spawn("Wm_cs_firespark", 0, &efPos, &efRot, 0);
    }

    // Koopa Planet
    else if (mapID == 6) {
        const VEC3 efPos = {2200.0f, -2000.0f, 7000.0f};
        effects[0].spawn("Wm_cs_firespark", 0, &efPos, &efRot, 0);
    }

    // Koopa Core
    else if (mapID == 7) {
        // Main area
        const VEC3 efPos = {2500.0f, -2900.0f, 7000.0f};
        effects[0].spawn("Wm_cs_firespark", 0, &efPos, &efRot, 0);
        // Castle area
        const VEC3 efPos2 = {4500.0f, -3800.0f, 7000.0f};
        effects[1].spawn("Wm_cs_firespark", 0, &efPos2, &efRot, 0);
        // Challenge House area
        const VEC3 efPos3 = {2500.0f, -5500.0f, 7000.0f};
        effects[2].spawn("Wm_cs_firespark", 0, &efPos2, &efRot, 0);
    }
}*/



dKPMap_c::dMapRender_c::dMapRender_c() { }

void dKPMap_c::dMapRender_c::drawOpa() { }

void dKPMap_c::dMapRender_c::drawXlu() {
    drawLayers();
}

void dKPMap_c::dMapRender_c::loadTexture(GXTexObj *obj) {
    if (mpCurrentTexture == obj)
        return;

    GXLoadTexObj(obj, GX_TEXMAP0);
    mpCurrentTexture = obj;
}

void dKPMap_c::dMapRender_c::loadCamera() {
    GXLoadPosMtxImm(mRenderMtx, GX_PNMTX0);
}

void dKPMap_c::dMapRender_c::loadCamera(nw4r::math::MTX34 matrix) {
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
}

void dKPMap_c::dMapRender_c::beginRendering() {
    mpCurrentTexture = nullptr;

    nw4r::g3d::Camera cam3d(m3d::getCamera(0));
    cam3d.GetCameraMtx(&mRenderMtx);
    PSMTXTransApply(mRenderMtx, mRenderMtx, 0, 0, mBaseZ);

    GXSetCurrentMtx(GX_PNMTX0);

    dKPCamera_c *camera = dKPCamera_c::m_instance;
    mMinX = ((int)camera->mScreenLeft) / 24;
    mMinY = ((int)(-camera->mScreenTop) - 23) / 24;
    mMaxX = (((int)(camera->mScreenLeft + camera->mScreenWidth)) + 23) / 24;
    mMaxY = ((int)(-camera->mScreenTop + camera->mScreenHeight)) / 24;

    GXClearVtxDesc();

    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // Tiles
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    // Doodads
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_U8, 8);

    GXSetNumIndStages(0);
    for (int i = 0; i < 0x10; i++) {
        GXSetTevDirect((GXTevStageID)i);
    }

    GXSetNumChans(0);
    GXSetNumTexGens(1);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    GXSetNumTevStages(1);
    GXSetNumIndStages(0);

    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C1, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetZCompLoc(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);

    GXColor fogColor = {0, 0, 0, 0};
    GXSetFog(GX_FOG_NONE, fogColor, 0.0f, 0.0f, 0.0f, 0.0f);
    GXSetFogRangeAdj(GX_FALSE, 0, 0);

    GXSetCullMode(GX_CULL_NONE);

    GXSetDither(GX_TRUE);

    GXColor regColor0 = {255, 255, 255, 255};
    GXColor regColor1 = {0, 0, 0, 255};
    GXSetTevColor(GX_TEVREG0, regColor0);
    GXSetTevColor(GX_TEVREG1, regColor1);
}

void dKPMap_c::dMapRender_c::endRendering() { }

void dKPMap_c::dMapRender_c::drawLayers() {
    dScKoopatlas_c *wm = dScKoopatlas_c::m_instance;
    dKPMapData_c::MapFile_s *data = wm->mMapData.mpData;

    if (data == nullptr) {
        return;
    }

    mBaseZ = -100.0f - (2 * data->mLayerCount);

#ifdef KP_SKIP_TOP_LAYER_W1
    bool skipFirstLayer = (wm->mCurrentMapID == 0) && !(wm->mIsFirstPlay);
#endif

    beginRendering();

    for (int iLayer = data->mLayerCount - 1; iLayer >= 0; iLayer--) {
#ifdef KP_SKIP_TOP_LAYER_W1
        if (skipFirstLayer && iLayer == 0)
            continue;
#endif

        dKPLayer_s *layer = data->mpLayers[iLayer];
        mRenderMtx[2][3] += 2.0f;

        if (layer->mLayerType == dKPLayer_s::PATH) {
            // Rebase the camera matrix
            mBaseZ = 3500.0f;
            nw4r::g3d::Camera cam3d(m3d::getCamera(0));
            cam3d.GetCameraMtx(&mRenderMtx);
            PSMTXTransApply(mRenderMtx, mRenderMtx, 0, 0, mBaseZ);
        }

        // Invisible
        if (layer->mLayerAlpha == 0)
            continue;

        TileReport("Checking layer %d with type %d\n", iLayer, layer->mLayerType);

        GXColor layerClr = {255,255,255,layer->mLayerAlpha};
        GXSetTevColor(GX_TEVREG0, layerClr);

        if (layer->mLayerType == dKPLayer_s::OBJECT) {
            renderTileLayer(layer, data->mpSectors);
        } else if (layer->mLayerType == dKPLayer_s::DOODAD) {
            renderDoodadLayer(layer);
        }
    }

    endRendering();
}

void dKPMap_c::dMapRender_c::renderTileLayer(dKPLayer_s *layer, dKPLayer_s::sector_s *sectors) {
    //TileReport("Rendering layer %p\n", layer);

    // Don't render it if we don't need to
    if (mMaxX < layer->mLeft || mMinX > layer->mRight)
        return;
    if (mMaxY < layer->mTop || mMinY > layer->mBottom)
        return;

    loadCamera();
    loadTexture(layer->mpTileset);

    // Figure out -what- to render
    BoundReport("Regular render area: %d,%d to %d,%d\n", mMinX, mMinY, mMaxX, mMaxY);
    BoundReport("Layer bounds: %d,%d to %d,%d\n", layer->mLeft, layer->mTop, layer->mRight, layer->mBottom);
    int toRenderMinX = max(mMinX, layer->mLeft);
    int toRenderMinY = max(mMinY, layer->mTop);

    int toRenderMaxX = min(mMaxX, layer->mRight);
    int toRenderMaxY = min(mMaxY, layer->mBottom);

    int sectorBaseX = layer->mSectorLeft;
    int sectorBaseY = layer->mSectorTop;
    int sectorIndexStride = (layer->mSectorRight - layer->mSectorLeft + 1);

    int sectorMinX = toRenderMinX / 16;
    int sectorMinY = toRenderMinY / 16;
    int sectorMaxX = toRenderMaxX / 16;
    int sectorMaxY = toRenderMaxY / 16;
    BoundReport("To render: %d,%d to %d,%d\n", toRenderMinX, toRenderMinY, toRenderMaxX, toRenderMaxY);
    BoundReport("sectors %d,%d to %d,%d\n", sectorMinX, sectorMinY, sectorMaxX, sectorMaxY);

    for (int sectorY = sectorMinY; sectorY <= sectorMaxY; sectorY++) {
        int baseIndex = (sectorY - sectorBaseY) * sectorIndexStride;

        int iMinY = (sectorY == sectorMinY) ? (toRenderMinY & 0xF) : 0;
        int iMaxY = (sectorY == sectorMaxY) ? (toRenderMaxY & 0xF) : 15;

        int worldSectorY = sectorY << 4;

        for (int sectorX = sectorMinX; sectorX <= sectorMaxX; sectorX++) {
            u16 index = layer->mIndices[baseIndex + sectorX - sectorBaseX];
            TileReport("Sector index @ %d,%d: %d\n", sectorX, sectorY, index);
            if (index == 0xFFFF)
                continue;

            dKPLayer_s::sector_s *sector = &sectors[index];

            int iMinX = (sectorX == sectorMinX) ? (toRenderMinX & 0xF) : 0;
            int iMaxX = (sectorX == sectorMaxX) ? (toRenderMaxX & 0xF) : 15;
            TileReport("Min/Max: X: %d,%d Y: %d,%d\n", iMinX, iMaxX, iMinY, iMaxY);

            int worldSectorX = sectorX << 4;

            for (int inY = iMinY; inY <= iMaxY; inY++) {
                for (int inX = iMinX; inX <= iMaxX; inX++) {
                    u16 tileID = (*sector)[inY][inX];
                    if (tileID == 0xFFFF)
                        continue;

                    s16 worldX = (worldSectorX | inX) * 24;
                    s16 worldY = -((worldSectorY | inY) * 24);

                    TileReport("Drawing tile %d at %d,%d\n", tileID, worldX, worldY);

                    float tileX = (float)((tileID & 0x1F) * 28);
                    float tileY = (float)(((tileID & 0x1E0) >> 5) * 28);

                    float xMult = (1.0f / 896.0f);
                    float yMult = (1.0f / 448.0f);

                    float coordX1 = xMult * (tileX + 2.0f);
                    float coordX2 = xMult * (tileX + 26.0f);
                    float coordY1 = yMult * (tileY + 2.0f);
                    float coordY2 = yMult * (tileY + 26.0f);

                    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                    GXPosition2s16(worldX + 24, worldY - 24);
                    GXTexCoord2f32(coordX2, coordY2);
                    GXPosition2s16(worldX + 24, worldY);
                    GXTexCoord2f32(coordX2, coordY1);
                    GXPosition2s16(worldX, worldY);
                    GXTexCoord2f32(coordX1, coordY1);
                    GXPosition2s16(worldX, worldY - 24);
                    GXTexCoord2f32(coordX1, coordY2);
                    GXEnd();
                }
            }

            TileReport("Sector complete\n");
        }
    }

    //TileReport("Layer complete\n");
}

void dKPMap_c::dMapRender_c::renderDoodadLayer(dKPLayer_s *layer) {
    for (int i = 0; i < layer->mDoodadCount; i++) {
        // TODO: Implement doodad culling

        dKPDoodad_s *doodad = layer->mpDoodads[i];
        DoodadReport("Doodad @ %f,%f sized %f,%f with angle %f\n", doodad->mPos.x, doodad->mPos.y, doodad->mWidth, doodad->mHeight, doodad->mAngle);

        float effectiveX = doodad->mPos.x, effectiveY = doodad->mPos.y;
        float effectiveWidth = doodad->mWidth, effectiveHeight = doodad->mHeight;
        float effectiveAngle = doodad->mAngle;
        int effectiveAlpha = layer->mLayerAlpha;

        // Animate it
        if (doodad->mAnimCount > 0) {
            for (int j = 0; j < doodad->mAnimCount; j++) {
                dKPDoodad_s::Animation_s *anim = &doodad->mAnims[j];

                if (anim->mInitialDelay == 0) {
                    u32 baseTick = anim->mBaseTick;
                    if (baseTick == 0) {
                        anim->mBaseTick = baseTick = cCounter_c::m_gameFrame;
                    }

                    u32 elapsed = cCounter_c::m_gameFrame - baseTick;
                    if (anim->mIsReverse) {
                        elapsed = anim->mFrameCount - 1 - elapsed;
                    }
                    u32 elapsedAdjusted = elapsed;

                    if (elapsed >= anim->mFrameCount) {
                        if (elapsed >= (anim->mFrameCount + anim->mDelay)) {

                            // We've reached the end
                            switch (anim->mLoopType) {
                                case dKPDoodad_s::Animation_s::CONTIGUOUS:
                                    // Stop here
                                    elapsed = anim->mFrameCount - 1;
                                    break;

                                case dKPDoodad_s::Animation_s::LOOP:
                                    // Start over
                                    elapsed = 0;
                                    anim->mBaseTick = cCounter_c::m_gameFrame;
                                    break;

                                case dKPDoodad_s::Animation_s::REVERSE_LOOP:
                                    // Change direction
                                    anim->mIsReverse = !anim->mIsReverse;
                                    elapsed = (anim->mIsReverse) ? (anim->mFrameCount - 1) : 0;
                                    anim->mBaseTick = cCounter_c::m_gameFrame;
                                    break;
                            }
                            elapsedAdjusted = elapsed;
                        } else {
                            elapsedAdjusted = anim->mFrameCount;
                        }
                    }

                    // Calculate the thing
                    float progress = elapsedAdjusted / (float)anim->mFrameCount;
                    // float progress = elapsed / (float)anim->frameCount;
                    float value;

                    switch (anim->mCurveType) {
                        case dKPDoodad_s::Animation_s::LINEAR:
                            value = progress;
                            break;
                        case dKPDoodad_s::Animation_s::SIN:
                            value = (sin(((progress * M_PI * 2)) - (M_PI/2)) + 1) / 2;
                            break;
                        case dKPDoodad_s::Animation_s::COS:
                            value = (cos(((progress * M_PI * 2)) - (M_PI/2)) + 1) / 2;
                            break;
                    }

                    float delta = anim->mEnd - anim->mStart;
                    float frame;

                    if (anim->mIsReverse)
                        frame = anim->mStart + ceil(delta * value);
                    else
                        frame = anim->mStart + (delta * value);

                    float scaleYMod;

                    // Apply it
                    switch (anim->mAnimType) {
                        case dKPDoodad_s::Animation_s::X_POS:
                            effectiveX += frame;
                            break;
                        case dKPDoodad_s::Animation_s::Y_POS:
                            effectiveY += frame;
                            break;
                        case dKPDoodad_s::Animation_s::ANGLE:
                            effectiveAngle += frame;
                            break;
                        case dKPDoodad_s::Animation_s::X_SCALE:
                            effectiveWidth = (effectiveWidth * frame / 100.0);
                            break;
                        case dKPDoodad_s::Animation_s::Y_SCALE:
                            effectiveHeight = (effectiveHeight * frame / 100.0);

                            scaleYMod = doodad->mHeight - effectiveHeight;
                            effectiveY += scaleYMod;
                            break;
                        case dKPDoodad_s::Animation_s::OPACITY:
                            effectiveAlpha = (effectiveAlpha * (frame * 2.55f)) / 255;
                            break;
                    }
                } else {
                    // Doodad has an initial delay
                    anim->mInitialDelay -= 1;
                }
            }
        }

        float halfW = effectiveWidth * 0.5f, halfH = effectiveHeight * 0.5f;

        nw4r::math::MTX34 doodadMtx;
        PSMTXTransApply(mRenderMtx, doodadMtx, effectiveX + halfW, -effectiveY - halfH, 0);

        if (effectiveAngle != 0) {
            Mtx rotMtx;
            PSMTXRotRad(rotMtx, 'z', (-effectiveAngle)*0.01745329252f);

            PSMTXConcat(doodadMtx, rotMtx, doodadMtx);
        }

        GXColor doodadClr = {255,255,255,u8(effectiveAlpha)};
        GXSetTevColor(GX_TEVREG0, doodadClr);

        loadCamera(doodadMtx);
        loadTexture(doodad->mpTexObj);

        GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        GXPosition2f32(halfW, -halfH);
        GXTexCoord2u8(255, 255);
        GXPosition2f32(halfW, halfH);
        GXTexCoord2u8(255, 0);
        GXPosition2f32(-halfW, halfH);
        GXTexCoord2u8(0, 0);
        GXPosition2f32(-halfW, -halfH);
        GXTexCoord2u8(0, 255);
        GXEnd();
    }
}

void dKPMap_c::renderPathLayer(dKPLayer_s *layer) {
    for (int i = 0; i < layer->mNodeCount; i++) {
        dKPNode_s *node = layer->mpNodes[i];

        if (node->mNodeType == dKPNode_s::LEVEL) {
            OSReport("Node %d: POS:%f,%f, %02d-%02d\n", i, node->mPosX, node->mPosY, node->mLevelNumber[0], node->mLevelNumber[1]);
            // Used for the "ending scene" where the W9 path appears
            // TODO: Dehardcode this, if possible
            if (node->mLevelNumber[0] == 80) {
                continue;
            }

            mAng3_c angle(0x4000, 0x8000, 0x6000);

            PSMTXTrans(node->mpNodeMdl->mMatrix, node->mPosX, -node->mPosY + 4.0, 498.0);
            node->mpNodeMdl->mMatrix.ZXYrotM(angle.y, angle.x, angle.z);
            node->mpNodeMdl->mModel.setLocalMtx(&node->mpNodeMdl->mMatrix);
            node->mpNodeMdl->mModel.setScale(0.8f, 0.8f, 0.8f);
            node->mpNodeMdl->mModel.calc(false);

            node->mpNodeMdl->mModel.entry();
        }
    }
}
#endif