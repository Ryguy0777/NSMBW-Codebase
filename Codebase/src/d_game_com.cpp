#include <game/bases/d_game_com.hpp>

int dGameCom::getUnspentStarCoinCount() {
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    int coinsSpent = save->mSpentStarCoins;
    return dGameCom::getStarCoinCount() - coinsSpent;
}

int dGameCom::getStarCoinCount() {
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    int coinsEarned = 0;

    for (int w = 0; w < 10; w++) {
        for (int l = 0; l < 42; l++) {
            u32 conds = save->getCourseDataFlag(w, l);

            if (conds & dMj2dGame_c::COIN1_COLLECTED) { coinsEarned++; }
            if (conds & dMj2dGame_c::COIN2_COLLECTED) { coinsEarned++; }
            if (conds & dMj2dGame_c::COIN3_COLLECTED) { coinsEarned++; }
        }
    }

    return coinsEarned;
}