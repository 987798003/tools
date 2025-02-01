#ifndef IXDEVICE_H_
#define IXDEVICE_H_

#include <iostream>
#include <cstdint> // uint32_t 定义
#include <string>
#include <memory>
#include <csignal>
#include <csetjmp>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <bitset>
#include <map>

#include "utils.h"
#include "common.h"

#define MAILBOX_BUSY                                    0x01
#define MAILBOX_SET_OK                                  0x01
#define FW_MEM_BASE                                     0x800
#define RAM_ADDR_REG                                    0x900c
#define RAM_DATA_REG                                    0x9010
#define MAILBOX_SET_STATUS_REG                          0x9180

#define REGG_CLK_RST_CENTRAL__FW_CTRL                   0x4
#define REGG_CLK_RST_PLL_SYS__WARM_RSTN_LOW             0x200
#define REGG_CLK_RST_PLL_SCU__WARM_RSTN	                0x200
#define REGG_CLK_RST_PLL_SCPU__WARM_RSTN                0x200
#define REGG_CLK_RST_PLL_MC__WARM_RSTN                  0x200

#define REGG_SCPU__SCRATCH_3                            0x917c
#define REGG_SCPU__SCRATCH_4                            0x9180
#define REGG_SCPU__MAILBOX_READY                        0x66
#define REGG_SCPU__MAILBOX_ACC_SRCID                    0x91a0
#define REGG_SCPU__MAILBOX0_0                           0x9128
#define REGG_SCPU__MAILBOX0_1                           0x912c
#define REGG_SCPU__MAILBOX0_2                           0x9130
#define REGG_SCPU__MAILBOX0_3                           0x9134
#define REGG_SCPU__MAILBOX0_4                           0x9138
#define REGG_SCPU__MAILBOX0_STATUS                      0x9148
#define REGG_HMBG__HMBG_CNTL                            0x0

using namespace std;

struct IxBoard {
    string deviceBusId;
    string snNumber;
    string pnNumber;
    string dateNumber;
    string deviceName;
    uint32_t deviceId;
    uint32_t customId;
    uint32_t productId;
    uint32_t boardId;
};

class IxDevice
{
public:
    IxDevice(string deviceBusId);
    virtual ~IxDevice();

    bool initialDevice();
    uint32_t readReg(uint32_t addr); 
    bool writeReg(uint32_t addr, uint32_t value);
    uint32_t readMemory();
    bool writeMemory(int addr, uint32_t size, const void *data);
    // 校验写入到HBM中的数据
    bool checkDataInHBM(const void *data, uint32_t size);

    // 数据写入到HBM中
    virtual bool putDataToHBM(const void *data, uint32_t size) = 0;
    // Mailbox下发cmd
    virtual bool setMailbox(uint32_t cmd, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t data4) = 0;
    // 初始化Mailbox
    virtual bool initMailbox() = 0;
    // 检查Mailbox
    virtual bool checkMailbox() = 0;
    // 热复位
    virtual bool warnReset() = 0;
    // reset device
    virtual bool resetDevice() = 0;
    // 数据分块循环写入SRAM，每次写完Mailbox下发一次cmd
    virtual bool upgradeBySRAM(uint8_t *data, size_t length) = 0;
    // 烧录SN
    virtual bool burnSNByHBM(const void *data, uint32_t size) = 0;
    
public:
    uint32_t deviceId, customId, productId, boardId;
    string deviceBusId = "", productName = "";
    int memHandle = -1, regHandle = -1;
    int r_srcid = 0, w_srcid = 0;
};

#endif // IXDEVICE_H_