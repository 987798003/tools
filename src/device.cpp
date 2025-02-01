#include "device.h"

IxDevice::IxDevice(string deviceBusId) : deviceBusId(deviceBusId) 
{
}

IxDevice::~IxDevice()
{
    if (memHandle != -1) 
        close(memHandle);

    if (regHandle != -1) 
        close(regHandle);
}

bool IxDevice::initialDevice()
{
    string mem_source_path = string(PCI_DEVICE_PREFIX) + deviceBusId + "/resource0";
    string reg_source_path = string(PCI_DEVICE_PREFIX) + deviceBusId + "/resource2";

    memHandle = open(mem_source_path.c_str(), O_RDWR);
    if (memHandle < 0) {
        printf("Error: Failed to open %s\n", mem_source_path.c_str());
        return false;
    }

    regHandle = open(reg_source_path.c_str(), O_RDWR);
    if (regHandle < 0) {
        printf("Error: Failed to open %s\n", reg_source_path.c_str());
        return false;
    }

    return true;
}

uint32_t IxDevice::readReg(uint32_t addr)
{
    unsigned int value, ret;
    volatile void *regBase = nullptr;
    struct stat statinfo;

    if (fstat(regHandle, &statinfo) < 0) {
        cerr << "Failed to get device register size." << endl;
        return -1;
    }

    // Map the device register file into memory.
    regBase = mmap(nullptr, statinfo.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, regHandle, 0);
    if (regBase == MAP_FAILED) {
        cerr << "Map device register failed!" << endl;
        return -1;
    }

    value = *((volatile uint32_t *)((uint8_t *)regBase + addr));

    // Unmap the device register file from memory.
    ret = munmap((void *)regBase, statinfo.st_size);
    if (ret < 0) {
        cerr << "Failed to unmap device register!" << endl;
        return -1;
    }

    return value;
}

bool IxDevice::writeReg(uint32_t addr, uint32_t value)
{
    unsigned int size;
    volatile void *regBase;
    struct stat statinfo;

    fstat(regHandle, &statinfo);
    size = statinfo.st_size;

    regBase = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, regHandle, 0);
    if (regBase == MAP_FAILED) {
        printf("Map device register failed!\n");
	    return false;
    }

    *((volatile uint32_t *)((uint8_t *)regBase + addr)) = value;
    munmap((void *)regBase, size);

    return true;
}

uint32_t IxDevice::readMemory()
{
    return -1;
}

bool IxDevice::writeMemory(int addr, uint32_t size, const void *data) 
{
    unsigned int memSize;
    void *memBase;

    memSize = addr + size;
    memBase = mmap(NULL, memSize, PROT_READ | PROT_WRITE, MAP_SHARED, memHandle, 0);

    if (memBase == MAP_FAILED) {
        printf("Map device memory failed!\n");
	    return false;
    }
    
	// FIXME http://jira.iluvatar.ai:8080/browse/SWTEST-2876
    void *dest = (void *)(reinterpret_cast<char*>(memBase) + FW_MEM_BASE);
    #ifdef __aarch64__
        while (size--) {
		    *(char *)(dest) = *(char *)data;
		    dest = (char *)dest + 1;
		    data = (char *)data + 1;
        }
    #else
        memcpy((void *)(dest), data, size);
    #endif

    munmap((void *)memBase, memSize);

    return true;
}

bool IxDevice::checkDataInHBM(const void* data, uint32_t size)
{
    #ifdef __aarch64__
        return true;
    #else
    
        void *memBase, *destData;
        unsigned int memSize = FW_MEM_BASE + size;
        destData = malloc(memSize);
        memBase = mmap(NULL, memSize, PROT_READ, MAP_SHARED, memHandle, 0);

        if (memBase == MAP_FAILED) {
            printf("Map device memory failed!\n");
            return false;
        }

        memcpy(destData, (void *)(reinterpret_cast<char*>(memBase) + FW_MEM_BASE), size);
        if (memcmp(destData, data, size) == 0) {
            if (destData)
                free(destData);

            return true;
        } else {
            printf("Write memory data error!\n");
            
            if (destData)
                free(destData);
            return false;
        }
    #endif

    return true;
}