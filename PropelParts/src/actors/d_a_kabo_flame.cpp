#include <kamek.h>
#include <new/bases/d_a_kabo_flame.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(AC_KABOKURIBO_FLAME, daKaboFlame_c, fProfile::JR_FLOOR_FIRE, fProfile::DRAW_ORDER::JR_FLOOR_FIRE, 0x22);

dCustomProfile_c l_KABOKURIBO_FLAME_profile(&g_profile_AC_KABOKURIBO_FLAME, "AC_KABOKURIBO_FLAME", fProfile::AC_KABOKURIBO_FLAME);

STATE_DEFINE(daKaboFlame_c, Burn);

void ccCallback_kaboFlame(dCc_c *self, dCc_c *other) {
    dActor_c *ac = other->mpOwner;
    daKaboFlame_c *flame = (daKaboFlame_c*)self->mpOwner;
    if (ac->mKind == dActor_c::STAGE_ACTOR_PLAYER) {
        daPlBase_c *pl = (daPlBase_c *)other;
        pl->setDamage(flame, daPlBase_c::DAMAGE_NONE);
        flame->deleteRequest();
    } else if (ac->mKind == dActor_c::STAGE_ACTOR_YOSHI) {
        daYoshi_c *ys = (daYoshi_c *)other;
        s8 *playerNum = ys->getPlrNo();
        if (*playerNum > -1) {
            ys->setDamage(flame, daPlBase_c::DAMAGE_NONE);
            flame->deleteRequest();
        }
    } else if (other->mCcData.mAttack == CC_ATTACK_ICEBALL) {
        self->mInfo |= CC_NO_HIT;
        flame->extinguish();
    }
}

const sCcDatNewF l_kabo_flame_cc = {
    {1.0f, 4.0f},
    {3.5f, 3.5f},
    CC_KIND_TAMA,
    CC_ATTACK_KOOPA_FIRE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_TAMA),
    0x6,
    CC_STATUS_NONE,
    &ccCallback_kaboFlame,
};

int daKaboFlame_c::create() {
    mScale.set(1.5, 1.5, 1.5);

    mCc.set(this, (sCcDatNewF *)&l_kabo_flame_cc);
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
    mVec3_c effScale(mScale.x * 0.3f, mScale.y * 0.3f, mScale.z * 0.3f);
    mEf::createEffect("Wm_en_burst_m", 0, &mPos, nullptr, &effScale);
    mEf::createEffect("Wm_en_firesnk_icehitsmk_h", 0, &mPos, nullptr, &effScale);
    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjMap->startSound(SE_BOSS_CMN_MAGIC_EXTINCT, soundPos, 0);
    deleteRequest();
}