#include <kamek.h>
#include <game/mLib/m_3d.hpp>
#include <game/bases/d_cd.hpp>
#include <game/bases/d_s_stage.hpp>

// Fix the graphical issues when using Jellybeam outside of darkness zones
kmBranchDefCpp(0x80A2EDF0, NULL, void, void *this_) {
    bool visible = (bool)((int)(this_) + 0x194);
	m3d::mdl_c *lightMask = (m3d::mdl_c*)((int)(this_) + 0x20);
	m3d::mdl_c *lightDisp = (m3d::mdl_c*)((int)(this_) + 0x60);
	m3d::mdl_c *lightFog = (m3d::mdl_c*)((int)(this_) + 0xA0);

	if (visible) {
        u8 courseIdx = dScStage_c::m_instance->mCurrCourse;
        u8 areaIdx = dScStage_c::m_instance->mCurrArea;

		// only draw lightmask if we're in darkness
        u8 mask = dCd_c::m_instance->mFiles[courseIdx].getAreaDataP(areaIdx, nullptr)->mMaskNo;
		if (mask & 0x20) {
			lightMask->entry();
		}
		lightDisp->entry();
		lightFog->entry();
	}
}