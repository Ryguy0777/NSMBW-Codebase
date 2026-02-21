#include <new/bases/d_debug_text_writer.hpp>
#include <game/bases/d_font_manager.hpp>

dDebugTextWriter_c::dDebugTextWriter_c() {
    SetFont(*dFontMng_c::getFont(0));
    mIsInBegin = false;
}

void dDebugTextWriter_c::Reset() {
    SetAlpha(255);

    EnableFixedWidth(false);
    SetFixedWidth(0.0f);

    SetColorMapping(nw4r::ut::Color::ELEMENT_MIN, nw4r::ut::Color::ELEMENT_MAX);
    SetGradationMode(GRADMODE_NONE);
    SetTextColor(nw4r::ut::Color::ELEMENT_MAX);

    SetScale(1.0f, 1.0f);
    SetCursor(0.0f, 0.0f);

    EnableLinearFilter(true, true);
    SetTabWidth(4);
    SetCharSpace(0.0f);
    SetLineSpace(0.0f);
    SetDrawFlag(0);
}

void dDebugTextWriter_c::Print(f32 x, f32 y, const char* msg, ...) {
    std::va_list list;
    va_start(list, msg);

    SetCursor(x, y);
    VPrintf(msg, list);

    va_end(list);
}

void dDebugTextWriter_c::Begin() {
    mIsInBegin = true;
    SetupGX();

    // Always pass the Z test (draw over everything)
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

    nw4r::math::MTX34 posMtx;
    nw4r::math::MTX34Identity(&posMtx);

    GXLoadPosMtxImm(posMtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

void dDebugTextWriter_c::End() {
    mIsInBegin = false;
}