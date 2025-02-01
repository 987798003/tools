#include "fileoperate.h"

unsigned short FileOperator::crc16cal(const vector<unsigned char>& data)
{
    unsigned short a = 0x0000;
    unsigned short b = 0x1021;
    for (const auto& byte : data) {
        a ^= (byte & 0xFF) << 8;
        for (int i = 0; i < 8; ++i) {
            if (a & 0x8000) {
                a = (a << 1) ^ b;
            } else {
                a = a << 1;
            }
        }
        a &= 0xFFFF;
    }
    return a;
}

void FileOperator::generateSnBinaryFile(const string& productDate, const string& sn_h, const string& sn_m, const string& sn_l, string& snBinFile)
{
    string sn_m_tmp = sn_m;
    string new_sn_m = sn_m_tmp;

    while (new_sn_m.length() < 6) {
        new_sn_m = "0" + new_sn_m;
    }
    string boardSn = sn_h + new_sn_m + sn_l;
    int sn_len = boardSn.length();
    int date_len = productDate.length();
    int pn_len = devicePnNumber.length();
    snBinFile = "sn_" + boardSn + "_" + productDate + "_" + devicePnNumber + ".bin";

    vector<unsigned char> binList;
    ofstream fout(snBinFile, ios::binary);
    if (!fout) {
        cout << "Error!!! create sn file failed" << endl;
        return;
    } else {
        cout << "Generate Bin File: " << snBinFile << endl;
        if (sn_len % 2 != 0) {
            cout << "Error!!! invalid serial number length" << endl;
            return;
        }
    }

    unsigned char sn0, sn1, sn2, sn3, sn4,sn5,sn6;
    for (int i = 0; i < sn_len / 2; ++i) {
        int index = i * 2;
        unsigned char high = boardSn[index];
        unsigned char low = boardSn[index + 1];
        
        if (high >= '0' && high <= '9') {
            high -= '0';
        } else if (high >= 'a' && high <= 'f') {
            high = high - 'a' + 10;
        } else if (high >= 'A' && high <= 'F') {
            high = high - 'A' + 10;
        } else {
            high = 0;
        }
        
        if (low >= '0' && low <= '9') {
            low -= '0';
        } else if (low >= 'a' && low <= 'f') {
            low = low - 'a' + 10;
        } else if (low >= 'A' && low <= 'F') {
            low = low - 'A' + 10;
        } else {
            low = 0;
        }

        unsigned char byteValue = (high << 4) | low;
        if (i == 0) {
            sn0 = byteValue;
        } else if (i == 1) {
            sn1 = byteValue;
        } else if (i == 2) {
            sn2 = byteValue;
        } else if (i == 3) {
            sn3 = byteValue;
        } else if (i == 4) {
            sn4 = byteValue;
        } else if (i == 5) {
            sn5 = byteValue;
        } else if (i == 6) {
            sn6 = byteValue;
        }
    }

    binList.push_back(sn6);
    binList.push_back(sn5);
    binList.push_back(sn4);    
    binList.push_back(sn3);
    binList.push_back(sn2);
    binList.push_back(sn1);
    binList.push_back(sn0);
    binList.push_back(0xff);

    unsigned char byteValue0, byteValue1, byteValue2, byteValue3;
    for (int i = 0; i < pn_len / 2; ++i) {
        int index = i * 2;
        unsigned char high = devicePnNumber[index];
        unsigned char low = devicePnNumber[index + 1];
        
        if (high >= '0' && high <= '9') {
            high -= '0';
        } else {
            high = high - 'a' + 10;
        }
        
        if (low >= '0' && low <= '9') {
            low -= '0';
        } else {
            low = low - 'a' + 10;
        }
        
        unsigned char byteValue = (high << 4) | low;
        if (i == 0) {
            byteValue0 = byteValue;
        } else if (i == 1) {
            byteValue1 = byteValue;
        } else if (i == 2) {
            byteValue2 = byteValue;
        } else if (i == 3) {
            byteValue3 = byteValue;
        }
    }
    binList.push_back(byteValue3);
    binList.push_back(byteValue2);
    binList.push_back(byteValue1);
    binList.push_back(byteValue0);

    for (int i = 0; i < 4; ++i) {
        binList.push_back(0xFF);
    }

    int year = 0;
    int month = 0;
    int day = 0;
    for (int i = 0; i < date_len; ++i) {
        int index = 7 - i;
        unsigned char low = productDate[index];
        if ((low >= '0' && low <= '9') || (low >= 'A' && low <= 'F') || (low >= 'a' && low <= 'f')) {
            if (low >= '0' && low <= '9') {
                low -= '0';
            } else if (low >= 'A' && low <= 'F') {
                low -= 'A' + 10;
            } else {
                low -= 'a' + 10;
            }
        } else {
            cout << "Error!!! invalid product date" << endl;
            return;
        }

        if (index == 0) {
            year += low * 1000;
            binList.push_back(year & 0xFF);
            binList.push_back((year & 0xFF00) >> 8);
        } else if (index == 1) 
            year += low * 100;
        else if (index == 2) 
            year += low * 10;
        else if (index == 3) 
            year += low;
        else if (index == 4) { 
            month += low * 10;
            binList.push_back(month & 0xF);
        }
        else if (index == 5) 
            month += low;
        else if (index == 6) { 
            day += low * 10;
            binList.push_back(day & 0x1F);
        }
        else if (index == 7) 
            day += low;
    }

    for (int i = 0; i < 26; ++i)
        binList.push_back(0xFF);

    unsigned short crc16 = crc16cal(vector<unsigned char>(binList.begin(), binList.begin() + 46));
    binList.push_back(crc16 & 0xFF);
    binList.push_back((crc16 >> 8) & 0xFF);

    for (int i = 0; i < 208; ++i)
        binList.push_back(0xFF);

    for (const auto& byte : binList)
        fout.write(reinterpret_cast<const char*>(&byte), sizeof(byte));

    fout.close();
}

string FileOperator::currentTime()
{
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    tm* localTime = localtime(&now_c);

    char timeBuffer[20];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d", localTime);

    return string(timeBuffer);
}

void FileOperator::generateFiles(string& boardSn, string& date, string& snBinFile)
{
    string productDate;
    if (date.empty())
        productDate = currentTime();
    else
        productDate = date;

    string str1 = boardSn.substr(0, 4);
    string str2 = boardSn.substr(4, 6);
    string str3 = boardSn.substr(10, 4);

    generateSnBinaryFile(productDate, str1, str2, str3, snBinFile);
}

uint32_t FileOperator::readSNBinFile(const char *file, void **data)
{
    FILE *fileFp = fopen(file, "rb");
    if (fileFp == NULL)
	    printf("Error: failed to open file %s\n", file);

    fseek(fileFp, 8 ,SEEK_SET);
    unsigned char buffer[4];
    size_t bytesRead = fread(buffer, 1, 4, fileFp);
    stringstream valueStream;
    valueStream << std::hex << std::setfill('0') << std::setw(2);
    for (int i = 3; i >= 0; --i) {
        valueStream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]);
    }
    string pnNumber = valueStream.str();

    // 从 BDF 映射的pn码与文件解析的做对比
    if (pnNumber != devicePnNumber) {
        printf("PN verification is error or the SN file is error. \n");
        return 0;
    }

    fseek(fileFp, 0, SEEK_SET);
    unsigned char size = 48;
    char *fileData = (char *)malloc(size);
    fread(fileData, 1, size, fileFp);

    *data = (uint8_t *)malloc(size);
    memcpy(*data, fileData, size);
    free(fileData);
    fclose(fileFp);
    return size;
}

// 检查输入 sn && pn 号 是否合法
bool FileOperator::checkDeviceNumFormat(string& boardSn, string& devicePn)
{
    if (!boardSn.empty()) {
        if (boardSn.length() == 14) {
            for (int i = 0; i < boardSn.length(); i++) {
                if (boardSn[i] < '0' || boardSn[i] > 'f') {
                    printf("The boardSn is error. \n");
                    return false;
                }
            }
        } else {
            printf("The length of boardSn is error. \n");
            return false;
        }
    }

    if (!devicePn.empty()) {
        if (devicePn.length() == 8) {
            for (int i = 0; i < devicePn.length(); i++) {
                if (devicePn[i] < '0' || devicePn[i] > 'f') {
                    printf("The devicePn is error. \n");
                    return false;
                }
            }
        } else {
            printf("The length of devicePn is error. \n");
            return false;
        }
    }

    return true;
}