#pragma once
#include <new/game_config.h>

#if defined(KOOPATLAS_DEV_ENABLED) || defined(NEWER_MAP_HUD)
#include <lib/nw4r/lyt/lyt_texMap.h>

// Colourises an IA8 texture
class dTexMapColouriser_c {
public:
    dTexMapColouriser_c();
    ~dTexMapColouriser_c();

    void destroy();
    void initTexMap(nw4r::lyt::TexMap *tm);
    void copyTexMap(nw4r::lyt::TexMap *tm); ///< Only call this after using `initTexMap()`
    void setColor(int h, int s, int l);

private:
    nw4r::lyt::TexMap *mpTexMap;
    u16 *mpSourceImg;
    u16 *mpNewImg;
};
#endif