#include <kamek.h>
#include <propelparts/game_config.h>

#include <game/bases/d_player_model_manager.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/bases/d_mario_model.hpp>
#include <game/bases/d_kinopio_model.hpp>
#include <game/bases/d_yoshi_model.hpp>

kmBranchDefCpp(0x800d6e00, NULL, void, dPyMdlMng_c *mng, u8 index) {
    switch (index) {
        case dPyMdlMng_c::MODEL_MARIO:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) MARIO_MDL_CLASS(index);
            break;
        
        case dPyMdlMng_c::MODEL_LUIGI:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) LUIGI_MDL_CLASS(index);
            break;
        
        case dPyMdlMng_c::MODEL_TOAD_BLUE:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) BLUE_TOAD_MDL_CLASS(index);
            break;

        case dPyMdlMng_c::MODEL_TOAD_YELLOW:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) YELLOW_TOAD_MDL_CLASS(index);
            break;

        case dPyMdlMng_c::MODEL_TOAD_RED:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) RED_TOAD_MDL_CLASS(index);
            break;

        case dPyMdlMng_c::MODEL_YOSHI:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) YOSHI_MDL_CLASS(index);
            break;

        default:
            mng->mpMdl = nullptr;
            break;
    }
}
