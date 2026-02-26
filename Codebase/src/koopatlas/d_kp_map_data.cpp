#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_kp_map_data.hpp>

#include <game/bases/d_game_com.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/mLib/m_heap.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>

dKPMapData_c::dKPMapData_c() {
    mpData = nullptr;
    mpCourseNodes = nullptr;
    mpTilesetLoaders = nullptr;

    mIsDataInited = false;
    mIsTilesetLoaded = false;
}

dKPMapData_c::~dKPMapData_c() {
    if (mpCourseNodes != nullptr) {
        delete[] mpCourseNodes;
    }

    freeTilesets();
    mBgLoader.freeResouce();
    mMapLoader.freeResouce();
}

bool dKPMapData_c::create(const char *filename) {
    mpData = (dKPMapFile_s*)mMapLoader.request(filename, 0, mHeap::g_archiveHeap);
    if (mpData == nullptr) {
        return false;
    }

    if (!mIsDataInited) {
        initMapData();
    }

    bool tsSuccess = loadTilesets();
    bool bgSuccess = (mBgLoader.request(mpData->mpBgName, 0, nullptr) != nullptr);

    return tsSuccess && bgSuccess;
}

const dKPWorldDef_s *dKPMapData_c::findWorldDef(int id) const {
    for (int i = 0; i < mpData->mWorldDefNum; i++) {
        if (mpData->mpWorldDefs[i].mKeyID == id) {
            return &mpData->mpWorldDefs[i];
        }
    }
    return nullptr;
}

void dKPMapData_c::initMapData() {
    OSReport("Initing MapData\n");

    mIsDataInited = true;

    // Init main file
    fixRef(mpData->mpLayers);
    fixRef(mpData->mpTilesets);
    fixRef(mpData->mpUnlockData);
    fixRef(mpData->mpSectors);
    fixRef(mpData->mpBgName);

    // Init world definitions if the KPBIN supports them
    if (mpData->mVersion >= 2) {
        fixRef(mpData->mpWorldDefs);
        for (int i = 0; i < mpData->mWorldDefNum; i++) {
            fixRef(mpData->mpWorldDefs[i].mpWorldName);
        }
    }

    // Iterate through all layers
    for (int iLayer = 0; iLayer < mpData->mLayerNum; iLayer++) {
        dKPLayer_s *layer = fixRef(mpData->mpLayers[iLayer]);

        switch (layer->mLayerType) {
            case dKPLayer_s::TYPE_OBJECT:
                fixRef(layer->mpTileset);
                break;

            case dKPLayer_s::TYPE_DOODAD:
                for (int iDoodad = 0; iDoodad < layer->mDoodadNum; iDoodad++) {
                    dKPDoodad_s *doodad = fixRef(layer->mpDoodads[iDoodad]);

                    fixRef(doodad->mpTexObj);
                    fixTexObjSafe(doodad->mpTexObj);
                }
                break;

            case dKPLayer_s::TYPE_PATH:
                mpPathLayer = layer;

                fixRef(layer->mpPaths);
                fixRef(layer->mpNodes);

                // Init all paths
                for (int iPath = 0; iPath < layer->mPathNum; iPath++) {
                    dKPPath_s *path = fixRef(layer->mpPaths[iPath]);

                    fixRef(path->mpStartPoint);
                    fixRef(path->mpEndPoint);

                    fixRef(path->mpTileLayer);
                    fixRef(path->mpDoodadLayer);
                }

                // Init nodes
                for (int iNode = 0; iNode < layer->mNodeNum; iNode++) {
                    dKPNode_s *node = fixRef(layer->mpNodes[iNode]);

                    for (int i = 0; i < 4; i++) {
                        fixRef(node->mpExits[i]);
                    }

                    fixRef(node->mpTileLayer);
                    fixRef(node->mpDoodadLayer);

                    if (node->mNodeType == dKPNode_s::CHANGE) {
                        fixRef(node->mpDestMap);
                        OSReport("Map Change: %x, %s\n", node->mpDestMap, node->mpDestMap);
                    }
                }
                break;

            default:
                OSReport("WARNING: Detected unknown layer [%d] with type %02d!\n", iLayer, layer->mLayerType);
                break;
        }
    }

    // Next up, make all of the course node models

    // Count how many we need...
    int count = 0;
    for (int nodeIdx = 0; nodeIdx < mpPathLayer->mNodeNum; nodeIdx++) {
        if (mpPathLayer->mpNodes[nodeIdx]->mNodeType == dKPNode_s::LEVEL) {
            count++;
        }
    }

    // Make an array here...
    mpCourseNodes = new dKPCourseNode_c[count];

    // Now assign them to nodes
    int courseIdx = 0;
    for (int nodeIdx = 0; nodeIdx < mpPathLayer->mNodeNum; nodeIdx++) {
        if (mpPathLayer->mpNodes[nodeIdx]->mNodeType == dKPNode_s::LEVEL) {
            mpPathLayer->mpNodes[nodeIdx]->mpCourseNode = &mpCourseNodes[courseIdx];
            courseIdx++;
        }
    }
}

bool dKPMapData_c::loadTilesets() {
    if (mIsTilesetLoaded) {
        return true;
    }

    if (mpTilesetLoaders == nullptr) {
        mpTilesetLoaders = new dDvd::loader_c[mpData->mTilesetNum];
    }

    bool result = true;

    for (int i = 0; i < mpData->mTilesetNum; i++) {
        char *filename = ((char*)mpData) + (mpData->mpTilesets[i].dummy[3] - 0x10000000);
        EGG::Heap *heap = (i < 9) ? mHeap::g_gameHeaps[2] : mHeap::g_archiveHeap;

        result &= (mpTilesetLoaders[i].request(filename, 0, heap) != nullptr);
        OSReport("fn: %s\n", filename);
    }

    if (result) {
        mIsTilesetLoaded = true;

        for (int i = 0; i < mpData->mTilesetNum; i++) {
            mpData->mpTilesets[i].dummy[3] = (((u32)mpTilesetLoaders[i].GetBuffer() & ~0xC0000000) >> 5);
        }
    }

    return mIsTilesetLoaded;
}

void dKPMapData_c::freeTilesets() {
    if (mpTilesetLoaders != nullptr) {
        bool result = true;

        for (int i = 0; i < mpData->mTilesetNum; i++) {
            result &= mpTilesetLoaders[i].freeResouce();
            if (result) {
                OSReport("tileset [%d] was freed!\n", i);
            }
        }

        if (!result) {
            OSReport("failed to free!\n");
            return;
        }

        delete[] mpTilesetLoaders;
        mpTilesetLoaders = nullptr;
    }
}


/******************************************************************************
 * HELPER FUNCTIONS
 ******************************************************************************/
bool dKPNode_s::chkOpenStatus() {
    for (int i = 0; i < 4; i++) {
        if (mpExits[i]) {
            if (mpExits[i]->mIsOpen) {
                return true;
            }
        }
    }
    return false;
}

void dKPNode_s::createCourseNode() {
    int world = mLevelNum[0];
    int level = mLevelNum[1];

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    u32 conds = save->getCourseDataFlag(world-1, level-1);

    bool isOpen = this->chkOpenStatus();
    bool exitComplete = false;
    bool secretComplete = false;

    if (conds & dMj2dGame_c::GOAL_NORMAL) {
        exitComplete = true;
    }
    if (conds & dMj2dGame_c::GOAL_SECRET) {
        secretComplete = true;
    }

    const char *colour;

    //OSReport("Level %d-%d, isOpen: %d, exitComplete: %d", world, level, isUnlocked, exitComplete);

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
        if (mHasSecretExit) {
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

    mpCourseNode->mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], 0, 0x20);

    nw4r::g3d::ResFile res(dResMng_c::m_instance->getRes("cobCourse", colour));
    mpCourseNode->mModel.create(res.GetResMdl("cobCourse"), &mpCourseNode->mAllocator, 0x224, 1, 0);
    PSMTXIdentity(mpCourseNode->mMatrix);
    dGameCom::SetSoftLight_MapObj(mpCourseNode->mModel, 0);

    mpCourseNode->mAllocator.adjustFrmHeap();
}

dKPPath_s *dKPNode_s::getAcrossPath(dKPPath_s *path, bool requireOpenState) {
    for (int i = 0; i < 4; i++) {
        dKPPath_s *check = mpExits[i];

        if ((check != nullptr) && (check != path)) {
            if (requireOpenState && !check->mIsOpen) {
                continue;
            }
            return check;
        }
    }
    return nullptr;
}

int dKPNode_s::getExitNum(bool requireOpenState) {
    int count = 0;

    for (int i = 0; i < 4; i++) {
        if (mpExits[i] && (requireOpenState ? mpExits[i]->mIsOpen : true)) {
            count++;
        }
    }
    return count;
}

void dKPNode_s::setLayerAlpha(u8 alpha) {
    if (mpTileLayer) {
        mpTileLayer->mAlpha = alpha;
    }
    if (mpDoodadLayer) {
        mpDoodadLayer->mAlpha = alpha;
    }
}

void dKPPath_s::setLayerAlpha(u8 alpha) {
    if (mpTileLayer) {
        mpTileLayer->mAlpha = alpha;
    }
    if (mpDoodadLayer) {
        mpDoodadLayer->mAlpha = alpha;
    }
}

int dKPLayer_s::findNodeID(dKPNode_s *node) {
    for (int i = 0; i < mNodeNum; i++) {
        if (mpNodes[i] == node) {
            return i;
        }
    }
    return -1;
}
#endif