#include <kamek.h>
#include <game/bases/d_tag_processor.hpp>
#include <new/constants/message_list.h>

// Add other colored switch blocks to the TagProcessor_c class
kmBranchDefCpp(0x800e72b0, NULL, int, TagProcessor_c *this_, MsgRes_c *msgRes, char *param_3) {
    ulong msgCat = 0;
    ulong msgID = 0;
    switch (*param_3) {
        default:
        case '\0': // Worldmap block
            msgID = 0x22;
            break;
        case '\x01': // Worldmap dotted block
            msgID = 0x21;
            break;
        case '\x02': // Yellow dotted block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x1;
            break;
        case '\x03': // Yellow block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x0;
            break;
        case '\x04': // Red dotted block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x3;
            break;
        case '\x05': // Red block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x2;
            break;
        case '\x06': // Green dotted block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x5;
            break;
        case '\x07': // Green block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x4;
            break;
        case '\x08': // Blue dotted block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x7;
            break;
        case '\x09': // Blue block
            msgCat = BMG_CATEGORY_PICTUREFONT_ICONS;
            msgID = 0x6;
            break;
    }
    return this_->MsgIDSet(msgRes, msgCat, msgID);
}