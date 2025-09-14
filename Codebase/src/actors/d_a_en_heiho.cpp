#include <kamek.h>
#include <new/bases/d_a_en_heiho.hpp>
#include <new/new_profile.hpp>

CUSTOM_ACTOR_PROFILE(EN_HEIHO, daEnHeiho_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

const char* heihoArcList [] = {"heiho", NULL};

const SpriteData heihoSpriteData = {fProfile::EN_HEIHO, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c heihoProfile(&g_profile_EN_HEIHO, "EN_HEIHO", SpriteId::EN_HEIHO, &heihoSpriteData, heihoArcList);

dCc_c::CcData_s l_heiho_cc = {
    0.0,                        // mOffsetX
    8.0,                        // mOffsetY
    8.0,                        // mWidth
    8.0,                        // mHeight
    dCc_c::CAT_ENTITY,          // mCategory
    0,                          // mAttackCategory
    0x6F,                       // mCategoryInteract
    0xFFBAFFFE,                 // mAttackCategoryInteract     
    0,                          // mFlag
    &dEn_c::normal_collcheck,   // mCallback
};

int daEnHeiho_c::create() {
    OSReport("creating heiho\n");
    
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    createModel();
    mAllocator.adjustFrmHeap();

    initialize();

    return SUCCEEDED;
}

int daEnHeiho_c::execute() {
    mStateMgr.executeState();
    ActorScrOutCheck(0);
    return true;
}

int daEnHeiho_c::draw() {
    mHeihoModel.entry();
    return SUCCEEDED;
}

void daEnHeiho_c::finalUpdate() {
    //OSReport("final updated\n");
    // calc base model
    Mtx someMatrix;
    Mtx thirdMatrix;

    dActor_c::changePosAngle(&mPos, &mAngle, 1);
    PSMTXTrans(mMatrix, mPos.x, mPos.y, mPos.z);
    mMatrix.YrotM(mAngle.y);
    
    PSMTXTrans(someMatrix, 0.0, mCenterOffs.y, 0.0);
    PSMTXConcat(mMatrix, someMatrix, mMatrix);
    mMatrix.XrotM(mAngle.x);

    PSMTXTrans(thirdMatrix, 0.0, -mCenterOffs.y, 0.0);
    PSMTXConcat(mMatrix, thirdMatrix, mMatrix);
    mMatrix.ZrotM(mAngle.z);

    mHeihoModel.setLocalMtx(&mMatrix);
    mHeihoModel.setScale(mBoyon.mScale.x, mBoyon.mScale.y, mBoyon.mScale.z);
    mHeihoModel.calc(false);

    return;
}


void daEnHeiho_c::createModel() {
    mRes = dResMng_c::m_instance->mRes.getRes("heiho", "g3d/heiho.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("heiho");
    mHeihoModel.create(mdl, &mAllocator, 0x20, 1, nullptr);
    dActor_c::setSoftLight_Enemy(mHeihoModel);
    return;
}

void daEnHeiho_c::initialize() {
    OSReport("initialize\n");
    mCc.registerCc(this, &l_heiho_cc);

    mSpeedMax.y = -4.0;

    u8 direction = getPl_LRflag(mPos);
    mDirection = direction;
    mAngle.y = l_look_angles[direction];

    mCenterOffs = mVec3_c(0.0, 12.0, 0.0);
    OSReport("i: %f %f %f\n", mCenterOffs.x, mCenterOffs.y, mCenterOffs.z);

	static const dBcSensorLine_c below(-4<<12, 4<<12, 0<<12);
	static const dBcSensorPoint_c above(0<<12, 20<<12);
	static const dBcSensorLine_c adjacent(5<<12, 10<<12, 8<<12);

	mBc.set(this, (dBcSensor_c*)&below, (dBcSensor_c*)&above, (dBcSensor_c*)&adjacent);
}