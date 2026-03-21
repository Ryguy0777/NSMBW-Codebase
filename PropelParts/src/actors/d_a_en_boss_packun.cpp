#include <kamek.h>
#include <propelparts/bases/d_a_en_boss_packun.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_res_mng.hpp>

CUSTOM_ACTOR_PROFILE(EN_BOSS_PACKUN, daEnBossPackun_c, fProfile::EN_BOSS_KAMECK, fProfile::DRAW_ORDER::EN_BOSS_KAMECK, 0x2);

const char* l_BOSS_PACKUN_res[] = {"BossPackun", NULL};
const dActorData_c c_BOSS_PACKUN_actor_data = {fProfile::EN_BOSS_PACKUN, 8, -16, 0, 0, 0, 0, 0, 0, 0, 0, ACTOR_CREATE_GLOBAL};
dCustomProfile_c l_BOSS_PACKUN_profile(&g_profile_EN_BOSS_PACKUN, "EN_BOSS_PACKUN", CourseActor::EN_BOSS_PACKUN, &c_BOSS_PACKUN_actor_data, l_BOSS_PACKUN_res);

int daEnBossPackun_c::execute() {
    mStateMgr.executeState();
    return SUCCEEDED;
}

int daEnBossPackun_c::draw() {
    mModel.entry();

    return SUCCEEDED;
}

void daEnBossPackun_c::finalUpdate() {
    PSMTXTrans(mMatrix, mPos.x, mPos.y, mPos.z);
    mMatrix.XYZrotM(mAngle.x, mAngle.y, mAngle.z);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mBoyoMng.mScale);
    mModel.calc(false);

    return;
}

void daEnBossPackun_c::createModel() {
    mRes = dResMng_c::m_instance->getRes("BossPackun", "g3d/BossPackun.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("BossPackun");
    mModel.create(mdl, &mAllocator, 0x12B, 1, nullptr);
    setSoftLight_Boss(mModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("sleep_wait");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);

    nw4r::g3d::ResAnmClr resAnmClr = mRes.GetResAnmClr("damage");
    mAnmClr.create(mdl, resAnmClr, &mAllocator, 0, 1);
    mAnmClr.setPlayMode(m3d::FORWARD_LOOP, 0);
}