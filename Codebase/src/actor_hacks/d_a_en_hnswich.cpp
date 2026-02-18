#include <kamek.h>
#include <game/bases/d_a_en_hnswich.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_info.hpp>
#include <game/snd/snd_scene_manager.hpp>
#include <new/bases/d_msgbox_window.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

// Add support for colored ! Switches that fill in associated switch blocks
// Nybbles 12.0-12.3 in Reggie are now used as a "switch type" 

// Switch Palace switch
extern "C" void *daEnHnswich_c_classinit();
fProfile::fActorProfile_c g_profile_EN_PALACE_SWITCH = { &daEnHnswich_c_classinit, fProfile::EN_BOSS_KOOPA_BIG_SWITCH, fProfile::DRAW_ORDER::EN_BOSS_KOOPA_BIG_SWITCH, 0x4 };
const char* l_PALACE_SWITCH_res [] = {"switch_palace", NULL};
const dActorData_c c_PALACE_SWITCH_actor_data = {fProfile::EN_PALACE_SWITCH, 8, -16, 0, 0, 8, 8, 0, 0, 0, 0, ACTOR_CREATE_MAPOBJ};
dCustomProfile_c l_PALACE_SWITCH_profile(&g_profile_EN_PALACE_SWITCH, "EN_PALACE_SWITCH", SpriteId::EN_PALACE_SWITCH, &c_PALACE_SWITCH_actor_data, l_PALACE_SWITCH_res);

#if GAME_REVISION < GAME_REVISION_K
#define PALACE_SWITCH_ID 787
#elif GAME_REVISION < GAME_REVISION_C
#define PALACE_SWITCH_ID 789
#else
#define PALACE_SWITCH_ID 791
#endif

kmBranchDefAsm(0x80a1984c, 0x80a19850) {
    cmplwi r0, PALACE_SWITCH_ID
    bne notPalaceSwitch
    li r0, 0x4
    stw r0, 0x88c(r3)
    stw r0, 0x884(r3)

    notPalaceSwitch:
    or r3, r28, r28
    blr
}

// Originally, the switch will place itself upside down if 
// Nybble 12 is greater than 1, we change this to just check if nybble 12.4 is 1
kmWrite32(0x80a198d0, 0x550007fe);

// Add custom texpat animation to the switch model
const char *l_switch_res_name[5] = {
    "switch",
    "switch",
    "switch",
    "switch",
    "switch_koopa"
};

const char *l_switch_brres_name[5] = {
    "g3d/switch.brres",
    "g3d/switch.brres",
    "g3d/switch.brres",
    "g3d/switch.brres",
    "g3d/switch_koopa.brres"
};

const char *l_switch_model_name[5] = {
    "switch_hatena",
    "switch_P",
    "switch_bikkuri",
    "switch_bikkuri",
    "switch_koopa"
};

const char *l_color_switch_model_name[5] = {
    "switch_bikkuri",
    "switch_bikkuri_y",
    "switch_bikkuri",
    "switch_bikkuri_g",
    "switch_bikkuri_b",
};

const char *l_palace_switch_model_name[5] = {
    "switch_palace",
    "switch_palace_y",
    "switch_palace",
    "switch_palace_g",
    "switch_palace_b",
};

ulong l_switch_buffer[5] = {
    0x224,
    0x224,
    0x224,
    0x224,
    0x128,
};

const char *l_switch_anim_name[5] = {
    "down_switch_hatena",
    "down_switch_P",
    "down_switch_bikkuri",
    "down_switch_bikkuri",
    "down"
};

kmBranchDefCpp(0x80a19e70, NULL, void, daEnHnswich_c *this_) {
    this_->mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    if (this_->mProfName == fProfile::EN_PALACE_SWITCH) {
        this_->mRes = dResMng_c::m_instance->getRes("switch_palace", "g3d/switch_palace.brres");
    } else {
        this_->mRes = dResMng_c::m_instance->getRes(l_switch_res_name[this_->mSwitchType], l_switch_brres_name[this_->mSwitchType]);
    }

    // Load a colored switch model if switch type is 2
    int switchColor = this_->mParam >> 1 & 7;
    nw4r::g3d::ResMdl mdl;
    if (this_->mSwitchType != 2) {
        if (this_->mProfName == fProfile::EN_PALACE_SWITCH) {
            mdl = this_->mRes.GetResMdl(l_palace_switch_model_name[switchColor]);
        } else {
            mdl = this_->mRes.GetResMdl(l_switch_model_name[this_->mSwitchType]);
        }
    } else {
        mdl = this_->mRes.GetResMdl(l_color_switch_model_name[switchColor]);
    }

    this_->mModel.create(mdl, &this_->mAllocator, l_switch_buffer[this_->mSwitchType], 1, nullptr);
    dActor_c::setSoftLight_MapObj(this_->mModel);

    nw4r::g3d::ResAnmChr resAnmChr = this_->mRes.GetResAnmChr(l_switch_anim_name[this_->mSwitchType]);
    this_->mAnmChr.create(mdl, resAnmChr, &this_->mAllocator, 0);

    if (this_->mSwitchType != 4) {
        nw4r::g3d::ResAnmTexSrt resAnmTexSrt = this_->mRes.GetResAnmTexSrt(l_switch_anim_name[this_->mSwitchType]);
        this_->mAnmTexSrt.create(mdl, resAnmTexSrt, &this_->mAllocator, nullptr, 1);
        this_->mAnmTexSrt.setAnm(this_->mModel, resAnmTexSrt, 0, m3d::FORWARD_ONCE);
        this_->mModel.setAnm(this_->mAnmTexSrt, 1.0f);
    } else if (this_->mProfName != fProfile::EN_PALACE_SWITCH) {
        nw4r::g3d::ResAnmClr resAnmClr = this_->mRes.GetResAnmClr("blink");
        this_->mAnmMatClr.create(mdl, resAnmClr, &this_->mAllocator, nullptr, 1);
        this_->mModel.setAnm(this_->mAnmMatClr);
        this_->mAnmMatClr.setAnm(this_->mModel, resAnmClr, 0, m3d::FORWARD_LOOP);
        this_->mAnmMatClr.setFrame(0.0f, 0);
    }

    this_->mAllocator.adjustFrmHeap();
    this_->mAnmChr.setAnm(this_->mModel, resAnmChr, m3d::FORWARD_ONCE);
    this_->mModel.setAnm(this_->mAnmChr, 1.0f);
}

// Patch out the CLR animation for switch palace switches
kmBranchDefAsm(0x80a1ac08, 0x80a1ac20) {
    cmplwi r0, 0x4
    bne dontDoClrAnim

    lhz r12, 0x8(r3)
    cmplwi r12, PALACE_SWITCH_ID
    beq dontDoClrAnim

    lwzu r12, 0x5e8(r3)
    lwz r12, 0x14(r12)
    mtspr CTR, r12
    bctrl

    dontDoClrAnim:
    blr
}

extern "C" void noScoreAddSwitch(void);

kmBranchDefAsm(0x80a1af90, 0x80a1af94) {
    nofralloc
    lhz r12, 0x8(r30)
    cmplwi r12, PALACE_SWITCH_ID
    bne notPalace
    b noScoreAddSwitch

    notPalace:
    lbz r6, 0x8a8(r30)
    blr
}

extern "C" void activateSwitchPalace(dActor_c *swich);

void activateSwitchPalace(dActor_c *swich) {
    int palaceType = swich->mParam >> 1 & 7;
    if (dInfo_c::m_instance->mWmSwitch & (1 << palaceType)) {
        dInfo_c::m_instance->mWmSwitch &= ~(1 << palaceType);
    } else {
        dInfo_c::m_instance->mWmSwitch |= (1 << palaceType);
    }

    mVec2_c soundPos = dAudio::cvtSndObjctPos(swich->mPos);
    dAudio::g_pSndObjMap->startSound(STRM_BGM_ALL_CMPLT_5STARS, soundPos, 1);

    SndSceneMgr::sInstance->stopBgm(1);

    dMsgBoxWindow_c::m_instance->showMessage(1 + palaceType, false, 270, true);
}

// Activate switch palace flags when pressed
kmBranchDefAsm(0x80a1b0a4, 0x80a1b100) {
    cmplwi r0, 0x4
    bne noActivation
    lwz r12, 0x4(r30)
    mr r3, r30
    bl activateSwitchPalace
    li r31, 0

    noActivation:
    blr
}

// Change the "activation type" for the switch manager when a ! Switch is pressed
kmBranchDefAsm(0x80a1b0c8, 0x80a1b0cc) {
    // Load mParam of this (r30) into r12
    lwz r12, 0x4(r30)
    // mParam >> 1 & 7
    rlwinm r4, r12, 31, 29, 31
    // Add 1 to differentiate it from P Switch
    addi r4, r4, 1
    blr
}