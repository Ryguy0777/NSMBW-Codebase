#ifndef RVL_SDK_PAD_H
#define RVL_SDK_PAD_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

extern u32 __PADSpec;

// https://github.com/devkitPro/libogc/blob/bc4b778d558915aa40676e33514c4c9ba2af66b8/gc/ogc/pad.h#L46
// Size: 0x0B (arrays padded to 0x0C)
typedef struct PADStatus {
  // 0x00
  u16 button;
  // 0x02
  s8 stickX;
  // 0x03
  s8 stickY;
  // 0x04
  s8 substickX;
  // 0x05
  s8 substickY;
  // 0x06
  u8 triggerL;
  // 0x07
  u8 triggerR;
  // 0x08
  u8 analogA;
  // 0x09
  u8 analogB;
  // 0x0A
  s8 err;
} PADStatus;

typedef enum {
    PAD_FLAG_NO_RECALIBRATE = (1 << 6),
} PADFlag;

BOOL __PADDisableRecalibration(BOOL disable);

#ifdef __cplusplus
}
#endif
#endif