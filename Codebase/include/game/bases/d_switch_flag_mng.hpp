#pragma once
#include <types.h>

class dSwitchFlagMng_c {
public:
    void set(u32 eventId, int timer, bool activateEvent, bool resetEvent, bool isTimed, bool musicInterruption);

    u64 mFlags;
    u8 mPad[0x6C8];

    static dSwitchFlagMng_c* m_instance;
};

inline bool checkEvent(u8 event) {return (dSwitchFlagMng_c::m_instance->mFlags & (u64)(1 << event));}