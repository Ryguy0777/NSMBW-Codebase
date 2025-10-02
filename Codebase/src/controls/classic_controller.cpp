#include <kamek.h>
#include <new/bases/d_custom_controller.hpp>

// dRemoconMng_c state
STATE_DEFINE(dRemoconMng_c::dConnect_c::dExtension_c, Classic);

void dRemoconMng_c::dConnect_c::dExtension_c::initializeState_Classic() {
    OSReport("classic controller detected in port %d!\n", mChannel+1);
    mControllerType = CONTROLLER_TYPE_e::CLASSIC;
    dCustomControllerInfo *cont = getCustomController(mChannel);
    cont->mFlags |= CCFLAG_HAS_CUSTOM;
    cont->mFlags |= CCFLAG_NO_ACC;
    cont->mFlags &= ~CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::CLASSIC;
    return;
}

void dRemoconMng_c::dConnect_c::dExtension_c::finalizeState_Classic() {}

void dRemoconMng_c::dConnect_c::dExtension_c::executeState_Classic() {
    u8 extension = mPad::g_core[mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_CLASSIC) {
        dCustomController_c::changeRemoconMgrState(this, extension);
    }
    return;
}

// controller mappings
void dCustomController_c::mapClassicButtons(EGG::CoreController *controller, int controllerID) {
    KPADStatus *kStatus = controller->maStatus;
    KPADEXStatus_cl *cStatus = &controller->maStatus->ex_status.cl;
    dCustomControllerInfo *customController = getCustomController(controllerID);

#ifdef DEBUG_INPUTS
    OSReport("Classic: %08x %08x %08x\n", cStatus->hold, cStatus->trig, cStatus->release);
    OSReport("Classic L: %f %f, %f R: %f %f, %f\n", cStatus->lstick.x, cStatus->lstick.y, cStatus->ltrigger, cStatus->rstick.x, cStatus->rstick.y, cStatus->rtrigger);
#endif

    clearWiimotedata(kStatus);

    // convert to sideways Wii Remote buttons
    if (cStatus->hold & WPAD_BUTTON_CL_LEFT) {
        kStatus->hold |= WPAD_BUTTON_UP;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_RIGHT) {
        kStatus->hold |= WPAD_BUTTON_DOWN;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_DOWN) {
        kStatus->hold |= WPAD_BUTTON_LEFT;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_UP) {
        kStatus->hold |= WPAD_BUTTON_RIGHT;
    }

    if (cStatus->hold & WPAD_BUTTON_CL_A) {
        kStatus->hold |= WPAD_BUTTON_2;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_B) {
        kStatus->hold |= WPAD_BUTTON_1;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_Y) {
        kStatus->hold |= WPAD_BUTTON_FS_C;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_PLUS) {
        kStatus->hold |= WPAD_BUTTON_PLUS;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_MINUS) {
        kStatus->hold |= WPAD_BUTTON_MINUS;
    }
    if (cStatus->trig & (WPAD_BUTTON_CL_ZL | WPAD_BUTTON_CL_ZR)) {
        dCustomController_c::shakeController(controllerID);
    }
    if (cStatus->hold & WPAD_BUTTON_CL_HOME) {
        kStatus->hold |= WPAD_BUTTON_HOME;
    }

    // map stick to D-pad
    if (cStatus->lstick.x < -0.5 && !(kStatus->hold & WPAD_BUTTON_DOWN)) {
        kStatus->hold |= WPAD_BUTTON_UP;
    }
    if (cStatus->lstick.x > 0.5 && !(kStatus->hold & WPAD_BUTTON_UP)) {
        kStatus->hold |= WPAD_BUTTON_DOWN;
    }
    if (cStatus->lstick.y < -0.5 && !(kStatus->hold & WPAD_BUTTON_RIGHT)) {
        kStatus->hold |= WPAD_BUTTON_LEFT;
    }
    if (cStatus->lstick.y > 0.5 && !(kStatus->hold & WPAD_BUTTON_LEFT)) {
        kStatus->hold |= WPAD_BUTTON_RIGHT;
    }

    kStatus->trig = kStatus->hold & ~customController->mLastPressed;
    kStatus->release = customController->mLastPressed & ~kStatus->hold;
    kStatus->wpad_err = 0;

    customController->mLastPressed = kStatus->hold;

    // for in-game but not for use in menus
    if (cStatus->hold & WPAD_BUTTON_CL_B) {
        kStatus->hold |= WPAD_BUTTON_2;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_Y) {
        kStatus->hold |= WPAD_BUTTON_1;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_X) {
        kStatus->hold |= WPAD_BUTTON_1;
    }
    if (cStatus->hold & WPAD_BUTTON_CL_MINUS) {
        kStatus->hold |= WPAD_BUTTON_A;
    }

    // default tilt behavior
    // some actors are programmed to not use this value directly!
    float tilt = cStatus->ltrigger;
    tilt -= cStatus->rtrigger;

    kStatus->acc.z = tilt;
    kStatus->acc_vertical.y = -tilt;
}

// enable use of the classic controller in the homebutton menu
kmWriteNop(0x80109c78);
kmWriteNop(0x80109c7c);
kmWriteNop(0x80109c80);
kmWriteNop(0x80109c84);

static const f32 stickMoveCoefficent = 2048.0f/72.0f;

static f32 AbsClamp(f32 val, f32 max) {
    return ((val > max) ? max : (val < -max) ? -max : val);
}

void mapStickToPosHBM(int channel, Vec2 *pos) {
    KPADEXStatus_cl *cStatus = &mPad::g_core[channel]->maStatus->ex_status.cl;
    Vec2 stick;
    stick.x = cStatus->lstick.x / stickMoveCoefficent;
    stick.y = cStatus->lstick.y / stickMoveCoefficent;
    // don't let cursor go off screen edge
    pos->x = AbsClamp(pos->x + stick.x, 1.0f);
    pos->y = AbsClamp(pos->y - stick.y, 1.0f);
}

// fix dAcPyKey_c not working with our face button setup

kmBranchDefCpp(0x8005e590, NULL, u16, dAcPyKey_c *_this) {
    KPADEXStatus_cl *cStatus = &mPad::g_core[_this->mRemoconID]->maStatus->ex_status.cl;
    switch (dGameKey_c::m_instance->mRemocon[_this->mRemoconID]->mAttachedExtension) {
        case CONTROLLER_TYPE_e::CLASSIC:
            if (_this->mStatus & 0x181) return 0; // player is in demo state
            return cStatus->trig & (WPAD_BUTTON_CL_Y | WPAD_BUTTON_CL_X);
        case CONTROLLER_TYPE_e::NUNCHUCK:
            return _this->mTriggeredButtons & WPAD_BUTTON_B;
        default:
            return _this->mTriggeredButtons & WPAD_BUTTON_1;
    }
}

kmBranchDefCpp(0x8005e5d0, NULL, u16, dAcPyKey_c *_this) {
    KPADEXStatus_cl *cStatus = &mPad::g_core[_this->mRemoconID]->maStatus->ex_status.cl;
    switch (dGameKey_c::m_instance->mRemocon[_this->mRemoconID]->mAttachedExtension) {
        case CONTROLLER_TYPE_e::CLASSIC:
            if (_this->mStatus & 0x181) return 0; // player is in demo state
            return cStatus->hold & (WPAD_BUTTON_CL_Y | WPAD_BUTTON_CL_X);
        case CONTROLLER_TYPE_e::NUNCHUCK:
            return _this->mDownButtons & WPAD_BUTTON_B;
        default:
            return _this->mDownButtons & WPAD_BUTTON_1;
    }
}

kmBranchDefCpp(0x8005e610, NULL, u16, dAcPyKey_c *_this) {
    KPADEXStatus_cl *cStatus = &mPad::g_core[_this->mRemoconID]->maStatus->ex_status.cl;
    switch (dGameKey_c::m_instance->mRemocon[_this->mRemoconID]->mAttachedExtension) {
        case CONTROLLER_TYPE_e::CLASSIC:
            if (_this->mStatus & 0x181) return 0; // player is in demo state
            return cStatus->hold & (WPAD_BUTTON_CL_Y | WPAD_BUTTON_CL_X);
        default:
            return _this->mDownButtons & WPAD_BUTTON_1;
    }
}