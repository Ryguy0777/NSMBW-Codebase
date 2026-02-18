#pragma once
#include <new/bases/d_csv_parser.hpp>

class dWmCullCsvData_c : public dCsvParser_c {
public:
    struct Node_s {
        f32 mRadius;
        char mNodeName[32];
    };

    dWmCullCsvData_c() { }

    void init();
    void readData();

    void readDistance(char *data, int *index);
    void readNodeName(char *data, int *index);

    int mNodeNum;
    Node_s mNodes[30];

    static dWmCullCsvData_c m_instance;
};