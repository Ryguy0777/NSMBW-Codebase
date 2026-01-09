#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED

#include <constants/sound_list.h>
#include <game/bases/d_dvd.hpp>
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

static nw4r::snd::SoundHandle sStreamHandle;
static nw4r::snd::SoundHandle sStarHandle;

static dDvd::loader_c s_adpcmInfoLoader;

#define FADE_OUT_LEN 30
#define FADE_IN_LEN 30
#define BUFFER_CLEAR_DELAY 60

extern "C" void AxVoice_SetADPCM(void *axVoice, void *adpcm);
extern "C" void Voice_SetADPCMLoop(void *voice, int channel, void *adpcmLoop);

u8 hijackMusicWithSongName(const char *songName, int themeID, bool hasFast, int channelCount, int trackCount, int *wantRealStreamID);

dKPMusic_c *dKPMusic_c::m_instance = nullptr;

bool dKPMusic_c::init() {
    if (dKPMusic_c::m_instance == nullptr) {
        dKPMusic_c::m_instance = new dKPMusic_c;
    }
    return s_adpcmInfoLoader.request("/MapADPCMInfo.bin", 0, nullptr);
}

dKPMusic_c::dKPMusic_c() {
    mIsPlaying = false;
    mIsStarPlaying = false;
    mCurrentSongID = -1;
    mNextSongID = -1;
    mSwitchDelay = -1;
    mFadeInDelay = -1;
}

void dKPMusic_c::start(int id) {
    if (mIsPlaying) {
        // Switch track
        MusicReport("Trying to switch to song %d (Current one is %d)...\n", id, mCurrentSongID);
        if ((mCurrentSongID == id && mNextSongID == -1) || mNextSongID == id) {
            MusicReport("This song is already playing or is scheduled. Not gonna do that.\n");
        //} else if (mFadeInDelay >= 0) {
        //	MusicReport("There's already a track being faded in (CountdownToFadeIn=%d)\n", mFadeInDelay);
        } else if (mSwitchDelay >= 0) {
            MusicReport("We were already going to switch tracks, but the rstm hasn't been changed yet, so the next song is being changed to this one\n");
            mNextSongID = id;
        } else {
            MusicReport("Will switch; Fading out current track 2 over %d frames\n", FADE_OUT_LEN);

            if (sStreamHandle.IsAttachedSound())
                sStreamHandle.SetVolume(0.0f, FADE_OUT_LEN);
            mNextSongID = id;
            mSwitchDelay = FADE_OUT_LEN;
        }

    } else {
        // New track
        MusicReport("Playing song %d from the start.\n", id);

        int realStreamID;
        char brstmName[8];
        sprintf(brstmName, "map%d", id);
        hijackMusicWithSongName(brstmName, -1, false, 4, 2, &realStreamID);

        SndAudioMgr::sInstance->startSound(&sStreamHandle, realStreamID, 1);

        mIsPlaying = true;
        mCurrentSongID = id;
    }
}

void dKPMusic_c::startStarSe() {
    if (mIsStarPlaying)
        return;

    SndAudioMgr::sInstance->startSound(&sStarHandle, SE_BGM_CS_STAR, 1);
    mIsStarPlaying = true;
}

void dKPMusic_c::execute() {
    if (!mIsPlaying)
        return;

    /*if (sStreamHandle.detail_GetAttachedSound() == nullptr) {
       MusicException("SOUND IS NOT PLAYING!\n");
       return;
    }*/

    if (mSwitchDelay >= 0) {
        mSwitchDelay--;
        if (mSwitchDelay == 0) {
            MusicException("Switching brstm files to song %d.\n", mNextSongID);

            char brstmPath[48];
            sprintf(brstmPath, "/Sound/new/map%d.er", mNextSongID);

            u8 *sound = (u8*)(sStreamHandle.detail_GetAttachedSound());
            u8 *player = sound+0x110;
            u8 **fileStreamPointer = (u8**)(player+0x808);
            u8 *fileStream = *fileStreamPointer;
            DVDFileInfo *fileInfo = (DVDFileInfo*)(fileStream+0x28);

            if (fileInfo->block.state == 1) {
                MusicReport("Was reading chunk, will try again next frame...\n");
                mSwitchDelay++;
                return;
            }

            if (mNextSongID > 0)
                mFadeInDelay = BUFFER_CLEAR_DELAY;

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
                int sourceBlockID = (mNextSongID*2)+i;
                u8 *sourceData = ((u8*)(s_adpcmInfoLoader.GetBuffer())) + (0x30*sourceBlockID);
                MusicException("Using ADPCM data for channel %d from block %d, data at %p\n", i, sourceBlockID, sourceData);

                if (!voice)
                    continue;

                Voice_SetADPCMLoop(voice, i, sourceData+0x28);

                // loop through all axVoices
                for (int j = 0; j < 4; j++) {
                    int axVoiceID = (i*4) + j;
                    u8 **axVoicePointer = (u8**)(voice + 0xC + (axVoiceID*4));
                    u8 *axVoice = *axVoicePointer;
                    MusicException("Setting AxVoice ID %d, with pointer at %p, located at %p\n", axVoiceID, axVoicePointer, axVoice);

                    if (axVoice)
                        AxVoice_SetADPCM(axVoice, sourceData);
                }
            }

            MusicReport("All done\n");

            mCurrentSongID = mNextSongID;
            mNextSongID = -1;
        }

    } else if (mFadeInDelay >= 0) {
        mFadeInDelay--;
        if (mFadeInDelay == 0) {
            MusicReport("Going to fade in the second track now!\n");
            if (sStreamHandle.IsAttachedSound())
                sStreamHandle.SetVolume(1.0f, FADE_IN_LEN);
        }
    }
}

void dKPMusic_c::pause() {
    if (sStreamHandle.IsAttachedSound())
        sStreamHandle.Pause(true, 15);

    if (sStarHandle.IsAttachedSound())
        sStarHandle.Pause(true, 15);
}

void dKPMusic_c::stop() {
    if (!mIsPlaying)
        return;

    MusicReport("Stopping song\n");
    mIsPlaying = false;
    mCurrentSongID = -1;
    mNextSongID = -1;
    mSwitchDelay = -1;
    mFadeInDelay = -1;

    if (sStreamHandle.IsAttachedSound())
        sStreamHandle.Stop(30);

    if (sStarHandle.IsAttachedSound())
        sStarHandle.Stop(15);
}

void dKPMusic_c::updTrackVolume(bool isOpenMenu) {
    if (!sStreamHandle.IsAttachedSound())
        return;

    f32 newVol = (isOpenMenu) ? 0.3 : 1.0;
    sStreamHandle.SetVolume(newVol, 15);
}

/* Types are:
 * 0: on map, default volume
 * 1: in menu, make it silent
 * 2: in stockItem, make it loud
*/
void dKPMusic_c::updStarVolume(int type) {
    if (!sStarHandle.IsAttachedSound())
        return;

    float volume = 1.5f;
    switch (type) {
        case 1:
            volume = 0.0f;
            break;
        case 2:
            volume = 4.0f;
            break;
    };

    sStarHandle.SetVolume(volume, 15);
}
#endif