#include <kamek.h>

// Always allow skipping cutscenes with plus

kmWriteNop(0x80922db0);
kmWriteNop(0x80922dd0);