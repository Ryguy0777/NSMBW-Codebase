#include <kamek.h>
#include <game/bases/d_a_wm_island.hpp>
#include <game/bases/d_res_mng.hpp>

extern "C" void createAnm__12daWmIsland_cFv(void);
extern "C" void executeAnm__12daWmIsland_cFv(void);

// Expand class size by 0x4
kmWrite32(0x808D33E8, 0x386002D8);

kmBranchDefAsm(0x808D3D4C, 0x808D3E00) {
    or r3, r28, r28
    bl createAnm__12daWmIsland_cFv
    blr
}

void daWmIsland_c::createAnm() {
    setSignLanguage();

    if (chkAnmExist(TYPE_SRT)) {
        mHasAnmSrt = true;
        setupAnmTexSrt();
    }

    if (chkAnmExist(TYPE_CLR)) {
        mHasAnmClr = true;
        setupAnmClr();
    }
}

bool daWmIsland_c::chkAnmExist(int type) {
    char name[6];
    bool isAnm = false;

    snprintf(name, 6, "WS_W%X", mWorldNo + 1);

    nw4r::g3d::ResFile res = dResMng_c::m_instance->getRes(name, "g3d/model.brres");
    if (type == 0) {
        nw4r::g3d::ResAnmTexSrt resSrt = res.GetResAnmTexSrt(name);
        if (resSrt.ptr() != nullptr) {
            isAnm = true;
        }
    } else {
        nw4r::g3d::ResAnmClr resClr = res.GetResAnmClr(name);
        if (resClr.ptr() != nullptr) {
            isAnm = true;
        }
    }

    return isAnm;
}

// Execute
kmBranchDefAsm(0x808D378C, 0x808D3844) {
    or r3, r31, r31
    bl executeAnm__12daWmIsland_cFv
    blr
}

void daWmIsland_c::executeAnm() {
    mModel.play();

    if (mHasAnmSrt) {
        mAnmTexSrt.play();
    }

    if (mHasAnmClr) {
        mAnmMatClr.play();
    }
}

// Pause anims
kmBranchDefCpp(0x808D4260, NULL, void, daWmIsland_c *this_) {
    if (this_->mHasAnmSrt) {
        this_->mCurrAnmSrtRate = this_->mAnmTexSrt.getRate(0);
        this_->mAnmTexSrt.setRate(0.0f, 0);
    }

    if (this_->mHasAnmClr) {
        this_->mCurrAnmClrRate = this_->mAnmMatClr.getRate(0);
        this_->mAnmMatClr.setRate(0.0f, 0);
    }

    this_->mCurrAnmChrRate = this_->mAnmChr.getRate();   
    this_->mAnmChr.setRate(0.0f);
}

// Restore anms
kmBranchDefCpp(0x808D43A0, NULL, void, daWmIsland_c *this_) {
    if (this_->mHasAnmSrt) {
        this_->mAnmTexSrt.setRate(this_->mCurrAnmSrtRate, 0);
    }

    if (this_->mHasAnmClr) {
        this_->mAnmMatClr.setRate(this_->mCurrAnmClrRate, 0);
    }

    this_->mAnmChr.setRate(this_->mCurrAnmChrRate);
}