#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <game/bases/d_world_select_guide.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

kmBranchDefCpp(0x8092B7B0, NULL, void, dWorldSelectGuide_c *this_) {
    if (this_->mCurrentWorld == this_->mNewWorld)
        return;

    this_->mCurrentWorld = this_->mNewWorld;

    const wchar_t *str = getWorldNumber(this_->mCurrentWorld);
    this_->mpTextBoxes[dWorldSelectGuide_c::T_worldNum_00]->SetString(str, 0);
}
#endif