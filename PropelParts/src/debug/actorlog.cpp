#include <kamek.h>
#include <revolution/OS/OSError.h>
#include <game/framework/f_profile.hpp>
#include <propelparts/bases/d_debug_config.hpp>

void printActorInfo(u16 profId, void *parent, ulong settings, int group) {
    char *profName = dProf_getName(profId);
	if (dDebugConfig_c::m_instance->mActorLog)
    	OSReport("I: %04d, P:%p S:%08x T:%d N:%s\n", profId, parent, settings, group, profName);
    return;
}

extern "C" void printActorInfo(void);

kmCallDefAsm(0x80162BC0) {
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r3, 0x1C(sp)
	stw r4, 0x18(sp)
	stw r5, 0x14(sp)
	stw r6, 0x10(sp)

	bl printActorInfo

	lwz r6, 0x10(sp)
	lwz r5, 0x14(sp)
	lwz r4, 0x18(sp)
	lwz r3, 0x1C(sp)
	lwz r0, 0x24(sp)
	mtlr r0
	addi sp, sp, 0x20

	// Return to __objCreate
	clrlslwi r31, r3, 16, 2
	blr
}