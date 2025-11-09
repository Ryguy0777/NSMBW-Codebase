#include <kamek.h>

#include <new/game_config.h>

// Newer File Select screen

#ifdef NEWER_FILE_SELECT

#include <game/bases/d_lytbase.hpp>

// Change layout name
const char *newerFileSelectName = "fileSelectBase_newer/fileSelectBase.arc";
kmBranchDefAsm(0x80782EA4, 0x80782EA8) {
    lis r4, newerFileSelectName@h
    ori r4, r4, newerFileSelectName@l
    lwz r4, 0(r4)
}

// Dummy out multiplayer button animations
kmWritePointer(0x80943A74, "fileSelectBase_18_DummyAnim.brlan"); // inMulti
kmWritePointer(0x80943AB4, "fileSelectBase_18_DummyAnim.brlan"); // onMulti
kmWritePointer(0x80943AB8, "fileSelectBase_18_DummyAnim.brlan"); // idleMulti
kmWritePointer(0x80943ABC, "fileSelectBase_18_DummyAnim.brlan"); // hitMulti
kmWritePointer(0x80943AC0, "fileSelectBase_18_DummyAnim.brlan"); // offMulti
kmWritePointer(0x80943AC4, "fileSelectBase_18_DummyAnim.brlan"); // outMulti

kmWritePointer(0x80943B88, "DummyGroup"); // inMulti
kmWritePointer(0x80943B8C, "DummyGroup"); // inMulti
kmWritePointer(0x80943BDC, "DummyGroup"); // onMulti
kmWritePointer(0x80943BE0, "DummyGroup"); // onMulti
kmWritePointer(0x80943BE4, "DummyGroup"); // idleMulti
kmWritePointer(0x80943BE8, "DummyGroup"); // idleMulti
kmWritePointer(0x80943BEC, "DummyGroup"); // hitMulti
kmWritePointer(0x80943BF0, "DummyGroup"); // hitMulti
kmWritePointer(0x80943BF4, "DummyGroup"); // offMulti
kmWritePointer(0x80943BF8, "DummyGroup"); // offMulti
//kmWritePointer(0x80943BFC, "DummyGroup"); // outMulti
//kmWritePointer(0x80943C00, "DummyGroup"); // outMulti

// Dummy out N_back
kmWritePointer(0x80943C5C, "DummyPane");

// Don't print BMG strings
kmWrite32(0x80782F38, 0x38E00007);

// Don't register some textboxes
kmWrite32(0x80782F4C, 0x38C00002);

// Don't write BMG to T_back_00
kmWriteNop(0x80782F70);

// Don't register W_multiButton_00 and W_multiCoin_00 panes
kmWriteNop(0x80782F98);

// Don't show/hide N_back
kmWriteNop(0x80783428);
kmWriteNop(0x80783440);
kmWriteNop(0x80783458);
kmWriteNop(0x8078346C);


// D-Pad direction changes
kmWrite32(0x80783D04, 0x4800001C); // File 1, don't move down
kmWrite32(0x80783D20, 0x548007BD); // File 1, left -> up
kmWrite32(0x80783D3C, 0x548007FF); // File 1, right -> down

kmWrite32(0x8078456C, 0x540007BD); // File 2, left -> up
kmWrite32(0x80784598, 0x540007FF); // File 2, right ->down

kmWrite32(0x80784B08, 0x540007BD); // File 3, left -> up
kmWrite32(0x80784B40, 0x540007FF); // File 3, right -> down


// Load new BRLANs
kmWritePointer(0x80943ACC, "fileSelectBase_18_ShowEraseCopy.brlan");
kmWritePointer(0x80943AD0, "fileSelectBase_18_HideEraseCopy.brlan");
kmWrite32(0x80782EE4, 0x38A0001F); // Init new ones

// New A00_window string since the BRLAN names overwrite it
kmWritePointer(0x80943B54, "A00_window");

// New groups (and group indexes)
kmWritePointer(0x80943BFC, "CDHolderGroup");
kmWritePointer(0x80943C00, "CDHolderGroup");
kmWrite32(0x8093422C, 0x0000001D);
kmWrite32(0x80934230, 0x0000001E);

// Disable outMulti anims
kmWriteNop(0x80784220);
kmWriteNop(0x80784230);

// Reset ShowEraseCopy anim (for when the menu opens)
kmWrite32(0x80783610, 0x3880002A);

// Play ShowEraseCopy instead of multi button anim
kmWrite32(0x80783954, 0x3880002A);
kmWrite32(0x807839AC, 0x3880002A);

extern "C" void AnimeStartSetup__9LytBase_cFib(void);

// Hide EraseCopy
kmBranchDefAsm(0x80784200, 0x80784204) {
    addi r3, r3, 0x74
    li r4, 0x2B
    li r5, 0x0
    b AnimeStartSetup__9LytBase_cFib
}

// Show EraseCopy
kmBranchDefAsm(0x80783810, 0x80783814) {
    addi r3, r31, 0x74
    li r4, 0x2A
    li r5, 0x0
    bl AnimeStartSetup__9LytBase_cFib
    lwz r0, 0x14(r1)
    lwz r31, 0xC(r1)
    mtlr r0
    addi r1, r1, 0x10
    blr
}

#endif