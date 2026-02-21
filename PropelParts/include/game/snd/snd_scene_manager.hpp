#pragma once
#include <types.h>

class SndSceneMgr {
public:
    void enterHBM();
    void exitHBM();

    void stopBgm(u32);

    u8 mPad1[0x10];
    int m_10;
    int m_14;

    static SndSceneMgr *sInstance;
};
