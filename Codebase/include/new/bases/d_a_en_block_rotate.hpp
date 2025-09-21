#pragma once
#include <game/bases/d_a_en_block_main.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_eff_actor_manager.hpp>

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

    static void callBackF(dActor_c *self, dActor_c *other);

    STATE_FUNC_DECLARE(daEnBlockRotate_c, Wait);
    STATE_FUNC_DECLARE(daEnBlockRotate_c, Flipping);
    STATE_FUNC_DECLARE(daEnBlockRotate_c, Empty);

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mRes;
	m3d::mdl_c mFlipBlockModel;

    dPanelObjList_c mUsedTile;

	int mFlipsRemaining;
    int mCoinsRemaining;

    int mContents;
    bool mIndestructible;
};

const u32 l_item_values[16] = {
	0xF,				// empty
	2,					// mushroom
	0,					// fire
	0,					// fire
	0x15,				// propeller
	0x11,				// penguin
	0x19,				// mini
	1,					// star
	0x1B,				// mushroom
	0xC,				// mushroom
	4,					// coin 2
	7,					// 1up
	0xA,				// mushroom
	0xD,				// mushroom
	8,					// mushroom
	0xE					// ice
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