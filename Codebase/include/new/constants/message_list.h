#pragma once

#include <constants/message_list.h>

enum NEW_BMG_GROUPS_e {
    BMG_CATEGORY_MESSAGE_BOX_TITLE = 0x400,
    BMG_CATEGORY_MESSAGE_BOX_TEXT = 0x401,
    BMG_CATEGORY_LEVEL_NAMES = 0x500,
};

// level names use format:
// category: BMG_CATEGORY_LEVEL_NAMES + world number
// ID: level number