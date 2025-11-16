#pragma once

#include <types.h>
#include <lib/egg/gfxe/eggFrustum.h>
#include <nw4r/math.h>

namespace EGG {

/// @unofficial
class OrthoData {
public:
    OrthoData();

    void setTBLR(float top, float bottom, float left, float right);
    void setG3DCamera(nw4r::g3d::Camera &);

    void *m_vtable;
    float m_04;
    u8 mPad[0x3C];
    int m_44;

    float mOrthoNear;
    float mOrthoFar;
    float mOrthoTop;
    float mOrthoBottom;
    float mOrthoLeft;
    float mOrthoRight;
};

} // namespace EGG