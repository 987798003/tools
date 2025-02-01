#ifndef _DEVICE_MR_
#define _DEVICE_MR_

#include "device.h"
#include "register_mr.h"

class IxDeviceMR : public IxDevice
{
public:
    IxDeviceMR(string busId);

    bool initMailbox() override;
    bool putDataToHBM(const void *data, uint32_t size) override;
    bool setMailbox(uint32_t cmd, uint32_t data_1, uint32_t data_2, uint32_t data_3, uint32_t data_4) override;
    bool checkMailbox() override;
    bool warnReset() override;
    bool resetDevice() override;
    bool upgradeBySRAM(uint8_t *data, size_t length) override;
    bool burnSNByHBM(const void *data, uint32_t size) override;

public:
    string deviceBusId = "";
};

#endif // _DEVICE_MR_