#include <kamek.h>
#include <game/bases/d_base_actor.hpp>

// add scale and rotation values to the signboard

kmBranchDefCpp(0x8081c928, 0x8081c92c, void, dBaseActor_c *this_) {
    // scale
    int scaleVal = this_->mParam >> 28 & 0xF; // nybble 5
    f32 newScale = 1.0f + (0.5f * scaleVal);

    // assign new actor scale
    this_->mScale.x = newScale;
    this_->mScale.y = newScale;
    this_->mScale.z = newScale;

    // rotation
    int rotVal = this_->mParam >> 24 & 0b111; // nybble 6, bits 1-3
    s16 newRot = rotVal * 0x1FFE;

    this_->mAngle.z = newRot;

    return;
}