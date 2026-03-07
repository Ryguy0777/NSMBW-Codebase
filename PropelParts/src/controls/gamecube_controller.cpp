#include <kamek.h>
#include <propelparts/bases/d_custom_controller.hpp>
#include <propelparts/lib/revolution/SI.h>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_actor.hpp>
#include <propelparts/lib/revolution/PAD.h>

// dRemoconMng_c state
STATE_DEFINE(dRemoconMng_c::dConnect_c::dExtension_c, Gamecube);

void dRemoconMng_c::dConnect_c::dExtension_c::initializeState_Gamecube() {
    OSReport("gamecube controller detected in port %d!\n", mChannel+1);
    mControllerType = CONTROLLER_TYPE_e::GAMECUBE;
    dCustomControllerInfo *cont = getCustomController(mChannel);
    cont->mFlags |= CCFLAG_HAS_CUSTOM;
    cont->mFlags |= CCFLAG_NO_ACC;
    cont->mFlags |= CCFLAG_NO_WIIMOTE;
    cont->mControllerType = CONTROLLER_TYPE_e::GAMECUBE;
    return;
}

void dRemoconMng_c::dConnect_c::dExtension_c::finalizeState_Gamecube() {}

void dRemoconMng_c::dConnect_c::dExtension_c::executeState_Gamecube() {
    u8 extension = mPad::g_core[mChannel]->maStatus->dev_type;
    if (extension != WPAD_DEV_GAMECUBE) {
        dCustomController_c::changeRemoconMgrState(this, extension);
    }
    return;
}

// Controller mappings
void dCustomController_c::mapGameCubeButtons(EGG::CoreController *controller, int controllerID) {
    KPADStatus *kStatus = controller->maStatus;
    dCustomControllerInfo *customController = getCustomController(controllerID);
    dPADInfo *gStatus = &customController->mPadInfo;

#ifdef DEBUG_INPUTS
    OSReport("GameCube: %08x %08x %08x\n", gStatus->mHold, gStatus->mTrig, gStatus->mRelease);
    OSReport("GameCube L: %f %f, %f R: %f %f, %f\n", gStatus->mStick.x, gStatus->mStick.y, gStatus->mLTrigger, gStatus->mSubstick.x, gStatus->mSubstick.y, gStatus->mRTrigger);
#endif

    clearWiimotedata(kStatus, false);

    // Convert to sideways Wii Remote buttons
    if (gStatus->mHold & PAD_BUTTON_LEFT) {
        kStatus->hold |= WPAD_BUTTON_UP;
    }
    if (gStatus->mHold & PAD_BUTTON_RIGHT) {
        kStatus->hold |= WPAD_BUTTON_DOWN;
    }
    if (gStatus->mHold & PAD_BUTTON_DOWN) {
        kStatus->hold |= WPAD_BUTTON_LEFT;
    }
    if (gStatus->mHold & PAD_BUTTON_UP) {
        kStatus->hold |= WPAD_BUTTON_RIGHT;
    }

    if (gStatus->mHold & PAD_BUTTON_A) {
        kStatus->hold |= WPAD_BUTTON_2;
    }
    if (gStatus->mHold & PAD_BUTTON_B) {
        kStatus->hold |= WPAD_BUTTON_1;
    }
    if (gStatus->mHold & PAD_BUTTON_Y) {
        kStatus->hold |= WPAD_BUTTON_FS_C;
    }
    if (gStatus->mHold & PAD_BUTTON_START) {
        kStatus->hold |= WPAD_BUTTON_PLUS;
    }
    if (gStatus->mHold & PAD_TRIGGER_Z) {
        kStatus->hold |= WPAD_BUTTON_MINUS;
    }
    if (gStatus->mTrig & PAD_TRIGGER_Z) {
        dCustomController_c::shakeController(controllerID);
    }

    // Map stick to D-pad
    if (gStatus->mStick.x < -0.5f && !(kStatus->hold & WPAD_BUTTON_DOWN)) {
        kStatus->hold |= WPAD_BUTTON_UP;
    }
    if (gStatus->mStick.x > 0.5f && !(kStatus->hold & WPAD_BUTTON_UP)) {
        kStatus->hold |= WPAD_BUTTON_DOWN;
    }
    if (gStatus->mStick.y < -0.5f && !(kStatus->hold & WPAD_BUTTON_RIGHT)) {
        kStatus->hold |= WPAD_BUTTON_LEFT;
    }
    if (gStatus->mStick.y > 0.5f && !(kStatus->hold & WPAD_BUTTON_LEFT)) {
        kStatus->hold |= WPAD_BUTTON_RIGHT;
    }

    kStatus->trig = kStatus->hold & ~customController->mLastPressed;
    kStatus->release = customController->mLastPressed & ~kStatus->hold;
    kStatus->wpad_err = 0;

    customController->mLastPressed = kStatus->hold;

    // For in-game but not for use in menus
    if (gStatus->mHold & PAD_BUTTON_Y) {
        kStatus->hold |= WPAD_BUTTON_1;
    }
    if (gStatus->mHold & PAD_BUTTON_X) {
        kStatus->hold |= WPAD_BUTTON_A;
    }

    // Default tilt behavior
    // Some actors are programmed to not use this value directly!
    float tilt = gStatus->mLTrigger;
    tilt -= gStatus->mRTrigger;

    kStatus->acc.z = tilt;
    kStatus->acc_vertical.y = -tilt;
}

// Handle GC controller connection
u32 dCustomController_c::checkForGCConnection(int channel) {
    u32 probe = SIProbe(channel);
    if (probe == SI_ERROR_NO_RESPONSE) {
        // No controller
        mControllers[channel].mFlags |= CCFLAG_HAS_AUTOCONNECT;
        mControllers[channel].mFlags &= ~CCFLAG_NO_AUTOCONNECT;
        return GCC_TYPE_e::NONE;
    } else if (probe & SI_GC_STANDARD) {
        if (saPadStatus[channel].err != PAD_ERR_NO_CONTROLLER) {
            if (!(mControllers[channel].mFlags & CCFLAG_NO_AUTOCONNECT)) {
                if (dScene_c::m_nowScene == fProfile::BOOT && !(mControllers[channel].mFlags & CCFLAG_HAS_AUTOCONNECT) && WPADProbe(channel, NULL) != -1) {
                    // Don't detect a GC controller if connected on game boot, unless a button is pressed or there's no wiimote
                    OSReport("GC Controller detected on boot! ignoring for now...\n");
                    // Don't connect GC controller on boot
                    mControllers[channel].mFlags |= CCFLAG_NO_AUTOCONNECT;
                    return GCC_TYPE_e::NONE;
                }
                return GCC_TYPE_e::STANDARD;
            } else { // GC is connected on boot, don't connect unless we press a button
                if (mControllers[channel].mPadInfo.mHold) {
                    OSReport("Pressing something! connecting the controller\n");
                    mControllers[channel].mFlags |= CCFLAG_HAS_AUTOCONNECT;
                    mControllers[channel].mFlags &= ~CCFLAG_NO_AUTOCONNECT;
                    return GCC_TYPE_e::STANDARD;
                }
                return GCC_TYPE_e::NONE;
            }
        }
        // No controller
        mControllers[channel].mFlags |= CCFLAG_HAS_AUTOCONNECT;
        mControllers[channel].mFlags &= ~CCFLAG_NO_AUTOCONNECT;
        return GCC_TYPE_e::NONE;
    } else if (probe != SI_ERROR_BUSY && probe != SI_ERROR_UNKNOWN) {
        return GCC_TYPE_e::INVALID;
    }
    return GCC_TYPE_e::NONE;
}

// Do GC rumble
kmBranchDefCpp(0x802bcbf0, NULL, void, EGG::CoreController *this_) {
    if (getCustomController(this_->mChannel)->mControllerType == CONTROLLER_TYPE_e::GAMECUBE) {
        PADControlMotor(this_->mChannel, PAD_MOTOR_RUMBLE);
    } else {
        WPADControlMotor(this_->mChannel, 1);
    }
}

kmBranchDefCpp(0x802bcc00, NULL, void, EGG::CoreController *this_) {
    if (getCustomController(this_->mChannel)->mControllerType == CONTROLLER_TYPE_e::GAMECUBE) {
        PADControlMotor(this_->mChannel, PAD_MOTOR_STOP);
    } else {
        WPADControlMotor(this_->mChannel, 0);
    }
}

// Disable speaker if GC connected
u32 remote_players[4] = {0x2, 0x4, 0x8, 0x10};

kmBranchDefCpp(0x80069530, NULL, u32, u32 playerID) {
    if (dAudio::isForbidRemoteSE() || getCustomController(playerID)->mFlags & CCFLAG_NO_WIIMOTE) {
        return 0;
    } else {
        return remote_players[playerID];
    }
}

// Always report full battery with GC
u32 accountForGCBattery(int channel) {
    if (getCustomController(channel)->mFlags & CCFLAG_NO_WIIMOTE) {
        // If a Wii remote is not connected, always return full battery
        if (mPad::getBatteryLevel_ch((mPad::CH_e)channel) < 0)
            return 4;
    }
    return mPad::getBatteryLevel_ch((mPad::CH_e)channel);
}

kmCall(0x800dc930, &accountForGCBattery);
kmCall(0x800dca40, &accountForGCBattery);

// Read & reset GC inputs
kmCallDefCpp(0x8016f398, void, EGG::CoreControllerMgr *this_) {
    // Read GC inputs & clamp sticks
    PADRead(dCustomController_c::saPadStatus);
    PADClamp(dCustomController_c::saPadStatus);
    // Set padInfo
    dCustomController_c::setPadInfo();
    return this_->beginFrame();
}

kmBranchDefCpp(0x8016f560, NULL, void, EGG::CoreControllerMgr *this_) {
    // Call original function
    this_->endFrame();

    u32 resetBits = 0;
    u32 padBit;
    for (int i = 0; i < 4; i++) {
        padBit = (PAD_CHAN0_BIT >> i);
        if (dCustomController_c::saPadStatus[i].err == PAD_ERR_NO_CONTROLLER) {
            resetBits |= padBit;
        }
        this_->mDevTypes(i) = mPad::g_core[i]->maStatus->dev_type;
    }
    if (resetBits) {
        PADReset(resetBits);
    }
    return;
}