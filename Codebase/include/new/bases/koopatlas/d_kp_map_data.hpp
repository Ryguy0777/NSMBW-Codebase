#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/koopatlas_config.h>

#include <game/bases/d_dvd.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <nw4r/math/math_types.h>
#include <nw4r/lyt/lyt_common.h>

// Forward declarations
struct dKPLayer_s;
struct dKPPath_s;

// Represents a World Editor entry
// TODO: Figure out what to do with this
struct dKPWorldDef_s {
    const char *name;
    GXColor fsTextColours[2];
    GXColor fsHintColours[2];
    GXColor hudTextColours[2];
    u16 hudHintH;
    s8 hudHintS, hudHintL;
    u8 key, trackID;
    u8 worldID;
    u8 titleScreenWorld;
    u8 titleScreenLevel;
    u8 padding[3];
};

// Draws models for nodes
// Currently only available for LEVEL nodes
class dKPNodeMdl_c {
public:
    dHeapAllocator_c mAllocator;
    mMtx_c mMatrix;
    m3d::mdl_c mModel;
};

/******************************************************************************
 * Doodads
 ******************************************************************************/
struct dKPDoodad_s {
    struct Animation_s {
        enum LOOP_TYPE_e {
            CONTIGUOUS,
            LOOP,
            REVERSE_LOOP
        };

        enum CURVE_TYPE_e {
            LINEAR,
            SIN,
            COS
        };

        enum ANIM_TYPE_e {
            X_POS,
            Y_POS,
            ANGLE,
            X_SCALE,
            Y_SCALE,
            OPACITY
        };

        LOOP_TYPE_e mLoopType;
        CURVE_TYPE_e mCurveType;
        int mFrameCount;
        ANIM_TYPE_e mAnimType;
        int mStart;
        int mEnd;
        int mDelay;
        int mInitialDelay;

        u32 mBaseTick;
        bool mIsReverse;
    };

    nw4r::math::VEC2 mPos;
    float mWidth, mHeight;
    float mAngle;

    GXTexObj *mpTexObj;
    int mAnimCount;
    Animation_s mAnims[1]; // Variable size
};

/******************************************************************************
 * Paths
 ******************************************************************************/
struct dKPNode_s {
    enum NODE_TYPE_e {
        PASS_THROUGH,
        STOP,
        LEVEL,
        CHANGE,
        WORLD_CHANGE
    };

    dKPPath_s *getAnyExit() {
        for (int i = 0; i < 4; i++) {
            if (mpExits[i]) {
                return mpExits[i];
            }
        }
        return nullptr;
    }

    bool checkOpenStatus();
    void setupCourseNode();

    dKPPath_s *getOppositeExitTo(dKPPath_s *path, bool requirePathOpen=false);
    // TODO: Remove this, just use the above
    dKPPath_s *getOppositeAvailableExitTo(dKPPath_s *path) {
        return getOppositeExitTo(path, true);
    }

    int getExitCount(bool requirePathOpen=false);
    // TODO: Remove this, just use the above
    int getAvailableExitCount() {
        return getExitCount(true);
    }

    void setLayerAlpha(u8 alpha);

    short mPosX, mPosY;
    union {
        dKPPath_s *mpExits[4];
        struct {
            dKPPath_s *mpLeftExit;
            dKPPath_s *mpRightExit;
            dKPPath_s *mpUpExit;
            dKPPath_s *mpDownExit;
        };
    };
    dKPLayer_s *mpTileLayer, *mpDoodadLayer;

    u8 mReserved1;
    u8 mReserved2;
    u8 mReserved3;
    NODE_TYPE_e mNodeType;

    bool mIsNewUnlock;

    dKPNodeMdl_c *mpNodeMdl;

    // The union is placed at the very end so we can leave out padding in the kpbin
    union {
        struct { u8 mLevelNumber[2]; bool mHasSecretExit; };
        struct { const char *mpDestMap; u8 mID, mDestID, mTransition, _; };
        struct { u8 mWorldID, __[3]; };
    };
};

struct dKPPath_s {
    enum PATH_STATUS_e {
        LOCKED = 0,
        OPEN = 1,
        NEW_OPEN = 2,
        ALWAYS_OPEN = 3
    };

    enum ANIM_TYPE_e {
        WALK = 0, WALK_SAND = 1, WALK_SNOW = 2, WALK_WATER = 3,
        JUMP = 4, JUMP_SAND = 5, JUMP_SNOW = 6, JUMP_WATER = 7,
        LADDER = 8, LADDER_LEFT = 9, LADDER_RIGHT = 10, FALL = 11,
        SWIM = 12, RUN = 13, PIPE = 14, DOOR = 15,
        TJUMPED = 16, ENTER_CAVE_UP = 17, RESERVED_18 = 18, INVISIBLE = 19,
        MAX_ANIM = 20
    };

    dKPNode_s *getOtherNodeTo(dKPNode_s *n) {
        return (n == mpStart) ? mpEnd : mpStart;
    }

    void setLayerAlpha(u8 alpha);

    dKPNode_s *mpStart, *mpEnd;
    dKPLayer_s *mpTileLayer, *mpDoodadLayer;

    u8 mPathStatus; // Computed on-the-fly - default from Koopatlas is LOCKED or ALWAYS_OPEN
    u8 mIsSecretPath;
    u8 mPad[2];

    float mPathSpeed;
    ANIM_TYPE_e mAnimType;
};

/******************************************************************************
 * Tying it all together
 ******************************************************************************/
struct dKPLayer_s {
    enum LAYER_TYPE_e {
        OBJECT,
        DOODAD,
        PATH
    };

    LAYER_TYPE_e mLayerType;
    u8 mLayerAlpha;
    u8 mPad[3];

    typedef u16 sector_s[16][16];

    union {
        struct {
            GXTexObj *mpTileset;
            union {
                int mSectorBounds[4];
                struct {
                    int mSectorLeft;
                    int mSectorTop;
                    int mSectorRight;
                    int mSectorBottom;
                };
            };
            union {
                int mBounds[4];
                struct {
                    int mLeft;
                    int mTop;
                    int mRight;
                    int mBottom;
                };
            };
            u16 mIndices[1]; // Variable size
        };

        struct {
            int mDoodadCount;
            dKPDoodad_s *mpDoodads[1]; // Variable size
        };

        struct {
            int mNodeCount;
            dKPNode_s **mpNodes;
            int mPathCount;
            dKPPath_s **mpPaths;
        };
    };

    int findNodeID(dKPNode_s *node);
};

class dKPMapData_c {
public:
    // Represents a KPBIN
    struct MapFile_s {
        u32 mMagic; // "KP_m"
        int mVersion;

        int mLayerCount;
        dKPLayer_s **mpLayers;

        int mTilesetCount;
        GXTexObj *mpTilesets;

        u8 *mpUnlockData;

        dKPLayer_s::sector_s *mpSectors;

        const char *mpBgName;

        dKPWorldDef_s *mpWorlds;
        int mWorldCount;
    };

    dKPMapData_c();
    ~dKPMapData_c();

    bool load(const char *filename);
    const dKPWorldDef_s *getWorldDef(int id) const;

private:
    void bindReferences();

    bool loadTilesets();
    void freeTilesets();

    template <typename T>
        inline T* bindRef(T*& pIndex) {
            u32 index = (u32)pIndex;
            OSReport("Index: %08x\n", index);
            if (index == 0xFFFFFFFF || index == 0)
                pIndex = nullptr;
            else
                //pIndex = (T*)((u32)(&mpData[index]));
                pIndex = (T*)(((char*)mpData) + index);
            OSReport("PIndex: %08X\n", pIndex);
            return pIndex;
        }

    template <typename T>
        inline T* bindRefSafe(T*& pIndex) {
            u32 index = (u32)pIndex;
            if (index == 0xFFFFFFFF || index == 0)
                pIndex = nullptr;
            else if (index < 0x80000000)
                pIndex = (T*)(((char*)mpData) + index);
            return pIndex;
        }

    inline void fixTexObjSafe(GXTexObj *obj) {
        if (obj->dummy[3] >= 0x10000000) {
            obj->dummy[3] = (((u32)mpData) & 0x7FFFFFFF) + (obj->dummy[3] - 0x10000000);
            obj->dummy[3] >>= 5;
        }
    }

    dDvd::loader_c mFileLoader;

    bool mIsRefBound;
    bool mTilesetsLoaded;

public:
    dDvd::loader_c *mpTilesetLoaders;
    MapFile_s *mpData;
    dKPLayer_s *mpPathLayer;

    dDvd::loader_c mBgLoader;

    dKPNodeMdl_c *mpNodeMdls;
};
#endif