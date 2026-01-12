#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_save_mng.hpp>

#include <new/bases/koopatlas/d_s_koopatlas.hpp>
#include <new/bases/koopatlas/d_kp_map_data.hpp>

dKPMapData_c::dKPMapData_c() {
    mpData = nullptr;
    mpNodeMdls = nullptr;
    mpTilesetLoaders = nullptr;

    mIsRefBound = false;
    mTilesetsLoaded = false;
}

dKPMapData_c::~dKPMapData_c() {
    if (mpNodeMdls != nullptr)
        delete[] mpNodeMdls;

    freeTilesets();
    mFileLoader.freeResouce();
}

bool dKPMapData_c::load(const char *filename) {
    mpData = (MapFile_s*)mFileLoader.request(filename, 0, mHeap::g_archiveHeap);
    if (mpData == nullptr) {
        return false;
    }

    if (!mIsRefBound) {
        bindReferences();
    }

    bool tilesetsLoaded = loadTilesets();
    bool bgResLoaded = (mBgLoader.request(mpData->mpBgName, 0, nullptr) != nullptr);

    return tilesetsLoaded && bgResLoaded;
}
static dDvd::loader_c s_tempLoaders[5];
bool dKPMapData_c::loadTilesets() {
    if (mTilesetsLoaded)
        return true;

    if (mpTilesetLoaders == nullptr) {
        OSReport("Creating %d loaders\n", mpData->mTilesetCount);
        mpTilesetLoaders = new dDvd::loader_c[mpData->mTilesetCount];
    }

    bool result = true;

    for (int i = 0; i < mpData->mTilesetCount; i++) {
        char *filename = ((char*)mpData) + (mpData->mpTilesets[i].dummy[3] - 0x10000000);
        EGG::Heap *heap = (i < 9) ? mHeap::g_gameHeaps[2] : mHeap::g_archiveHeap;
        OSReport("Tileset %d: Name %s\n", i, filename);
        result &= (s_tempLoaders[i].request(filename, 0, heap) != nullptr);
        OSReport("result is %d\n", result);
    }

    if (result) {
        mTilesetsLoaded = true;
        OSReport("did this work?\n");

        for (int i = 0; i < mpData->mTilesetCount; i++) {
            mpData->mpTilesets[i].dummy[3] = (((u32)s_tempLoaders[i].GetBuffer() & ~0xC0000000) >> 5);
        }
    }

    return mTilesetsLoaded;
}

void dKPMapData_c::freeTilesets() {
    if (mpTilesetLoaders != nullptr) {
        for (int i = 0; i < mpData->mTilesetCount; i++) {
            mpTilesetLoaders[i].freeResouce();
        }

        delete[] mpTilesetLoaders;
    }

    mBgLoader.freeResouce();
}

void dKPMapData_c::bindReferences() {
    MapReport("Binding ptr references\n");

    if (mpData->mMagic != 'KP_m') {
        OSReport("KPBIN magic doesn't match expected value!!!\n");
        return;
    }

    mIsRefBound = true;

    bindRef(mpData->mpLayers);
    bindRef(mpData->mpTilesets);
    bindRef(mpData->mpUnlockData);
    bindRef(mpData->mpSectors);
    bindRef(mpData->mpBgName);
    if (mpData->mVersion >= 2) {
        bindRef(mpData->mpWorlds);
        for (int i = 0; i < mpData->mWorldCount; i++) {
            bindRef(mpData->mpWorlds[i].name);
        }
    }

    for (int iLayer = 0; iLayer < mpData->mLayerCount; iLayer++) {
        dKPLayer_s *layer = bindRef(mpData->mpLayers[iLayer]);

        switch (layer->mLayerType) {
            case dKPLayer_s::OBJECT:
                bindRef(layer->mpTileset);
                break;

            case dKPLayer_s::DOODAD:
                for (int iDood = 0; iDood < layer->mDoodadCount; iDood++) {
                    dKPDoodad_s *doodad = bindRef(layer->mpDoodads[iDood]);

                    bindRef(doodad->mpTexObj);
                    fixTexObjSafe(doodad->mpTexObj);
                }
                break;

            case dKPLayer_s::PATH:
                mpPathLayer = layer;

                bindRef(layer->mpPaths);
                bindRef(layer->mpNodes);

                for (int iPath = 0; iPath < layer->mPathCount; iPath++) {
                    dKPPath_s *path = bindRef(layer->mpPaths[iPath]);

                    bindRef(path->mpStart);
                    bindRef(path->mpEnd);

                    bindRef(path->mpTileLayer);
                    bindRef(path->mpDoodadLayer);
                }

                for (int iNode = 0; iNode < layer->mNodeCount; iNode++) {
                    dKPNode_s *node = bindRef(layer->mpNodes[iNode]);

                    for (int i = 0; i < 4; i++)
                        bindRef(node->mpExits[i]);

                    bindRef(node->mpTileLayer);
                    bindRef(node->mpDoodadLayer);

                    if (node->mNodeType == dKPNode_s::CHANGE) {
                        bindRef(node->mpDestMap);
                        MapReport("Change Node: %x, %s\n", node->mpDestMap, node->mpDestMap);
                    }
                }
                break;
        }
    }

    // Now setup the node models
    MapReport("Setting up Nodes\n");
    int count = 0;

    // Count how many we need
    for (int nodeIdx = 0; nodeIdx < mpPathLayer->mNodeCount; nodeIdx++) {
        if (mpPathLayer->mpNodes[nodeIdx]->mNodeType == dKPNode_s::LEVEL)
            count++;
    }

    mpNodeMdls = new dKPNodeMdl_c[count];

    int modelIdx = 0;

    // Bind them to LEVEL nodes
    for (int nodeIdx = 0; nodeIdx < mpPathLayer->mNodeCount; nodeIdx++) {
        if (mpPathLayer->mpNodes[nodeIdx]->mNodeType == dKPNode_s::LEVEL) {
            mpPathLayer->mpNodes[nodeIdx]->mpNodeMdl = &mpNodeMdls[modelIdx];
            modelIdx++;
        }
    }
}

// HELPER FUNCTIONS
const dKPWorldDef_s *dKPMapData_c::getWorldDef(int id) const {
    for (int i = 0; i < mpData->mWorldCount; i++) {
        if (mpData->mpWorlds[i].key == id)
            return &mpData->mpWorlds[i];
    }
    return nullptr;
}

dKPPath_s *dKPNode_s::getOppositeExitTo(dKPPath_s *path, bool requirePathOpen) {
    for (int i = 0; i < 4; i++) {
        dKPPath_s *check = mpExits[i];

        if (check != nullptr && check != path) {
            if (requirePathOpen && !check->mPathStatus) {
                continue;
            }

            return check;
        }
    }
    return nullptr;
}

int dKPNode_s::getExitCount(bool requirePathOpen) {
    int exitCount = 0;

    for (int i = 0; i < 4; i++) {
        if ((mpExits[i] != nullptr) && (requirePathOpen ? mpExits[i]->mPathStatus : true)) {
            exitCount++;
        }
    }
    return exitCount;
}

bool dKPNode_s::checkOpenStatus() {
    for (int i = 0; i < 4; i++) {
        if ((mpExits[i] != nullptr) && (mpExits[i]->mPathStatus)) {
            return true;
        }
    }
    return false;
}

void dKPNode_s::setupCourseNode() {
    int world = mLevelNumber[0];
    int level = mLevelNumber[1];

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    int conds = save->getCourseDataFlag(world-1, level-1);

    bool isOpen = checkOpenStatus();
    bool exitComplete = false;
    bool secretComplete = false;

    if (conds & dMj2dGame_c::GOAL_NORMAL)
        exitComplete = true;
    if (conds & dMj2dGame_c::GOAL_SECRET)
        secretComplete = true;

    const char *colour;

    SpammyReport("Level %02d-%02d, isOpen: %d, exits complete: N:%d S:%d", world, level, isOpen, exitComplete, secretComplete);

    // Default: Non-unlocked levels AND completed one-time levels
    colour = "g3d/black.brres";

    // One-time levels
    if ((level >= 30) && (level <= 37)) {
        if (isOpen && !exitComplete)
            colour = "g3d/red.brres";
    }

    // Shops
    else if (level == 99) {
        colour = "g3d/shop.brres";
    }

    else if (isOpen) {
        if (mHasSecretExit) {
            if (exitComplete && secretComplete)
                colour = "g3d/blue.brres";
            else if (exitComplete || secretComplete)
                colour = "g3d/purple.brres";
            else
                colour = "g3d/red.brres";
        } else {
            if (exitComplete)
                colour = "g3d/blue.brres";
            else
                colour = "g3d/red.brres";
        }
    }

    mpNodeMdl->mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    nw4r::g3d::ResFile rf = dResMng_c::m_instance->getRes("cobCourse", colour);
    mpNodeMdl->mModel.create(rf.GetResMdl("cobCourse"), &mpNodeMdl->mAllocator, 0x224, 1, nullptr);
    PSMTXIdentity(mpNodeMdl->mMatrix);
    dGameCom::SetSoftLight_MapObj(mpNodeMdl->mModel, 0);

    mpNodeMdl->mAllocator.adjustFrmHeap();
}

void dKPNode_s::setLayerAlpha(u8 alpha) {
    if (mpTileLayer)
        mpTileLayer->mLayerAlpha = alpha;
    if (mpDoodadLayer)
        mpDoodadLayer->mLayerAlpha = alpha;
}

void dKPPath_s::setLayerAlpha(u8 alpha) {
    if (mpTileLayer)
        mpTileLayer->mLayerAlpha = alpha;
    if (mpDoodadLayer)
        mpDoodadLayer->mLayerAlpha = alpha;
}

int dKPLayer_s::findNodeID(dKPNode_s *node) {
    for (int i = 0; i < mNodeCount; i++) {
        if (mpNodes[i] == node) {
            return i;
        }
    }

    return -1;
}

#endif