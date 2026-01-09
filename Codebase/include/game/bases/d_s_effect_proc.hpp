#pragma once

#include <game/bases/d_wm_effect_proc.hpp>

class dScEffectProc_c : public dWmEffectProc_c {
public:
    ~dScEffectProc_c();
    leafType_e getType() const;
    virtual void drawOpa();
    virtual void drawXlu();
    virtual bool create(mAllocator_c *allocator, int opaPriority, int xluPriority, int effectGroupID);

    int mEffectGroupID;
};