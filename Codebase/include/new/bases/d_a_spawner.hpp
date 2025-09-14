#pragma once

#include <game/bases/d_actor.hpp>

// an actor that spawns another actor when a target event is triggered.
// this is currently a test for custom actors.

class daSpawner_c : public dActor_c {
public:
    daSpawner_c() {};
    ~daSpawner_c() {};

    virtual int execute();

    u8 mEventID;
    u16 mChildProfileID;
    bool mHasChild;
};