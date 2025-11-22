#include <types.h>
#include <constants/sound_list.h>
#include <new/constants/newer_track_list.h>
#include <game/bases/d_dvd.hpp>
#include <game/snd/snd_audio_mgr.hpp>

struct HijackedStream {
    u32 stringOffset;
    u32 stringOffsetFast;
    u32 infoOffset;
    u8 originalID;
    int streamID;
};

struct Hijacker {
    HijackedStream stream[2];
    u8 currentStream;
    u8 currentCustomTheme;
};

// Offsets are from the start of the INFO block, not the start of the brsar.
// INFO begins at 0x212C0, so that has to be subtracted from absolute offsets
// within the brsar.
#define _I(offs) ((offs)-0x212C0)

Hijacker Hijackers[2] = {
    {
        {
            {_I(0x4A8F8), _I(0x4A938), _I(0x476C4), 4, STRM_BGM_ATHLETIC},
            {_I(0x4B2E8), _I(0x4B320), _I(0x48164), 10, STRM_BGM_SHIRO}
        },
        0, 0
    },

    {
        {
            {_I(0x4A83C), _I(0x4A8B4), 0, 1, STRM_BGM_CHIJOU},
            {_I(0x4A878), _I(0x4A780), 0, 2, STRM_BGM_CHIKA},
        },
        0, 0
    }
};

inline char *BrsarInfoOffset(u32 offset) {
    return (char*)(*(u32*)(((u32)SndAudioMgr::sInstance) + 0x5CC)) + offset;
}

void FixFilesize(u32 streamNameOffset) {
    char *streamName = BrsarInfoOffset(streamNameOffset);

    char nameWithSound[80];
    sprintf(nameWithSound, "/Sound/%s", streamName);

    s32 entryNum;
    DVDFileInfo info;
    
    if ((entryNum = DVDConvertPathToEntrynum(nameWithSound)) >= 0) {
        if (DVDFastOpen(entryNum, &info)) {
            u32 *lengthPtr = (u32*)(streamName - 0x1C);
            *lengthPtr = info.size;
        }
    } else
        OSReport("What, I couldn't find \"%s\" :(\n", nameWithSound);
}

u8 hijackMusicWithSongName(const char *songName, int themeID, bool hasFast, int channelCount, int trackCount, int *wantRealStreamID) {
    Hijacker *hj = &Hijackers[channelCount == 4 ? 1 : 0];

    // do we already have this theme in this slot?
    // if so, don't switch streams
    // if we do, NSMBW will think it's a different song, and restart it ...
    // but if it's just an area transition where both areas are using the same
    // song, we don't want that
    if ((themeID >= 0) && hj->currentCustomTheme == themeID)
        return hj->stream[hj->currentStream].originalID;

    // which one do we use this time...?
    int toUse = (hj->currentStream + 1) & 1;

    hj->currentStream = toUse;
    hj->currentCustomTheme = themeID;

    // write the stream's info
    HijackedStream *stream = &hj->stream[hj->currentStream];

    if (stream->infoOffset) {
        u16 *thing = (u16*)(BrsarInfoOffset(stream->infoOffset) + 4);
        OSReport("Modifying stream info, at offset %x which is at pointer %x\n", stream->infoOffset, thing);
        OSReport("It currently has: channel count %d, track bitfield 0x%x\n", thing[0], thing[1]);
        thing[0] = channelCount;
        thing[1] = (1 << trackCount) - 1;
        OSReport("It has been set to: channel count %d, track bitfield 0x%x\n", thing[0], thing[1]);
    }

    sprintf(BrsarInfoOffset(stream->stringOffset), "new/%s.er", songName);
    sprintf(BrsarInfoOffset(stream->stringOffsetFast), hasFast ? "new/%s_F.er" : "new/%s.er", songName);

    // update filesizes
    FixFilesize(stream->stringOffset);
    FixFilesize(stream->stringOffsetFast);

    // done!
    if (wantRealStreamID)
        *wantRealStreamID = stream->streamID;

    return stream->originalID;
}
