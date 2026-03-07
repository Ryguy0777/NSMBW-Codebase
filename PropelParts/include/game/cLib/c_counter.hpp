#pragma once
#include <types.h>
/// @file

/// @ingroup clib
class cCounter_c {
public:
    static void clear();

    static u32 m_gameFrame;
    static u32 m_exeFrame;
};