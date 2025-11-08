#include <kamek.h>
#include <game/bases/d_a_en_coin_angle.hpp>
#include <lib/MSL/math.h>

// Add additional rotation settings to roation-controlled coins

kmBranchDefCpp(0x809E5248, 0x809E524C, void, daEnCoinAngle_c *this_) {
    this_->rotateCoin();

    // Rotation lock
    int forceAngle = this_->mParam >> 24 & 0xF; // Nybble 6
    if (forceAngle != 0) {
        this_->mAngle.z = -0x1000 * forceAngle;
        return;
    }

    // Always upright flag
    if ((this_->mParam >> 28) & 1) {
        this_->mAngle.z = 0;
    }

    // Tilted coins
    if ((this_->mParam >> 28) & 2) {
        dActor_c *rotController = (dActor_c *)fManager_c::searchBaseByID(this_->mControllerUniqueID);
        if (rotController != 0x0) {
            Vec2 deltaPos;
            deltaPos.x = this_->mPos.x - rotController->mPos.x;
            deltaPos.y = this_->mPos.y - rotController->mPos.y;

            this_->mAngle.z = -atan2(deltaPos.x, deltaPos.y) * 0x8000 / M_PI;
        }
    }
}