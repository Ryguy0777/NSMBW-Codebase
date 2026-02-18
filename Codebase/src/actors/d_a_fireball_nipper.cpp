#include <kamek.h>
#include <new/bases/d_a_fireball_nipper.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(AC_PAKKUN_PUCHI_FIRE, daFireBall_Nipper_c, fProfile::PAKKUN_FIREBALL, fProfile::DRAW_ORDER::PAKKUN_FIREBALL, 0x22);

dCustomProfile_c l_PAKKUN_PUCHI_FIRE_profile(&g_profile_AC_PAKKUN_PUCHI_FIRE, "AC_PAKKUN_PUCHI_FIRE", fProfile::AC_PAKKUN_PUCHI_FIRE);

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
    mSpeed.y = 3.0f;
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;
    return true;
}

extern "C" void ccCallback_BrosFire(dCc_c *, dCc_c *);

sCcDatNewF l_nipper_fire_cc = {
    {0.0f, 0.0f},
    {3.0f, 3.0f},
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
    BIT_FLAG(CC_KIND_ENEMY),
    0x8800,    
    CC_STATUS_NONE,
    &ccCallback_BrosFire,
};

void daFireBall_Nipper_c::setCc() {
    mCc.set(this, &l_nipper_fire_cc);
    mCc.entry();
}

#if GAME_REVISION < GAME_REVISION_K
#define NIPPER_FIRE_ID 758
#elif GAME_REVISION < GAME_REVISION_C
#define NIPPER_FIRE_ID 760
#else
#define NIPPER_FIRE_ID 762
#endif

extern "C" void continueToMeltIce(void);
kmBranchDefAsm(0x807f8104, 0x807f812c) {
    nofralloc

    cmpwi r4, 19 // YOSHI_FIRE 
    beq meltIce // Broken check? not sure
    cmpwi r4, NIPPER_FIRE_ID // AC_PAKKUN_PUCHI_FIRE
    bne dontMelt

    meltIce:
    b continueToMeltIce

    dontMelt:
    blr
}