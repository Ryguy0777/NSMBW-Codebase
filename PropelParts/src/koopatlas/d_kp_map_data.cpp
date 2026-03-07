#include <kamek.h>
#include <propelparts/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <propelparts/bases/koopatlas/d_kp_map_data.hpp>

#include <game/mLib/m_heap.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <lib/revolution/CX/CXUncompression.h>

dKpMapData_c::dKpMapData_c() {
    mpData = nullptr;
    mpTilesetBuffers = nullptr;

    mIsDataInited = false;
    mIsTilesetLoaded = false;
}

dKpMapData_c::~dKpMapData_c() {
    freeTilesets();
    mBgLoader.freeResouce();
    mMapLoader.freeResouce();
}

bool dKpMapData_c::create(const char *filename) {
    mpData = (dKpMapFile_s*)mMapLoader.request(filename, 0, mHeap::g_archiveHeap);
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

const dKpWorldDef_s *dKpMapData_c::findWorldDef(int id) const {
    for (int i = 0; i < mpData->mWorldDefNum; i++) {
        if (mpData->mpWorldDefs[i].mKeyID == id) {
            return &mpData->mpWorldDefs[i];
        }
    }
    return nullptr;
}

void dKpMapData_c::initMapData() {
    OSReport("Initing MapData\n");

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
        dKpLayer_s *layer = fixRef(mpData->mpLayers[iLayer]);

        switch (layer->mLayerType) {
            case dKpLayer_s::TYPE_OBJECT:
                fixRef(layer->mpTileset);
                break;

            case dKpLayer_s::TYPE_DOODAD:
                for (int iDoodad = 0; iDoodad < layer->mDoodadNum; iDoodad++) {
                    dKpDoodad_s *doodad = fixRef(layer->mpDoodads[iDoodad]);

                    fixRef(doodad->mpTexObj);
                    fixTexObjSafe(doodad->mpTexObj);
                }
                break;

            case dKpLayer_s::TYPE_PATH:
                mpPathLayer = layer;

                fixRef(layer->mpPaths);
                fixRef(layer->mpNodes);

                // Init all paths
                for (int iPath = 0; iPath < layer->mPathNum; iPath++) {
                    dKpPath_s *path = fixRef(layer->mpPaths[iPath]);

                    fixRef(path->mpStartPoint);
                    fixRef(path->mpEndPoint);

                    fixRef(path->mpTileLayer);
                    fixRef(path->mpDoodadLayer);
                }

                // Init nodes
                for (int iNode = 0; iNode < layer->mNodeNum; iNode++) {
                    dKpNode_s *node = fixRef(layer->mpNodes[iNode]);

                    for (int i = 0; i < 4; i++) {
                        fixRef(node->mpExits[i]);
                    }

                    fixRef(node->mpTileLayer);
                    fixRef(node->mpDoodadLayer);

                    if (node->mNodeType == dKpNode_s::CHANGE) {
                        fixRef(node->mpDestMap);
                        OSReport("Initing Map Change: %x, %s\n", node->mpDestMap, node->mpDestMap);
                    }
                }
                break;

            default:
                OSReport("WARNING: Detected unknown layer [%d] with type %02d!\n", iLayer, layer->mLayerType);
                break;
        }
    }

    mIsDataInited = true;

    // Next up, make all of the course node models

    // Count how many we need...
    int levelNum = 0;
    for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
        if (mpPathLayer->mpNodes[i]->mNodeType == dKpNode_s::LEVEL) {
            levelNum++;
        }
    }

    for (int i = 0; i < mpPathLayer->mNodeNum; i++) {
        dKpNode_s *node = mpPathLayer->mpNodes[i];

        if (node->mNodeType == dKpNode_s::LEVEL) {
            node->mpNodeMdl = new dKpCourseMdl_c;
            node->mpNodeMdl->mpParent = node;
        }
    }
}

bool dKpMapData_c::loadTilesets() {
    if (mIsTilesetLoaded) {
        return true;
    }

    if (mpTilesetBuffers == nullptr) {
        mpTilesetBuffers = new void*[mpData->mTilesetNum];
    }

    bool result = false;

    for (int i = 0; i < mpData->mTilesetNum; i++) {
        char *filename = ((char*)mpData) + (mpData->mpTilesets[i].dummy[3] - 0x10000000);
        EGG::Heap *heap = (i < 9) ? mHeap::g_gameHeaps[2] : mHeap::g_archiveHeap;

        bool isCompressed = false;

        int entrynum = DVDConvertPathToEntrynum(filename);
        if (entrynum == -1) {
            // Check for an LZ-compressed file
            strcat(filename, ".LZ");
            entrynum = DVDConvertPathToEntrynum(filename);
            if (entrynum == -1) continue;

            isCompressed = true;
        }

        // Load the file
        DVDFileInfo dvdHandle;
        bool fileLoaded = DVDFastOpen(entrynum, &dvdHandle);
        if (!fileLoaded) continue;

        // Allocate memory
        mpTilesetBuffers[i] = EGG::Heap::alloc(dvdHandle.size, 0x20, heap);
        if (mpTilesetBuffers[i] == nullptr) continue;

        // Read file
        s32 length = DVDReadPrio(&dvdHandle, mpTilesetBuffers[i], dvdHandle.size, 0, 2);
        if (length > 0) {

            // Gotta decompress this first
            if (isCompressed) {
                int size = CXGetUncompressedSize(mpTilesetBuffers[i]);
                void *bufLZ = mpTilesetBuffers[i];

                mpTilesetBuffers[i] = EGG::Heap::alloc(size, 0x20, heap);
                CXUncompressLZ(bufLZ, mpTilesetBuffers[i]);

                if (bufLZ != nullptr) {
                    EGG::Heap::free(bufLZ, heap);
                    bufLZ = nullptr;
                }
            }

            mpData->mpTilesets[i].dummy[3] = (((u32)mpTilesetBuffers[i] & ~0xC0000000) >> 5);
            result = true;
        }

        // Unload file
        DVDClose(&dvdHandle);
    }

    if (result) {
        mIsTilesetLoaded = true;
    }
    return mIsTilesetLoaded;
}

void dKpMapData_c::freeTilesets() {
    for (int i = 0; i < mpData->mTilesetNum; i++) {
        EGG::Heap *heap = (i < 9) ? mHeap::g_gameHeaps[2] : mHeap::g_archiveHeap;

        if (mpTilesetBuffers[i] != nullptr) {
            EGG::Heap::free(mpTilesetBuffers[i], heap);
            mpTilesetBuffers[i] = nullptr;
        }
    }
}


/******************************************************************************
 * HELPER FUNCTIONS
 ******************************************************************************/
bool dKpNode_s::chkOpenStatus() {
    for (int i = 0; i < 4; i++) {
        if (mpExits[i]) {
            if (mpExits[i]->mIsOpen) {
                return true;
            }
        }
    }
    return false;
}

dKpPath_s *dKpNode_s::getAcrossPath(dKpPath_s *path, bool requireOpenState) {
    for (int i = 0; i < 4; i++) {
        dKpPath_s *check = mpExits[i];

        if ((check != nullptr) && (check != path)) {
            if (requireOpenState && !check->mIsOpen) {
                continue;
            }
            return check;
        }
    }
    return nullptr;
}

int dKpNode_s::getExitNum(bool requireOpenState) {
    int count = 0;

    for (int i = 0; i < 4; i++) {
        if (mpExits[i] && (requireOpenState ? mpExits[i]->mIsOpen : true)) {
            count++;
        }
    }
    return count;
}

void dKpNode_s::setLayerAlpha(u8 alpha) {
    if (mpTileLayer) {
        mpTileLayer->mAlpha = alpha;
    }
    if (mpDoodadLayer) {
        mpDoodadLayer->mAlpha = alpha;
    }
}

void dKpPath_s::setLayerAlpha(u8 alpha) {
    if (mpTileLayer) {
        mpTileLayer->mAlpha = alpha;
    }
    if (mpDoodadLayer) {
        mpDoodadLayer->mAlpha = alpha;
    }
}

int dKpLayer_s::findNodeID(dKpNode_s *node) {
    for (int i = 0; i < mNodeNum; i++) {
        if (mpNodes[i] == node) {
            return i;
        }
    }
    return -1;
}
#endif