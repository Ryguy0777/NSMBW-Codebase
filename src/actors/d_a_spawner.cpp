#include <kamek.h>
#include <new/bases/d_a_spawner.hpp>
#include <game/bases/d_switch_flag_mng.hpp>

inline bool checkEvent(u8 event) {return (dSwitchFlagMng_c::m_instance->mFlags & (u64)(1 << event));}

daSpawner_c *daSpawner_c::build() {
    return new daSpawner_c;
};

// TEMPORARY replace sprite 88 with our actor spawner
// remove when profile expansion is implemented
kmWritePointer(0x80ADEA00, &daSpawner_c::build);

int daSpawner_c::execute() {
    mEventID = mEventNums >> 10 & 0b111111;
    mChildProfileID = mEventNums & 0b1111111111;
    // if triggering event id is set and it has not spawned anything
    if (checkEvent(mEventID)) {
        if (mHasChild == false) {
            // spawn child actor
            construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
            mHasChild = true;
        }
    } else {
        mHasChild = false;
    }
    return SUCCEEDED;
}