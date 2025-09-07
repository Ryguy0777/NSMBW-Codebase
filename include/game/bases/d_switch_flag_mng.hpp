#pragma once
#include <types.h>

class dSwitchFlagMng_c {
public:
    u64 mFlags;
    u8 mPad[0x6C8];

    static dSwitchFlagMng_c* m_instance;
};