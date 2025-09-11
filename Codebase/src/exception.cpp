#include <kamek.h>
#include <revolution/OS/OSContext.h>
#include <game_versions_nsmbw.h>

// replace the exception handler with our own version
// designed with mod debugging in mind

namespace nw4r { namespace db {
    void Exception_Printf_(const char *msg, ...);
    // I hate this
	void* sException();
}}

#define GAME_NAME "NSMBW - Custom Codebase"

const bool gprFun = false;

#ifdef IS_GAME_VERSION_DYNAMIC
#error Dynamic compilation is unsupported for this patch.
#endif

#if GAME_VERSION == GAME_VERSION_P1
#define VERSION_NAME "PALv1"
#elif GAME_VERSION == GAME_VERSION_E1
#define VERSION_NAME "USAv1"
#elif GAME_VERSION == GAME_VERSION_J1
#define VERSION_NAME "JPNv1"
#elif GAME_VERSION == GAME_VERSION_P2
#define VERSION_NAME "PALv2"
#elif GAME_VERSION == GAME_VERSION_E2
#define VERSION_NAME "USAv2"
#elif GAME_VERSION == GAME_VERSION_J2
#define VERSION_NAME "JPNv2"
#elif GAME_VERSION == GAME_VERSION_K
#define VERSION_NAME "KOR"
#elif GAME_VERSION == GAME_VERSION_W
#define VERSION_NAME "TWN"
#elif GAME_VERSION == GAME_VERSION_C
#define VERSION_NAME "CHN"
#else
#define VERSION_NAME "UNK"
#endif

const char *errDesc[] = {
    "SYSTEM RESET",
    "MACHINE CHECK",
    "DSI", "ISI",
    "EXTERNAL INTERRUPT",
    "ALIGNMENT",
    "PROGRAM",
    "FLOATING POINT",
    "DECREMENTER",
    "SYSTEM CALL",
    "TRACE",
    "PERFORMANCE MONITOR",
    "BREAKPOINT",
    "SYSTEM INTERRUPT",
    "THERMAL INTERRUPT",
    "PROTECTION",
    "FPE"
};

// write our new exception info
kmBranchDefCpp(0x80234CA0, NULL, void, u16 OSError, OSContext *osContext, u32 dsisr, u32 dar) {
    nw4r::db::Exception_Printf_("Whoops! " GAME_NAME " has crashed - %s\n\nPlease send the information below to\nthe github repo\nYou can scroll through this report using the D-Pad.\n" VERSION_NAME "\n", errDesc[OSError]);
    nw4r::db::Exception_Printf_("SRR0: %08X | DSISR: %08X | DAR: %08X\n", osContext->srr0, dsisr, dar);

    if (gprFun) {
        int i = 0;
        do {
            nw4r::db::Exception_Printf_("R%02d:%08X  R%02d:%08X  R%02d:%08X\n", i, osContext->gprs[i], i + 0xB, osContext->gprs[i + 0xB], i + 0x16, osContext->gprs[i + 0x16]);
            i++;
        } while (i < 10);
        nw4r::db::Exception_Printf_("R%02d:%08X  R%02d:%08X\n\n", 10, osContext->gprs[10], 0x15, osContext->gprs[0x15]);
    }

    //Stack trace
    //nw4r::db::Exception_Printf_("\nException info trace (most recent on top):\n");
    int i = 0;
    u32 *stackPointer = (u32 *)((char *)nw4r::db::sException + 0x33C);
    do {
        if (stackPointer == 0 || (u32)stackPointer == 0xFFFFFFFF)
            break;

        nw4r::db::Exception_Printf_("%08X", stackPointer[1]);
        /* if (stackPointer[1] >= dlcode) {
            nw4r::db::Exception_Printf_(" - %08X NewerASM", stackPointer[1] - dlcode);
        } */
        nw4r::db::Exception_Printf_("\n");

        i++;
        stackPointer = (u32 *)*stackPointer;
    } while (i < 0x10);
}

// disable the button sequence
kmWrite32(0x802D7528, 0x48000060);

// disable unnecessary debug info
kmWrite8(0x800E4E8F, 0x21);