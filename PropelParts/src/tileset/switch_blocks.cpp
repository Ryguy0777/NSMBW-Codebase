#include <kamek.h>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_pswitch_manager.hpp>

// Read entire switch palace u8 from replay data
kmWriteNop(0x8091F800);
kmWriteNop(0x8091F804);
kmWriteNop(0x8091F808);
kmWrite32(0x8091F80C, 0x98830380);

// Store switch palace u8 into dInfo_c from savedata
kmWriteNop(0x80102C28);
kmWriteNop(0x80102C30);
kmWriteNop(0x80102C38);
kmWrite32(0x80102C3C, 0x98830380);

// Directly store dInfo_c switch palace u8 into dBg_c
kmBranchDefCpp(0x800784D0, NULL, void, dBg_c *_self) {
    _self->mTexMng.mWmSwitchRelated = dInfo_c::m_instance->mWmSwitch;

    if (dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_4) {
        _self->mTexMng.mWmSwitchRelated = 1;
    }
}

// Add switch block checks to dBg_c

// Swap the tile texture
kmBranchDefCpp(0x80081ab0, NULL, u16, u16 tileNumber) {
    if (dPSwManager_c::ms_instance->checkMove()) {
        // P-Switch pressed
        if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::P_SWICH)) {
            if (tileNumber == 0x1E) {
                return 0x30;
            }
            if (tileNumber == 0x30) {
                return 0x1E;
            }
            if (tileNumber == 0x2E) {
                return 0x2F;
            }
        }
        // !-Switch pressed
        if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::Q_SWICH)) {
            if (tileNumber == 0x3B) {
                return 0x37;
            }
            if (tileNumber == 0x3C) {
                return 0x37;
            }
        }
        // Yellow !-Switch pressed
        if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::Y_SWICH)) {
            if (tileNumber == 0xA6) {
                tileNumber = 0x96;
            }
        }
        // Red !-Switch pressed
        if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::R_SWICH)) {
            if (tileNumber == 0xA7) {
                tileNumber = 0x97;
            }
        }
        // Green !-Switch pressed
        if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::G_SWICH)) {
            if (tileNumber == 0xA8) {
                tileNumber = 0x98;
            }
        }
        // Green !-Switch pressed
        if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::B_SWICH)) {
            if (tileNumber == 0xA9) {
                tileNumber = 0x99;
            }
        }
    }

    u8 switchFlags = dBg_c::m_bg_p->IsWmSwitchPushed();
    // Worldmap switch
    if (switchFlags & 1) {
        if (tileNumber == 0x3B) {
            tileNumber = 0x37;
        } else if (tileNumber == 0x3C) {
            tileNumber = 0x37;
        }
    }
    // Yellow switch
    if (switchFlags & 2) {
        if (tileNumber == 0xA6) {
            tileNumber = 0x96;
        }
    }
    // Red switch
    if (switchFlags & 4) {
        if (tileNumber == 0xA7) {
            tileNumber = 0x97;
        }
    }
    // Green switch
    if (switchFlags & 8) {
        if (tileNumber == 0xA8) {
            tileNumber = 0x98;
        }
    }
    // Blue switch
    if (switchFlags & 16) {
        if (tileNumber == 0xA9) {
            tileNumber = 0x99;
        }
    }

    return tileNumber;
}

// Swap tile behavior
kmBranchDefCpp(0x80081960, NULL, u64, u64 tileBehavior, u16 tileNumber) {
    int behaviorUpper = tileBehavior >> 0x20;
    if (dPSwManager_c::ms_instance->checkMove()) {
        u32 behaviorLower = tileBehavior & 0xFF;
        // P-Switch 
        if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::P_SWICH)) {
            if (tileBehavior & 0x200000000 && behaviorLower == 0) {
                return 0x1000000000;
            }
            if (tileBehavior & 0x1000000000 && behaviorLower == 0 && tileNumber == 0x30) {
                return 0x200000000;
            }
            if (behaviorUpper == 0 && behaviorLower == 0x35) {
                return 0x200000004;
            }
        }
        if (behaviorUpper == 0) {
            // !-Switch pressed
            if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::Q_SWICH)) {
                if (behaviorLower == 0x36) {
                    return 0x800000003;
                }
                if (behaviorLower == 0x37) {
                    return 0x800000003;
                }
            }
            // Yellow !-Switch pressed
            if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::Y_SWICH)) {
                if (tileNumber == 0xA6) {
                    tileBehavior = 0x800000003;
                }
            }
            // Red !-Switch pressed
            if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::R_SWICH)) {
                if (tileNumber == 0xA7) {
                    tileBehavior = 0x800000003;
                }
            }
            // Green !-Switch pressed
            if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::G_SWICH)) {
                if (tileNumber == 0xA8) {
                    tileBehavior = 0x800000003;
                }
            }
            // Blue !-Switch pressed
            if (dPSwManager_c::ms_instance->checkSwitch(dPSwManager_c::B_SWICH)) {
                if (tileNumber == 0xA9) {
                    tileBehavior = 0x800000003;
                }
            }
        }
    }
    if (behaviorUpper == 0) {
        u8 switchFlags = dBg_c::m_bg_p->IsWmSwitchPushed();
        // Worldmap switch
        if (switchFlags & 1) {
            if (tileNumber == 0x3B) {
                tileBehavior = 0x800000003;
            } else if (tileNumber == 0x3C) {
                tileBehavior = 0x800000003;
            }
        }
        // Yellow switch
        if (switchFlags & 2) {
            if (tileNumber == 0xA6) {
                tileBehavior = 0x800000003;
            }
        }
        // Red switch
        if (switchFlags & 4) {
            if (tileNumber == 0xA7) {
                tileBehavior = 0x800000003;
            }
        }
        // Green switch
        if (switchFlags & 8) {
            if (tileNumber == 0xA8) {
                tileBehavior = 0x800000003;
            }
        }
        // Blue switch
        if (switchFlags & 16) {
            if (tileNumber == 0xA9) {
                tileBehavior = 0x800000003;
            }
        }
    }

    return tileBehavior;
}

// Use bit 1 of switch palace u8 for red blocks

// Enable switch
kmBranchDefCpp(0x80927e90, NULL, void) {
    dInfo_c::m_instance->mWmSwitch |= (1 << 0);
}

// Disable switch
kmBranchDefCpp(0x80927eb0, NULL, void) {
    dInfo_c::m_instance->mWmSwitch &= ~(1 << 0);
}

// Get switch status
kmBranchDefCpp(0x80927ed0, NULL, u8) {
    return dInfo_c::m_instance->mWmSwitch & 1;
}

kmBranchDefAsm(0x800784e0, 0x800784e4) {
    lbz r0, 0x380(r5)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x800ee6d0, 0x800ee6d4) {
    lbz r4, 0x380(r4)
    andi. r4, r4, 1
    blr
}

kmBranchDefAsm(0x800fbb34, 0x800fbb38) {
    lbz r0, 0x380(r4)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x801025ec, 0x801025f0) {
    lbz r0, 0x380(r6)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x808ce71c, 0x808ce720) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x808ce768, 0x808ce76c) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x808ce83c, 0x808ce840) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x808cedc8, 0x808cedcc) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x808edf44, 0x808edf48) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x808fb7f8, 0x808fb7fc) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x808fc8b0, 0x808fc8b4) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x80906674, 0x80906678) {
    lbz r0, 0x380(r3)
    andi. r0, r0, 1
    blr
}

kmBranchDefAsm(0x809252e8, 0x809252ec) {
    lbz r6, 0x380(r6)
    andi. r6, r6, 1
    blr
}

kmBranchDefAsm(0x809256cc, 0x809256d0) {
    lbz r6, 0x380(r6)
    andi. r6, r6, 1
    blr
}

kmBranchDefAsm(0x80ac37e8, 0x80ac37ec) {
    lbz r6, 0x380(r6)
    andi. r6, r6, 1
    blr
}