#pragma once

#include <nw4r/ut/ut_charStrmReader.h>

// debugging config ini from NSMASRv2

struct LaunchType {
    enum Value {
        Normal,
        ToStage,
        ToMovie,
        ToWorldmap,
    };
};

struct LaunchGameMode {
    enum Value {
        Normal,
        FreePlay,
        CoinBattle,
        SuperGuideReplay,
        HintMovieReplay,
        TitleReplay,
    };
};

struct ColliderDisplayFlags {
    enum Value {
        None,
        Hitboxes,
        Colliders,
        Sensors,
        RideableColliders,
    };
};

struct DebugKey {
    enum KeyId {
        LaunchType,
        SaveNumber,
        PlayerCount,
        PowerUp,
        HasStar,
        World,
        Level,
        Area,
        Entrance,
        GameMode,
        HintMovieType,
        ShowPregame,
        DrawHitboxes,
        DrawColliders,
        DrawSensors,
        DrawRideableColliders,
        MovieId,
        ActorLog,
    };

    KeyId keyId;
    char* keyName;
};

class dDebugConfig_c {
    public:
        dDebugConfig_c();

        void loadConfig();
        void parseConfig(nw4r::ut::CharStrmReader* reader, void* bufferEnd);
        void parseConfigLine(char* key, char* param, int paramSize);

        static void setupConfig();

        // Generic
        u8 mLaunchType;
        u8 mSaveNo;
        u8 mPlayerCount;
        u8 mPowerUp;
        bool mStar;

        // Levels only
        u8 mWorld;
        u8 mLevel;
        u8 mArea;
        u8 mEntrance;
        u8 mGameMode;
        u8 mHintMovieType;
        bool mShowPregame;
        u8 mCollisionDebugFlags;

        // Movie only
        u8 mMovieId;

        // Debug only
        bool mActorLog;

        // Static instance
        static dDebugConfig_c* m_instance;
};
