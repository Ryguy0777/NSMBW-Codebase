#include <kamek.h>
#include <game/bases/d_res_mng.hpp>
#include <new/game_config.h>

// Patch the function that loads files from the Object folder on game initialization

const char *crsinLoadNames[] = {
    "Mario",
    "Luigi",
    "Kinopio",
#ifdef PLAYER_MODEL_HACKS
    "Kinopico",
#endif
    "Yoshi",
    "P_rcha",
    "L_rcha",
    "K_rcha",
#ifdef PLAYER_MODEL_HACKS
    "C_rcha",
#endif
    "Y_rcha",
    "Y_TexGreen",
    "Y_TexRed",
    "Y_TexYellow",
    "Y_TexBlue",
    "obj_coin",
    "balloon",
    "I_kinoko",
    "I_fireflower",
    "I_iceflower",
    "I_star",
    "I_propeller",
    "I_penguin",
    "I_yoshi_egg",
    "block_tsuta",
    "teresa",
    "jump_step",
    "ice",
    "ice_piece",
    "obj_dokan",
    "obj_door",
    "obj_kusa",
    "obj_kusa_custom",
    "obj_hana",
    "obj_hana_custom",
    "obj_hana_daishizen",
    "block_jump",
    "obj_chikuwa_block",
    "lift_rakka_ashi",
    "Mask",
};

kmBranchDefCpp(0x8091ec50, NULL, u32) {
    dResMng_c::m_instance->setRes("Object", crsinLoadNames, ARRAY_SIZE(crsinLoadNames), nullptr);
    return 1;
}