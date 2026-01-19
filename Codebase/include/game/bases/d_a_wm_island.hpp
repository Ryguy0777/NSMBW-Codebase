#pragma once

#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>

/// @unofficial
class daWmIsland_c : public dBaseActor_c {
public:
    enum AnmType_e {
        TYPE_SRT = 0,
        TYPE_CLR,
    };

    void setupAnmTexSrt();
    void setupAnmClr();
    void setSignLanguage();

    void createAnm();
    bool chkAnmExist(int type);
    void executeAnm();

    dHeapAllocator_c mAllocator;
    m3d::mdl_c mModel;
    m3d::anmChr_c mAnmChr;
    m3d::anmTexSrt_c mAnmTexSrt;
    m3d::anmMatClr_c mAnmMatClr;
    m3d::anmTexPat_c mAnmTexPat;
    nw4r::g3d::ResFile mResFile;
    u8 mPad[0x38];

    int mWorldNo;
    u8 mPad2[0x34];

    float mCurrAnmChrRate;
    float mCurrAnmSrtRate;
    float mCurrAnmClrRate;
    u8 mPad3[0x14];

    // New
    bool mHasAnmSrt;
    bool mHasAnmClr;
};