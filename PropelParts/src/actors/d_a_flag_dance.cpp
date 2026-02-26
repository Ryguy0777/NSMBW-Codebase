#include <kamek.h>
#include <propelparts/bases/d_a_flag_dance.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_switch_flag_mng.hpp>
#include <game/bases/d_audio.hpp>

CUSTOM_ACTOR_PROFILE(AC_FLAG_DANCE, daFlagDance_c, fProfile::AC_FLAGON, fProfile::DRAW_ORDER::AC_FLAGON, 0);

const dActorData_c c_FLAG_DANCE_actor_data = {fProfile::AC_FLAG_DANCE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ACTOR_CREATE_GLOBAL};
dCustomProfile_c l_FLAG_DANCE_profile(&g_profile_AC_FLAG_DANCE, "AC_FLAG_DANCE", CourseActor::AC_FLAG_DANCE, &c_FLAG_DANCE_actor_data);

int daFlagDance_c::create() {
    mDanceType = mParam & 0xF;

	return SUCCEEDED;
}

int daFlagDance_c::execute() {
	bool dance = dAudio::isBgmAccentSign(mDanceType);
	bool flagActiveState = dSwitchFlagMng_c::checkEvent(mEventNums[0]-1);
	if (dance) {
		dSwitchFlagMng_c::m_instance->set(mEventNums[0]-1, 0, !flagActiveState, false, false, false);
	}
	return SUCCEEDED;
}