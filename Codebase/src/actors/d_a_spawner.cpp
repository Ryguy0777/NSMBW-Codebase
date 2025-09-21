#include <kamek.h>
#include <new/bases/d_a_spawner.hpp>
#include <game/bases/d_switch_flag_mng.hpp>
#include <new/new_profile.hpp>

// to make a new profile id
// call one of the 4 profile macros from f_profile.hpp to make the profile itself
CUSTOM_ACTOR_PROFILE(AC_SPAWNER, daSpawner_c, fProfile::EN_BOYON, fProfile::DRAW_ORDER::DUMMY_ACTOR, 0);

// if a stage actor is the new actor, create spritedata for it 
const SpriteData ActorSpawnerSpriteData = {fProfile::AC_SPAWNER, 8, -16, 0, 0, 0x18, 0x18, 0, 0, 0, 0, 0};
// make a new object of dCustomProfile_c, the ctor call for it will assign profile data on game boot
dCustomProfile_c ActorSpawnerProfile(&g_profile_AC_SPAWNER, "AC_SPAWNER", SpriteId::AC_SPAWNER, &ActorSpawnerSpriteData);

// you can even assign multiple profile/sprite ids to the same profile!
const SpriteData MultiActorSpawnerSpriteData = {fProfile::AC_MULTI_SPAWNER, 8, -16, 0, 0, 0x18, 0x18, 0, 0, 0, 0, 0};
dCustomProfile_c MultiActorSpawnerProfile(&g_profile_AC_SPAWNER, "AC_MULTI_SPAWNER", SpriteId::AC_MULTI_SPAWNER, &MultiActorSpawnerSpriteData);

// third actor spawner variant
const SpriteData PairedActorSpawnerSpriteData = {fProfile::AC_SPAWNER_PAIRED, 8, -16, 0, 0, 0x18, 0x18, 0, 0, 0, 0, 0};
dCustomProfile_c PairedActorSpawnerProfile(&g_profile_AC_SPAWNER, "AC_SPAWNER_PAIRED", SpriteId::AC_SPAWNER_PAIRED, &PairedActorSpawnerSpriteData);

int daSpawner_c::execute() {
    mEventID = mEventNums >> 10 & 0b111111;
    // we use the event ids to store the profile id, so we can reserve the settings
    // this does limit the amount of spawnable actors from 0-1023, but I don't think we'll hit 1023 profiles
    mChildProfileID = mEventNums & 0b1111111111;
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
    // if triggering event id is set
    if (checkEvent(mEventID)) {
        if (mProfName == fProfile::AC_SPAWNER) {
            // regular spawner
            if (mHasChild == false) {
                // spawn child actor if it hasn't spawned one already
                construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
                mHasChild = true;
            }
        } else if (mProfName == fProfile::AC_MULTI_SPAWNER) {
            // multi spawner
            if (mTimer < 1) {
                // spawn child actor every 120 frames
                construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
                mTimer = 120;
            }
            mTimer--;
        } else if (mProfName == fProfile::AC_SPAWNER_PAIRED) {
            // paired spawner
            if (mHasChild == false) {
                // actor has not been created yet
                dActor_c *child = construct(mChildProfileID, mParam, &mPos, &mAngle, mLayer);
                // store child ID for deletion purposes
                mChildID = child->mUniqueID;
                mHasChild = true;
            }
        }
    } else {
        if (mProfName == fProfile::AC_SPAWNER_PAIRED) {
            // delete the child actor now that the event is disabled
            dActor_c *child = (dActor_c *)fManager_c::searchBaseByID(mChildID);
            if (child)
                child->deleteActor(0);
        }
        mHasChild = false;
        mTimer = 0;
        mChildID = BASE_ID_NULL;
    }

    return SUCCEEDED;
}