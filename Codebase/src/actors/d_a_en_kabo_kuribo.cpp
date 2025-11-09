#include <kamek.h>
#include <new/bases/d_a_en_kabo_kuribo.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_game_com.hpp>
#include <constants/sound_list.h>

STATE_DEFINE(daEnKaboKuribo_c, Attack);

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
    if (!mNoPumpkin) {
        mVec3_c centerPos = getCenterPos();
        mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, nullptr);
    }

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
           changeState(StateID_Attack);
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

    nw4r::g3d::ResAnmChr resAnmChrFlame= mRes.GetResAnmChr("flame");
	mAnmChrFlame.create(bmdl, resAnmChrFlame, &mAllocator, 0);

    mAnmChrBlend.create(bmdl, 2, &mAllocator, 0);
    mAnmChrBlend.attach(0, &mAnmChr, 1.0);
}

void daEnKaboKuribo_c::initialize() {
    daEnKuribo_c::initialize();

    mAttackTimer = mAttackTimer = dGameCom::rndInt(200) + 450;
    nw4r::g3d::ResAnmVis resVis = mRes.GetResAnmVis("pumpkin_vis");

    mAnmVis.setAnm(mModel, resVis, m3d::FORWARD_LOOP);
    mAnmVis.setRate(0.0);
    mModel.setAnm(mAnmVis, 1.0);
    mAnmVis.setFrame(0);

    mEatBehaviour = EAT_TYPE_FIREBALL;
}

void daEnKaboKuribo_c::playWalkAnm() {
    mAnmVis.play();
    return daEnKuribo_c::playWalkAnm();
}

void daEnKaboKuribo_c::setWalkAnm() {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");

    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mAnmChr.setRate(2.0);
    mModel.setAnm(mAnmChrBlend, 2.0);
}

bool daEnKaboKuribo_c::isBgmSync() const {
    return mNoPumpkin;
}

void daEnKaboKuribo_c::setFlameAnm() {
    nw4r::g3d::ResAnmChr resAnmChrFlame = mRes.GetResAnmChr("flame");

    mAnmChrFlame.setAnm(mModel, resAnmChrFlame, m3d::FORWARD_ONCE);
    mAnmChrFlame.setRate(1.0);
    mAnmChrFlame.setFrame(0);

    mAnmChrBlend.attach(1, &mAnmChrFlame, 1.0);

    mModel.setAnm(mAnmChrBlend, 0.0);
}

void daEnKaboKuribo_c::initializeState_Attack() {
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_OBJ_FIREBALL_DISAPP, soundPos, 0);
    construct(fProfile::AC_KABOKURIBO_FLAME, 0, &mPos, nullptr, mLayer);

    setFlameAnm();
    setWalkSpeed();
}

void daEnKaboKuribo_c::finalizeState_Attack() {
    mAnmChrBlend.detach(1);
    mAttackTimer = dGameCom::rndInt(200) + 450;
}

void daEnKaboKuribo_c::executeState_Attack() {
    playWalkAnm();
    calcSpeedY();
    posMove();

    if ((EnBgCheck() & 1) == 0) {
        if (mBc.isFoot() && (mInLiquid == false) && (mSpeed.y <= 0.0f)) {
            mFootPush2.x = mFootPush2.x + m_1eb.x;
        }
    } else {
        mFootPush2.x = 0.0;
        mSpeed.y = 0.0;
    }
    if (mAnmChrFlame.isStop()) {
        changeState(StateID_Walk);
    }
    if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
        changeState(StateID_Turn);
    }
    killIfTouchingLava(mPos, 1.0);
    return;
}