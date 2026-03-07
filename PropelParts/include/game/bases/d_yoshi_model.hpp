#pragma once
#include <game/bases/d_player_model_base.hpp>

class dYoshiMdl_c : public dPyMdlBase_c {
public:
    dYoshiMdl_c(u8 index);

    u8 mPad[0x20C];
};