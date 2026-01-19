#include <kamek.h>
#include <game/bases/d_a_en_kuribo_base.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

// Fix the unused EN_MAME_KURIBO actor

// Assign the actor to sprite slot 22
kmWrite16(0x8030a6b0, 0x0035);

// Make it's hitbox ever so slightly taller to fix the collision detection with mini players
kmWrite32(0x80ad2d3c, 0x40900000);


// Fix the player collision function to kill the goomba when jumped on by a mini player
void mameKuribo_Normal_VsPlHitCheck(daEnKuriboBase_c *this_, dCc_c *self, dCc_c *other) {
    dActor_c *player = other->mpOwner;
    int hitcheck = this_->Enfumi_check(self, other, false);
    if (hitcheck == 1 || hitcheck == 2) {
        if (hitcheck == 2) {
            this_->FumiScoreSet(player);
            this_->fumiEffect(player);
        }
        this_->reactFumiProc(player);
    } else if (hitcheck == 3) {
        this_->reactSpinFumiProc(player);
    } else if (hitcheck == 0 && this_->isDamageInvalid() == false) {
        this_->dEn_c::Normal_VsPlHitCheck(self, other);
    }
}

kmWritePointer(0x80afaad0, &mameKuribo_Normal_VsPlHitCheck);

// Play a sound and graphical effect when dying from contact with a normal sized player
bool mameKuribo_hitCallback_Large(daEnKuriboBase_c *this_, dCc_c *self, dCc_c *other) {
    mVec2_c soundPos = dAudio::cvtSndObjctPos(this_->mPos);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_MAMEKURIBO_DEAD, soundPos, 0);
    mVec3_c centerPos = this_->getCenterPos();
    centerPos.z = 5500.0f;
    mEf::createEffect("Wm_en_hit", 0, &centerPos, nullptr, nullptr);
    return this_->dEn_c::hitCallback_Large(self, other);
}

kmWritePointer(0x80afaae0, &mameKuribo_hitCallback_Large);

// Play the correct sound when jumped on by a mini player
void mameKuribo_mameFumiSE(daEnKuriboBase_c *this_, dActor_c *actor) {
    mVec2_c soundPos = dAudio::cvtSndObjctPos(this_->getCenterPos());
    dAudio::g_pSndObjEmy->startSound(SE_EMY_MAMEKURIBO_DEAD, soundPos, 0);
}

kmWritePointer(0x80afac48, &mameKuribo_mameFumiSE);