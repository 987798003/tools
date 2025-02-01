#ifndef ENMU_DEVICE_H_
#define ENMU_DEVICE_H_

#include <iostream>
#include <memory>

#include "fileoperate.h"
#include "device.h"
#include "common.h"

class EnmuDevice
{
public:
    void initPciDevice();
    string getPnSerial(const string& deviceBus);
    string getPnName(const string& deviceBus);
    string getDeviceName(const string& deviceBus);
    string getBoardProductDate(const string& deviceBus, uint32_t deviceId);
    string getSnNumber(const string& deviceBus, uint32_t deviceId);

public:
    vector<IxBoard*> allBoards;

private:
    template<typename T>
    bool isInVector(const vector<T>& vec, const T& value);
    vector<int> mr50_buf  = {0x1d, 0x01, 0x02, 0x05, 0x07, 0x0c, 0x1c};
    vector<int> mr100_buf = {0x00};
    vector<int> bi150_buf = {0x03, 0x13, 0x10};
};

#endif // ENMU_DEVICE_H_