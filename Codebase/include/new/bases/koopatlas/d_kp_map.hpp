#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_s_effect_proc.hpp>
#include <game/mLib/m_3d.hpp>

#include <new/bases/koopatlas/d_s_koopatlas.hpp>

#ifdef KP_MAP_TILE_REPORT
#define TileReport OSReport
#else
#define TileReport(...)
#endif

#ifdef KP_MAP_BOUND_REPORT
#define BoundReport OSReport
#else
#define BoundReport(...)
#endif

#ifdef KP_MAP_DOODAD_REPORT
#define DoodadReport OSReport
#else
#define DoodadReport(...)
#endif

#define EFFECT_PROC_COUNT 12

class dKPMap_c : public dBase_c {
public:
    class dMapRender_c : public m3d::proc_c {
    public:
        dMapRender_c();

        void drawOpa();
        void drawXlu();

    private:
        void loadTexture(GXTexObj *obj);

        void loadCamera();
        void loadCamera(mMtx_c matrix);

        void beginRendering();
        void endRendering();

        void drawLayers();

        //void renderTileLayer(dKPLayer_s *layer, dKPLayer_s::sector_s *sector);
        //void renderDoodadLayer(dKPLayer_s *layer);

    public:
        mAllocator_c mAllocator;

    private:
        GXTexObj *mpCurrentTexture;
        mMtx_c mRenderMtx;
        float mBaseZ;

        int mMinX;
        int mMinY;
        int mMaxX;
        int mMaxY;
    };

    dKPMap_c();

    int create();
    int doDelete();
    int execute();
    int draw();

    void spinLaunchStar();

    void spawnMapEffects();
    //void renderPathLayer(dKPLayer_s *layer);

    dMapRender_c mRender;
    dScEffectProc_c mEffectProcs[EFFECT_PROC_COUNT];

    mEf::levelEffect_c mEffects[7];

    dHeapAllocator_c mAllocator;
    m3d::mdl_c mBgModel;
    m3d::anmTexSrt_c mBgSrtAnm;
    mMtx_c mBgMtx;

    bool mDispLaunchStar;
    nw4r::math::VEC2 mLaunchStarPos;
    m3d::mdl_c mLaunchStarModel;
    m3d::anmChr_c mLaunchStarAnm;
    mMtx_c mLaunchStarMtx;

    static dKPMap_c *m_instance;
    static const int sc_effectGroups[EFFECT_PROC_COUNT];
    static const int sc_effectPrios[EFFECT_PROC_COUNT];
};

#endif

