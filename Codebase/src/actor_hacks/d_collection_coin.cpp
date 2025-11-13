#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <game/bases/d_collection_coin.hpp>
#include <new/bases/d_level_info.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

// World name (while menu is stationary)
kmBranchDefCpp(0x80774B60, NULL, void, dCollectionCoin_c *this_) {
    const wchar_t *worldName = getWorldName(this_->mCurrentWorld);

    this_->mpTextBoxes[dCollectionCoin_c::T_world_00]->SetString(worldName, 0);
    this_->mpTextBoxes[dCollectionCoin_c::T_world_01]->SetString(worldName, 0);
    return;
}

// World name (during page scroll)
kmBranchDefCpp(0x80774C10, NULL, void, dCollectionCoin_c *this_, int textBoxIdx) {
    const wchar_t *worldName = getWorldName(this_->mCurrentWorld);

    this_->mpTextBoxes[dCollectionCoin_c::T_world_00 + textBoxIdx]->SetString(worldName, 0);
    return;
}

extern "C" void collectionCoinDate_loadInfo(dCollectionCoinDate_c *);

// Course info hack
kmBranchDefAsm(0x80776BB0, 0x80776CE4) {
    or r3, r30, r30
    bl collectionCoinDate_loadInfo
    blr
}

void collectionCoinDate_loadInfo(dCollectionCoinDate_c *this_) {
    this_->mpRootPane->SetVisible(this_->mLevelNumber != 42);

    dLevelInfo_c::entry_s *level = dLevelInfo_c::m_instance.getEntryFromSlotID(this_->mWorldNumber, this_->mLevelNumber);
    if (level) {
        const wchar_t *worldName = getWorldNumber(level->mDisplayWorld);
        ulong number = getLevelNumberIdx(level->mDisplayWorld, level->mDisplayLevel, true);
        MsgRes_c *msgRes = dMessage_c::getMesRes();

        this_->mpTextBoxes[dCollectionCoinDate_c::T_worldNum_00]->SetString(worldName, 0);
        if (level->mDisplayLevel > 19) {
            this_->mpTextBoxes[dCollectionCoinDate_c::T_corseNum_00]->SetVisible(false);
            this_->mpTextBoxes[dCollectionCoinDate_c::T_pictureFont_00]->SetVisible(true);
            this_->mpTextBoxes[dCollectionCoinDate_c::T_pictureFont_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        } else {
            this_->mpTextBoxes[dCollectionCoinDate_c::T_pictureFont_00]->SetVisible(false);
            this_->mpTextBoxes[dCollectionCoinDate_c::T_corseNum_00]->SetVisible(true);
            this_->mpTextBoxes[dCollectionCoinDate_c::T_corseNum_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        }
    } else {
        this_->mpTextBoxes[dCollectionCoinDate_c::T_worldNum_00]->SetString(getWorldNumber(0), 0);
        this_->mpTextBoxes[dCollectionCoinDate_c::T_pictureFont_00]->SetVisible(false);
        this_->mpTextBoxes[dCollectionCoinDate_c::T_corseNum_00]->SetVisible(true);
        this_->mpTextBoxes[dCollectionCoinDate_c::T_corseNum_00]->SetString(getLevelNumber(0), 0);
    }
}
#endif