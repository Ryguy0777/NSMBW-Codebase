#pragma once
#include <new/game_config.h>

#include <revolution/GX/GXTypes.h>

class dWorldInfo_c {
public:
    struct world_s {
        u8 mMapListIdx;
        u8 mWorldNameMsgID;
        u8 mLevelInfoID;
        u8 mMusicID;
        u8 mLastLevelID;
        u8 pad[3]; // Making room for any future additions

        GXColor mFileTextColors[2];
        GXColor mFileHintColors[2];
        GXColor mHudTextColors[2];

        s16 mHudHue;
        s8 mHudSat;
        s8 mHudLight;

        u16 mTitleWorldID;
        u16 mTitleLevelID;

        u32 mLightNameOffset;
    };

    struct header_s {
        u32 mMagic;
        u8  mShopDatas[10][12];
        u32 mWorldCount;
        world_s mWorlds[];
    };

private:
    header_s *mpData;

public:
    world_s *getWorld(u32 worldNumber) {
        return &mpData->mWorlds[worldNumber];
    }

    const char *getLightName(u32 worldNumber) {
        return (const char*)((u32)mpData + mpData->mWorlds[worldNumber].mLightNameOffset);
    }

    static bool loadWorldInfo();

    static dWorldInfo_c m_instance;
};