#pragma once
#include <game/bases/d_enemy_boss.hpp>

class daEnBossPackun_c : public dEnBoss_c {
public:
    enum BOSS_PACKUN_ANIM_e {
        ANIM_ATTACK,
        ANIM_ATTACK_ED,
        ANIM_ATTACK_WAIT,
        ANIM_DEAD,
        ANIM_DOWN_WAIT,
        ANIM_FLY,
        ANIM_GET_UP,
        ANIM_GROGGY,
        ANIM_JUMP_END,
        ANIM_JUMP_START,
        ANIM_LAYDOWN_SMOOSHED,
        ANIM_SLEEP_WAIT,
        ANIM_WAKEUP,
        ANIM_SLEEP_JR_DMG
    };

    virtual int execute();
    virtual int draw();

    virtual void finalUpdate();

    virtual void createModel();
};