#pragma once
#include <types.h>

class dPSwManager_c {
public:
    enum SwType_e {
        P_SWITCH,
        RED_SWITCH,
    };
    
    dPSwManager_c();
    virtual ~dPSwManager_c();

    bool checkMove();
    u32 checkSwitch(dPSwManager_c::SwType_e);

    u32 mSwitchState;
    int mSwitchTimers[3];

    static dPSwManager_c *ms_instance;
};