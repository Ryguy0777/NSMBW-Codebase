// source: https://github.com/CLF78/NSMASR-v2/blob/master/game/debug/collisionRender.cpp
#include <kamek.h>
#include <new/bases/d_collision_render.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_ride_circle.hpp>
#include <nw4r/g3d/g3d_camera.h>
#include <nw4r/math/math_triangular.h>
#include <revolution/GX.h>
#include <game_versions_nsmbw.h>

static const bool renderCollision = false;
static const bool renderTileColliders = false;

#ifdef IS_GAME_VERSION_DYNAMIC
#error Dynamic compilation is unsupported for this patch.
#endif

#if defined(IS_GAME_VERSION_P1_COMPATIBLE)
#define BC_VTABLE 0x8030f6d0
#elif defined(IS_GAME_VERSION_E1_COMPATIBLE)
#define BC_VTABLE 0x8030f3d0
#elif defined(IS_GAME_VERSION_J1_COMPATIBLE)
#define BC_VTABLE 0x8030f1f0
#elif defined(IS_GAME_VERSION_K_COMPATIBLE)
#define BC_VTABLE 0x8031bc10
#elif defined(IS_GAME_VERSION_W_COMPATIBLE)
#define BC_VTABLE 0x80319fd0
#elif defined(IS_GAME_VERSION_C_COMPATIBLE)
#define BC_VTABLE 0x80313f10
#else
#define BC_VTABLE 0
#endif

// Drawing helper functions
void DrawPoint(float x, float y, float z, u8 r, u8 g, u8 b, u8 a) {
    GXBegin(GX_POINTS, GX_VTXFMT0, 1);

    GXPosition3f32(x, y, z);
    GXColor4u8(r,g,b,a);

    GXEnd();
}

void DrawLine(float x1, float y1, float x2, float y2, float z, u8 r, u8 g, u8 b, u8 a) {
    GXBegin(GX_LINES, GX_VTXFMT0, 2);

    GXPosition3f32(x1, y1, z);
    GXColor4u8(r,g,b,a);
    GXPosition3f32(x2, y2, z);
    GXColor4u8(r,g,b,a);

    GXEnd();
}

void DrawQuad(float tlX, float tlY, float trX, float trY, float blX, float blY, float brX, float brY, float z, u8 r, u8 g, u8 b, u8 a, bool addDiagonal) {

    // Setup drawing
    GXBegin(GX_LINES, GX_VTXFMT0, 8 + addDiagonal * 2);

    // Draw top
    GXPosition3f32(tlX, tlY, z);
    GXColor4u8(r,g,b,a);
    GXPosition3f32(trX, trY, z);
    GXColor4u8(r,g,b,a);

    // Draw left
    GXPosition3f32(tlX, tlY, z);
    GXColor4u8(r,g,b,a);
    GXPosition3f32(blX, blY, z);
    GXColor4u8(r,g,b,a);

    // Draw right
    GXPosition3f32(trX, trY, z);
    GXColor4u8(r,g,b,a);
    GXPosition3f32(brX, brY, z);
    GXColor4u8(r,g,b,a);

    // Draw bottom
    GXPosition3f32(blX, blY, z);
    GXColor4u8(r,g,b,a);
    GXPosition3f32(brX, brY, z);
    GXColor4u8(r,g,b,a);

    // Draw diagonal if enabled
    if (addDiagonal) {
        GXPosition3f32(trX, trY, z);
        GXColor4u8(r,g,b,a);
        GXPosition3f32(blX, blY, z);
        GXColor4u8(r,g,b,a);
    }

    GXEnd();
}

// Credits to Ismy for helping me with this one
void DrawCircle(float centreX, float centreY, float radiusX, float radiusY, float z, u8 r, u8 g, u8 b, u8 a) {

    // Define a few variables
    const int numVert = 64;
    const float step = 256.0f/numVert;
    float sin, cos, xDist, yDist;

    // Initialize the prev variables
    float prevXDist = radiusX;
    float prevYDist = 0.0f;

    // Begin drawing
    GXBegin(GX_LINES, GX_VTXFMT0, numVert * 2);

    // Draw each line
    for (int i = 1; i <= numVert / 4; i++) {

        // Calculate sin and cos for the current angle
        nw4r::math::SinCosFIdx(&sin, &cos, step * i);

        // Calculate the distances from the center
        xDist = radiusX * cos;
        yDist = radiusY * sin;

        // Draw on the first quadrant
        GXPosition3f32(centreX + prevXDist, centreY + prevYDist, z);
        GXColor4u8(r,g,b,a);
        GXPosition3f32(centreX + xDist, centreY + yDist, z);
        GXColor4u8(r,g,b,a);

        // Draw on the second quadrant
        GXPosition3f32(centreX - prevYDist, centreY + prevXDist, z);
        GXColor4u8(r,g,b,a);
        GXPosition3f32(centreX - yDist, centreY + xDist, z);
        GXColor4u8(r,g,b,a);

        // Draw on the third quadrant
        GXPosition3f32(centreX - prevXDist, centreY - prevYDist, z);
        GXColor4u8(r,g,b,a);
        GXPosition3f32(centreX - xDist, centreY - yDist, z);
        GXColor4u8(r,g,b,a);

        // Draw on the fourth quadrant
        GXPosition3f32(centreX + prevYDist, centreY - prevXDist, z);
        GXColor4u8(r,g,b,a);
        GXPosition3f32(centreX + yDist, centreY - xDist, z);
        GXColor4u8(r,g,b,a);

        // Override the "previous" values
        prevXDist = xDist;
        prevYDist = yDist;
    }

    // End drawing
    GXEnd();
}

void DrawPartialCircle(float centreX, float centreY, float radius, float z, u16 minAngle, u16 maxAngle, u8 r, u8 g, u8 b, u8 a) {

    // Initialize variables
    const int maxVerts = 64;
    const float step = 256.0f/maxVerts;
    float sin, cos, xDist, yDist;

    // Calculate the number of required vertices and the starting angle
    u16 totalAngle = (maxAngle - minAngle) & 0xFFFF;
    int numVert = (totalAngle * maxVerts / 0x10000);
    float startAngle = minAngle / 256.0f;

    // Set up the prev variables
    nw4r::math::SinCosFIdx(&sin, &cos, startAngle);
    float prevXDist = radius * cos;
    float prevYDist = radius * sin;

    // Begin drawing
    GXBegin(GX_LINES, GX_VTXFMT0, numVert * 2);

    // Draw each line
    for (int i = 1; i <= numVert; i++) {

        // Calculate sin and cos for the current angle
        nw4r::math::SinCosFIdx(&sin, &cos, startAngle + step * i);

        // Calculate the distances from the center
        xDist = radius * cos;
        yDist = radius * sin;

        // Draw the line
        GXPosition3f32(centreX + prevXDist, centreY + prevYDist, z);
        GXColor4u8(r,g,b,a);
        GXPosition3f32(centreX + xDist, centreY + yDist, z);
        GXColor4u8(r,g,b,a);

        // Override the "previous" values
        prevXDist = xDist;
        prevYDist = yDist;
    }
}

static dCollisionRender_c m_instance;

dCollisionRender_c::dCollisionRender_c() {
    this->create(NULL, NULL);
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

    // Draw all instances of dCc_c
    if (renderCollision) {
        dCc_c* currCc = dCc_c::mEntryN;
        while (currCc) {

            // Make sure the actor isn't dead and that its owner exists
            if (currCc->mFlag != dCc_c::CC_DISABLE) {

                u32 uptr = (u32)currCc;
                u8 r = (uptr >> 16) & 0xFF;
                u8 g = (uptr >> 8) & 0xFF;
                u8 b = uptr & 0xFF;
                u8 a = 0xFF;

                float centreX = currCc->getCenterPosX();
                float centreY = currCc->getCenterPosY();
                float edgeDistX = currCc->mCcData.mWidth;
                float edgeDistY = currCc->mCcData.mHeight;
                u8 collType = currCc->mShape;

                // Call DrawCircle for circles
                if (collType == dCc_c::CC_SHAPE_CIRCLE)
                    DrawCircle(centreX, centreY, edgeDistX, edgeDistY, 9000.0f, r, g, b, a);

                // Else call DrawQuad
                else {
                    float tlX, tlY, trX, trY, blX, blY, brX, brY;
                    bool addDiagonal = true;

                    // Use trapezoidDist for Y coordinates if collType is 2
                    // Else edge distance
                    if (collType == dCc_c::CC_SHAPE_DAIKEI_UD) {
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
                    if (collType == dCc_c::CC_SHAPE_DAIKEI_LR) {
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
    if (renderCollision) {
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
    if (renderTileColliders) {
        dActor_c *owner = NULL;
        while (owner = (dActor_c*)fManager_c::searchBaseByGroupType(2, owner)) {
            // verify if dBc_c vtable is set
            u8 *vtablePtr = ((u8*)owner) + 0x1EC;
                if (*((u32*)vtablePtr) != BC_VTABLE)
                    continue;

            // Get the color
            u32 uptr = (u32)owner;
            u8 r = (uptr >> 16) + 0x20;
            u8 g = (uptr >> 8) + 0x30;
            u8 b = (uptr & 0xFF) + 0x80;
            u8 a = 0xFF;

            // grab bc pointer
            dBc_c *currBc = &owner->mBc;

            // Get the actor's position
            float ownerPosX = owner->mPos.x;
            float ownerPosY = owner->mPos.y;

            // Make an array of sensors
            dBcSensor_c* sensors[4] = {currBc->mpSensorFoot, currBc->mpSensorHead, currBc->mpSensorWall, currBc->mpSensorWall};

            // Draw the sensors
            for (int i = 0; i < 4; i++) {

                // Check if the sensor exists
                dBcSensor_c* sensor = sensors[i];
                if (sensor == NULL)
                    continue;

                // Multiplier for the adjacent sensors
                int mult = (i == 3) ? -1 : 1;
                float x1, y1, x2, y2;
                bool isLine = sensor->mFlags & 1;

                if (isLine == false) {
                    dBcSensorPoint_c* pointSensor = (dBcSensorPoint_c*)sensor;

                    x1 = ownerPosX + (float)(mult * pointSensor->mX / 4096);
                    y1 = ownerPosY + (float)(pointSensor->mY / 4096);

                    DrawPoint(x1, y1, 8005.0f, r, g, b, a);

                } else {
                    dBcSensorLine_c* lineSensor = (dBcSensorLine_c*)sensor;

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
    if (renderCollision) {
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
                float centreX = currCircle->mpOwner->mPos.x + currCircle->mCentreOffset.x;
                float centreY = currCircle->mpOwner->mPos.y + currCircle->mCentreOffset.y;
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

// Schedule renderer for drawing
kmBranchDefCpp(0x80830BD8, NULL, int, void) {
    m_instance.entry();
    return 1;
}