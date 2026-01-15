#include <kamek.h>

#include <new/game_config.h>
#include <game/bases/d_course_select_manager.hpp>

// Use regular save behavior
#ifdef DISABLE_QUICK_SAVE
kmWriteNop(0x8092FD08);
#endif

// Override retail Star Coins Menu with Newer's
#ifdef NEWER_STAR_COINS_MENU
#include <new/bases/koopatlas/d_kp_star_coin_menu.hpp>

// Create SCM
kmBranchDefCpp(0x80926E9C, 0x80926F24, void, int, fBase_c *wm) {
    dKPStarCoinMenu_c *scm = (dKPStarCoinMenu_c*)fBase_c::createChild(fProfile::KP_STAR_COIN_MENU, wm, 0, 0);
    if (scm != nullptr) {
        OSReport("Created SCM @ %p\n", scm);
    }
}

// Remove original SCM check
kmWriteNop(0x8092710C);

// Check if SCM is finished creating
kmBranchDefCpp(0x80927174, 0x80927178, bool) {
    return dKPStarCoinMenu_c::m_instance->mHasLayoutLoaded;
}

// Wait for SCM to be exited (executeState_CollectionCoinWait)
kmBranchDefCpp(0x8092FEE0, NULL, void, dCourseSelectManager_c *this_) {
    if (dKPStarCoinMenu_c::m_instance->mIsVisible) {
        return;
    }
    this_->changeToNormalState();
}

// Open SCM
kmBranchDefCpp(0x8092FC74, 0x8092FD60, int, int worldNum) {
    dKPStarCoinMenu_c::m_instance->dispMenu(worldNum);
    dCourseSelectManager_c::m_instance->mStateMgr.changeState(dCourseSelectManager_c::StateID_CollectionCoinWait);
    return 0;
}
#endif