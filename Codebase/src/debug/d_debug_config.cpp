#include <kamek.h>
#include <new/bases/d_debug_config.hpp>
#include <constants/game_constants.h>
#include <egg/core/eggHeap.h>
#include <nw4r/db/mapfile.h>
#include <game/mLib/m_heap.hpp>
#include <game/bases/d_s_restart_crsin.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_s_crsin.hpp>

#undef clamp
#undef OSRoundUp32B
#define clamp(n, lower, upper) n <= lower ? lower : n >= upper ? upper : n
#define OSRoundUp32B(x) (((u32)(x) + 32 - 1) & ~(32 - 1))

extern "C" int atoi(const char* str);

const DebugKey keys[] = {
    {DebugKey::LaunchType, "LaunchType"},
    {DebugKey::SaveNumber, "SaveNumber"},
    {DebugKey::PlayerCount, "PlayerCount"},
    {DebugKey::PowerUp, "PowerUp"},
    {DebugKey::HasStar, "HasStar"},
    {DebugKey::World, "World"},
    {DebugKey::Level, "Level"},
    {DebugKey::Area, "Area"},
    {DebugKey::Entrance, "Entrance"},
    {DebugKey::GameMode, "GameMode"},
    {DebugKey::HintMovieType, "HintMovieType"},
    {DebugKey::ShowPregame, "ShowPregame"},
    {DebugKey::DrawHitboxes, "DrawHitboxes"},
    {DebugKey::DrawColliders, "DrawColliders"},
    {DebugKey::DrawSensors, "DrawSensors"},
    {DebugKey::DrawRideableColliders, "DrawRideableColliders"},
    {DebugKey::MovieId, "MovieId"},
    {DebugKey::ActorLog, "ActorLog"},
};

static dDebugConfig_c instance;
dDebugConfig_c* dDebugConfig_c::m_instance = &instance;

dDebugConfig_c::dDebugConfig_c() {
    // Initialize the values to sane defaults
    mLaunchType = LaunchType::Normal;

    mSaveNo = 0;
    mPlayerCount = 0;
    mPowerUp = POWERUP_NONE;
    mStar = false;

    mWorld = 0;
    mLevel = 0;
    mArea = 0;
    mEntrance = 0xFF;
    mGameMode = LaunchGameMode::Normal;
    mHintMovieType = 0;
    mShowPregame = false;
    mCollisionDebugFlags = ColliderDisplayFlags::None;

    mMovieId = 1;

    mActorLog = false;
}

void dDebugConfig_c::parseConfigLine(char* key, char* param, int paramSize) {

    // Initialize variables
    int decodedParam = 0;
    bool isNumeric = true;

    // If the first two characters are "0x", assume it's a hex number
    bool isHex = (strstr(param, "0x") == param);
    if (isHex) {
        param += 2;
        paramSize -= 2;
    }

    // Check if the value is numeric and convert all uppercase characters to lowercase
    for (int i = 0; i < paramSize; i++) {
        u8 currChar = param[i];
        if (currChar < '0' || currChar > '9') {
            isNumeric = false;
            if (currChar >= 'A' && currChar <= 'Z')
                param[i] = currChar - 0x20;
        }
    }

    // If the value is numeric, convert it
    if (isNumeric) {
        if (isHex)
            decodedParam = nw4r::db::XStrToU32_((const u8*)param);
        else
            decodedParam = atoi(param);

    // If the value is not numeric, check for booleans
    } else
        decodedParam = (strcmp(param, "true") == 0);

    // Parse the key
    int keyId = -1;
    for (int i = 0; i < sizeof(keys) / sizeof(DebugKey); i++) {
        if (strcmp(key, keys[i].keyName) == 0) {
            keyId = keys[i].keyId;
            break;
        }
    }

    // Act depending on the key we found
    switch(keyId) {
        case DebugKey::LaunchType:
            mLaunchType = clamp(decodedParam, 0, 3);
            break;

        case DebugKey::SaveNumber:
            mSaveNo = (decodedParam + 1) & 3;
            break;

        case DebugKey::PlayerCount:
            mPlayerCount = (decodedParam - 1) & 3;
            break;

        case DebugKey::PowerUp:
            mPowerUp = clamp(decodedParam, 0, 6);
            break;

        case DebugKey::HasStar:
            mStar = decodedParam & 1;
            break;

        case DebugKey::World:
            mWorld = clamp(decodedParam - 1, 0, 9);
            break;

        case DebugKey::Level:
            mLevel = clamp(decodedParam - 1, 0, 99);
            break;

        case DebugKey::Area:
            mArea = (decodedParam - 1) & 3;
            break;

        case DebugKey::Entrance:
            mEntrance = (decodedParam - 1) & 0xFF;
            break;

        case DebugKey::GameMode:
            mGameMode = clamp(decodedParam, 0, 5);
            break;

        case DebugKey::HintMovieType:
            mHintMovieType = decodedParam & 3;
            break;

        case DebugKey::ShowPregame:
            mShowPregame = decodedParam & 1;
            break;

        case DebugKey::DrawHitboxes:
            mCollisionDebugFlags |= ((decodedParam & 1) << ColliderDisplayFlags::Hitboxes);
            break;

        case DebugKey::DrawColliders:
            mCollisionDebugFlags |= ((decodedParam & 1) << ColliderDisplayFlags::Colliders);
            break;

        case DebugKey::DrawSensors:
            mCollisionDebugFlags |= ((decodedParam & 1) << ColliderDisplayFlags::Sensors);
            break;

        case DebugKey::DrawRideableColliders:
            mCollisionDebugFlags |= ((decodedParam & 1) << ColliderDisplayFlags::RideableColliders);
            break;

        case DebugKey::MovieId:
            mMovieId = clamp(decodedParam, 1, 3);
            break;
        
        case DebugKey::ActorLog:
            mActorLog = decodedParam & 1;

        default:
    }
}

void dDebugConfig_c::parseConfig(nw4r::ut::CharStrmReader* reader, void* bufferEnd) {

    // Initialize variables
    bool isComment = false;
    bool isParam = false;
    char keyBuffer[64];
    char paramBuffer[64];
    int keyBufferSize = 0;
    int paramBufferSize = 0;

    // Clear the buffer
    memset(keyBuffer, 0, sizeof(keyBuffer));
    memset(paramBuffer, 0, sizeof(paramBuffer));

    // Set the file read function (needed by the hex to u32 function later)
    nw4r::db::GetCharPtr_ = &nw4r::db::GetCharOnMem_;

    // Read one character at a time until the file ends
    do {
        wchar_t nextChar = reader->Next();

        // Ignore all non-ASCII characters, i cannot be arsed
        if (nextChar >= 0x80)
            continue;
        u8 currChar = nextChar & 0x7F;

        // Ignore whitespace characters
        if (currChar == ' ' || currChar == '\t' || currChar == '\r')
            continue;

        // If a newline is reached, clear the variables
        if (currChar == '\n') {

            // Only parse lines with a proper key and param
            if (keyBufferSize != 0 && paramBufferSize != 0)
                parseConfigLine(keyBuffer, paramBuffer, paramBufferSize);

            // Reset variables
            memset(keyBuffer, 0, keyBufferSize);
            memset(paramBuffer, 0, paramBufferSize);
            keyBufferSize = 0;
            paramBufferSize = 0;
            isComment = false;
            isParam = false;

        // Ignore any comment character on the rest of this line
        } else if (!isComment) {

            // Mark characters after a # as a comment
            if (currChar == '#')
                isComment = true;

            // Mark characters after a = as the param
            else if (currChar == '=')
                isParam = true;

            // Store character in the key buffer unless it's full
            else if (!isParam) {
                if (keyBufferSize < sizeof(keyBuffer) - 1) {
                    keyBuffer[keyBufferSize] = currChar;
                    keyBufferSize++;
                }
            }

            // Store character in the param buffer unless it's full
            else if (paramBufferSize < sizeof(paramBuffer) - 1) {
                paramBuffer[paramBufferSize] = currChar;
                paramBufferSize++;
            }
        }

    } while (reader->GetCurrentPos() < bufferEnd);
}

bool dDebugConfig_c::loadConfig() {

    // Locate the file
    int entrynum = DVDConvertPathToEntrynum("config/debug_config.ini");
    if (entrynum == -1) {
        OSReport("Debug config not found, bailing!\n");
        return false;
    }

    // Try to load it
    DVDFileInfo dvdHandle;
    bool fileLoaded = DVDFastOpen(entrynum, &dvdHandle);
    if (!fileLoaded) {
        OSReport("Debug config not loaded, bailing!\n");
        return false;
    }

    // Allocate the necessary space
    size_t size = OSRoundUp32B(dvdHandle.size);
    void* buffer = EGG::Heap::alloc(size, 0x20, mHeap::g_archiveHeap);
    if (buffer == nullptr) {
        OSReport("Failed to allocate buffer, bailing!\n");
        return false;
    }

    // Read the file
    s32 length = DVDReadPrio(&dvdHandle, buffer, size, 0, 2);
    if (length > 0) {

        // Initialize the stream reader, assume the file uses UTF-8
        nw4r::ut::CharStrmReader reader(&nw4r::ut::CharStrmReader::ReadNextCharUTF8);
        reader.Set((const char*)buffer);

        // Parse the file
        parseConfig(&reader, (void*)((u32)buffer + dvdHandle.size));
    }

    // Close the file, free the buffer and return
    DVDClose(&dvdHandle);
    EGG::Heap::free(buffer, mHeap::g_archiveHeap);
    return true;
}

bool dDebugConfig_c::setupConfig() {
    return instance.loadConfig();
}

extern "C" void CrsinLoadFiles();

// process launch type
kmBranchDefCpp(0x8015D850, NULL, void, void) {
    // If launch type is 0, do the original call and nothing else
    u8 launchType = instance.mLaunchType;
    if (launchType == LaunchType::Normal) {
        dScRestartCrsin_c::startTitle(false, false);
        return;
    }

    // Set up the save game
    dGameCom::initGame();
    u8 saveNo = (instance.mSaveNo > 0) ? instance.mSaveNo - 1 : dSaveMng_c::m_instance->getLastSelectedFile();
    dSaveMng_c::m_instance->initLoadGame(saveNo);

    // Act depending on the launch type
    if (launchType == LaunchType::ToStage) {
        dScWMap_c::SetCourseTypeForce(instance.mWorld, instance.mLevel, dScWMap_c::COURSE_TYPE_NO_STAR_COINS);
        dScRestartCrsin_c::m_startGameInfo.mWorld1 = instance.mWorld;
        dScRestartCrsin_c::m_startGameInfo.mWorld2 = instance.mWorld;
        dScRestartCrsin_c::m_startGameInfo.mLevel1 = instance.mLevel;
        dScRestartCrsin_c::m_startGameInfo.mLevel2 = instance.mLevel;
        dScRestartCrsin_c::m_startGameInfo.mArea = instance.mArea;
        dScRestartCrsin_c::m_startGameInfo.mEntrance = instance.mEntrance;

        for (int i = 0; i <= instance.mPlayerCount; i++) {
            daPyMng_c::mPlayerEntry[i] = 1;
            daPyMng_c::mPlayerType[i] = i;
            daPyMng_c::mPlayerMode[i] = instance.mPowerUp;
            daPyMng_c::mCreateItem[i] = instance.mStar;
        }

        dScCrsin_c::m_isDispOff = !instance.mShowPregame;

        switch(instance.mGameMode) {
            case LaunchGameMode::CoinBattle:
                dInfo_c::mGameFlag |= dInfo_c::GAME_FLAG_IS_COIN_COURSE;

            case LaunchGameMode::FreePlay:
                dInfo_c::mGameFlag |= dInfo_c::GAME_FLAG_IS_EXTRA_MODE;
                break;

            case LaunchGameMode::SuperGuideReplay:
                dScRestartCrsin_c::m_startGameInfo.mIsReplay = true;
                dScRestartCrsin_c::m_startGameInfo.mScreenType = 1;
                break;

            case LaunchGameMode::HintMovieReplay:
                dScRestartCrsin_c::m_startGameInfo.mIsReplay = true;
                dScRestartCrsin_c::m_startGameInfo.mScreenType = 4;
                dScRestartCrsin_c::m_startGameInfo.mMovieType = instance.mHintMovieType;
                break;

            case LaunchGameMode::TitleReplay:
                dScRestartCrsin_c::m_startGameInfo.mIsReplay = true;
                dScRestartCrsin_c::m_startGameInfo.mScreenType = 3;
                break;

            default:
        }

        dScene_c::setNextScene(fProfile::RESTART_CRSIN, 0, false);
    } else if (launchType == LaunchType::ToMovie) {
        dScene_c::setNextScene(fProfile::MOVIE, instance.mMovieId, false);
    } else if (launchType == LaunchType::ToWorldmap) {
        CrsinLoadFiles();
        dScene_c::setNextScene(fProfile::WORLD_MAP, instance.mWorld, false);
    }
}
