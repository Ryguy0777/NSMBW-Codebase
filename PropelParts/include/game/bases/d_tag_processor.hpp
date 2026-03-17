#pragma once
#include <game/bases/d_message.hpp>

class TagProcessor_c {
public:
    int MsgIDSet(MsgRes_c *, unsigned long, unsigned long);

    u8 mPad[0xC5];
    u8 mIconFontID; // @unofficial
    u8 mPad2[2];
};