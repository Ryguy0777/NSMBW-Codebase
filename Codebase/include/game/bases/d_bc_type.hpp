#include <types.h>

// Forward declarations
class dActor_c;
class dBg_ctr_c;
class dRc_c;

struct SensorFlags {
    enum Type {
        Point,
        Line,
    };

    enum Value {
        Unk_2 = 2, // ??
        IgnoreSimpleColl = 4, // ??
        Unk_8 = 8, // Can enter pipe (?)
        Unk_10 = 0x10, // Related to ice (?)
        Unk_20 = 0x20, // Related to ice (?)
        Unk_40 = 0x40, // ??
        Unk_80 = 0x80, // ??
        Unk_100 = 0x100, // ??
        IgnoreMouseHole = 0x200,
        Unk_400 = 0x400, // ??
        Unk_800 = 0x800, // ??
        NoQuicksand = 0x1000,
        Unk_2000 = 0x2000, // Related to fences (?)
        BreakBlock = 0x4000, // Used by Spike Balls
        JumpDai = 0x8000,
        Unk_10000 = 0x10000, // ??
        Coin1 = 0x20000,
        Coin2 = 0x40000, // Difference with 0x20000?
        CoinOutline = 0x80000,
        ActivateQuestion = 0x100000,
        ActivateDonuts = 0x200000,
        ActivateBrick = 0x400000,
        BreakBrick = 0x800000,
        Unk_1000000 = 0x1000000, // ??
        Unk_2000000 = 0x2000000, // ??
        Unk_4000000 = 0x4000000, // Something related to hitting blocks (?)
        Unk_8000000 = 0x8000000, // ??
        KeepMomentum = 0x10000000, // Used by Giant Spike Balls to keep moving
        Unk_20000000 = 0x20000000, // ??
        Unk_40000000 = 0x40000000, // ??
        NoCollision = 0x80000000,
    };
};

struct SensorOutputFlags {
    enum Value {
        AboveMask = 0xFC000000,
        BelowMask = 0x3FFE000,
        AdjacentMask = 0x1FFF,
    };
};

struct SakaType {
    enum Value {
        None,
        Solid,
        Slope,
        Unk_3,
    };
};

// Represents the first 4 bytes of the tileset behaviour
struct UnitType {
    enum Value {
        None = 0,
        Solid = 1,
        Coin = 2,
        QuestionBlock = 4,
        ExplodableBlock = 8,
        BreakableBlock = 0x10,
        Slope = 0x20,
        ReverseSlope = 0x40,
        ClimbableFence = 0x400,
        PartialSolid = 0x800,
        Spike = 0x1000,
        SolidOnBottom = 0x2000,
        SolidOnTop = 0x8000,
        Pipe = 0x10000,
        DonutBlock = 0x20000,
        Ledge = 0x80000,
        FakeBlockMaybe = 0x100000, // ??
        Meltable = 0x2000000,
    };
};

// Represents the last 4 bytes of the tileset behaviour
struct UnitKind {
    enum Value {
        None,

        // Generic flags
        Icy = 0x10000,
        Snowy = 0x20000,
        Quicksand = 0x30000,
        ConveyorRight = 0x40000,
        ConveyorLeft = 0x50000,
        HorizontalRope = 0x60000,
        NoWallJumps = 0x70000,
        Ledge = 0x80000,
        Ladder = 0x90000,
        Staircase = 0xA0000,
        Carpet = 0xB0000,
        Dusty = 0xC0000,
        Grass = 0xD0000,
        Muffled = 0xE0000,
        BeachSand = 0xF0000,

        // Unused pipe flags
        GreenPipe = 0x200,
        RedPipe = 0x600,
        YellowPipe = 0xA00,

        // No flag values
        RailUpslope = 1,
        RailDownslope = 2,
        RailCornerFill = 3,
        RailCorner = 4,
        RailHorizontal = 5,
        RailVertical = 6,
        RailGentleUpslopeRight = 8,
        RailGentleUpslopeLeft = 9,
        RailGentleDownslopeRight = 0xA,
        RailGentleDownslopeLeft = 0xB,
        RailSteepUpslopeTop = 0xC,
        RailSteepUpslopeBottom = 0xD,
        RailSteepDownslopeBottom = 0xE,
        RailSteepDownslopeTop = 0xF,
        RailCircle = 0x10,
        Rail2CircleTR = 0x11,
        Rail2CircleTL = 0x12,
        Rail2CircleBR = 0x13,
        Rail2CircleBL = 0x14,
        Rail4Circle00 = 0x15,
        Rail4Circle01 = 0x16,
        Rail4Circle02 = 0x17,
        Rail4Circle03 = 0x18,
        Rail4Circle10 = 0x19,
        Rail4Circle13 = 0x1A,
        Rail4Circle20 = 0x1B,
        Rail4Circle23 = 0x1C,
        Rail4Circle30 = 0x1D,
        Rail4Circle31 = 0x1E,
        Rail4Circle32 = 0x1F,
        Rail4Circle33 = 0x20,
        RailStop = 0x22,
        BeanstalkStop = 0x23,
        CoinOutline = 0x28,
        CoinBattle = 0x35,
        RedBlockOutline = 0x36,
        RedBlockOutline2 = 0x37,
        CaveEntranceRight = 0x38,
        CaveEntranceLeft = 0x39,
        Layer0Pit = 0x3B,

        // Climbable Fence flag
        FenceFree = 1,
        FenceTL = 2,
        FenceTop = 3,
        FenceTR = 4,
        FenceLeft = 5,
        FenceMiddle = 6,
        FenceRight = 7,
        FenceBL = 8,
        FenceBottom = 9,
        FenceBR = 0xA,

        // Coin flag
        CoinGeneric = 0,
        BlueCoin = 4,

        // Explodable Block flag
        StoneBlock = 1,
        WoodenBlock = 2,
        RedBlock = 3,

        // Pipe flag
        PipeVertTL = 0,
        PipeVertTR = 1,
        PipeVertBL = 2,
        PipeVertBR = 3,
        PipeVertLeft = 4,
        PipeVertRight = 5,
        PipeVertJunctionLeft = 6,
        PipeVertJunctionRight = 7,
        PipeHorzalTL = 8,
        PipeHorzBL = 9,
        PipeHorzTR = 0xA,
        PipeHorzBR = 0xB,
        PipeHorzTop = 0xC,
        PipeHorzBottom = 0xD,
        PipeHorzJunctionTop = 0xE,
        PipeHorzJunctionBottom = 0xF,
        PipeMiniVertTop = 0x10,
        PipeMiniVertBottom = 0x12,
        PipeMiniVertJunction = 0x16,
        PipeMiniHorzLeft = 0x18,
        PipeMiniHorzRight = 0x1A,
        PipeMiniVertCenter = 0x1C,
        PipeMiniHorzCenter = 0x1D,
        PipeMiniHorzJunction = 0x1E,
        PipeCorner = 0x1F,

        // Question block flag
        QBlockFireFlower = 0,
        QBlockStar = 1,
        QBlockCoin = 2,
        QBlockVine = 3,
        QBlock1UP = 4,
        QBlockMiniShroom = 5,
        QBlockPropeller = 6,
        QBlockPenguinSuit = 7,
        QBlockIceFlower = 8,

        // Partial solid flag
        PartialBlockTL = 1,
        PartialBlockTR = 2,
        PartialBlockTLTR = 3,
        PartialBlockBL = 4,
        PartialBlockTLBL = 5,
        PartialBlockBLTR = 6,
        PartialBlockTLTRBL = 7,
        PartialBlockBR = 8,
        PartialBlockTLBR = 9,
        PartialBlockTRBR = 0xA,
        PartialBlockTLTRBR = 0xB,
        PartialBlockBLBR = 0xC,
        PartialBlockTLBLBR = 0xD,
        PartialBlockTRBLBR = 0xE,
        PartialBlockFull = 0xF,

        // Slope flag
        SteepSlopeUp = 0,
        SteepSlopeDown = 1,
        SlopeUpLeft = 2,
        SlopeUpRight = 3,
        SlopeDownLeft = 4,
        SlopeDownRight = 5,
        VerySteepSlopeUpBottom = 6,
        VerySteepSlopeUpTop = 7,
        VerySteepSlopeDownBottom = 8,
        VerySteepSlopeDownTop = 9,
        SlopeEdge = 0xA,
        GentleSlopeUp1 = 0xB,
        GentleSlopeUp2 = 0xC,
        GentleSlopeUp3 = 0xD,
        GentleSlopeUp4 = 0xE,
        GentleSlopeDown1 = 0xF,
        GentleSlopeDown2 = 0x10,
        GentleSlopeDown3 = 0x11,
        GentleSlopeDown4 = 0x12,

        // Reverse Slope flag
        SteepRSlopeDown = 0,
        SteepRSlopeUp = 1,
        RSlopeDownLeft = 2,
        RSlopeDownRight = 3,
        RSlopeUpLeft = 4,
        RSlopeUpRight = 5,
        VerySteepRSlopeDownTop = 6,
        VerySteepRSlopeDownBottom = 7,
        VerySteepRSlopeUpTop = 8,
        VerySteepRSlopeUpBottom = 9,
        RSlopeEdge = 0xA,
        GentleRSlopeDown1 = 0xB,
        GentleRSlopeDown2 = 0xC,
        GentleRSlopeDown3 = 0xD,
        GentleRSlopeDown4 = 0xE,
        GentleRSlopeUp1 = 0xF,
        GentleRSlopeUp2 = 0x10,
        GentleRSlopeUp3 = 0x11,
        GentleRSlopeUp4 = 0x12,

        // Spike flag
        SpikeLeft = 0,
        SpikeRight = 1,
        SpikeUp = 2,
        SpikeDown = 3,
        SpikeLongDownBottom = 4,
        SpikeLongDownTop = 5,
        SpikeSingleDown = 6,
        SpikeBlock = 7,

        // Conveyor Belt flag
        ConveyorLeftSlow = 0,
        ConveyorLeftFast = 1,
        ConveyorRightSlow = 2,
        ConveyorRightFast = 3,
    };
};

struct BgDamageFlag {
    enum Value {
        None = 0,
        SpikeLeft = 1,
        SpikeRight = 2,
        SpikeUp = 4,
        SpikeDown = 8,
        SpikeLongDownBottom = 0x10,
        SpikeLongDownTop = 0x20,
        SpikeSingleDown = 0x40,
        SpikeBlock = 0x80,
    };
};