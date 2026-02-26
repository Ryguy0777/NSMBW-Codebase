#include <kamek.h>
#include <new/bases/d_a_effect_spawner.hpp>
#include <new/bases/d_custom_profile.hpp>
#include <game/bases/d_switch_flag_mng.hpp>
#include <game/bases/d_audio.hpp>
#include <game/mLib/m_effect.hpp>

CUSTOM_ACTOR_PROFILE(AC_EFFECT_SPAWNER, daEffectSpawner_c, fProfile::LIFT_TORIDE_ROLL, fProfile::DRAW_ORDER::LIFT_TORIDE_ROLL, 0);

const dActorData_c c_EFFECT_SPAWNER_actor_data = {fProfile::AC_EFFECT_SPAWNER, 8, -16, 0, 0, 24, 24, 0, 0, 0, 0, 0};
dCustomProfile_c l_EFFECT_SPAWNER_profile(&g_profile_AC_EFFECT_SPAWNER, "AC_EFFECT_SPAWNER", CourseActor::AC_EFFECT_SPAWNER, &c_EFFECT_SPAWNER_actor_data);

int daEffectSpawner_c::create() {
    mPlayEffect = (mParam >> 16) & 1;
    mEffectIndex = mParam & 0xFFF;
    mScale = float((mParam >> 20) & 0xF) / 4.0f;
    mDelay = ((mParam >> 12) & 0xF) * 30;

    if (mScale == 0.0f) {
        mScale = 1.0f;
    }

    return SUCCEEDED;
}

int daEffectSpawner_c::execute() {
    if (dSwitchFlagMng_c::checkEvent(mEventNums[1]-1)) {
        if (mTimer == mDelay) {
            if (mPlayEffect == 0) {
                mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
                dAudio::g_pSndObjEmy->startSound(mEffectIndex, soundPos, 0);
            } else {
                mVec3_c effectScale = mVec3_c(mScale, mScale, mScale);
                mEf::createEffect(c_effect_spawner_list[mEffectIndex], 0, &mPos, nullptr, &effectScale);
            }

            mTimer = 0;
            if (mDelay == 0) {
                mDelay = -1;
            }
        }
        mTimer++;
    }

    return SUCCEEDED;
}