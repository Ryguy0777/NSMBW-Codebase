// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/debug/collisionRender.cpp
#include <kamek.h>
#include <new/bases/d_collision_render.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_rc.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <nw4r/g3d/g3d_camera.h>
#include <nw4r/math/math_triangular.h>
#include <game_versions_nsmbw.h>
#include <new/bases/d_debug_config.hpp>
#include <new/gx_helper.hpp>

extern void *bc_vtable;

dCollisionRender_c dCollisionRender_c::m_instance;

dCollisionRender_c::dCollisionRender_c() {
    create(nullptr, nullptr);
}

// Dummy function
void dCollisionRender_c::drawOpa() {
}

// Actual drawing
void dCollisionRender_c::drawXlu() {

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
    u32 flags = dDebugConfig_c::m_instance->mCollisionDebugFlags;

    // Draw all instances of dCc_c
    if (flags & (1 << ColliderDisplayFlags::Hitboxes)) {
        dCc_c* currCc = dCc_c::mEntryN;
        while (currCc) {

            // Make sure the actor isn't dead and that its owner exists
            if (currCc->mInfo != CC_NO_HIT) {

                u32 uptr = (u32)currCc;
                u8 r = (uptr >> 16) & 0xFF;
                u8 g = (uptr >> 8) & 0xFF;
                u8 b = uptr & 0xFF;
                u8 a = 0xFF;

                float centreX = currCc->getCenterPosX();
                float centreY = currCc->getCenterPosY();
                float edgeDistX = currCc->mCcData.mSize.x;
                float edgeDistY = currCc->mCcData.mSize.y;
                u8 collType = currCc->mShape;

                // Call DrawCircle for circles
                if (collType == CC_SHAPE_CIRCLE)
                    DrawCircle(centreX, centreY, edgeDistX, edgeDistY, 9000.0f, r, g, b, a);

                // Else call DrawQuad
                else {
                    float tlX, tlY, trX, trY, blX, blY, brX, brY;
                    bool addDiagonal = true;

                    // Use trapezoidDist for Y coordinates if collType is 2
                    // Else edge distance
                    if (collType == CC_SHAPE_DAIKEI_UD) {
                        tlY = centreY + currCc->mTrpOffsets[0];
                        trY = centreY + currCc->mTrpOffsets[2];
                        blY = centreY + currCc->mTrpOffsets[1];
                        brY = centreY + currCc->mTrpOffsets[3];
                        addDiagonal = false;
                    } else {
                        tlY = centreY + edgeDistY;
                        trY = centreY + edgeDistY;
                        blY = centreY - edgeDistY;
                        brY = centreY - edgeDistY;
                    }

                    // Use trapezoidDist for X coordinates if collType is 3
                    // Else edge distance
                    if (collType == CC_SHAPE_DAIKEI_LR) {
                        tlX = centreX + currCc->mTrpOffsets[0];
                        trX = centreX + currCc->mTrpOffsets[1];
                        blX = centreX + currCc->mTrpOffsets[2];
                        brX = centreX + currCc->mTrpOffsets[3];
                        addDiagonal = false;
                    } else {
                        tlX = centreX - edgeDistX;
                        trX = centreX + edgeDistX;
                        blX = centreX - edgeDistX;
                        brX = centreX + edgeDistX;
                    }

                    // Draw the quad
                    DrawQuad(tlX, tlY, trX, trY, blX, blY, brX, brY, 9000.0f, r, g, b, a, addDiagonal);
                }
            }

            currCc = currCc->mpNext;
        }
    }

    // Draw all instances of dBg_ctr_c
    if (flags & (1 << ColliderDisplayFlags::Colliders)) {
        dBg_ctr_c* currBgCtr = dBg_ctr_c::mEntryN;
        while (currBgCtr) {

            u32 uptr = (u32)currBgCtr;
            u8 r = (uptr >> 16) & 0xFF;
            u8 g = (uptr >> 8) & 0xFF;
            u8 b = uptr & 0xFF;
            u8 a = 0xFF;

            // If round, draw a circle
            if (currBgCtr->mIsRound)
                DrawCircle(currBgCtr->mLastPos.x, currBgCtr->mLastPos.y, currBgCtr->mRadius, currBgCtr->mRadius, 9000.0f, r, g, b, a);

            // Else draw a quad
            else {
                float tlX = currBgCtr->mCalcedPos[0].x;
                float tlY = currBgCtr->mCalcedPos[0].y;
                float trX = currBgCtr->mCalcedPos[3].x;
                float trY = currBgCtr->mCalcedPos[3].y;
                float blX = currBgCtr->mCalcedPos[1].x;
                float blY = currBgCtr->mCalcedPos[1].y;
                float brX = currBgCtr->mCalcedPos[2].x;
                float brY = currBgCtr->mCalcedPos[2].y;
                DrawQuad(tlX, tlY, trX, trY, blX, blY, brX, brY, 9000.0f, r, g, b, a, false);
            }

            currBgCtr = currBgCtr->mpNext;
        }
    }

    // Draw all instances of dBc_c
    if (flags & (1 << ColliderDisplayFlags::Sensors)) {
        dActor_c *owner = nullptr;
        while (owner = (dActor_c*)fManager_c::searchBaseByGroupType(2, owner)) {
            // Verify if dBc_c vtable is set
            u8 *vtablePtr = ((u8*)owner) + 0x1EC;
                if ((void*)*((u32*)vtablePtr) != &bc_vtable)
                    continue;

            // Get the color
            u32 uptr = (u32)owner;
            u8 r = (uptr >> 16) + 0x20;
            u8 g = (uptr >> 8) + 0x30;
            u8 b = (uptr & 0xFF) + 0x80;
            u8 a = 0xFF;

            // Grab dBc_c pointer
            dBc_c *currBc = &owner->mBc;

            // Get the actor's position
            float ownerPosX = owner->mPos.x;
            float ownerPosY = owner->mPos.y;

            // Make an array of sensors
            sBcSensorBase *sensors[4] = {currBc->mpSensorFoot, currBc->mpSensorHead, currBc->mpSensorWall, currBc->mpSensorWall};

            // Draw the sensors
            for (int i = 0; i < 4; i++) {

                // Check if the sensor exists
                sBcSensorBase *sensor = sensors[i];
                if (sensor == nullptr)
                    continue;

                // Multiplier for the adjacent sensors
                int mult = (i == 3) ? -1 : 1;
                float x1, y1, x2, y2;
                bool isLine = sensor->mFlags & 1;

                if (isLine == false) {
                    sBcSensorPoint *pointSensor = (sBcSensorPoint *)sensor;

                    x1 = ownerPosX + (float)(mult * pointSensor->mX / 4096);
                    y1 = ownerPosY + (float)(pointSensor->mY / 4096);

                    DrawPoint(x1, y1, 8005.0f, r, g, b, a);

                } else {
                    sBcSensorLine *lineSensor = (sBcSensorLine *)sensor;

                    if (i < 2) {
                        x1 = ownerPosX + (float)(lineSensor->mLineA / 4096);
                        x2 = ownerPosX + (float)(lineSensor->mLineB / 4096);
                        y1 = ownerPosY + (float)(lineSensor->mDistanceFromCenter / 4096);
                        y2 = ownerPosY + (float)(lineSensor->mDistanceFromCenter / 4096);
                    } else {
                        x1 = ownerPosX + (float)(mult * lineSensor->mDistanceFromCenter / 4096);
                        x2 = ownerPosX + (float)(mult * lineSensor->mDistanceFromCenter / 4096);
                        y1 = ownerPosY + (float)(lineSensor->mLineA / 4096);
                        y2 = ownerPosY + (float)(lineSensor->mLineB / 4096);
                    }

                    DrawLine(x1, y1, x2, y2, 8005.0f, r, g, b, a);
                }
            }
        }
    }

    // Draw all instances of dRide_ctr_c
    if (flags & (1 << ColliderDisplayFlags::RideableColliders)) {
        dRide_ctr_c* currRide = dRide_ctr_c::mEntryN;
        while (currRide) {

            u32 uptr = (u32)currRide;
            u8 r = (uptr >> 16) & 0xFF;
            u8 g = (uptr >> 8) & 0xFF;
            u8 b = uptr & 0xFF;
            u8 a = 0xFF;

            // For dRide2Point and dRideRoll, draw a simple line
            if (currRide->mType <= 2)
                DrawLine(currRide->mLeft.x, currRide->mLeft.y, currRide->mRight.x, currRide->mRight.y, 9000.0f, r, g, b, a);

            // dRideCircle is a little bit more complex
            else {
                dRideCircle_c* currCircle = (dRideCircle_c*)currRide;

                // Get centre and radius
                float centreX = currCircle->mpOwner->mPos.x + currCircle->mCenterOffset.x;
                float centreY = currCircle->mpOwner->mPos.y + currCircle->mCenterOffset.y;
                float radius = currCircle->mRadius;

                // If the circle is full, use the regular circle method
                if (currCircle->mLeftAngleOffset + currCircle->mRightAngleOffset == 0x10000)
                    DrawCircle(centreX, centreY, radius, radius, 9000.0f, r, g, b, a);

                // Else draw a partial circle
                else {
                    u16 minAngle = currCircle->mRotation - currCircle->mRightAngleOffset;
                    u16 maxAngle = minAngle + currCircle->mLeftAngleOffset;
                    DrawPartialCircle(centreX, centreY, radius, 9000.0f, minAngle, maxAngle, r, g, b, a);
                }
            }

            currRide = currRide->mpNext;
        }
    }
}