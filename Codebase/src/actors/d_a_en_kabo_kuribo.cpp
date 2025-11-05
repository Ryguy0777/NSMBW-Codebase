#include <kamek.h>
#include <new/bases/d_a_en_kabo_kuribo.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_game_com.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(EN_KABOKURIBO, daEnKaboKuribo_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

const char* jackogoombaArcList [] = {"kabokuribo", NULL};
const SpriteData jackogoombaSpriteData = {fProfile::EN_KABOKURIBO, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c jackogoombaProfile(&g_profile_EN_KABOKURIBO, "EN_KABOKURIBO", SpriteId::EN_KABOKURIBO, &jackogoombaSpriteData, jackogoombaArcList);

void daEnKaboKuribo_c::Normal_VsPlHitCheck(dCc_c *cc1, dCc_c *cc2) {
    dActor_c *player = cc2->mpOwner;
    int hitcheck = Enfumi_check(cc1, cc2, !mNoPumpkin);
    if (!mNoPumpkin) {
        if (hitcheck == 1) {
            boyonBegin();
        } else if (hitcheck == 3) {
            boyonBegin();
            mNoPumpkin = true;
            mEatBehaviour = EAT_TYPE_EAT_PERMANENT;
            mAnmVis.setFrame(1.0);
            mVec3_c centerPos = getCenterPos();
            mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, nullptr);
            fumiSE(player);
            fumiEffect(player);
        } else if (hitcheck == 0 && isDamageInvalid() == false) {
            dEn_c::Normal_VsPlHitCheck(cc1, cc2);
        }
    } else {
        if (hitcheck == 1) {
            reactFumiProc(player);
        } else if (hitcheck == 3) {
            reactSpinFumiProc(player);
        } else if (hitcheck == 0 && isDamageInvalid() == false) {
            dEn_c::Normal_VsPlHitCheck(cc1, cc2);
        }
    }
}

bool daEnKaboKuribo_c::hitCallback_HipAttk(dCc_c *cc1, dCc_c *cc2) {
    mAnmVis.setFrame(1.0);
    mVec3_c centerPos = getCenterPos();
    mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, nullptr);

    return dEn_c::hitCallback_HipAttk(cc1, cc2);
}

void daEnKaboKuribo_c::FumiScoreSet(dActor_c *actor) {
    if (mNoPumpkin) {
        return dEn_c::FumiScoreSet(actor);
    }
}

void daEnKaboKuribo_c::executeState_Walk() {
    daEnKuriboBase_c::executeState_Walk();
    if (!mNoPumpkin) {
        if (mAttackTimer < 1) {
            nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
            dAudio::g_pSndObjEmy->startSound(SE_OBJ_FIREBALL_DISAPP, soundPos, 0);
            construct(fProfile::AC_KABOKURIBO_FLAME, 0, &mPos, nullptr, mLayer);
            mAttackTimer = dGameCom::rndInt(1000);
        }
        mAttackTimer--;
    }
}

void daEnKaboKuribo_c::createBodyModel() {
    mRes = dResMng_c::m_instance->mRes.getRes("kabokuribo", "g3d/kabokuribo.brres");
    nw4r::g3d::ResMdl bmdl = mRes.GetResMdl("kabokuribo");
	mModel.create(bmdl, &mAllocator, 0x16B, 1, 0);
	dActor_c::setSoftLight_Enemy(mModel);

	nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
	mAnmChr.create(bmdl, resAnmChr, &mAllocator, 0);

    mResPat = mRes.GetResAnmTexPat("walk");
	mAnmTexPat.create(bmdl, mResPat, &mAllocator, 0, 1);

    nw4r::g3d::ResAnmVis resVis = mRes.GetResAnmVis("pumpkin_vis");
    mAnmVis.create(bmdl, resVis, &mAllocator, 0);
}

void daEnKaboKuribo_c::initialize() {
    daEnKuribo_c::initialize();

    mAttackTimer = dGameCom::rndInt(1000);
    mAnmVis.setFrame(0);

    mEatBehaviour = EAT_TYPE_FIREBALL;
}

void daEnKaboKuribo_c::playWalkAnm() {
    mAnmVis.play();
    return daEnKuribo_c::playWalkAnm();
}

void daEnKaboKuribo_c::setWalkAnm() {
    nw4r::g3d::ResAnmVis resVis = mRes.GetResAnmVis("pumpkin_vis");

    mAnmVis.setAnm(mModel, resVis, m3d::FORWARD_LOOP);
    mAnmVis.setRate(0.0);
    mModel.setAnm(mAnmVis, 1.0);

    return daEnKuribo_c::setWalkAnm();
}

bool daEnKaboKuribo_c::isBgmSync() const {
    return mNoPumpkin;
}