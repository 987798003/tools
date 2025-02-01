#include "enmudevice.h"

template<typename T>
bool EnmuDevice::isInVector(const vector<T>& vec, const T& value)
{
    return find(vec.begin(), vec.end(), value) != vec.end();
}

string EnmuDevice::getDeviceName(const string& deviceBus)
{
    string deviceName, deviceIdStr;
    string deviceFile = PCI_DEVICE_PREFIX + deviceBus + "/device";
    ifstream ixDeviceStream(deviceFile);
    ixDeviceStream >> deviceIdStr;
    unsigned int deviceId = stoi(deviceIdStr, nullptr, 16);


    unsigned int cardInfo = getDeviceInfoByReg(deviceBus, MR_PRODUCT_ID_REG);
    unsigned int customerIdInfo = getDeviceInfoByReg(deviceBus, MR_CUSTOM_ID_REG);
    int productId = (cardInfo >> 25) & 0x1f;
    unsigned int boardId = cardInfo & 0x7;
    unsigned int customerId = customerIdInfo & 0xffffffff;

    if (deviceId == BI100_SERIES) {
        deviceName = "BI_V100";
    } else if (deviceId == BI150X_SERIES) {
        deviceName = "BI_V150X";
    } else if (deviceId == BI150S_SERIES) {
        deviceName = "BI_V150S";
    } else if (deviceId == BI150OAM_SERIES) {
        deviceName = "BI_V150OAM";
    } else if (deviceId == QS200_SERIES) {
        if (isTGPcieBoard(deviceBus))
            deviceName = "TG_V200PCIE";
        else
            deviceName = "TG_V200OAM";
    } else {
        if (IS_STANDARD_CARD != customerId) {
            if (BI_V150_C_CUSTOMER_ID == customerId)
                deviceName = "BI_V150C";
            else
                deviceName = "MR-" + to_string(productId);
        }
        else {
            if (isInVector(mr50_buf, productId)) {
                if ((productId == 0x1d) && (boardId == 0x01))
                    deviceName = "MR_V100";
                else
                    deviceName = "MR_V50";
            }
            else if (isInVector(mr100_buf, productId))
                deviceName = "MR_V100";
            else if (isInVector(bi150_buf, productId))
                if ((productId == 0x10) && (boardId == 0x01))
                    deviceName = "BI_V150OAM";
                else
                    deviceName = "BI_V150";
            else
                deviceName = "MR-" + to_string(productId);
        }
    }

    return deviceName;
}

string EnmuDevice::getSnNumber(const string& deviceBus, uint32_t deviceId)
{
    unsigned int value0, value1;
    if (deviceId == BI100_SERIES) {
        value0 = getDeviceInfoByReg(deviceBus, 0x2418 * 4);
        value1 = getDeviceInfoByReg(deviceBus, 0x2463 * 4);
    } else if (deviceId == QS200_SERIES) {
        value0 = getDeviceInfoByReg(deviceBus, 0x72dc);
        value1 = getDeviceInfoByReg(deviceBus, 0x72d8);
    } else {
        value0 = getDeviceInfoByReg(deviceBus, 0x78dc);
        value1 = getDeviceInfoByReg(deviceBus, 0x78d8);
    }

    char snNumber[8];
    sprintf(snNumber, "%06x%08x", value0, value1);
    return snNumber;
}

string EnmuDevice::getBoardProductDate(const string& deviceBus, uint32_t deviceId)
{
    unsigned int value0, value1;
    unsigned int year, month, day;
    if (deviceId == BI100_SERIES) {
        value0 = getDeviceInfoByReg(deviceBus, 0x1e07 * 4);
    } else if (deviceId == QS200_SERIES) {
        value0 = getDeviceInfoByReg(deviceBus, 0x72c4);
    } else {
        value0 = getDeviceInfoByReg(deviceBus, 0x78c4);
    }

    year  = (value0 >> 16) & 0xfff;
    month = (value0 >> 8)  & 0x0f;
    day   = (value0 >> 0)  & 0x1f;

    stringstream valueStream;
    valueStream << setw(4) << setfill('0') << year;
    valueStream << setw(2) << setfill('0') << month;
    valueStream << setw(2) << setfill('0') << day;
    string snNumber = valueStream.str();
    return snNumber;
}

string EnmuDevice::getPnSerial(const string& deviceBus)
{
    unsigned int value;
    string pnNumber;
    uint32_t deviceId = getDeviceId(deviceBus);
    if (deviceId == BI100_SERIES)
        value = getDeviceInfoByReg(deviceBus, 0x2461 * 4);
    else if (deviceId == QS200_SERIES)
        value = getDeviceInfoByReg(deviceBus, 0x72d0);
    else
        value = getDeviceInfoByReg(deviceBus, 0x78d0);

    stringstream valueStream;
    valueStream << hex << setw(8) << setfill('0') << value;
    pnNumber = valueStream.str();
    return pnNumber;
}

string EnmuDevice::getPnName(const string& deviceBus)
{
    unsigned int value0, value1;
    string pnNumber, boardName;
    uint32_t deviceId = getDeviceId(deviceBus);
    if (deviceId == BI100_SERIES)
        value0 = getDeviceInfoByReg(deviceBus, 0x2461 * 4);
    else if (deviceId == QS200_SERIES)
        value0 = getDeviceInfoByReg(deviceBus, 0x72d0);
    else
        value0 = getDeviceInfoByReg(deviceBus, 0x78d0);

    unsigned int nameBit     = (value0 >> 24) & 0xff;
    unsigned int reserveBit  = (value0 >> 20) & 0x0f;
    unsigned int typeBitHigh = (value0 >> 8)  & 0xfff;
    unsigned int typeBitLow  = (value0 >> 0)  & 0Xff;

    if (nameBit == 0x00)
        boardName = "BI";
    else if (nameBit == 0x01)
        boardName = "MR";
    else if (nameBit == 0x02)
        boardName = "TG";
    else
        boardName = "XX";

    stringstream valueStream;
    valueStream << boardName << "-V" << hex << typeBitHigh << "-" << setw(2) << setfill('0') << typeBitLow;
    pnNumber = valueStream.str();
    return pnNumber;
}

void EnmuDevice::initPciDevice()
{
    vector<string> devices = GetAllDeviceBusId();

    for (const auto& busId : devices) {
        IxBoard *boardObj = new IxBoard();

        boardObj->deviceBusId = busId;
        boardObj->deviceId = getDeviceId(busId);
        boardObj->boardId  = getBoardId(busId, boardObj->deviceId);
        boardObj->productId = getProductId(busId, boardObj->deviceId);
        boardObj->customId = getCustomId(busId, boardObj->deviceId);
        boardObj->deviceName = getDeviceName(busId);
        boardObj->snNumber = getSnNumber(busId, boardObj->deviceId);
        boardObj->pnNumber = getPnSerial(busId);
        boardObj->dateNumber = getBoardProductDate(busId, boardObj->deviceId);       

        allBoards.push_back(boardObj);
    }

    sort(allBoards.begin(), allBoards.end(), [](IxBoard* a, IxBoard* b) {
        return a->deviceBusId < b->deviceBusId;
    });
}