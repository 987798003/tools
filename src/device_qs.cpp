#include "device_qs.h"

IxDeviceQS::IxDeviceQS(string deviceBusId) : IxDevice(deviceBusId), deviceBusId(deviceBusId){}

bool IxDeviceQS::initMailbox() 
{
    return true;
}

bool IxDeviceQS::putDataToHBM(const void *data, uint32_t size)
{
    writeMemory(QS_MEM_FW_START, size, data);
    return true;
}

bool IxDeviceQS::setMailbox(uint32_t cmd, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t data4)
{
    uint32_t value;
    // Increase the number of polls, shorten each time.
    int maxTryCount = 200;
    int tryCount = 0;
    int delay = 100000;

    /* Wait Mailbox becomes ready */
    for (tryCount = 0; tryCount < maxTryCount; tryCount++) {
        // Replace mdelay.
        usleep(delay);
        value = readReg(QS_REG_SCPU__MAILBOX0_STATUS);
        if (value != MAILBOX_BUSY) {
            break;
        }

        if (tryCount == (maxTryCount - 1)) {
            printf("Mailbox: no respond.\n");
            return false;
        }
    }

    /* reset mailbox */
    writeReg(QS_REG_SCPU__MAILBOX0_7, 0);
    writeReg(QS_REG_SCPU__MAILBOX0_6, 0);
    writeReg(QS_REG_SCPU__MAILBOX0_5, 0);
    writeReg(QS_REG_SCPU__MAILBOX0_4, 0);
    writeReg(QS_REG_SCPU__MAILBOX0_3, 0);
    writeReg(QS_REG_SCPU__MAILBOX0_2, 0);
    writeReg(QS_REG_SCPU__MAILBOX0_1, data1);
    writeReg(QS_REG_SCPU__MAILBOX0_0, cmd);

    /* Wait Mailbox becomes ready */
    for (tryCount = 0; tryCount < maxTryCount; tryCount++) {
        // Replace mdelay.
        usleep(delay);
        value = readReg(QS_REG_SCPU__MAILBOX0_STATUS);
        if (value != MAILBOX_BUSY) {
            break;
        }

        if (tryCount == (maxTryCount - 1)) {
            printf("Mailbox: no respond.\n");
            return false;
        }
    }

    return true;
}

bool IxDeviceQS::checkMailbox()
{
    uint32_t qsMailbox0_status = readReg(QS_REG_SCPU__MAILBOX0_STATUS);
    uint32_t qsMailbox0_7 = readReg(QS_REG_SCPU__MAILBOX0_7);
    uint32_t qsScratch4 = readReg(QS_REG_SCPU__SCRATCH_4);
    bool isMailboxReady = true;

    bitset<32> binaryRepresentationMailbox0_7(qsMailbox0_7);
    for (size_t i = 0; i < binaryRepresentationMailbox0_7.size(); ++i) {
        if (binaryRepresentationMailbox0_7.test(i)) {
            uint32_t bitValue = 0x0001 << i;
            if (qsMailbox0_7ErrorTable.find(bitValue) != qsMailbox0_7ErrorTable.end()){
                printf("Mailbox0_7 error: %s\n", qsMailbox0_7ErrorTable.find(bitValue)->second.c_str());
                isMailboxReady = false;
            }
        } 
    } 

    bitset<32> binaryRepresentationScratch4(qsScratch4);
    for (size_t i = 0; i < binaryRepresentationScratch4.size(); ++i) {
        if (binaryRepresentationScratch4.test(i)) {
            uint32_t bitValue = 0x0001 << i;
            if (qsFirmwareScratch4ErrorTable.find(bitValue) != qsFirmwareScratch4ErrorTable.end()){
                printf("FW scratch4 error: %s\n", qsFirmwareScratch4ErrorTable.find(bitValue)->second.c_str());  
                isMailboxReady = false;       
            }
        } 
    }

    return isMailboxReady;
}

bool IxDeviceQS::warnReset()
{
    // cout << "Please reboot ... " << endl;
    return true;
}

bool IxDeviceQS::resetDevice()
{
    return true;
}

bool IxDeviceQS::upgradeBySRAM(uint8_t *data, size_t length)
{
    return true;
}

bool IxDeviceQS::burnSNByHBM(const void *data, uint32_t size)
{
    return initialDevice() 
    && initMailbox() 
    && putDataToHBM(data, size) 
    && checkDataInHBM(data, size) 
    && setMailbox(QS_CMD_WRITE_SN, size, 0, 0, 0) 
    && checkMailbox() 
    && warnReset();
} 