#include <kamek.h>

#include <new/game_config.h>
#include <game/bases/d_course_select_manager.hpp>

// Use regular save behavior
#ifdef DISABLE_QUICK_SAVE
kmWriteNop(0x8092FD08);
#endif

// Override retail layouts with Newer equivalents
#if defined(NEWER_STAR_COINS_MENU) || defined(NEWER_MAP_HUD)
#include <new/bases/koopatlas/d_kp_star_coin_menu.hpp>
#include <new/bases/koopatlas/d_kp_hud.hpp>

// Create new layouts (SCM replaces COLLECTION_COIN, so it doesn't need to be created manually)
kmBranchDefCpp(0x80926E50, 0x80926E54, dCourseSelectManager_c *, u16 profID, fBase_c *wm, ulong param, u8 group) {
#ifdef NEWER_MAP_HUD
    dKPHud_c *hud = (dKPHud_c*)fBase_c::createChild(fProfile::KP_HUD, wm, 0, 0);
#endif
    return (dCourseSelectManager_c*)fBase_c::createChild(profID, wm, param, group);
}

// Check if layouts are finished creating
kmBranchDefCpp(0x80927174, 0x80927178, bool) {
    bool ret = true;

#ifdef NEWER_STAR_COINS_MENU
    ret &= dKPStarCoinMenu_c::m_instance->mHasLayoutLoaded;
#endif

#ifdef NEWER_MAP_HUD
    ret &= dKPHud_c::m_instance->mLayoutLoaded;
#endif
    return ret;
}

#ifdef NEWER_STAR_COINS_MENU
// Remove original SCM check
kmWriteNop(0x8092710C);

// Skip creating COLLECTION_COIN children
kmBranchDefCpp(0x80926EAC, 0x80926F24, void) { return; }

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

#endif