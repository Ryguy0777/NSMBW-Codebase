#include <kamek.h>
#include <new/bases/d_a_flag_invert.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_switch_flag_mng.hpp>

CUSTOM_ACTOR_PROFILE(AC_FLAG_INVERT, daFlagInvert_c, fProfile::AC_FLAGON, fProfile::DRAW_ORDER::AC_FLAGON, 0);

const SpriteData c_FLAG_INVERT_spritedata = {fProfile::AC_FLAG_INVERT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x2};
dCustomProfile_c l_FLAG_INVERT_profile(&g_profile_AC_FLAG_INVERT, "AC_FLAG_INVERT", SpriteId::AC_FLAG_INVERT, &c_FLAG_INVERT_spritedata);

int daFlagInvert_c::execute() {
    u8 eventId1 = (mEventNums >> 8 & 0xFF)-1;
    u8 eventId2 = (mEventNums & 0xFF)-1;
	bool target = dSwitchFlagMng_c::checkEvent(eventId2);
	bool flagActiveState = dSwitchFlagMng_c::checkEvent(eventId1);
	if (target != mPreviousFlagState) {
        switch (mParam & 0b11) {
            default:
                dSwitchFlagMng_c::m_instance->set(eventId1, 0, !flagActiveState, false, false, false);
                break;
            case 1:
                if (target && !mPreviousFlagState) {
                    // Only when event is enabled
                    dSwitchFlagMng_c::m_instance->set(eventId1, 0, !flagActiveState, false, false, false);
                }
                break;
            case 2:
                if (!target && mPreviousFlagState) {
                    // Only when event is disabled
                    dSwitchFlagMng_c::m_instance->set(eventId1, 0, !flagActiveState, false, false, false);
                }
                break;
        }
	}
    mPreviousFlagState = target;
	return SUCCEEDED;
}