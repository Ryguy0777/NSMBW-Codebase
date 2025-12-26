#pragma once

#include <types.h>

class dWmMapModel_c {
public:
    u8 mPad[0x8];
    m3d::bmdl_c mModel;
    u8 mPad2[0xbe4];
};
