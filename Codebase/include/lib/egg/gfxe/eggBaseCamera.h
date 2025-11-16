#pragma once

#include <types.h>
#include <lib/egg/gfxe/eggFrustum.h>
#include <nw4r/math.h>

namespace EGG {

/// @unofficial
class BaseCamera {
public:
    virtual void updateMatrix();

    void setG3DCamera(nw4r::g3d::Camera &);

    Mtx mMatrix;
    Mtx mPreviousMatrix;

    nw4r::math::VEC3 mCamPos;
    nw4r::math::VEC3 mTarget;
    nw4r::math::VEC3 mCamUp;
};

} // namespace EGG