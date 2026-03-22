#include <kamek.h>
#include <game/bases/d_a_en_snake_block.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_switch_flag_mng.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_rail.hpp>
#include <game/bases/d_s_stage.hpp>

// Only apply ice tile collision to icy snake blocks
kmBranchDefCpp(0x80AA6F10, NULL, void, daEnSnakeBlock_c::dBlock_c *this_, daEnSnakeBlock_c *parent, mVec3_c *blockPos, int isIce) {
    mVec3_c targetPos = *blockPos;
    // Adjust spawn position based on travel direction
    switch (parent->mpTravelInfo[1]) {
        case 1: // Up
            targetPos.x -= parent->mBlockNum * 16.0f;
            targetPos.y += parent->mBlockNum * 16.0f;
            break;
        case 2: // Down
            targetPos.x -= parent->mBlockNum * 16.0f;
            targetPos.y -= parent->mBlockNum * 16.0f;
            break;
        case 3: // Left
            targetPos.x -= parent->mBlockNum * 32.0f;
        // Vanilla behavior is to spawn right, so nothing to do here
    }
    this_->mPos = targetPos;

    if (this_->mpOwner == nullptr) {
        this_->mpOwner = parent;
    }

    float blockX = this_->mPos.x - this_->mpOwner->mPos.x;
    float blockY = this_->mPos.y - this_->mpOwner->mPos.y;
    this_->mBgCtr.set(this_->mpOwner, blockX - 8.0f, blockY + 8.0f, blockX + 8.0f, blockY - 8.0f,
        &daEnSnakeBlock_c::dBlock_c::callBackF, &daEnSnakeBlock_c::dBlock_c::callBackH, &daEnSnakeBlock_c::dBlock_c::callBackW,
        1, 0, nullptr
    );

    if (isIce) {
        this_->mBgCtr.mFlags = 4;
    }
    this_->mBgCtr.entry();
}

// Replace create
kmBranchDefCpp(0x80AA7470, NULL, int, daEnSnakeBlock_c *this_) {
    // Handle spawn at midway
    if ((this_->mParam >> 21) & 1) {
        if (dInfo_c::m_instance->mChukanPointNum < 0) {
            this_->deleteRequest();
            return fBase_c::CANCELED;
        }
    }
    daEnSnakeBlock_c::sc_glbSnakeNum++;
    this_->mBlockNum = (this_->mParam & 0xF) + 2;
    this_->mSnakeType = (this_->mParam >> 20) & 1;
    
    // Set block owners so we can access it for createMdl()
    this_->mHead.mpOwner = this_;
    this_->mTail.mpOwner = this_;
    for (int i = 0; i < this_->mBlockNum; i++) {
        this_->mBlocks[i].mpOwner = this_;
    }

    this_->initBlock();
    this_->initBlockPath();
    this_->setStopState();

    if ((this_->mParam >> 10) & 1) {
        this_->changeState(daEnSnakeBlock_c::StateID_Move);
    } else {
        this_->changeState(daEnSnakeBlock_c::StateID_Wait);
    }

    return fBase_c::SUCCEEDED;
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

// Thanks to Grop for decompiling the following functions
kmBranchDefCpp(0x80AA8540, NULL, void, daEnSnakeBlock_c *this_) {
	bool b3 = this_->mHead.calcPos(this_->mpTravelInfo);
    bool b4 = this_->mTail.calcPos(this_->mpTravelInfo);
    bool b5 = false;
    bool b6 = false;

    if (b3 || b4) {
        b5 = this_->mHead.calcTravelPos(this_->mpTravelInfo);
        b6 = this_->mTail.calcTravelPos(this_->mpTravelInfo);
		
		// Don't play sound effect if disabled
		if (!(this_->mParam >> 11 & 1)) {
			dAudio::SoundEffectID_t(SE_OBJ_SNAKE_BLOCK).playMapSound(this_->mPos, 0);
		}

        this_->setBlockPos();
        this_->mCreateAnmBlockIdx++;
        if (this_->mCreateAnmBlockNum < this_->mCreateAnmBlockIdx) {
            this_->mCreateAnmBlockIdx = this_->mCreateAnmBlockNum;
        }

        int frame_idx = 0;
        int frame_delta = (int)(16.0f / daEnSnakeBlock_c::sc_snakeSpeeds[(this_->mParam >> 8) & 3]);

        for (int i = 0; i < this_->mCreateAnmBlockIdx; i++) {
			if (i < this_->mBlockNum) {
				this_->mBlocks[i].setAnmClr("create");
				this_->mBlocks[i].mAnmClr.setFrame(frame_idx, 0);
				frame_idx += frame_delta;
			}
        }
    }

    if (b5 || b6) {
        if (this_->mpStopState->isEqual(daEnSnakeBlock_c::StateID_Stop)) {
            this_->changeState(daEnSnakeBlock_c::StateID_Stop);
        } else {
            this_->changeState(daEnSnakeBlock_c::StateID_Shake);
        }
    }
}

kmBranchDefCpp(0x80AA7730, NULL, void, daEnSnakeBlock_c *this_) {
	sRailInfoData *rail = dRail_c::getRailInfoP((this_->mParam >> 4) & 0xF);
    dCdFile_c *file = dCd_c::m_instance->getFileP(dScStage_c::m_instance->mCurrFile);

    sRailNodeData *node = &file->mpRailNodes[rail->mNodeIdx + ((this_->mParam >> 12) & 0xFF)];

    float f1 = node->mX + (this_->mBlockNum * 16.0f) + 8.0f;
    float f2 = -node->mY - 8.0f;
    mVec3_c head_pos = mVec3_c(f1, f2, 1516.0f);
    this_->mHead.initBgCtr(this_, &head_pos, (this_->mParam >> 20) & 1);

    mVec3_c mid_pos;
    for (int i = 0; i < this_->mBlockNum; i++) {
        mid_pos.set(f1, f2, 1500.0f);
        this_->mBlocks[i].initBgCtr(this_, &mid_pos, (this_->mParam >> 20) & 1);
		// Adjust position changes depending on starting direction
		switch (this_->mpTravelInfo[1]) {
			case 1: // Up
				f2 -= 16.0f;
				break;
			case 2: // Down
				f2 += 16.0f;
				break;
			case 3: // Left
				f1 += 16.0f;
				break;
			default: // Right
				f1 -= 16.0f;
				break;
		}
    }

    mVec3_c tail_pos = mVec3_c(f1, f2, 1516.0f);
    this_->mTail.initBgCtr(this_, &tail_pos, (this_->mParam >> 20) & 1);

    this_->mHead.mTravelInfoIdx = this_->mBlockNum + 1;
    this_->mTail.mTravelInfoIdx = 1;

    // Note: Value 3 is out of bounds
    this_->mHead.mSnakeSpeedIdx = (this_->mParam >> 8) & 3;
    this_->mTail.mSnakeSpeedIdx = (this_->mParam >> 8) & 3;
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

kmBranchDefAsm(0x80AA7C14, 0x80AA7C18) {
	addi r3, r25, 2
	slwi r3, r3, 1 // r3 *= 2
    blr
}


kmCallDefAsm(0x80AA8150) {
    // r3 is mpTravelInfo; r25 is mTravelInfoIdx
	stb r4, 1(r3) // Restore instruction

	mr r3, r0

	stwu sp, -0x100(sp)
	mflr r0
	stw r0, 0x104(sp)
	stw r31, 0xFC(sp)
	stw r30, 0xF8(sp)
	stw r29, 0xF4(sp)
	stw r28, 0xF0(sp)
	stw r27, 0xEC(sp)
	stw r26, 0xE8(sp)
	stw r25, 0xE4(sp)
	stw r24, 0xE0(sp)
	stw r23, 0xDC(sp)
	stw r22, 0xD8(sp)
	stw r21, 0xD4(sp)
	stw r20, 0xD0(sp)
	stw r19, 0xCC(sp)
	stw r18, 0xC8(sp)
	stw r17, 0xC4(sp)
	stw r16, 0xC0(sp)
	stw r15, 0xBC(sp)
	stw r14, 0xB8(sp)

	li r0, 0 // r0 will be our index to the beginning
	mr r4, r25
	addi r4, r4, 2
	slwi r4, r4, 1 // r4 will be out index to the end
	addi r4, r4, -1

	addi r6, r25, 1

    _startLoop:
	lbzx r16, r3, r0 // Load r0(mpTravelInfo)
	cmpw r0, r6
	bgt _endLoop

    _doLoop:
	cmpwi r16, 1
	beq _setToTwo
	cmpwi r16, 2
	beq _setToOne
	cmpwi r16, 3
	beq _setToFour
	cmpwi r16, 4
	beq _setToThree

	b _storeNewInfo

	_setToOne:
	li r16, 1
	b _storeNewInfo
	_setToTwo:
	li r16, 2
	b _storeNewInfo
	_setToThree:
	li r16, 3
	b _storeNewInfo
	_setToFour:
	li r16, 4

	_storeNewInfo:
	stbx r16, r3, r4
	mr r16, r0
	addi r0, r16, 1 // That's to avoid it addi r0, r0 to compile as li r0
	addi r4, r4, -1
	b _startLoop

    _endLoop:
	lwz r0, 0x104(sp)
	lwz r31, 0xFC(sp)
	lwz r30, 0xF8(sp)
	lwz r29, 0xF4(sp)
	lwz r28, 0xF0(sp)
	lwz r27, 0xEC(sp)
	lwz r26, 0xE8(sp)
	lwz r25, 0xE4(sp)
	lwz r24, 0xE0(sp)
	lwz r23, 0xDC(sp)
	lwz r22, 0xD8(sp)
	lwz r21, 0xD4(sp)
	lwz r20, 0xD0(sp)
	lwz r19, 0xCC(sp)
	lwz r18, 0xC8(sp)
	lwz r17, 0xC4(sp)
	lwz r16, 0xC0(sp)
	lwz r15, 0xBC(sp)
	lwz r14, 0xB8(sp)
	mtlr r0
	addi sp, sp, 0x100
	blr
}