#include <new/bases/d_wm_cull_csv_data.hpp>
#include <game/bases/d_s_world_map_static.hpp>

dWmCullCsvData_c dWmCullCsvData_c::m_instance;

void dWmCullCsvData_c::init() {
    mNodeNum = 0;
    for (int i = 0; i < 30; i++) {
        // Dummy data that Nintendo used
        mNodes[i].mRadius = 100.0f;
        for (int j = 0; j < 32; j++) {
            mNodes[i].mNodeName[j] = '\0';
        }
    }
}

void dWmCullCsvData_c::readData() {
    char resName[40];
    sprintf(resName, "mapModelCull/map_model_cull_W%X.csv", dScWMap_c::m_WorldNo + 1);

    // Load the file, and make sure it isn't literally just a newline
    if (getRes("mapModelCull", resName) && !isLineEnd(mpData, 0)) {
        int dataIdx = 0;
        do {
            readDistance(mpData, &dataIdx);
            dataIdx++;
            readNodeName(mpData, &dataIdx);
            dataIdx += 2;
            mNodeNum++;
        } while (dataIdx != mDataSize);
    }
}

void dWmCullCsvData_c::readDistance(char *data, int *dataIndex) {
    int idx = *dataIndex;

    char name[8];
    int length = read(name, &data[idx], -1);

    int value = atoi(name);
    mNodes[mNodeNum].mRadius = (f32)value;

    *dataIndex = idx + length;
}

void dWmCullCsvData_c::readNodeName(char *data, int *dataIndex) {
    int idx = *dataIndex;
    char name[32];
    int length = read(name, &data[idx], -1);

    strncpy(mNodes[mNodeNum].mNodeName, name, length);
    *dataIndex = idx + length;
}