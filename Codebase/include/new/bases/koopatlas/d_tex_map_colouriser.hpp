#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <lib/nw4r/lyt/lyt_texMap.h>

// Colourises an IA8 texture
class dTexMapColouriser_c {
public:
    dTexMapColouriser_c();
    ~dTexMapColouriser_c();

    void resetAndClear();
    void setTexMap(nw4r::lyt::TexMap *tm);
    void applyAlso(nw4r::lyt::TexMap *tm);
    void colourise(int h, int s, int l);

private:
    nw4r::lyt::TexMap *mTexMap;
    u16 *mSourceImage;
    u16 *mNewImage;
};
#endif