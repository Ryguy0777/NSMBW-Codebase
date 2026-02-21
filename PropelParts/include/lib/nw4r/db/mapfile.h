#ifndef NW4R_DB_MAPFILE_H
#define NW4R_DB_MAPFILE_H

#include <nw4r/types_nw4r.h>

namespace nw4r {
namespace db {

// Pointer to data read out function
extern u8 (*GetCharPtr_)(const u8* buf);

// Actual function
static u8 GetCharOnMem_(const u8* buf);

// Convert hex string to u32
static u32 XStrToU32_(const u8* str);

} // namespace db
} // namespace nw4r

#endif