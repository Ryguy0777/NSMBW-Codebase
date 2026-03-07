#pragma once
#include <propelparts/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <propelparts/koopatlas_config.h>

#include <propelparts/bases/koopatlas/d_s_koopatlas.hpp>
#include <propelparts/constants/koopatlas_constants.h>
#include <lib/egg/gfxe/eggLookAtCamera.h>
#include <lib/egg/gfxe/eggOrthoData.h>
#include <lib/egg/gfxe/eggScreen.h>

template <typename T>
inline T min(T a, T b) { return (a < b) ? a : b; }

template <typename T>
inline T max(T a, T b) { return (a > b) ? a : b; }

class dKpCamera_c : public dBase_c {
public:
    dKpCamera_c();

    int create();
    int execute();
    int draw();

    void calcScreenGeometry();
    void calcCameras();
    void updateCameras();
    void setPosAndTarget(float camPosZ);

    void panToBounds(float left, float top, float right, float bottom);
    void panToPosition(float x, float y, float zoom=CAMERA_ZOOM);

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

    bool mDoFollowPlayer;
    bool mIsPanning;

    static dKpCamera_c *m_instance;
};
#endif