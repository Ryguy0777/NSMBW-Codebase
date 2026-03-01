#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_kp_path_manager.hpp>

#include <constants/sound_list.h>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_course_select_manager.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/bases/d_s_restart_crsin.hpp>
#include <game/bases/d_s_stage.hpp>
#include <lib/MSL/math.h>
#include <new/bases/koopatlas/d_a_kp_player.hpp>
#include <new/bases/koopatlas/d_kp_camera.hpp>
#include <new/bases/koopatlas/d_kp_map.hpp>
#include <new/bases/koopatlas/d_s_koopatlas.hpp>
#include <new/constants/koopatlas_constants.h>

dKpPathManager_c::CompletionData_s dKpPathManager_c::s_cmpData;
u8 *dKpPathManager_c::sp_openPathData;
u8 *dKpPathManager_c::sp_openNodeData;

dKpPathManager_c::~dKpPathManager_c() {
    bool isEnter8Castle = (s_cmpData.mPrevLevelID[0] == 7) && (s_cmpData.mPrevLevelID[1] == 24) && s_cmpData.mChkSavePrompt;
    if (!mIsCourseIn && !isEnter8Castle) {
        clearPathData();
    }

    if (mIsCourseIn) {
        resetCompletionData();

        dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
        if ((mEnteredLevel->mDisplayLevel >= 21 && mEnteredLevel->mDisplayLevel <= 27 && mEnteredLevel->mDisplayLevel != 26)
                || (mEnteredLevel->mDisplayLevel >= 29 && mEnteredLevel->mDisplayLevel <= 42)) {
            if (!save->isCourseDataFlag(mEnteredLevel->mWorldSlot, mEnteredLevel->mLevelSlot, dMj2dGame_c::GOAL_NORMAL)) {
                s_cmpData.mChkSavePrompt = true;
            }
        }

        s_cmpData.mPrevLevelID[0] = mEnteredLevel->mWorldSlot;
        s_cmpData.mPrevLevelID[1] = mEnteredLevel->mLevelSlot;

        if (mPenguinSlideHandle.IsAttachedSound()) {
            mPenguinSlideHandle.Stop(5);
        }

        // Now, a fuckton of checks for the various possible things we can finish!
        dLevelInfo_c *lvlInfo = &dLevelInfo_c::m_instance;
        u32 theseConds = save->getCourseDataFlag(mEnteredLevel->mWorldSlot, mEnteredLevel->mLevelSlot);

        int coinNum = 0;
        int exitNum = 0;
        int glbCoinNum = 0;
        int glbCoinNumExcW9 = 0;
        int glbExitNum = 0;

        int totalCoinNum = 0;
        int totalExitNum = 0;
        int totalGlbCoinNum = 0;
        int totalGlbCoinNumExcW9 = 0;
        int totalGlbExitNum = 0;

        for (int sIdx = 0; sIdx < lvlInfo->sectionCount(); sIdx++) {
            dLevelInfo_c::section_s *sect = lvlInfo->getSection(sIdx);

            for (int idx = 0; idx < sect->mLevelCount; idx++) {
                dLevelInfo_c::entry_s *entry = &sect->mLevels[idx];
                u32 entryConds = save->getCourseDataFlag(entry->mWorldSlot, entry->mLevelSlot);

                // Only track actual levels
                if (!(entry->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL)) {
                    continue;
                }

                // Collection for this world
                if (entry->mDisplayWorld == mEnteredLevel->mDisplayWorld) {
                    totalCoinNum++;
                    if ((entryConds & dMj2dGame_c::COIN_MASK) == dMj2dGame_c::COIN_MASK) {
                        coinNum++;
                    }

                    // Normal exit
                    if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) {
                        totalExitNum++;
                        if (entryConds & dMj2dGame_c::GOAL_NORMAL) {
                            exitNum++;
                        }
                    }

                    // Secret exit
                    if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET) {
                        totalExitNum++;
                        if (entryConds & dMj2dGame_c::GOAL_SECRET) {
                            exitNum++;
                        }
                    }
                }

                // Global collection
                totalGlbCoinNum++;
                if ((entryConds & dMj2dGame_c::COIN_MASK) == dMj2dGame_c::COIN_MASK) {
                    glbCoinNum++;
                }

                if (entry->mDisplayWorld != 9) {
                    totalGlbCoinNumExcW9++;
                    if ((entryConds & dMj2dGame_c::COIN_MASK) == dMj2dGame_c::COIN_MASK) {
                        glbCoinNumExcW9++;
                    }
                }

                // Normal exit
                if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) {
                    totalGlbExitNum++;
                    if (entryConds & dMj2dGame_c::GOAL_NORMAL) {
                        glbExitNum++;
                    }
                }

                // Secret exit
                if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET) {
                    totalGlbExitNum++;
                    if (entryConds & dMj2dGame_c::GOAL_SECRET) {
                        glbExitNum++;
                    }
                }
            }
        }

        // Don't consider non-levels for this
        if (!(mEnteredLevel->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL)) {
            return;
        }

        // Check if we're candidates for any of these
        int everythingFlag = 0, gEverythingFlag = 0;
        if (coinNum == totalCoinNum) {
            everythingFlag |= 1;
        }
        if (exitNum == totalExitNum) {
            everythingFlag |= 2;
        }
        if (glbCoinNum == totalGlbCoinNum) {
            gEverythingFlag |= 1;
        }
        if (glbExitNum == totalGlbExitNum) {
            gEverythingFlag |= 2;
        }

        // Check if we could obtain every star coin
        if ((theseConds & dMj2dGame_c::COIN_MASK) != dMj2dGame_c::COIN_MASK) {
            if ((coinNum + 1) == totalCoinNum) {
                s_cmpData.mIsCmpCoins = true;
                everythingFlag |= 1;
            }
            if ((glbCoinNum + 1) == totalGlbCoinNum) {
                s_cmpData.mIsCmpAllCoins = true;
                gEverythingFlag |= 1;
            }
            if ((glbCoinNumExcW9 + 1) == totalGlbCoinNumExcW9)
                s_cmpData.mIsCmpCoinsExcW9 = true;
        }

        // Check if we could obtain every exit
        int elExits = 0, elTotalExits = 0;
        if (mEnteredLevel->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) {
            elTotalExits++;
            if (theseConds & dMj2dGame_c::GOAL_NORMAL) {
                elExits++;
            }
        }
        if (mEnteredLevel->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET) {
            elTotalExits++;
            if (theseConds & dMj2dGame_c::GOAL_SECRET) {
                elExits++;
            }
        }

        if ((elExits + 1) == elTotalExits) {
            if ((exitNum + 1) == totalExitNum) {
                s_cmpData.mIsCmpExits = true;
                everythingFlag |= 2;
            }
            if ((glbExitNum + 1) == totalGlbExitNum) {
                s_cmpData.mIsCmpAllExits = true;
                gEverythingFlag |= 2;
            }
        }

        // And could we obtain EVERYTHING?
        if ((s_cmpData.mIsCmpCoins || s_cmpData.mIsCmpExits) && everythingFlag == 3) {
            s_cmpData.mIsCmpWorld = true;
        }
        if ((s_cmpData.mIsCmpAllCoins || s_cmpData.mIsCmpAllExits) && gEverythingFlag == 3) {
            s_cmpData.mIsCmpAll = true;
        }
    }
}

void dKpPathManager_c::create() {
    // Only reset data if coming from the File Select
    if (dScene_c::m_oldScene == fProfile::GAME_SETUP) {
        resetCompletionData();
    }

    dScKoopatlas_c *wm = dScKoopatlas_c::m_instance;

    mpCurrentPath = nullptr;
    mIsMoving = false;
    mIsJumpAnm = false;
    mScaleDuration = -1;
    mTimer = 0.0;
    mReverseOnPath = false;

    mDispSavePrompt = ((wm->mParam & dScKoopatlas_c::REQUEST_SAVE) != 0);
    mDidAutoWalkCheck = ((wm->mParam & dScKoopatlas_c::CHECK_AUTOWALK) != 0);
    mAutoWalkMode = ((wm->mParam & dScKoopatlas_c::AFTER_FORTRESS) != 0);

    mpPathLayer = wm->mMapData.mpPathLayer;

    SpammyReport("Setting up PathManager\n");
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

    mustComplainToMapCreator = false;

    SpammyReport("Unlocking paths\n");
    mIsCourseIn = false;
    mCourseInDelay = -1;
    unlockPaths();

    mInitialPlyAnmDelay = -1;
    if (dScStage_c::m_exitMode == dScStage_c::EXIT_LOSE) {
        mDoCourseFailAnm = true;
        daKPPlayer_c::m_instance->mVisible = false;
        dScStage_c::m_exitMode = dScStage_c::EXIT_CLEAR;
    } else if ((dScStage_c::m_exitMode == dScStage_c::EXIT_CLEAR) && s_cmpData.mPrevLevelID[0] < 0x80 && !wm->mIsAfter8Castle) {
        mDoCourseClearAnm = true;
        daKPPlayer_c::m_instance->mVisible = false;
    }

    SpammyReport("done\n");

    // Figure out what path node to start at
    if (wm->mParam & dScKoopatlas_c::CHANGE_NODE) {
        // Start off from a "Change"
        u8 changeID = (wm->mParam >> 20) & 0xFF;
        SpammyReport("entering at Change ID %d\n", changeID);
        SpammyReport("Path layer: %p\n", mpPathLayer);
        SpammyReport("Node count: %d\n", mpPathLayer->mNodeNum);

        bool found = false;

        for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
            dKPNode_s *node = mpPathLayer->mpNodes[i];
            SpammyReport("Checking node: %p\n", node);

            if (node->mNodeType == dKPNode_s::CHANGE && node->mCurrID == changeID) {
                found = true;
                mpCurrentNode = node;

                SpammyReport("Found CHANGE node: %d %p\n", changeID, node);

                // Figure out where we should move to
                dKPPath_s *exitTo = nullptr;

                for (int i = 0; i < 4; i++) {
                    dKPPath_s *candidateExit = node->mpExits[i];
                    //SpammyReport("Candidate exit: %p\n", candidateExit);
                    if (!candidateExit)
                        continue;

                    // Find out if this path is a candidate
                    dKPNode_s *srcNode = node;
                    dKPPath_s *path = candidateExit;

                    while (true) {
                        dKPNode_s *destNode = (path->mpStartPoint == srcNode) ? path->mpEndPoint : path->mpStartPoint;
                        //SpammyReport("Path: %p nodes %p to %p\n", path, srcNode, destNode);
                        int ct = destNode->getOpenExitNum();
                        //SpammyReport("Dest Node available exits: %d; type: %d\n", ct, destNode->type);
                        if (destNode == node || ct > 2 || destNode->mNodeType == dKPNode_s::LEVEL || destNode->mNodeType == dKPNode_s::CHANGE) {
                            exitTo = candidateExit;
                            //SpammyReport("Accepting this node\n");
                            break;
                        }

                        if (ct == 1) {
                            break;
                        }

                        // Where to next?
                        path = destNode->getOpenAcrossPath(path);
                        srcNode = destNode;
                    }

                    if (exitTo) {
                        break;
                    }
                }

                if (!exitTo) {
                    exitTo = node->getAnyExit();
                }
                startMovementTo(exitTo);
                break;
            }
        }

        if (!found) {
            mpCurrentNode = mpPathLayer->mpNodes[0];
            mustComplainToMapCreator = true;
        }

        mInitialDelay = 1;
    } else {
        if (!mPathFadeInDelay) {
            mInitialDelay = 50;
        } else {
            mInitialDelay = 1;
        }

        if (wm->mIsFirstPlay) {
            mInitialDelay = 280;
        }

        int currNode = save->getCurrentPathNode();
        SpammyReport("Saved path node: %d\n", currNode);
        if (currNode >= mpPathLayer->mNodeNum) {
            SpammyReport("Path node out of bounds (%d), using node 0\n", mpPathLayer->mNodeNum);
            mpCurrentNode = mpPathLayer->mpNodes[0];
        } else {
#ifdef KP_CHK_SET_DEFAULT_NODE
            int prevHeld = dGameKey_c::m_instance->mRemocon[0]->mPrevDownButtons;
            if ((prevHeld & WPAD_BUTTON_MINUS) && (prevHeld & WPAD_BUTTON_PLUS) && (prevHeld & WPAD_BUTTON_1)) {
                OSReport("Debug Mode toggled! Putting player at node 0\n");
                mpCurrentNode = mpPathLayer->mpNodes[0];
            } else {
                mpCurrentNode = mpPathLayer->mpNodes[currNode];
            }
#else
            mpCurrentNode = mpPathLayer->mpNodes[currNode];
#endif
        }

        // Set the player position to a specific course
        int destWorld = -1;
        int destLevel = -1;
        bool saveNode = true;

        if (wm->mIsAfterKamekCutscene) {
            destWorld = 8;
            destLevel = 1;
        } else if (wm->mIsAfter8Castle) {
            destWorld = 8;
            destLevel = 5;
        } else if (wm->mIsEndingScene) {
            destWorld = 80;
            destLevel = 80;
            saveNode = false;
        }

        if (destWorld > -1) {
            for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
                dKPNode_s *node = mpPathLayer->mpNodes[i];
                if (node->mNodeType == dKPNode_s::LEVEL && node->mLevelNum[0] == destWorld && node->mLevelNum[1] == destLevel) {
                    mpCurrentNode = node;
                    if (saveNode) {
                        save->setCurrentPathNode(i);
                    }
                    break;
                }
            }
        }
    }

    for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
        if (mpPathLayer->mpNodes[i]->mNodeType == dKPNode_s::LEVEL) {
            mpPathLayer->mpNodes[i]->createCourseNode();
        }
    }

    // Do we have any completions?
    if (s_cmpData.mPrevLevelID[0] < 0x80) {
        u32 conds = save->getCourseDataFlag(s_cmpData.mPrevLevelID[0], s_cmpData.mPrevLevelID[1]);
        dLevelInfo_c::entry_s *entry = dLevelInfo_c::m_instance.getEntryFromSlotID(s_cmpData.mPrevLevelID[0], s_cmpData.mPrevLevelID[1]);

        // Check if we should display a save prompt
        if (s_cmpData.mChkSavePrompt) {
            if (conds & dMj2dGame_c::GOAL_NORMAL) {
                mDispSavePrompt = true;
            }
        }

        // Count exits
        int exits = 0;
        int exitNum = 0;

        if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) {
            exitNum++;
            if (conds & dMj2dGame_c::GOAL_NORMAL) {
                exits++;
            }
        }
        if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET) {
            exitNum++;
            if (conds & dMj2dGame_c::GOAL_SECRET) {
                exits++;
            }
        }

        // Do message checks
        int flag = 0;
        int totalFlag = 0;

        if (s_cmpData.mIsCmpCoins) {
            totalFlag |= 1;
            if ((conds & dMj2dGame_c::COIN_MASK) == dMj2dGame_c::COIN_MASK) {
                flag |= 1;
                mCmpMsgType = CMP_MSG_COINS;
            }
        }
        if (s_cmpData.mIsCmpExits) {
            totalFlag |= 2;
            if (exits == exitNum) {
                flag |= 2;
                mCmpMsgType = CMP_MSG_EXITS;
            }
        }
        if (s_cmpData.mIsCmpWorld && flag == totalFlag) {
            mDispSavePrompt = true;
            mCmpMsgType = CMP_MSG_WORLD;
        }

        if (s_cmpData.mIsCmpCoinsExcW9) {
            if ((conds & dMj2dGame_c::COIN_MASK) == dMj2dGame_c::COIN_MASK) {
                mCmpMsgType = CMP_MSG_GLOBAL_COINS_EXC_W9;
            }
        }

        int glbFlag = 0;
        int glbTotalFlag = 0;

        if (s_cmpData.mIsCmpAllCoins) {
            glbTotalFlag |= 1;
            if ((conds & dMj2dGame_c::COIN_MASK) == dMj2dGame_c::COIN_MASK) {
                glbFlag |= 1;
                mCmpMsgType = CMP_MSG_GLOBAL_COINS;
            }
        }
        if (s_cmpData.mIsCmpAllExits) {
            glbTotalFlag |= 2;
            if (exits == exitNum) {
                glbFlag |= 2;
                mCmpMsgType = CMP_MSG_GLOBAL_EXITS;
            }
        }
        if (s_cmpData.mIsCmpAll && glbFlag == glbTotalFlag) {
            //save->titleScreenWorld = 3;
            //save->titleScreenLevel = 10;

            mDispSavePrompt = true;
            mCmpMsgType = CMP_MSG_EVERYTHING;
        }
    }

    resetCompletionData();

    if (wm->mIsAfterKamekCutscene || wm->mIsAfter8Castle || wm->mIsEndingScene) {
        copyWorldDefToSave(wm->mMapData.findWorldDef(1));
    }

    finalisePathUnlocks();
}

void dKpPathManager_c::execute() {
    dScKoopatlas_c *wm = dScKoopatlas_c::m_instance;

    // Entering a course
    if (mIsCourseIn) {
        if (mCourseInDelay > 0) {
            mCourseInDelay--;

            if (mCourseInDelay == 0) {
                dScKoopatlas_c::m_instance->startGame(mEnteredLevel);
            }
        }
        return;
    }

    if (mInitialDelay > 0) {
        mInitialDelay--;

        if (mInitialDelay == 0) {
            if (mDoCourseFailAnm) {
                daKPPlayer_c::m_instance->mVisible = true;
                daKPPlayer_c::m_instance->startAnimation(dPyMdlBase_c::ENDING_WAIT, 1.0f, 0.0f, 0.0f);
                mInitialPlyAnmDelay = 60;

                SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_CS_COURSE_MISS, 1);
            } else if (mDoCourseClearAnm) {
                daKPPlayer_c::m_instance->mVisible = true;
                if (dScKoopatlas_c::m_instance->mIsAfter8Castle) {
                    mInitialPlyAnmDelay = 1;
                } else {
                    daKPPlayer_c::m_instance->startAnimation(dPyMdlBase_c::DM_SURP_WAIT, 1.0f, 0.0f, 0.0f);
                    mInitialPlyAnmDelay = 38;

                    nw4r::snd::SoundHandle something;
                    if (!wm->mIsEndingScene) {
                        SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_CS_JUMP, 1);
                    }
                }
            }
        }
        return;
    }

    if (mInitialPlyAnmDelay > 0) {
        mInitialPlyAnmDelay--;

        if (mInitialPlyAnmDelay == 0) {
            daKPPlayer_c::m_instance->startAnimation(dPyMdlBase_c::WAIT_SELECT, 1.0f, 0.0f, 0.0f);
        }

        if (mDoCourseClearAnm && (mInitialPlyAnmDelay == 9) && !wm->mIsEndingScene) {
            SndAudioMgr::sInstance->startSystemSe(SE_PLY_LAND_ROCK, 1);
        }
        return;
    }

    // Handle path unlocking
    if (mPathFadeInDelay > 0) {
        mPathFadeInDelay--;

        if (mPathFadeInDelay <= 0) {
            if (mIsCamBoundsValid) {
                dKPCamera_c::m_instance->mCurrentX = mpCurrentNode->mPosX;
                dKPCamera_c::m_instance->mCurrentY = -mpCurrentNode->mPosY;
                dKPCamera_c::m_instance->panToBounds(mCamMinX, mCamMinY, mCamMaxX, mCamMaxY);

                mIsCamPanToPaths = true;
            } else {
                mPathUnlockAlpha = 0;
            }

            SndAudioMgr::sInstance->startSystemSe(SE_OBJ_GEN_LOAD, 1);
        }
    }

    if (mIsCamPanToPaths) {
        if (dKPCamera_c::m_instance->mIsPanning) {
            return;
        }

        mIsCamPanToPaths = false;
        mPathUnlockAlpha = 0;
    }

    if (mPathUnlockAlpha != -1) {
        mPathUnlockAlpha += PATH_ALPHA_INC;

        for (int i = 0; i < mpPathLayer->mPathNum; i++) {
            dKPPath_s *path = mpPathLayer->mpPaths[i];

            if (path->mIsOpen == dKPPath_s::NEWLY_OPEN) {
                path->setLayerAlpha(mPathUnlockAlpha);
            }
        }

        for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
            dKPNode_s *node = mpPathLayer->mpNodes[i];

            if (node->mIsNewlyOpen) {
                node->setLayerAlpha(mPathUnlockAlpha);
            }
        }

        // We've reached the end
        if (mPathUnlockAlpha == PATH_OPEN_ALPHA) {
            mPathUnlockAlpha = -1;

            mNodeUnlockDuration = 15;

            for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
                dKPNode_s *node = mpPathLayer->mpNodes[i];

                if (node->mIsNewlyOpen && node->mNodeType == dKPNode_s::LEVEL) {
                    mVec3_c effPos(node->mPosX, -node->mPosY + 4.0, 3300.0f);
                    mAng3_c effAng(0x2000, 0, 0);
                    mVec3_c effScale(0.8f, 0.8f, 0.8f);
                    mEf::createEffect("Wm_cs_pointlight", 0, &effPos, &effAng, &effScale);

                    SndAudioMgr::sInstance->startSystemSe(SE_OBJ_GEN_NEW_COURSE, 1);
                }
            }
        }

        return;
    }

    if (mNodeUnlockDuration > 0) {
        mNodeUnlockDuration--;

        if (mNodeUnlockDuration == 0) {
            mPanBackDelay = 20;
        }
        return;
    }

    if (mPanBackDelay > 0) {
        mPanBackDelay--;

        if (mPanBackDelay == 0 && mIsCamBoundsValid) {
            dKPCamera_c::m_instance->panToPosition(mpCurrentNode->mPosX, -mpCurrentNode->mPosY, STD_ZOOM);
            mIsCamPanFromPaths = true;
        }
        return;
    }

    if (mIsCamPanFromPaths) {
        if (dKPCamera_c::m_instance->mIsPanning) {
            return;
        }
        mIsCamPanFromPaths = false;
        dKPCamera_c::m_instance->mFollowPlayer = true;
    }

    if (mCmpAnimDuration > 0) {
        mCmpAnimDuration--;
        if (mCmpAnimDuration == 60) {
            SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_CLEAR_MULTI, 1);
        } else if (mCmpAnimDuration == 0) {
            daKPPlayer_c::m_instance->startAnimation(dPyMdlBase_c::WAIT_SELECT, 1.0f, 0.0f, 0.0f);
        }
        return;
    }

    if (mCmpMsgDelay > 0) {
        mCmpMsgDelay--;

        if (mCmpMsgDelay == 0) {
            mIsCmpMsgActive = true;
        }
        return;
    }

    // Just in case
    if (mIsCmpMsgActive) {
        return;
    }

    if (mCmpMsgType > 0) {
        PathMngReport("We have a completion message type: %d\n", mCmpMsgType);
        mCmpAnimDuration = 154;

        int soundID;
        if (mCmpMsgType == CMP_MSG_GLOBAL_COINS) {
            soundID = STRM_BGM_STAR_COIN_CMPLT_ALL;
            mCmpMsgDelay = 240 - mCmpAnimDuration;
        } else if (mCmpMsgType == CMP_MSG_EVERYTHING) {
            soundID = STRM_BGM_ALL_CMPLT_5STARS;
            mCmpMsgDelay = 216 - mCmpAnimDuration;
        } else {
            soundID = STRM_BGM_STAR_COIN_CMPLT_WORLD;
            mCmpMsgDelay = 1; //138;
        }

        SndAudioMgr::sInstance->startSystemSe(soundID, 1);
        daKPPlayer_c::m_instance->startAnimation(dPyMdlBase_c::COIN_COMP, 1.0f, 0.0f, 0.0f);
        return;
    }

    // Go no further for the ending
    if (dScKoopatlas_c::m_instance->mIsEndingScene) {
        if (!mEndingSceneComplete) {
            if (!mDidEndingSceneSave) {
                mDidEndingSceneSave = true;

                dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
                //save->titleScreenWorld = 3;
                //save->titleScreenLevel = 9;

                dCourseSelectManager_c::m_instance->saveGame(false);
            } else {
                if (!dSaveMng_c::m_instance->isNandBusy()) {
                    dScRestartCrsin_c::startTitle(false, 0);
                    mEndingSceneComplete = true;
                }
            }
        }
        return;
    }

    // Check if we should auto-walk to the next point
    if (mDispSavePrompt && !mDidAutoWalkCheck) {
        mDidAutoWalkCheck = true;

        // TODO: Control this via WorldInfo
        static const int endLevels[11][3] = {
            {1, 38, 1}, // W1 right
            {2, 38, 2}, // W2 up
            {3, 38, 0}, // W3 left
            {4, 38, 1}, // W4 right
            {5, 38, 1}, // W5 right
            {6, 38, 2}, // W6 up
            {7, 38, 1}, // W7 right
            {10, 5, 2}, // GW up
            {10, 10, 0}, // MM left
            {10, 15, 3}, // CC down
            {10, 25, 1}, // SC right
        };

        int w = (s_cmpData.mPrevLevelID[0] & 0x7F) + 1;
        int l = s_cmpData.mPrevLevelID[1] + 1;

        for (int i = 0; i < 11; i++) {
            if (endLevels[i][0] == w && endLevels[i][1] == l) {
                mAutoWalkMode = true;
                startMovementTo(mpCurrentNode->mpExits[endLevels[i][2]]);
                return;
            }
        }
    }

    if (mAutoWalkMode) {
        if (mIsMoving) {
            moveThroughPath(-1);
        } else {
            mAutoWalkMode = false;
        }
        return;
    }

    if (mDispSavePrompt) {
        dScKoopatlas_c::m_instance->showSaveWindow();
        mDispSavePrompt = false;
        return;
    }

    if (!mInitialLoadComplete) {
        dScKoopatlas_c::m_instance->startMusic();
        dKPHud_c::m_instance->loadInitially();
        mInitialLoadComplete = true;
        return;
    }

    // Prevent the player from reversing on a path after triggering a Map Change
    if (mDisableInput && mIsMoving) {
        moveThroughPath(-1);
        return;
    }

    int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    // Left, right, up, down
    int pressedDir = getPressedDir(pressed);

    if (mIsMoving) {
        moveThroughPath(pressedDir);
    } else {
        if (pressedDir >= 0) {
            // Use an exit if possible
            if (canUseExit(mpCurrentNode->mpExits[pressedDir])) {
                startMovementTo(mpCurrentNode->mpExits[pressedDir]);
            } else {
                // TODO: maybe remove this? got to see how it looks
                static u16 directions[] = {-0x4000,0x4000,-0x7FFF,0};
                daKPPlayer_c::m_instance->setTargetRotY(directions[pressedDir]);
            }
        } else if (pressed & WPAD_BUTTON_2) {
            activatePoint();
        }
    }
}

bool dKpPathManager_c::isPathMgrActive() {
    if (mIsCourseIn || (mNodeUnlockDuration > 0) || (mCmpMsgDelay > 0) ||
            (mInitialDelay > 0) || (mInitialPlyAnmDelay > 0) ||
            mIsCamPanToPaths || mIsCamPanFromPaths ||
            (mPanBackDelay > 0) || !mInitialLoadComplete ||
            (mPathFadeInDelay > 0) || (mPathUnlockAlpha != -1) ||
            dScKoopatlas_c::m_instance->mIsEndingScene) {
        return true;
    }

    if (mIsMoving) {
        return true;
    }
    return false;
}

void dKpPathManager_c::unlockPaths() {
    u8 *oldPathAvData = sp_openPathData;
    sp_openPathData = new u8[mpPathLayer->mPathNum];

    u8 *oldNodeAvData = sp_openPathData;
    sp_openPathData = new u8[mpPathLayer->mPathNum];

    SpammyReport("Unlocking paths\n");

    // Unlock all needed paths
    for (int i = 0; i < mpPathLayer->mPathNum; i++) {
        dKPPath_s *path = mpPathLayer->mpPaths[i];

        sp_openPathData[i] = path->mIsOpen;

        //SpammyReport("Path %d: %d\n", i, path->isAvailable);
        // If this path is not "always open", then reset its alpha
        path->setLayerAlpha((path->mIsOpen == dKPPath_s::ALWAYS_OPEN) ? PATH_OPEN_ALPHA : PATH_LOCK_ALPHA);
    }

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

    mCollectStarCoinNum = dGameCom::getStarCoinCount();
    mUnspentStarCoinNum = mCollectStarCoinNum - save->mSpentStarCoins;

    u8 *in = (u8*)dScKoopatlas_c::m_instance->mMapData.mpData->mpUnlockData;
    SpammyReport("UNLOCKING PATHS: Unlock data @ %p\n", in);

    int cmdID = 0;

    while (*in != 0) {
        UnlockCmdReport("[%p] Cmd %d: Evaluating condition\n", in, cmdID);
        // Begin processing a block
        bool value = evaluateUnlockCondition(in, save, 0);
        UnlockCmdReport("[%p] Cmd %d: Condition evaluated, result: %d\n", in, cmdID, value);
        //UnlockCmdReport("Unlock condition: %d\n", value);

        // Get what it's supposed to affect
        // For now we'll assume that it affects one or more paths
        u8 affectedCount = *(in++);
        UnlockCmdReport("[%p] Cmd %d: Affects %d path(s)\n", in, cmdID, affectedCount);

        for (int i = 0; i < affectedCount; i++) {
            u8 one = *(in++);
            u8 two = *(in++);
            u16 pathID = (one << 8) | two;
            UnlockCmdReport("[%p] Cmd %d: Affected %d: PathID: %d\n", in, cmdID, i, pathID);

            dKPPath_s *path = mpPathLayer->mpPaths[pathID];
            UnlockCmdReport("[%p] Cmd %d: Affected %d: Path: %p\n", in, cmdID, i, path);
            path->mIsOpen = value ? dKPPath_s::OPEN : dKPPath_s::NOT_OPEN;
            UnlockCmdReport("[%p] Cmd %d: Affected %d: IsAvailable written\n", in, cmdID, i);
            sp_openPathData[pathID] = value ? dKPPath_s::OPEN : dKPPath_s::NOT_OPEN;
            UnlockCmdReport("[%p] Cmd %d: Affected %d: AvailabilityData written\n", in, cmdID, i);
            // NEWLY_AVAILABLE is set later, when that stuff is figured out

            path->setLayerAlpha(value ? PATH_OPEN_ALPHA : PATH_LOCK_ALPHA);
            UnlockCmdReport("[%p] Cmd %d: Affected %d: Layer alpha applied\n", in, cmdID, i);
        }

        UnlockCmdReport("[%p] Cmd %d: %d affected path(s) processed\n", in, cmdID, affectedCount);

        cmdID++;
    }

    SpammyReport("UNLOCKING PATHS: All complete @ %p\n", in);

    for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
        dKPNode_s *node = mpPathLayer->mpNodes[i];
        sp_openNodeData[i] = node->chkOpenStatus();

        // Set special UNLOCKED data flag
        if (node->mNodeType == node->LEVEL && node->chkOpenStatus() && node->mLevelNum[1] != 99) {
            save->onCourseDataFlag(node->mLevelNum[0]-1, node->mLevelNum[1]-1, 0x200);
        }
    }

    // Did anything become newly available?!
    mNewOpenPathNum = 0;
    mNewOpenNodeNum = 0;

    dScKoopatlas_c *wm = dScKoopatlas_c::m_instance;
    bool forceFlag = (wm->mIsAfter8Castle || wm->mIsAfterKamekCutscene);

    if (!wm->mIsEndingScene && (oldPathAvData || forceFlag)) {
        for (int i = 0; i < mpPathLayer->mPathNum; i++) {
            if ((sp_openPathData[i] > 0) && (forceFlag || oldPathAvData[i] == 0)) {
                if (forceFlag && sp_openPathData[i] == dKPPath_s::ALWAYS_OPEN) {
                    continue;
                }

                dKPPath_s *path = mpPathLayer->mpPaths[i];
                path->mIsOpen = dKPPath_s::NEWLY_OPEN;
                mNewOpenPathNum++;

                // Set this path's alpha to 0, we'll fade it in later
                path->setLayerAlpha(PATH_LOCK_ALPHA);
            }
        }

        // Check nodes too
        for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
            if ((sp_openNodeData[i] > 0) && (forceFlag || oldNodeAvData[i] == 0)) {
                dKPNode_s *node = mpPathLayer->mpNodes[i];
                node->mIsNewlyOpen = true;
                mNewOpenNodeNum++;
            }
        }
    }

    if (oldPathAvData) {
        delete[] oldPathAvData;
        delete[] oldNodeAvData;
    }

    if (wm->mIsEndingScene) {
        dKPNode_s *yoshiHouse = nullptr;
        for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
            dKPNode_s *node = mpPathLayer->mpNodes[i];

            if (node->mNodeType != dKPNode_s::LEVEL) {
                continue;
            }
            if (node->mLevelNum[0] != YOSHI_HOUSE_WORLD) {
                continue;
            }
            if (node->mLevelNum[1] != YOSHI_HOUSE_STAGE) {
                continue;
            }

            yoshiHouse = node;
            break;
        }

        if (yoshiHouse != nullptr) {
            dKPNode_s *mpCurrentNode = yoshiHouse;
            dKPPath_s *nextPath = yoshiHouse->mpExitR;

            while (true) {
                if (nextPath->mIsOpen == dKPPath_s::OPEN) {
                    nextPath->mIsOpen = dKPPath_s::NEWLY_OPEN;
                    mNewOpenPathNum++;
                    nextPath->setLayerAlpha(PATH_LOCK_ALPHA);
                }

                dKPNode_s *nextNode = nextPath->getOtherNodeTo(mpCurrentNode);
                if (nextNode == nullptr) {
                    break;
                }

                if (nextNode->chkOpenStatus()) {
                    nextNode->mIsNewlyOpen = true;
                    mNewOpenNodeNum++;
                }

                mpCurrentNode = nextNode;
                nextPath = nextNode->getAcrossPath(nextPath);

                if (nextPath == nullptr) {
                    break;
                }
            }
        }
    }

    // Now set all node alphas
    for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
        dKPNode_s *node = mpPathLayer->mpNodes[i];

        node->setLayerAlpha((node->chkOpenStatus() & !node->mIsNewlyOpen) ? PATH_OPEN_ALPHA : PATH_LOCK_ALPHA);
    }
}

void dKpPathManager_c::finalisePathUnlocks() {
    // If anything was new, set it as such
    if (mNewOpenPathNum || mNewOpenNodeNum) {
        mPathFadeInDelay = 30;
        findCameraBoundsForUnlockedPaths();
    }
    mPathUnlockAlpha = -1;
}

bool dKpPathManager_c::evaluateUnlockCondition(u8 *&in, dMj2dGame_c *save, int stack) {
    UnlockCmdReport("[%p] CondStk:%d begin\n", in, stack);
    u8 controlByte = *(in++);

    u8 conditionType = (controlByte >> 6);
    UnlockCmdReport("[%p] CondStk:%d control byte: %d; condition type: %d\n", in, stack, controlByte, conditionType);

    if (conditionType == 0) {
        u8 one;
        u8 two;
        int compareOne;
        int compareTwo;

        u8 subConditionType = controlByte & 0x3F;
        switch (subConditionType) {
            case 0:
            case 1:
            case 2:
            case 3:
                one = *(in++);
                two = *(in++);

                compareOne = (one & 0x80) ? mUnspentStarCoinNum : mCollectStarCoinNum;
                compareTwo = ((one & 0x7F) << 8) | two;

                switch (subConditionType) {
                    case 0:
                        return compareOne == compareTwo;
                    case 1:
                        return compareOne != compareTwo;
                    case 2:
                        return compareOne < compareTwo;
                    case 3:
                        return compareOne > compareTwo;
                }

            case 15:
                UnlockCmdReport("[%p] CondStk:%d end, returning CONSTANT 1\n", in, stack);
                return true;
            default:
                UnlockCmdReport("[%p] CondStk:%d unknown subCondition %d, returning 0\n", in, stack, subConditionType);
                return false;
        }
    }

    if (conditionType == 1) {
        // Simple level

        bool isSecret = (controlByte & 0x10);
        u8 worldNumber = controlByte & 0xF;
        u8 levelNumber = *(in++);
        UnlockCmdReport("[%p] CondStk:%d level, w:%d l:%d secret:%d\n", in, stack, worldNumber, levelNumber, isSecret);

        u32 conds = save->getCourseDataFlag(worldNumber, levelNumber);
        UnlockCmdReport("[%p] CondStk:%d returning for level conditions: %d / %x\n", in, stack, conds, conds);

        if (isSecret) {
            return (conds & dMj2dGame_c::GOAL_NORMAL) != 0;
        } else {
            return (conds & dMj2dGame_c::GOAL_SECRET) != 0;
        }
    }

    // Type: 2 = AND, 3 = OR
    bool isAnd = (conditionType == 2);
    bool isOr = (conditionType == 3);

    bool value = isOr ? false : true;

    u8 termCount = (controlByte & 0x3F) + 1;
    UnlockCmdReport("[%p] CondStk:%d and:%d or:%d startValue:%d termCount:%d\n", in, stack, isAnd, isOr, value, termCount);

    for (int i = 0; i < termCount; i++) {
        bool what = evaluateUnlockCondition(in, save, stack+1);

        if (isOr) {
            value |= what;
        } else {
            value &= what;
        }
    }

    UnlockCmdReport("[%p] CondStk:%d end, returning %d\n", in, stack, value);
    return value;
}

int dKpPathManager_c::getPressedDir(int buttons) {
    if (buttons & WPAD_BUTTON_UP) return 0;
    else if (buttons & WPAD_BUTTON_DOWN) return 1;
    else if (buttons & WPAD_BUTTON_RIGHT) return 2;
    else if (buttons & WPAD_BUTTON_LEFT) return 3;
    return -1;
}

void dKpPathManager_c::startMovementTo(dKPPath_s *path) {
    if (!path->mIsOpen) {
        return;
    }

    SpammyReport("Moving to path %p [%d,%d to %d,%d]\n", path, path->mpStartPoint->mPosX, path->mpStartPoint->mPosY, path->mpEndPoint->mPosX, path->mpEndPoint->mPosY);

    if ((mpCurrentNode != nullptr) && (dKPHud_c::m_instance)) {
        dKPHud_c::m_instance->leftNode();
    }

    mIsEnterNode = false;

    mIsMoving = true;
    mReverseOnPath = (path->mpEndPoint == mpCurrentNode);

    mpCurrentPath = path;

    // Calculate direction of the path
    short deltaX = path->mpEndPoint->mPosX - path->mpStartPoint->mPosX;
    short deltaY = path->mpEndPoint->mPosY - path->mpStartPoint->mPosY;
    u16 direction = (u16)(atan2(deltaX, deltaY) / ((M_PI * 2) / 65536.0));

    if (mReverseOnPath) {
        direction += 0x8000;
    }

    daKPPlayer_c *player = daKPPlayer_c::m_instance;

    // Consider adding these as options
    // wall_walk_l = 60,
    // wall_walk_r = 61,

    // hang_walk_l = 65,
    // hang_walk_r = 66,

    struct PathAction_s {
        dPyMdlBase_c::ChrAnmType_e mAnmType;
        float mAnmUpdRate;
        float mAnmUnk;

        s16 mForceAngle;
        float mForceSpeed;

        int mLoopSoundID;
        int mStartSoundID;

        const char *mpLoopEffectName;
        const char *mpStartEffectName;
    };

    static const PathAction_s sc_pathActions[] = {
        // Walking
        {dPyMdlBase_c::RUN, 2.0f, 10.0f, -1, -1.0f, SE_PLY_FOOTNOTE_DIRT,     SE_NULL, nullptr,            nullptr}, // TODO: maybe make this use default particles?
        {dPyMdlBase_c::RUN, 2.0f, 10.0f, -1, -1.0f, SE_PLY_FOOTNOTE_CS_SAND,  SE_NULL, "Wm_mr_foot_sand",  nullptr},
        {dPyMdlBase_c::RUN, 2.0f, 10.0f, -1, -1.0f, SE_PLY_FOOTNOTE_CS_SNOW,  SE_NULL, "Wm_mr_foot_snow",  nullptr},
        {dPyMdlBase_c::RUN, 2.0f, 10.0f, -1, -1.0f, SE_PLY_FOOTNOTE_CS_WATER, SE_NULL, "Wm_mr_foot_water", nullptr},

        // Jumping
        {dPyMdlBase_c::JUMP, 1.0f, 1.0f,  -1, 2.5f, SE_NULL, SE_PLY_JUMP, nullptr, nullptr},
        {dPyMdlBase_c::JUMP, 1.0f, 10.0f, -1, 2.5f, SE_NULL, SE_PLY_JUMP, nullptr, nullptr},
        {dPyMdlBase_c::JUMP, 1.0f, 10.0f, -1, 2.5f, SE_NULL, SE_PLY_JUMP, nullptr, nullptr},

        // Jump water (actually cannon)
        {dPyMdlBase_c::DM_NOTICE, 1.0f, 10.0f, -1, -1.0f, SE_NULL, SE_VOC_MA_CANNON_SHOT, nullptr, nullptr},

        // Ladder up, left, right
        {dPyMdlBase_c::PEA_PLANT,  1.2f, 10.0f, -0x7FFF, 1.5f, SE_PLY_FOOTNOTE_CS_ROCK_CLIMB, SE_NULL, nullptr, nullptr},
        {dPyMdlBase_c::TREE_CLIMB, 1.2f, 10.0f, -0x4000, 1.5f, SE_PLY_FOOTNOTE_CS_ROCK_CLIMB, SE_NULL, nullptr, nullptr},
        {dPyMdlBase_c::TREE_CLIMB, 1.2f, 10.0f, 0x4000,  1.5f, SE_PLY_FOOTNOTE_CS_ROCK_CLIMB, SE_NULL, nullptr, nullptr},

        // Fall (default?)
        {dPyMdlBase_c::RUN, 2.0f, 10.0f, -1, -1.0f, SE_PLY_FOOTNOTE_DIRT, SE_NULL, nullptr, nullptr},

        // Swim
        {dPyMdlBase_c::SWIM_WAIT, 1.2f, 10.0f, -1, 2.0f, SE_PLY_SWIM, SE_NULL, "Wm_mr_waterswim", nullptr},
        // Run
        {dPyMdlBase_c::B_DASH2, 3.0f, 10.0f, -1, 5.0f, SE_PLY_FOOTNOTE_DIRT, SE_NULL, nullptr, nullptr},
        // Pipe
        {dPyMdlBase_c::WAIT, 2.0f, 10.0f, 0, 1.0f, SE_NULL, SE_PLY_DOKAN_IN_OUT, nullptr, nullptr},
        // Door
        {dPyMdlBase_c::WAIT, 2.0f, 10.0f, -0x7FFF, 0.2f, SE_NULL, SE_OBJ_DOOR_OPEN, nullptr, nullptr},

        // TJumped
        {dPyMdlBase_c::_2JUMPED, 2.0f, 0.0f, -1, -1.0f, SE_NULL, SE_NULL, nullptr, nullptr},

        // Enter cave, this is handled specially
        {dPyMdlBase_c::RUN, 1.0f, 10.0f, -1, 1.0f, SE_NULL, SE_NULL, nullptr, nullptr},

        // Cannon 2
        {dPyMdlBase_c::DM_NOTI_WAIT, 1.0f, 10.0f, -1, -1.0f, SE_NULL, SE_VOC_MA_CANNON_SHOT, nullptr, nullptr},

        // Invisible, this is handled specially
        {dPyMdlBase_c::WAIT, 2.0f, 10.0f, -1, 1.0f, SE_NULL, SE_NULL, nullptr, nullptr},
    };

    mIsJumpAnm = (path->mAction >= dKPPath_s::JUMP && path->mAction <= dKPPath_s::JUMP_SAND);

    float playerScale = 1.6f;

    if (path->mAction == dKPPath_s::ENTER_CAVE_UP) {
        mScaleDuration = 60;
        // What direction does this path go in?
        static u16 directions[] = {-0x4000,0x4000,-0x7FFF,0};
        mIsScaleUp = (deltaY < 0) ^ mReverseOnPath;

        if (!mIsScaleUp) {
            playerScale = 0.0f;
        }
    } else if (mScaleDuration >= 0) {
        // Keep the current scale
        playerScale = player->mScale.x;
    }

    player->mVisible = (path->mAction != dKPPath_s::INVISIBLE);
    player->mScale.x = player->mScale.y = player->mScale.z = playerScale;

    int id = (path->mAction >= dKPPath_s::ACTION_NUM) ? 0 : (int)path->mAction;
    int whichAnim = sc_pathActions[id].mAnmType;
    float updateRate = sc_pathActions[id].mAnmUpdRate;

    if (whichAnim == dPyMdlBase_c::SWIM_WAIT) {
        int playerMode = player->mpPyMdlMng->mpMdl->mPlayerMode;

        if (playerMode == POWERUP_MINI_MUSHROOM) {
            whichAnim = dPyMdlBase_c::B_DASH;
            updateRate = 2.5f;
        } else if (playerMode == POWERUP_PENGUIN_SUIT) {
            whichAnim = dPyMdlBase_c::P_SLIP;
        }
    }

    if (sc_pathActions[id].mForceAngle != -1) {
        mIsForceAngle = true;
        player->setTargetRotY(sc_pathActions[id].mForceAngle);
    } else if (id == dKPPath_s::JUMP_WATER || id == dKPPath_s::RESERVED_18) {
        // Keep the current rotation
        mIsForceAngle = true;
        dPyMdlBase_c::scPyAnmData[dPyMdlBase_c::DM_NOTICE].mPlayMode = m3d::FORWARD_LOOP;

        SndAudioMgr::sInstance->startSystemSe(SE_OBJ_WARP_CANNON_SHOT, 1);
        dKPMap_c::m_instance->spinLaunchStar();
    } else {
        mIsForceAngle = false;
        player->setTargetRotY(direction);
    }

    player->startAnimation(whichAnim, updateRate, sc_pathActions[id].mAnmUnk, 0.0f);

    mMoveSpeed = (sc_pathActions[id].mForceSpeed >= 0.0f) ? sc_pathActions[id].mForceSpeed : 3.0f;
    mMoveSpeed = path->mPathSpeed * mMoveSpeed * 1.3f;

    if (path->mAction == dKPPath_s::SWIM) {
        // Penguin
        if (player->mpPyMdlMng->mpMdl->mPlayerMode == 3) {
            mMoveSpeed *= 1.1f;
        // Mini
        } else if (player->mpPyMdlMng->mpMdl->mPlayerMode == 5) {
            mMoveSpeed *= 2.0f;
        }
    }

    // Setup looping effect and sound
    if (sc_pathActions[id].mpLoopEffectName != nullptr) {
        player->mHasEffect = true;
        player->mpEffectName = sc_pathActions[id].mpLoopEffectName;
    } else {
        player->mHasEffect = false;
    }

    if (sc_pathActions[id].mLoopSoundID != SE_NULL) {
        player->mHasSound = true;
        player->mSoundName = sc_pathActions[id].mLoopSoundID;
    } else {
        player->mHasSound = false;
    }

    // Setup the initial sound
    if (sc_pathActions[id].mpStartEffectName) {
        mEf::createEffect(sc_pathActions[id].mpStartEffectName, 0, &player->mPos, nullptr, &player->mScale);
    }

    if (path->mAction == dKPPath_s::SWIM) {
        if (player->mpPyMdlMng->mpMdl->mPlayerMode == 5) {
            if (!mIsSwimAnm) {
                if (mDoPlayPenguinSlide) {
                    SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_PNGN_SLIDE, 1);
                }
                SndAudioMgr::sInstance->startSound(&mPenguinSlideHandle, SE_EMY_PENGUIN_SLIDE, 1);
            }
            player->mHasSound = false;
        } else if (player->mpPyMdlMng->mpMdl->mPlayerMode == 3) {
            player->mHasSound = true;
            player->mSoundName = SE_PLY_FOOTNOTE_WATER;
        }

        mIsSwimAnm = true;
    } else {
        if (mIsSwimAnm && mPenguinSlideHandle.IsAttachedSound()) {
            mPenguinSlideHandle.Stop(10);
            mPenguinSlideHandle.DetachSound();
        }
        mIsSwimAnm = false;

        if (sc_pathActions[id].mStartSoundID != SE_NULL) {
            SndAudioMgr::sInstance->startSystemSe(sc_pathActions[id].mStartSoundID, 1);

            // Pitch the jump voice for Mini Mushroom
            if (sc_pathActions[id].mStartSoundID == SE_PLY_JUMP) {
                nw4r::snd::SoundHandle jumpHandle;
                SndAudioMgr::sInstance->startSound(&jumpHandle, SE_VOC_MA_CS_JUMP, 1);
                jumpHandle.SetPitch((player->mpPyMdlMng->mpMdl->mPlayerMode == 3) ? 1.5f : 1.0f);
            }
        }
    }

    // Fixes the player stopping for a frame before walking again
    if (mIsPassThrough) {
        moveThroughPath(-1);
        mIsPassThrough = false;
    }
}

void dKpPathManager_c::moveThroughPath(int pressedDir) {
    dKPNode_s *from = mReverseOnPath ? mpCurrentPath->mpEndPoint : mpCurrentPath->mpStartPoint;
    dKPNode_s* to = mReverseOnPath ? mpCurrentPath->mpStartPoint : mpCurrentPath->mpEndPoint;

    daKPPlayer_c *player = daKPPlayer_c::m_instance;

    if (pressedDir >= 0 && !mIsEnterNode) {
        int deltaX = to->mPosX - from->mPosX;
        int deltaY = to->mPosY - from->mPosY;

        int atanResult = ((int)(atan2(deltaX, deltaY) / ((M_PI * 2) / 360.0)) + 360) % 360;
        // dirs are: left, right, up, down
        int reverseDir;
        if (atanResult >= 225 && atanResult <= 315) {
            reverseDir = 1; // moving Left, so reversing requires Right
        } else if (atanResult >= 45 && atanResult <= 135) {
            reverseDir = 0; // moving Right, so reversing requires Left
        } else if (atanResult > 135 && atanResult < 225) {
            reverseDir = 3; // moving Up, so reversing requires Down
        } else if (atanResult > 315 || atanResult < 45) {
            reverseDir = 2; // moving Down, so reversing requires Up
        }
        PathMngReport("Delta: %d, %d; Degrees: %d (Atan result is %f); Calced dir is %d; Pressed dir is %d\n",
            deltaX, deltaY, atanResult, atan2(deltaX, deltaY), reverseDir, pressedDir);

        if (reverseDir == pressedDir) {
            // Check if we're playing an anim we won't reverse on
            static const dKPPath_s::ActionType_e sc_noReverseList[] = {
                dKPPath_s::JUMP,
                dKPPath_s::JUMP_SAND,
                dKPPath_s::JUMP_SNOW,
                dKPPath_s::JUMP_WATER,
                dKPPath_s::PIPE,
                dKPPath_s::DOOR,
                dKPPath_s::ENTER_CAVE_UP,
                dKPPath_s::INVISIBLE,
                dKPPath_s::RESERVED_18,
                dKPPath_s::ACTION_NUM,
            };

            bool allowReverse = true;
            for (int i = 0;; i++) {
                if (sc_noReverseList[i] == dKPPath_s::ACTION_NUM) {
                    break;
                }
                if (sc_noReverseList[i] == mpCurrentPath->mAction) {
                    allowReverse = false;
                }
            }

            if (allowReverse) {
                mReverseOnPath = !mReverseOnPath;
                if (!mIsForceAngle) {
                    player->setTargetRotY(player->mTargetRotY + 0x8000);
                }

                // Start over with the reversed path
                moveThroughPath(-1);
                return;
            }
        }
    }

    // Process the player scale
    // TODO: make this part of player
    if (mScaleDuration >= 0) {
        float soFar = mScaleDuration * (1.6f / 60.0f);
        float sc = mIsScaleUp ? soFar : (1.6f - soFar);
        player->mScale.x = player->mScale.y = player->mScale.z = sc;

        mScaleDuration--;
    }

    // TODO: Test if this fixes the cave entrance scale issue
    /*if (mScaleDuration == 0 && mIsScaleUp) {
        mScaleDuration = -1;
        player->mScale.x = player->mScale.y = player->mScale.z = 1.6f;
    }*/

    int deltaX = to->mPosX - from->mPosX;
    int deltaY = to->mPosY - from->mPosY;

    Vec movePos = {deltaX, deltaY, 0.0f};
    PSVECNormalize(&movePos, &movePos);
    PSVECScale(&movePos, &movePos, mMoveSpeed);

    if (mIsJumpAnm) {
        bool isFalling;

        if (from->mPosY == to->mPosY) {
            float xDistance = deltaX;
            if (xDistance < 0)
                xDistance = -xDistance;
            float currentPoint = max(to->mPosX, from->mPosX) - player->mPos.x;
            player->mJumpOffset = (xDistance / 3.0f) * sin((currentPoint / xDistance) * 3.1415f);

            if (to->mPosX > from->mPosX) {
                // Moving right
                isFalling = (player->mPos.x > (to->mPosX - (movePos.x * 10.0f)));
            } else {
                // Moving left
                isFalling = (player->mPos.x < (to->mPosX - (movePos.x * 10.0f)));
            }

        } else {
            float startY = (float)from->mPosY;
            float endY = (float)to->mPosY;
            float midpoint = (from->mPosY + to->mPosY) / 2;

            float top, len;
            if (startY > endY) {
                len = startY - endY;
                top = startY - midpoint + 10.0;
            } else {
                len = endY - startY;
                top = endY - midpoint + 10.0;
            }

            if (len == 0.0) {
                len = 2.0;
            }

            float a;
            if (mTimer > 0.0) {
                a = -mTimer;
            } else {
                a = mTimer;
            }

            player->mJumpOffset = -sin(a * M_PI / len) * top;
            mTimer -= movePos.y;

            if (endY > startY) {
                // Moving down
                isFalling = (-player->mPos.y) > (endY - (movePos.y * 10.0f));
            } else {
                // Moving up
                isFalling = (-player->mPos.y) < (endY - (movePos.y * 10.0f));
            }
        }

        if (isFalling) {
            player->startAnimation(dPyMdlBase_c::JUMPED, 1.0f, 10.0f, 0.0f);
        }
    }

    player->mPos.x += movePos.x;
    player->mPos.y -= movePos.y;

    // Show HUD course info if we're about to reach a level
    if (to->mNodeType == dKPNode_s::LEVEL && !mIsEnterNode) {
        Vec remainDist = {to->mPosX - player->mPos.x, to->mPosY + player->mPos.y, 0.0f};
        float distToEnd = PSVECMag(&remainDist);
        //PathMngReport("Distance: %f; To:%d,%d; Player:%f,%f; Diff:%f,%f\n", distToEnd, to->mPosX, to->mPosY, player->mPos.x, player->mPos.y, remainDist.x, remainDist.y);

        if (distToEnd < 64.0f && (dKPHud_c::m_instance != nullptr)) {
            mIsEnterNode = true;
            dKPHud_c::m_instance->enteredNode(to);
        }
    }

    // Check if we've reached the end yet
    if (
            (((movePos.x > 0) ? (player->mPos.x >= to->mPosX) : (player->mPos.x <= to->mPosX)) &&
             ((movePos.y > 0) ? (-player->mPos.y >= to->mPosY) : (-player->mPos.y <= to->mPosY)))
            ||
            (from->mPosX == to->mPosX && from->mPosY == to->mPosY)
       ) {

        mpCurrentNode = to;
        player->mPos.x = to->mPosX;
        player->mPos.y = -to->mPosY;

        mIsJumpAnm = false;
        mTimer = 0.0;

        SpammyReport("Reached path end (%p) with node type %d\n", to, to->mNodeType);

        // Should we stop here?
        bool doStop = false;

        if (to->mNodeType == dKPNode_s::LEVEL) {
            // Always stop on levels
            doStop = true;
        } else if (to->mNodeType == dKPNode_s::CHANGE || to->mNodeType == dKPNode_s::WORLD_CHANGE || to->mNodeType == dKPNode_s::PASS_THROUGH) {
            // If there's only one exit here, then stop even though
            // it's a passthrough node
            doStop = (to->getOpenExitNum() == 1);
        } else {
            // Quick check: do we *actually* need to stop on this node?
            // If it's a junction with more than two exits, but only two are open,
            // take the opposite open one
            if (!dScKoopatlas_c::m_instance->mWarpZoneHacks && to->getExitNum() > 2 && to->getOpenExitNum() == 2) {
                doStop = false;
            } else {
                doStop = true;
            }
        }

        // Set the world info
        if (to->mNodeType == dKPNode_s::WORLD_CHANGE) {
            dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

            PathMngReport("Activating world change %d\n", to->mWorldID);
            const dKPWorldDef_s *world = dScKoopatlas_c::m_instance->mMapData.findWorldDef(to->mWorldID);
            if (world) {
                bool visiblyChange = true;
                /*if (strncmp(save->newerWorldName, world->name, 32) == 0) {
                    PathMngReport("Already here, but setting BGM track\n");
                    visiblyChange = false;
                }*/

                PathMngReport("Found!\n");
                copyWorldDefToSave(world);

                bool wzHack = false;
                if (dScKoopatlas_c::m_instance->mWarpZoneHacks) {
                    //save->hudHintH += 1000;

                    if (world->mWorldID > 0) {
                        dLevelInfo_c *linfo = &dLevelInfo_c::m_instance;
                        dLevelInfo_c::entry_s *lastLevel;

                        if (world->mWorldID == 0) {
                            lastLevel = linfo->getEntryFromDispID(1, 27);
                        } else if (world->mWorldID != 7) {
                            // TODO: Use lastlevel from WorldInfo
                            //lastLevel = linfo->getEntryFromDispID(world->mWorldID-1, lastLevelIDs[world->mWorldID-1]);
                        } else {
                            lastLevel = linfo->getEntryFromDispID(7, 3);
                        }

                        if (lastLevel != nullptr) {
                            wzHack = !save->isCourseDataFlag(lastLevel->mWorldSlot,lastLevel->mLevelSlot, dMj2dGame_c::GOAL_NORMAL);
                        }

                        // another stupid thing
                        if ((world->mWorldID == 7) && wzHack) {
                            if (save->isCourseDataFlag(9, 24, dMj2dGame_c::GOAL_NORMAL)) {
                                wzHack = false;
                            }
                        }
                    }
                }

                if (wzHack) {
                    //save->hudHintH = 2000;
                    dKPHud_c::m_instance->hideFooter();
                } else {
                    if (visiblyChange) {
                        dKPHud_c::m_instance->showFooter();
                    }
                }

                dKPMusic_c::m_instance->start(world->mTrackID);
            } else if (to->mWorldID == 0) {
                PathMngReport("No world\n");
                //save->currentMapMusic = 0;
                dKPMusic_c::m_instance->start(0);
                //save->newerWorldName[0] = 0;
                dKPHud_c::m_instance->hideFooter();
            } else {
                PathMngReport("Not found!\n");
            }
        }

        // Go to another map
        if (to->mNodeType == dKPNode_s::CHANGE) {
            mIsPassThrough = true;
            mDisableInput = true;

            // Should we continue moving?
            if (to->getOpenExitNum() == 1) {
                PathMngReport("Stopping\n");
                mIsMoving = false;
            } else {
                PathMngReport("Continuing\n");
                startMovementTo(to->getOpenAcrossPath(mpCurrentPath));
            }

            dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
            SpammyReport("Node: %x, %s\n", to->mpDestMap, to->mpDestMap);

            int destWorld = dScKoopatlas_c::m_instance->getIndexForMapName(to->mpDestMap);
            save->setCurrentWorld(destWorld);

            SpammyReport("Changing to map ID %d (%s), entrance ID %d\n", save->getCurrentWorld(), to->mpDestMap, to->mDestID);

            dScKoopatlas_c::m_instance->mMusicPersist = true;
            dFader_c::setFader((dFader_c::fader_type_e)to->mFadeType);

            // OR param and set next scene
            u32 param = (mDispSavePrompt ? dScKoopatlas_c::REQUEST_SAVE : 0);
            param |= (mDidAutoWalkCheck ? dScKoopatlas_c::CHECK_AUTOWALK : 0);
            param |= (mAutoWalkMode ? dScKoopatlas_c::AFTER_FORTRESS : 0);

            dScene_c::setNextScene(fProfile::WORLD_MAP, dScKoopatlas_c::CHANGE_NODE | (to->mDestID << 20) | param, 0);

        // Stop at this point
        } else if (doStop) {
            bool movingAgain = false;
            player->mHasEffect = false;
            player->mHasSound = false;

            SpammyReport("Stopping at this point\n");

            mIsMoving = false;
            mIsSwimAnm = false;

            dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
            save->setCurrentPathNode(mpPathLayer->findNodeID(to));
            if (!mIsEnterNode && (dKPHud_c::m_instance != nullptr)) {
                dKPHud_c::m_instance->enteredNode();
            }

            // Should we continue here? (Requested by Jason)
            int held = dGameKey_c::m_instance->mRemocon[0]->mPrevDownButtons;

            int pressedDir = getPressedDir(held);
            if (pressedDir >= 0) {
                // Use an exit if possible
                if (canUseExit(mpCurrentNode->mpExits[pressedDir])) {
                    startMovementTo(mpCurrentNode->mpExits[pressedDir]);
                    movingAgain = true;
                } else {
                    // TODO: maybe remove this? got to see how it looks
                    // (the above is an og koopatlas comment, but do make the smoothrotation and static rotation a togglable behavior)
                    static u16 directions[] = {-0x4000,0x4000,-0x7FFF,0};
                    daKPPlayer_c::m_instance->setTargetRotY(directions[pressedDir]);
                }
            }

            if (!movingAgain) {
                player->startAnimation(dPyMdlBase_c::WAIT_SELECT, 1.2, 10.0, 0.0);
            }

        // Pass-through point
        } else {
            mIsPassThrough = true;
            startMovementTo(to->getOpenAcrossPath(mpCurrentPath));
            SpammyReport("Pass-through node, continuing to next path\n");
        }
    }

    mDoPlayPenguinSlide = true;
}

void dKpPathManager_c::copyWorldDefToSave(const dKPWorldDef_s *world) {
    OSReport("copyWorldDefToSave(): TODO!!!\n");
    /*SaveBlock *save = GetSaveFile()->GetBlock(-1);

    strncpy(save->newerWorldName, world->name, 32);
    save->newerWorldName[31] = 0;
    save->newerWorldID = world->worldID;
    save->currentMapMusic = world->trackID;

    for (int i = 0; i < 2; i++) {
        save->fsTextColours[i] = world->fsTextColours[i];
        save->fsHintColours[i] = world->fsHintColours[i];
        save->hudTextColours[i] = world->hudTextColours[i];
    }

    save->hudHintH = world->hudHintH;
    save->hudHintS = world->hudHintS;
    save->hudHintL = world->hudHintL;

    if (save->titleScreenWorld == 3 && save->titleScreenLevel == 10)
        return;
    save->titleScreenWorld = world->titleScreenWorld;
    save->titleScreenLevel = world->titleScreenLevel;*/
}

void dKpPathManager_c::activatePoint() {
    if (mCourseInDelay >= 0) {
        return;
    }

    if (mpCurrentNode->mNodeType == dKPNode_s::LEVEL) {
        int world = mpCurrentNode->mLevelNum[0] - 1;
        int level = mpCurrentNode->mLevelNum[1] - 1;

        // Shop override
        if (level == 98) {
            //dWMShop_c::instance->show(w);
            dScKoopatlas_c::m_instance->openMenu(1, -1);
            dScKoopatlas_c::m_instance->mStateMgr.changeState(dScKoopatlas_c::m_instance->StateID_ShopWait);
            return;
        }

        // One-time courses
        if ((level >= 29) && (level <= 36)) {
            dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
            u32 conds = save->getCourseDataFlag(world, level);

            SpammyReport("One-time course flags: %x", conds);
            if (conds & dMj2dGame_c::GOAL_MASK) {
#ifdef KP_PLAY_INVALID_SND
                SndAudioMgr::sInstance->startSystemSe(SE_SYS_INVALID, 1);
#endif
                return;
            }
        }

        SndAudioMgr::sInstance->startSystemSe(SE_SYS_GAME_START, 1);

        static const int sc_courceInVoice[] = {
            SE_VOC_MA_CS_COURSE_IN,
            SE_VOC_LU_CS_COURSE_IN_MULTI,
            SE_VOC_KO_CS_COURSE_IN_MULTI,
            SE_VOC_KO2_CS_COURSE_IN_MULTI,
        };

        int type = daPyMng_c::mPlayerType[0];
        nw4r::snd::SoundHandle voiceHandle;
        SndAudioMgr::sInstance->startSound(&voiceHandle, sc_courceInVoice[type], 1);

        // Pitch up the sound for mini players, since only Mario has a proper _MAME sound
        if (daPyMng_c::mPlayerMode[type] == 3) {
            voiceHandle.SetPitch(1.5f);
        }

        daKPPlayer_c::m_instance->startAnimation(dPyMdlBase_c::COURSE_IN, 1.2, 10.0, 0.0);
        daKPPlayer_c::m_instance->setTargetRotY(0);

        mIsCourseIn = true;
        mCourseInDelay = 40;
        mEnteredLevel = dLevelInfo_c::m_instance.getEntryFromSlotID(world, level);

        dKPMusic_c::m_instance->stop();
    }
}

// Types:
// 0 - Unlock ALL normal paths
// 1 - Unlock ALL secret paths
// 2 - Unlock current node's normal & secret paths
void dKpPathManager_c::unlockAllPaths(char type) {
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

    // Unlock all normal or secret paths
    if (type == 0 || type == 1) {
        for (int w = 0; w < WORLD_COUNT; w++) {
            for (int l = 0; l < STAGE_COUNT; l++) {
                if (w != 1 || l != 9) {
                    u8 goalBit = (type == 1) ? dMj2dGame_c::GOAL_NORMAL : dMj2dGame_c::GOAL_SECRET;
                    save->onCourseDataFlag(w, l, dMj2dGame_c::COIN_MASK | goalBit);
                }
            }
        }
        unlockPaths();
    }

    // Unlocks current path, regular and secret
    if (type == 2) {
        if (mpCurrentNode->mNodeType == dKPNode_s::LEVEL) {
            int w = mpCurrentNode->mLevelNum[0] - 1;
            int l = mpCurrentNode->mLevelNum[1] - 1;

            save->onCourseDataFlag(w, l, dMj2dGame_c::COIN_MASK | dMj2dGame_c::GOAL_MASK);
            unlockPaths();
        }
    }

    // Can't change node models - the price we pay for not using anims
    // for (int i = 0; i < mpPathLayer->nodeCount; i++) {
    // 	dKPNode_s *node = mpPathLayer->nodes[i];
    // 	node->setupNodeExtra();
    // }

    // TODO: Update node models, once the CLR anims are implemented
}

void dKpPathManager_c::findCameraBoundsForUnlockedPaths() {
    dKPMapData_c *data = &dScKoopatlas_c::m_instance->mMapData;

    mCamMinX = 10000;
    mCamMaxX = 0;
    mCamMinY = 10000;
    mCamMaxY = 0;

    mNodeStackLen = 0;
    for (int i = 0; i < data->mpPathLayer->mNodeNum; i++) {
        data->mpPathLayer->mpNodes[i]->mIsSetInPanBounds = false;
    }

    visitNodeForCamCheck(mpCurrentNode);
    PathMngReport("Worked out camera bounds: %d,%d to %d,%d with validity %d\n", mCamMinX, mCamMinY, mCamMaxX, mCamMaxY, mIsCamBoundsValid);
}

void dKpPathManager_c::visitNodeForCamCheck(dKPNode_s *node) {
    mNodeStackLen++;
    node->mIsSetInPanBounds = true;

    for (int i = 0; i < 4; i++) {
        dKPPath_s *path = node->mpExits[i];
        if (path == nullptr) {
            continue;
        }

        PathMngReport("Checking path %p, whose status is %d\n", path, path->mIsOpen);
        if (path->mIsOpen == dKPPath_s::NEWLY_OPEN) {
            addNodeToCameraBounds(path->mpStartPoint);
            addNodeToCameraBounds(path->mpEndPoint);
        }

        // Should we follow the other node?
        dKPNode_s *otherNode = path->getOtherNodeTo(node);

        // If this node is already in the camera bounds, skip it
        if (otherNode->mIsSetInPanBounds) {
            continue;
        }

        if (otherNode->mNodeType == otherNode->LEVEL) {
            PathMngReport("Not travelling to %p because it's level %d-%d\n", otherNode, otherNode->mLevelNum[0], otherNode->mLevelNum[1]);
            continue;
        }
        if (otherNode->mNodeType == otherNode->CHANGE) {
            PathMngReport("Not travelling to %p because it's a change\n", otherNode);
            continue;
        }
        if (otherNode->mNodeType == otherNode->WORLD_CHANGE) {
            PathMngReport("Not travelling to %p because it's a world change\n", otherNode);
            continue;
        }

        visitNodeForCamCheck(otherNode);
    }

    mNodeStackLen--;
}

void dKpPathManager_c::addNodeToCameraBounds(dKPNode_s *node) {
    mIsCamBoundsValid = true;
    PathMngReport("Adding node to camera bounds: %p at %d,%d\n", node, node->mPosX, node->mPosY);

    if (node->mPosX < mCamMinX) {
        mCamMinX = node->mPosX;
    }
    if (node->mPosX > mCamMaxX) {
        mCamMaxX = node->mPosX;
    }
    if (node->mPosY < mCamMinY) {
        mCamMinY = node->mPosY;
    }
    if (node->mPosY > mCamMaxY) {
        mCamMaxY = node->mPosY;
    }
}

void dKpPathManager_c::resetCompletionData() {
    s_cmpData.mPrevLevelID[0] = 0xFF;
    s_cmpData.mIsCmpCoins = false;
    s_cmpData.mIsCmpExits = false;
    s_cmpData.mIsCmpWorld = false;

    s_cmpData.mIsCmpCoinsExcW9 = false;
    s_cmpData.mIsCmpAllCoins = false;
    s_cmpData.mIsCmpAllExits = false;
    s_cmpData.mIsCmpAll = false;
}

void dKpPathManager_c::clearPathData() {
    if (sp_openPathData != nullptr) {
        delete[] sp_openPathData;
        sp_openPathData = nullptr;
    }

    if (sp_openNodeData != nullptr) {
        delete[] sp_openNodeData;
        sp_openNodeData = nullptr;
    }
}

#endif