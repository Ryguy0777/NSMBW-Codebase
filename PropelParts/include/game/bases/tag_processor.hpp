#pragma once

#include <nw4r/lyt/lyt_pane.h>
#include <nw4r/ut/ut_TagProcessorBase.h>

class TagProcessor_c : public nw4r::ut::TagProcessorBase<wchar_t> {
public:
    ///@unofficial
    struct ScissorMask_s {
        nw4r::lyt::Pane *mScissorMask;
        nw4r::math::VEC2 mScissorSize;
        nw4r::lyt::DrawInfo *mDrawInfo;

        nw4r::math::VEC2 mPos;
        nw4r::math::VEC2 mSize;
        nw4r::math::VEC2 mClipSize;
    };

    wchar_t mTextBuf[16];
    u8 pad[0x18];
    ScissorMask_s mScissorMasks[3];
    u8 pad2[0x10];
    u8 mMsgFontID;

    void TextBufClear();
};