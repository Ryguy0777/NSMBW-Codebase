#pragma once

#include <game/mLib/m_3d.hpp>

// collision debug rendering

class dCollisionRender_c : public m3d::proc_c {
    public:
        dCollisionRender_c();

        virtual void drawOpa();
        virtual void drawXlu();
};