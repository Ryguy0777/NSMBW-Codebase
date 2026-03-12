#include <kamek.h>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/mLib/m_heap.hpp>
#include <propelparts/bases/d_heap_bar_render.hpp>
#include <propelparts/bases/d_debug_config.hpp>

// Render heap bars when drawing mFader_c
kmBranchDefCpp(0x800B0B40, NULL, void) {
    dHeapBarRender_c::m_instance.draw();
}

extern "C" int MEMGetTotalFreeSizeForExpHeap(MEMiHeapHead *);

#define END_AT 80.0f
#define BAR_HEIGHT 16.0f

// Drawing helper function
void DrawHeapBarQuad(float left, float right, float top, float bottom, float z, u32 color) {
    // TL
    GXPosition3f32(left, top, z);
    GXColor1u32(color);

    // TR
    GXPosition3f32(right, top, z);
    GXColor1u32(color);

    // BR
    GXPosition3f32(right, bottom, z);
    GXColor1u32(color);

    // BL
    GXPosition3f32(left, bottom, z);
    GXColor1u32(color);
}

dHeapBarRender_c dHeapBarRender_c::m_instance;

// Heap array
const EGG::Heap *dHeapBarRender_c::sc_heapList[] = {
    nullptr,
    mHeap::g_gameHeaps[1],
    mHeap::g_gameHeaps[2],
    mHeap::g_archiveHeap
};

// Y offsets for each heap bar
const float dHeapBarRender_c::sc_heapPosY[] = {
    -1.0f,
    0.0f,
    16.0f,
    32.0f
};

// Heap bar colors, 0 is unused
const u32 dHeapBarRender_c::sc_heapColors[] = {
    0x00000000,
    0x0000FFFF,
    0xFF0000FF,
    0xFFFFFFFF,
};

// dActor_c::l_gameStop flag colors
const u32 dHeapBarRender_c::sc_stopColors[] = {
    0x00FF00FF,
    0xFFFF00FF,
    0x00FFFFFF,
    0xFF00FFFF
};

void dHeapBarRender_c::draw() {
    if (!dDebugConfig_c::m_instance->mDrawHeapBars) {
        return;
    }

    setupGX();

    // Draw heaps
    for (int i = 0; i < 4; i++) {
        if (sc_heapList[i] == nullptr) {
            continue;
        }

        MEMiHeapHead *pHandle = sc_heapList[i]->mHeapHandle;
        int freeSize = MEMGetTotalFreeSizeForExpHeap(pHandle);
        int heapSize = pHandle->end - pHandle->start;
        const char *heapName = sc_heapList[i]->mpName;

        drawHeapBar(freeSize, heapSize, i, heapName);
    }

    // Draw game stop labels
    GXBegin(GX_QUADS, GX_VTXFMT0, 16);
    for (int i = 0; i < 4; i++) {
        float left = BAR_HEIGHT * i;
        float right = (BAR_HEIGHT * i) + BAR_HEIGHT;
        float top = END_AT;
        float bottom = END_AT + BAR_HEIGHT;
        u32 color = sc_stopColors[i];

        DrawHeapBarQuad(left, right, top, bottom, 0.0f, color);
    }
    GXEnd();

    // Draw game stop flags
    for (int i = 0; i < 4; i++) {
        if (dGameCom::l_gameStop & (1 << i)) {
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);

            float left = BAR_HEIGHT * i;
            float right = (BAR_HEIGHT * i) + BAR_HEIGHT;
            float top = END_AT + 16.0f;
            float bottom = END_AT + 24.0f;
            u32 color = 0xFFFFFFFF;

            DrawHeapBarQuad(left, right, top, bottom, 0.0f, color);
            GXEnd();
        }
    }

    // Render dActor_c::mExecStop flags
    for (int i = 0; i < 4; i++) {
        if (dActor_c::mExecStop & (1 << i)) {
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);

            float left = BAR_HEIGHT * i;
            float right = (BAR_HEIGHT * i) + BAR_HEIGHT;
            float top = END_AT + 32.0f;
            float bottom = END_AT + 40.0f;

            u8 clrRGB = 0x20 * (i + 1);
            u32 color = (clrRGB << 24) | (clrRGB << 16) | (clrRGB << 8) | clrRGB;
            
            DrawHeapBarQuad(left, right, top, bottom, 0.0f, color);
            GXEnd();
        }
    }
}

void dHeapBarRender_c::setupGX() {
    nw4r::math::MTX44 ortho;
    
    C_MTXOrtho(ortho, 0.0f, 456.0f, 0.0f, 686.0f, 0.0f, 1.0f);
    GXSetProjection(ortho, GX_ORTHOGRAPHIC);
    
    Mtx identity;
    PSMTXIdentity(identity);
    
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevDirect(GX_TEVSTAGE0);
    
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBX8, 0);
    
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_NOOP);
}

void dHeapBarRender_c::drawHeapBar(int freeSize, int heapSize, int index, const char *name) {
    //OSReport("%X,%X,%d,%s\n", freeSize, heapSize, index, name);

    // Get info
    float posY = sc_heapPosY[index];
    u32 color = sc_heapColors[index];

    // Get our bounds
    float left = 0;
    float right = ((heapSize - freeSize) * 686.0f) / heapSize;
    float top = posY;
    float bottom = posY + BAR_HEIGHT;

    // Draw them
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    DrawHeapBarQuad(left, right, top, bottom, 0.0f, color);
    GXEnd();
}
