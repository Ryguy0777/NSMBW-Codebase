#pragma once
#include <propelparts/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <propelparts/koopatlas_config.h>

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_mtx.hpp>

// Forward declaration
struct dKpNode_s;

class dKpNodeMdl_c {
public:
    virtual void createMdl();
    virtual void draw();

    dKpNode_s *mpParent;
    dHeapAllocator_c mAllocator;
    mMtx_c mMatrix;
    m3d::mdl_c mModel;
};

// Course Nodes
class dKpCourseMdl_c : public dKpNodeMdl_c {
public:
    void createMdl();
    void draw();

    // Nothing new yet
};

#endif