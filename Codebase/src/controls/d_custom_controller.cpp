#include <kamek.h>
#include <new/bases/d_custom_controller.hpp>
#include <game/bases/d_fukidashi_manager.hpp>
#include <game/snd/snd_scene_manager.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_ac_py_key.hpp>
#include <new/lib/revolution/PAD.h>

dCustomController_c *dCustomController_c::m_instance = nullptr;
PADStatus dCustomController_c::saPadStatus[4];

void dCustomController_c::makeInstance() {
	dCustomController_c::m_instance = new(sizeof(dCustomController_c)) dCustomController_c;
}

#include <new/game_config.h>

void dCustomController_c::changeRemoconMgrState(dRemoconMng_c::dConnect_c::dExtension_c *self, u32 extension) {
    switch (extension) {
        case WPAD_DEV_CORE: // sideways wiimote
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_None);
            return;
        case WPAD_DEV_FREESTYLE: // nunchuck
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Freestyle);
            return;
#ifdef CONTROLLER_EXPANSION_ENABLED
        case WPAD_DEV_CLASSIC: // classic
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Classic);
            return;
        case WPAD_DEV_GAMECUBE: // gamecube
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Gamecube);
            return;
        /* case WPAD_DEV_GUITAR: // guitar
            self->mStateMgr.changeState(dRemoconMng_c::dConnect_c::dExtension_c::StateID_Guitar);
            return; */
#endif
        case WPAD_DEV_FUTURE: // unsupported
        case WPAD_DEV_GCN_FUTURE: // unsupported gc
#ifndef CONTROLLER_EXPANSION_ENABLED
        case WPAD_DEV_CLASSIC:
#endif
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

#ifdef CONTROLLER_EXPANSION_ENABLED
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
#endif
}

// assign game key id based on remoconmng
kmBranchDefAsm(0x800b5df8, 0x800b5e08) {
    stw r0, 0x8(r3)
}

// hide fukidashi if custom controller
kmBranchDefCpp(0x800b1990, NULL, void, dfukidashiInfo_c *_this) {
    if (!_this->mVisible || getCustomController(_this->mPlayer)->mFlags & CCFLAG_HAS_CUSTOM)
        return;
    _this->mLayout.entry();
}

// fix dAcPyKey_c not working with our face button setup
kmBranchDefCpp(0x8005e590, NULL, u16, dAcPyKey_c *_this) {
    KPADEXStatus_cl *cStatus = &mPad::g_core[_this->mRemoconID]->maStatus->ex_status.cl;
    dPADInfo *gStatus = &getCustomController(_this->mRemoconID)->mPadInfo;
    switch (dGameKey_c::m_instance->mRemocon[_this->mRemoconID]->mAttachedExtension) {
        case CONTROLLER_TYPE_e::CLASSIC:
            if (_this->mStatus & 0x181) return 0; // player is in demo state
            return cStatus->trig & (WPAD_BUTTON_CL_Y | WPAD_BUTTON_CL_X);
        case CONTROLLER_TYPE_e::GAMECUBE:
            if (_this->mStatus & 0x181) return 0; // player is in demo state
            return gStatus->mTrig & PAD_BUTTON_Y;
        case CONTROLLER_TYPE_e::NUNCHUCK:
            return _this->mTriggeredButtons & WPAD_BUTTON_B;
        default:
            return _this->mTriggeredButtons & WPAD_BUTTON_1;
    }
}

kmBranchDefCpp(0x8005e5d0, NULL, u16, dAcPyKey_c *_this) {
    KPADEXStatus_cl *cStatus = &mPad::g_core[_this->mRemoconID]->maStatus->ex_status.cl;
    dPADInfo *gStatus = &getCustomController(_this->mRemoconID)->mPadInfo;
    switch (dGameKey_c::m_instance->mRemocon[_this->mRemoconID]->mAttachedExtension) {
        case CONTROLLER_TYPE_e::CLASSIC:
            if (_this->mStatus & 0x181) return 0; // player is in demo state
            return cStatus->hold & (WPAD_BUTTON_CL_Y | WPAD_BUTTON_CL_X);
        case CONTROLLER_TYPE_e::GAMECUBE:
            if (_this->mStatus & 0x181) return 0; // player is in demo state
            return gStatus->mHold & PAD_BUTTON_Y;
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

// bool to see if hbm is open
kmBranchDefCpp(0x800695a0, NULL, void) {
    SndSceneMgr::sInstance->enterHBM();
    OSReport("opening HBM!\n");
    dCustomController_c::m_instance->mIsHBMOpen = true;
}

kmBranchDefCpp(0x800695b0, NULL, void) {
    SndSceneMgr::sInstance->exitHBM();
    OSReport("closing HBM!\n");
    dCustomController_c::m_instance->mIsHBMOpen = false;
}

// use sideways inputs for new controller types
kmWrite32(0x800b60dc, 0x2c000001);

kmWrite32(0x800b6228, 0x2c000001);
kmWrite32(0x800b622c, 0x41820024);

// separate view map button from bubble button for new controller types
kmBranchDefCpp(0x800fd4a0, NULL, bool, int player) {
    dCustomControllerInfo *cont = getCustomController(player);
    if (cont->mFlags & CCFLAG_NO_ACC || cont->mControllerType == CONTROLLER_TYPE_e::NUNCHUCK)
        return false;
    return true;
}

// tilt controlled objects

// instead of using tilt, these will use shoulder buttons
s16 getLRShort(u32 pid) {
    dGameKeyCore_c *input = dGameKey_c::m_instance->mRemocon[pid];
    // return tilt value as a usable s16
    return (s16)(int)(((input->mAccVertX).y / 30.0) * 16384.0);
}

inline s16 clamp(s16 value, s16 one, s16 two) { return (value < one) ? one : ((value > two) ? two : value); }

// for objects where L and R values simply add to the object's rotation
bool handleTiltAdd(s16 *rot, s16 target, s16 increment, s8 player, s16 max) {
    if (player < 0)
        return sLib::chaseAngle(rot, target, increment);
    
    // if no accelerometer, return
    dCustomControllerInfo *cont = getCustomController(player, 1);
    if (cont == NULL || ((cont->mFlags & CCFLAG_NO_ACC) == 0))
        return sLib::chaseAngle(rot, target, increment);
    
    // add current L/R values to object rotation
    s16 tiltVal = getLRShort(player);
    s16 newTarget = (*rot + tiltVal);
    if (max != 0)
        newTarget = clamp(newTarget, -max, max);
    
    return sLib::chaseAngle(rot, newTarget, increment);
}

// 6-6 boat
kmBranchDefAsm(0x80871184, 0x80871188) {
    nofralloc

	mr r12, r6
	mr r11, r7
	lbz r6, 0x430(r30)
	li r7, 0x4000
	bl handleTiltAdd
	mr r6, r12
	mr r7, r11
	blr
}

// 1-up blast cannon
kmBranchDefCpp(0x80a964e0, NULL, bool, void *cannon, short target) {
    return handleTiltAdd((s16*)((u8*)cannon + 0x7a4), target, 400, *(u32*)(((u8*)cannon)+0x778), 0x3520);
}

// tilt controlled wire fence
extern "C" s16 remoWireGetTilt(void *);

kmBranchDefCpp(0x808c4860, NULL, s16, void *remoWire) {
    // get tilt amount
    s16 tilt = remoWireGetTilt(remoWire);
    if (tilt < 0x1a2c) {
        if (tilt < -0x1a2b)
            tilt = 0xffffe5d4;
    } else {
        tilt = 0x1a2c;
    }

    // if no accelerometer, return
    dCustomControllerInfo *cont = getCustomController((int)*(s8*)((u8*)remoWire+0x63d), 1);
    if (cont == NULL || ((cont->mFlags & CCFLAG_NO_ACC) == 0))
        return tilt;
    
    // otherwise return the inverse of the tilt value
    return -tilt;
}

// tilt controlled girder
extern "C" s16 getRemoTiltValue(int, float, float);

s16 seesawRots[4] = {0, 0, 0, 0};

kmBranchDefCpp(0x8083f550, NULL, s16, void *remoSeesaw) {
    int player = (int)*(char *)((u8*)remoSeesaw+0x430);
    s16 tiltVal;
    // if no accelerometer, return
    dCustomControllerInfo *cont = getCustomController(player, 1);
    if (cont == NULL || ((cont->mFlags & CCFLAG_NO_ACC) == 0)) {
        tiltVal = getRemoTiltValue(player,5.0,1.0);
    } else {
        s16 LR = getLRShort(player);
        s16 currentRot = *(short *)((u8*)remoSeesaw+0x54a);
        seesawRots[player] = clamp(seesawRots[player] + LR, -0x1a2c, 0x1a2c);
        tiltVal = seesawRots[player];
        // called when we gain control, so reset the tilt value
        seesawRots[player] = 0;
    }
    return tiltVal;
}

kmBranchDefCpp(0x8083f4f0, NULL, s16, void *remoSeesaw) {
    int player = (int)*(char *)((u8*)remoSeesaw+0x430); 
    s16 tiltVal;
    // if no accelerometer, return
    dCustomControllerInfo *cont = getCustomController(player, 1);
    if (cont == NULL || ((cont->mFlags & CCFLAG_NO_ACC) == 0)) {
        tiltVal = getRemoTiltValue(player,5.0,1.0);
    } else {
        s16 LR = getLRShort(player);
        s16 currentRot = *(short *)((u8*)remoSeesaw+0x54a);
        seesawRots[player] = clamp(seesawRots[player] + LR, -0x1a2c, 0x1a2c);
        tiltVal = seesawRots[player];
    }
    if (tiltVal < 0x1a2c) {
        if (tiltVal < -0x1a2b) {
            tiltVal = -0x1a2c;
        }
    } else {
        tiltVal = 0x1a2c;
    }
    return tiltVal;
}