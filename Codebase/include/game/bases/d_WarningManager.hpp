#pragma once

#include <game/bases/d_base.hpp>

class dWarningManager_c : public dBase_c {
public:
    static bool isWarningCheck(); ///< @unofficial
    u8 pad[0xAE0];
    int mCurrentSaveError;
    u8 pad2[0x3B];
    bool mIsSaveError;

    static dWarningManager_c *m_instance;
    static bool m_WarningCheck;
    static bool m_Created;
    static int m_WarningForbid;
};
