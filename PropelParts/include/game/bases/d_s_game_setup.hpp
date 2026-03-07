#pragma once

#include <game/bases/d_scene.hpp>

/// @unofficial
class dScGameSetup_c : public dScene_c {
public:
    void prepareLightScene();

    static dScGameSetup_c *m_instance;
};