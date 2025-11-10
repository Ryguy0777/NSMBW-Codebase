#pragma once
#include <game/mLib/m_vec.hpp>

class EffectManager_c {
public:
    enum Kind_e {
        EFF_VS,
        EFF_MAP,
        EFF_MINIGAME,
        EFF_LARRY,
        EFF_ROY,
        EFF_WENDY,
        EFF_IGGY,
        EFF_LEMMY,
        EFF_MORTON,
        EFF_LUGWIG,
        EFF_KAMECK,
        EFF_JR,
        EFF_KOOPA,
        EFF_DEMO,
        KIND_COUNT
    };

    static void SetIceBallMissshitEffect(mVec3_c *);
    static void calcGroupForWm();
    static void calcGroup();

    static void freeBreft(Kind_e kind);
    static void freeBreff(Kind_e kind);
    static void resetResource(Kind_e kind);

    static void courseOut();

    static void draw(int, int);

    static bool m_firstCalc;
};
