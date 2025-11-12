#include <kamek.h>
#include <game/bases/d_info.hpp>
#include <nw4r/lyt/lyt_pane.h>

// Allows a message printed from the BMG to not use the X-scale
// from the message flags if a certain userdata is present

// To use:
// Add an `ignoreScale` userdata to a pane

extern "C" void returnFromMessageScaleHack();

static bool disableMessageScaling = false;

// Disable the flag
kmBranchDefAsm(0x800C9CCC, 0x800C9CD0) {
    stb     r0, 0x3DA(r30)

    lis     r4, disableMessageScaling@h
    ori     r4, r4, disableMessageScaling@l
    stb     r0, 0(r4)
    blr
}

// Check pane for ExtUserData
kmBranchDefCpp(0x800C9BF0, NULL, void, nw4r::lyt::Pane *this_) {
    ExtUserData_s *usd;

    // PictureFont button shadow
    usd = this_->FindExtUserDataByName("shadow");
    if (usd != nullptr) {
        dInfo_c::m_instance->mShadowFlag = true;
    }

    // New: Ignore X-scale message flag
    usd = this_->FindExtUserDataByName("ignoreScale");
    if (usd != nullptr) {
        disableMessageScaling = true;
    }
}

// Skip text scaling
kmBranchDefAsm(0x800C9D90, 0x800C9D94) {
    lis     r4, disableMessageScaling@h
    ori     r4, r4, disableMessageScaling@l
    lbz     r4, 0(r4)
    cmpwi   r4, 1
    beq     skip_scaling

    fmuls   f3, f31, f4
    b returnFromMessageScaleHack

    skip_scaling:
    blr
}