#include <lib/revolution/SI.h>

/* 0x803402D0 */
extern SIControl Si;

/* 0x8038FAD0 */
extern SIPacket Packet[4];

/* 0x803402E8 */
extern u32 Type[4];

/* 0x8038fc50 */
extern u32 InputBuffer[4][2];
/* 0x8038fc70 */
extern BOOL InputBufferValid[4];
/* 0x8038fc80 */
extern volatile u32 InputBufferVcount[4];

/* 0x8038fcb0 */
extern SITypeCallback TypeCallback[4][4];

static OSInterruptHandler RDSTHandler[4];

BOOL SIBusy(void) {
    return (Si.chan != -1) ? TRUE : FALSE;
}

BOOL SIIsChanBusy(s32 chan) {
    return Packet[chan].chan != -1 || Si.chan == chan;
}

static BOOL SIEnablePollingInterrupt(BOOL enable) {
    BOOL enabled;
    BOOL rc;
    u32 reg;
    int i;

    enabled = OSDisableInterrupts();
    reg = SI_HW_REGS[SI_COMCSR_IDX];
    rc = ((reg & SI_COMCSR_RDSTINTMSK_MASK) != 0) ? TRUE : FALSE;

    if (enable) {
        reg |= SI_COMCSR_RDSTINTMSK_MASK;
        
        for (i = 0; i < 4; i++) {
            InputBufferVcount[i] = 0;
        }
    } else {
        reg &= ~SI_COMCSR_RDSTINTMSK_MASK;
    }

    reg &= ~(SI_COMCSR_TCINT_MASK | SI_COMCSR_TSTART_MASK);
    SI_HW_REGS[SI_COMCSR_IDX] = reg;

    OSRestoreInterrupts(enabled);
    return rc;
}

BOOL SIUnregisterPollingHandler(OSInterruptHandler handler) {
    BOOL enabled;
    int i;

    enabled = OSDisableInterrupts();
    for (i = 0; i < 4; i++) {
        if (RDSTHandler[i] == handler) {
            RDSTHandler[i] = 0;
            
            for (i = 0; i < 4; i++) {
                if (RDSTHandler[i] != 0) {
                    break;
                }
            }

            if (i == 4) {
                SIEnablePollingInterrupt(FALSE);
            }

            OSRestoreInterrupts(enabled);
            return TRUE;
        }
    }    

    OSRestoreInterrupts(enabled);
    return FALSE;
}

u32 SIGetStatus(s32 chan) {
    BOOL enabled;
    u32 sr;
    int chanShift;

    enabled = OSDisableInterrupts();
    sr = SI_HW_REGS[SI_STATUS_IDX];
    chanShift = (3 - chan) * 8;
    sr >>= chanShift;

    if ((sr & 8) != 0) {
        if ((Type[chan] & SI_ERROR_BUSY) == 0) {
            Type[chan] = 8;
        }
    }

    OSRestoreInterrupts(enabled);
    return sr;
}

void SISetCommand(s32 chan, u32 command) {
    SI_HW_REGS[chan* 3] = command;
}

void SITransferCommands(void) {
    SI_HW_REGS[SI_STATUS_IDX] = SI_COMCSR_TCINT_MASK;
}

u32 SIEnablePolling(u32 poll) {
    BOOL enabled;
    u32 en;

    if (poll == 0) {
        return Si.poll;
    }
    
    enabled = OSDisableInterrupts();
    poll = poll >> 24;
    en = poll & 0xF0;
    poll &= ((en >> 4) | 0x03FFFFF0);
    poll &= 0xFC0000FF;
    
    Si.poll &= ~(en >> 4);
    Si.poll |= poll;
    poll = Si.poll;
    SITransferCommands();
    SI_HW_REGS[0x30 / 4] = poll;
    OSRestoreInterrupts(enabled);
    return poll;
}

u32 SIDisablePolling(u32 poll) {
    BOOL enabled;

    if (poll == 0) {
        return Si.poll;
    }

    enabled = OSDisableInterrupts();
    poll = poll >> 24;
    poll &= 0xF0;
    poll = Si.poll & ~poll;
    SI_HW_REGS[0x30 / 4] = poll;
    Si.poll = poll;
    OSRestoreInterrupts(enabled);
    return poll;
}

static BOOL SIGetResponseRaw(s32 chan) {
    u32 sr;

    sr = SIGetStatus(chan);
    if (sr & 0x20) {
        InputBuffer[chan][0] = SI_HW_REGS[1 + chan * 3];
        InputBuffer[chan][1] = SI_HW_REGS[2 + chan * 3];
        InputBufferValid[chan] = TRUE;
        return TRUE;
    }

    return FALSE;
}

bool SIGetResponse(s32 chan, void* data) {
    BOOL rc;
    BOOL enabled;

    enabled = OSDisableInterrupts();
    SIGetResponseRaw(chan);
    rc = InputBufferValid[chan];
    InputBufferValid[chan] = FALSE;
    
    if (rc) {
        ((u32*)data)[0] = InputBuffer[chan][0];
        ((u32*)data)[1] = InputBuffer[chan][1];
    }

    OSRestoreInterrupts(enabled);
    return rc;
}

u32 SIGetTypeAsync(s32 chan, SITypeCallback callback) {
    BOOL enabled;
    u32 type;
    int i;

    enabled = OSDisableInterrupts();
    type = SIGetType(chan);

    if ((Type[chan] & SI_ERROR_BUSY) != 0) {
        for (i = 0; i < SI_MAX_TYPE; i++) {
            if (TypeCallback[chan][i] == callback) {
                break;
            }

            if (TypeCallback[chan][i] == 0) {
                TypeCallback[chan][i] = callback;
                break;
            }
        }

    } else {
        (*callback)(chan, type);
    }

    OSRestoreInterrupts(enabled);
    return type;
}

u32 SIDecodeType(u32 type) {
    u32 error;

    error = type & 0xFF;
    type &= ~0xFF;

    if (error & SI_ERROR_NO_RESPONSE) {
        return SI_ERROR_NO_RESPONSE;
    }

    if (error & (SI_ERROR_UNKNOWN | SI_ERROR_COLLISION | SI_ERROR_OVER_RUN | SI_ERROR_UNDER_RUN)) {
        return SI_ERROR_UNKNOWN;
    }

    if (error != 0) {
        return SI_ERROR_BUSY;
    }

    if ((type & SI_TYPE_MASK) == SI_TYPE_N64) {
        switch (type & 0xFFFF0000) {
        case SI_N64_MIC:
        case SI_N64_KEYBOARD:
        case SI_GBA:
        case SI_N64_MOUSE:
        case SI_N64_CONTROLLER:
            return type & 0xFFFF0000;
        default:
            return SI_ERROR_UNKNOWN;
        }
    }

    if ((type & SI_TYPE_MASK) != SI_TYPE_DOLPHIN) {
        return SI_ERROR_UNKNOWN;
    }

    switch (type & 0xFFFF0000) {
    case SI_GC_STEERING:
    case SI_GC_CONTROLLER:
        return type & 0xFFFF0000;
    }

    if ((type & 0xFFE00000) == SI_GC_KEYBOARD) {
        return SI_GC_KEYBOARD;
    }

    if ((type & SI_GC_WIRELESS) != 0 && (type & SI_WIRELESS_IR) == 0) {
        if ((type & SI_GC_WAVEBIRD) == SI_GC_WAVEBIRD) {
            return SI_GC_WAVEBIRD;
        }

        if ((type & SI_WIRELESS_STATE) == 0) {
            return SI_GC_RECEIVER;
        }
    }

    if ((type & SI_GC_CONTROLLER) == SI_GC_CONTROLLER) {
        return SI_GC_CONTROLLER;
    }

    return SI_ERROR_UNKNOWN;
}