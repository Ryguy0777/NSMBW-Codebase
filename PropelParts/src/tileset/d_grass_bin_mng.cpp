#include <kamek.h>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_res_mng.hpp>
#include <propelparts/bases/d_grass_bin_mng.hpp>
#include <game/bases/d_s_stage.hpp>

// Externs for ASM calls
extern "C" {
bool AreCustomFlowersLoaded();
int AddFlowerEntry(dBg_c *bg, u16 tileNum, u32 x, u32 y);
void *LoadCustomFlowerBrres();
void *LoadCustomGrassBrres(dRes_c *res, char *originalArc, char *originalName, int style);
}

// Static instance (and a pointer to it for ASM because the compiler is garbage)
dGrassBinMng_c *dGrassBinMng_c::m_instance;
extern "C" {dGrassBinMng_c **instancePtr = &dGrassBinMng_c::m_instance;}

// Build function
dGrassBinMng_c *dGrassBinMng_c::build(GrassBin_s *rawData, u8 slot) {

    // If an instance is already initialized, return it
    if (dGrassBinMng_c::m_instance != NULL) {
        return dGrassBinMng_c::m_instance;
    }

    // Create class and return it
    return new dGrassBinMng_c(rawData, slot);
}

// Constructor
dGrassBinMng_c::dGrassBinMng_c(GrassBin_s *rawData, u8 slot) {

    // Set variables
    mpData = rawData;
    mTileSlot = slot;

    // Set static instance
    dGrassBinMng_c::m_instance = this;
}

// Destructor
dGrassBinMng_c::~dGrassBinMng_c() {

    // Reset static instance
    dGrassBinMng_c::m_instance = NULL;
}

// Get flower data from tile
GrassBinEntry_s *dGrassBinMng_c::getFlowerData(u16 tileNum) {

    // Traverse through all entries
    for (int i = 0; i < mpData->mNumEntries; i++) {

        // Get entry
        GrassBinEntry_s *current = &mpData->mEntries[i];

        // Check if tile number matches, and if so return this entry
        if (tileNum == (current->mTileNum | 0x100 * mTileSlot)) {
            return current;
        }
    }

    // None found, return NULL
    return NULL;
}

// Helper function
bool AreCustomFlowersLoaded() {
    return (dGrassBinMng_c::m_instance != NULL);
}

int AddFlowerEntry(dBg_c *bg, u16 tileNum, u32 x, u32 y) {

    // Get the flower data for this tile
    GrassBinEntry_s *data = dGrassBinMng_c::m_instance->getFlowerData(tileNum);

    // If none is found, it's just a regular tile, ignore it
    if (data == NULL) {
        return tileNum;
    }

    // Set up flower if it was enabled and we can allocate one
    if (data->mFlowerValue > 0 && bg->mFlowerCount < 99) {
        dBgAnimObj_c *flower = &bg->mFlowers[bg->mFlowerCount];
        flower->mX = float(x);
        flower->mY = -float(y);
        flower->mType = data->mFlowerValue - 1;
        bg->mFlowerCount++;
    }

    // Set up grass if it was enabled and we can allocate one
    if (data->mGrassValue > 0 && bg->mGrassCount < 99) {
        dBgAnimObj_c *grass = &bg->mGrass[bg->mGrassCount];
        grass->mX = float(x);
        grass->mY = -float(y);
        grass->mType = data->mGrassValue - 1;
        bg->mGrassCount++;
    }

    // Return null tile so it doesn't get drawn
    return 0;
}

void *LoadCustomFlowerBrres() {

    // Allocate name buffer on the stack
    char buffer[16];

    // Pick correct arc and brres files
    char *arcFile = NULL;
    char *brresFile = NULL;

    switch(dGrassBinMng_c::m_instance->mpData->mFlowerStyle) {
        case 0:
            arcFile = "obj_hana";
            brresFile = "g3d/obj_hana.brres";
            break;
        case 1:
            arcFile = "obj_hana_daishizen";
            brresFile = "g3d/obj_hana_daishizen.brres";
            break;
        default:
            int area = dScStage_c::m_instance->mCurrArea;
            arcFile = dBgGlobal_c::ms_pInstance->getEnvironment(area, dGrassBinMng_c::m_instance->mTileSlot);
            brresFile = "g3d/obj_hana.brres";
    }

    // Load it
    return dResMng_c::m_instance->getRes(arcFile, brresFile).ptr();
}

void *LoadCustomGrassBrres(dRes_c *res, char *originalArc, char *originalName, int style) {

    // Replicate original call if custom brres is not to be loaded
    if (style < 2 || dGrassBinMng_c::m_instance->mpData->mGrassStyle < 2) {
        return res->getRes(originalArc, originalName).ptr();
    }

    // Load custom brres otherwise
    int area = dScStage_c::m_instance->mCurrArea;
    const char *arcFile = dBgGlobal_c::ms_pInstance->getEnvironment(area, dGrassBinMng_c::m_instance->mTileSlot);
    return res->getRes(arcFile, "g3d/obj_kusa.brres").ptr();
}

// Dehardcode tileset name check
kmBranchDefAsm(0x80077F88, 0x80077F8C) {

    // No stack saving necessary here
    nofralloc

    // Check if flowers were loaded
    bl AreCustomFlowersLoaded

    // Move result to r29 and restore r0
    mr. r29, r3
    lbz r0, 0(r26)

    // Check result
    beq end

    // If result is 1, automatically set first character of string to 0 to skip the string checks
    li r0, 0

    // Restore r3 and execute original instruction
    end:
    mr r3, r26
    extsh. r0, r0
    blr
}

// Dehardcode tile position checks - trampoline
kmCallDefAsm(0x8007803C) {

    // Prevent register restoring
    nofralloc

    // Original instruction
    addis r3, r6, 1

    // If dGrassBinMng is not initialized, fall back to Nintendo's code
    lis r4, instancePtr@ha
    lwz r4, instancePtr@l(r4)
    lwz r4, 0(r4)
    cmpwi r4, 0
    beqlr+

    // Push stack manually
    stwu r1, -0x10(r1)
    mflr r0
    stw r0, 0x14(r1)

    // Call the C++ function
    mr r3, r21
    mr r4, r6
    addi r5, r26, 0x8
    addi r6, r25, 0x10
    bl AddFlowerEntry

    // Move result to r6
    mr r6, r3

    // Pop stack and return (later)
    lwz r12, 0x14(r1)
    addi r12, r12, 0x1AC
    mtlr r12
    addi r1, r1, 0x10
    blr
}

// Dehardcode tileset name check 2
kmCallDefAsm(0x808762CC) {

    // Prevent register overwriting
    nofralloc

    // Push stack manually and save r3
    stwu r1, -0x10(r1)
    mflr r0
    stw r0, 0x14(r1)
    stw r31, 0xC(r1)
    mr r31, r3

    // Check if flowers were loaded
    bl AreCustomFlowersLoaded

    // Modified original instruction
    lbz r0, 0(r31)

    // Multiply result by 2 and move it to r28
    slwi. r28, r3, 1
    beq end

    // Override r0 to skip the string check
    li r0, 0

    // Restore r3, push stack manually and return
    end:
    mr r3, r31
    lwz r31, 0xC(r1)
    lwz r12, 0x14(r1)
    mtlr r12
    addi r1, r1, 0x10
    blr
}

// Switch ARC+BRRES file on the fly
kmCallDefAsm(0x80876364) {

    // Let me free
    nofralloc

    // Original instruction
    cmpwi r28, 0

    // If style is not custom, return immediately
    cmpwi cr7, r28, 2
    bltlr+ cr7

    // Alter LR to return later
    mflr r12
    addi r12, r12, 0x2EC
    mtlr r12

    // Call C++ function (does not return)
    b LoadCustomFlowerBrres
}

// Switch ARC+BRRES file on the fly 2
kmCallDefAsm(0x808763C8) {

    // Let me free
    nofralloc

    // Original instruction
    cmpwi r28, 0

    // If style is not custom, return immediately
    cmpwi cr7, r28, 2
    bltlr+ cr7

    // Alter LR to return later
    mflr r12
    addi r12, r12, 0x38
    mtlr r12

    // Call C++ function (does not return)
    b LoadCustomFlowerBrres
}

// Switch ARC+BRRES file on the fly 3
kmCallDefAsm(0x8087657C) {

    // Let me free
    nofralloc

    // Call c++ function (does not return)
    mr r6, r28
    b LoadCustomGrassBrres
}

// Swap 4th flower texture if found
kmCallDefCpp(0x80876410, nw4r::g3d::ResTex, nw4r::g3d::ResFile *res, const char *originalName) {

    // Get custom texture
    nw4r::g3d::ResTex tex = res->GetResTex("obj_hana05");

    // Return if not null
    if (tex.ptr() != NULL)
        return tex;

    // Else return original
    return res->GetResTex(originalName);
}

// Swap 5th flower texture if found
kmCallDefCpp(0x80876420, nw4r::g3d::ResTex, nw4r::g3d::ResFile *res, const char *originalName) {

    // Get custom texture
    nw4r::g3d::ResTex tex = res->GetResTex("obj_hana04");

    // Return if not null
    if (tex.ptr() != NULL)
        return tex;

    // Else return original
    return res->GetResTex(originalName);
}
