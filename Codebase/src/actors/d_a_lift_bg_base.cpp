#include <kamek.h>
#include <new/bases/d_a_lift_bg_base.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_a_player_base.hpp>

void daLiftBgBase_c::callBackF(dActor_c *self, dActor_c *other) {
    if (other->mKind == STAGE_ACTOR_PLAYER) {
        daPlBase_c *player = (daPlBase_c *)other;
        player->setStatus5D(self->mPosDelta.x);
        if (self->mPosDelta.y <= 0.0f) {
            player->fn_80056370(self, daPlBase_c::BG_PRESS_FOOT);
        } else {
            player->fn_80056370(self, (daPlBase_c::BgPress_e)1);
        }
    }
}

void daLiftBgBase_c::callBackH(dActor_c *self, dActor_c *other) {
    if (other->mKind == STAGE_ACTOR_PLAYER) {
        daPlBase_c *player = (daPlBase_c *)other;
        if (self->mPosDelta.y < 0.0f) {
            player->fn_80056370(self, (daPlBase_c::BgPress_e)2);
        } else {
            player->fn_80056370(self, daPlBase_c::BG_PRESS_HEAD);
        }
    }
}

void daLiftBgBase_c::callBackW(dActor_c *self, dActor_c *other, u8 direction) {
    if (other->mKind == STAGE_ACTOR_PLAYER) {
        daPlBase_c *player = (daPlBase_c *)other;
        if (direction == 1) {
            if (self->mPosDelta.x > 0.f) {
                player->fn_80056370(self, (daPlBase_c::BgPress_e)6);
            } else {
                player->fn_80056370(self, daPlBase_c::BG_PRESS_L);
            }
        } else {
            if (self->mPosDelta.x < 0.0f) {
                player->fn_80056370(self, (daPlBase_c::BgPress_e)5);
            } else {
                player->fn_80056370(self, daPlBase_c::BG_PRESS_R);
            }
        }
    }
}

bool daLiftBgBase_c::checkRevFoot(dActor_c *self, dActor_c *other) {
    return self->mPosDelta.y > 0.0f;
}

bool daLiftBgBase_c::checkRevHead(dActor_c *self, dActor_c *other) {
    return self->mPosDelta.y < 0.0f;
}

bool daLiftBgBase_c::checkRevWall(dActor_c *self, dActor_c *other, u8 direction) {
    if (direction == 0) {
        return self->mPosDelta.x < 0.0f;
    } else {
        return self->mPosDelta.x > 0.0f;
    }
}

int daLiftBgBase_c::create() {
    // Setup movement controller
    if (!initMovement()) {
        // Delay creation by one frame if unable to setup movement controller
        // To allow for controller actors to spawn
        // If we can't find it, delete ourself
        if (!mFindFail) {
            mFindFail = true;
        } else {
            deleteRequest();
        }
        return NOT_READY;
    }

    // Setup tiles
    mRectIdx = mParam & 0xFF;
    getRange();
    createPanelObj();

	// Set size for culling
    mVisibleAreaSize.set(mWidth*16.0f, mHeight*16.0f);

    // Set Z order
    if (mParam & 0x7000) {
        // Nybble 9.2-9.4
        int zVal = (mParam >> 12) & 7;
        zOrder = -3600.0f - (zVal * 16);
    }

    // Collision
    mCollType = (COLLTYPE_e)(mParam >> 8 & 0xF);

    float fWidth = mWidth * 16.0f;
    float fHeight = mHeight * 16.0f;

    float leftOffset, topOffset, rightOffset, bottomOffset;

    switch (mCollType) {
        case COLL_SOLID:
            leftOffset = -fWidth/2;
            topOffset = fHeight/2;
            rightOffset = fWidth/2;
            bottomOffset = -fHeight/2;
            break;
        
        case COLL_SEMISOLID:
            mRide.set(this, 0.0f, 0.0f, fHeight/2, fWidth/2, -fWidth/2, mAngle.y, 1, nullptr);
            mRide.mSubType = 0xA;
            mRide.mFlags = 0x80180 | 0xC00;
            mRide.entry();

        case COLL_NONE:
            return SUCCEEDED;

        case COLL_THINRIGHT:
            leftOffset = fWidth/2 - 4.0f;
            topOffset = fHeight/2;
            rightOffset = fWidth/2; 
            bottomOffset = -fHeight/2;
            break;
        
        case COLL_THINLEFT:
            leftOffset = -fWidth/2;
            topOffset = fHeight/2;
            rightOffset = -fWidth/2 + 4.0f; 
            bottomOffset = -fHeight/2;
            break;

        case COLL_THINTOP:
            leftOffset = -fWidth/2;
            topOffset = fHeight/2;
            rightOffset = fWidth/2; 
            bottomOffset = fHeight/2 - 4.0f;
            break;

        case COLL_THINBOTTOM:
            leftOffset = -fWidth/2;
            topOffset = -fHeight/2 + 4.0f;
            rightOffset = fWidth/2; 
            bottomOffset = -fHeight/2;
            break;
        
        case COLL_CIRCLE:
            mBg.set_circle(this, 0.0f, 0.0f, fWidth/2,
                &daLiftBgBase_c::callBackF,
                &daLiftBgBase_c::callBackH,
                &daLiftBgBase_c::callBackW, 1, 0);

            mBg.mBelowCheckFunc = &daLiftBgBase_c::checkRevFoot;
            mBg.mAboveCheckFunc = &daLiftBgBase_c::checkRevHead;
            mBg.mAdjCheckFunc = &daLiftBgBase_c::checkRevWall;

            mBg.mBelowCallback = &daLiftBgBase_c::callBackF;
            mBg.mAboveCallback = &daLiftBgBase_c::callBackH;
            mBg.mAdjCallback = &daLiftBgBase_c::callBackW;

            mBg.entry();
            mBg.setAngleY3((short *)&mAngle.y);

            return SUCCEEDED;
    }

    mBg.set(this, leftOffset, topOffset, rightOffset, bottomOffset,
        &daLiftBgBase_c::callBackF,
        &daLiftBgBase_c::callBackH,
        &daLiftBgBase_c::callBackW,
        1, 0, nullptr);
    
    mBg.mBelowCheckFunc = &daLiftBgBase_c::checkRevFoot;
    mBg.mAboveCheckFunc = &daLiftBgBase_c::checkRevHead;
    mBg.mAdjCheckFunc = &daLiftBgBase_c::checkRevWall;

    mBg.mBelowCallback = &daLiftBgBase_c::callBackF;
    mBg.mAboveCallback = &daLiftBgBase_c::callBackH;
    mBg.mAdjCallback = &daLiftBgBase_c::callBackW;

    mBg.entry();
    mBg.setAngleY3((short *)&mAngle.y);

    return SUCCEEDED;
};

int daLiftBgBase_c::doDelete() {
    deletePanelObj();

    switch (mCollType) {
        case COLL_SOLID:
        case COLL_THINRIGHT:
        case COLL_THINLEFT:
        case COLL_THINTOP:
        case COLL_THINBOTTOM:
        case COLL_CIRCLE:
            mBg.release();
            break;
    }

    return SUCCEEDED;
}

int daLiftBgBase_c::execute() {
    // Handle movement controller
    executeMovement();

	// Hide if not on screen
	bool shouldCull = ActorDrawCullCheck();
    setVisible(!shouldCull);

    updatePanelObj();

    switch (mCollType) {
        case COLL_SOLID:
        case COLL_THINRIGHT:
        case COLL_THINLEFT:
        case COLL_THINTOP:
        case COLL_THINBOTTOM:
        case COLL_CIRCLE:
            mBg.calc();
            break;

        case COLL_SEMISOLID:
            mRide.mRotation = mAngle.y;
            mRide.calc();
            break;
    }

    return SUCCEEDED;
};

bool daLiftBgBase_c::initMovement() {
    return true;
}

void daLiftBgBase_c::executeMovement() {
    return;
}

void daLiftBgBase_c::getRange() {
    dCdFile_c *currArea = dCd_c::m_instance->getFileP(dScStage_c::m_instance->mCurrCourse);
    sRangeDataF rect;
    currArea->getRangeDataP(mRectIdx, &rect);

    // Round the positions down/up to get the rectangle
    int left = rect.mOffset.x;
    int right = left + rect.mSize.x;
    int top = -rect.mOffset.y;
    int bottom = top + rect.mSize.y;

    left &= 0xFFF0;
    right = (right + 15) & 0xFFF0;

    top &= 0xFFF0;
    bottom = (bottom + 15) & 0xFFF0;

    // Calculate the actual stuff
    mSrcX = left >> 4;
    mSrcY = top >> 4;
    mWidth = (right - left) >> 4;
    mHeight = (bottom - top) >> 4;
}

void daLiftBgBase_c::createPanelObj() {
    mTileCount = mWidth * mHeight;
    mTiles = new dPanelObjList_c[mTileCount];

    // Copy all the tiles over
    int baseWorldX = mSrcX << 4, worldY = mSrcY << 4, rendererID = 0;

    for (int y = 0; y < mHeight; y++) {
        int worldX = baseWorldX;

        for (int x = 0; x < mWidth; x++) {
            
            u16 *pExistingTile = (u16 *)dBg_c::m_bg_p->fn_80077520(worldX, worldY, mLayer, nullptr, false);

            if (*pExistingTile > 0) {
                dPanelObjList_c *r = &mTiles[rendererID];
                r->mTileNumber = *pExistingTile;
            }

            worldX += 16;
            rendererID++;
        }

        worldY += 16;
    }
}

void daLiftBgBase_c::deletePanelObj() {
    if (mTiles != nullptr) {
        setVisible(false);
        delete[] mTiles;
        mTiles = nullptr;
    }
}

void daLiftBgBase_c::updatePanelObj() {
    mVec2_c center(mPos.x, -mPos.y);

    int rendererID = 0;

    float cosA = nw4r::math::CosF(-mAngle.y);
    float sinA = nw4r::math::SinF(-mAngle.y);

    for (int yIdx = 0; yIdx < mHeight; yIdx++) {
        for (int xIdx = 0; xIdx < mWidth; xIdx++) {
            dPanelObjList_c *r = &mTiles[rendererID];

            // The center of the actor is the center of all the tiles
            float tileX = (xIdx * 16.0f) - (mWidth * 16.0f / 2) + 8.0f;
            float tileY = (yIdx * 16.0f) - (mHeight * 16.0f / 2) + 8.0f;

            float rotX = tileX * cosA - tileY * sinA;
            float rotY = tileX * sinA + tileY * cosA;

            float finalX = center.x + rotX - 8.0f;
            float finalY = center.y + rotY - 8.0f;

            r->setPos(finalX, finalY, zOrder);
            r->setScaleAngle(mScale.x, -mAngle.y);

            rendererID++;
        }
    }
}

void daLiftBgBase_c::setVisible(bool isVisible) {
    if (isVisible == mVisible) {
        return;
    }

    mVisible = isVisible;

    dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);

    for (int i = 0; i < mTileCount; i++) {
        if (mTiles[i].mTileNumber > 0) {
            if (mVisible) {
                list->addPanelObjList(&mTiles[i]);
            } else {
                list->removePanelObjList(&mTiles[i]);
            }
        }
    }
}