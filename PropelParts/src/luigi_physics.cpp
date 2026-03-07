#include <kamek.h>

#include <game/bases/d_a_player.hpp>
#include <propelparts/game_config.h>

// Source: https://github.com/luigi-phys/NSMBW-Updated/blob/luigi-physics/code/src/luigi_physics/patches.cpp

#ifdef LUIGI_PHYSICS

extern "C" {

const float luigi__x_speed_inc = 0.07f;
const float luigi__swim_y_speed_add = 1.8f;

extern const float FLOAT_ARRAY_802eedc0[2];

void daPlBase_c__slipPowerSet_1(daPlBase_c *_this) {
    if (_this->isSaka()) {
        _this->mAccelF = _this->getSakaStopAccele((_this->mSpeedF < 0.0f) ? 1 : 0);
        int walk_dir;
        if (_this->mKey.buttonWalk(&walk_dir)) {
            _this->mAccelF *= FLOAT_ARRAY_802eedc0[_this->mBc.getSakaUpDown(walk_dir)];
        }
    } else {
        if (_this->isStatus(daPlBase_c::STATUS_31)) {
            _this->mAccelF = 0.05f;
        } else {
            _this->mAccelF = 0.09f;
        }
    }

    _this->icePowerChange(1);
}

void daPlBase_c__slipPowerSet_0(daPlBase_c *_this) {
    if (std::fabs(_this->mSpeedF) > _this->getSpeedData()->mHighSpeed) {
        _this->mAccelF = 0.75f;
    }

    {
        // HACK: Patching addresses 0x802ef118 & 0x802ef208 using Kamek is not enough
        // as Kamek is not able to perform patches before the ctors of the original game run
        const_cast<sSpeedData *>(_this->mSpeedDataNormal)->mPowerChangeNormal.mTurnAccel = 0.04f;
    }

    sPowerChangeSpeedData local_4c;
    _this->getPowerChangeSpeedData(local_4c);

    int walk_dir;
    if (_this->mKey.buttonWalk(&walk_dir)) {
        if (_this->mSpeedF * daPlBase_c::sc_DirSpeed[_this->mDirection] < 0.0f) {
            _this->mAccelF = local_4c.mTurnAccel;
            if (_this->isStatus(daPlBase_c::STATUS_89)) {
                _this->mAccelF *= 3;
            }

            return;
        }

        if (_this->isSaka()) {
            _this->mAccelF = _this->getSakaMoveAccele((_this->mSpeedF < 0.0f) ? 1 : 0) * FLOAT_ARRAY_802eedc0[_this->mBc.getSakaUpDown(walk_dir)];
            _this->icePowerChange(0);
            return;
        }

        const float x_speed_1_mag = std::fabs(_this->mSpeedF);
        const float x_speed_2_mag = std::fabs(_this->mMaxSpeedF);

        if (x_speed_1_mag < 0.5f) {
            _this->mAccelF = local_4c.mVerySlowAccel;
        } else if (x_speed_1_mag < _this->getSpeedData()->mLowSpeed) {
            if (_this->mKey.buttonDush()) {
                _this->mAccelF = local_4c.mRunSlowAccel;
            } else {
                _this->mAccelF = local_4c.mSlowAccel;
            }
        } else if (x_speed_1_mag < _this->getSpeedData()->mMediumSpeed) {
            if (x_speed_2_mag >= _this->getSpeedData()->mMediumSpeed) {
                _this->mAccelF = local_4c.mMediumAccel;
            } else {
                _this->mAccelF = local_4c.mDefaultAccel;
            }
        } else {
            if (x_speed_2_mag >= _this->getSpeedData()->mMediumSpeed) {
                _this->mAccelF = local_4c.mFastAccel;
            } else {
                _this->mAccelF = local_4c.mDefaultAccel;
            }
        }
    } else {
        if (_this->isSaka()) {
            _this->mAccelF = _this->getSakaStopAccele((_this->mSpeedF < 0.0f) ? 1 : 0);
        } else if (_this->mSpeedF * daPlBase_c::sc_DirSpeed[_this->mDirection] < 0.0f) {
            _this->mAccelF = local_4c.mTurnNoInputAccel;
        } else if (std::fabs(_this->mSpeedF) < _this->getSpeedData()->mLowSpeed) {
            _this->mAccelF = local_4c.mNoInputAccel;
        } else {
            _this->mAccelF = local_4c.mDefaultAccel;
        }

        if (_this->isStatus(daPlBase_c::STATUS_89)) {
            _this->mAccelF *= 3;
        }

        _this->icePowerChange(0);
    }
}

}

kmBranchDefAsm(0x8004c394, 0x8004c398) {
    // No stack saving needed
    nofralloc

    lis       r3, luigi__x_speed_inc@ha
    lfs       f0, luigi__x_speed_inc@l(r3)
    blr
}

// TODO: Patch check at 8013080c 
kmBranchDefAsm(0x801307fc, 0x80130800) {
    // No stack saving needed
    nofralloc

    lis       r3, luigi__swim_y_speed_add@ha
    lfs       f1, luigi__swim_y_speed_add@l(r3)
    blr
}

kmCall(0x80135290, daPlBase_c__slipPowerSet_0); // Originally daPlBase_c::normalPowerSet()

kmCall(0x8004ba98, daPlBase_c__slipPowerSet_1); // Originally daPlBase_c::slipPowerSet(1)
kmCall(0x8004baa8, daPlBase_c__slipPowerSet_0); // Originally daPlBase_c::slipPowerSet(0)

kmWriteFloat(0x802ef118, 0.04f);
kmWriteFloat(0x802ef208, 0.04f);

kmWriteFloat(0x802f591c, 1.6f);
kmWriteFloat(0x802f5920, 1.0f);
kmWriteFloat(0x802f5924, 0.25f);
kmWriteFloat(0x802f5928, -0.18f);
kmWriteFloat(0x802f5930, -0.18f);
kmWriteFloat(0x802f5934, -0.25f);
kmWriteFloat(0x802f5950, -0.3f);
kmWriteFloat(0x802f5954, -0.03f);
kmWriteFloat(0x802f5958, -0.25f);
kmWriteFloat(0x802f5964, 2.0f);
kmWriteFloat(0x802f5968, 1.2f);
kmWriteFloat(0x802f5990, -0.035f);
kmWriteFloat(0x802f5994, -0.05f);
kmWriteFloat(0x802f5998, -0.075f);
// I don't think the following are relevant
kmWriteFloat(0x802f59c0, -0.18f);
kmWriteFloat(0x802f59c4, -0.25f);
kmWriteFloat(0x802f59dc, -0.1f);
kmWriteFloat(0x802f59e0, -0.1f);
kmWriteFloat(0x802f59e4, -0.08f);
kmWriteFloat(0x802f59ec, -0.34f);

kmWriteFloat(0x802f58b8, 0.05f);
kmWriteFloat(0x802f58bc, 0.05f);
kmWriteFloat(0x802f58c0, 0.04f);

kmWriteFloat(0x8042db68, -0.05f);
kmWriteFloat(0x8042db70, -0.035f);
kmWriteFloat(0x8042db6c, -0.082f);
kmWriteFloat(0x8042bb3c, 2.0f);


static int s_jumpAnmID[4] = {-1, -1, -1, -1};
static float s_jumpAnmRate[4] = {0.0f, 0.0f, 0.0f, 0.0f};

kmBranchDefCpp(0x80128120, 0x8012812c, void, dAcPy_c *_this) {
    _this->mSubstate = 0;
    s_jumpAnmID[_this->mPlayerNo] = -1;
    s_jumpAnmRate[_this->mPlayerNo] = 0.5;
    _this->mAngle.x = 0;
    _this->mAngle.y = _this->getMukiAngle(_this->mDirection);
    _this->setWaterWalkFlag();
}

kmBranchDefCpp(0x80127cb8, 0x80127ce8, void, dAcPy_c *_this, int status_bit)
{
    _this->offStatus(status_bit);
    _this->mPyMdlMng.setAnm(6, 10.0, 0.0);
    s_jumpAnmID[_this->mPlayerNo] = 6; // New addition
}

extern "C" {

extern int DWORD_ARRAY_802f5108[3];

int dAcPy_c__getJump2AnimID(dAcPy_c *_this) {
    if (_this->isStatus(daPlBase_c::STATUS_KANI_JUMP)) {
        return 68;
    }
    if (_this->isStatus(daPlBase_c::STATUS_SIT_JUMP)) {
        return 119;
    }
    if (_this->mJumpCounter == 2) {
        return -1;
    }

    return DWORD_ARRAY_802f5108[_this->mJumpCounter];
}

int dAcPy_c__getJump3AnimID(dAcPy_c *_this) {
    if (_this->isStatus(daPlBase_c::STATUS_KANI_JUMP)) {
        return 84;
    }
    if (_this->isStatus(daPlBase_c::STATUS_SIT_JUMP)) {
        return 119;
    }
    if (_this->mJumpCounter == 2) {
        return -1;
    }

    return 158;
}

int dAcPy_c__getPhysicsAnimType(dAcPy_c *_this) {
    if ((_this->mNowBgCross1 & 1) == 0 && _this->isStar() == 0) {
        if (_this->mKey.buttonJump()) {
            return 0;
        } else {
            return 1;
        }
    }
    return 2;
}

void dAcPy_c__jumpStartWithPhysics(dAcPy_c *_this) {
    if (_this->isStatus(daPlBase_c::STATUS_STAR_JUMP)) {
        _this->mSubstate = 1;
    } else if (_this->isStatus(0x10)) {
        if (_this->mPyMdlMng.mpMdl->mAnm.isStop()) {
            _this->mPyMdlMng.setAnm(6, 3.0, 0.0);
            s_jumpAnmID[_this->mPlayerNo] = 6;
            _this->mSubstate = 1;
        }
    } else {
        if (_this->mSpeed.y < 0.0) {
            const int anmID = dAcPy_c__getJump2AnimID(_this);
            if (anmID != -1) {
                if (anmID == 68) {
                    _this->mPyMdlMng.setAnm(68, 10.0, 0.0);
                } else {
                    _this->mPyMdlMng.setAnm(anmID, 0.0);
                }

                s_jumpAnmID[_this->mPlayerNo] = anmID;
                _this->mSubstate = 1;
            }
        }
        if (dAcPy_c__getPhysicsAnimType(_this) == 0 &&
            (_this->mPyMdlMng.mpMdl->mAnm.isStop() || _this->mSpeed.y < 2.5)) {
            const int anmID = dAcPy_c__getJump3AnimID(_this);
            if (anmID != -1) {
                switch (anmID) {
                case 84:
                    _this->mPyMdlMng.setAnm(84, 3.0, 0.0); // Not 100% accurate, but perhaps good enough
                    break;
                default:
                    _this->mPyMdlMng.setAnm(anmID, 0.0); // Not 100% accurate, but perhaps good enough
                    break;
                case 158:
                    _this->mPyMdlMng.setAnm(158, 10.0, 0.0);
                }
                s_jumpAnmID[_this->mPlayerNo] = dAcPy_c__getJump2AnimID(_this);
                _this->mSubstate = 1;
            }
        }
    }
    _this->jumpExecAir();
}

void dAcPy_c__jumpExecAirWithPhysics(dAcPy_c *_this)
{
    if (_this->mJumpCounter == 2) {
        u8 dir = _this->mDirection;
        if (dir != _this->m_12f4 || (_this->mNowBgCross1 >> 1 & 1) != 0) {
            _this->m_12f4 = dir;
            _this->mPyMdlMng.setAnm(
                11,
                0.0,
                _this->mPyMdlMng.mpMdl->mAnm.mFrameMax - 1.0);
            s_jumpAnmID[_this->mPlayerNo] = 11;
            //_this->setForceNeutralJumpFall(false); // TODO: Figure out equivalent in NSMBW (May be unnecessary)
        }
        if (_this->mPyMdlMng.mpMdl->mAnm.isStop()) {
            //_this->setForceNeutralJumpFall(false); // TODO: Figure out equivalent in NSMBW (May be unnecessary)
        }
    }
    int anmID;
    switch (dAcPy_c__getPhysicsAnimType(_this)) {
        case 0:
            anmID = dAcPy_c__getJump3AnimID(_this);
            if (anmID != -1) {
                const float rate = dPyMdlMng_c::m_hio.mPyAnm.mAnm[anmID].mRate;
                float blendDuration;
                switch (anmID) {
                    case 84:
                        blendDuration = 3.0; // Not 100% accurate, but perhaps good enough
                        break;
                    default:
                        blendDuration = dPyMdlMng_c::m_hio.mPyAnm.mAnm[anmID].mBlendDuration;
                        break;
                    case 158:
                        blendDuration = 10.0;
                }
                sLib::chase(&(s_jumpAnmRate[_this->mPlayerNo]), 1.5, 0.02);
                _this->mPyMdlMng.setAnm(
                    anmID,
                    s_jumpAnmRate[_this->mPlayerNo] * rate,
                    blendDuration,
                    0.0);
            }

            if (_this->mPyMdlMng.mpMdl->mAnm.checkFrame(8.0) ||
                _this->mPyMdlMng.mpMdl->mAnm.checkFrame(19.0)) {
                _this->startSound(284, 0); // 284 -> SE_PLY_PRPL_FLY
            }
            break;
    case 1:
        if (s_jumpAnmID[_this->mPlayerNo] != -1) {
            _this->mPyMdlMng.setAnm(s_jumpAnmID[_this->mPlayerNo], 5.0, 0.0);
        } // Fall-through
    default:
        s_jumpAnmRate[_this->mPlayerNo] = 0.5;
        break;
    }

    _this->jumpExecAir();
}

}

kmWritePointer(0x80324e88, &dAcPy_c__jumpStartWithPhysics);
kmWritePointer(0x80324e94, &dAcPy_c__jumpExecAirWithPhysics);

#endif