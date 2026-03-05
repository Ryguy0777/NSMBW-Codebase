#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/koopatlas_config.h>

#include <game/bases/d_dvd.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_mtx.hpp>

#include <new/bases/koopatlas/d_kp_node_mdl.hpp>

// Forward declarations
struct dKpLayer_s;
struct dKpNode_s;

/******************************************************************************
 * Doodads
 ******************************************************************************/
struct dKpDoodad_s {
    struct Anim_s {
        enum LoopType_e {
            CONTIGUOUS = 0,
            LOOP,
            REVERSE_LOOP
        };

        enum CurveType_e {
            LINEAR = 0,
            SIN,
            COS
        };

        enum AnimType_e {
            X_POS = 0,
            Y_POS,
            ANGLE,
            X_SCALE,
            Y_SCALE,
            OPACITY
        };

        LoopType_e mLoopType;
        CurveType_e mCurveType;
        int mFrameCount;
        AnimType_e mAnimType;
        int mStartVal, mEndVal;
        int mDelay, mInitialDelay;

        u32 mBaseTick;
        bool mIsReverse;
    };

    mVec2_c mPos;
    float mWidth, mHeight;
    float mAngle;

    GXTexObj *mpTexObj;
    int mAnimNum;
    Anim_s mAnims[1]; // variable size
};

/******************************************************************************
 * Paths and Nodes
 ******************************************************************************/
struct dKpPath_s;

struct dKpNode_s {
    enum NodeType_e {
        PASS_THROUGH = 0,
        STOP,
        LEVEL,
        CHANGE,
        WORLD_CHANGE
    };

    short mPosX, mPosY;
    union {
        dKpPath_s *mpExits[4];
        struct {
            dKpPath_s *mpExitL;
            dKpPath_s *mpExitR;
            dKpPath_s *mpExitU;
            dKpPath_s *mpExitD;
        };
    };
    dKpLayer_s *mpTileLayer;
    dKpLayer_s *mpDoodadLayer;

    u8 mIsSetInPanBounds; // Used when setting up panning bounds for path unlocks
    u8 mReserved2;
    u8 mReserved3;
    u8 mNodeType;

    bool mIsNewlyOpen;

    dKpNodeMdl_c *mpNodeMdl;

    // The union is placed at the very end so
    // we can leave out padding in the kpbin
    union {
        struct { u8 mLevelNum[2]; bool mHasSecretExit; };
        struct { const char *mpDestMap; u8 mCurrID, mDestID, mFadeType, _; };
        struct { u8 mWorldID, __[3]; };
    };

    dKpPath_s *getAnyExit() {
        for (int i = 0; i < 4; i++) {
            if (mpExits[i]) {
                return mpExits[i];
            }
        }

        return nullptr;
    }

    bool chkOpenStatus();

    dKpPath_s *getAcrossPath(dKpPath_s *path, bool requireOpenState=false);
    dKpPath_s *getOpenAcrossPath(dKpPath_s *path) {
        return getAcrossPath(path, true);
    }

    int getExitNum(bool requireOpenState=false);
    int getOpenExitNum() {
        return getExitNum(true);
    }

    void setLayerAlpha(u8 alpha);
};

struct dKpPath_s {
    enum OpenStatus_e {
        NOT_OPEN = 0,
        OPEN,
        NEWLY_OPEN,
        ALWAYS_OPEN
    };

    enum ActionType_e {
        WALK = 0,
        WALK_SAND,
        WALK_SNOW,
        WALK_WATER,
        JUMP,
        JUMP_SAND,
        JUMP_SNOW,
        JUMP_WATER,
        LADDER,
        LADDER_LEFT,
        LADDER_RIGHT,
        FALL,
        SWIM,
        RUN,
        PIPE,
        DOOR,
        TJUMPED,
        ENTER_CAVE_UP,
        RESERVED_18,
        INVISIBLE,

        ACTION_NUM
    };

    dKpNode_s *mpStartPoint, *mpEndPoint;
    dKpLayer_s *mpTileLayer, *mpDoodadLayer;

    dKpNode_s *getOtherNodeTo(dKpNode_s *n) {
        return (n == mpStartPoint) ? mpEndPoint : mpStartPoint;
    }

    u8 mIsOpen; // computed on-the-fly - default from Koopatlas is NOT or ALWAYS
    u8 mIsSecret;
    u8 mPad[2];
    float mPathSpeed;
    ActionType_e mAction;

    void setLayerAlpha(u8 alpha);
};

/******************************************************************************
 * Layers
 ******************************************************************************/
struct dKpLayer_s {
    enum LayerType_e {
        TYPE_OBJECT = 0,
        TYPE_DOODAD,
        TYPE_PATH
    };

    LayerType_e mLayerType;
    u8 mAlpha;
    u8 mPad[3];

    typedef u16 sector_s[16][16];

    union {
        struct {
            GXTexObj *mpTileset;
            union {
                int mSectorBounds[4];
                struct {
                    int mSectorL;
                    int mSectorU;
                    int mSectorR;
                    int mSectorD;
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
            u16 mIndices[1]; // variable size
        };

        struct {
            int mDoodadNum;
            dKpDoodad_s *mpDoodads[1]; // variable size
        };

        struct {
            int mNodeNum;
            dKpNode_s **mpNodes;
            int mPathNum;
            dKpPath_s **mpPaths;
        };
    };

    int findNodeID(dKpNode_s *node);
};

/******************************************************************************
 * World Definitions
 ******************************************************************************/
struct dKpWorldDef_s {
    const char *mpWorldName; // TODO: Repurpose for lighting data
    GXColor fsTextColours[2];
    GXColor fsHintColours[2];
    GXColor hudTextColours[2];
    u16 mHudHintH;
    s8 mHudHintS;
    s8 mHudHintL;
    u8 mKeyID; // Internal index of the definition
    u8 mTrackID;
    u8 mWorldID;
    u8 mTitleWorldNo;
    u8 mTitleLevelNo;
    u8 mPad[3];
};

/******************************************************************************
 * Map File
 ******************************************************************************/
struct dKpMapFile_s {
    u32 mMagic; // "KP_m"
    int mVersion;

    int mLayerNum;
    dKpLayer_s **mpLayers;

    int mTilesetNum;
    GXTexObj *mpTilesets;

    u8 *mpUnlockData;

    dKpLayer_s::sector_s *mpSectors;

    const char *mpBgName;

    dKpWorldDef_s *mpWorldDefs;
    int mWorldDefNum;
};

class dKpMapData_c {
public:
    dKpMapData_c();
    ~dKpMapData_c();

    bool create(const char *filename);

    const dKpWorldDef_s *findWorldDef(int id) const;

private:
    template <typename T>
        inline T* fixRef(T*& pIndex) {
            unsigned int index = (unsigned int)pIndex;
            if (index == 0xFFFFFFFF || index == 0) {
                pIndex = nullptr;
            } else {
                pIndex = (T*)(((char*)mpData) + index);
            }
            return pIndex;
        }

    template <typename T>
        inline T* fixRefSafe(T*& pIndex) {
            unsigned int index = (unsigned int)pIndex;
            if (index == 0xFFFFFFFF || index == 0) {
                pIndex = nullptr;
            } else if (index < 0x80000000) {
                pIndex = (T*)(((char*)mpData) + index);
            }
            return pIndex;
        }

    inline void fixTexObjSafe(GXTexObj *obj) {
        if (obj->dummy[3] >= 0x10000000) {
            obj->dummy[3] = (((u32)mpData) & 0x7FFFFFFF) + (obj->dummy[3] - 0x10000000);
            obj->dummy[3] >>= 5;
        }
    }

    void initMapData();

    bool loadTilesets();
    void freeTilesets();

private:
    dDvd::loader_c mMapLoader;

    bool mIsDataInited;
    bool mIsTilesetLoaded;

public:
    void **mpTilesetBuffers;
    dKpMapFile_s *mpData;
    dKpLayer_s *mpPathLayer;

    dDvd::loader_c mBgLoader;
};
#endif