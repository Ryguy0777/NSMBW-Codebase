#pragma once
#include <lib/egg/gfxe/eggBaseCamera.h>

namespace EGG {

/// @unofficial
class LookAtCamera : public BaseCamera {
public:
    virtual nw4r::math::MTX34 getViewMatrix();
    virtual nw4r::math::MTX34 getViewMatrix() const;
    virtual void updateMatrix();
    virtual void doUpdateMatrix();
    virtual void loadMatrix();
    virtual void loadOldMatrix();
    virtual nw4r::math::VEC3 getPosition();
    virtual void draw(EGG::BaseCamera *);
    virtual void doDraw();
    virtual nw4r::math::MTX34 *getViewMatrixOld();
};

} // namespace EGG