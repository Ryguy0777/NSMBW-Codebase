#pragma once
#include <propelparts/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <propelparts/koopatlas_config.h>

#include <game/bases/d_base.hpp>
#include <game/bases/d_simple_capture_draw.hpp>
#include <game/bases/d_simple_capture.hpp>
#include <game/bases/d_wm_effect_proc.hpp>
#include <game/mLib/m_allocator.hpp>

class dKpDirector_c : public dBase_c {
public:
    dKpDirector_c() { }

    int create();
    int doDelete();
    int execute();
    int draw();

    void toggleCaptureDisp(bool);

    mHeapAllocator_c mAllocator;
    dWmEffectProc_c *mpEffectProc;
    dSimpleCapture_c *mpSimpleCapture;
    dSimpleCaptureDraw_c *mpSimpleCaptureDraw;

    int mRenderModeDelay;
    bool mDoRenderCapture;
};
#endif