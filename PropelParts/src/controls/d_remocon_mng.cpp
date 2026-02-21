#include <kamek.h>
#include <game/bases/d_remocon_mng.hpp>
#include <new/bases/d_custom_controller.hpp>
#include <new/game_config.h>

// Replaces InitializeNone
kmBranchDefCpp(0x800dcc40, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    OSReport("sideways wiimote detected in port %d!\n", this_->mChannel+1);
    this_->mControllerType = CONTROLLER_TYPE_e::WIIMOTE;
    dCustomControllerInfo *cont = getCustomController(this_->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::WIIMOTE;
    return;
}

// Replaces ExecuteNone
kmBranchDefCpp(0x800dcc60, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    u8 extension = mPad::g_core[this_->mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_CORE) {
        dCustomController_c::changeRemoconMgrState(this_, extension);
    }
    return;
}

// Replaces InitalizeFreestyle
kmBranchDefCpp(0x800dcd70, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    OSReport("nunchuck detected in port %d!\n", this_->mChannel+1);
    this_->mControllerType = CONTROLLER_TYPE_e::NUNCHUCK;
    dCustomControllerInfo *cont = getCustomController(this_->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::NUNCHUCK;
    return;
}

// Replaces ExecuteFreestyle
kmBranchDefCpp(0x800dcd90, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    u8 extension = mPad::g_core[this_->mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_FREESTYLE) {
        dCustomController_c::changeRemoconMgrState(this_, extension);
    }
    return;
}

// Replace InitializeOther
kmBranchDefCpp(0x800dcea0, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    OSReport("invalid controller detected in port %d!\n", this_->mChannel+1);
    this_->mControllerType = CONTROLLER_TYPE_e::INVALID;
    dCustomControllerInfo *cont = getCustomController(this_->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::INVALID;
    return;
}

// Replaces ExecuteOther
kmBranchDefCpp(0x800dcec0, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    u8 extension = mPad::g_core[this_->mChannel]->maStatus->dev_type;
#ifdef CONTROLLER_EXPANSION_ENABLED
    if (extension != WPAD_DEV_FUTURE || extension != WPAD_DEV_GCN_FUTURE) {
        dCustomController_c::changeRemoconMgrState(this_, extension);
    }
#else
    if (extension != WPAD_DEV_FUTURE || extension != WPAD_DEV_CLASSIC) {
        dCustomController_c::changeRemoconMgrState(this_, extension);
    }
#endif
    return;
}

// Replace InitializeWait
kmBranchDefCpp(0x800dcaf0, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    OSReport("no controller detected in port %d!\n", this_->mChannel+1);
    this_->mControllerType = CONTROLLER_TYPE_e::NONE;
    dCustomControllerInfo *cont = getCustomController(this_->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::NONE;
    return;
}

// Replaces ExecuteWait
kmBranchDefCpp(0x800dcb10, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *this_) {
    u8 extension = mPad::g_core[this_->mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_NOT_FOUND) {
        dCustomController_c::changeRemoconMgrState(this_, extension);
    }
    return;
}