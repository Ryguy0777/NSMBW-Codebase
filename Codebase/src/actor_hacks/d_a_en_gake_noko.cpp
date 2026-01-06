#include <kamek.h>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>

// source: https://github.com/Meatball132/NSMBW_GAKENOKO_fix/blob/master/gakenoko/gakenoko.cpp

struct daEnGakeNoko_c : public dEn_c {
	dHeapAllocator_c mAllocator;
	nw4r::g3d::ResFile mRes;
	m3d::mdl_c mModel;
	m3d::anmChr_c mAnmChr;
	nw4r::g3d::ResAnmTexPat mResAnmTexPat;
	m3d::anmTexPat_c mAnmTexPat;
	u32 m_5ec;
	int mCountdown;
	u8 mPad[0x12C];

	STATE_FUNC_DECLARE(daEnGakeNoko_c, FoolMove);
	STATE_FUNC_DECLARE(daEnGakeNoko_c, Move);
	STATE_FUNC_DECLARE(daEnGakeNoko_c, Air);
	STATE_FUNC_DECLARE(daEnGakeNoko_c, Awake);
	STATE_FUNC_DECLARE(daEnGakeNoko_c, Fall);
	STATE_FUNC_DECLARE(daEnGakeNoko_c, Fall2);
	STATE_FUNC_DECLARE(daEnGakeNoko_c, Hang);
	STATE_FUNC_DECLARE(daEnGakeNoko_c, DieFumi);
};

sCcDatNewF l_gakenoko_cc = {
    0.0f,  // float xDistToCenter;
    11.0f,  // float yDistToCenter;
    8.0f,  // float xDistToEdge;
    11.0f,  // float yDistToEdge;
    3,  // u8 category1;
    0,  // u8 category2;
    0x4F,  // u32 bitfield1;
    0xFFBAFFFE,  // u32 bitfield2;
    0,  // u16 unkShort1C;
    &dEn_c::normal_collcheck  // Callback callback;
};

kmBranchDefCpp(0x80A025A0, NULL, int, daEnGakeNoko_c *self) {
	self->mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], 0, 0x20);
	self->mRes = dResMng_c::m_instance->getRes("nokonokoB", "g3d/nokonokoB.brres");

	nw4r::g3d::ResMdl resMdl = self->mRes.GetResMdl("nokonokoB");
	self->mModel.create(resMdl, &self->mAllocator, 0x7FFF, 1, 0);
	dActor_c::setSoftLight_Enemy(self->mModel);

	nw4r::g3d::ResAnmChr resAnmChr = self->mRes.GetResAnmChr("net_walk2");
	self->mAnmChr.create(resMdl, resAnmChr, &self->mAllocator, 0);

	self->mResAnmTexPat = self->mRes.GetResAnmTexPat("nokonokoB");
	self->mAnmTexPat.create(resMdl, self->mResAnmTexPat, &self->mAllocator, 0, 1);
	self->mAnmTexPat.setAnm(self->mModel, self->mResAnmTexPat, 0, m3d::FORWARD_ONCE);

	self->mAllocator.adjustFrmHeap();

	// Nybble 10: green (0) / red (1)
	if ((self->mParam >> 8) & 1) {
		self->changeState(daEnGakeNoko_c::StateID_Move);
	} else {
		self->changeState(daEnGakeNoko_c::StateID_FoolMove);
	}
	if ((self->mParam >> 8) & 1) {
		self->mAnmTexPat.setPlayMode(m3d::FORWARD_ONCE, 0);
		self->mModel.setAnm(self->mAnmTexPat, 0.0);
		self->mAnmTexPat.setFrame(1.0, 0);
	} else {
		self->mAnmTexPat.setFrame(0.0, 0);
	}

	// Nybble 11: initial direction: right (0) / left (1)
	if ((self->mParam >> 4) & 1) {
		// (Logic copied from how the Move state switches directions
		// at the end of a ledge -- basic block at 80A02E00 in PALv1)
		self->mDirection ^= 1;
		self->mSpeed.x = -self->mSpeed.x;
		self->mAnmChr.mPlayMode = 2;
	}

	// Nybble 12: if 1, vertical positioning is improved
	if (self->mParam & 1) {
		self->mPos.y += 16.0;
	}

	// ActivePhysics for 2021
	self->mCc.set(self, &l_gakenoko_cc);
	self->mCc.entry();

	self->mAngle.y = 0x8000;  // Face the wall

	return 1;
}


// Edit some Y-position-related constants in various states if nybble 12 is enabled
static const float twenty_four_or_alt_f[2] = { 24.0f, 8.0f };
static const float thirty_one_or_alt_f[2] = { 31.0f, 15.0f };
kmCallDefAsm(0x80a02de4) {  // Move
	lwz r9, 4(r30)
	lis r10, twenty_four_or_alt_f@h
	ori r10, r10, twenty_four_or_alt_f@l
	clrlslwi r9, r9, 31, 2
	lfsx fp0, r10, r9
}
kmCallDefAsm(0x80a02f80) {  // FoolMove
	lwz r9, 4(r30)
	lis r10, twenty_four_or_alt_f@h
	ori r10, r10, twenty_four_or_alt_f@l
	clrlslwi r9, r9, 31, 2
	lfsx fp0, r10, r9
}
kmCallDefAsm(0x80a032c8) {  // Fall
	lwz r9, 4(r30)
	lis r10, thirty_one_or_alt_f@h
	ori r10, r10, thirty_one_or_alt_f@l
	clrlslwi r9, r9, 31, 2
	add r5, r10, r9
}