#include <cstdalign>
#include <cstddef>
#include <game/mLib/m_dvd.hpp>
#include <iterator>
#include <kamek.h>
#include <lib/revolution/dvd.h>
#include <propelparts/game_config.h>

namespace mDvd {
extern mDvd::TUncompressInfo_c<EGG::StreamDecompSZS> s_UncompressInfoSZS;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompLZ> s_UncompressInfoLZ;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompLH> s_UncompressInfoLH;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompLRC> s_UncompressInfoLRC;
extern mDvd::TUncompressInfo_c<EGG::StreamDecompRL> s_UncompressInfoRL;

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

kmBranchDefCpp(mDvd::initDefaultAutoStreamDecompInfo, 0, void) {
    mDvd::initAutoStreamDecompInfo(
      l_AutoStreamDecompInfoArray,
      l_AutoStreamDecompInfoArray + std::size(l_AutoStreamDecompInfoArray)
    );
}

kmBranchDefCpp(mDvd::ConvertPathToEntrynum, 0, int, const char *path, u8 *outType) {
    // Change: Consider compressed files before uncompressed ones. This makes it easier to replace
    // uncompressed files with compressed ones, as Riivolution doesn't support unlinking files.
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
