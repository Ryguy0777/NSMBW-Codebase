#include <new/bases/d_csv_parser.hpp>

dCsvParser_c::dCsvParser_c() {
    mpData = nullptr;
    mDataSize = 0;
}

dCsvParser_c::~dCsvParser_c() { }

bool dCsvParser_c::getRes(const char *arcName, const char *resPath) {
    // Override as needed
    ulong resSize = 0;
    void *res = dResMng_c::m_instance->mRes.getRes(arcName, resPath, &resSize, nullptr);

    mpData = (char*)res;
    mDataSize = resSize;
    return (res != nullptr);
}

bool dCsvParser_c::isLineEnd(char *data, int index) {
    if (data[index] != '\r') {
        return false;
    }
    if (data[index + 1] != '\n') {
        return false;
    }

    return true;
}
