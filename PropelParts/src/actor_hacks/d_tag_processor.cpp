#include <kamek.h>
#include <game/bases/d_tag_processor.hpp>
#include <propelparts/constants/message_list.h>

// Change MsgIDSet() to get the string from the given MsgRes_c, instead of the original BMG
kmCallDefCpp(0x800E6AF4, const wchar_t *, TagProcessor_c *this_, MsgRes_c *msgRes, ulong cat, ulong msg) {
    return msgRes->getMsg(cat, msg);
}

// Nop dMessage_c::getMsg() call
kmWriteNop(0x800E6AFC);

// Add other colored switch blocks to the TagProcessor_c class
kmBranchDefCpp(0x800e72b0, NULL, int, TagProcessor_c *this_, MsgRes_c *msgRes, char *param_3) {
    ulong msgCat = 0;
    ulong msgID = 0;
    bool forceGameBmg = false;
    switch (*param_3) {
        default:
        case '\0': // Worldmap block
            msgID = 0x22;
            forceGameBmg = true;
            break;
        case '\x01': // Worldmap dotted block
            msgID = 0x21;
            forceGameBmg = true;
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

    if (forceGameBmg) {
        msgRes = dMessage_c::getMesRes();
    }
    return this_->MsgIDSet(msgRes, msgCat, msgID);
}

// Controls per-font TagProcessor behavior
// 0xB is for printing icons like PictureFont
// Others don't serve a purpose?
const u16 sc_newFontParam[] = {
    0xA,
    0xB,
    0xC,
    0xE,
    0xB,
};

// Load new param array
kmBranchDefAsm(0x800E6B3C, 0x800E6B40) {
    lis r3, sc_newFontParam@h
    ori r3, r3, sc_newFontParam@l

    // While we're here, store the icon message fontID for later
    // (This is normally a padding byte)
    stb r31, 0xC5(r26)
    blr
}

// For context, FontChange() and PictureFontCalcRect() are hardcoded to use PictureFont,
// preventing us from using custom fonts to print icons, so whenever MsgIDSet() is called,
// we save the fontID into a padding byte, and use that to load the desired font

// FontChange()
kmBranchDefAsm(0x800E5958, 0x800E595C) {
    lbz r3, 0xC5(r28)
    blr
}

// PictureFontCalcRect()
kmBranchDefAsm(0x800E64B8, 0x800E64BC) {
    lbz r3, 0xC5(r3)
    blr
}