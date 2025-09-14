#include <kamek.h>
#include <new/bases/d_a_spawner.hpp>
#include <game/bases/d_switch_flag_mng.hpp>
#include <new/new_profile.hpp>

inline bool checkEvent(u8 event) {return (dSwitchFlagMng_c::m_instance->mFlags & (u64)(1 << event));}

const SpriteData ActorSpawnerSpriteData = { fProfile::AC_SPAWNER, 0, 0, 0 , 0, 0x18, 0x18, 0, 0, 0, 0, 0 };
CUSTOM_ACTOR_PROFILE(AC_SPAWNER, daSpawner_c, fProfile::AC_SPAWNER, fProfile::DRAW_ORDER::DUMMY_ACTOR, 0);
dCustomProfile_c CustomActorSpawnerProfile(&g_profile_AC_SPAWNER, "AC_SPAWNER", SpriteId::AC_SPAWNER, &ActorSpawnerSpriteData);

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