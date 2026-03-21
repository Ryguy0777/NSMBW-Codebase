#include <kamek.h>
#include <game/bases/d_a_en_snakeblock.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_switch_flag_mng.hpp>

// Only apply ice tile collision to icy snake blocks
kmBranchDefCpp(0x80AA6F10, NULL, void, daEnSnakeBlock_c::dBlock_c *this_, daEnSnakeBlock_c *parent, mVec3_c *blockPos) {
    this_->mPos = *blockPos;
    if (this_->mpOwner == nullptr) {
        this_->mpOwner = parent;
    }

    float blockX = this_->mPos.x - this_->mpOwner->mPos.x;
    float blockY = this_->mPos.y - this_->mpOwner->mPos.y;
    this_->mBgCtr.set(this_->mpOwner, blockX - 8.0f, blockY + 8.0f, blockX + 8.0f, blockY - 8.0f,
        &daEnSnakeBlock_c::dBlock_c::callBackF, &daEnSnakeBlock_c::dBlock_c::callBackH, &daEnSnakeBlock_c::dBlock_c::callBackW,
        1, 0, nullptr
    );

    if (this_->mpOwner->mSnakeType == daEnSnakeBlock_c::TYPE_ICE) {
        this_->mBgCtr.mFlags = 4;
    }
    this_->mBgCtr.entry();
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

extern "C" void skipSnakeBlockCLR(void);

kmBranchDefAsm(0x80AA8684, 0x80AA8688) {
    lwz r0, 0x2B34(r29) // Snake Block CLR index
    lwz r12, 0x2B18(r29) // Snake Block Length
    cmpw r23, r12
    blt canSetCLR
    b skipSnakeBlockCLR
    
    canSetCLR:
    blr
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

// The following was made by RedStoneMatt

extern "C" void firstLoop(void);
kmBranchDefAsm(0x80AA7C0C, 0x80AA7C10) {
    blt _firstLoop

	lhz r5, 6(r28) // sRailInfoData.mFlags
	andi. r5, r5, 2
	stb r5, 0x2B26(r30)
	beq _notLooped

	mulli r5, r29, 0x10
	neg r5, r5
	lha       r3, 0(r24)
	lhax r0, r5, r24
	srawi     r3, r3, 4
	srawi     r0, r0, 4
	subf      r3, r3, r0
	bl        abs
	lha       r4, 2(r24)
	extsh     r21, r3
	mulli r5, r29, 0x10
	addi r5, r5, -2
	neg r5, r5
	lhax r0, r5, r24
	srawi     r3, r4, 4
	srawi     r0, r0, 4
	subf      r3, r3, r0
	bl        abs
	extsh     r3, r3
	add       r0, r25, r21
	add       r25, r3, r0
	addi      r24, r24, 0x10
	addi      r26, r26, 1
    b _notLooped

    _firstLoop:
	b firstLoop

    _notLooped:
	blr
}

extern "C" void loopForNodes(void);
extern "C" void finishNodeLoop(void);
extern "C" void cancelNodeLoop(void);
kmBranchDefAsm(0x80AA8134, 0x80AA7C80) {
    blt _loopForNodes

	lhz r7, 6(r28) // sRailInfoData.mFlags
	andi. r7, r7, 2
	beq _notNodeLooped

	lhz r7, 4(r28) // sRailInfoData.mCount
	cmpw r31, r7
	bge _finishNodeLoop
	addi r7, r7, -1

	mulli r6, r7, 0x10
	neg r6, r6

	lha       r3, 0(r27)
	lhax r0, r6, r27
	srawi     r5, r3, 4
	lha       r3, 2(r27)
	srawi     r4, r0, 4
	mulli r6, r7, 0x10
	addi r6, r6, -2
	neg r6, r6
	lhax r0, r6, r27
	srawi     r3, r3, 4
	subf      r4, r5, r4
	srawi     r0, r0, 4
	subf      r0, r3, r0
	extsh     r23, r4
	mr        r3, r23
	extsh     r22, r0
	bl        abs
	extsh     r26, r3
	mr        r3, r22
	bl        abs
	cmpwi     r23, 0
	extsh     r6, r3
	li        r7, 0
	li        r8, 0
    b _returnFromNodeLoop

    _loopForNodes:
	b loopForNodes

    _finishNodeLoop:
	b finishNodeLoop

    _notNodeLooped:
	b cancelNodeLoop

    _returnFromNodeLoop:
    blr
}

kmBranchDefAsm(0x80AA7450, 0x80AA7454) {
	cmpwi r0, 0
	bne _doUsual
    lbz r9, 0x2B26(r29)
    lbz r14, 0x2B27(r29)
	cmpwi r9, 0 // Is looped
	beq _checkForEndState
	li r0, 1
	stw r0, 0x1AC(r3) // dCtrlBlock_c.mTravelInfoIdx
	b _doUsual

_checkForEndState:
	cmpwi r15, 4 // End State
	bne _doUsual
	cmpwi r17, 1 // Are we the head
	bne _doUsual


	lwz r0, 0x1AC(r3) // dCtrlBlock_c.mTravelInfoIdx
	cmpwi r14, 0
	bne _subTwo
	lwz r17, 0x2B20(r29) // daEnSnakeBlock_c.mTravelInfoIdx
	mr r16, r17
	addi r16, r16, 3
	lwz r17, 0x2B18 (r29) // daEnSnakeBlock_c.mBlockNum
	add r0, r16, r17
	b _storeNumberInChain
	_subTwo:
	li r0, 1
	_storeNumberInChain:
	stw  r0, 0x1AC(r3) // dCtrlBlock_c.mTravelInfoIdx


	lwz r0, 0x8A8(r29) // daEnSnakeBlock_c.mTail.mTravelInfoIdx
	cmpwi r14, 0
	bne _subTwo2
	lwz r17, 0x2B20(r29) // daEnSnakeBlock_c.mTravelInfoIdx
	mr r16, r17
	addi r0, r16, 3
	b _storeNumberInChain2
	_subTwo2:
	lwz r16, 0x2B18 (r29) // daEnSnakeBlock_c.mBlockNum
	addi r0, r16, 1
	_storeNumberInChain2:
	stw  r0, 0x8A8(r29) // daEnSnakeBlock_c.mTail.mTravelInfoIdx

	li r0, 1
	cmpwi r14, 0
	beq _setR14ToOne
	li r14, 0
	li r0, 1
	b _doUsual
	_setR14ToOne:
	li r14, 1
	li r0, 1

_doUsual:
    stb r14, 0x2B27(r29)
	stfs f5, 0xC(r1) // Restore Instruction
	blr
}