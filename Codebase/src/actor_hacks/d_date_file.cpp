#include <kamek.h>

#include <new/game_config.h>

// disable the disabling of the fileselect stars
// if the player has died more than 8 times in a level

#ifdef NO_SUPER_GUIDE
kmWriteNop(0x8077dd2c);
#endif

// Newer File Select screen

#ifdef NEWER_FILE_SELECT

#include <game/bases/d_date_file.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/bases/d_game_com.hpp>
#include <new/bases/d_level_info.hpp>
#include <new/bases/d_world_info.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

// Change layout name
const char *newerDateFileName = "dateFile_newer/dateFile.arc";
kmBranchDefAsm(0x8077CF54, 0x8077CF58) {
    lis r4, newerDateFileName@h
    ori r4, r4, newerDateFileName@l
    lwz r4, 0(r4)
    blr
}

// Externs for ASM calls
extern "C" void setFileNum(dDateFile_c *);
extern "C" void setFileInfo(dDateFile_c *, dMj2dGame_c *);

kmBranchDefAsm(0x8077D044, 0x8077D048) {
    mr r3, r30
    bl setFileNum
    blr
}

void setFileNum(dDateFile_c *this_) {
    int fileNum = this_->mFileNum;

    MsgRes_c *msgRes = dMessage_c::getMesRes();
    this_->mpTextBoxes[this_->T_fileNumber_01]->setMessage(msgRes, BMG_CATEGORY_NEWER_FILE_SELECT, fileNum, 0);
}

kmBranchDefAsm(0x8077DA10, 0x8077DA14) {
    mr r3, r30
    mr r4, r31
    bl setFileInfo
    blr
}

struct FileInfo {
    FileInfo() : mCoinCount(0), mExitCount(0) {}

    int mCoinCount;
    int mExitCount;
};
extern "C" FileInfo *GetFileInfo(FileInfo *out, dMj2dGame_c *save) {
    out->mCoinCount = 0;
    out->mExitCount = 0;

#ifdef LEVEL_INFO_ENABLED
    for (int i = 0; i < dLevelInfo_c::m_instance.sectionCount(); i++) {
        dLevelInfo_c::section_s *section = dLevelInfo_c::m_instance.getSection(i);

        for (int j = 0; j < section->mLevelCount; j++) {
            dLevelInfo_c::entry_s *entry = &section->mLevels[j];
			if (entry->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL) {
                u32 cond = save->getCourseDataFlag(entry->mWorldSlot, entry->mLevelSlot);
                if ((entry->mFlag & 0x10) && (cond & dMj2dGame_c::GOAL_NORMAL))
                    out->mExitCount++;
                if ((entry->mFlag & 0x20) && (cond & dMj2dGame_c::GOAL_SECRET))
                    out->mExitCount++;
                if (cond & dMj2dGame_c::COIN1_COLLECTED)
                    out->mCoinCount++;
                if (cond & dMj2dGame_c::COIN2_COLLECTED)
                    out->mCoinCount++;
                if (cond & dMj2dGame_c::COIN3_COLLECTED)
                    out->mCoinCount++;
            }
        }
    }
#else
    for (int i = 0; i < WORLD_COUNT; i++) {
        for (int j = 0; j < STAGE_COUNT; j++) {
            if ((j > STAGE_CASTLE_2 && j < STAGE_CANNON) || j == STAGE_COIN_BATTLE || j == STAGE_UNK37 || j > STAGE_DOOMSHIP)
                continue;
            u32 cond = save->getCourseDataFlag(i, j);
            if (cond & dMj2dGame_c::GOAL_NORMAL)
                out->mExitCount++;
            if (cond & dMj2dGame_c::GOAL_SECRET)
                out->mExitCount++;
            if (cond & dMj2dGame_c::COIN1_COLLECTED)
                out->mCoinCount++;
            if (cond & dMj2dGame_c::COIN2_COLLECTED)
                out->mCoinCount++;
            if (cond & dMj2dGame_c::COIN3_COLLECTED)
                out->mCoinCount++;
        }
    }
#endif

    //OSReport("Done, got %d coins and %d exits\n", out->mCoinCount, out->mExitCount);
    return out;
}

void setFileInfo(dDateFile_c *this_, dMj2dGame_c *save) {
#ifdef KOOPATLAS_DEV_ENABLED
    int worldIdx = save->mWorldInfoIdx;
#else
    int worldIdx = save->getCurrentWorld();
#endif

    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(worldIdx);

#ifdef KOOPATLAS_DEV_ENABLED 
    const wchar_t *str = getKoopatlasWorldName(world->mWorldNameMsgID);
#else
    const wchar_t *str = getWorldName(worldIdx);
#endif
    this_->mpTextBoxes[this_->T_worldNumber_01]->SetString(str, 0);

    this_->mpTextBoxes[this_->T_worldNumber_01]->SetTextColor(0, world->mFileTextColors[0]);
    this_->mpTextBoxes[this_->T_worldNumber_01]->SetTextColor(1, world->mFileTextColors[1]);

    // Recolor the background
    nw4r::lyt::Picture *Picture_00;
    Picture_00 = this_->mLayout.findPictureByName("Picture_00");

    for (int i = 0; i < 2; i++) {
        Picture_00->SetVtxColor(0+i, world->mFileHintColors[0]);
        Picture_00->SetVtxColor(2+i, world->mFileHintColors[1]);
    }

    FileInfo info;
    GetFileInfo(&info, save);

    LytTextBox_c *StarCoinCount, *ExitCount;
    StarCoinCount = (LytTextBox_c *)this_->mLayout.findTextBoxByName("StarCoinCount");
    ExitCount = (LytTextBox_c *)this_->mLayout.findTextBoxByName("ExitCount");

    dGameCom::LayoutDispNumberDigit(info.mCoinCount, StarCoinCount, 0);
    dGameCom::LayoutDispNumberDigit(info.mExitCount, ExitCount, 0);

    // Newer code sets X size to 100, since somebody was too lazy to edit the layout
    nw4r::lyt::Size newSize(100.0f, 38.40f);
    StarCoinCount->SetSize(newSize);
}

#endif