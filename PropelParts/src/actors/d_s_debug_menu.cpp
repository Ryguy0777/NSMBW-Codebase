#include <kamek.h>
#include <new/bases/d_s_debug_menu.hpp>
#include <new/bases/d_custom_profile.hpp>
#include <game/mLib/m_pad.h>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_s_restart_crsin.hpp>
#include <game/snd/snd_audio_mgr.hpp>

// Fix dAudio_c TEMPORARY
kmWrite32(0x80069668, 0x4800001C);

dScDebugMenu_c *dScDebugMenu_c::m_instance = nullptr;

static const dDebugCursorInfo scDebugCursors[dScDebugMenu_c::OPTION_COUNT] = {
    {-295.0f, -180.0f, ">>"},
    {-295.0f, -140.0f, ">>"},
    {-295.0f, -100.0f, ">>"},
    {-295.0f, -60.0f, ">>"},

    {0.0f, 0.0f, ""},

    {-215.0f, -165.0f, "^"},
    {-175.0f, -165.0f, "^"},
    {-100.0f, -165.0f, "^"},
    {0.0f, -165.0f, "^"},
    {-180.0f, -80.0f, "^"},
};

CUSTOM_BASE_PROFILE(DEBUG_MENU, dScDebugMenu_c, fProfile::GAME_SETUP, fProfile::DRAW_ORDER::GAME_SETUP);
dCustomProfile_c l_DEBUG_MENU_profile(&g_profile_DEBUG_MENU, "DEBUG_MENU", fProfile::DEBUG_MENU);

dScDebugMenu_c::dScDebugMenu_c() {
    OSReport("constructing debug menu!\n");
    m_instance = this;
    mpTextWriter = new dDebugTextWriter_c();
    mpTextWriter->Reset();
}

dScDebugMenu_c::~dScDebugMenu_c() {
    m_instance = nullptr;
}

int dScDebugMenu_c::create() {
    mEntrance = 0xFF;
    mSound = 0;

    return SUCCEEDED;
}

int dScDebugMenu_c::execute() {
    switch (mPad::g_currentCore->maStatus->trig) {
        case WPAD_BUTTON_LEFT:
            switch (mCurrentSelection) {
                case PICK_WORLD:
                    mWorld--;
                    break;
                
                case PICK_LEVEL:
                    mLevel--;
                    break;
                
                case PICK_AREA:
                    mArea--;
                    break;
                
                case PICK_ENTRANCE:
                    mEntrance--;
                    break;
                
                case PICK_SOUND:
                    if (mSound > 0) {
                        mSound--;
                    }
                    break;

                default:
                    if (mCurrentSelection < CHOICE_COUNT-1) {
                        mCurrentSelection++;
                    }
                    break;
            }
            break;

        case WPAD_BUTTON_RIGHT:
            switch (mCurrentSelection) {
                case PICK_WORLD:
                    mWorld++;
                    break;
                
                case PICK_LEVEL:
                    mLevel++;
                    break;
                
                case PICK_AREA:
                    mArea++;
                    break;
                
                case PICK_ENTRANCE:
                    mEntrance++;
                    break;
                
                case PICK_SOUND:
                    mSound++;
                    break;
                    
                default:
                    if (mCurrentSelection > 0) {
                        mCurrentSelection--;
                    }
                    break;
            }
            break;

        case WPAD_BUTTON_2:
        case WPAD_BUTTON_A:
            switch (mCurrentSelection) {
                case LEVEL_PICKER:
                    mCurrentSelection = PICK_WORLD;
                    break;

                case GOTO_LEVEL:
                    dScWMap_c::SetCourseTypeForce(mWorld, mLevel, dScWMap_c::COURSE_TYPE_NO_STAR_COINS);
                    dScRestartCrsin_c::m_startGameInfo.mWorld1 = mWorld;
                    dScRestartCrsin_c::m_startGameInfo.mWorld2 = mWorld;
                    dScRestartCrsin_c::m_startGameInfo.mLevel1 = mLevel;
                    dScRestartCrsin_c::m_startGameInfo.mLevel2 = mLevel;
                    dScRestartCrsin_c::m_startGameInfo.mArea = mArea;
                    dScRestartCrsin_c::m_startGameInfo.mEntrance = mEntrance;

                    daPyMng_c::mPlayerEntry[0] = 1;
                    dScene_c::setNextScene(fProfile::RESTART_CRSIN, 0, false);
                    break;
                
                case SOUND_PICKER:
                    mCurrentSelection = PICK_SOUND;
                    break;
                
                case SOUND_TEST:
                    SndAudioMgr::sInstance->startSystemSe(mSound, 1);
                    break;
                
                case PICK_WORLD:
                case PICK_LEVEL:
                case PICK_AREA:
                    mCurrentSelection++;
                    break;
                
                case PICK_ENTRANCE:
                    mCurrentSelection = GOTO_LEVEL;
                    break;
                
                case PICK_SOUND:
                    mCurrentSelection = SOUND_TEST;
                    break;
                
            }
    }
    
    return SUCCEEDED;
}

int dScDebugMenu_c::draw() {
    dDebugCursorInfo currentCursor = scDebugCursors[mCurrentSelection];

    mpTextWriter->Begin();

    mpTextWriter->Print(-300.0f, -220.0f, "PropelParts - Debug Menu\n");
    mpTextWriter->Print(-270.0f, -180.0f, "Level %02d - %02d Area %d Entrance %d\n", mWorld+1, mLevel+1, mArea+1, mEntrance);
    mpTextWriter->Print(-270.0f, -140.0f, "Go To Level\n");
    mpTextWriter->Print(-270.0f, -100.0f, "Sound ID %d\n", mSound);
    mpTextWriter->Print(-270.0f, -60.0f, "Play Sound\n");

    mpTextWriter->Print(currentCursor.x, currentCursor.y, currentCursor.character);

    mpTextWriter->End();

    return SUCCEEDED;
}