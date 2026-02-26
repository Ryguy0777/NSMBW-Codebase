#include <kamek.h>
#include <new/bases/d_a_en_bomb_bros.hpp>
#include <new/bases/d_custom_profile.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_bg_parameter.hpp>

CUSTOM_ACTOR_PROFILE(EN_BOMBBROS, daEnBombBros_c, fProfile::EN_HAMMERBROS, fProfile::DRAW_ORDER::EN_HAMMERBROS, 0x12);

const char* l_BOMBBROS_res [] = {"bros_bombhei", "bros", NULL};
const dActorData_c c_BOMBROS_actor_data = {fProfile::EN_BOMBBROS, 8, -16, 0, 16, 8, 16, 0, 0, 0, 0, 0};
dCustomProfile_c l_BOMBBROS_profile(&g_profile_EN_BOMBBROS, "EN_BOMBBROS", CourseActor::EN_BOMBBROS, &c_BOMBROS_actor_data, l_BOMBBROS_res);

daEnBombBros_c::daEnBombBros_c() : daEnBrosBase_c() {
    mNodeCallback.mpOwner = this;
}

bool daEnBombBros_c::hitCallback_Rolling(dCc_c *self, dCc_c *other) {
    if (other->mpOwner->mProfName == fProfile::BROS_BOMB) {
        daBombProjectile_c *bomb = (daBombProjectile_c *)other->mpOwner;
        if (!bomb->mPlayerBomb) {
            return true;
        }
    }
    return dEn_c::hitCallback_Rolling(self, other);
}

void daEnBombBros_c::initializeState_Ice() {
    daBombProjectile_c *bomb = getProjectile();
    if (bomb) {
        bomb->deleteActor(0);
    }
    return dEn_c::initializeState_Ice();
}

double daEnBombBros_c::getCreateWeaponFrm() const {
    return 1.875;
}

double daEnBombBros_c::getAttackFrm() const {
    return 30.0;
}

void daEnBombBros_c::setSpeed() {
    mSpeed.x = (mFacingDirection) ? -0.6f : 0.6f;
    mSpeed.y = 0.0f;
}

int daEnBombBros_c::checkAtkArea() {
    float bgParameterY = dBgParameter_c::ms_Instance_p->mPos.y;
    float bgParameterX = dBgParameter_c::ms_Instance_p->mPos.x;
    if (bgParameterY - 16.0f < mPos.y) {
        return false;
    }
    if ((mPos.y + (mVisibleAreaSize.y * 16.0f)) < (bgParameterY - dBgParameter_c::ms_Instance_p->mSize.y - 32.0f)) {
        return false;
    }
    if ((mPos.x + mVisibleAreaSize.x) < (bgParameterX)) {
        return false;
    }
    return (mPos.x - mVisibleAreaSize.x) <= (bgParameterX + dBgParameter_c::ms_Instance_p->mSize.x);
}

int daEnBombBros_c::getColor() {
    return 4;
}

void daEnBombBros_c::setAtkTimer() {
    mAttackTimer = dGameCom::rndInt(0x32) + 10;
}

void daEnBombBros_c::weaponCreate() {
    mVec3_c bombVec;
    mMtx1.multVecZero(bombVec);
    dActor_c *bomb = construct(fProfile::BROS_BOMB, mUniqueID, &bombVec, nullptr, mLayer);
    mWeaponID = bomb->mUniqueID;
}

void daEnBombBros_c::weaponAttack() {
    daBombProjectile_c *bomb = getProjectile();
    if (bomb) {
        bomb->mReadyToThrow = true;
        mWeaponID = BASE_ID_NULL;
    }
}

int bombBroCountdown[8] = {10, 20, 30, 50, 40, 30, 50, 60};

void daEnBombBros_c::setJumpCnt() {
    mJumpTimer = (bombBroCountdown[dGameCom::rndInt(8)] + 60);
}

void daEnBombBros_c::setJump() {
    if (dGameCom::rndInt(7) == 0) {
        mSpeed.y = 3.5f;
    } else {
        mSpeed.y = 2.5f;
    }
}

void daEnBombBros_c::beginJump() {
    setJump();
    changeState(StateID_AirAttack);
}

daBombProjectile_c *daEnBombBros_c::getProjectile() {
    return (daBombProjectile_c*)fManager_c::searchBaseByID(mWeaponID);
};