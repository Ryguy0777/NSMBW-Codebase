#pragma once

#include <game/bases/d_game_key.hpp>
#include <lib/egg/core/eggHeap.h>
#include <game/mLib/m_pad.h>
#include <game/sLib/s_FStateFct.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>
#include <game/sLib/s_StateMgr.hpp>

#define REMOCON_CONNECT_COUNT 4

class dRemoconMng_c {
public:
    static const int CONNECT_COUNT = REMOCON_CONNECT_COUNT;

    class dConnect_c {
    public:
        class dExtension_c {
        public:
            void shutdown();
            void execute();

            STATE_FUNC_DECLARE(dExtension_c, Freestyle);
            STATE_FUNC_DECLARE(dExtension_c, None);
            STATE_FUNC_DECLARE(dExtension_c, Other);
            STATE_FUNC_DECLARE(dExtension_c, Wait);

            // new states
            STATE_FUNC_DECLARE(dExtension_c, Classic);
            STATE_FUNC_DECLARE(dExtension_c, Gamecube);

            u32 m_00; // vtable(?)
            mPad::CH_e mChannel;
            u32 mControllerType;
            sStateMgr_c<dExtension_c, sStateMethodUsr_FI_c, sFStateFct_c, sStateIDChk_c> mStateMgr;
        };

        dConnect_c(mPad::CH_e channel);

        void execute();
        void onRumbleEnable();

        STATE_FUNC_DECLARE(dConnect_c, Shutdown);
        STATE_FUNC_DECLARE(dConnect_c, Sleep);

        void *m_00; // vtable
        mPad::CH_e mChannel;
        dExtension_c mExtension;
        u8 mPad[0x50-0x9];
        int mBattery;
        u8 m_54;
        int m_58;
        sStateMgr_c<dConnect_c, sStateMethodUsr_FI_c, sFStateFct_c, sStateIDChk_c> mStateMgr;

        static bool m_isBoot;
    };

    dRemoconMng_c();

public:
    void *m_00; // vtable
    dConnect_c* mpaConnect[CONNECT_COUNT];

    static void execute();
    static void recreate(EGG::Heap* heap);

    static dRemoconMng_c* m_instance;
};