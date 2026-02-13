#include <kamek.h>
#include <game/bases/d_a_en_hnswich.hpp>

// Add support for colored ! Switches that fill in associated switch blocks
// Nybbles 12.0-12.3 in Reggie are now used as a "switch type" 

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

    this_->mRes = dResMng_c::m_instance->getRes(l_switch_res_name[this_->mSwitchType], l_switch_brres_name[this_->mSwitchType]);

    // Load a colored switch model if switch type is 2
    nw4r::g3d::ResMdl mdl;
    if (this_->mSwitchType != 2) {
        mdl = this_->mRes.GetResMdl(l_switch_model_name[this_->mSwitchType]);
    } else {
        int switchColor = this_->mParam >> 1 & 7;
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
    } else {
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