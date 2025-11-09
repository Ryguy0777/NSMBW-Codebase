#include <kamek.h>
#include <new/bases/d_a_en_large_kabochan.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_effect.hpp>

CUSTOM_ACTOR_PROFILE(EN_LARGE_KABOCHAN, daEnLargeKabochan_c, fProfile::EN_LARGE_KURIBO, fProfile::DRAW_ORDER::EN_LARGE_KURIBO, 0x12);

const char* bigSplunkinArcList [] = {"kabochan", NULL};
const SpriteData bigSplunkinSpriteData = {fProfile::EN_LARGE_KABOCHAN, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c bigSplunkinProfile(&g_profile_EN_LARGE_KABOCHAN, "EN_LARGE_KABOCHAN", SpriteId::EN_LARGE_KABOCHAN, &bigSplunkinSpriteData, bigSplunkinArcList);

bool daEnLargeKabochan_c::hitCallback_HipAttk(dCc_c *cc1, dCc_c *cc2) {
    daPlBase_c *player = (daPlBase_c *)cc2->mpOwner;
    u8 direction = getTrgToSrcDir_Main(player->getCenterX(), getCenterY());
    s8 *playerNo = player->getPlrNo();

    setDeathSound_HipAttk();
    mVec3_c effPos(mPos.x, getCenterY(), 5500.0);
    hipatkEffect(effPos);
    int comboCnt = dEnCombo_c::calcPlComboCnt(player);
    int score = mCombo.getComboScore(comboCnt);
    sDeathInfoData splunkinDeathInfo = {
        0.0, 0.0,
        0.0, 0.0,
        &StateID_DieOther,
        score,
        -1,
        direction,
        *playerNo
    };
    mDeathInfo.set(splunkinDeathInfo);
    return true;
}

bool daEnLargeKabochan_c::hitCallback_Fire(dCc_c *cc1, dCc_c *cc2) {
    if (mHitByFire) {
        return dEn_c::hitCallback_Fire(cc1, cc2);
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
    mVec3_c scale(1.5, 1.5, 1.5);
    dEf::createEffect_change("Wm_mr_softhit", 0, &pos, nullptr, &scale);
    mCc.release();
    deleteActor(mNoRespawn);
}

void daEnLargeKabochan_c::hipatkEffect(const mVec3_c &pos) {
    if (mCracked < 2) {
        dEf::createEffect_change("Wm_en_pumpkinbreak", 0, &pos, nullptr, &mScale);
    } else {
        mVec3_c scale(3.0, 3.0, 3.0);
        dEf::createEffect_change("Wm_en_pumpkinbreak_b", 0, &pos, nullptr, &scale);
    }
}

void daEnLargeKabochan_c::createIceActor() {
    dIceInfo splunkinIceInfo[1] = {
        3,                                      // mFlags
        mVec3_c(mPos.x, mPos.y-2.0, mPos.z),    // mPos
        mVec3_c(0.85, 0.87, 1.1),               // mScale
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0
    };
    mIceMng.createIce(&splunkinIceInfo[0], 1);
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
        mVec3_c scale(3.0, 3.0, 3.0);
        mBreakEffect.createEffect("Wm_en_pumpkinbreak_b", 0, &centerPos, nullptr, &scale);
        setDeathInfo_Fumi(player, mVec2_c(mSpeed.x, mSpeed.y), StateID_DieOther, 0);
    }
    mAnmChr.setRate(1.0 + (float)mCracked);
    mAnmClr.setFrame((float)mCracked, 0);
}

void daEnLargeKabochan_c::reactSpinFumiProc(dActor_c* player) {
    mVec3_c centerPos = getCenterPos();
    if (mCracked < 1) {
        mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &mScale);
        mCracked += 2;
        setWalkSpeed();
        boyonBegin();
    } else {
        mVec3_c scale(3.0, 3.0, 3.0);
        if (mCracked == 1) {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &scale);
        } else {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak_b", 0, &centerPos, nullptr, &scale);
        }
        setDeathInfo_Fumi(player, mVec2_c(mSpeed.x, mSpeed.y), StateID_DieOther, 0);
    }
    mAnmChr.setRate(1.0 + (float)mCracked);
    mAnmClr.setFrame((float)mCracked, 0);
}

void daEnLargeKabochan_c::reactYoshiFumiProc(dActor_c* yoshi) {
    mVec3_c centerPos = getCenterPos();
    if (mCracked < 1) {
        mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &mScale);
        mCracked += 2;
        setWalkSpeed();
        boyonBegin();
    } else {
        mVec3_c scale(3.0, 3.0, 3.0);
        if (mCracked == 1) {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, &scale);
        } else {
            mBreakEffect.createEffect("Wm_en_pumpkinbreak_b", 0, &centerPos, nullptr, &scale);
        }
        setDeathInfo_Fumi(yoshi, mVec2_c(mSpeed.x, mSpeed.y), StateID_DieOther, 0);
    }
    mAnmChr.setRate(2.0 + (float)mCracked);
    mAnmClr.setFrame((float)mCracked, 0);
}

void daEnLargeKabochan_c::createBodyModel() {
    mRes = dResMng_c::m_instance->mRes.getRes("kabochan", "g3d/kabochan.brres");
    nw4r::g3d::ResMdl bmdl = mRes.GetResMdl("kabochan");
	mModel.create(bmdl, &mAllocator, 0x32C, 1, 0);
	dActor_c::setSoftLight_Enemy(mModel);

	nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
	mAnmChr.create(bmdl, resAnmChr, &mAllocator, 0);

    nw4r::g3d::ResAnmClr resClr = mRes.GetResAnmClr("break");
	mAnmClr.create(bmdl, resClr, &mAllocator, 0, 1);
}

void daEnLargeKabochan_c::initialize() {
    daEnKuribo_c::initialize();

    mScale = mVec3_c(2.5, 2.5, 2.5);
    mCenterOffs = mVec3_c(0.0, 20.0, 0.0);

    mCc.mCcData.mOffsetY = 20;
    mCc.mCcData.mHeight = 20;
    mCc.mCcData.mWidth = 20;

    static const dBcSensorLine_c below(-16<<12, 16<<12, 0<<12);
    static const dBcSensorLine_c adjacent(20<<12, 10<<12, 20<<12);

    mBc.set(this, (dBcSensor_c*)&below, nullptr, (dBcSensor_c*)&adjacent);

    mVisibleAreaOffset.y = 20.0;
    mVisibleAreaSize = mVec2_c(40.0, 40.0);

    mEatBehaviour = 0;
}

void daEnLargeKabochan_c::setWalkSpeed() {
    mSpeed.x = l_large_splunkin_speeds[mCracked][mDirection];
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
    mModel.setAnm(mAnmChr, 2.0);
    mAnmChr.setRate(2.0);

    mAnmClr.setAnm(mModel, resClr, 0, m3d::FORWARD_LOOP);
    mAnmClr.setRate(0.0, 0);
    mAnmClr.setFrame(0, 0);
    mModel.setAnm(mAnmClr, 1.0);
}

bool daEnLargeKabochan_c::isBgmSync() const {
    return false;
}