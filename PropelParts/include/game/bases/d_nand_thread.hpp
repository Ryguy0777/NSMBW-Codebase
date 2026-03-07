#pragma once

#include <types.h>

class dNandThread_c {
public:
    u8 mPad[0x74];
    int mCurrentState;
    int mCurrentError;

    static dNandThread_c *m_instance;
};