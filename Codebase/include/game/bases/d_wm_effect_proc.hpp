#pragma once

#include <game/mLib/m_3d/scn_leaf.hpp>

class dWmEffectProc_c : public m3d::scnLeaf_c {
public:
    ~dWmEffectProc_c();
    leafType_e getType() const;
    virtual void drawOpa();
    virtual void drawXlu();
    virtual bool create(mAllocator_c *allocator, int opaPriority, int xluPriority, int unused);
};