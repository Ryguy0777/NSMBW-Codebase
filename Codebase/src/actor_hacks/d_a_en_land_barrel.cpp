#include <kamek.h>
#include <game/bases/d_actor.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

// change daEnLandBarrel_c's break effect from brick shards to the barrel break effect

kmBranchDefCpp(0x80a590d0, NULL, void, dActor_c *this_) {
    // get effect scale & position
    mVec3_c effPos = mVec3_c(this_->mPos.x, this_->mPos.y + this_->mCenterOffs.y, this_->mPos.z);
    mVec3_c effScale = mVec3_c(0.6f, 0.6f, 0.6f);
    mVec3_c effScale2 = mVec3_c(0.4f, 0.4f, 0.4f);
    mAng3_c effAng = mAng3_c(0, 0, 0);

    mEf::createEffect("Wm_ob_cmnboxsmoke", 0, &effPos, &effAng, &effScale);
    mEf::createEffect("Wm_ob_cmnboxgrain", 0, &effPos, &effAng, &effScale2);
    mEf::createEffect("Wm_ob_cmnboxpiece", 0, &effPos, &effAng, &effScale);

    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(this_->mPos);
    dAudio::g_pSndObjMap->startSound(SE_OBJ_TARU_BREAK, soundPos, 0);

    this_->deleteActor(0);
    return;
}