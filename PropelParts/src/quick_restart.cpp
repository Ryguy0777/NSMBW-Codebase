#include <kamek.h>
#include <new/game_config.h>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_info.hpp>

// Restart the level on death, unless the player is pressing 2/A
#ifdef QUICK_RESTART

kmCallDefCpp(0x8013D8EC, void) {
    dScStage_c::m_exitMode = (dScStage_c::Exit_e)1;

    // Go to Game Over regardless if anyone is out of lives
    if(((!daPyMng_c::mRest[0]) || 
        ((daPyMng_c::mPlayerEntry[1]) && !daPyMng_c::mRest[1]) ||
        ((daPyMng_c::mPlayerEntry[2]) && !daPyMng_c::mRest[2]) ||
        ((daPyMng_c::mPlayerEntry[3]) && !daPyMng_c::mRest[3]))) {
        dScStage_c::setNextScene(fProfile::GAME_OVER, 0, (dScStage_c::Exit_e)1, (dFader_c::fader_type_e)2);
        return;
    }

    // Return to worldmap if 2/A pressed OR in Coin Battle/Free Play
    int pressed = dGameKey_c::m_instance->mRemocon[0]->mDownButtons;
    if (pressed & (WPAD_BUTTON_2 | WPAD_BUTTON_A) ||
        dInfo_c::mGameFlag & (dInfo_c::GAME_FLAG_IS_COIN_COURSE | dInfo_c::GAME_FLAG_4)) {
        dScStage_c::setNextScene(fProfile::WORLD_MAP, 0, (dScStage_c::Exit_e)1, (dFader_c::fader_type_e)2);
        return;
    }

    dFader_c::setFader((dFader_c::fader_type_e)2);
    dInfo_c::getInstance()->startGame(dInfo_c::m_startGameInfo);
}

#endif