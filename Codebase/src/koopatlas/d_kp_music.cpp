#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED

#include <constants/sound_list.h>
#include <game/snd/snd_audio_mgr.hpp>
#include <new/bases/koopatlas/d_kp_music.hpp>
#include <nw4r/db/exception.h>
#include <revolution/DVD.h>

#ifdef KP_MUSIC_REPORT
#define MusicReport OSReport
#else
#define MusicReport(...)
#endif

#ifdef KP_MUSIC_DEBUGPRINT
#define MusicException nw4r::db::Exception_Printf_
#else
#define MusicException(...)
#endif

#define FADE_OUT_LEN 30
#define FADE_IN_LEN 30
#define BUFFER_CLEAR_DELAY 60

extern "C" void AxVoice_SetADPCM(void *axVoice, void *adpcm);
extern "C" void Voice_SetADPCMLoop(void *voice, int channel, void *adpcmLoop);

u8 hijackMusicWithSongName(const char *songName, int themeID, bool hasFast, int channelCount, int trackCount, int *wantRealStreamID);

dKpMusic_c *dKpMusic_c::m_instance = nullptr;

nw4r::snd::SoundHandle dKpMusic_c::s_streamHandle;
nw4r::snd::SoundHandle dKpMusic_c::s_starHandle;
dDvd::loader_c dKpMusic_c::s_adpcmInfoLoader;

bool dKpMusic_c::create() {
    if (dKpMusic_c::m_instance == nullptr) {
        dKpMusic_c::m_instance = new dKpMusic_c;
    }
    return s_adpcmInfoLoader.request("/MapADPCMInfo.bin", 0, nullptr);
}

dKpMusic_c::dKpMusic_c() {
    mIsPlaying = false;
    mIsStarPlaying = false;
    mCurrTrack = -1;
    mNextTrack = -1;
    mSwitchDelay = -1;
    mFadeInDelay = -1;
}

void dKpMusic_c::startBgmTrack(int trackID) {
    // Switch track
    if (mIsPlaying) {
        MusicReport("Trying to switch to song %d (Current one is %d)...\n", trackID, mCurrTrack);
        if ((mCurrTrack == trackID && mNextTrack == -1) || mNextTrack == trackID) {
            MusicReport("This song is already playing or is scheduled. Not gonna do that.\n");
        } else if (mSwitchDelay >= 0) {
            MusicReport("We were already going to switch tracks, but the rstm hasn't been changed yet, so the next song is being changed to this one\n");
            mNextTrack = trackID;
        } else {
            MusicReport("Will switch; Fading out current track 2 over %d frames\n", FADE_OUT_LEN);

            if (s_streamHandle.IsAttachedSound()) {
                s_streamHandle.SetVolume(0.0f, FADE_OUT_LEN);
            }
            mNextTrack = trackID;
            mSwitchDelay = FADE_OUT_LEN;
        }

    // New track
    } else {
        MusicReport("Playing song %d from the start.\n", trackID);

        int realStreamID;
        char brstmName[8];
        sprintf(brstmName, "map%d", trackID);
        hijackMusicWithSongName(brstmName, -1, false, 4, 2, &realStreamID);

        SndAudioMgr::sInstance->startSound(&s_streamHandle, realStreamID, 1);

        mIsPlaying = true;
        mCurrTrack = trackID;
    }
}

void dKpMusic_c::execute() {
    if (!mIsPlaying) {
        return;
    }

    /*if (s_streamHandle.detail_GetAttachedSound() == nullptr) {
       MusicException("SOUND IS NOT PLAYING!\n");
       return;
    }*/

    if (mSwitchDelay >= 0) {
        mSwitchDelay--;

        if (mSwitchDelay == 0) {
            MusicException("Switching brstm files to song %d.\n", mNextTrack);

            char brstmPath[48];
            sprintf(brstmPath, "/Sound/new/map%d.er", mNextTrack);

            u8 *sound = (u8*)(s_streamHandle.detail_GetAttachedSound());
            u8 *player = sound+0x110;
            u8 **fileStreamPointer = (u8**)(player+0x808);
            u8 *fileStream = *fileStreamPointer;
            DVDFileInfo *fileInfo = (DVDFileInfo*)(fileStream+0x28);

            if (fileInfo->block.state == 1) {
                MusicReport("Was reading chunk, will try again next frame...\n");
                mSwitchDelay++;
                return;
            }

            if (mNextTrack > 0) {
                mFadeInDelay = BUFFER_CLEAR_DELAY;
            }

            DVDCancel(&fileInfo->block);
            //MusicReport("CANCEL successfully called!\n");
            bool result = DVDFastOpen(DVDConvertPathToEntrynum(brstmPath), fileInfo);

            MusicException("StrmSound is at %p, StrmPlayer is at %p, FileStream pointer is at %p, FileStream is at %p, FileInfo is at %p\n", sound, player, fileStreamPointer, fileStream, fileInfo);
            MusicException("Changed to name %s. FastOpen returned: %d\n", brstmPath, result);

            u8 *trackArray = player+0xB58;
            u8 *track = (trackArray + (0x38 * 1));
            u8 **voicePointer = (u8**)(track+4);
            u8 *voice = *voicePointer;
            MusicException("Track Array: %p; Track: %p; Voice Pointer: %p; Voice: %p\n", trackArray, track, voicePointer, voice);

            for (int i = 0; i < 2; i++) {
                int sourceBlockID = (mNextTrack*2)+i;
                u8 *sourceData = ((u8*)(s_adpcmInfoLoader.GetBuffer())) + (0x30*sourceBlockID);
                MusicException("Using ADPCM data for channel %d from block %d, data at %p\n", i, sourceBlockID, sourceData);

                if (voice == nullptr) {
                    continue;
                }

                Voice_SetADPCMLoop(voice, i, sourceData+0x28);

                // loop through all axVoices
                for (int j = 0; j < 4; j++) {
                    int axVoiceID = (i*4) + j;
                    u8 **axVoicePointer = (u8**)(voice + 0xC + (axVoiceID*4));
                    u8 *axVoice = *axVoicePointer;
                    MusicException("Setting AxVoice ID %d, with pointer at %p, located at %p\n", axVoiceID, axVoicePointer, axVoice);

                    if (axVoice) {
                        AxVoice_SetADPCM(axVoice, sourceData);
                    }
                }
            }

            MusicReport("All done\n");

            mCurrTrack = mNextTrack;
            mNextTrack = -1;
        }

    } else if (mFadeInDelay >= 0) {
        mFadeInDelay--;

        if (mFadeInDelay == 0) {
            MusicReport("Going to fade in the second track now!\n");
            if (s_streamHandle.IsAttachedSound()) {
                s_streamHandle.SetVolume(1.0f, FADE_IN_LEN);
            }
        }
    }
}

void dKpMusic_c::pauseAllSound(bool flag) {
    if (s_streamHandle.IsAttachedSound()) {
        s_streamHandle.Pause(flag, 15);
    }

    if (s_starHandle.IsAttachedSound()) {
        s_starHandle.Pause(flag, 15);
    }
}

void dKpMusic_c::stopAllSound() {
    if (!mIsPlaying) {
        return;
    }

    MusicReport("Stopping song\n");
    mIsPlaying = false;
    mCurrTrack = -1;
    mNextTrack = -1;
    mSwitchDelay = -1;
    mFadeInDelay = -1;

    if (s_streamHandle.IsAttachedSound()) {
        s_streamHandle.Stop(30);
    }

    if (s_starHandle.IsAttachedSound()) {
        s_starHandle.Stop(15);
    }
}

void dKpMusic_c::onStarSe() {
    if (mIsStarPlaying) {
        return;
    }

    SndAudioMgr::sInstance->startSound(&s_starHandle, SE_BGM_CS_STAR, 1);
    mIsStarPlaying = true;
}

void dKpMusic_c::offStarSe() {
    if (!mIsStarPlaying) {
        return;
    }

    s_starHandle.Stop(15);
    mIsStarPlaying = false;
}

void dKpMusic_c::setBgmVolume(float volume) {
    if (!s_streamHandle.IsAttachedSound()) {
        return;
    }

    s_streamHandle.SetVolume(volume, 15);
}

void dKpMusic_c::setStarMode(STAR_MODE_e mode) {
    if (!s_starHandle.IsAttachedSound()) {
        return;
    }

    if (mode == MODE_PAUSE) {
        s_starHandle.detail_GetAttachedSound()->Pause(true, 15);
        return;
    }

    float volume = (mode == MODE_STOCK_ITEM) ? 4.0f : 1.5f;

    s_starHandle.SetVolume(volume, 15);
    s_starHandle.detail_GetAttachedSound()->Pause(false, 15);
}
#endif