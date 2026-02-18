#include <kamek.h>
#include <new/bases/d_msgbox_window.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_pause_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/snd/snd_audio_mgr.hpp>
#include <game/bases/d_game_key.hpp>
#include <constants/sound_list.h>
#include <new/constants/message_list.h>
#include <game/bases/d_s_stage.hpp>

dMsgBoxWindow_c *dMsgBoxWindow_c::m_instance = nullptr;
bool dMsgBoxWindow_c::m_isShowing = false;

CUSTOM_BASE_PROFILE(MSGBOX_WINDOW, dMsgBoxWindow_c, fProfile::MESSAGE_WINDOW, fProfile::DRAW_ORDER::MESSAGE_WINDOW);

STATE_DEFINE(dMsgBoxWindow_c, Wait);
STATE_DEFINE(dMsgBoxWindow_c, BoxAppearWait);
STATE_DEFINE(dMsgBoxWindow_c, ShownWait);
STATE_DEFINE(dMsgBoxWindow_c, BoxDisappearWait);

dCustomProfile_c l_MSGBOX_WINDOW_profile(&g_profile_MSGBOX_WINDOW, "MSGBOX_WINDOW", fProfile::MSGBOX_WINDOW);

dMsgBoxWindow_c::dMsgBoxWindow_c() : mIsCreated(false), mStateMgr(*this, StateID_Wait) {}

dMsgBoxWindow_c::~dMsgBoxWindow_c() {
    m_instance = nullptr;
}

int dMsgBoxWindow_c::create() {
    static const char *AnmNameTbl[] = {
        "messageBox_00_BoxAppear.brlan",
        "messageBox_00_BoxDisappear.brlan",
    };

    static const int ANIME_INDEX_TBL[] = {0, 1};
    static const char *GROUP_NAME_DT[] = {
        "G_Box",
        "G_Box",
    };

    static const char *T_PANE_NAME_TBL[] = {
        "T_title_00",
        "T_msg_00",
        "T_msgS_00",
    };

    static const char *P_PANE_NAME_TBL[] = {
        "P_button_00",
    };

    static const char *W_PANE_NAME_TBL[] = {
        "W_frame_00",
    };
    
    bool res = mLayout.ReadResource("messageBox.arc", false);
    if (!res) {
        return NOT_READY;
    }

    // Setup layout
    mLayout.build("messageBox.brlyt", nullptr);
    mLayout.AnimeResRegister(AnmNameTbl, ARRAY_SIZE(AnmNameTbl));
    mLayout.GroupRegister(GROUP_NAME_DT, ANIME_INDEX_TBL, ARRAY_SIZE(GROUP_NAME_DT));
    mLayout.TPaneRegister(T_PANE_NAME_TBL, mpTextBoxes, ARRAY_SIZE(T_PANE_NAME_TBL));
    mLayout.PPaneRegister(P_PANE_NAME_TBL, mpPicturePanes, ARRAY_SIZE(P_PANE_NAME_TBL));

    mLayout.AnimePlay();
    mLayout.calc();
    mIsCreated = true;
    mVisible = false;
    mCanCancel = true;
    mDelay = -1;

    return SUCCEEDED;
}

int dMsgBoxWindow_c::execute() {
    mStateMgr.executeState();
    if (mVisible) {
        mLayout.AnimePlay();
        mLayout.calc();
    }
    return SUCCEEDED;
}

int dMsgBoxWindow_c::draw() {
    if (mVisible) {
        mLayout.entry();
    }
    return SUCCEEDED;
}

int dMsgBoxWindow_c::doDelete() {
    return mLayout.doDelete();
}

void dMsgBoxWindow_c::initializeState_Wait() {}

void dMsgBoxWindow_c::finalizeState_Wait() {}

void dMsgBoxWindow_c::executeState_Wait() {}

void dMsgBoxWindow_c::initializeState_BoxAppearWait() {
    mVisible = true;
    m_isShowing = true;
    PauseManager_c::m_instance->mDisablePause = 1; // Enable no-pause
    mLayout.AnimeStartSetup(ANIM_BOX_APPEAR, false);

    SndAudioMgr::sInstance->startSystemSe(SE_SYS_KO_DIALOGUE_IN, 1);
}

void dMsgBoxWindow_c::finalizeState_BoxAppearWait() {}

void dMsgBoxWindow_c::executeState_BoxAppearWait() {
    if (!mLayout.isAnime(ANIM_BOX_APPEAR)) {
        mStateMgr.changeState(StateID_ShownWait);
    }
}

void dMsgBoxWindow_c::initializeState_ShownWait() {}

void dMsgBoxWindow_c::finalizeState_ShownWait() {}

void dMsgBoxWindow_c::executeState_ShownWait() {
    if (mCanCancel) {
        int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
        if (pressed & (WPAD_BUTTON_2 | WPAD_BUTTON_A)) {
            mStateMgr.changeState(StateID_BoxDisappearWait);
        }
    }

    if (mDelay > 0) {
        mDelay--;
        if (mDelay == 0) {
            mStateMgr.changeState(StateID_BoxDisappearWait);
        }
    }
}

void dMsgBoxWindow_c::initializeState_BoxDisappearWait() {
	mLayout.AnimeStartSetup(ANIM_BOX_DISAPPEAR, false);

	SndAudioMgr::sInstance->startSystemSe(SE_SYS_DIALOGUE_OUT_AUTO, 1);
}

void dMsgBoxWindow_c::finalizeState_BoxDisappearWait() {
	mVisible = false;
    m_isShowing = false;
	if (mCanCancel && PauseManager_c::m_instance) {
		PauseManager_c::m_instance->mDisablePause = 0; // Disable no-pause
    }
    if (mLeaveLevel) {
        dScStage_c::setNextScene(fProfile::WORLD_MAP, 0x80000000, (dScStage_c::Exit_e)0, (dFader_c::fader_type_e)4);
    }
}

void dMsgBoxWindow_c::executeState_BoxDisappearWait() {
	if (!mLayout.isAnime(ANIM_BOX_DISAPPEAR)) {
		mStateMgr.changeState(StateID_Wait);

		for (int i = 0; i < 2; i++) {
			mLayout.ReverseAnimeStartSetup(i, false);
        }
		mLayout.AllAnimeEndSetup();
	}
}

void dMsgBoxWindow_c::showMessage(unsigned long id, bool canCancel, int delay, bool leavesLevel) {
    mCanCancel = canCancel;
    mDelay = delay;
    mLeaveLevel = leavesLevel;

    MsgRes_c *msgRes = dMessage_c::getMesRes();

    mpTextBoxes[T_title_00]->setMessage(msgRes, BMG_CATEGORY_MESSAGE_BOX_TITLE, id, 0);
    mpTextBoxes[T_msg_00]->setMessage(msgRes, BMG_CATEGORY_MESSAGE_BOX_TEXT, id, 0);
    mpTextBoxes[T_msgS_00]->setMessage(msgRes, BMG_CATEGORY_MESSAGE_BOX_TEXT, id, 0);

    mpPicturePanes[P_button_00]->SetVisible(mCanCancel);

    mStateMgr.changeState(StateID_BoxAppearWait);
}

// Patch dGameCom::isGameStop to add a check for the message box
kmBranchDefCpp(0x800b3b50, NULL, bool, ulong stopType) {
    if (dMsgBoxWindow_c::m_isShowing) {
        return true;
    } else {
        return dGameCom::l_gameStop & stopType;
    }
}