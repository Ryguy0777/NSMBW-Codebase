#pragma once

#include <game/bases/d_rc.hpp>
#include <game/bases/d_bc_type.hpp>

// Sensors
class dBcSensor_c {
public:
    u32 mFlags;
};

class dBcSensorPoint_c : public dBcSensor_c {
public:
    dBcSensorPoint_c() { }
	dBcSensorPoint_c(s32 _x, s32 _y) {
		mX = _x; mY = _y;
	}
	dBcSensorPoint_c(u32 _flags, s32 _x, s32 _y) {
		mFlags = _flags | SensorFlags::Point;
		mX = _x; mY = _y;
	}
    s32 mX, mY;
};

class dBcSensorLine_c : public dBcSensor_c {
public:
    dBcSensorLine_c() { }
	dBcSensorLine_c(s32 _a, s32 _b, s32 _d) {
		mFlags = SensorFlags::Line;
		mLineA = _a; mLineB = _b;
		mDistanceFromCenter = _d;
	}
	dBcSensorLine_c(u32 _flags, s32 _a, s32 _b, s32 _d) {
		mFlags = _flags | SensorFlags::Line;
		mLineA = _a; mLineB = _b;
		mDistanceFromCenter = _d;
	}
    s32 mLineA, mLineB, mDistanceFromCenter;
};

class dBc_c {
public:
    dBc_c();
    virtual ~dBc_c();
    void checkLink();
    bool checkRide();
    bool checkHead(unsigned long);
    int getSakaMoveAngle(u8);
    void checkWall(float *);
    u32 checkWallEnm(float *);

    bool checkFootEnm();
    bool hasSensorFoot() { return mpSensorFoot != nullptr; }
    bool hasSensorHead() { return mpSensorHead != nullptr; }
    bool hasSensorWall() { return mpSensorWall != nullptr; }
    u32 getFootAttr();

    static u32 getUnitType(float x, float y, u8 layer);
    static u32 getUnitKind(float x, float y, u8 layer);
    static u32 getUnitType(u16 x, u16 y, u8 layer);
    static u32 getUnitKind(u16 x, u16 y, u8 layer);

    static bool checkGround(const mVec3_c *, float *, u8, u8, s8);

    void set(dActor_c *owner, dBcSensor_c *feet, dBcSensor_c *head, dBcSensor_c *wall);

    enum WaterCheckResult_e {
        R0 = 0,
        R1 = 1,
        R2 = 2,
        R3 = 3,
        R4 = 4
    };

    dActor_c *mpOwner;
    dBcSensor_c *mpSensorFoot;
    dBcSensor_c *mpSensorHead;
    dBcSensor_c *mpSensorWall;
    mVec3_c *mpOwnerPos;
    mVec3_c *mpOwnerLastPos;
    mVec3_c *mpOwnerSpeed;
    mVec3_c m_20;
    mVec2_c mOwnerPosDelta;
    mVec2_c m_34;
    mVec2_c m_3c;
    float m_44;
    float m_48;
    float m_4c;
    dRc_c *mpRc;
    char pad2[0x34];
    u32 mFlags;
    char mPad3[0x59];
    u8 mLineKind;
    char mPad4[0x6];
    u8 mLayer;

    static int checkWaterDepth(float, float, u8, u8, float *);
    static WaterCheckResult_e checkWater(float, float, u8, float *);
    static void checkBg(float, float, u8, u8, unsigned long);

    bool isHead()  { return mFlags & 0b00000000000000000000000000010101; }
    bool isWallL() { return mFlags & 0b00000000000000000000000000101010; }
    bool isFoot(); //  { return mFlags & 0b00000000000111111110000000000000; }
    bool isWallR() { return mFlags & 0b00111100000000000000000000000000; }

};