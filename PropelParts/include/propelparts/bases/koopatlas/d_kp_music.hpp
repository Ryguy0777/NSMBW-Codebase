#pragma once
#include <propelparts/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <propelparts/koopatlas_config.h>

#include <propelparts/custom_music.hpp>
#include <game/bases/d_dvd.hpp>
#include <nw4r/snd.h>

class dKpMusic_c {
public:
    enum STAR_MODE_e {
        MODE_DEFAULT,
        MODE_PAUSE,
        MODE_STOCK_ITEM,
    };

    dKpMusic_c();

    void startBgmTrack(int trackID);

    void execute();
    void pauseAllSound(bool flag);
    void stopAllSound();

    void onStarSe();
    void offStarSe();

    void setBgmVolume(float volume);
    void setStarMode(STAR_MODE_e mode);

    inline void onMenuVolume() { setBgmVolume(0.3f); }
    inline void offMenuVolume() { setBgmVolume(1.0f); }

    static bool create();

    bool mIsPlaying;
    bool mIsStarPlaying;

    int mCurrTrack;
    int mNextTrack;

    int mSwitchDelay;
    int mFadeInDelay;

    static dKpMusic_c *m_instance;
    static nw4r::snd::SoundHandle s_streamHandle;
    static nw4r::snd::SoundHandle s_starHandle;

    static dDvd::loader_c s_adpcmInfoLoader;
};
#endif