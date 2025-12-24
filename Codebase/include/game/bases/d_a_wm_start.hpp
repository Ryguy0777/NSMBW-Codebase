#pragma once

#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>

class daWmStart_c : public dWmDemoActor_c {
public:
    void getKinokoHouseNodePos(mVec3_c out);

    u32 mNodeParam;
    int _188;

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;

    m3d::smdl_c mModel;
    m3d::anmTexPat_c mAnmTexPat;

    void *mpKinokoHouse; // TODO: idk
    void *mpCourse; // TODO: daWmCourse_c

    bool mIsKinokoAppear;

    u8 mPad[3];

    mVec3_c _1F0;

    int mKinokoAppearDelay;
    bool mIsKinokoKindSet;
    u8 mPad2[3];
};