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
void mameKuribo_Normal_VsPlHitCheck(daEnKuriboBase_c *_this, dCc_c *cc1, dCc_c *cc2) {
    dActor_c *player = cc2->mpOwner;
    int hitcheck = _this->Enfumi_check(cc1, cc2, false);
    if (hitcheck == 1 || hitcheck == 2) {
        if (hitcheck == 2) {
            _this->FumiScoreSet(player);
            _this->fumiEffect(player);
        }
        _this->reactFumiProc(player);
    } else if (hitcheck == 2) {
        _this->reactSpinFumiProc(player);
    } else if (hitcheck == 0 && _this->isDamageInvalid() == false) {
        _this->dEn_c::Normal_VsPlHitCheck(cc1, cc2);
    }
}

kmWritePointer(0x80afaad0, &mameKuribo_Normal_VsPlHitCheck);

// Play a sound and graphical effect when dying from contact with a normal sized player
bool mameKuribo_hitCallback_Large(daEnKuriboBase_c *_this, dCc_c *cc1, dCc_c *cc2) {
    mVec2_c soundPos = dAudio::cvtSndObjctPos(_this->mPos);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_MAMEKURIBO_DEAD, soundPos, 0);
    mVec3_c centerPos = _this->getCenterPos();
    centerPos.z = 5500.0;
    mEf::createEffect("Wm_en_hit", 0, &centerPos, nullptr, nullptr);
    return _this->dEn_c::hitCallback_Large(cc1, cc2);
}

kmWritePointer(0x80afaae0, &mameKuribo_hitCallback_Large);

// Play the correct sound when jumped on by a mini player
void mameKuribo_mameFumiSE(daEnKuriboBase_c *_this, dActor_c *actor) {
    mVec2_c soundPos = dAudio::cvtSndObjctPos(_this->getCenterPos());
    dAudio::g_pSndObjEmy->startSound(SE_EMY_MAMEKURIBO_DEAD, soundPos, 0);
}

kmWritePointer(0x80afac48, &mameKuribo_mameFumiSE);