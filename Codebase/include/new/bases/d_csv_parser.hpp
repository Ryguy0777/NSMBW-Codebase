#pragma once

#include <game/bases/d_res_mng.hpp>

// Reads a string from a CSV file, returns the string length
int read(char *out, const char *data, int unused);

// Converts ASCII to Int
extern "C" int atoi(char *string);

// General-purpose base class for parsing CSVs
// Vaguely based on dCsvData_c from the retail game
class dCsvParser_c {
public:
    dCsvParser_c();
    virtual ~dCsvParser_c();

    virtual bool getRes(const char *arcName, const char *resPath);

    bool isLineEnd(char *data, int offset);

    char *mpData;
    ulong mDataSize;
};