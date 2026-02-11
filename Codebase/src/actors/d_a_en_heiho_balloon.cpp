#include <kamek.h>
#include <new/bases/d_a_en_heiho_balloon.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_a_player_manager.hpp>

CUSTOM_ACTOR_PROFILE(EN_HEIHO_BALLOON, daEnHeihoBalloon_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

STATE_DEFINE(daEnHeihoBalloon_c, Balloon);
STATE_DEFINE(daEnHeihoBalloon_c, BalloonFall);

const char* l_HEIHO_BALLOON_res[] = {"heiho", "obj_balloon", NULL};
const dActorData_c c_HEIHO_BALLOON_actor_data = {fProfile::EN_HEIHO_BALLOON, 8, -16, 0, 8, 16, 16, 0, 16, 0, 0, 0};
dCustomProfile_c l_HEIHO_BALLOON_profile(&g_profile_EN_HEIHO_BALLOON, "EN_HEIHO_BALLOON", SpriteId::EN_HEIHO_BALLOON, &c_HEIHO_BALLOON_actor_data, l_HEIHO_BALLOON_res);

int daEnHeihoBalloon_c::create() {
    mIsFloating = true;
    mIsBalloon = (mParam >> 18) & 1;
    mMovement = (BALLOON_MOVEMENT_e)(mParam >> 21 & 3);
    mBalloonDistance = (mParam >> 4) & 0xF;
    mPopSensor = (mParam >> 23) & 1;
    mBalloonColor = mParam & 0xF;

    if (mIsBalloon) {
        mBalloonScale = 1.2f;
    } else {
        mBalloonScale = 1.0f;
    }

    daEnHeiho_c::create();

    if (mIsBalloon) {
        mVisibleAreaSize.set(32.0f, 48.0f);
        mVisibleAreaOffset.set(0.0f, -4.0f);
    } else {
        mVisibleAreaSize.set(32.0f, 32.0f);
    }

    return SUCCEEDED;
}

void daEnHeihoBalloon_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    if (self == &mBalloonCc || (mIsBalloon && mIsFloating)) {
        Enfumi_check(self, other, 0);
        popBubble(other->mpOwner);
    } else {
        return daEnHeiho_c::Normal_VsPlHitCheck(self, other);
    }
}

void daEnHeihoBalloon_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    if (mIsBalloon) {
        popBubble(other->mpOwner);
        return daEnHeiho_c::Normal_VsYoshiHitCheck(self, other);
    } else if (self == &mBalloonCc) {
        Enfumi_check(self, other, 0);
        popBubble(other->mpOwner);
    } else {
        return daEnHeiho_c::Normal_VsYoshiHitCheck(self, other);
    }
}

bool daEnHeihoBalloon_c::hitCallback_Star(dCc_c *self, dCc_c *other) {
    if (self == &mBalloonCc) {
        popBubble(other->mpOwner);
        return true;
    }
    return dEn_c::hitCallback_Star(self, other);
}

bool daEnHeihoBalloon_c::hitCallback_Spin(dCc_c *self, dCc_c *other) {
    if (self == &mBalloonCc) {
        popBubble(other->mpOwner);
        return true;
    }
    return dEn_c::hitCallback_Spin(self, other);
}

bool daEnHeihoBalloon_c::hitCallback_HipAttk(dCc_c *self, dCc_c *other) {
    if (self == &mBalloonCc) {
        popBubble(other->mpOwner);
        return true;
    }
    return dEn_c::hitCallback_HipAttk(self, other);
}

bool daEnHeihoBalloon_c::hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other) {
    if (self == &mBalloonCc) {
        popBubble(other->mpOwner);
        return true;
    }
    return dEn_c::hitCallback_YoshiHipAttk(self, other);
}

bool daEnHeihoBalloon_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    if (self == &mBalloonCc) {
        popBubble(other->mpOwner);
        return true;
    }
    return dEn_c::hitCallback_Fire(self, other);
}

bool daEnHeihoBalloon_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    if (self == &mBalloonCc) {
        popBubble(other->mpOwner);
        return true;
    }
    return dEn_c::hitCallback_Ice(self, other);
}

void daEnHeihoBalloon_c::initializeState_DieFall() {
    if (mIsFloating) {
        popBubble(nullptr);
    }
    return daEnHeiho_c::initializeState_DieFall();
}

void daEnHeihoBalloon_c::initializeState_EatIn() {
    if (mIsFloating) {
        popBubble(nullptr);
    }
    return daEnHeiho_c::initializeState_EatIn();
}

bool daEnHeihoBalloon_c::createIceActor() {
    if (mIsFloating) {
        popBubble(nullptr);
    }
    return daEnHeiho_c::createIceActor();
}

void daEnHeihoBalloon_c::FumiScoreSet(dActor_c *actor) {
    if (!mIsBalloon && mIsFloating) {
        return;
    } else {
        return dEn_c::FumiScoreSet(actor);
    }
}

void daEnHeihoBalloon_c::fumiSE(dActor_c *actor){
    if (!mIsBalloon && mIsFloating) {
        return;
    } else {
        return dEn_c::fumiSE(actor);
    }
}

void daEnHeihoBalloon_c::fumiEffect(dActor_c *actor) {
    if (!mIsBalloon && mIsFloating) {
        return;
    } else {
        return dEn_c::fumiEffect(actor);
    }
}

void daEnHeihoBalloon_c::spinfumiSE(dActor_c *actor) {
    if (!mIsBalloon && mIsFloating) {
        return;
    } else {
        return dEn_c::spinfumiSE(actor);
    }
}

void daEnHeihoBalloon_c::spinfumiEffect(dActor_c *actor) {
    if (!mIsBalloon && mIsFloating) {
        return;
    } else {
        return dEn_c::spinfumiEffect(actor);
    }
}

void daEnHeihoBalloon_c::yoshifumiSE(dActor_c *actor) {
    if (!mIsBalloon && mIsFloating) {
        return;
    } else {
        return dEn_c::yoshifumiSE(actor);
    }
}

void daEnHeihoBalloon_c::yoshifumiEffect(dActor_c *actor) {
    if (!mIsBalloon && mIsFloating) {
        return;
    } else {
        return dEn_c::yoshifumiEffect(actor);
    }
}

void daEnHeihoBalloon_c::loadModel() {
    daEnHeiho_c::loadModel();
    if (mIsBalloon) {
        mBalloonRes = dResMng_c::m_instance->getRes("obj_balloon", "g3d/obj_balloon.brres");
        nw4r::g3d::ResMdl balloonmdl = mBalloonRes.GetResMdl("obj_balloon");
        mBalloonModel.create(balloonmdl, &mAllocator, 0x128, 1, nullptr);
        setSoftLight_MapObj(mBalloonModel);

        nw4r::g3d::ResAnmClr resClrBalloon = mBalloonRes.GetResAnmClr("color");
        mBalloonClr.create(balloonmdl, resClrBalloon, &mAllocator, 0, 1);
        mBalloonClr.setAnm(mBalloonModel, resClrBalloon, 0, m3d::FORWARD_LOOP);
        mBalloonModel.setAnm(mBalloonClr, 0.0f);
        mBalloonClr.setRate(0.0f, 0);
        mBalloonClr.setFrame(mBalloonColor, 0);

        nw4r::g3d::ResAnmTexSrt resSrtBalloon = mBalloonRes.GetResAnmTexSrt("nohimo");
        mBalloonSrt.create(balloonmdl, resSrtBalloon, &mAllocator, 0, 1);
        mBalloonSrt.setAnm(mBalloonModel, resSrtBalloon, 0, m3d::FORWARD_LOOP);
        mBalloonModel.setAnm(mBalloonSrt, 1.0f);
        mBalloonSrt.setRate(0.0f, 0);
        mBalloonSrt.setFrame(1.0f, 0);
    } else {
        mBalloonRes = dResMng_c::m_instance->getRes("balloon", "g3d/balloon.brres");
        nw4r::g3d::ResMdl bubblemdl = mBalloonRes.GetResMdl("balloon");
        mBalloonModel.create(bubblemdl, &mAllocator, 0x20, 1, nullptr);

        nw4r::g3d::ResAnmChr resAnmBalloon = mBalloonRes.GetResAnmChr("float");
        mBalloonAnm.create(bubblemdl, resAnmBalloon, &mAllocator, 0);
        mBalloonAnm.setAnm(mBalloonModel, resAnmBalloon, m3d::FORWARD_LOOP);
        mBalloonModel.setAnm(mBalloonAnm, 0.0f);
        mBalloonAnm.setRate(1.0f);

        nw4r::g3d::ResAnmTexSrt resSrtBalloon = mBalloonRes.GetResAnmTexSrt("float");
        mBalloonSrt.create(bubblemdl, resSrtBalloon, &mAllocator, 0, 1);
        mBalloonSrt.setAnm(mBalloonModel, resSrtBalloon, 0, m3d::FORWARD_LOOP);
        mBalloonModel.setAnm(mBalloonSrt, 1.0f);
        mBalloonSrt.setRate(1.0f, 0);
    }
}

void daEnHeihoBalloon_c::updateModel() {
    daEnHeiho_c::updateModel();
    if (!mIsBalloon) {
        mBalloonModel.play();
        mBalloonSrt.play();
    }
}

void daEnHeihoBalloon_c::drawModel() {
    mHeihoModel.entry();
    if (mIsFloating) {
        mBalloonModel.entry();
    }
}

void daEnHeihoBalloon_c::calcModel() {
    daEnHeiho_c::calcModel();
    if (mIsFloating) {
        mVec3_c balloonPos;
        if (mIsBalloon) {
            balloonPos.set(mPos.x, mPos.y-10.0f, mPos.z-60.0f);
        } else {
            balloonPos.set(mPos.x, mPos.y+10.0f, mPos.z+60.0f);
        }
        // Set matrix to world position
        PSMTXTrans(mMatrix, balloonPos.x, balloonPos.y, balloonPos.z);

        // Apply rotation vector
        mMatrix.YrotM(mAngle.y);
        mMatrix.XrotM(mAngle.x);
        mMatrix.ZrotM(mAngle.z);

        // Set the matrix for the model
        mBalloonModel.setLocalMtx(&mMatrix);
        mBalloonModel.setScale(mBalloonScale, mBalloonScale, mBalloonScale);
        mBalloonModel.calc(false);
    }
}

void daEnHeihoBalloon_c::setTurnByPlayerHit(dActor_c *player) {
    u8 direction = getTrgToSrcDir_Main(player->getCenterPos().x, getCenterPos().x);
    if (!isState(StateID_Balloon) && !isState(StateID_BalloonFall)
        && !isState(StateID_Sleep) && !isState(StateID_Dizzy)) {
        mDirection = direction;
        mAngle.y = l_base_angleY[mDirection];
    }
    if (isState(StateID_Turn)) {
        changeState(StateID_Walk);
    }
    if (isState(StateID_Walk) || isState(StateID_Turn)) {
        setWalkSpeed();
    }
}
void daEnHeihoBalloon_c::setInitialState() {
    changeState(StateID_Balloon);
}

void daEnHeihoBalloon_c::popBubble(dActor_c *player) {
    mIsFloating = false;
    mBalloonCc.release();

    if (mIsBalloon) {
        mVec3_c efPos(mPos.x, mPos.y-10.0f, 5500.0f);
        mEf::createEffect("Wm_en_explosion_ln", 0, &efPos, nullptr, nullptr);

        mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
        dAudio::g_pSndObjEmy->startSound(SE_OBJ_CMN_BALLOON_BREAK, soundPos, 0);
    } else {
        mVec3_c efPos(mPos.x, mPos.y+10.0f, 5500.0f);
        mEf::createEffect("Wm_mr_balloonburst", 0, &efPos, nullptr, nullptr);

        mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
        dAudio::g_pSndObjEmy->startSound(SE_OBJ_CMN_BALLOON_BREAK, soundPos, 0);
    }

    if (player != nullptr) {
        if (mType < HEIHO_TYPE_JUMPER || mType > HEIHO_TYPE_PACER) {
            mDirection = getTrgToSrcDir_Main(player->getCenterPos().x, getCenterPos().x);
        }
        changeState(StateID_BalloonFall);
    }
}

void daEnHeihoBalloon_c::popIfPlayerBelow() {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        // Grab any active players
        if (player) {
            // Are we in the same x range as the shyguy?
            if (48.0f >= std::fabs(player->mPos.x - mPos.x)) {
                // Are we in the y range?
                if (mPos.y >= player->mPos.y && player->mPos.y >= mPos.y - 104.0f) {
                    popBubble(player);
                }
            }
        }
    }
}

const sCcDatNewF l_heiho_balloon_cc = {
    {0.0f, 10.5f},
    {15.0f, 15.0f},
    CC_KIND_BALLOON,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
    BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_BALLOON) | BIT_FLAG(CC_KIND_TAMA),
    0xFF80A086,
    CC_STATUS_NONE,
    &dEn_c::normal_collcheck,
};

void daEnHeihoBalloon_c::initializeState_Balloon() {
    if (mIsBalloon) {
        playChrAnim("balloon", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    } else {
        if (mMovement > MOVEMENT_NONE) {
            playChrAnim("bubble2", m3d::FORWARD_LOOP, 0.0f, 1.0f);
        } else {
            playChrAnim("bubble", m3d::FORWARD_LOOP, 0.0f, 1.0f);
        }
    }

    mAngle.y = 0;

    mBalloonBaseline.set(mPos.x, mPos.y);
    mTimer = 0;

    mBalloonCc.set(this, (sCcDatNewF *)&l_heiho_balloon_cc);
    mBalloonCc.entry();

    if (mIsBalloon) {
        mBalloonCc.mCcData.mOffset.y = -10.0f;
    }
}

void daEnHeihoBalloon_c::finalizeState_Balloon() {};

void daEnHeihoBalloon_c::executeState_Balloon() {
    updateModel();

    switch (mMovement) {
        case MOVEMENT_HORIZONTAL:
            mPos.x = mBalloonBaseline.x + (EGG::Mathf::sin(mTimer * M_PI / 600.0f) * (mBalloonDistance * 16.0f));
            mPos.y = mBalloonBaseline.y + (EGG::Mathf::sin(mTimer * M_PI / 60.0f) * 3.0f);
            break;
        
        case MOVEMENT_VERTICAL:
            mPos.x = mBalloonBaseline.x + (EGG::Mathf::sin(mTimer * M_PI / 60.0f) * 3.0f);
            mPos.y = mBalloonBaseline.y + (EGG::Mathf::sin(mTimer * M_PI / 600.0f) * (mBalloonDistance * 16.0f));
            break;
        
        case MOVEMENT_CIRCLE:
            mPos.x = mBalloonBaseline.x + (EGG::Mathf::sin(mTimer * M_PI / 600.0f) * (mBalloonDistance * 16.0f));
            mPos.y = mBalloonBaseline.y + (EGG::Mathf::cos(mTimer * M_PI / 600.0f) * (mBalloonDistance * 16.0f));
            break;
        
        default:
            mPos.y = mBalloonBaseline.y + EGG::Mathf::sin(mTimer * M_PI / 60.0f);
            break;
    }

    if (!mIsBalloon && mPopSensor) {
        popIfPlayerBelow();
    }

    mTimer++;

    WaterCheck(mPos, 1.0f);
}

void daEnHeihoBalloon_c::initializeState_BalloonFall() {
    playChrAnim("fall", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    mSpeed.x = 0.0f;
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;
}

void daEnHeihoBalloon_c::finalizeState_BalloonFall() {}

void daEnHeihoBalloon_c::executeState_BalloonFall() {
    updateModel();
    calcSpeedY();
    posMove();

    if (EnBgCheck() & 1) {
        // Set pacer final distances
        if (mType == HEIHO_TYPE_PACER) {
            float finalOffset = 16.0f * mDistance - 8.0f;
            mFinalPos[0] = mPos.x + finalOffset;
            mFinalPos[1] = mPos.x - finalOffset;
        }

        mVisibleAreaSize.set(16.0f, 24.0f);
        mVisibleAreaOffset.set(0.0f, 12.0f);

        switch (mType) {
            case HEIHO_TYPE_WALKER:
            case HEIHO_TYPE_WALKER_LEDGE:
            case HEIHO_TYPE_PACER:
                mAngle.y = l_base_angleY[mDirection];
                changeState(StateID_Walk);
                break;

            case HEIHO_TYPE_SLEEPER:
                changeState(StateID_Sleep);
                break;

            case HEIHO_TYPE_JUMPER:
                mAngle.y = l_base_angleY[mDirection];
                changeState(StateID_Jump);
                break;

            default:
                mAngle.y = l_base_angleY[mDirection];
                changeState(StateID_Idle);
                break;
        }
    }
}