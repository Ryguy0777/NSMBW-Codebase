#include <kamek.h>
#include <new/bases/d_a_flag_dance.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_switch_flag_mng.hpp>
#include <game/bases/d_audio.hpp>

CUSTOM_ACTOR_PROFILE(AC_FLAG_DANCE, daFlagDance_c, fProfile::AC_FLAGON, fProfile::DRAW_ORDER::AC_FLAGON, 0);

const SpriteData bahpEventSpriteData = {fProfile::AC_FLAG_DANCE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x2};
dCustomProfile_c bahpEventProfile(&g_profile_AC_FLAG_DANCE, "AC_FLAG_DANCE", SpriteId::AC_FLAG_DANCE, &bahpEventSpriteData);

int daFlagDance_c::create() {
    mDanceType = mParam & 0xF;

	return SUCCEEDED;
}

int daFlagDance_c::execute() {
	bool dance = dAudio::isBgmAccentSign(mDanceType);
	bool flagActiveState = checkEvent((mEventNums & 0xFF)-1);
	if (dance) {
		dSwitchFlagMng_c::m_instance->set((mEventNums & 0xFF)-1, 0, !flagActiveState, false, false, false);
	}
	return SUCCEEDED;
}