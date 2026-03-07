#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_kp_director.hpp>
#include <game/bases/d_info.hpp>

dKpDirector_c *dKpDirector_c_classInit() {
    dKpDirector_c *c = new dKpDirector_c;
    return c;
}

// Replace WM_DIRECTOR actor
kmWritePointer(0x80987BD8, &dKpDirector_c_classInit);

int dKpDirector_c::create() {
    mAllocator.createFrmHeapToCurrent(-1, EGG::Heap::sCurrentHeap, nullptr, 0x20, mHeap::OPT_0);

    mpSimpleCapture = new dSimpleCapture_c;
    mpSimpleCapture->create(&mAllocator, -1, 0x82);

    mpSimpleCaptureDraw = new dSimpleCaptureDraw_c;
    mpSimpleCaptureDraw->create(&mAllocator);

    mpEffectProc = new dWmEffectProc_c;
    mpEffectProc->create(&mAllocator, -1, 0x80, -1);

    mAllocator.adjustFrmHeapRestoreCurrent();

    mDoRenderCapture = false;
    return SUCCEEDED;
}

int dKpDirector_c::doDelete() {
    mpSimpleCapture->destroy();
    return SUCCEEDED;
}

int dKpDirector_c::execute() {
    if (mRenderModeDelay == 0) {
        if (mDoRenderCapture) {
            dInfo_c::m_instance->mIsWorldSelect = 1;
            mDoRenderCapture = false;
        }
    } else {
        mRenderModeDelay--;
    }
    return SUCCEEDED;
}

int dKpDirector_c::draw() {
    // Create the capture
    if (mDoRenderCapture) {
        mpSimpleCapture->entry();
    }

    if (!dInfo_c::m_instance->mIsWorldSelect) {
        if (mpEffectProc != nullptr) {
            mpEffectProc->entry();
        }
    } else {
        mpSimpleCaptureDraw->mpCaptureTexture = mpSimpleCapture->mpCaptureTexture;
        mpSimpleCaptureDraw->entry();
    }
    return SUCCEEDED;
}

void dKpDirector_c::toggleCaptureDisp(bool value) {
    mDoRenderCapture = value;
    if (value) {
        mRenderModeDelay = 1;
    } else {
        dInfo_c::m_instance->mIsWorldSelect = false;
    }
}

#endif