#include <kamek.h>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_w_camera.hpp>

// Patch size (0x6C510 + 0x4)
kmWrite32(0x808DEB90, 0x3863C514);

// Create culling CSV
kmBranchDefCpp(0x808DF334, 0x808DF338, void, daWmMap_c *this_) {
    this_->mpCullData = new dWmCullCsvData_c;

    this_->SetCullNodeIndex();
}

void daWmMap_c::SetCullNodeIndex() {
    for (int i = 0; i < 30; i++) {
        const char *name = mpCullData->mNodes[i].mNodeName;
        if (!strcmp(name, "") || currIdx != 0) {
            mCullNodeIdx[i] = -1;
        } else {
            nw4r::g3d::ResNode n = mModels[currIdx].mModel.getResMdl().GetResNode(name);
            nw4r::g3d::ResNode *node = &n;
            if (node != nullptr) {
                mCullNodeIdx[i] = node->GetID();
            } else {
                // We fix a Nintendo bug here:
                // Normally this is set to 0 if the bone doesn't exist. However, bone 0 would
                // be the first index in the model, which causes the ENTIRE map to be culled instead
                mCullNodeIdx[i] = -1;
            }
        }
    }
}

kmBranchDefCpp(0x808E3230, NULL, void, daWmMap_c *this_) {
    for (int i = 0; i < 30; i++) {
        if (this_->mCullNodeIdx[i] < 0) {
            continue;
        }

        mVec3_c nodePos;
        this_->GetNodePos(this_->mCullNodeIdx[i], nodePos);
    
        this_->mCullSphere.mCenter.x = nodePos.x;
        this_->mCullSphere.mCenter.y = nodePos.y;
        this_->mCullSphere.mCenter.z = nodePos.z;
        this_->mCullSphere.mRadius = this_->mpCullData->mNodes[i].mRadius;

        nw4r::g3d::ResMdl mdl = this_->mModels[this_->currIdx].mModel.getResMdl();
        nw4r::g3d::ResNode n = mdl.GetResNode(this_->mCullNodeIdx[i]);
        nw4r::g3d::ResNode *node = &n;

        if (node == nullptr) {
            continue;
        }

        bool visible = dWCamera_c::m_instance->mViewClip.CheckClipSphere(&this_->mCullSphere);
        this_->mModels[this_->currIdx].mModel.cullNode(this_->mCullNodeIdx[i], visible, true);
    }
}
