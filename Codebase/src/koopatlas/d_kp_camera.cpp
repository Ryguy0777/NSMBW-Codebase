#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <game/bases/d_screen.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_video.hpp>
#include <lib/revolution/VI.h>
#include <new/bases/koopatlas/d_kp_camera.hpp>
#include <new/bases/koopatlas/d_a_kp_player.hpp>

#ifdef KP_CAMERA_REPORT
#define CameraReport OSReport
#else
#define CameraReport(...)
#endif

template <typename T>
inline T max(T a, T b) { return (a > b) ? a : b; }

dKPCamera_c *dKPCamera_c::m_instance = nullptr;

dKPCamera_c *dKPCamera_c::build() {
    dKPCamera_c *c = new dKPCamera_c;
    m_instance = c;
    return c;
}

// Replace WORLD_CAMERA actor
kmWritePointer(0x8098EEC8, &dKPCamera_c::build);


dKPCamera_c::dKPCamera_c() {
    Vec camPos = {0.0, 10.0, 0.0};
    Vec target = {0.0, 0.0, 0.0};
    Vec camUp = {0.0, 1.0, 0.0};

    mCamera3D.mCamPos = camPos;
    mCamera3D.mTarget = target;
    mCamera3D.mCamUp = camUp;

    mCamera2D.mCamPos = camPos;
    mCamera2D.mTarget = target;
    mCamera2D.mCamUp = camUp;

    mCamPos = nw4r::math::VEC3(0.0f, 0.0f, 1.0f);
    mCamUp = nw4r::math::VEC3(0.0f, 1.0f, 0.0f);

    mOrthoData2D.mOrthoNear = -20000.0f;
    mOrthoData2D.mOrthoFar = 20000.0f;

    mCurrentX = 416;
    mCurrentY = -224;
    mZoomLevel = STD_ZOOM;

    mFollowPlayer = true;
}

int dKPCamera_c::create() {
    // Bad Code
    mScreen.mProjType = EGG::Frustum::PROJ_ORTHO;
    mScreen.mFlags |= 1;

    EGG::Screen::TVModeInfo *tvInfo = &mScreen.sTVModeInfo[mScreen.sTVMode];
    mScreen.mSize.x = tvInfo->width;
    mScreen.mSize.y = tvInfo->height;
    mScreen.mNearZ = 1.0;
    mScreen.mFarZ = 20000.0;

    if (mScreen.mCanvasMode != EGG::Frustum::CANVASMODE_1) {
        mScreen.mFlags |= 1;
        mScreen.mCanvasMode = EGG::Frustum::CANVASMODE_1;
    }

    doStuff(10000.0);
    generateCameraMatrices();
    updateCameras();
    return true;
}

#define SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x)))

int dKPCamera_c::execute() {
    if (dScKoopatlas_c::instance->mWarpZoneHacks) {
        mCurrentX = 2040.0f;
        mCurrentY = -1460.0f;
        mZoomLevel = 3.4f;
    } else if (mIsPanning) {
        // Calculate where we are
        float stepRatio = mPanCurrentStep / mPanTotalSteps;
        stepRatio = 1.0f - SMOOTHSTEP(stepRatio);
        CameraReport("PAN: Step %f / %f ---- Ratio: %f", mPanCurrentStep, mPanTotalSteps, stepRatio);
        CameraReport("From %f, %f to %f, %f --- Zoom: %f to %f\n", mPanFromX, mPanFromY, mPanToX, mPanToY, mPanFromZoom, mPanToZoom);

        mCurrentX = (mPanFromX * stepRatio) + (mPanToX * (1.0f - stepRatio));
        mCurrentY = (mPanFromY * stepRatio) + (mPanToY * (1.0f - stepRatio));
        mZoomLevel = (mPanFromZoom * stepRatio) + (mPanToZoom * (1.0f - stepRatio));
        CameraReport("Calculated: %f, %f with zoom %f\n", currentX, currentY, zoomLevel);

        mPanCurrentStep += 1.0f;

        if (mPanCurrentStep > mPanTotalSteps) {
            // YAY, we reached the end
            mIsPanning = false;
            mCurrentX = mPanToX;
            mCurrentY = mPanToY;
            mZoomLevel = mPanToZoom;
        }
    } else if (mFollowPlayer) {
        daKPPlayer_c *player = daKPPlayer_c::m_instance;
        mCurrentX = player->mPos.x;
        mCurrentY = player->mPos.y;
    }

    calculateScreenGeometry();
    doStuff(10000.0);
    generateCameraMatrices();
    updateCameras(); 
    return true;
}

void dKPCamera_c::panToBounds(float left, float top, float right, float bottom) {
    // Pad it a bit
    left -= 64.0f;
    right += 64.0f;
    top -= 48.0f;
    bottom += 48.0f;

    CameraReport("Panning camera to bounds %f,%f to %f,%f\n", left, top, right, bottom);

    // Figure out the centre x/y we want
    float width = right - left;
    float height = bottom - top;

    float desiredCentreX = left + (width * 0.5f);
    float desiredCentreY = -(top + (height * 0.5f));

    CameraReport("Size: %f x %f ; Desired Centre: %f, %f\n", width, height, desiredCentreX, desiredCentreY);

    // Our default zoom is 2.8
    float minScreenWidth = mVideo::l_rayoutWidthF * 1.2f;
    float minScreenHeight = mVideo::l_rayoutHeightF * 1.2f;
    float maxScreenWidth = mVideo::l_rayoutWidthF * 4.0f;
    float maxScreenHeight = mVideo::l_rayoutHeightF * 4.0f;

    CameraReport("Screen Sizes: Minimum possible %f x %f ; Maximum possible %f x %f\n", minScreenWidth, minScreenHeight, maxScreenWidth, maxScreenHeight);

    // First off, gotta cap it to the ratio
    float screenRatio = mVideo::l_rayoutWidthF / mVideo::l_rayoutHeightF;
    float boundsRatio = width / height;
    float correctedWidth = width, correctedHeight = height;
    float desiredZoomLevel;
    CameraReport("Actual screen size is %f x %f --- Screen Ratio: %f, Bounds Ratio: %f\n", mVideo::l_rayoutWidthF, mVideo::l_rayoutHeightF, screenRatio, boundsRatio);

    float widthScale = width / mVideo::l_rayoutWidthF;
    float heightScale = height / mVideo::l_rayoutHeightF;

    if (heightScale > widthScale) {
        // Thing is constrained on the top/bottom
        desiredZoomLevel = heightScale;
    } else {
        // Thing is constrained on the left/right
        desiredZoomLevel = widthScale;
    }

    CameraReport("Desired zoom level is %f\n", desiredZoomLevel);

    // Cap the zoom
    if (desiredZoomLevel < 2.0f)
        desiredZoomLevel = 2.0f;
    if (desiredZoomLevel > 4.5f)
        desiredZoomLevel = 4.5f;
    CameraReport("After capping: %f\n", desiredZoomLevel);

    // And we're almost there YAY
    panToPosition(desiredCentreX, desiredCentreY, desiredZoomLevel);
}


void dKPCamera_c::panToPosition(float x, float y, float zoom) {
    mPanFromX = mCurrentX;
    mPanFromY = mCurrentY;
    mPanFromZoom = mZoomLevel;

    mPanToX = x;
    mPanToY = y;
    mPanToZoom = zoom;

    float xDiff = abs(mPanToX - mPanFromX);
    float yDiff = abs(mPanToY - mPanFromY);

    float panLength = sqrtf((xDiff*xDiff) + (yDiff*yDiff));
    float panSteps = panLength / 2.3f;
    float scaleSteps = abs(mPanToZoom - mPanFromZoom) / 0.1f;
    float stepCount = max(panSteps, scaleSteps);

    CameraReport("Pan length: %f over %f steps\n", panLength, panSteps);
    CameraReport("Scale steps: %f\n", scaleSteps);
    CameraReport("Step Count: %f\n", stepCount);

    mPanCurrentStep = 0.0f;
    mPanTotalSteps = stepCount;

    mIsPanning = true;
    mFollowPlayer = false;
}


int dKPCamera_c::draw() {
    const GXRenderModeObj *rmode = nw4r::g3d::G3DState::GetRenderModeObj();

    nw4r::g3d::Camera cam(m3d::getCamera(0));
    nw4r::g3d::Camera cam2d(m3d::getCamera(1));

    if (rmode->field_rendering != 0) {
        cam.SetViewportJitter(VIGetNextField());
        cam2d.SetViewportJitter(VIGetNextField());
    }
    //cam2d.SetOrtho(rmode->efbHeight, 0.0f, 0.0f, rmode->fbWidth * (IsWideScreen() ? 1.3333334f : 1.0f), -100000.0f, 100000.0f);
    return true;
}


void dKPCamera_c::calculateScreenGeometry() {
    mZoomDivisor = 1.0 / mZoomLevel;

    mScreenWidth = mVideo::l_rayoutWidthF / mZoomDivisor;
    mScreenHeight = mVideo::l_rayoutHeightF / mZoomDivisor;

    mScreenLeft = mCurrentX - (mScreenWidth * 0.5);
    mScreenTop = mCurrentY + (mScreenHeight * 0.5);
}


void dKPCamera_c::doStuff(float camPosZ) {
    mCamTarget = nw4r::math::VEC3(mScreenLeft + (mScreenWidth * 0.5), (mScreenTop - mScreenHeight) + (mScreenHeight * 0.5), 0.0);
    mCamPos = nw4r::math::VEC3(mCamTarget.x, mCamTarget.y, camPosZ);
}


void dKPCamera_c::generateCameraMatrices() {
    float orthoTop = mScreenLeft * 0.5;
    float orthoLeft = -mScreenWidth * 0.5;
    float orthoBottom = -orthoTop;
    float orthoRight = -orthoLeft;

    mCamera3D.mCamPos = mCamPos;
    mCamera3D.mTarget = mCamTarget;
    mCamera3D.mCamUp = mCamUp;

    mCamera3D.doUpdateMatrix();

    mScreen.ResetOrthographic(orthoTop, orthoBottom, orthoLeft, orthoRight, -100000.0f, 100000.0f);

    nw4r::math::VEC2 screenSize = dScreen::GetScreenSize();
    nw4r::math::VEC3 screenScale = dScreen::GetScreenScale(orthoTop, orthoBottom, orthoLeft, orthoRight);
    mScreen.mPosition.y = dScreen::GetScreenPosY();

    mScreen.mSize.x = screenSize.x;
    mScreen.mFlags |= 1;
    mScreen.mSize.y = screenSize.y;
    mScreen.mScale = screenScale;

    mCamera2D.mCamPos = nw4r::math::VEC3(0.0, 0.0, 0.0);
    mCamera2D.mTarget = nw4r::math::VEC3(0.0, 0.0, -100.0);
    mCamera2D.mCamUp = mCamUp;

    mCamera2D.doUpdateMatrix();

    const GXRenderModeObj *rmode = nw4r::g3d::G3DState::GetRenderModeObj();
    mOrthoData2D.setTBLR(rmode->efbHeight, 0.0, 0.0, rmode->fbWidth * (dGameCom::GetAspectRatio() ? 1.3333334f : 1.0f));
}


void dKPCamera_c::updateCameras() {
    nw4r::g3d::Camera cam0(m3d::getCamera(0));
    mCamera3D.setG3DCamera(cam0);
    mScreen.CopyToG3D(cam0);

    nw4r::g3d::Camera cam1(m3d::getCamera(1));
    mOrthoData2D.setG3DCamera(cam1);
    mCamera2D.setG3DCamera(cam1);
}
#endif