#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>

// custom shyguy enemy

class daEnHeiho_c : public dEn_c {
public:
    daEnHeiho_c() : mRes(nullptr) {};
    ~daEnHeiho_c() {};

    virtual int create();
    virtual int execute();
    virtual int draw();

    virtual void finalUpdate();

    virtual void createModel();
    virtual void initialize();

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mRes;
    m3d::mdl_c mHeihoModel;

};

const s16 l_look_angles[2] = {0x2000, 0xE000};