#pragma once

#include <types.h>
#include <lib/egg/gfxe/eggFrustum.h>
#include <nw4r/math.h>

namespace EGG {

class BaseCamera {
public:
    virtual nw4r::math::MTX34 getViewMatrix() = 0;
    virtual nw4r::math::MTX34 getViewMatrix() const = 0;
    virtual void updateMatrix() = 0;
    virtual void doUpdateMatrix() = 0;
    virtual void loadMatrix() = 0;
    virtual void loadOldMatrix() = 0;
    virtual nw4r::math::VEC3 getPosition() = 0;
    virtual void draw(EGG::BaseCamera *) = 0;
    virtual void doDraw() = 0;
    virtual nw4r::math::MTX34 *getViewMatrixOld() = 0;

    void setG3DCamera(nw4r::g3d::Camera &);

private:
    nw4r::math::MTX34 mViewMatrix;
    nw4r::math::MTX34 mViewMatrixOld;

public:
    nw4r::math::VEC3 mCamPos;
    nw4r::math::VEC3 mTarget;
    nw4r::math::VEC3 mCamUp;
};

} // namespace EGG