#include <kamek.h>
#include <new/game_config.h>

// TODO: Make this support other players and not be so mario-hardcoded? and do the other thing ryguy asked

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_a_kp_player.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_math.hpp>

daKPPlayer_c *daKPPlayer_c::m_instance = nullptr;

daKPPlayer_c *daKPPlayer_c::build() {
    daKPPlayer_c *c = new daKPPlayer_c;
    m_instance = c;
    return c;
}

// Replace WM_PLAYER actor
kmWritePointer(0x80988DDC, &daKPPlayer_c::build);

static const char *mdlNames[] = {"MB_model", "SMB_model", "PLMB_model", "PMB_model"};
static const char *patNames[] = {"PB_switch_swim", "PB_switch_swim", "PLMB_switch_swim", "PLB_switch_swim"};

int daKPPlayer_c::create() {
    mpPyMdlMng = new dPyMdlMng_c(dPyMdlMng_c::MODEL_MARIO);
    dPlayerMdl_c *pyMdl = (dPlayerMdl_c*)mpPyMdlMng->mpMdl;

    pyMdl->mMode = 1;
    pyMdl->mPlayerID = 0;
    pyMdl->mAllocator.createFrmHeap(0xC000, mHeap::g_gameHeaps[0], 0, 0x20, mHeap::OPT_0);
    pyMdl->createModel();

    /*for (int i = 0; i < 4; i++) {
        nw4r::g3d::ResMdl mdl = pyMdl->mModelResFile.GetResMdl(mdlNames[i]);
        nw4r::g3d::ResAnmTexPat pat = pyMdl->mModelResFile.GetResAnmTexPat(patNames[i]);

        mTexPats[i].create(mdl, pat, &pyMdl->mAllocator, 0, 1);
    }*/

    pyMdl->mAllocator.adjustFrmHeap();
    pyMdl->setPlayerMode(3);
    pyMdl->initialize();

    pyMdl->setAnm(0, 1.2, 10.0, 0.0);
    mpPyMdlMng->calc(mVec3_c(0.0, 100.0, -100.0), mAng3_c(0,0,0), mVec3_c(2.0, 2.0, 2.0));

    mPos = mVec3_c(0.0f,0.0f,3000.0f);
    mAngle = mAng3_c(0x1800,0,0);
    mScale = mVec3_c(1.6f,1.6f,1.6f);

    mHasEffect = false;
    mHasSound = false;
    mStep = false;
    mpEffectName = "";
    mSoundName = 0;
    mStepSndTimer = 0;
    mJumpOffset = 0.0;

    return true;
}

int daKPPlayer_c::doDelete() {
    delete mpPyMdlMng;
    return true;
}

int daKPPlayer_c::execute() {
    if (daPyMng_c::mCreateItem[0] & 1) {
        mpPyMdlMng->mpMdl->onStarAnm();
        mpPyMdlMng->mpMdl->onStarEffect();
        dKPMusic_c::m_instance->playStarSfx();
    }

    if (mIsSpinning) {
        mAngle.y += 0xC00;
    } else {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetRotY, 0xC00);
    }

    //if (dScKoopatlas_c::instance->mapIsRunning())
    //	dScKoopatlas_c::instance->pathManager.execute();

    mpPyMdlMng->play();
    //pats[((dPlayerModel_c*)mpPyMdlMng->mdlClass)->currentPlayerModelID].process();

    mMtx_c myMatrix;
    PSMTXScale(myMatrix, mScale.x, mScale.y, mScale.z);
    myMatrix.trans(mPos.x, mPos.y + mJumpOffset, mPos.z);
    if (dScKoopatlas_c::instance->mWarpZoneHacks && (mCurrentAnim == dPyMdlBase_c::JUMP || mCurrentAnim == dPyMdlBase_c::JUMPED))
        myMatrix.trans(0, 0, 600.0f);	
    myMatrix.XrotM(mAngle.x);
    myMatrix.YrotM(mAngle.y);
    // Z is unused for now
    mpPyMdlMng->calc(myMatrix);

    if (dScKoopatlas_c::instance->mapIsRunning()) {
        if (mHasEffect) { 
            mVec3_c effPos(mPos.x, mPos.y, 3300.0f);
            mEffect.createEffect(mpEffectName, 0, &effPos, &mAngle, &mScale);
        }

        if (mHasSound) {
            mStepSndTimer++;

            if (mStepSndTimer == 12) {
                if (mStep) {
                    SndAudioMgr::sInstance->startSystemSe(mSoundName, 1);
                    mStep = false;
                } else {
                    SndAudioMgr::sInstance->startSystemSe(mSoundName+1, 1);
                    mStep = true;
                }
                mStepSndTimer = 0;
            }

            if (mStepSndTimer > 12) {
                mStepSndTimer = 0;
            }
        }
    }
    return true;
}

int daKPPlayer_c::draw() {
    if (!mVisible)
        return true;
    if (dScKoopatlas_c::instance->mIsEndingScene)
        return true;

    mpPyMdlMng->draw();
    return true;
}


void daKPPlayer_c::startAnimation(int id, float frame, float unk, float updateRate) {
    if (id == mCurrentAnim && frame == mCurrentFrame && unk == mCurrentUnk && updateRate == mCurrentUpdateRate)
        return;

    //bool isOldSwimming = (mCurrentAnim == swim_wait);
    //bool isNewSwimming = (id == swim_wait);

    mCurrentAnim = id;
    mCurrentFrame = frame;
    mCurrentUnk = unk;
    mCurrentUpdateRate = updateRate;
    this->mpPyMdlMng->mpMdl->setAnm(id, frame, unk, updateRate);

    //if (isOldSwimming != isNewSwimming)
    //	bindPats();
}

void daKPPlayer_c::bindPats() {
    /*dPlayerModel_c *pm = (dPlayerModel_c*)mpPyMdlMng->mdlClass;
    int id = pm->currentPlayerModelID;

    static const float frames[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 3.0f};
    float frame = frames[pm->powerup_id];
    if (currentAnim == swim_wait)
        frame += (pm->powerup_id == 4 || pm->powerup_id == 5) ? 1.0f : 4.0f;

    nw4r::g3d::ResAnmTexPat pat = pm->modelResFile.GetResAnmTexPat(patNames[id]);
    pats[id].bindEntry(
            &pm->models[id].body,
            &pat,
            0, 4);
    pats[id].setUpdateRateForEntry(0.0f, 0);
    pats[id].setFrameForEntry(frame, 0);

    pm->models[id].body.bindAnim(&pats[id]);*/
}
#endif