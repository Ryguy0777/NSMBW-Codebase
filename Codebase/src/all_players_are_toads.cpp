#ifdef ALL_PLAYER_ARE_TOAD

#include <kamek.h>
#include <game/bases/d_player_model.hpp>
#include <game/bases/d_yoshi_model.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_heap.hpp>

class dKinopioMarioMdl_c : public dKinopioMdl_c {
public:
    dKinopioMarioMdl_c(u8 index);
};

static dPlayerMdl_c::ModelData toadMarioData = {
    "KinopioM",
    "K_rcha",
    {"KB_model", "SKB_model", "PLKB_model", "PKB_model"},
    {"KH_model", "SKH_model", "PLKH_model", "PKH_model"},
    18.0,
    13.0,
    18.0,
    18.0,
};

dKinopioMarioMdl_c::dKinopioMarioMdl_c(u8 index) : dKinopioMdl_c(index) {
    mpModelData = &toadMarioData;
    switch (index) {
        case dPyMdlMng_c::MODEL_LUIGI:
            OSReport("luig\n");
            mCharaID = dPyMdlMng_c::MODEL_TOAD_YELLOW;
            break;
        
        default:
            OSReport("mari\n");
            mCharaID = dPyMdlMng_c::MODEL_TOAD_BLUE;
            break;
    }
    mFaceJointIdx = 0xF;
}

kmBranchDefCpp(0x800d6e00, NULL, void, dPyMdlMng_c *mng, u8 index) {
    switch (index) {
        case dPyMdlMng_c::MODEL_MARIO:
        case dPyMdlMng_c::MODEL_LUIGI:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) dKinopioMarioMdl_c(index);
            break;
        
        case dPyMdlMng_c::MODEL_TOAD_BLUE:
        case dPyMdlMng_c::MODEL_TOAD_YELLOW:
        case dPyMdlMng_c::MODEL_TOAD_RED:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) dKinopioMdl_c(index);
            break;
        
        case dPyMdlMng_c::MODEL_YOSHI:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) dYoshiMdl_c(index);
            break;

        default:
            mng->mpMdl = nullptr;
            break;
    }
}

kmWritePointer(0x8098c488, "KinopioM");

#endif