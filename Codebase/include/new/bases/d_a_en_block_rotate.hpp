#pragma once
#include <game/bases/d_a_en_blockmain.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_bg.hpp>

// SMW-style flip blocks

class daEnBlockRotate_c : public daEnBlockMain_c {
public:
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual int preDraw();

    virtual void initialize_upmove();
    virtual void initialize_downmove();

    virtual void block_upmove();
    virtual void block_downmove();

    void calcModel();
    void blockWasHit(bool isDown);
    bool playerOverlaps();
    void createItem();
    void createEmpty();
    void destroyBlock();

    static void callBackF(dActor_c *self, dActor_c *other);

    STATE_FUNC_DECLARE(daEnBlockRotate_c, Wait);
    STATE_FUNC_DECLARE(daEnBlockRotate_c, Flipping);

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mRes;
    m3d::mdl_c mFlipBlockModel;

    int mFlipsRemaining;
    int mCoinsRemaining;

    int mContents;
    bool mIndestructible;
};

const u32 l_item_values[16] = {
    0xF,				// Empty
    2,					// Mushroom
    0,					// Fire
    0,					// Fire
    0x15,				// Propeller
    0x11,				// Penguin
    0x19,				// Mini
    1,					// Star
    0x1B,				// Mushroom
    0xC,				// Mushroom
    4,					// Coin 2
    7,					// 1-up
    0xA,				// Mushroom
    0xD,				// Mushroom
    8,					// Mushroom
    0xE					// Ice
};

const bool l_early_items[16] = {
    false,
    true,
    false,
    false,
    true,
    false,
    false,
    false,
    true,
    false,
    true,
    false,
    false,
    false,
    false,
    false
};