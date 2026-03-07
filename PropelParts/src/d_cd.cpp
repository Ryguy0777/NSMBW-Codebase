#include <kamek.h>
#include <game/bases/d_cd.hpp>

// Load Newer-styled custom music
// TODO: find a way to do this that doesn't conflict with retail tracks
/*kmBranchDefCpp(0x8008E5E4, 0x8008E5E8, u8, u8 zoneMusicID) {
    // Not a custom track, just return the ID
    if (zoneMusicID < 100)
        return zoneMusicID;

    bool usesDrums = (zoneMusicID >= 200);
    return hijackMusicWithSongName(newerTrackList[zoneMusicID-100], zoneMusicID, true, usesDrums?4:2, usesDrums?2:1, 0);
}*/