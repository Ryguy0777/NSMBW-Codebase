#include <kamek.h>
#include <new/bases/d_a_spawner.hpp>
#include <game/bases/d_switch_flag_mng.hpp>
#include <new/bases/d_custom_profile.hpp>

// To make a new profile id
// Call one of the 4 profile macros from f_profile.hpp to make the profile itself
CUSTOM_ACTOR_PROFILE(AC_SPAWNER, daSpawner_c, fProfile::EN_BOYON, fProfile::DRAW_ORDER::DUMMY_ACTOR, 0);

// If a stage actor is the new actor, create spritedata for it 
const dActorData_c l_SPAWNER_actor_data = {fProfile::AC_SPAWNER, 8, -16, 0, 0, 0x18, 0x18, 0, 0, 0, 0, 0};
// Make a new object of dCustomProfile_c, the ctor call for it will assign profile data on game boot
dCustomProfile_c l_SPAWNER_profile(&g_profile_AC_SPAWNER, "AC_SPAWNER", CourseActor::AC_SPAWNER, &l_SPAWNER_actor_data);

// You can even assign multiple profile/sprite ids to the same profile!
const dActorData_c l_MULTI_SPAWNER_actor_data = {fProfile::AC_MULTI_SPAWNER, 8, -16, 0, 0, 0x18, 0x18, 0, 0, 0, 0, 0};
dCustomProfile_c l_MULTI_SPAWNER_profile(&g_profile_AC_SPAWNER, "AC_MULTI_SPAWNER", CourseActor::AC_MULTI_SPAWNER, &l_MULTI_SPAWNER_actor_data);

// Third actor spawner variant
const dActorData_c l_SPAWNER_PAIRED_actor_data = {fProfile::AC_SPAWNER_PAIRED, 8, -16, 0, 0, 0x18, 0x18, 0, 0, 0, 0, 0};
dCustomProfile_c l_SPAWNER_PAIRED_profile(&g_profile_AC_SPAWNER, "AC_SPAWNER_PAIRED", CourseActor::AC_SPAWNER_PAIRED, &l_SPAWNER_PAIRED_actor_data);

int daSpawner_c::execute() {
    u16 eventIDs = (mEventNums[0] << 8) | mEventNums[1];
    mEventID = eventIDs >> 10 & 0b111111;
    // We use the event ids to store the profile id, so we can reserve the settings
    // This does limit the amount of spawnable actors from 0-1023, but I don't think we'll hit 1023 profiles
    mChildProfileID = eventIDs & 0b1111111111;
    // Adjust profile id!!
    #if GAME_REVISION >= GAME_REVISION_C
    if (mChildProfileID > fProfile::LOGO_SCREEN) {
        mChildProfileID += 2;
    }
    if (mChildProfileID > fProfile::MULTI_COURSE_SELECT) {
        mChildProfileID += 2;
    }
    #elif GAME_REVISION >= GAME_REVISION_K
    if (mChildProfileID > fProfile::MULTI_COURSE_SELECT) {
        mChildProfileID += 2;
    }
    #endif
    // If triggering event id is set
    if (dSwitchFlagMng_c::checkEvent(mEventID)) {
        if (mProfName == fProfile::AC_SPAWNER) {
            // Regular spawner
            if (mHasChild == false) {
                // Spawn child actor if it hasn't spawned one already
                construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
                mHasChild = true;
            }
        } else if (mProfName == fProfile::AC_MULTI_SPAWNER) {
            // Multi spawner
            if (mTimer < 1) {
                // Spawn child actor every 120 frames
                construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
                mTimer = 120;
            }
            mTimer--;
        } else if (mProfName == fProfile::AC_SPAWNER_PAIRED) {
            // Paired spawner
            if (mHasChild == false) {
                // Actor has not been created yet
                dActor_c *child = construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
                // Store child ID for deletion purposes
                mChildID = child->mUniqueID;
                mHasChild = true;
            }
        }
    } else {
        if (mProfName == fProfile::AC_SPAWNER_PAIRED) {
            // Delete the child actor now that the event is disabled
            dActor_c *child = (dActor_c *)fManager_c::searchBaseByID(mChildID);
            if (child) {
                child->deleteActor(0);
            }
        }
        mHasChild = false;
        mTimer = 0;
        mChildID = BASE_ID_NULL;
    }

    return SUCCEEDED;
}