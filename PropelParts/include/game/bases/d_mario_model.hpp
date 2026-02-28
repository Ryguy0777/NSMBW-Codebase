#pragma once
#include <game/bases/d_player_model.hpp>

class dMarioMdl_c : public dPlayerMdl_c {
public:
    dMarioMdl_c(u8 index);

    u8 mPad[0x50];
};