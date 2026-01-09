#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_s_koopatlas.hpp>
#include <lib/egg/gfxe/eggLookAtCamera.h>
#include <lib/egg/gfxe/eggOrthoData.h>
#include <lib/egg/gfxe/eggScreen.h>

#define STD_ZOOM 2.8f

class dKPCamera_c : public dBase_c {
public:
    dKPCamera_c();

    int create();
    int execute();
    int draw();

    void calcScreenGeometry();
    void calcCameras();
    void updateCameras();
    void setPosAndTarget(float camPosZ);

    void panToBounds(float left, float top, float right, float bottom);
    void panToPosition(float x, float y, float zoom=STD_ZOOM);

    EGG::Screen mScreen;
    EGG::LookAtCamera mCamera3D;

    EGG::OrthoData mOrthoData2D;
    EGG::LookAtCamera mCamera2D;

    nw4r::math::VEC3 mCamPos;
    nw4r::math::VEC3 mCamTarget;
    nw4r::math::VEC3 mCamUp;

    float mCurrentX;
    float mCurrentY;
    float mZoomLevel;

    float mZoomDivisor;

    float mScreenLeft;
    float mScreenTop;
    float mScreenWidth;
    float mScreenHeight;

    float mPanFromX;
    float mPanFromY;
    float mPanToX;
    float mPanToY;

    float mPanFromZoom;
    float mPanToZoom;

    float mPanCurrentStep;
    float mPanTotalSteps;

    bool mFollowPlayer;
    bool mIsPanning;

    static dKPCamera_c *m_instance;
};
#endif