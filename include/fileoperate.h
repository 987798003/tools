#ifndef _FILEOPERATE_H_
#define _FILEOPERATE_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "enmudevice.h"

using namespace std;

#define SN_MR100      "01010000"
#define SN_MR50       "01005000"
#define SN_BI150      "00015000"
#define SN_BI150S     "00015001"
#define SN_BI1500AM   "00015002"
#define SN_BI150X     "00015003"
#define SN_BI100      "00010000"
#define SN_TG200_OAM  "02020001"
#define SN_TG200_PCIE "02020002"

class FileOperator {
public:
    string currentTime();
    unsigned short crc16cal(const vector<unsigned char>& data);
    void generateSnBinaryFile(const string& productDate, const string& sn_h, const string& sn_m, const string& sn_l, string& snBinFile);
    void generateFiles(string& boardSn, string& date, string& snBinFile);
    uint32_t readSNBinFile(const char *file, void **data);
    bool checkDeviceNumFormat(string& boardSn, string& devicePn);

public:
    string devicePnNumber;

    unordered_map<string, string> devidPnMap = {
        {"MR_V100",       SN_MR100},
        {"MR_V50" ,       SN_MR50 },
        {"BI_V150",       SN_BI150},
        {"BI_V150S",      SN_BI150S},
        {"BI_V150X",      SN_BI150X},
        {"BI_V150OAM",    SN_BI1500AM},
        {"BI_V150C",      SN_BI150},
        {"BI_V100",       SN_BI100},
        {"TG_V200OAM",    SN_TG200_OAM},
        {"TG_V200PCIE",   SN_TG200_PCIE},
    };
};

#endif // _FILEOPERATE_H_