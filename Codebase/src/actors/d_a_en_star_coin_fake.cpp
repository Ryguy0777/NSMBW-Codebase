#include <kamek.h>
#include <new/bases/d_a_en_star_coin_fake.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_coin.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(EN_STAR_COIN_FAKE, daEnStarCoinFake_c, fProfile::EN_STAR_COIN, fProfile::DRAW_ORDER::EN_STAR_COIN, 0x2);

const char* l_STAR_COIN_FAKE_res[] = {"star_coin", NULL};
const dActorData_c c_STAR_COIN_FAKE_actor_data = {fProfile::EN_STAR_COIN_FAKE, 0, 0, 0, 0, 32, 32, 0, 0, 0, 0, 8};
dCustomProfile_c l_STAR_COIN_FAKE_profile(&g_profile_EN_STAR_COIN_FAKE, "EN_STAR_COIN_FAKE", SpriteId::EN_STAR_COIN_FAKE, &c_STAR_COIN_FAKE_actor_data, l_STAR_COIN_FAKE_res);

const sCcDatNewF l_fakeStarCoin_cc = {
    {0.0f, -3.0f},
    {12.0f, 15.0f},
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
    BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_TAMA),
    0x30020E,
    CC_STATUS_NONE,
    &dEn_c::normal_collcheck,
};

int daEnStarCoinFake_c::create() {
    // Load the model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("star_coin", "g3d/star_coin.brres");
    nw4r::g3d::ResMdl mdl = resFile.GetResMdl("star_coinA");
    mModel.create(mdl, &mAllocator, 0x20, 1, nullptr);
    setSoftLight_MapObj(mModel);

    mAllocator.adjustFrmHeap();

    mCc.set(this, (sCcDatNewF *)&l_fakeStarCoin_cc);
    mCc.entry();

    mPos.x += 16.0f;
    mPos.y -= 19.0f;
    mPos.z = 500.0f;

    mRotateX = mParam & 1;

    return SUCCEEDED;
};

int daEnStarCoinFake_c::execute() {
    mStateMgr.executeState();
    if (!mRotateX) {
        mAngle.y = -dCoin_c::m_shapeAngle.y;
    } else {
        mAngle.x = -dCoin_c::m_shapeAngle.y;
    }

    return SUCCEEDED;
}

int daEnStarCoinFake_c::draw() {
    changePosAngle(&mPos, &mAngle, 1);
    PSMTXTrans(mMatrix, mPos.x, mPos.y, mPos.z);
    mMatrix.YrotM(mAngle.y);
    mMatrix.XrotM(mAngle.x);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
    mModel.entry();

    return SUCCEEDED;
}

void daEnStarCoinFake_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    collect();
}

void daEnStarCoinFake_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    collect();
}

bool daEnStarCoinFake_c::hitCallback_Star(dCc_c *self, dCc_c *other) {
    collect();
    return true;
}

bool daEnStarCoinFake_c::hitCallback_Shell(dCc_c *self, dCc_c *other) {
    collect();
    return true;
}

bool daEnStarCoinFake_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    mEf::createEffect("Wm_en_explosion", 0, &mPos, nullptr, nullptr);
    mEf::createEffect("Wm_en_explosion_smk", 0, &mPos, nullptr, nullptr);

    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_OBJ_EMY_FIRE_DISAPP, soundPos, 0);

    deleteActor(1);
    return true;
}

bool daEnStarCoinFake_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    mEf::createEffect("Wm_ob_cmnicekira", 0, &mPos, nullptr, nullptr);
    mEf::createEffect("Wm_ob_icebreakwt", 0, &mPos, nullptr, nullptr);
    mEf::createEffect("Wm_ob_iceattack", 0, &mPos, nullptr, nullptr);

    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_OBJ_PNGN_ICE_BREAK, soundPos, 0);

    deleteActor(1);
    return true;
}

bool daEnStarCoinFake_c::hitCallback_YoshiFire(dCc_c *self, dCc_c *other) {
    mEf::createEffect("Wm_en_explosion", 0, &mPos, nullptr, nullptr);
    mEf::createEffect("Wm_en_explosion_smk", 0, &mPos, nullptr, nullptr);

    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_OBJ_EMY_FIRE_DISAPP, soundPos, 0);

    deleteActor(1);
    return true;
}

void daEnStarCoinFake_c::collect() {
    mEf::createEffect("Wm_en_obakedoor", 0, &mPos, nullptr, nullptr);

    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_OBJ_DDOOR_OPEN, soundPos, 0);

    deleteActor(1);
}