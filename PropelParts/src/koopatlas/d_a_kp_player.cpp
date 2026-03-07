#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_a_kp_player.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_math.hpp>

daKpPlayer_c *daKpPlayer_c::m_instance = nullptr;

daKpPlayer_c *daKpPlayer_c_classInit() {
    daKpPlayer_c *c = new daKpPlayer_c;
    daKpPlayer_c::m_instance = c;
    return c;
}

// Replace WM_PLAYER actor
kmWritePointer(0x80988DDC, &daKpPlayer_c_classInit);

const char *daKpPlayer_c::sc_mdlNames[] = {"MB_model", "SMB_model", "PLMB_model", "PMB_model"};
const char *daKpPlayer_c::sc_texPatNames[] = {"PB_switch_swim", "PB_switch_swim", "PLMB_switch_swim", "PLB_switch_swim"};

int daKpPlayer_c::create() {
    createMdl();

    mPos = mVec3_c(0.0f, 0.0f, 3000.0f);
    mAngle = mAng3_c(0x1800, 0, 0);
    mScale = mVec3_c(1.6f, 1.6f, 1.6f);

    mHasEffect = false;
    mHasSound = false;
    mStep = false;
    mpEffectName = "";
    mSoundID = 0;
    mStepSndTimer = 0;
    mJumpOffset = 0.0;

    return SUCCEEDED;
}

int daKpPlayer_c::doDelete() {
    delete mpPyMdlMng;
    return SUCCEEDED;
}

int daKpPlayer_c::execute() {
    if (daPyMng_c::mCreateItem[mPlyType] & CREATE_ITEM_STAR_POWER) {
        mpPyMdlMng->mpMdl->onStarAnm();
        mpPyMdlMng->mpMdl->onStarEffect();
        dKpMusic_c::m_instance->onStarSe();
    } else {
        dKpMusic_c::m_instance->offStarSe();
    }

    if (mIsSpinning) {
        mAngle.y += 0xC00;
    } else {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetRotY, 0xC00);
    }

    if (dScKoopatlas_c::m_instance->chkMapIdleState()) {
        dScKoopatlas_c::m_instance->mPathManager.execute();
    }

    calcMdl();
    mpPyMdlMng->play();
    //mTexPats[((dPlayerMdl_c*)mpPyMdlMng->mpMdl)->mModelIdx].play();

    if (dScKoopatlas_c::m_instance->chkMapIdleState()) {
        if (mHasEffect) {
            mVec3_c effPos(mPos.x, mPos.y, 3300.0f);
            mEffect.createEffect(mpEffectName, 0, &effPos, &mAngle, &mScale);
        }

        if (mHasSound) {
            mStepSndTimer++;

            if (mStepSndTimer == 12) {
                if (mStep) {
                    SndAudioMgr::sInstance->startSystemSe(mSoundID, 1);
                    mStep = false;
                } else {
                    SndAudioMgr::sInstance->startSystemSe(mSoundID + 1, 1);
                    mStep = true;
                }
                mStepSndTimer = 0;
            }

            if (mStepSndTimer > 12) {
                mStepSndTimer = 0;
            }
        }
    }
    return SUCCEEDED;
}

int daKpPlayer_c::draw() {
    // TODO: Why doesn't the ending scene just set the player to be invisible...?
    if (!mVisible || dScKoopatlas_c::m_instance->mIsEndingScene) {
        return SUCCEEDED;
    }

    mpPyMdlMng->draw();
    return SUCCEEDED;
}

void daKpPlayer_c::createMdl() {
    mPlyType = daPyMng_c::mPlayerType[0];
    mpPyMdlMng = new dPyMdlMng_c((dPyMdlMng_c::ModelType_e)mPlyType);

    dPlayerMdl_c *pyMdl = (dPlayerMdl_c*)mpPyMdlMng->mpMdl;
    pyMdl->mSceneType = 1;
    pyMdl->mPlayerType = mPlyType;

    pyMdl->mAllocator.createFrmHeap(0xC000, mHeap::g_gameHeaps[0], 0, 0x20, mHeap::OPT_0);
    pyMdl->createModel();

    /*for (int i = 0; i < 4; i++) {
        nw4r::g3d::ResMdl mdl = pyMdl->mModelResFile.GetResMdl(sc_mdlNames[i]);
        nw4r::g3d::ResAnmTexPat pat = pyMdl->mModelResFile.GetResAnmTexPat(sc_texPatNames[i]);

        mTexPats[i].create(mdl, pat, &pyMdl->mAllocator, 0, 1);
    }*/

    pyMdl->mAllocator.adjustFrmHeap();

    pyMdl->setPlayerMode(daPyMng_c::mPlayerMode[mPlyType]);
    pyMdl->initialize();

    pyMdl->setAnm(dPyMdlBase_c::WAIT, 1.2, 10.0, 0.0);
}

void daKpPlayer_c::chkUpdateMdl() {
    if (mPlyType == daPyMng_c::mPlayerType[0]) {
        return;
    }

    createMdl();
    startAnimation(mCurrentAnim, mCurrentFrame, mCurrentUnk, mCurrentUpdateRate);
}

void daKpPlayer_c::calcMdl() {
    mMtx_c matrix;
    PSMTXScale(matrix, mScale.x, mScale.y, mScale.z);
    PSMTXTransApply(matrix, matrix, mPos.x, mPos.y + mJumpOffset, mPos.z);

    if (dScKoopatlas_c::m_instance->mWarpZoneHacks && (mCurrentAnim == dPyMdlBase_c::JUMP || mCurrentAnim == dPyMdlBase_c::JUMPED)) {
        PSMTXTransApply(matrix, matrix, 0, 0, 600.0f);
    }
    matrix.XrotM(mAngle.x);
    matrix.YrotM(mAngle.y);

    mpPyMdlMng->calc(matrix);
}

void daKpPlayer_c::startAnimation(int id, float frame, float unk, float updateRate) {
    if (id == mCurrentAnim && frame == mCurrentFrame && unk == mCurrentUnk && updateRate == mCurrentUpdateRate) {
        return;
    }

    //bool isCurrAnmSwim = (mCurrentAnim == dPyMdlBase_c::SWIM_WAIT);
    //bool isNewAnmSwim = (id == dPyMdlBase_c::SWIM_WAIT);

    mCurrentAnim = id;
    mCurrentFrame = frame;
    mCurrentUnk = unk;
    mCurrentUpdateRate = updateRate;
    mpPyMdlMng->mpMdl->setAnm(id, frame, unk, updateRate);

    /*if (isCurrAnmSwim != isNewAnmSwim) {
        updatePatAnm();
    }*/
}

void daKpPlayer_c::updatePatAnm() {
    /*dPlayerMdl_c *pyMdl = (dPlayerMdl_c*)mpPyMdlMng->mpMdl;
    int currModel = pyMdl->mModelIdx;

    static const float sc_modeFrames[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 3.0f};

    float frame = sc_modeFrames[pyMdl->mPlayerMode];
    if (mCurrentAnim == dPyMdlBase_c::SWIM_WAIT) {
        frame += (pyMdl->mPlayerMode == 4 || pyMdl->mPlayerMode == 5) ? 1.0f : 4.0f;
    }

    nw4r::g3d::ResAnmTexPat resPat = pyMdl->mModelResFile.GetResAnmTexPat(sc_texPatNames[currModel]);
    mTexPats[currModel].setAnm(pyMdl->mModels[currModel].mBody, resPat, 0, m3d::PLAYMODE_INHERIT);
    mTexPats[currModel].setRate(0.0f, 0);
    mTexPats[currModel].setFrame(frame, 0);

    pyMdl->mModels[currModel].mBody.setAnm(mTexPats[currModel]);*/
}
#endif