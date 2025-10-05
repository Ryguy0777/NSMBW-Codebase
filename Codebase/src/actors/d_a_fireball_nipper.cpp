#include <kamek.h>
#include <new/bases/d_a_fireball_nipper.hpp>
#include <new/new_profile.hpp>

CUSTOM_ACTOR_PROFILE(AC_PAKKUN_PUCHI_FIRE, daFireBall_Nipper_c, fProfile::PAKKUN_FIREBALL, fProfile::DRAW_ORDER::PAKKUN_FIREBALL, 0x22);

dCustomProfile_c nipperFireProfile(&g_profile_AC_PAKKUN_PUCHI_FIRE, "AC_PAKKUN_PUCHI_FIRE", fProfile::AC_PAKKUN_PUCHI_FIRE);

void daFireBall_Nipper_c::executeState_Move() {
    calcSpeedY();
    posMove();

    if (mBc.checkFoot() || mBc.checkHead(0) || mBc.checkWall(0)) {
        kill();
        mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
        dAudio::g_pSndObjMap->startSound(SE_OBJ_EMY_FIRE_DISAPP, soundPos, 1);
    }
}

bool daFireBall_Nipper_c::initialize() {
    mDirection = mParam & 1;
    int distance = mParam >> 4 & 0xF;
    float baseXSpeed = l_nipper_fire_speeds[distance];
    mSpeed.x = (mDirection) ? -baseXSpeed : baseXSpeed;
    mSpeed.y = 3.0;
    mAccelY = -0.1875;
    mSpeedMax.y = -4.0;
    return true;
}

extern "C" void ccCallback_BrosFire(dCc_c *, dCc_c *);

dCc_c::CcData_s l_nipper_fire_cc = {
    0.0,                        // mOffsetX
    0.0,                        // mOffsetY
    3.0,                        // mWidth
    3.0,                        // mHeight
    dCc_c::CAT_ENTITY,          // mCategory
    0,                          // mAttackCategory
    0xF,                        // mCategoryInteract
    0x8800,                     // mAttackCategoryInteract     
    0,                          // mFlag
    &ccCallback_BrosFire,       // mCallback
};

void daFireBall_Nipper_c::setCc() {
    mCc.registerCc(this, &l_nipper_fire_cc);
    mCc.entry();
}

extern "C" void continueToMeltIce(void);
kmBranchDefAsm(0x807f8104, 0x807f812c) {
    nofralloc

    cmpwi r4, 0x13
    beq meltIce // broken check? not sure
    cmpwi r4, 758
    bne dontMelt

    meltIce:
    b continueToMeltIce

    dontMelt:
    blr
}