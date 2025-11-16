#pragma once
#include <nw4r/snd.h>

class SndAudioMgr {
public:
    void startSystemSe(unsigned int soundID, unsigned long);

    void startSound(nw4r::snd::SoundHandle *handle, unsigned int soundID, unsigned long);

public:
    static SndAudioMgr *sInstance;
};
