#pragma once
#include <nw4r/math/math_types.h>

namespace dScreen {
    nw4r::math::VEC2 GetScreenSize();
    nw4r::math::VEC3 GetScreenScale(float, float, float, float);

    float GetScreenPosY();
}
