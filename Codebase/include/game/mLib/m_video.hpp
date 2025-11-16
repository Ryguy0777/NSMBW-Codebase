#pragma once
#include <revolution/GX.h>

class mVideo {
public:
    GXRenderModeObj &mRenderModeObj;

    static float getSmth(float offs) { return (m_video->mRenderModeObj.efbHeight - offs) * 0.5f; }

    static float l_fbAspect;

    static u32 l_rayoutWidth;
    static u32 l_rayoutHeight;

    static float l_rayoutWidthF;
    static float l_rayoutHeightF;

    static float l_rayoutAspect;

    static mVideo *m_video;
};
