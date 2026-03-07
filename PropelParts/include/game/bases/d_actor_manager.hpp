#pragma once

#include <game/mLib/m_vec.hpp>

class dActorMng_c {
public:
    void createUpCoin(const mVec3_c &pos, u8 dir, u8 count, u8 layer);
    void createJumpCoin(const mVec3_c&, u8, u8);
    void createBlockDownCoin(const mVec3_c&, u8, u8);

    static dActorMng_c *m_instance;
};