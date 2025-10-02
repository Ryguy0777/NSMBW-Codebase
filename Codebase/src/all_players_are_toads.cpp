#ifdef FUCK_YOU_BITCH

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

class dKinopicoMdl_c : public dKinopioMdl_c {
public:
    dKinopicoMdl_c(u8 index);

    void setColorType(u8 colorType);
    void createPlayerModel();

    m3d::anmTexPat_c* getBodyTexAnm();
    void createAnim(m3d::anmTexPat_c& anm, const char* name, u8 modelIdx);
};

dKinopicoMdl_c::dKinopicoMdl_c(u8 index) : dKinopioMdl_c(index) {
    static ModelData modelData = {
      "Kinopico",
      "K_rcha",
      {"CB_model", "SCB_model", "PLCB_model", "PCB_model"},
      {"CH_model", "SCH_model", "PLCH_model", "PCH_model"},
      18.0,
      13.0,
      18.0,
      18.0,
    };

    
    mpModelData = &modelData;

    mFaceJointIdx = 0xF;
}

void dKinopicoMdl_c::createAnim(m3d::anmTexPat_c& anm, const char* name, u8 modelIdx) {
    nw4r::g3d::ResAnmTexPat anmTexPat = mModelResFile.GetResAnmTexPat(name);
    nw4r::g3d::ResMdl mdl = mModelResFile.GetResMdl(
      modelIdx < MODEL_COUNT ? mpModelData->mBodyModels[modelIdx]
                             : mpModelData->mHeadModels[modelIdx - MODEL_COUNT]
    );
    anm.create(mdl, anmTexPat, &mAllocator, nullptr, 2);
}

void dKinopicoMdl_c::createPlayerModel() {
    dPlayerMdl_c::createPlayerModel();

    createAnim(mHeadSwitchAnim, "PH_wait", MODEL_COUNT + MODEL_NORMAL);
    createAnim(mBodySwitchAnim, "KB_switch", MODEL_NORMAL);

    createAnim(mPropelHeadSwitchAnim, "PH_wait", MODEL_COUNT + MODEL_PROPELLER);
    createAnim(mPropelBodySwitchAnim, "PLKB_switch", MODEL_PROPELLER);

    createAnim(mPenguinHeadSwitchAnim, "PH_wait", MODEL_COUNT + MODEL_PENGUIN);
    createAnim(mPenguinBodySwitchAnim, "PKB_switch", MODEL_PENGUIN);
}

m3d::anmTexPat_c* dKinopicoMdl_c::getBodyTexAnm() {
    switch (mModelIdx) {
        default:
            return &mBodySwitchAnim;
        case MODEL_PROPELLER:
            return &mPropelBodySwitchAnim;
        case MODEL_PENGUIN:
            return &mPenguinBodySwitchAnim;
    }
}

const char *headSwitchAnims[] = {
    "KH_switch", "KH_switch", "PLKH_switch", "PKH_switch"
};

const char *bodySwitchAnims[] = {
    "KB_switch", "KB_switch", "PLKB_switch", "PKB_switch"
};

void dKinopicoMdl_c::setColorType(u8 colorType) {
    if (mCurColorType == colorType) {
        return;
    }

    mCurColorType = colorType;
    int modelIdx = mModelIdx;

    float frame = 0;
    if (colorType == 1) { // fire
        frame = 1;
    } else if (colorType == 2) { // ice
        frame = 2;
    }

    // Head animation
    m3d::anmTexPat_c* headTexAnm = getHeadTexAnm();

    nw4r::g3d::ResAnmTexPat anmTexPat = mModelResFile.GetResAnmTexPat(headSwitchAnims[modelIdx]);
    headTexAnm->setAnm(mModels[modelIdx].mHead, anmTexPat, 0, m3d::PLAYMODE_INHERIT);

    headTexAnm->setRate(0.0, 0);
    headTexAnm->setFrame(frame, 0);
    mModels[modelIdx].mHead.setAnm(*headTexAnm);

    // Body animation
    m3d::anmTexPat_c* bodyTexAnm = getBodyTexAnm();

    anmTexPat = mModelResFile.GetResAnmTexPat(bodySwitchAnims[modelIdx]);
    m3d::mdl_c* bodyMdl = getBodyMdl();

    bodyTexAnm->setAnm(*bodyMdl, anmTexPat, 0, m3d::PLAYMODE_INHERIT);

    bodyTexAnm->setRate(0.0, 0);
    bodyTexAnm->setFrame(frame, 0);
    bodyMdl->setAnm(*bodyTexAnm);
}

kmBranchDefCpp(0x800d6e00, NULL, void, dPyMdlMng_c *mng, u8 index) {
    switch (index) {
        case dPyMdlMng_c::MODEL_MARIO:
        case dPyMdlMng_c::MODEL_LUIGI:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) dMarioMdl_c(index);
            break;
        
        case dPyMdlMng_c::MODEL_TOAD_BLUE:
        case dPyMdlMng_c::MODEL_TOAD_RED:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) dKinopioMdl_c(index);
            break;
        
        case dPyMdlMng_c::MODEL_TOAD_YELLOW:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) dKinopicoMdl_c(index);
            break;

        case dPyMdlMng_c::MODEL_YOSHI:
            mng->mpMdl = new (mHeap::g_gameHeaps[0], 4) dYoshiMdl_c(index);
            break;

        default:
            mng->mpMdl = nullptr;
            break;
    }
}

kmWritePointer(0x8098c488, "Kinopico");
kmWritePointer(0x8098C490, "KinopioM");

#endif