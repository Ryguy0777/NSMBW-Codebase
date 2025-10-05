#include <kamek.h>
#include <game/bases/d_remocon_mng.hpp>
#include <new/bases/d_custom_controller.hpp>

// dRemoconMng_c stuff

// replaces InitializeNone
kmBranchDefCpp(0x800dcc40, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    OSReport("sideways wiimote detected in port %d!\n", _this->mChannel+1);
    _this->mControllerType = CONTROLLER_TYPE_e::WIIMOTE;
    dCustomControllerInfo *cont = getCustomController(_this->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::WIIMOTE;
    return;
}

// replaces ExecuteNone
kmBranchDefCpp(0x800dcc60, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    u8 extension = mPad::g_core[_this->mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_CORE) {
        dCustomController_c::changeRemoconMgrState(_this, extension);
    }
    return;
}

// replaces InitalizeFreestyle
kmBranchDefCpp(0x800dcd70, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    OSReport("nunchuck detected in port %d!\n", _this->mChannel+1);
    _this->mControllerType = CONTROLLER_TYPE_e::NUNCHUCK;
    dCustomControllerInfo *cont = getCustomController(_this->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::NUNCHUCK;
    return;
}

// replaces ExecuteFreestyle
kmBranchDefCpp(0x800dcd90, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    u8 extension = mPad::g_core[_this->mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_FREESTYLE) {
        dCustomController_c::changeRemoconMgrState(_this, extension);
    }
    return;
}

// replace InitializeOther
kmBranchDefCpp(0x800dcea0, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    OSReport("invalid controller detected in port %d!\n", _this->mChannel+1);
    _this->mControllerType = CONTROLLER_TYPE_e::INVALID;
    dCustomControllerInfo *cont = getCustomController(_this->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::INVALID;
    return;
}

// replaces ExecuteOther
kmBranchDefCpp(0x800dcec0, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    u8 extension = mPad::g_core[_this->mChannel]->maStatus->dev_type;
#ifdef CONTROLLER_EXPANSION_ENABLED
    if (extension != WPAD_DEV_FUTURE || extensions != WPAD_DEV_GCN_FUTURE) {
        dCustomController_c::changeRemoconMgrState(_this, extension);
    }
#else
    if (extension != WPAD_DEV_FUTURE || extension != WPAD_DEV_CLASSIC) {
        dCustomController_c::changeRemoconMgrState(_this, extension);
    }
#endif
    return;
}

// replace InitializeWait
kmBranchDefCpp(0x800dcaf0, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    OSReport("no controller detected in port %d!\n", _this->mChannel+1);
    _this->mControllerType = CONTROLLER_TYPE_e::NONE;
    dCustomControllerInfo *cont = getCustomController(_this->mChannel);
    cont->mFlags &= ~CCFLAG_HAS_CUSTOM;
    cont->mFlags &= ~CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::NONE;
    return;
}

// replaces ExecuteWait
kmBranchDefCpp(0x800dcb10, NULL, void, dRemoconMng_c::dConnect_c::dExtension_c *_this) {
    u8 extension = mPad::g_core[_this->mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_NOT_FOUND) {
        dCustomController_c::changeRemoconMgrState(_this, extension);
    }
    return;
}