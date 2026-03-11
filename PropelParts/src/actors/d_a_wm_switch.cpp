#include <kamek.h>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_switch.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_s_world_map_static.hpp>

// Expand daWmSwitch_c + 0x08 bytes
kmWrite16(0x808EF8DA, 0x220);

// Patch model bufferOption for TexPAT
kmWrite16(0x808EFDB2, 0x227);

// Param layout:
// 00AX YYZZ
// A:  [NEW] Switch type
// X:  Toggle mode, 0 for ON, 1 for OFF
// YY: CSV point index
// ZZ: Always 0xFF

extern "C" int atoi(const char* str);
extern "C" void createTexPat__12daWmSwitch_cFv(void);
extern "C" void setPressState__12daWmSwitch_cFv(void);
extern "C" void WmSwitchConstruct__FiPCcPC7mVec3_c(void);

// Create PAT anim
kmBranchDefAsm(0x808EFF04, 0x808EFF0C) {
    mr r3, r28
    bl createTexPat__12daWmSwitch_cFv
    blr
}

void daWmSwitch_c::createTexPat() {
    dWmActor_c::setSoftLight_MapObj(mModel); // Replaced

    nw4r::g3d::ResFile res = dResMng_c::m_instance->getRes("cobHatenaSwitch", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = res.GetResMdl("cobHatenaSwitch");
    nw4r::g3d::ResAnmTexPat resPat = res.GetResAnmTexPat("cobHatenaSwitch");

    mpAnmTexPat = new m3d::anmTexPat_c;

    mpAnmTexPat->create(resMdl, resPat, &mAllocator, 0, 1);
    mpAnmTexPat->setAnm(mModel, resPat, 0, m3d::FORWARD_ONCE);
    mpAnmTexPat->setRate(0.0f, 0);
    mModel.setAnm(*mpAnmTexPat);

    // Set our frame
    static const int sc_patFrames[] = {
        0, // W3 switch
        1, // Yellow
        0, // Red
        2, // Green
        3, // Blue
    };

    mSwitchType = (mParam >> 20) & 0xF;
    if (mSwitchType > 4) {
        mSwitchType = 0;
    }

    mpAnmTexPat->setFrame(sc_patFrames[mSwitchType], 0);
}

// Update the current pressed state of the switch (create and execute)
kmBranchDefAsm(0x808EFA90, 0x808EFAC8) {
    mr r3, r30
    bl setPressState__12daWmSwitch_cFv
    blr
}

kmBranchDefAsm(0x808EFBA4, 0x808EFBDC) {
    mr r3, r30
    bl setPressState__12daWmSwitch_cFv
    blr
}

void daWmSwitch_c::setPressState() {
    int type = (mParam >> 20) & 0xF;
    if (type > 4) {
        type = 0;
    }

    if (((mParam >> 16) & 1) == 0) {
        mIsPressed = isSwitchActive(type);
    } else {
        mIsPressed = !isSwitchActive(type);
    }
}

// Update script execution to account for new switch types
kmBranchDefCpp(0x808EFFF0, NULL, void, daWmSwitch_c *this_, int cutDataType, int unk) {
    if (cutDataType == -1) {
        return;
    }

    if (((unk != 0) && (cutDataType == 94)) && this_->mIsPressed) {
        if (this_->mSwitchType == daWmSwitch_c::TYPE_RED_W3 && (dScWMap_c::m_WorldNo == WORLD_3 && dScWMap_c::m_SceneNo != 0)) {
            int scene = daWmMap_c::m_instance->currIdx;
            bool active = this_->isSwitchActive(daWmSwitch_c::TYPE_RED_W3);
            daWmMap_c::m_instance->mModels[scene].playSwitchAnim(active, false);
        }
    }

    if (cutDataType == 94) {
        if (this_->mSwitchType == daWmSwitch_c::TYPE_RED_W3 && (dScWMap_c::m_WorldNo == WORLD_3 && dScWMap_c::m_SceneNo != 0)) {
            int scene = daWmMap_c::m_instance->currIdx;
            if (daWmMap_c::m_instance->mModels[scene].isSwitchAnimStop()) {
                this_->setCutEnd();
            }
        } else {
            this_->setCutEnd();
        }
    } else {
        this_->setCutEnd();
    }
}

// Modify WM_SWITCH actor creation to get switch type
kmBranchDefAsm(0x808E1974, 0x808E19A8) {
    mr r3, r4
    mr r4, r31
    addi r5, r1, 0xC
    bl WmSwitchConstruct__FiPCcPC7mVec3_c
    blr
}

daWmSwitch_c *WmSwitchConstruct(int pointIdx, const char *pNodeName, const mVec3_c *pos) {
    int flagType = atoi(&pNodeName[3]); // Toggle mode

    char nameTypePrm = pNodeName[2];
    int switchType = atoi(&nameTypePrm);

    // Actually dWmActor_c::construct, however that's inaccessible and it just calls this anyways
    return (daWmSwitch_c*)dBaseActor_c::construct(fProfile::WM_SWITCH, (switchType << 20) | (flagType << 16) | (pointIdx << 8) | 0xFF, pos, nullptr);
}

// Load WM_SWITCH in every world
// TODO: Once WorldMap branch is merged, move this into the global reslist
kmBranchDefCpp(0x80926D80, NULL, bool, bool ret) {
    dResMng_c::m_instance->setRes("WorldMap", "cobHatenaSwitch", nullptr);
    return ret;
}

// Delete the switch when unloading the map
// TODO: Same as above
kmBranchDefCpp(0x80927AB4, NULL, bool, bool ret) {
    dResMng_c::m_instance->getResP()->deleteRes("cobHatenaSwitch");
    return ret;
}

// (daWmPlayer_c) Toggle flag when a switch is hit
kmBranchDefCpp(0x809072A0, NULL, bool, void *, int pointIdx) {
    daWmSwitch_c *pSwitch = (daWmSwitch_c*)dWmLib::SearchMapObjFromCsvIndex(fProfile::WM_SWITCH, pointIdx);
    if (pSwitch == nullptr) {
        return false;
    }

    if (((pSwitch->mParam >> 16) & 1) == 0) {
        dInfo_c::m_instance->mWmSwitch |= (1 << pSwitch->mSwitchType);
    } else {
        dInfo_c::m_instance->mWmSwitch &= ~(1 << pSwitch->mSwitchType);
    }

    dCsSeqMng_c::ms_instance->FUN_801017c0(42, nullptr, nullptr, 0x80); // SMC_DEMO_SWITCH
    return true;
}
