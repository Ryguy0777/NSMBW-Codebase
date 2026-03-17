#include <kamek.h>
#include <game/bases/d_2d/resource.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_system.hpp>
#include <propelparts/bases/d_new_font_manager.hpp>

// Global font expansion hacks

// Global font list
const char *dNewFontMng_c::FONT_NAME_TBL[] = {
    "mj2d01_marioFont_64_IA4.brfnt",
    "mj2d00_PictureFont_32_RGBA8.brfnt",
    "mj2d00_MessageFont_32_I4.brfnt",
    "mj2d00_numberFont_I4A.brfnt",
    "propel00_PictureFont.brfnt",
};

#define FONT_NUM ARRAY_SIZE(dNewFontMng_c::FONT_NAME_TBL)

dNewFontMng_c *dNewFontMng_c::instance = nullptr;

dNewFontMng_c::dNewFontMng_c() {
    // Create dynamically-sized arrays
    mpFonts = new nw4r::ut::ResFont[FONT_NUM];
    mpFontLoaders = new dDvd::loader_c[FONT_NUM];
    mpFontDatas = new nw4r::ut::BinaryFileHeader*[FONT_NUM];

    for (int i = 0; i < FONT_NUM; i++) {
        mpFontDatas[i] = nullptr;
    }
}

int dNewFontMng_c::create(EGG::Heap *pHeap) {
    nw4r::ut::BinaryFileHeader *brfnt = nullptr;
    dNewFontMng_c *fontMng = dNewFontMng_c::instance;
    char resPath[48];
    char filename[100];

    for (int i = 0; i < FONT_NUM; i++) {
        memset(resPath, 0, sizeof(resPath));
        strcat(resPath, "Font/");
        strcat(resPath, FONT_NAME_TBL[i]);

        if (fontMng->mpFontDatas[i] == nullptr) {
            dGameCom::AreaLanguageFolder(resPath, filename);
            brfnt = (nw4r::ut::BinaryFileHeader*)fontMng->mpFontLoaders[i].request(filename, 0, pHeap);
            if (brfnt == nullptr) {
                return false;
            }

            fontMng->mpFontDatas[i] = brfnt;
            fontMng->mpFonts[i].SetResource(brfnt);
        }
    }
    return true;
}

u8 dNewFontMng_c::getResFontIndex(const char *name) {
    u8 index = 0;

    while (FONT_NAME_TBL[index & 0xFF] == nullptr || strcmp(name, FONT_NAME_TBL[index & 0xFF])) {
        index++;
        if (index == FONT_NUM) {
            return -1;
        }
    }
    return index;
}

nw4r::ut::BinaryFileHeader *dNewFontMng_c::getResFontData(const char *name) {
    u8 index = getResFontIndex(name);
    if (index < 0) {
        index = 0;
    }
    return dNewFontMng_c::instance->mpFontDatas[index & 0xFF];
}

nw4r::ut::ResFont *dNewFontMng_c::getFont(int index) {
    if (index < 0) {
        index = 0;
    }
    return &dNewFontMng_c::instance->mpFonts[index];
}

// dSystem::createFontManagerPhase()
// Initialize the new font manager instead of the original one
kmBranchDefCpp(0x800E51E0, NULL, bool) {
    if (dNewFontMng_c::instance == nullptr) {
        dNewFontMng_c::instance = new dNewFontMng_c;
    }

    if (dNewFontMng_c::instance != nullptr) {
        return dNewFontMng_c::create(dSystem::s_FontManagerHeap);
    }
    return false;
}

// dFontMng_c::getResFontData()
kmBranchDefCpp(0x800B12D0, NULL, nw4r::ut::BinaryFileHeader *, const char *name) {
    return dNewFontMng_c::getResFontData(name);
}

// dFontMng_c::getFont()
kmBranchDefCpp(0x800B1310, NULL, nw4r::ut::ResFont *, int index) {
    return dNewFontMng_c::getFont(index);
}

// dFontMng_c::getResFontIndex()
kmBranchDefCpp(0x800B1410, NULL, u8, const char *name) {
    return dNewFontMng_c::getResFontIndex(name);
}

// Restore internal font loading for layout archives

// d2d::ResAccMult_c::InternalAccessor::GetResource() has an override to grab a font
// from dFontMng_c if the requested resource is in the `font` folder, so we'll remove that
kmWriteNop(0x80006A54);
kmWriteNop(0x80006A58);
kmWriteNop(0x80006A5C);
kmWriteNop(0x80006A60);

// d2d::ResAccMult_c::InternalAccessor::GetFont()
// Allow internal fonts to be loaded in place of global fonts
//
// TODO: Reimplement FontRef behavior from ArcResourceAccessor::GetFont() 0x802B6880
// Set it up to fallback on global fonts if it can't find an internal font
kmBranchDefCpp(0x80006A70, NULL, nw4r::ut::Font *, d2d::ResAccMult_c *this_, const char *name) {
    int idx = dNewFontMng_c::getResFontIndex(name);
    if (idx < 0) {
        // Returning nullptr causes the internal font to load instead
        return nullptr;
    } else {
        return dNewFontMng_c::getFont(idx);
    }
}
