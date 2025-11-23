#pragma once

#include <types.h>
#include <lib/egg/gfxe/eggFrustum.h>
#include <nw4r/math.h>

namespace EGG {

/// @unofficial
class LookAtCamera /*: public BaseCamera*/ {
private:
    nw4r::math::MTX34 mMatrix;
    nw4r::math::MTX34 mPreviousMatrix;

public:
    nw4r::math::VEC3 mCamPos;
    nw4r::math::VEC3 mTarget;
    nw4r::math::VEC3 mCamUp;

    virtual nw4r::math::MTX34 getViewMatrix();
    virtual nw4r::math::MTX34 getViewMatrix() const;
    virtual void updateMatrix();
    virtual void doUpdateMatrix();
    virtual void loadMatrix();
    virtual void loadOldMatrix();
    virtual nw4r::math::VEC3 getPosition();
    virtual void draw(EGG::LookAtCamera *);
    virtual void doDraw();
    virtual Mtx *getViewMatrixOld();

    void setG3DCamera(nw4r::g3d::Camera &);
};

} // namespace EGG