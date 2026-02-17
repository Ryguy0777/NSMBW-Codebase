#pragma once
#include <game/mLib/m_3d.hpp>

// Map model and actor culling debug rendering

class dWmCullRender_c : public m3d::proc_c {
public:
    dWmCullRender_c();

    virtual void drawOpa();
    virtual void drawXlu();

    static dWmCullRender_c m_instance;
};