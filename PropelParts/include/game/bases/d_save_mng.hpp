#pragma once
#include <game/bases/d_mj2d_data.hpp>

class dSaveMng_c {
public:
    dMj2dGame_c* getSaveGame(s8 slot);
    void initLoadGame(s8 slot);

    inline u8 getLastSelectedFile() {return mSave.mLastSelectedFile;}

    bool mIsNowSave; // set to true when accessing NAND
    u8 mPad[31];
    dMj2dHeader_c mSave;

    static bool isNandBusy();
    static dSaveMng_c* m_instance;
};