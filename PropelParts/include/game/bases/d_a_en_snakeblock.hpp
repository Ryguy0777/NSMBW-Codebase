#pragma once
#include <game/bases/d_bg_ctr.hpp>
#include <game/bases/d_circle_light_mask.hpp>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_mat_clr.hpp>
#include <game/mLib/m_3d/mdl.hpp>

class daEnSnakeBlock_c : public dEn_c {
public:
    enum SNAKE_TYPE_e {
        TYPE_NORMAL,
        TYPE_ICE,
        TYPE_GLOW,
    };

    // Extras -- TODO
    /*class dBlockEx_c {
    public:
        dBlockEx_c();

        mHeapAllocator_c mLightMaskAlloc;
        dCircleLightMask_c mLightMask;
        int mSnakeType;
    };*/

    class dBlock_c {
    public:
        dBlock_c();
        virtual ~dBlock_c();

        void createMdl(dHeapAllocator_c *alloc);
        void doDelete();

        void calcAnm();
        void setAnmClr(const char *name);

        void draw(Vec pos);

        void initBgCtr(daEnSnakeBlock_c *owner, Vec *blockPos);
        void calcBgCtr();

        void setFallCollapse();
        void calcCollapse1(u8 *travelInfo);

        nw4r::g3d::ResFile mResFile;
        m3d::mdl_c mModel;
        m3d::anmMatClr_c mAnmClr;
        nw4r::g3d::ResAnmTexSrt mResTexSrt;
        m3d::anmTexSrt_c mAnmTexSrt;

        mVec3_c mPos;
        mVec3_c mSpeed;
        mVec3_c mLastPos;

        dBg_ctr_c mBgCtr;
        int mTravelInfoIdx;
        daEnSnakeBlock_c *mpOwner;

        short _1B4;
    };

    class dCtrlBlock_c : public dBlock_c {
    public:
        dCtrlBlock_c();
        virtual ~dCtrlBlock_c();

        bool calcPos(u8 *travelInfo);
        bool calcTravelPos(u8 *travelInfo);

        int mSnakeSpeed;
    };

    daEnSnakeBlock_c();
    ~daEnSnakeBlock_c();

    int create();
    int doDelete();
    int execute();
    int draw();

    void deleteReady();

    void initBlock();
    void initBlockPath();

    void calcAnm();
    void createMdl();
    void deleteBlock();

    void setStopState();

    void calcBgCtr();
    void initTravelInfo();

    void setBlockPos();
    void setActorPos();

    bool chkCollapseDelete(); ///< Returns if the Tail is below the screen edge (meaning the collapse has ended, and the actor can be deleted)
    bool chkOffScreen(); ///< Returns if the Snake Block is far past the right edge of the screen (used to delete it)

    // New
    void drawLightMask();
    void chkPathLoop();

    static void callBackF(dActor_c *, dActor_c *);
    static void callBackH(dActor_c *, dActor_c *);
    static void callBackW(dActor_c *, dActor_c *);

    dHeapAllocator_c mAllocator;

    dCtrlBlock_c mHead;
    dCtrlBlock_c mTail;
    dBlock_c mBlocks[20];

    int mBlockNum;
    u8 *mpTravelInfo;
    int mTravelInfoIdx;
    short mShakeTime;

    sStateID_c *mpStopState;
    int mSnakeType; ///< Used for custom snake types
    int mCollapse2Idx;
    int mCreateAnmBlockIdx;
    int mCreateAnmBlockNum;
    u8 _pad[4];

    mHeapAllocator_c mMaskAllocs[22];
    dCircleLightMask_c *mpLightMaskH;
    dCircleLightMask_c *mpLightMaskT;
    dCircleLightMask_c *mpLightMasks[20];

    static float sc_snakeSpeeds[4];
    static sStateID_c *sc_stopStates;
    static Vec2 sc_ctrlPosMods[5];
    static int sc_glbSnakeNum;
    static Vec2 sc_collapseSpeeds[5];

    static const char *sc_resName[];

    STATE_FUNC_DECLARE(daEnSnakeBlock_c, Wait);
    STATE_FUNC_DECLARE(daEnSnakeBlock_c, Move);
};