#pragma once

#include <types.h>
#include <lib/egg/gfxe/eggBaseCamera.h>

namespace EGG {

/// @unofficial
class LookAtCamera : public BaseCamera {
public:
    virtual void doUpdateMatrix();
};

} // namespace EGG