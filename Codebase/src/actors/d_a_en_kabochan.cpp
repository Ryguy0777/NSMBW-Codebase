#include <kamek.h>
#include <new/bases/d_a_en_kabochan.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_effect.hpp>

CUSTOM_ACTOR_PROFILE(EN_KABOCHAN, daEnKabochan_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

const char* l_KABOCHAN_res [] = {"kabochan", NULL};
const dActorData_c c_KABOCHAN_actor_data = {fProfile::EN_KABOCHAN, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_KABOCHAN_profile(&g_profile_EN_KABOCHAN, "EN_KABOCHAN", SpriteId::EN_KABOCHAN, &c_KABOCHAN_actor_data, l_KABOCHAN_res);

bool daEnKabochan_c::hitCallback_HipAttk(dCc_c *self, dCc_c *other) {
    daPlBase_c *player = (daPlBase_c *)other->mpOwner;
    u8 direction = getTrgToSrcDir_Main(player->getCenterPos().x, getCenterPos().y);
    s8 *playerNo = player->getPlrNo();

    setDeathSound_HipAttk();
    mVec3_c effPos(mPos.x, getCenterPos().y, 5500.0f);
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

void daEnKabochan_c::setDeathSound_HipAttk() {
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_KABO_BREAK, soundPos, 0);
}

void daEnKabochan_c::initializeState_DieOther() {
    mCc.release();
    deleteActor(mNoRespawn);
}

void daEnKabochan_c::hipatkEffect(const mVec3_c &pos) {
    dEf::createEffect_change("Wm_mr_softhit", 0, &pos, nullptr, nullptr);
    mVec3_c scale(1.5f, 1.5f, 1.5f);
    if (!mIsCracked) {
        dEf::createEffect_change("Wm_en_pumpkinbreak", 0, &pos, nullptr, nullptr);
    } else {
        dEf::createEffect_change("Wm_en_pumpkinbreak_b", 0, &pos, nullptr, &scale);
    }
}

bool daEnKabochan_c::createIceActor() {
    dIceInfo splunkinIceInfo[1] = {
        0,                                      // mFlags
        mVec3_c(mPos.x, mPos.y-2.0, mPos.z),    // mPos
        mVec3_c(1.2, 1.2, 1.4),                 // mScale
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0
    };
    return mIceMng.createIce(&splunkinIceInfo[0], 1);
}

void daEnKabochan_c::fumiSE(dActor_c *actor) {
    ulong sfxID = (mIsCracked) ? SE_EMY_KABO_BREAK : SE_EMY_KABO_HIBI;
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(getCenterPos());
    dAudio::g_pSndObjEmy->startSound(sfxID, soundPos, 0);
}

void daEnKabochan_c::reactFumiProc(dActor_c* player) {
    mVec3_c centerPos = getCenterPos();
    if (!mIsCracked) {
        mBreakEffect.createEffect("Wm_en_pumpkinbreak", 0, &centerPos, nullptr, nullptr);
        mIsCracked = true;
        mAnmChr.setRate(3.0f);
        mAnmClr.setFrame(2.0, 0);
        setWalkSpeed();
        boyonBegin();
    } else {
        mVec3_c scale(1.5, 1.5, 1.5);
        mBreakEffect.createEffect("Wm_en_pumpkinbreak_b", 0, &centerPos, nullptr, &scale);
        setDeathInfo_Fumi(player, mVec2_c(mSpeed.x, mSpeed.y), StateID_DieOther, 0);
    }
}

void daEnKabochan_c::createBodyModel() {
    mRes = dResMng_c::m_instance->getRes("kabochan", "g3d/kabochan.brres");
    nw4r::g3d::ResMdl bmdl = mRes.GetResMdl("kabochan");
	mModel.create(bmdl, &mAllocator, 0x32C, 1, 0);
	setSoftLight_Enemy(mModel);

	nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
	mAnmChr.create(bmdl, resAnmChr, &mAllocator, 0);

    nw4r::g3d::ResAnmClr resClr = mRes.GetResAnmClr("break");
	mAnmClr.create(bmdl, resClr, &mAllocator, 0, 1);
}

void daEnKabochan_c::setWalkSpeed() {
    mSpeed.x = l_splunkin_speeds[mIsCracked][mDirection];
    return;
}

void daEnKabochan_c::playWalkAnm() {
    mModel.play();
    mAnmClr.play();
    return;
}

void daEnKabochan_c::setWalkAnm() {
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