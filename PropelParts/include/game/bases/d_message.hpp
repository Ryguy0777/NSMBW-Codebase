#pragma once

#include <lib/egg/core/eggMsgRes.h>

class MsgRes_c : public EGG::MsgRes {};

class dMessage_c {
public:
    static MsgRes_c *getMesRes();

    static const wchar_t *getMsg(unsigned long group, unsigned long messageID);
};
