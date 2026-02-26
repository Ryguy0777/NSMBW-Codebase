#include <kamek.h>
#include <propelparts/bases/d_a_model_loader.hpp>
#include <propelparts/bases/d_custom_profile.hpp>

// Resource loader

const dActorData_c c_MODEL_RES_actor_data = {fProfile::DUMMY_ACTOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ACTOR_CREATE_GLOBAL};

const char* l_MODEL_RES0_res[] = {"modelLoaderRes0", NULL};
dCustomProfile_c l_MODEL_RES0_profile(CourseActor::MODEL_RES0, &c_MODEL_RES_actor_data, l_MODEL_RES0_res);

const char* l_MODEL_RES1_res[] = {"modelLoaderRes1", NULL};
dCustomProfile_c l_MODEL_RES1_profile(CourseActor::MODEL_RES1, &c_MODEL_RES_actor_data, l_MODEL_RES1_res);

const char* l_MODEL_RES2_res[] = {"modelLoaderRes2", NULL};
dCustomProfile_c l_MODEL_RES2_profile(CourseActor::MODEL_RES2, &c_MODEL_RES_actor_data, l_MODEL_RES2_res);

const char* l_MODEL_RES3_res[] = {"modelLoaderRes3", NULL};
dCustomProfile_c l_MODEL_RES3_profile(CourseActor::MODEL_RES3, &c_MODEL_RES_actor_data, l_MODEL_RES3_res);

const char* l_MODEL_RES4_res[] = {"modelLoaderRes4", NULL};
dCustomProfile_c l_MODEL_RES4_profile(CourseActor::MODEL_RES4, &c_MODEL_RES_actor_data, l_MODEL_RES4_res);

const char* l_MODEL_RES5_res[] = {"modelLoaderRes5", NULL};
dCustomProfile_c l_MODEL_RES5_profile(CourseActor::MODEL_RES5, &c_MODEL_RES_actor_data, l_MODEL_RES5_res);

const char* l_MODEL_RES6_res[] = {"modelLoaderRes6", NULL};
dCustomProfile_c l_MODEL_RES6_profile(CourseActor::MODEL_RES6, &c_MODEL_RES_actor_data, l_MODEL_RES6_res);

const char* l_MODEL_RES7_res[] = {"modelLoaderRes7", NULL};
dCustomProfile_c l_MODEL_RES7_profile(CourseActor::MODEL_RES7, &c_MODEL_RES_actor_data, l_MODEL_RES7_res);

const char* l_MODEL_RES8_res[] = {"modelLoaderRes8", NULL};
dCustomProfile_c l_MODEL_RES8_profile(CourseActor::MODEL_RES8, &c_MODEL_RES_actor_data, l_MODEL_RES8_res);

const char* l_MODEL_RES9_res[] = {"modelLoaderRes9", NULL};
dCustomProfile_c l_MODEL_RES9_profile(CourseActor::MODEL_RES9, &c_MODEL_RES_actor_data, l_MODEL_RES9_res);

const char* l_MODEL_RES10_res[] = {"modelLoaderRes10", NULL};
dCustomProfile_c l_MODEL_RES10_profile(CourseActor::MODEL_RES10, &c_MODEL_RES_actor_data, l_MODEL_RES10_res);

const char* l_MODEL_RES11_res[] = {"modelLoaderRes11", NULL};
dCustomProfile_c l_MODEL_RES11_profile(CourseActor::MODEL_RES11, &c_MODEL_RES_actor_data, l_MODEL_RES11_res);

const char* l_MODEL_RES12_res[] = {"modelLoaderRes12", NULL};
dCustomProfile_c l_MODEL_RES12_profile(CourseActor::MODEL_RES12, &c_MODEL_RES_actor_data, l_MODEL_RES12_res);

const char* l_MODEL_RES13_res[] = {"modelLoaderRes13", NULL};
dCustomProfile_c l_MODEL_RES13_profile(CourseActor::MODEL_RES13, &c_MODEL_RES_actor_data, l_MODEL_RES13_res);

const char* l_MODEL_RES14_res[] = {"modelLoaderRes14", NULL};
dCustomProfile_c l_MODEL_RES14_profile(CourseActor::MODEL_RES14, &c_MODEL_RES_actor_data, l_MODEL_RES14_res);

const char* l_MODEL_RES15_res[] = {"modelLoaderRes15", NULL};
dCustomProfile_c l_MODEL_RES15_profile(CourseActor::MODEL_RES15, &c_MODEL_RES_actor_data, l_MODEL_RES15_res);

// The model loader itself

CUSTOM_ACTOR_PROFILE(AC_MODEL_LOADER, daModelLoader_c, fProfile::WM_KOOPA_CASTLE, fProfile::DRAW_ORDER::WM_KOOPA_CASTLE, 0);

const dActorData_c c_MODEL_LOADER_actor_data = {fProfile::AC_MODEL_LOADER, 8, -8, 0, 0, 0, 0, 0, 0, 0, 0, ACTOR_CREATE_GLOBAL};
dCustomProfile_c l_MODEL_LOADER_profile(&g_profile_AC_MODEL_LOADER, "AC_MODEL_LOADER", CourseActor::AC_MODEL_LOADER, &c_MODEL_LOADER_actor_data);

float l_model_loader_z_pos[16] = {
    0.0, -5000.0, -4500.0, -2000.0,
    -1000.0, 300.0, 800.0, 1600.0,
    2000.0, 3600.0, 4000.0, 4500.0,
    6000.0, 6500.0, 7000.0, 7500.0
};

int daModelLoader_c::create() {
    mBankNo = (mParam >> 28) & 0xF;
    mBrresNo = (mParam >> 24) & 0xF;
    mSoftLightType = (SOFT_LIGHT_TYPE_e)(mParam >> 20 & 0xF);
    mPlaymode = (m3d::playMode_e)(mParam >> 16 & 0xF);
    mPos.z = l_model_loader_z_pos[(mParam >> 8) & 0b111];

    mAnmRate = float((mParam >> 4) & 0xF) / 4.0f;
    if (mAnmRate == 0.0f) {
        mAnmRate = 1.0f;
    }

    mModelScale = float(mParam & 0xF) / 4.0f;
    if (mModelScale == 0.0f) {
        mModelScale = 1.0f;
    }

    // Model setup
    char bankName[17];
    sprintf(bankName, "modelLoaderRes%d", mBankNo);
    char brresName[13];
    sprintf(brresName, "g3d/%d.brres", mBrresNo);

    // Get brres data
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    mRes = dResMng_c::m_instance->getRes(bankName, brresName);

    // Check what animations exist inside this brres
    ulong animBuffer = nw4r::g3d::ScnMdl::BUFFER_RESMATMISC;
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("anim");
    if (resAnmChr.ptr()) {
        mValidAnms |= ANM_CHR_VALID;
    }

    nw4r::g3d::ResAnmClr resAnmClr = mRes.GetResAnmClr("anim");
    if (resAnmClr.ptr()) {
        mValidAnms |= ANM_CLR_VALID;
        animBuffer |= nw4r::g3d::ScnMdl::ANM_MATCLR;
    }

    nw4r::g3d::ResAnmTexPat resAnmTexPat = mRes.GetResAnmTexPat("anim");
    if (resAnmTexPat.ptr()) {
        mValidAnms |= ANM_PAT_VALID;
        animBuffer |= nw4r::g3d::ScnMdl::ANM_TEXPAT;
    }

    nw4r::g3d::ResAnmTexSrt resAnmTexSrt = mRes.GetResAnmTexSrt("anim");
    if (resAnmTexSrt.ptr()) {
        mValidAnms |= ANM_SRT_VALID;
        animBuffer |= nw4r::g3d::ScnMdl::ANM_TEXSRT;
    }

    nw4r::g3d::ResAnmVis resAnmVis = mRes.GetResAnmVis("anim");
    if (resAnmVis.ptr()) {
        mValidAnms |= ANM_VIS_VALID;
        animBuffer |= nw4r::g3d::ScnMdl::ANM_VIS;
    }

    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("model");
    mModel.create(mdl, &mAllocator, animBuffer, 1, nullptr);

    if (mValidAnms & ANM_CHR_VALID) {
        mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
        mAnmChr.setAnm(mModel, resAnmChr, mPlaymode);
        mModel.setAnm(mAnmChr, 0.0f);
        mAnmChr.setRate(mAnmRate);
    }
    if (mValidAnms & ANM_CLR_VALID) {
        mAnmClr.create(mdl, resAnmClr, &mAllocator, 0, 1);
        mAnmClr.setAnm(mModel, resAnmClr, 0, mPlaymode);
        mModel.setAnm(mAnmClr, 0.0f);
        mAnmClr.setRate(mAnmRate, 0);
    }
    if (mValidAnms & ANM_PAT_VALID) {
        mAnmPat.create(mdl, resAnmTexPat, &mAllocator, 0, 1);
        mAnmPat.setAnm(mModel, resAnmTexPat, 0, mPlaymode);
        mModel.setAnm(mAnmPat, 0.0f);
        mAnmPat.setRate(mAnmRate, 0);
    }
    if (mValidAnms & ANM_SRT_VALID) {
        mAnmSrt.create(mdl, resAnmTexSrt, &mAllocator, 0, 1);
        mAnmSrt.setAnm(mModel, resAnmTexSrt, 0, mPlaymode);
        mModel.setAnm(mAnmSrt, 0.0f);
        mAnmSrt.setRate(mAnmRate, 0);
    }
    if (mValidAnms & ANM_VIS_VALID) {
        mAnmVis.create(mdl, resAnmVis, &mAllocator, 0);
        mAnmVis.setAnm(mModel, resAnmVis, mPlaymode);
        mModel.setAnm(mAnmVis, 0.0f);
        mAnmVis.setRate(mAnmRate);
    }
    
    switch (mSoftLightType) {
        case SOFT_LIGHT_PLAYER:
            setSoftLight_Player(mModel);
            break;

        case SOFT_LIGHT_ENEMY:
            setSoftLight_Enemy(mModel);
            break;

        case SOFT_LIGHT_MAP:
            setSoftLight_Map(mModel);
            break;
            
        case SOFT_LIGHT_MAPOBJ:
            setSoftLight_MapObj(mModel);
            break;

        case SOFT_LIGHT_BOSS:
            setSoftLight_Boss(mModel);
            break;

        case SOFT_LIGHT_ITEM:
            setSoftLight_Item(mModel);
            break;
    }

    mAllocator.adjustFrmHeap();

    return SUCCEEDED;
}

int daModelLoader_c::execute() {
    if (mValidAnms & ANM_CHR_VALID) {
        mModel.play();
    }
    if (mValidAnms & ANM_CLR_VALID) {
        mAnmClr.play();
    }
    if (mValidAnms & ANM_PAT_VALID) {
        mAnmPat.play();
    }
    if (mValidAnms & ANM_SRT_VALID) {
        mAnmSrt.play();
    }
    if (mValidAnms & ANM_VIS_VALID) {
        mAnmVis.play();
    }

    return SUCCEEDED;
}

int daModelLoader_c::draw() {
    changePosAngle(&mPos, &mAngle, 1);
    PSMTXTrans(mMatrix, mPos.x, mPos.y, mPos.z);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mModelScale, mModelScale, mModelScale);
    mModel.calc(false);
    mModel.entry();

    return SUCCEEDED;
}