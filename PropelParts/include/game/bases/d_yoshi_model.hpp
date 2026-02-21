#pragma once
#include <game/bases/d_a_player_base.hpp>

class dYoshiMdl_c : public dPyMdlBase_c {
public:
    dYoshiMdl_c(u8 index);

    u8 mPad[0x404-0x20C];
};