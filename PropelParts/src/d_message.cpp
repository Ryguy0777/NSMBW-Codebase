#include <kamek.h>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_message.hpp>
#include <lib/egg/core/eggArchive.h>

// Hacks to load a secondary BMG

static void *sc_msgFiles[] = {
    l_dMessage_obj.mpFile,
    l_dMessage_obj.mpPropelFile,
};

static const char *MSG_NAME_TBL[] = {
    "Message/Message.arc",
    "Message/PropelMsg.arc",
};

#define MSG_NUM ARRAY_SIZE(MSG_NAME_TBL)

// dMessage_c::create()
kmBranchDefCpp(0x800CDC20, NULL, bool, EGG::Heap *pHeap) {
    char filename[100];

    for (int i = 0; i < MSG_NUM; i++) {
        if (sc_msgFiles[i] == nullptr) {
            dGameCom::AreaLanguageFolder(MSG_NAME_TBL[i], filename);

            void *file = l_dMessage_obj.mMsgLoader.request(filename, 0, pHeap);
            if (file == nullptr) {
                return false;
            }

            EGG::Archive *pArc = EGG::Archive::mount(file, pHeap, 4);
            EGG::Archive::FileInfo fileInfo;
            fileInfo.mFileOffset = 0;
            fileInfo.mFileSize = 0;

            // Reusing the filename to avoid conflicts with tools that expect
            // the game's BMGs to have this name
            sc_msgFiles[i] = pArc->getFile("wii_mj2d.bmg", &fileInfo);
            if (sc_msgFiles[i] == nullptr) {
                return false;
            }

            l_dMessage_obj.buildMsgRes(pHeap, i);

            // Free the DVD loader so we can use it for the other archive
            l_dMessage_obj.mMsgLoader.freeResouce();
        }
    }
    return true;
}

void dMessage_c::buildMsgRes(EGG::Heap *pHeap, int index) {
    if (index == 0) {
        mpMsgRes = new MsgRes_c(sc_msgFiles[index], pHeap);
    } else {
        mpPropelMsgRes = new MsgRes_c(sc_msgFiles[index], pHeap);
    }
}

MsgRes_c *dMessage_c::getPropelMsgRes() {
    return l_dMessage_obj.mpPropelMsgRes;
}

const wchar_t *getPropelMsg(ulong cat, ulong msg) {
    return l_dMessage_obj.mpPropelMsgRes->getMsg(cat, msg);
}

// TODO: Allow LytBase_c:TPaneNameRegister() [800C9010] to use whatever BMG it wants