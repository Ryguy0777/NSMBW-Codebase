#include <kamek.h>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_s_world_map_static.hpp>

#include <new/world_map_config.h>

// Load new resources

// Load global res
kmBranchDefAsm(0x80926BFC, 0x80926C00) {
    lis r31, sc_GlobalResList@h
	ori r31, r31, sc_GlobalResList@l
	blr
}

// Unload global res
kmBranchDefAsm(0x809278F4, 0x809278F8) {
    lis r26, sc_GlobalResList@h
	ori r26, r26, sc_GlobalResList@l
	blr
}

kmWrite16(0x80926C22, ARRAY_SIZE(sc_GlobalResList)); // Load count
kmWrite16(0x80927912, ARRAY_SIZE(sc_GlobalResList)); // Delete count

// Skip LETTER_WINDOW creation
#ifdef REMOVE_LETTER_TOAD
kmWrite32(0x80927064, 0x48000018);
#endif