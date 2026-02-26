#include <propelparts/game_config.h>

#ifdef PLAYER_1_CAN_CHANGE_CHARACTER
// Source: https://github.com/Developers-Collective/mkwcat-gecko-codes-kamek-port/blob/kamek-ports/source/nsmbw/Kamek/src/Player1CanChangeCharacter.S
// originally from: https://github.com/mkwcat/gecko-codes/blob/master/source/nsmbw/Player-1-Can-Change-Character.cpp

#include <kamek.h>
#include <types.h>
#include <constants/sound_list.h>

extern int DWORD_8093CDF0;
extern int mPlayerType__9daPyMng_c[4];

// SkipConstructingBaseP1Model
kmWrite32(0x809027F8, 0x3880FFFF);

// SkipDestructingBaseP1Model
kmWriteNop(0x809028F8);

// SkipLoadingSubplayers
kmWriteNop(0x809029A8);

// CopySubplayerModelIntoP1Model
extern "C" void WMPlayer_loadSubplayers(void);
kmBranchDefAsm(0x80902B7C, 0x80902B80) {
    nofralloc

    mr      r3, r31
    bl      WMPlayer_loadSubplayers

    // Find the subplayer model
    lis     r9, mPlayerType__9daPyMng_c@ha
    lwz     r9, mPlayerType__9daPyMng_c@l(r9) // Player 1 character
    lwz     r10, 0x184(r31)

    L_WMPlayerUseSubplayerModel_Loop:
    lwz     r4, 0x4(r10) // Character index
    rlwinm  r4, r4, 2, 26, 29
    addis   r4, r4, DWORD_8093CDF0@ha
    lwz     r4, DWORD_8093CDF0@l(r4)

    cmpw    r9, r4
    bne     L_WMPlayerUseSubplayerModel_LoopCond

	// Copy the model
    lwz     r4, 0x1CC(r10)
    lwz     r4, 0x4(r4)
    stw     r4, 0x1CC + 0x4(r31)
    b       L_WMPlayerUseSubplayerModel_Out

    L_WMPlayerUseSubplayerModel_LoopCond:
    lwz     r10, 0x184(r10)
    cmpwi   r10, 0
    bne     L_WMPlayerUseSubplayerModel_Loop

    L_WMPlayerUseSubplayerModel_Out:
	blr
}

// LoadMarioModelForWMSubplayer
kmWrite32(0x80902BE8, 0x3BC00000);

// WMSubplayerFix01
kmBranchDefAsm(0x80902D58, 0x80902D5C) {
    nofralloc

    subic   r5, r23, 1
    subfe   r5, r5, r23
	blr
}

// WMSubplayerFix02
kmBranchDefAsm(0x80902D6C, 0x80902D70) {
    nofralloc

    subic   r5, r23, 1
    subfe   r5, r5, r23
    blr
}

// WMSubplayerFixFloat
kmBranchDefAsm(0x80902E14, 0x80902E18) {
    nofralloc

    stw     r3, 0x294(r4) // Original instruction
    lwz     r4, 0x184(r4)
    stw     r3, 0x294(r4)

    // Then copy player 1's status thing
    lwz     r9, 0x384(r31)
    lis     r10, mPlayerType__9daPyMng_c@ha
    lwz     r10, mPlayerType__9daPyMng_c@l(r10) // Player 1 character
    slwi    r10, r10, 2
    stwx    r9, r6, r10
    blr
}

// WMSubplayerFixStatusOnLoad
kmBranchDefAsm(0x80909410, 0x80909414) {
    nofralloc

    lwz     r0, 0x0(r6)
    slwi    r0, r0, 2

    lwz     r8, 0x384(r5)
    cmpwi   r8, 0x3
    beq     L_WMSubplayerFixStatusOnLoad_NotActive

    mr      r8, r4

    L_WMSubplayerFixStatusOnLoad_NotActive:
    stwx    r8, r3, r0
    addi    r7, r5, 0x8 // Original instruction
    blr
}

//FixWMSubplayerLoadModelArray1
kmBranchDefAsm(0x808EB7D8, 0x808EB7DC) {
    nofralloc

	lis r5, DWORD_8093CDF0@ha
	blr
}

// FixWMSubplayerLoadModelArray2
kmBranchDefAsm(0x808EB7E4, 0x808EB7E8) {
    nofralloc

	addi r5, r5, DWORD_8093CDF0@l
	blr
}

// FixVoiceOnAdd1
kmWriteNop(0x80902E78);

// FixVoiceOnAdd2
kmWrite32(0x80902E84, 0x9BBF0124);

// ChangeMarioDecideJoinSFX
kmWrite32(0x802F62C8, SE_VOC_MA_HUKKATSU);

// ChangeLuigiCourseInSFX1
kmWrite32(0x802F63D0, SE_VOC_LU_CS_COURSE_IN_MULTI);

// ChangeLuigiCourseInSFX2
kmWrite32(0x802F63D8, SE_VOC_LU_CS_COURSE_IN_MULTI);

// ChangeBlueToadCourseInSFX1
kmWrite32(0x802F64EC, SE_VOC_KO_CS_COURSE_IN_MULTI);

// ChangeBlueToadCourseInSFX2
kmWrite32(0x802F64F4, SE_VOC_KO_CS_COURSE_IN_MULTI);

// ChangeYellowToadCourseInSFX1
kmWrite32(0x802F6608, SE_VOC_KO2_CS_COURSE_IN_MULTI);

// ChangeYellowToadCourseInSFX2
kmWrite32(0x802F6610, SE_VOC_KO2_CS_COURSE_IN_MULTI);

// MakePlayerDecideSoundForEveryone
kmWriteNop(0x807A1C8C);

// PlayerDecideSound
static const short D_PlayerDecideSound_Sounds[4] = {
    SE_VOC_MA_PLAYER_DECIDE,
    SE_VOC_LU_CS_DECIDE_JOIN,
    SE_VOC_KO_CS_DECIDE_JOIN,
    SE_VOC_KO2_CS_DECIDE_JOIN
};
kmBranchDefAsm(0x807A1C94, 0x807A1C98) {
    lis     r10, D_PlayerDecideSound_Sounds@h
    ori     r10, r10, D_PlayerDecideSound_Sounds@l

    lis     r9, mPlayerType__9daPyMng_c@ha
    lwz     r9, mPlayerType__9daPyMng_c@l(r9) // Player 1 character
    slwi    r9, r9, 1
    lhzx    r4, r10, r9

    // TODO: Can't think of a good way to do mini sounds for other characters
    bne     L_PlayerDecideSound_NotMini
    cmpwi   r9, 0
    bne     L_PlayerDecideSound_NotMini

    li      r4, 804 // SE_VOC_MA_PLAYER_DECIDE_MAME

    L_PlayerDecideSound_NotMini:
	blr
}

// IncreaseCourseInVolumeSaveParam2
kmBranchDefAsm(0x8019AAB4, 0x8019AAB8) {
    nofralloc

    mflr    r0
    stw     r4, 0x8(r1)
	blr
}

// IncreaseCourseInVolume
static const float D_IncreaseCourseInVolume_Volume = 2.0f;
extern "C" void SetVolume__Q44nw4r3snd6detail10BasicSoundFfi(void);
kmBranchDefAsm(0x8019AB8C, 0x8019AB90) {
    nofralloc

    // Effectively the original instruction
    stfs    f1, 0xBC(r3)

    // Check if Mario
    lwz     r0, 0xAC(r29)
    cmpwi   r0, 0
    beq     cr7, IncreaseCourseInVolume_Out

    // Check if COURSE_IN sound
    lwz     r0, 0x8(r1)
    cmpwi   r0, 5
    beq     IncreaseCourseInVolume_CorrectSound

    cmpwi   r0, 7
    bne     IncreaseCourseInVolume_Out

    IncreaseCourseInVolume_CorrectSound:
	lis 	r4, D_IncreaseCourseInVolume_Volume@h
	ori     r4, r4, D_IncreaseCourseInVolume_Volume@l

    lfs     f1, 0(r4)
    li      r4, 0
    bl      SetVolume__Q44nw4r3snd6detail10BasicSoundFfi

    IncreaseCourseInVolume_Out:
	blr
}

kmWriteNop(0x807B0770);

// WMStarPowerupSound
static const short D_WMStarPowerupSound_Sounds[4] = {
    SE_VOC_MA_GET_STAR,
    SE_VOC_LU_GET_STAR,
    SE_VOC_KO_GET_STAR,
    SE_VOC_KO2_GET_STAR
};
kmBranchDefAsm(0x807B0778, 0x807B077C) {
    nofralloc

	lis 	r10, D_WMStarPowerupSound_Sounds@h
	ori 	r10, r10, D_WMStarPowerupSound_Sounds@l

    lwz     r9, 0x0(r6) // Character
    slwi    r9, r9, 1
    lhzx    r4, r10, r9

    // TODO: Mini sounds
	blr
}

// WMChangeP1Model_SaveReg
kmBranchDefAsm(0x80902ED4, 0x80902ED8) {
    nofralloc

    mflr    r0
    stw     r3, 0x8(r1)
	blr
}

// WMChangeP1Model
kmBranchDefAsm(0x80902F20, 0x80902F24) {
    nofralloc

    lis     r9, mPlayerType__9daPyMng_c@ha
    lwz     r9, mPlayerType__9daPyMng_c@l(r9) // Player 1 character
    cmpw    r9, r28
    bne     WMChangeP1Model_Out

    lwz     r9, 0x8(r1)
    stw     r3, 0x1CC + 0x4(r9)  

    WMChangeP1Model_Out:
    lwz     r12, 0x0(r3) // Original instruction
	blr
}

// SetZero
kmWrite32(0x808EE0D4, 0x38050000);
kmWrite32(0x808EE0E0, 0x2C050000);
kmWrite32(0x80902D1C, 0x3B440000);
kmWrite32(0x80902D20, 0x3B3F0000);
kmWrite32(0x80902D24, 0x3B030000);
kmWrite32(0x80902D2C, 0x3AE00000);
kmWrite32(0x8090302C, 0x3BDD0000);
kmWrite32(0x80903030, 0x3BA30000);
kmWrite32(0x80903038, 0x3B600000);
kmWrite32(0x807A0088, 0x3B830000);
kmWrite32(0x807A008C, 0x3B600000);
kmWrite32(0x807A0190, 0x38830000);
kmWrite32(0x807A11F0, 0x391F0000);
kmWrite32(0x807A1204, 0x39200000);
kmWrite32(0x807A14D8, 0x38C30000);
kmWrite32(0x807A14DC, 0x38E00000);
kmWrite32(0x807A1518, 0x38830000);
kmWrite32(0x807A152C, 0x38050000);
kmWrite32(0x807A1534, 0x2C050000);
kmWrite32(0x807A1628, 0x389F0000);
kmWrite32(0x807A162C, 0x38030000);
kmWrite32(0x807A1634, 0x2C030000);
kmWrite32(0x807A1658, 0x38BF0000);
kmWrite32(0x807A165C, 0x38C00000);
kmWrite32(0x807A15C8, 0x38BF0000);
kmWrite32(0x807A15CC, 0x38C00000);
kmWrite32(0x807A16D8, 0x38C30000);
kmWrite32(0x807A16DC, 0x38E00000);
kmWrite32(0x807A1718, 0x38830000);
kmWrite32(0x807A172C, 0x38050000);
kmWrite32(0x807A1734, 0x2C050000);
kmWrite32(0x807A17AC, 0x38BF0000);
kmWrite32(0x807A17B0, 0x38C00000);

// SetFour
kmWrite32(0x807A018C, 0x38000004);

// DontSetP1PowerupToMario
kmBranchDefAsm(0x80902FC8, 0x80903020) {
    nofralloc

    blr
}

// Change layout name
const char *newContentsName = "Layout/charaChangeSelectContents_playerChange/charaChangeSelectContents.arc";
kmBranchDefAsm(0x8079fae0, 0x8079fae4) {
    lis r4, newContentsName@h
    ori r4, r4, newContentsName@l
    lwz r4, 0(r4)
    blr
}
 
// ChangePSELBmgForPlayerCount
kmBranchDefAsm(0x807A10C8, 0x807A10CC) {
    nofralloc

	cmpwi 	r0, 1
	beq 	ChangePSELBmgForPlayerCount_1P

	li 		r6, 0x1C // Original instruction
	b 		ChangePSELBmgForPlayerCount_Out

    ChangePSELBmgForPlayerCount_1P:
	li 		r6, 0x63

    ChangePSELBmgForPlayerCount_Out:
	blr
}

// DontForceSetP1MarioPos1
kmWriteNop(0x807A068C);

// DontForceSetP1MarioPos2
kmWriteNop(0x807A0694);

// DontForceSetP1MarioPos3
kmWriteNop(0x807A069C);


// DontAutomaticallyRegisterMario1
kmWrite32(0x80918B28, 0x38000000);

// DontAutomaticallyRegisterMario2
kmWrite32(0x800BB1B4, 0x90E30384);

// AllowP1ToRegister
kmWriteNop(0x807A0FE8);

// CancelAddDropCheckPlayer1
kmBranchDefAsm(0x807A0D24, 0x807A0D28) {
    nofralloc

    mflr    r0 // Original instruction
    // Check if player 1 is registered
    lwz     r4, 0x74(r3)
    lbz     r4, 0x29A(r4)
	blr
}

// CancelAddDropFix
extern "C" void returnFrom_CancelAddDropFix(void);
kmBranchDefAsm(0x807A0D40, NULL) {
    nofralloc

    // Return 0 if player 1 is not registered
    cmpwi   r4, 0
    bne-    L_CancelAddDropFix_Out

    li      r3, 0

    // Return the function
    lwz     r0, 0x14(r1)
    mtlr    r0
    addi    r1, r1, 0x10
    blr

    L_CancelAddDropFix_Out:
    li      r4, 0x900 // Original instruction
	b 		returnFrom_CancelAddDropFix
}


// CancelAfterRegisteredFix
extern "C" void returnFrom_CancelAfterRegisteredFix(void);
kmBranchDefAsm(0x807A0D68, NULL) {
    nofralloc

    li      r3, 0 // Original instruction

    // Return 0 if player 1 is registered
    cmpwi   r4, 0
    beq-    L_CancelAfterRegisteredFix_Out

    // Return the function
    lwz     r0, 0x14(r1)
    mtlr    r0
    addi    r1, r1, 0x10
    blr

    L_CancelAfterRegisteredFix_Out:
	b 		returnFrom_CancelAfterRegisteredFix
}

// Prevent2DMarioFromJumping
kmWriteNop(0x807A12A8);

// PreventJumpOnWM
kmBranchDefAsm(0x808C5DFC, 0x808C5E00) {
    nofralloc

    lis     r9, mPlayerType__9daPyMng_c@ha
    lwz     r9, mPlayerType__9daPyMng_c@l(r9)
    lwz     r10, 0x4(r3)
    rlwinm.	r10, r10, 0, 28, 31
    // r9 == r10
    xor     r10, r10, r9
    cntlzw  r10, r10
    srwi.   r10, r10, 5
	blr
}


// AllowSelectingMario
kmWrite32(0x80770A8C, 0x2C030004);

// RemoveMarioHereWeGo
kmWrite32(0x807AC8CC, 0x38000001);

// DontSkipCharSelectWithP1
kmWriteNop(0x80918B50);

// FixArrows1
kmWrite32(0x8076E630, 0x2C000004);

// FixArrows2
kmWrite32(0x8076EAB8, 0x2C030004);

// Fix2ButtonNotGlowingForP1
kmWriteNop(0x8076DDE8);

// FixInUseSelect1
kmWrite32(0x80770988, 0x5400173A);

// FixInUseSelect2
kmWriteNop(0x807709A8);

// FixInUseSelect3
kmWrite32(0x807709B0, 0x540407FE);

// SimilarFixes
kmWrite32(0x8076FE4C, 0x5400173A);


// CharacterChangeSelectBase_FixDefaultSelection
kmBranchDefAsm(0x8076FF0C, 0x8076FF10) {
    nofralloc

    cmpwi   r0, 0
    bne     CharacterChangeSelectBase_FixDefaultSelection_Out

    li      r0, 4

    CharacterChangeSelectBase_FixDefaultSelection_Out:
    li      r4, 0 // Original instruction
	blr
}

// CharacterChangeSelectBase_FixDefaultSelection2
kmBranchDefAsm(0x8076FE84, 0x8076FE88) {
    nofralloc

    cmpwi   r0, 0
    bne     CharacterChangeSelectBase_FixDefaultSelection2_Out

    li      r0, 4

    CharacterChangeSelectBase_FixDefaultSelection2_Out:
    stw     r0, 0x2E0(r3) // Original instruction
	blr
}

// CharacterChangeSelectBase_FixMarioVoice
kmBranchDefAsm(0x8077117C, 0x80771180) {
    nofralloc

    li      r4, 1010 // SE_VOC_KO2_PLAYER_JOIN

    lwz     r0, 0x2D4(r31)
    cmpwi   r0, 0
    bne     CharacterChangeSelectBase_FixMarioVoice_NotMario

    li      r4, 797 // SE_VOC_MA_PLAYER_JOIN

    CharacterChangeSelectBase_FixMarioVoice_NotMario:
	blr
}

// CharacterChangeSelectBase_FixMarioVoiceMini
kmBranchDefAsm(0x80771164, 0x80771168) {
    nofralloc

    li      r4, 1011 // SE_VOC_KO2_PLAYER_JOIN_MAME

    lwz     r0, 0x2D4(r31)
    cmpwi   r0, 0
    bne     CharacterChangeSelectBase_FixMarioVoiceMini_NotMario

    li      r4, 798 // SE_VOC_MA_PLAYER_JOIN_MAME

    CharacterChangeSelectBase_FixMarioVoiceMini_NotMario:
	blr
}

// CharacterChangeSelectBase_FixInUseCheck
kmBranchDefAsm(0x8076FDC4, 0x8076FDC8) {
    nofralloc

    cmpwi   r31, 7
    bne     CharacterChangeSelectBase_FixInUseCheck_Out

    li      r4, 0

    CharacterChangeSelectBase_FixInUseCheck_Out:
    mr      r3, r29
	blr
}

// CharacterChangeSelectBase_FixInUseCheck2
kmBranchDefAsm(0x8076FDD4, 0x8076FDD8) {
    nofralloc

    cmpwi   r31, 7
    bne     CharacterChangeSelectBase_FixInUseCheck2_Out

    li      r31, 8

    CharacterChangeSelectBase_FixInUseCheck2_Out:
    cmpwi   r31, 0
	blr
}

// ExpandCharacterChangeSelectContents
kmWrite32(0x80772A58, 0x386002C0);

// CharacterChangeSelectContents_GrabMarioIconPanes
extern "C" void PPaneRegister__9LytBase_cFPPCcPPQ34nw4r3lyt7Picturei(void);
static const char *MarioIconPicturePanes[] = {
    "P_mar_00", "P_mar_01"
};
kmBranchDefAsm(0x80772EDC, 0x80772EE0) {
    nofralloc

	addi 	r3, r30, 0x70
	lis 	r4, MarioIconPicturePanes@h
	ori 	r4, r4, MarioIconPicturePanes@l
	addi 	r5, r30, 0x2B8
	li 		r6, 2
	bl 		PPaneRegister__9LytBase_cFPPCcPPQ34nw4r3lyt7Picturei

	addi 	r3, r30, 0x70 // Original instruction
	blr
}

// CharacterChangeSelectContents_MakeMarioIconPanesInvisible
kmBranchDefAsm(0x807733C0, 0x807733D8) {
    nofralloc

	// Original code
    CharacterChangeSelectContents_MakeMarioIconPanesInvisible:
	lwz     r6, 0x254(r7)
	addi    r7, r7, 8
	lbz     r0, 0xBB(r6)
	rlwinm  r0, r0, 0, 24, 30
	stb     r0, 0xBB(r6)
	bdnz    CharacterChangeSelectContents_MakeMarioIconPanesInvisible

	// New code

    slwi    r0, r31, 2
    add     r6, r30, r0

 	lwz     r6, 0x2B8(r6)
    lbz     r0, 0xBB(r6)
    rlwinm  r0, r0, 0, 24, 30
    stb     r0, 0xBB(r6)

    blr
}

// ReadArrayDifferently1
kmWrite32(0x8076FD8C, 0x7FE628AE);

// CharacterChangeSelectBase_ExpandIconArray
kmWrite32(0x80933CF0, 0x06000102);
kmWrite8(0x80933CF4, 0x07);

// CharacterChangeSelectBase_FixIconUpdate
extern "C" void returnFrom_CharacterChangeSelectBase_FixIconUpdate_2(void);
extern int DWORD_8093F520;
extern "C" void LayoutDispNumber__8dGameComFRCiRCiP12LytTextBox_cb(void);
kmBranchDefAsm(0x8077359C, 0x807735A0) {
    nofralloc

    cmpwi   r4, 7
    beq     CharacterChangeSelectBase_FixIconUpdate_MarioUnlocked

    cmpwi   r4, 8
    bne     CharacterChangeSelectBase_FixIconUpdate_Else

    CharacterChangeSelectBase_FixIconUpdate_MarioLocked:
    // Make the gray out thing visible
    slwi    r0, r5, 2
    add     r5, r3, r0
    lwz     r5, 0x254(r5)
    lbz     r0, 0xBB(r5)
    ori     r0, r0, 0x01
    stb     r0, 0xBB(r5)
    // Fall through

    CharacterChangeSelectBase_FixIconUpdate_MarioUnlocked:
    addi    r3, r3, 0x2A4 // param 1
    slwi    r0, r31, 2
    lis     r4, DWORD_8093F520@ha
    addi    r4, r4, DWORD_8093F520@l // param 2
    add     r5, r30, r0 // param 3
    lwz     r5, 0x274(r5)
    li      r6, 0 // param 4

    // Call the function
    lis     r12, LayoutDispNumber__8dGameComFRCiRCiP12LytTextBox_cb@h
    ori     r12, r12, LayoutDispNumber__8dGameComFRCiRCiP12LytTextBox_cb@l
    mtctr   r12
    bctrl

    slwi    r0, r31, 2
    add     r5, r30, r0

    lwz     r5, 0x2B8(r5)
    lbz     r0, 0xBB(r5)
    rlwinm  r0, r0, 0, 24, 30
    ori     r0, r0, 0x01
    stb     r0, 0xBB(r5)

    b returnFrom_CharacterChangeSelectBase_FixIconUpdate_2

    CharacterChangeSelectBase_FixIconUpdate_Else:
    cmpwi   r4, 4

    CharacterChangeSelectBase_FixIconUpdate_Out:
	blr
}

#endif