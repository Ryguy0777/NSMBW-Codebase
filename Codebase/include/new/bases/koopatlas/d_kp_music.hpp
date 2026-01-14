#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/koopatlas_config.h>

#include <new/custom_music.hpp>
#include <nw4r/snd.h>

class dKPMusic_c {
public:
    dKPMusic_c();

    void start(int id);
    void startStarSe();

    void execute();
    void pause();
    void stop();

    void updTrackVolume(bool isOpenMenu);
    void updStarVolume(int);

    static bool init();

    bool mIsPlaying;
    bool mIsStarPlaying;

    int mCurrentSongID;
    int mNextSongID;

    int mSwitchDelay;
    int mFadeInDelay;

    static dKPMusic_c *m_instance;
};
#endif