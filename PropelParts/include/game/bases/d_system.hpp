#pragma once
#include <lib/egg/core/eggHeap.h>

namespace dSys_c {
    void create();
    void setBlack(bool);
    void execute();
}

namespace dSystem {
    void fixArena();

    static EGG::Heap *s_FontManagerHeap;
}
