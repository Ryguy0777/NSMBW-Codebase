#include <kamek.h>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_w_camera.hpp>

#include <new/world_map_config.h>
#include <new/bases/d_wm_cull_csv_data.hpp>

// Setup culling info
kmBranchDefCpp(0x808E3160, NULL, void, daWmMap_c *this_) {
    // Load culling info for this world
    dWmCullCsvData_c::m_instance.init();
    dWmCullCsvData_c::m_instance.readData();

    for (int i = 0; i < 30; i++) {
        const char *name = dWmCullCsvData_c::m_instance.mNodes[i].mNodeName;
        if (!strcmp(name, "") || this_->currIdx != 0) {
            this_->mCullNodeIdx[i] = -1;
        } else {
            nw4r::g3d::ResNode n = this_->mModels[this_->currIdx].mModel.getResMdl().GetResNode(name);
            nw4r::g3d::ResNode *node = &n;
            if (node != nullptr) {
                this_->mCullNodeIdx[i] = node->GetID();
            } else {
                // We fix a Nintendo bug here:
                // Normally this is set to 0 if the bone doesn't exist. However, bone 0 would
                // be the first index in the model, which causes the ENTIRE map to be culled instead
                this_->mCullNodeIdx[i] = -1;
            }
        }
    }
}

// Process culling
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
        this_->mCullSphere.mRadius = dWmCullCsvData_c::m_instance.mNodes[i].mRadius;

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


// Remove enemy ambushes

#ifdef REMOVE_W1_GOOMBA
kmWriteNop(0x808DFE70);
kmWriteNop(0x808DFE74);
#endif

#ifdef REMOVE_W2_ANTLION
kmWriteNop(0x808E0130);
#endif

#ifdef REMOVE_W3_BROS
kmWriteNop(0x808DFE88);
kmWriteNop(0x808DFE8C);
#endif

#ifdef REMOVE_W4_FISH
kmWriteNop(0x808dfe78);
kmWriteNop(0x808dfe7c);
#endif

#ifdef REMOVE_W5_PIRANHA
kmWriteNop(0x808DFE80);
kmWriteNop(0x808DFE84);
#endif

#ifdef REMOVE_W6_BULLET
kmWrite32(0x808D5210, 0x4E800020);
#endif

#ifdef REMOVE_W7_LAKITU
kmWriteNop(0x808DFE90);
kmWriteNop(0x808DFE94);
#endif

#ifdef REMOVE_W8_BUBBLE
kmWrite32(0x808CA570, 0x4E800020);
#endif

// Do not construct Koopalings

#ifdef REMOVE_KOOPALINGS
kmWrite32(0x808F7100, 0x4E800020);
#endif

// Remove world-specific map actors

#ifdef REMOVE_W2_PALM
kmWriteNop(0x808E1C70);
kmWriteNop(0x808E1C74);
#endif

#ifdef REMOVE_W4_PALM
kmWriteNop(0x808E1C78);
kmWriteNop(0x808E1C7C);
#endif

#ifdef REMOVE_W5_WIGGLER
kmWriteNop(0x808E1D38);
#endif

#ifdef REMOVE_W5_VINES
kmWriteNop(0x808E1D88);
#endif

#ifdef REMOVE_W6_CLOUD
kmWriteNop(0x808E1DA4);
#endif

#ifdef REMOVE_W7_CLOUDS
kmWriteNop(0x808E1CF4);
kmWriteNop(0x808E1D10);
#endif

#ifdef REMOVE_W7_VINES
kmWriteNop(0x808E1CD0);
#endif

// Expand the map model heap size
kmWrite32(0x808DF15C, 0x3C800004);
