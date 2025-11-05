#include <kamek.h>
#include <new/bases/d_a_kabo_flame.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(AC_KABOKURIBO_FLAME, daKaboFlame_c, fProfile::JR_FLOOR_FIRE, fProfile::DRAW_ORDER::JR_FLOOR_FIRE, 0x22);

dCustomProfile_c kaboflameProfile(&g_profile_AC_KABOKURIBO_FLAME, "AC_KABOKURIBO_FLAME", fProfile::AC_KABOKURIBO_FLAME);

STATE_DEFINE(daKaboFlame_c, Burn);

void ccCallback_kaboFlame(dCc_c *cc1, dCc_c *cc2) {
    dActor_c *other = cc2->mpOwner;
    daKaboFlame_c *flame = (daKaboFlame_c*)cc1->mpOwner;
    if (other->mKind == dActor_c::STAGE_ACTOR_PLAYER) {
        daPlBase_c *pl = (daPlBase_c *)other;
        pl->setDamage(flame, daPlBase_c::DAMAGE_NONE);
        flame->deleteRequest();
    } else if (other->mKind == dActor_c::STAGE_ACTOR_YOSHI) {
        daYoshi_c *ys = (daYoshi_c *)other;
        s8 *playerNum = ys->getPlrNo();
        if (*playerNum > -1) {
            ys->setDamage(flame, daPlBase_c::DAMAGE_NONE);
            flame->deleteRequest();
        }
    }
    else if (cc2->mCcData.mAttackCategory == 2) {
        cc1->mFlag |= 2;
        flame->extinguish();
    }
}

sCcDatNewF l_kabo_flame_cc = {
    1.0,                        // mOffsetX
    4.0,                        // mOffsetY
    3.5,                        // mWidth
    3.5,                        // mHeight
    dCc_c::CAT_PROJECTILE,      // mCategory
    dCc_c::CAT_PROJECTILE,      // mAttackCategory
    0x45,                       // mCategoryInteract
    0x6,                        // mAttackCategoryInteract     
    0,                          // mFlag
    &ccCallback_kaboFlame,      // mCallback
};

int daKaboFlame_c::create() {
    mScale = mVec3_c(1.5, 1.5, 1.5);

    mCc.set(this, &l_kabo_flame_cc);
    mCc.entry();

    mTimer = 500;

    mStateMgr.changeState(StateID_Burn);

    return SUCCEEDED;
}

int daKaboFlame_c::execute() {
    mStateMgr.executeState();

    return SUCCEEDED;
}

int daKaboFlame_c::doDelete() {
    mCc.release();

    return SUCCEEDED;
}

void daKaboFlame_c::initializeState_Burn() {}

void daKaboFlame_c::finalizeState_Burn() {}

void daKaboFlame_c::executeState_Burn() {
    mFlameEffect.createEffect("Wm_en_fireplace", 0, &mPos, nullptr, &mScale);
    mFlameEffect2.createEffect("Wm_en_fireplace_ind", 0, &mPos, nullptr, &mScale);

    if (mTimer < 1) {
        deleteRequest();
    }
    mTimer--;
}

void daKaboFlame_c::extinguish() {
    mVec3_c effScale = mVec3_c(mScale.x * 0.3, mScale.y * 0.3, mScale.z * 0.3);
    mEf::createEffect("Wm_en_burst_m", 0, &mPos, nullptr, &effScale);
    mEf::createEffect("Wm_en_firesnk_icehitsmk_h", 0, &mPos, nullptr, &effScale);
    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjMap->startSound(SE_BOSS_CMN_MAGIC_EXTINCT, soundPos, 0);
    deleteRequest();
}