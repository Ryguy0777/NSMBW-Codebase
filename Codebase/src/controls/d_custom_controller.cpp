#include <kamek.h>
#include <new/bases/d_custom_controller.hpp>

dCustomController_c *dCustomController_c::m_instance = nullptr;
PADStatus dCustomController_c::saPadStatus[4];

void createCustomControllerClass() {
	dCustomController_c::m_instance = new(sizeof(dCustomController_c)) dCustomController_c;
}

void dCustomController_c::changeRemoconMgrState(dRemoconMng_c::dConnect_c::dExtension_c *self, u32 extension) {
    switch (extension) {
        case WPAD_DEV_CORE: // sideways wiimote
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_None);
            return;
        case WPAD_DEV_FREESTYLE: // nunchuck
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Freestyle);
            return;
        case WPAD_DEV_CLASSIC: // classic
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Classic);
            return;
        case WPAD_DEV_GAMECUBE: // gamecube
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Gamecube);
            return;
        /* case WPAD_DEV_GUITAR: // guitar
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Guitar);
            return; */
        case WPAD_DEV_FUTURE: // unsupported
        case WPAD_DEV_GCN_FUTURE: // unsupported gc
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Other);
            return;
    }
}

void dCustomController_c::setPadInfo() {
    for (int i = 0; i < 4; i++) {
        // get raw inputs
        PADStatus *padStatus = &saPadStatus[i];
        dCustomControllerInfo *cont = getCustomController(i);

        // buttons
        u32 lastButton = cont->mPadInfo.mHold;
        u32 gc_held = padStatus->button;

        cont->mPadInfo.mHold = gc_held;
        cont->mPadInfo.mTrig = gc_held & ~lastButton;
        cont->mPadInfo.mRelease = lastButton & gc_held;

        // triggers
        // apparently OEM controllers never report a full analog value
        // so we report a full press if the digital input is being pressed
        cont->mPadInfo.mLTrigger = (gc_held & PAD_TRIGGER_L) ? 1.0f : (float) padStatus->triggerL / 255;
        cont->mPadInfo.mRTrigger = (gc_held & PAD_TRIGGER_R) ? 1.0f : (float) padStatus->triggerR / 255;

        // sticks
        cont->mPadInfo.mStick.x = (float) padStatus->stickX / 110;
        cont->mPadInfo.mStick.y = (float) padStatus->stickY / 110;
        cont->mPadInfo.mSubstick.x = (float) padStatus->substickX / 110;
        cont->mPadInfo.mSubstick.y = (float) padStatus->substickY / 110;
    }
}

void dCustomController_c::shakeController(int channel) {
    // only shake if replay is not active
    if (dScStage_c::m_instance->m_replayPlay_p[channel] == nullptr) {
        dGameKey_c::m_instance->mRemocon[channel]->m_8d = true;
    }
}

void dCustomController_c::clearWiimotedata(KPADStatus *kStatus, bool disablePointer) {
    Vec zeroVec = {0,0,0};
    Vec2 zeroVec2 = {0,0};

    if (!disablePointer) {
        if (!mIsHBMOpen) {
            kStatus->hold &= WPAD_BUTTON_HOME;
            kStatus->trig &= WPAD_BUTTON_HOME;
            kStatus->release &= WPAD_BUTTON_HOME;
        }
    } else {
        kStatus->hold = 0;
        kStatus->trig = 0;
        kStatus->release = 0;
    }
    kStatus->acc = zeroVec;
    kStatus->acc_value = 0;
    kStatus->acc_speed = 0;
    if (disablePointer) {
        kStatus->pos = zeroVec2;
        kStatus->vec = zeroVec2;
        kStatus->speed = 0;
        kStatus->horizon = zeroVec2;
        kStatus->hori_vec = zeroVec2;
        kStatus->hori_speed = 0;
        kStatus->dist = 0;
        kStatus->dist_vec = 0;
        kStatus->dist_speed = 0;
    }
    kStatus->acc_vertical = zeroVec2;
}

kmCallDefCpp(0x802bdc24, void, EGG::CoreController *_this) {
    _this->beginFrame(nullptr);

#ifdef DEBUG_INPUTS
    OSReport("Wiimote: %08x %08x %08x\n", _this->maStatus->hold, _this->maStatus->trig, _this->maStatus->release);
#endif

    // assign wpad device type based on gc port type
    u32 gcctype = dCustomController_c::m_instance->checkForGCConnection(_this->mChannel);
    switch (gcctype) {
        case GCC_TYPE_e::INVALID:
            _this->maStatus->dev_type = WPAD_DEV_GCN_FUTURE;
            break;
        case GCC_TYPE_e::STANDARD:
            _this->maStatus->dev_type = WPAD_DEV_GAMECUBE;
            break;
    }

    if (gcctype != GCC_TYPE_e::NONE) {
        // register controller as present
        _this->mFlag.setBit(0);
    }
    // custom controller inputs
    switch (_this->maStatus->dev_type) {
        case WPAD_DEV_CLASSIC:
            return dCustomController_c::m_instance->mapClassicButtons(_this, _this->mChannel);
        case WPAD_DEV_GAMECUBE:
            return dCustomController_c::m_instance->mapGameCubeButtons(_this, _this->mChannel);
        /* case WPAD_DEV_GUITAR:
            return dCustomController_c::m_instance->mapGuitarButtons(_this, _this->mChannel); */
    }
}

// assign game key id based on remoconmng
kmBranchDefAsm(0x800b5df8, 0x800b5e08) {
    stw r0, 0x8(r3)
}