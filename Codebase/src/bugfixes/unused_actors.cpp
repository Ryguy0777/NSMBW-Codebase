#include <kamek.h>

// assign unused (but otherwise functional) actors to their original sprite ids

// "Mini Goomba" (actor 53, sprite 22)
kmWrite16(0x8030a6b0, 0x0035);

// "Jumbo Ray Respawner 2" (actor 539, sprite 273)
kmWrite16(0x8030cde8, 0x020E);
