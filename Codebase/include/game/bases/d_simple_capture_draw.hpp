#pragma once

#include <game/mLib/m_3d/scn_leaf.hpp>
#include <game/mLib/m_allocator.hpp>

class dSimpleCaptureDraw_c : public m3d::scnLeaf_c {
public:
    ~dSimpleCaptureDraw_c();
    leafType_e getType() const;
    virtual void drawOpa();
    virtual void drawXlu();

    void create(mAllocator_c *allocator);

    void *mpCaptureTexture;
};