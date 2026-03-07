#include <propelparts/game_config.h>

#if defined(KOOPATLAS_DEV_ENABLED) || defined(NEWER_MAP_HUD)
#include <propelparts/bases/koopatlas/d_kp_common.hpp>
#include <propelparts/bases/koopatlas/d_tex_map_colouriser.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/egg/core/eggHeap.h>

inline static float hslValue(float n1, float n2, float hue) {
    if (hue > 6.0f) {
        hue -= 6.0f;
    } else if (hue < 0.0f) {
        hue += 6.0f;
    }

    if (hue < 1.0f) {
        return n1 + (n2 - n1) * hue;
    } else if (hue < 3.0f) {
        return n2;
    } else if (hue < 4.0f) {
        return n1 + (n2 - n1) * (4.0f - hue);
    } else {
        return n1;
    }
}

dTexMapColouriser_c::dTexMapColouriser_c() {
    mpTexMap = nullptr;
    mpSourceImg = nullptr;
    mpNewImg = nullptr;
}

dTexMapColouriser_c::~dTexMapColouriser_c() {
    destroy();
}

void dTexMapColouriser_c::destroy() {
    mpTexMap = nullptr;
    if (mpNewImg) {
        EGG::Heap::free(mpNewImg, nullptr);
        mpNewImg = nullptr;
    }
}

void dTexMapColouriser_c::initTexMap(nw4r::lyt::TexMap *tm) {
    MapReport("Colourising TexMap: %p (w:%d h:%d)\n", tm, tm->GetWidth(), tm->GetHeight());
    if (mpTexMap != nullptr) {
        destroy();
    }

    if (tm->GetTexelFormat() != GX_TF_IA8) {
        MapReport("Warning: Trying to colourise image whose format is %d not GX_TF_IA8\n", tm->GetTexelFormat());
    }

    mpTexMap = tm;
    mpSourceImg = (u16*)tm->GetImage();
    mpNewImg = (u16*)EGG::Heap::alloc(tm->GetWidth() * tm->GetHeight() * 4, 0x20, mHeap::g_gameHeaps[2]);
    tm->SetImage(mpNewImg);
    tm->SetTexelFormat(GX_TF_RGBA8);
}

void dTexMapColouriser_c::copyTexMap(nw4r::lyt::TexMap *tm) {
    if (!mpTexMap) {
        initTexMap(tm);
    } else {
        tm->SetImage(mpNewImg);
        tm->SetTexelFormat(GX_TF_RGBA8);
    }
}

void dTexMapColouriser_c::setColor(int h, int s, int l) {
    if (!mpNewImg) {
        return;
    }

    int width = mpTexMap->GetWidth(), height = mpTexMap->GetHeight();
    int texelW = width / 4;
    int texelH = height / 4;

    u16 *source = mpSourceImg;
    u16 *dest = mpNewImg;

    float hueParam = h / 360.0f;
    float satParam = s / 100.0f;
    float lumParam = l / 100.0f;

    for (int texelY = 0; texelY < texelH; texelY++) {
        for (int texelX = 0; texelX < texelW; texelX++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    u8 intensity = *source & 0xFF;
                    u8 alpha = *source >> 8;

                    u8 r, g, b;

                    // This is a hack
                    if (alpha < 250) {
                        r = g = b = intensity;
                    } else {
                        // Converting from GIMP's colourise code...
                        // H and S are always the same
                        // L is the only thing we need to touch:
                        // We get the luminance from the source pixel
                        // (which, conveniently, is the intensity)
                        // manipulate it using the passed l and then
                        // convert the whole thing to RGB

                        float lum = intensity / 255.0f;

                        // Manipulate it
                        if (l > 0) {
                            lum = lum * (1.0f - lumParam);
                            lum += (1.0f - (1.0f - lumParam));
                        } else if (l < 0) {
                            lum = lum * (lumParam + 1.0f);
                        }

                        // Make it RGB
                        if (s == 0) {
                            r = g = b = lum*255.0f;
                        } else {
                            float m1, m2;
                            if (lum <= 0.5f)
                                m2 = lum * (1.0f + satParam);
                            else
                                m2 = lum + satParam - lum * satParam;

                            m1 = 2.0f * lum - m2;

                            r = hslValue(m1, m2, hueParam * 6.0f + 2.0) * 255.0f;
                            g = hslValue(m1, m2, hueParam * 6.0f) * 255.0f;
                            b = hslValue(m1, m2, hueParam * 6.0f - 2.0) * 255.0f;
                        }
                    }

                    // Now write it
                    dest[0] = (alpha<<8)|r;
                    dest[16] = (g<<8)|b;

                    source++;
                    dest++;
                }
            }

            dest += 16;
        }
    }
}
#endif