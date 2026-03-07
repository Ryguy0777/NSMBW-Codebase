#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/koopatlas_config.h>

#include <game/bases/d_game_com.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_save_mng.hpp>
#include <new/bases/koopatlas/d_kp_node_mdl.hpp>
#include <new/bases/koopatlas/d_kp_map_data.hpp>

// Base class
void dKpNodeMdl_c::createMdl() { }
void dKpNodeMdl_c::draw() { }

// Course Nodes
void dKpCourseMdl_c::createMdl() {
    int world = mpParent->mLevelNum[0];
    int level = mpParent->mLevelNum[1];

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    u32 conds = save->getCourseDataFlag(world-1, level-1);

    bool isOpen = mpParent->chkOpenStatus();
    bool exitComplete = false;
    bool secretComplete = false;

    if (conds & dMj2dGame_c::GOAL_NORMAL) {
        exitComplete = true;
    }
    if (conds & dMj2dGame_c::GOAL_SECRET) {
        secretComplete = true;
    }

    const char *colour;

    //OSReport("Setting up course %02d-%02d, isOpen: %d, cleared: N:%d S:%d", world, level, isOpen, exitComplete, secretComplete);

    // Default: Locked levels AND completed one-time levels
    colour = "g3d/black.brres";

    // Open one-time levels
    if ((level >= 30) && (level <= 37)) {
        if (isOpen && !exitComplete) {
            colour = "g3d/red.brres";
        }
    }

    // Shop houses
    else if (level == 99) {
        colour = "g3d/shop.brres";
    }

    else if (isOpen) {
        if (mpParent->mHasSecretExit) {
            if (exitComplete && secretComplete) {
                colour = "g3d/blue.brres";
            } else if (exitComplete || secretComplete) {
                colour = "g3d/purple.brres";
            } else {
                colour = "g3d/red.brres";
            }
        } else {
            if (exitComplete) {
                colour = "g3d/blue.brres";
            } else {
                colour = "g3d/red.brres";
            }
        }
    }

    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], 0, 0x20);

    nw4r::g3d::ResFile res(dResMng_c::m_instance->getRes("cobCourse", colour));
    mModel.create(res.GetResMdl("cobCourse"), &mAllocator, 0x224, 1, 0);
    PSMTXIdentity(mMatrix);
    dGameCom::SetSoftLight_MapObj(mModel, 0);

    mAllocator.adjustFrmHeap();
}

void dKpCourseMdl_c::draw() {
    // Calc model matrix
    mAng3_c angle(0x4000, 0x8000, 0x6000);
    PSMTXTrans(mMatrix, mpParent->mPosX, -mpParent->mPosY + 4.0, 498.0);
    mMatrix.ZXYrotM(angle.y, angle.x, angle.z);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(0.8f, 0.8f, 0.8f);
    mModel.calc(false);

    // Draw it
    mModel.entry();
}

#endif