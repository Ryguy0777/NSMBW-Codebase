#pragma once
#include <new/game_config.h>

#ifdef LEVEL_INFO_ENABLED
#include <types.h>

class dLevelInfo_c {
public:
    enum ENTRY_FLAG_e {
        FLAG_VALID_LEVEL = 2, // Actual level with Star Coins
        FLAG_SECOND_HALF = 0x400,
    };

    struct header_s {
        u32 mMagic;
        u32 mSectionCount;
        u32 mSections[1];
    };

    struct entry_s {
        u8 mWorldSlot;
        u8 mLevelSlot;
        u8 mDisplayWorld;
        u8 mDisplayLevel;
        u8 mNameStrLen;
        u8 mReserved3;
        u16 mFlag;
        u32 mNameOffset;
    };

    struct section_s {
        u32 mLevelCount;
        entry_s mLevels[1];
    };

private:
    header_s *mpData;

public:
    void init(void *buffer);

    entry_s *getEntryFromSlotID(int world, int level);
    entry_s *getEntryFromDispID(int world, int level);
    entry_s *getLastEntry(u32 sectionIdx);

    u32 sectionCount() {
        return mpData->mSectionCount;
    }

    section_s *getSection(u32 index) {
        return (section_s*)(((char*)mpData) + mpData->mSections[index]);
    }

    section_s *getLastSection() {
        return (section_s*)(((char*)mpData) + mpData->mSections[sectionCount()-1]);
    }

    const char *getLevelName(entry_s *entry) {
        return (const char*)mpData + entry->mNameOffset;
    }

    static bool loadLevelInfo();

    static dLevelInfo_c m_instance;
};
#endif