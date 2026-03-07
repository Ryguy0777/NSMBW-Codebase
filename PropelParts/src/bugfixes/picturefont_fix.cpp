#include <kamek.h>

// Always draw picturefont white
// So colored textboxes don't tint the image

kmBranchDefAsm(0x800E596C, 0x800E598C) {
    nofralloc

    lis r6, 0xFFFF
	ori r6, r6, 0xFF00
	lwz r0, 8(r5)
	or r0, r0, r6
	stw r0, 0x18(r1)
	lwz r0, 0xC(r5)
	or r0, r0, r6
	stw r0, 0x1C(r1)
	lwz r0, 0x10(r5)
	or r0, r0, r6
	stw r0, 0x20(r1)
	lwz r0, 0x14(r5)
	or r0, r0, r6
	stw r0, 0x24(r1)

    blr
}
