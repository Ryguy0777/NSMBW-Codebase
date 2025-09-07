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
    // we use the event ids to store the profile id, so we can reserve the settings
    // this does limit the amount of spawnable actors from 0-1023, but I don't think we'll hit 1023 profiles
    mChildProfileID = mEventNums & 0b1111111111;
    // if triggering event id is set and it has not spawned anything
    if (checkEvent(mEventID)) {
        if (mHasChild == false) {
            // spawn child actor
            // adjust profile id!!
            #if GAME_REVISION >= GAME_REVISION_C
            if (mChildProfileID > fProfile::WII_STRAP) {
                mChildProfileID += 3;
            }
            #elif GAME_REVISION >= GAME_REVISION_K
            if (mChildProfileID > fProfile::MULTI_COURSE_SELECT) {
                mChildProfileID += 2;
            }
            #endif
            construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
            mHasChild = true;
        }
    } else {
        mHasChild = false;
    }
    return SUCCEEDED;
}