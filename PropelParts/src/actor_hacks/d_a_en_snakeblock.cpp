#include <kamek.h>
#include <game/bases/d_a_en_snakeblock.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_switch_flag_mng.hpp>

extern "C" void afterSnakeBlockCollCheck(void);

// Only apply ice tile collision to icy snake blocks
kmBranchDefAsm(0x80AA6FC0, 0x80AA6FC4) {
    lwz r4, 0x1B0(r31) // Grab parent daEnSnakeBlock_c
    lwz r4, 0x2B2C(r4) // Get mSnakeType
    cmpwi r4, 1
    bne skipParamSet

    ori r0, r0, 4

    skipParamSet:
    blr
}

// Extra setup in create()
kmCallDefCpp(0x80AA74D8, void, daEnSnakeBlock_c *this_) {
    // Set snake type
    this_->mSnakeType = (this_->mParam >> 20) & 1;
    
    // Set block owners so we can access it for createMdl()
    this_->mHead.mpOwner = this_;
    this_->mTail.mpOwner = this_;
    for (int i = 0; i < this_->mBlockNum; i++) {
        this_->mBlocks[i].mpOwner = this_;
    }

    this_->initBlock(); // Replaced insn
}

// Check if we should automatically begin moving
kmBranchDefCpp(0x80AA74F8, 0x80AA750C, int, daEnSnakeBlock_c *this_) {
    if ((this_->mParam >> 10) & 1) {
        this_->changeState(daEnSnakeBlock_c::StateID_Move);
    } else {
        this_->changeState(daEnSnakeBlock_c::StateID_Wait);
    }

    return 1;
}

extern const char *l_SNAKEBLOCK_res[];

// Load custom snake block models
kmBranchDefCpp(0x80AA6B70, NULL, void, daEnSnakeBlock_c::dBlock_c *this_, dHeapAllocator_c *pAlloc) {
    int type = this_->mpOwner->mSnakeType;
    const char *resName = l_SNAKEBLOCK_res[type];
    char brresBuf[64];
    sprintf(brresBuf, "g3d/%s.brres", resName);

    this_->mResFile = dResMng_c::m_instance->getRes(resName, brresBuf);
    nw4r::g3d::ResMdl resMdl = this_->mResFile.GetResMdl(resName);
    this_->mModel.create(resMdl, pAlloc, 0x32C, 1, 0);
    dActor_c::setSoftLight_MapObj(this_->mModel);

    // Setup ice sheen
    if (type == daEnSnakeBlock_c::TYPE_ICE) {
        nw4r::g3d::ResAnmTexSrt resTexSrt = this_->mResFile.GetResAnmTexSrt(resName);
        this_->mAnmTexSrt.create(resMdl, resTexSrt, pAlloc, 0, 1);
        this_->mAnmTexSrt.setAnm(this_->mModel, resTexSrt, 0, m3d::FORWARD_LOOP);
        this_->mModel.setAnm(this_->mAnmTexSrt, 0.0f);
        this_->mAnmTexSrt.setRate(1.0f, 0);
    }

    nw4r::g3d::ResAnmClr resClr = this_->mResFile.GetResAnmClr("ridden");
    this_->mAnmClr.create(resMdl, resClr, pAlloc, 0, 1);
    this_->mAnmClr.setAnm(this_->mModel, resClr, 0, m3d::FORWARD_ONCE);
    this_->mModel.setAnm(this_->mAnmClr);
    this_->mAnmClr.setRate(0.0f, 0);
}

// dBlock_c::doDelete()
kmBranchDefCpp(0x80AA6D00, NULL, void, daEnSnakeBlock_c::dBlock_c *this_) {
    this_->mModel.remove();
    this_->mAnmClr.remove();
    if (this_->mpOwner->mSnakeType == daEnSnakeBlock_c::TYPE_ICE) {
        this_->mAnmTexSrt.remove();
    }
}

// dBlock_c::calcAnm()
kmBranchDefCpp(0x80AA6D70, NULL, void, daEnSnakeBlock_c::dBlock_c *this_) {
    this_->mAnmClr.play();
    if (this_->mpOwner->mSnakeType == daEnSnakeBlock_c::TYPE_ICE) {
        this_->mAnmTexSrt.play();
    }
}

// Event activated Snake Block
kmBranchDefCpp(0x80AA83D0, NULL, void, daEnSnakeBlock_c *this_) {
    if (this_->mEventNums[1] != 0) {
        if (dSwitchFlagMng_c::checkEvent(this_->mEventNums[1]-1)) {
            this_->changeState(daEnSnakeBlock_c::StateID_Move);
        }
    }
}

// Don't activate when stood on if we're event activated
kmBranchDefCpp(0x80AA7220, NULL, void, dActor_c *self, dActor_c *other) {
    daEnSnakeBlock_c *snakeBlock = (daEnSnakeBlock_c*)self;
    if (snakeBlock->mEventNums[1] == 0) {
        bool activate = false;
        if (other->mKind == dActor_c::STAGE_ACTOR_PLAYER) {
            activate = true;
        } else if (other->mKind == dActor_c::STAGE_ACTOR_YOSHI && *other->getPlrNo() != -1) {
            activate = true;
        }
        if (activate) {
            if (snakeBlock->isState(daEnSnakeBlock_c::StateID_Wait)) {
                snakeBlock->changeState(daEnSnakeBlock_c::StateID_Move);
            }
        }
    }
}

extern "C" void startSound__14SndObjctCmnMapFUlRCQ34nw4r4math4VEC2Ul(void);

// Don't play sound effect if disabled
kmBranchDefAsm(0x80AA85D8, 0x80AA85DC) {
    nofralloc
    lwz r12, 0x4(r29)
    rlwinm r12, r12, 21, 31, 31
    cmpwi r12, 1
    beq noSound
    bl startSound__14SndObjctCmnMapFUlRCQ34nw4r4math4VEC2Ul

    noSound:
    blr
}
