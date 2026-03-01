#include <cstdalign>
#include <cstddef>
#include <game/mLib/m_dvd.hpp>
#include <kamek.h>
#include <lib/revolution/dvd.h>
#include <propelparts/game_config.h>

namespace mDvd {
extern mDvd::TUncompressInfo_c<EGG::StreamDecompSZS> s_UncompressInfoSZS;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompLZ> s_UncompressInfoLZ;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompLH> s_UncompressInfoLH;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompLRC> s_UncompressInfoLRC;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompRL> s_UncompressInfoRL;

extern mDvd::UncompressInfo_c *const *s_UncompressInfoTable_begin;
extern mDvd::UncompressInfo_c *const *s_UncompressInfoTable_end;

extern bool isAutoStreamDecomp();
extern int ConvertPathToEntrynumASD_(const char *name, u8 *outType);
extern int ConvertPathToEntrynum(const char *path, u8 *outType);
} // namespace mDvd

namespace {
union {
#define BUFFER_OF(type, name) alignas(alignof(type)) u8 name[sizeof(type)];
    BUFFER_OF(EGG::StreamDecompSZS, m_SZS);
    BUFFER_OF(EGG::StreamDecompLZ, m_LZ);
#if defined(ENABLE_LH_COMPRESSION)
    BUFFER_OF(EGG::StreamDecompLH, m_LH);
#endif
#if defined(ENABLE_LRC_COMPRESSION)
    BUFFER_OF(EGG::StreamDecompLRC, m_LRC);
#endif
    BUFFER_OF(EGG::StreamDecompRL, m_RL);
#undef BUFFER_OF
} l_UnionObjectBufferData;

const mDvd::UncompressInfo_c *const l_AutoStreamDecompInfoArray[] = {
  &mDvd::s_UncompressInfoSZS,
#if defined(ENABLE_LH_COMPRESSION)
  &mDvd::s_UncompressInfoLH,
#endif
#if defined(ENABLE_LRC_COMPRESSION)
  &mDvd::s_UncompressInfoLRC,
#endif
  &mDvd::s_UncompressInfoRL, &mDvd::s_UncompressInfoLZ
};
} // namespace

kmWritePointer(&mDvd::UncompressInfo_c::m_UnionObjectBuffer, &l_UnionObjectBufferData);
kmWritePointer(&mDvd::s_UncompressInfoTable_begin, &l_AutoStreamDecompInfoArray[0]);
kmWritePointer(
  &mDvd::s_UncompressInfoTable_end,
  &l_AutoStreamDecompInfoArray
    [sizeof(l_AutoStreamDecompInfoArray) / sizeof(l_AutoStreamDecompInfoArray[0])]
);

kmBranchDefCpp(mDvd::initDefaultAutoStreamDecompInfo, 0, void) {
    // Prevent from overwriting our custom table with the default one IF this function is called
    // after our patches, which is not how it works currently
    return;
}

kmBranchDefCpp(mDvd::ConvertPathToEntrynum, 0, int, const char *path, u8 *outType) {
    // Change: Consider compressed files before uncompressed ones. This makes it easier to replace
    // uncompressed files with compressed ones, as Riivolution doesn't support unlinking files
    int num = -1;
    if (mDvd::isAutoStreamDecomp()) {
        num = mDvd::ConvertPathToEntrynumASD_(path, outType);
    }
    if (num == -1) {
        num = DVDConvertPathToEntrynum(path);
        if (num != -1 && outType != nullptr) {
            *outType = 0;
        }
    }
    return num;
}

// CXReadUncompLH: prevent returning -3. This appears to be some kind of "not enough data provided"
// error that EGG doesn't properly respond to
kmWriteNop(0x801D7884);

// CXReadUncompRL: Same story as above, except we need to manually load the typical return value
kmWrite32(0x801D6C50, 0x80630004); // lwz r3, 0x4(r3)
