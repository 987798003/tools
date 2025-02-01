#include "device_mr.h"

IxDeviceMR::IxDeviceMR(string busId) : IxDevice(busId), deviceBusId(busId){}

bool IxDeviceMR::initMailbox() 
{
    return true;
}

bool IxDeviceMR::putDataToHBM(const void *data, uint32_t size)
{
    writeMemory(FW_MEM_BASE, size, data);
    return true;
}

bool IxDeviceMR::setMailbox(uint32_t cmd, uint32_t data_1, uint32_t data_2, uint32_t data_3, uint32_t data_4)
{
    uint32_t value;
    int maxTryCount = 200;
    int tryCount = 0;
    int delay = 100000;

    /* Wait Mailbox becomes ready */
    for (tryCount = 0; tryCount < maxTryCount; tryCount++) {
        // Replace mdelay.
        usleep(delay);
        value = readReg(REGG_SCPU__MAILBOX0_STATUS);
        if (value != MAILBOX_BUSY) {
            break;
        }

        if (tryCount == (maxTryCount - 1)) {
            printf("Mailbox: no respond.\n");
            return false;
        }
    }
    
    /* reset mailbox */
    writeReg(REGG_SCPU__MAILBOX0_4, 0);
    writeReg(REGG_SCPU__MAILBOX0_3, 0);
    writeReg(REGG_SCPU__MAILBOX0_2, 0);
    writeReg(REGG_SCPU__MAILBOX0_1, data_1);
    writeReg(REGG_SCPU__MAILBOX0_0, cmd);

    uint32_t value2 = readReg(REGG_SCPU__MAILBOX0_1);
    /* Wait Mailbox becomes ready */
    for (tryCount = 0; tryCount < maxTryCount; tryCount++) {
        // Replace mdelay.
        usleep(delay);
        value = readReg(REGG_SCPU__MAILBOX0_STATUS);
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

bool IxDeviceMR::checkMailbox()
{
    return true;

    bool isMailboxReady = false;
    unsigned int value, value2;
    value = readReg(REGG_SCPU__MAILBOX0_1);

    switch (value) {
        case MR_MAILBOX_SET_FAIL:
            printf("Error: write failure.\n");
            break;
        case MR_MAILBOX_SET_ERROR_LEN:
            printf("Error: wrong firmware data length.\n");
            break;
        case MR_MAILBOX_SET_ERROR_CRC:
            printf("Error: wrong crc.\n");
            break;
        case MR_MAILBOX_SET_ERROR_NULL:
            printf("Error: null firmware data.\n");
            break;
        case MR_MAILBOX_SET_ERROR_RW:
            printf("Error: read/write flash failure.\n");
            break;
        default:
            isMailboxReady = true;
            // printf("Error: unknown error.\n");
    }

    value2 = readReg(MAILBOX_SET_STATUS_REG);
    bitset<32> binaryRepresentation(value2);
    
    for (size_t i = 0; i < binaryRepresentation.size(); ++i) {
        if (binaryRepresentation.test(i)) {
            unsigned int bit_value = 0x0001 << i;
            if (mr_exception_table.find(bit_value) != mr_exception_table.end()){
                printf("Check mailbox error: %s\n", mr_exception_table.find(bit_value)->second.c_str());           
            }
        } 
    } 

    return isMailboxReady;
}

bool IxDeviceMR::warnReset()
{
    return true;
    writeReg(MR_REG_CLK_RST_CENTRAL_BASE + REGG_CLK_RST_CENTRAL__FW_CTRL, 0x84);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SYS_BASE + REGG_CLK_RST_PLL_SYS__WARM_RSTN_LOW, 0x20003);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SYS_BASE + REGG_CLK_RST_PLL_SYS__WARM_RSTN_HIGH, 0x20);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SYS_BASE + REGG_CLK_RST_PLL_SYS__WARM_DEC_RSTN, 0x0);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SCU_BASE + REGG_CLK_RST_PLL_SCU__WARM_RSTN, 0x3);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SCPU_BASE + REGG_CLK_RST_PLL_SCPU__WARM_RSTN, 0x0);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_MC_BASE + REGG_CLK_RST_PLL_MC__WARM_RSTN, 0x0);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_DEC_DCLK_BASE + REGG_CLK_RST_PLL_DEC_DCLK__WARM_RSTN, 0x0);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_DEC_ICCLK_BASE + REGG_CLK_RST_PLL_DEC_ICCLK__WARM_RSTN, 0x0);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_DEC_VBCLK_BASE + REGG_CLK_RST_PLL_DEC_VBCLK__WARM_RSTN, 0x0);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_DEC_VCCLK_BASE + REGG_CLK_RST_PLL_DEC_VCCLK__WARM_RSTN, 0x0);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SCU_BASE + REGG_CLK_RST_PLL_SCU__WARM_RSTN, 0x817);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SYS_BASE + REGG_CLK_RST_PLL_SYS__WARM_RSTN_LOW, 0x2081f);
    usleep(10000);

    writeReg(MR_REG_CLK_RST_PLL_SCPU_BASE + REGG_CLK_RST_PLL_SCPU__WARM_RSTN, 0x1010);
    usleep(10000);
}

bool IxDeviceMR::resetDevice()
{
    return true;
}

bool IxDeviceMR::upgradeBySRAM(uint8_t *data, size_t length)
{
    uint32_t dataStartingPoint, sramCmdWrite, writeValue;
    size_t cycles, remainingBytes, chunkSize;

    dataStartingPoint = MR_DATA_STARTING_POINT;
    sramCmdWrite = MR_SRAM_CMD_WRITE;
    cycles = length / MR_CHUNK_SIZE;
    remainingBytes = length % MR_CHUNK_SIZE;
    chunkSize = MR_CHUNK_SIZE;

    for (size_t i = 0; i < cycles; ++i) {
        writeReg(RAM_ADDR_REG, dataStartingPoint);
        for (size_t j = 0; j < chunkSize; j += 4) {
            writeValue = *reinterpret_cast<uint32_t*>(data + j);
            writeReg(RAM_DATA_REG, writeValue);
        }
        if (setMailbox(sramCmdWrite | LEN(length), 0, 0, 0, 0)) {
            data += chunkSize;
        } else {
            return false;
        }
    }

    if (remainingBytes) {
        writeReg(RAM_ADDR_REG, dataStartingPoint);
        for (size_t i = 0; i < remainingBytes; i += 4) {
            writeValue = *reinterpret_cast<uint32_t*>(data + i);
            writeReg(RAM_DATA_REG, writeValue);
        }
        return setMailbox(sramCmdWrite | LEN(length), 0, 0, 0, 0);
    } 

    return true;
}

bool IxDeviceMR::burnSNByHBM(const void *data, uint32_t size)
{
    return initialDevice() 
    && initMailbox() 
    && putDataToHBM(data, size) 
    && checkDataInHBM(data, size) 
    && setMailbox(0x12 | LEN(size), 320 * 1024, 0, 0, 0) 
    && checkMailbox() 
    && warnReset();
}