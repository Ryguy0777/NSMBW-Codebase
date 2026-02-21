#pragma once
#include <game/bases/d_a_en_kuribo.hpp>

class daEnKakibo_c : public daEnKuribo_c {
public:
    virtual void executeState_Walk();

    virtual void createBodyModel();

    virtual bool isBgmSync() const;

    bool checkForLedge(float xOffset);
};