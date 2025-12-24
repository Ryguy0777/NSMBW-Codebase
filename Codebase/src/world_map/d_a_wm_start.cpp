#include <kamek.h>
#include <game/bases/d_a_wm_start.hpp>
#include <game/bases/d_res_mng.hpp>

kmBranchDefCpp(0x808E80B0, NULL, void, daWmStart_c *this_) {
    this_->mAllocator.createFrmHeap(0xFFFFFFFF, mHeap::g_gameHeaps[0], nullptr, 0x20);

    this_->mResFile = dResMng_c::m_instance->mRes.getRes("cobStart", "g3d/model.brres");
    nw4r::g3d::ResMdl mdl = this_->mResFile.GetResMdl("cobStart");
    this_->mModel.create(mdl, &this_->mAllocator, 0x20, 1, nullptr);

    nw4r::g3d::ResAnmTexPat resTexPat = this_->mResFile.GetResAnmTexPat("cobStart");
    this_->mAnmTexPat.create(mdl, resTexPat, &this_->mAllocator, nullptr, 1);
    this_->mModel.setAnm(this_->mAnmTexPat);
    this_->mAnmTexPat.setRate(0.0f, 0);

    // Add support for down/right nodes
    int patFrame = ((this_->mParam >> 0x10) & 0xF);
    if (patFrame > 3) {
        patFrame = 0;
    }
    this_->mAnmTexPat.setFrame(patFrame, 0);

    dWmActor_c::setSoftLight_MapObj(this_->mModel);
    this_->mAllocator.adjustFrmHeap();
}

extern "C" void getKinokoHouseNodePos__11daWmStart_cF7mVec3_c(mVec3_c *);

kmBranchDefAsm(0x808E79D0, 0x808E7A3C) {
    mr r3, r31
    addi r4, r1, 0x10
    bl getKinokoHouseNodePos__11daWmStart_cF7mVec3_c
    blr
}

kmBranchDefAsm(0x808E7B50, 0x808E7BC0) {
    addi r4, r1, 0x10
    bl getKinokoHouseNodePos__11daWmStart_cF7mVec3_c
    blr
}

void daWmStart_c::getKinokoHouseNodePos(mVec3_c out) {
    char *nodeName;
    u32 type = (mParam >> 0x10) & 0xF;
    switch (type) {
        default: nodeName = "s0"; break;
        case 1: nodeName = "s1"; break;
        case 3: nodeName = "s1"; break;
    }

    nw4r::g3d::ResMdl mdl = mModel.getResMdl();
    u32 nodeID = m3d::getNodeID(mdl, nodeName);
    nw4r::math::VEC3 nodePos;
    mModel.getNodeWorldMtxMultVecZero(nodeID, nodePos);

    out.x = nodePos.x;
    out.y = nodePos.y;
    out.z = nodePos.z;
}