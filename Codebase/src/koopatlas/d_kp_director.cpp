#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_kp_director.hpp>
#include <game/bases/d_info.hpp>

dKPDirector_c *dKPDirector_c_classInit() {
    dKPDirector_c *c = new dKPDirector_c;
    return c;
}

// Replace WM_DIRECTOR actor
kmWritePointer(0x80987BD8, &dKPDirector_c_classInit);

int dKPDirector_c::create() {
    mAllocator.createFrmHeapToCurrent(-1, EGG::Heap::sCurrentHeap, nullptr, 0x20, mHeap::OPT_0);

    mpSimpleCapture = new dSimpleCapture_c;
    mpSimpleCapture->create(&mAllocator, -1, 0x82);

    mpSimpleCaptureDraw = new dSimpleCaptureDraw_c;
    mpSimpleCaptureDraw->create(&mAllocator);

    mpEffectProc = new dWmEffectProc_c;
    mpEffectProc->create(&mAllocator, -1, 0x80, -1);

    mAllocator.adjustFrmHeapRestoreCurrent();

    mDoRenderCapture = false;
    return true;
}

int dKPDirector_c::doDelete() {
    mpSimpleCapture->destroy();
    return true;
}

int dKPDirector_c::execute() {
    if (mRenderModeDelay == 0) {
        if (mDoRenderCapture) {
            dInfo_c::m_instance->mIsWorldSelect = 1;
            mDoRenderCapture = false;
        }
    } else {
        mRenderModeDelay--;
    }
    return true;
}

int dKPDirector_c::draw() {
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
    return true;
}

void dKPDirector_c::toggleCaptureDisp(bool value) {
    mDoRenderCapture = value;
    if (value) {
        mRenderModeDelay = 1;
    } else {
        dInfo_c::m_instance->mIsWorldSelect = false;
    }
}

#endif