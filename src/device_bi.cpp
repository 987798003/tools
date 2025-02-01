#include "device_bi.h"

IxDeviceBI::IxDeviceBI(string deviceBusId) : IxDevice(deviceBusId), deviceBusId(deviceBusId){}

bool IxDeviceBI::initMailbox() 
{
    int v, i;
    int srcid[8] = { 0, 0, 0, 1, 1, 0, 1, 1 };

    for (i = 0; i < 4; i++) {
        /* Test the write */
        w_srcid = srcid[i * 2];
        writeReg(REGG_SCPU__MAILBOX_ACC_SRCID, w_srcid);
        writeReg(REGG_SCPU__MAILBOX0_1, ITR_TEST_VAL);

        /* Test the read */
        r_srcid = srcid[i * 2 + 1];
        writeReg(REGG_SCPU__MAILBOX_ACC_SRCID, r_srcid);
        v = readReg(REGG_SCPU__MAILBOX0_1);

        if (v == ITR_TEST_VAL)
            return true;

        /* Init again */
        writeReg(REGG_SCPU__MAILBOX_ACC_SRCID, w_srcid);
        writeReg(REGG_SCPU__MAILBOX0_1, 0);
    }

    printf("Error: cannot find the proper srcid for mailbox\n");
    return false;
}

bool IxDeviceBI::putDataToHBM(const void *data, uint32_t size)
{
    unsigned int v = readReg(BI_REG_HMBG_BASE + REGG_HMBG__HMBG_CNTL);
    v = (v & 0xf0ffffff) | 0x0c000000;

    writeReg(BI_REG_HMBG_BASE + REGG_HMBG__HMBG_CNTL, v);
    writeMemory(FW_MEM_BASE, size, data);

    return true;
}

bool IxDeviceBI::setMailbox(uint32_t cmd, uint32_t data_1, uint32_t data_2, uint32_t data_3, uint32_t data_4)
{
    unsigned int value;
    int maxTryCount = 11;
    int tryCount = 0;
    int delay = 1;

    value = readReg(REGG_SCPU__SCRATCH_3);
    if (value != REGG_SCPU__MAILBOX_READY) {
        printf("Mailbox not ready, status %x\n", value);
        return false;
    }

    // Write mailbox.
    writeReg(REGG_SCPU__MAILBOX_ACC_SRCID, w_srcid);

    /* Put the val in registers */
    writeReg(REGG_SCPU__MAILBOX0_1, data_1);
    writeReg(REGG_SCPU__MAILBOX0_2, data_2);
    writeReg(REGG_SCPU__MAILBOX0_3, data_3);
    writeReg(REGG_SCPU__MAILBOX0_4, data_4);

    /* Put the commond in 0_0, and fire it */
    writeReg(REGG_SCPU__MAILBOX0_0, cmd);

    /* Wait Mailbox becomes ready */
    for (tryCount = 0; tryCount < maxTryCount; tryCount++) {
        sleep(delay);
        value = readReg(REGG_SCPU__MAILBOX0_STATUS);
        if (value != MAILBOX_BUSY) {
            break;
	}

	    if (tryCount == (maxTryCount - 1)) {
            printf("Mailbox: no respond.\n");
	        return false;
	    }
    }

    writeReg(REGG_SCPU__MAILBOX_ACC_SRCID, r_srcid);

    return true;
}

bool IxDeviceBI::checkMailbox()
{
    writeReg(REGG_SCPU__MAILBOX_ACC_SRCID, r_srcid);
    unsigned int value = readReg(REGG_SCPU__MAILBOX0_1);
    if (value != MAILBOX_SET_OK) {
	    printf("Mailbox: read/write flash failure.\n");
        int val = readReg(MAILBOX_SET_STATUS_REG);
        printf("MAILBOX_SET_STATUS_REG: %x\n", val);
	    return false;
    }

    return true;
}

bool IxDeviceBI::warnReset()
{
    /* stop SCPU scheduling first */
    setMailbox(ITR_SCHED_CMD, ITR_CMD_OFF, 0, 0, 0);
    usleep(2000);

    /* Disable the PMU */
    setMailbox(ITR_PMU_MONITOR, ITR_CMD_OFF, 0, 0, 0);
    setMailbox(ITR_PVT, ITR_CMD_OFF, 0, 0, 0);
    setMailbox(ITR_SET_CLK, 0, 0, 0, 0x8006804d);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_CENTRAL_BASE + REGG_CLK_RST_CENTRAL__FW_CTRL, 0x00084);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_SYS_BASE + REGG_CLK_RST_PLL_SYS__WARM_RSTN_LOW, 0x20002);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_SCU_BASE + REGG_CLK_RST_PLL_SCU__WARM_RSTN, 0x00002);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_SCPU_BASE + REGG_CLK_RST_PLL_SCPU__WARM_RSTN, 0x00000);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_MC_BASE + REGG_CLK_RST_PLL_MC__WARM_RSTN, 0x00000);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_MC_BASE + REGG_CLK_RST_PLL_MC__WARM_RSTN, 0x01010);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_SCU_BASE + REGG_CLK_RST_PLL_SCU__WARM_RSTN, 0x00817);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_SYS_BASE + REGG_CLK_RST_PLL_SYS__WARM_RSTN_LOW, 0x2081f);
    usleep(10000);

    writeReg(BI_REG_CLK_RST_PLL_SCPU_BASE + REGG_CLK_RST_PLL_SCPU__WARM_RSTN, 0x01010);
    usleep(10000);
    return true;
}

bool IxDeviceBI::resetDevice()
{
    return true;
}

bool IxDeviceBI::upgradeBySRAM(uint8_t *data, size_t length)
{
    return true;
}

bool IxDeviceBI::burnSNByHBM(const void *data, uint32_t size)
{
    return initialDevice() 
    && initMailbox() 
    && putDataToHBM(data, size) 
    && checkDataInHBM(data, size) 
    && setMailbox(CMD_WRITE_SN | LEN(size), 320 * 1024, 0, 0, 0) 
    && checkMailbox() 
    && warnReset();
}