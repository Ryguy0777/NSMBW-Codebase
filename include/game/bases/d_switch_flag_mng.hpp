#include <kamek.h>

class dSwitchFlagMng_c {
public:
    u64 mFlags;
    u8 mPad[0x6C8];

    // Unofficial name
    void set(u32 eventId, int timer, bool activateEvent, bool resetEvent, bool isTimed, bool musicInterruption);

    static dSwitchFlagMng_c* m_instance;
};