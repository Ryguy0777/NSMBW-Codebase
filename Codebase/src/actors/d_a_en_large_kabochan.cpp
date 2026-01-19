#include <kamek.h>
#include <new/bases/d_a_en_large_kabochan.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_effect.hpp>

CUSTOM_ACTOR_PROFILE(EN_LARGE_KABOCHAN, daEnLargeKabochan_c, fProfile::EN_LARGE_KURIBO, fProfile::DRAW_ORDER::EN_LARGE_KURIBO, 0x12);

const char* l_LARGE_KABOCHAN_res [] = {"kabochan", NULL};
const dActorData_c c_LARGE_KABOCHAN_actor_data = {fProfile::EN_LARGE_KABOCHAN, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_LARGE_KABOCHAN_profile(&g_profile_EN_LARGE_KABOCHAN, "EN_LARGE_KABOCHAN", SpriteId::EN_LARGE_KABOCHAN, &c_LARGE_KABOCHAN_actor_data, l_LARGE_KABOCHAN_res);

const float daEnLargeKabochan_c::smc_WALK_SPEED = 0.5f;
const float daEnLargeKabochan_c::smc_WALK_SPEED_CRACK = 0.75f;
const float daEnLargeKabochan_c::smc_WALK_SPEED_CRACK2 = 1.0f;

const sBcSensorLine l_large_kabochan_foot = { 1, -0x10000, 0x10000, 0 };
const sBcSensorLine l_large_kabochan_wall = { 1, 0x14000, 0x0A000, 0x14000 };

bool daEnLargeKabochan_c::hitCallback_HipAttk(dCc_c *self, dCc_c *other) {
    daPlBase_c *player = (daPlBase_c *)other->mpOwner;
    u8 direction = getTrgToSrcDir_Main(player->getCenterPos().x, getCenterPos().y);
    s8 *playerNo = player->getPlrNo();

    setDeathSound_HipAttk();
    mVec3_c effPos(mPos.x, getCenterPos().y, 5500.0);
    hipatkEffect(effPos);
    int comboCnt = dEnCombo_c::calcPlComboCnt(player);
    int score = mCombo.getComboScore(comboCnt);
    sDeathInfoData splunkinDeathInfo = {
        0.0f, 0.0f,
        0.0f, 0.0f,
        &StateID_DieOther,
        score,
        -1,
        direction,
        *playerNo
    };
    mDeathInfo = splunkinDeathInfo;
    return true;
}

bool daEnLargeKabochan_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    if (mHitByFire) {
        return dEn_c::hitCallback_Fire(self, other);
    } else {
        mHitByFire = true;
        boyonBegin();
        nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
        dAudio::g_pSndObjEmy->startSound(SE_EMY_KURIBO_M_DAMAGE, soundPos, 0);
    }
    return true;
}

void daEnLargeKabochan_c::setDeathSound_HipAttk() {
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_KABO_BREAK, soundPos, 0);
}

void daEnLargeKabochan_c::initializeState_DieOther() {
    mVec3_c pos = getCenterPos();
    mVec3_c scale(1.5f, 1.5f, 1.5f);
    dEf::createEffect_change("Wm_mr_softhit", 0, &pos, nullptr, &scale);
    mCc.release();
    deleteActor(mNoRespawn);
}

void daEnLargeKabochan_c::hipatkEffect(const mVec3_c &pos) {
    if (mCracked < 2) {
        dEf::createEffect_change("Wm_en_pumpkinbreak", 0, &pos, nullptr, &mScale);
    } else {
        mVec3_c scale(3.0f, 3.0f, 3.0f);
        dEf::createEffect_change("Wm_en_pumpkinbreak_b", 0, &pos, nullptr, &scale);
    }
}

bool daEnLargeKabochan_c::createIceActor() {
    dIceInfo splunkinIceInfo[1] = {
        3,                                      // mFlags
        mVec3_c(mPos.x, mPos.y-2.0f, mPos.z),   // mPos
        mVec3_c(0.85f, 0.87f, 1.1f),            // mScale
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f
    };
    return mIceMng.createIce(&splunkinIceInfo[0], 1);
}

void daEnLargeKabochan_c::fumiSE(dActor_c *actor) {
    ulong sfxID = (mCracked == 2) ? SE_EMY_KABO_BREAK : SE_EMY_KABO_HIBI;
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(getCenterPos());
    dAudio::g_pSndObjEmy->startSound(sfxID, soundPos, 0);
}

void daEnLargeKabochan_c::spinfumiSE(dActor_c *actor) {
    ulong sfxID = (mCracked > 0) ? SE_EMY_KABO_BREAK : SE_EMY_KABO_HIBI;
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(getCenterPos());
    dAudio::g_pSndObjEmy->startSound(sfxID, soundPos, 0);
}

void daEnLargeKabochan_c::reactFumiProc(dActor_c* player) {
    mVec3_c centerPos = getCenterPos();
    if (mCracked < 2) {
        mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &mScale);
        mCracked++;
        setWalkSpeed();
        boyonBegin();
    } else {
        mVec3_c scale(3.0f, 3.0f, 3.0f);
        mBreakEffect.createEffect("Wm_en_pumpkinbreak_b", 0, &centerPos, nullptr, &scale);
        setDeathInfo_Fumi(player, mVec2_c(mSpeed.x, mSpeed.y), StateID_DieOther, 0);
    }
    mAnmChr.setRate(1.0f + mCracked);
    mAnmClr.setFrame(mCracked, 0);
}

void daEnLargeKabochan_c::reactSpinFumiProc(dActor_c* player) {
    mVec3_c centerPos = getCenterPos();
    if (mCracked < 1) {
        mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &mScale);
        mCracked += 2;
        setWalkSpeed();
        boyonBegin();
    } else {
        mVec3_c scale(3.0f, 3.0f, 3.0f);
        if (mCracked == 1) {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &scale);
        } else {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak_b", 0, &centerPos, nullptr, &scale);
        }
        setDeathInfo_Fumi(player, mVec2_c(mSpeed.x, mSpeed.y), StateID_DieOther, 0);
    }
    mAnmChr.setRate(1.0 + mCracked);
    mAnmClr.setFrame(mCracked, 0);
}

void daEnLargeKabochan_c::reactYoshiFumiProc(dActor_c* yoshi) {
    mVec3_c centerPos = getCenterPos();
    if (mCracked < 1) {
        mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &mScale);
        mCracked += 2;
        setWalkSpeed();
        boyonBegin();
    } else {
        mVec3_c scale(3.0f, 3.0f, 3.0f);
        if (mCracked == 1) {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &scale);
        } else {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak_b", 0, &centerPos, nullptr, &scale);
        }
        setDeathInfo_Fumi(yoshi, mVec2_c(mSpeed.x, mSpeed.y), StateID_DieOther, 0);
    }
    mAnmChr.setRate(2.0f + mCracked);
    mAnmClr.setFrame(mCracked, 0);
}

void daEnLargeKabochan_c::createBodyModel() {
    mRes = dResMng_c::m_instance->getRes("kabochan", "g3d/kabochan.brres");
    nw4r::g3d::ResMdl bmdl = mRes.GetResMdl("kabochan");
	mModel.create(bmdl, &mAllocator, 0x32C, 1, 0);
	setSoftLight_Enemy(mModel);

	nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
	mAnmChr.create(bmdl, resAnmChr, &mAllocator, 0);

    nw4r::g3d::ResAnmClr resClr = mRes.GetResAnmClr("break");
	mAnmClr.create(bmdl, resClr, &mAllocator, 0, 1);
}

void daEnLargeKabochan_c::initialize() {
    daEnKuribo_c::initialize();

    mScale = mVec3_c(2.5f, 2.5f, 2.5f);
    mCenterOffs = mVec3_c(0.0f, 20.0f, 0.0f);

    mCc.mCcData.mOffset.y = 20.0f;
    mCc.mCcData.mSize.set(20.0f, 20.0f);

    mBc.set(this, l_large_kabochan_foot, nullptr, l_large_kabochan_wall);

    mVisibleAreaOffset.y = 20.0f;
    mVisibleAreaSize.set(40.0f, 40.0f);

    mEatBehaviour = EAT_TYPE_NONE;
}

void daEnLargeKabochan_c::setWalkSpeed() {
    static const float dirSpeed[3][2] = {
        { smc_WALK_SPEED, -smc_WALK_SPEED },
        { smc_WALK_SPEED_CRACK, -smc_WALK_SPEED_CRACK },
        { smc_WALK_SPEED_CRACK2, -smc_WALK_SPEED_CRACK2 }
    };
    mSpeed.x = dirSpeed[mCracked][mDirection];
    return;
}

void daEnLargeKabochan_c::playWalkAnm() {
    mModel.play();
    mAnmClr.play();
    return;
}

void daEnLargeKabochan_c::setWalkAnm() {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
    nw4r::g3d::ResAnmClr resClr = mRes.GetResAnmClr("break");

    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 2.0f);
    mAnmChr.setRate(2.0f);

    mAnmClr.setAnm(mModel, resClr, 0, m3d::FORWARD_LOOP);
    mAnmClr.setRate(0.0f, 0);
    mAnmClr.setFrame(0.0f, 0);
    mModel.setAnm(mAnmClr, 1.0f);
}

bool daEnLargeKabochan_c::isBgmSync() const {
    return false;
}