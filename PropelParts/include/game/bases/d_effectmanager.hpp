#pragma once
#include <game/mLib/m_vec.hpp>

class EffectManager_c {
public:
    enum Kind_e {
        KIND_VS,
        KIND_MAP,
        KIND_MINIGAME,
        KIND_LARRY,
        KIND_ROY,
        KIND_WENDY,
        KIND_IGGY,
        KIND_LEMMY,
        KIND_MORTON,
        KIND_LUGWIG,
        KIND_KAMECK,
        KIND_JR,
        KIND_KOOPA,
        KIND_DEMO,

        KIND_NUM
    };

    static void SetIceBallMissshitEffect(mVec3_c *);
    static void calcGroupForWm();
    static void calcGroup();

    static void *loadBreff(Kind_e kind, unsigned char, EGG::Heap *);
    static void *loadBreft(Kind_e kind, unsigned char, EGG::Heap *);

    static void freeBreff(Kind_e kind);
    static void freeBreft(Kind_e kind);

    static void setResource(Kind_e kind, void *pBreff, void *pBreft);
    static void resetResource(Kind_e kind);

    static void courseOut();

    static void draw(int, int);

    static bool m_firstCalc;
};
