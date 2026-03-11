#pragma once
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_3d/smdl.hpp>

// @unofficial
class daWmSwitch_c : public dWmDemoActor_c {
public:
    enum TYPE_e {
        TYPE_RED_W3,

        TYPE_YELLOW,
        TYPE_RED,
        TYPE_GREEN,
        TYPE_BLUE,
    };

    int create();
    int doDelete();
    int execute();
    int draw();

    void createTexPat();
    void setPressState();

    inline bool isSwitchActive(int type) { return (dInfo_c::m_instance->mWmSwitch & (1 << type)) != 0; }

    u32 mMapNodeIdx;
    dHeapAllocator_c mAllocator;
    m3d::smdl_c mModel;
    m3d::anmChr_c mAnmChr;
    m3d::anmTexSrt_c mAnmTexSrt;

    bool mIsPressed;

    m3d::anmTexPat_c *mpAnmTexPat;
    int mSwitchType;
};