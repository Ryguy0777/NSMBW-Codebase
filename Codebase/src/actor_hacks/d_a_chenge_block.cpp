// Source: https://github.com/CLF78/NSMASR-v2/blob/master/game/sprites/tilegod.cpp
#include <kamek.h>
#include <game/mLib/m_effect.hpp>
#include <game/bases/d_eff_actor_manager.hpp>

// External tables
extern u16 TileTable[664];
extern u16 TileGodTiles[5];

// Externs for ASM calls
extern "C" {
u16 GetTileForTileGod(u32 settings, u32 blocktype);
void SpawnTileGodEffect(const char* name, ulong owner, mVec3_c* position, const mAng3_c* rotation, const mVec3_c* scale, u32 settings);
}

u16 GetTileForTileGod(u32 settings, u32 blockType) {

    // If block is > 8, use custom tileset
    if (blockType & 8)
        return 0x8000 | ((blockType & 3) << 8) | ((settings & 0xFF00000) >> 20);

    // Else fall back to original behaviour
    return TileGodTiles[blockType];
}

void SpawnTileGodEffect(const char* name, ulong owner, mVec3_c* position, const mAng3_c* rotation, const mVec3_c* scale, u32 settings) {

    // Get shard flag
    u32 shardType = settings >> 9 & 7;

    // If not zero, spawn shard
    if (shardType) {
        ulong shardSettings = ((shardType - 1) << 8) | 2;
        position->y -= 8;
        dEffActorMng_c::m_instance->createBlockFragEff(*position, shardSettings, -1);

    // Else spawn original effect
    } else
        mEf::createEffect(name, owner, (const mVec3_c*)position, rotation, scale);
}


// Prevent shard setting from overlapping the action type
kmWrite32(0x807E14D8, 0x5508C7FE);

// Patch block type loading
kmBranchDefAsm(0x807E1668, 0x807E1684) {

    // No need for stack saving
    nofralloc

    // Call CPP function
    lwz r4, 0x3AC(r3)
    lwz r3, 0x4(r3)
    bl GetTileForTileGod

    // Move result to r25
    mr r25, r3
    blr
}

// Patch tile table lookup
kmBranchDefCpp(0x8008349C, NULL, u16, u16 tile) {

    // Custom tile, return it as is
    if (tile & 0x8000)
        return tile & 0x7FFF;

    // Fallback
    return TileTable[tile];
}

// Spawn shard effect if set
kmCallDefAsm(0x807E1748) {

    // No need for stack saving
    nofralloc

    // Call CPP function
    lwz r8, 0x4(r18)
    b SpawnTileGodEffect
}

// Fix tile spawn effect Y Offset
kmWrite32(0x807E17A0, 0xEC00F028);