#ifndef NW4R_DB_EXCEPTION_H
#define NW4R_DB_EXCEPTION_H

#include <revolution/OS/OSContext.h>

namespace nw4r {
namespace db {

void Exception_Printf_(const char* format, ...);
void ShowMainInfo_(u16 error, const OSContext* context, ulong dsisr, ulong dar);

// I hate this
void* sException();

} // namespace db
} // namespace nw4r

#endif