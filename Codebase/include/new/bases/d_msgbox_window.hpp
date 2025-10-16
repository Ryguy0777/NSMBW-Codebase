#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

// message box window

#define BMG_CATEGORY_MESSAGE_BOX_TITLE 0x400
#define BMG_CATEGORY_MESSAGE_BOX_TEXT  0x401

class dMsgBoxWindow_c : public dBase_c {
public:
    enum ANIMS_e {
        ANIM_BOX_APPEAR,
        ANIM_BOX_DISAPPEAR
    };

    dMsgBoxWindow_c();
    virtual ~dMsgBoxWindow_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    void showMessage(unsigned long id, bool canCancel=true, int delay=-1);
    
    sFStateMgr_c<dMsgBoxWindow_c, sStateMethodUsr_FI_c> mStateMgr;

    STATE_FUNC_DECLARE(dMsgBoxWindow_c, Wait);
    STATE_FUNC_DECLARE(dMsgBoxWindow_c, BoxAppearWait);
    STATE_FUNC_DECLARE(dMsgBoxWindow_c, ShownWait);
    STATE_FUNC_DECLARE(dMsgBoxWindow_c, BoxDisappearWait);

    LytBase_c mLayout;
    bool mIsCreated;
    bool mVisible;
    bool mCanCancel;
    int mDelay;

    LytTextBox_c *T_title_00,
                 *T_msg_00,
                 *T_msgS_00;

    nw4r::lyt::Picture *P_button_00;

    nw4r::lyt::Window *W_frame_00;

    static dMsgBoxWindow_c *m_instance;
    static bool m_isShowing;
};