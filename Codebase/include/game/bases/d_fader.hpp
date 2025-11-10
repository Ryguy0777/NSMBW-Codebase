#pragma once
#include <types.h>

class dFader_c {
public:
    enum fader_type_e {
        FADE,
        CIRCLE,
        BOWSER,
        WAVY,
        MARIO,
        CIRCLE_SLOW,
        NONE
    };

    /// @brief Starts a fade in with the @ref mFader_c::mFader "current fader".
    /// @param duration The duration of the fade in.
    static void startFadeIn(u16 duration);

    /// @brief Starts a fade out with the @ref mFader_c::mFader "current fader".
    /// @param duration The duration of the fade out.
    static void startFadeOut(u16 duration);

    static void setFader(fader_type_e type);
};
