#include <kamek.h>
#include <game/bases/d_3d.hpp>
#include <game/bases/d_a_en_snakeblock.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_save_mng.hpp>

// TODO:
// Clean this up, its kinda messy
// Separate Glow Snake Block into its own actor

extern "C" void afterSnakeBlockCollCheck(void);
extern "C" void drawLightMask__16daEnSnakeBlock_cFv(void);

const char *daEnSnakeBlock_c::sc_resName[] = {
    "block_snake",
    "block_snake_ice",
    "block_snake",
    "block_snake_glow",
    NULL,
};

// Replace l_SNAKEBLOCK_res
// (Temporary)
kmWritePointer(0x8031ADE4, daEnSnakeBlock_c::sc_resName);

// Patch class size (+0x74)
kmWrite16(0x80AA67EA, 0x2E00);

// Read entire nybble 7
//kmWrite32(0x80AA74C4, 0x5400673E);

// Only apply ice tile collision to icy snake blocks
kmCallDefAsm(0x80AA6FC0) {
    lwz r4, 0x1B0(r31) // Grab parent daEnSnakeBlock_c
    lwz r4, 0x2B2C(r4) // Get mSnakeType
    cmpwi r4, 1
    bne skipParamSet

    ori r0, r0, 4

skipParamSet:
    b afterSnakeBlockCollCheck
}

// Extra setup in create()
kmCallDefCpp(0x80AA74D8, void, daEnSnakeBlock_c *this_) {
    // Set snake type depending on profile ID
    if (this_->mProfName == fProfile::EN_SNAKEBLOCK) {
        this_->mSnakeType = (this_->mParam >> 20) & 1;
    }/*else if (this_->mProfName == fProfile::EN_SNAKEBLOCK_GLOW) {
        this_->mSnakeType = daEnSnakeBlock_c::TYPE_GLOW;
    }*/
    
    // Set block owners so we can access it for createMdl()
    this_->mHead.mpOwner = this_;
    this_->mTail.mpOwner = this_;
    for (int i = 0; i < this_->mBlockNum; i++) {
        this_->mBlocks[i].mpOwner = this_;
    }

    // Setup lighting for Glow blocks
    if (this_->mSnakeType == daEnSnakeBlock_c::TYPE_GLOW) {
        for (int i = 0; i < 22; i++) {
            this_->mMaskAllocs[i] = mHeapAllocator_c();
        }
        
        this_->mpLightMaskH = new dCircleLightMask_c;
        this_->mpLightMaskH->init(&this_->mMaskAllocs[0], 2);

        this_->mpLightMaskT = new dCircleLightMask_c;
        this_->mpLightMaskT->init(&this_->mMaskAllocs[1], 2);

        for (int i = 0; i < this_->mBlockNum; i++) {
            this_->mpLightMasks[i] = new dCircleLightMask_c;
            this_->mpLightMasks[i]->init(&this_->mMaskAllocs[i+2], 2);
        }
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

// Init all block models
//kmWrite32(0x80AA79F4, 0x38000014);

// Load custom snake block models
kmBranchDefCpp(0x80AA6B70, NULL, void, daEnSnakeBlock_c::dBlock_c *this_, dHeapAllocator_c *pAlloc) {
    int type = this_->mpOwner->mSnakeType;
    const char *resName = daEnSnakeBlock_c::sc_resName[type];
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

// Draw lights
kmBranchDefAsm(0x80AA75C0, 0x80AA75C4) {
    or r29, r3, r3
    bl drawLightMask__16daEnSnakeBlock_cFv
    or r3, r29, r29
    blr
}

void daEnSnakeBlock_c::drawLightMask() {
    if (mSnakeType == daEnSnakeBlock_c::TYPE_GLOW) {
        mpLightMaskH->draw();
        mpLightMaskT->draw();

        for (int i = 0; i < mBlockNum; i++) {
            mpLightMasks[i]->draw();
        }
    }
}

// Calc lights
kmBranchDefCpp(0x80AA754C, 0x80AA7550, void, daEnSnakeBlock_c *this_) {
    if (this_->mSnakeType == daEnSnakeBlock_c::TYPE_GLOW) {
        this_->mpLightMaskH->mPos.x = this_->mHead.mPos.x;
        this_->mpLightMaskH->mPos.y = this_->mHead.mPos.y;
        this_->mpLightMaskH->mPos.z = this_->mHead.mPos.z;
        this_->mpLightMaskH->mRadius = 120.0f;
        this_->mpLightMaskH->execute();

        nw4r::g3d::ResMdl resMdl = this_->mHead.mResFile.GetResMdl(this_->sc_resName[daEnSnakeBlock_c::TYPE_GLOW]);
        nw4r::g3d::ResMat glowMat = resMdl.GetResMat("mat_light_block_source");

        resMdl = this_->mTail.mResFile.GetResMdl(this_->sc_resName[daEnSnakeBlock_c::TYPE_GLOW]);
        glowMat = resMdl.GetResMat("mat_light_block_glow");
        d3d::setMatCullMode(&this_->mHead.mModel, 1, GX_CULL_ALL);

        this_->mpLightMaskT->mPos.x = this_->mTail.mPos.x;
        this_->mpLightMaskT->mPos.y = this_->mTail.mPos.y;
        this_->mpLightMaskT->mPos.z = this_->mTail.mPos.z;
        this_->mpLightMaskT->mRadius = 120.0f;
        this_->mpLightMaskT->execute();

        for (int i = 0; i < this_->mBlockNum; i++) {
            this_->mpLightMasks[i]->mPos.x = this_->mBlocks[i].mPos.x;
            this_->mpLightMasks[i]->mPos.y = this_->mBlocks[i].mPos.y;
            this_->mpLightMasks[i]->mPos.z = this_->mBlocks[i].mPos.z;
            this_->mpLightMasks[i]->mRadius = 120.0f;
            this_->mpLightMasks[i]->execute();
        }
    }
    
    this_->calcAnm();
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
