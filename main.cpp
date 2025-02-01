#include <iostream>
#include "device_bi.h"
#include "device_mr.h"
#include "device_qs.h"
#include "enmudevice.h"
#include <CLI/CLI.hpp>

static void listBoards(vector<IxBoard*> Boards)
{
    printf("%-4s %-13s %-11s %-15s %-11s %s\n", "NUM", "DEVICE", "PRODUCT", "SN", "PN", "DATE");

    int deciceNum = 0;
    for (auto board = Boards.begin(); board != Boards.end(); board++) {
        printf("%-4d %-13s %-11s %-15s %-11s %s\n",
                deciceNum,
                ((*board)->deviceBusId).c_str(),
                ((*board)->deviceName).c_str(),
                ((*board)->snNumber).c_str(),
                ((*board)->pnNumber).c_str(),
                ((*board)->dateNumber).c_str());
	    deciceNum++;
    }
}

int main(int argc, const char** argv)
{
    string specificDeviceBusId("");
    string snBinFile("");
    string snNumForFile("");
    string pnNumForFile("");
    string snDate("");
    bool snListDevice(false);

    CLI::App app{"A tool to read/write S/N from Iluvatar Corex GPU flash."};
    app.add_option("-d, --device", specificDeviceBusId, "Specify device bus id.");
    app.add_option("-f, --file", snBinFile, "File contains of the data to flash.");
    app.add_option("--sn, --snserial", snNumForFile, "Specify sn sequence number and generate sn file.");
    app.add_option("--pn, --pnserial", pnNumForFile, "Specify pn sequence number.");
    app.add_option("--date", snDate, "Generate sn file.");
    app.add_flag("-l, --list", snListDevice, "List all device information.");
    app.set_help_flag("-h, --help", "Print this help message and exit.");

    CLI11_PARSE(app, argc, argv);

    if (geteuid() != 0) {
        printf("Requires root privileges.\n");
	    return EXIT_FAILURE;
    }

    if (getDriver()) {
        printf("Please unload Iluvatar GPU driver firstly.\n");
	    return EXIT_FAILURE;
    }

    unique_ptr<EnmuDevice> enmudevice = make_unique<EnmuDevice>();
    unique_ptr<FileOperator> fileOperator = make_unique<FileOperator>();

    enmudevice->initPciDevice();
    vector<IxBoard*> allBoards = enmudevice->allBoards;
    if (allBoards.empty()) {
        printf("No Iluvatar Corex GPU found.\n");
        return EXIT_FAILURE;
    }

    if (snListDevice) {
        listBoards(allBoards);
        return EXIT_SUCCESS;
    }

    // 默认查找第一张卡
    if (specificDeviceBusId.empty()) {
        specificDeviceBusId = allBoards[0]->deviceBusId;
    } else if (specificDeviceBusId.length() != 12) {
        if(getDomain() == "")
            specificDeviceBusId = "0000:" + specificDeviceBusId.substr(specificDeviceBusId.length()-7);
        else
            specificDeviceBusId = getDomain() + specificDeviceBusId.substr(specificDeviceBusId.length()-7);
    }

    if (pnNumForFile.empty() ) {
        string deviceName = enmudevice->getDeviceName(specificDeviceBusId);
        auto it = fileOperator->devidPnMap.find(deviceName);
        if (it != fileOperator->devidPnMap.end())
            fileOperator->devicePnNumber = fileOperator->devidPnMap[deviceName];
        else {
            printf("No gpu device found with the specified bdf. \n");
            return EXIT_FAILURE;
        }
    } else {
        fileOperator->devicePnNumber = pnNumForFile;
    }

    // 生成SN.bin文件，并应用到下面的烧录操作中
    if (!snNumForFile.empty()) {
        fileOperator->generateFiles(snNumForFile, snDate, snBinFile);
    }

    if (snBinFile.empty()) {
        printf("Error: SN bin file should be specified.\n");
        return EXIT_FAILURE;
    }

    vector<unique_ptr<IxDevice>> ixDevices;
    for (size_t i = 0; i < allBoards.size(); i++) {
        if (allBoards[i]->deviceId == BI100_SERIES) {
            if (allBoards[i]->deviceBusId == specificDeviceBusId) {
                ixDevices.push_back(unique_ptr<IxDevice>(new IxDeviceBI(allBoards[i]->deviceBusId)));
            }
        } else if (allBoards[i]->deviceId != BI100_SERIES && allBoards[i]->deviceId != QS200_SERIES) {
            if (ifDualChipboard(specificDeviceBusId)) {
                if (allBoards[i]->deviceBusId == specificDeviceBusId) {
                    ixDevices.push_back(unique_ptr<IxDevice>(new IxDeviceMR(allBoards[i]->deviceBusId)));
                } else if (onSameBoard(allBoards[i]->deviceBusId, specificDeviceBusId)) { // mr50 和 bi150板卡自动push另一个芯的busId
                    ixDevices.push_back(unique_ptr<IxDevice>(new IxDeviceMR(allBoards[i]->deviceBusId)));
                }
            }
            else {
                if (allBoards[i]->deviceBusId == specificDeviceBusId) {
                    ixDevices.push_back(unique_ptr<IxDevice>(new IxDeviceMR(allBoards[i]->deviceBusId)));
                }  
            }
        } else if (allBoards[i]->deviceId == QS200_SERIES) {
            if (ifDualChipboard(specificDeviceBusId)) {
                if (allBoards[i]->deviceBusId == specificDeviceBusId) { 
                    ixDevices.push_back(unique_ptr<IxDevice>(new IxDeviceQS(allBoards[i]->deviceBusId)));
                } else if (onSameBoard(allBoards[i]->deviceBusId, specificDeviceBusId)) { //QS OAM板卡自动push另一个芯的busId
                    ixDevices.push_back(unique_ptr<IxDevice>(new IxDeviceQS(allBoards[i]->deviceBusId)));
                }
            } else {
                if (allBoards[i]->deviceBusId == specificDeviceBusId) {
                    ixDevices.push_back(unique_ptr<IxDevice>(new IxDeviceQS(allBoards[i]->deviceBusId)));
                }
            }
        }
    }
    
    void *data = nullptr;
    uint32_t length = fileOperator->readSNBinFile(snBinFile.c_str(), &data);
    if (length == 0) {
        return EXIT_FAILURE;
    }

    for(auto& devicePtr : ixDevices) {
        printf("[%s] start burning [%s].\n\n", devicePtr->deviceBusId.c_str(), snBinFile.c_str());
        if (!devicePtr->burnSNByHBM(data, length)) {
            printf("[%s] fails to burn [%s]\n\n", devicePtr->deviceBusId.c_str(), snBinFile.c_str());
        } else {
            sleep(2);
            printf("[%s] successfully to burn product info.\n\n", devicePtr->deviceBusId.c_str());// 给出 sn pn data
        }
    }

    free(data);
    for(auto& board : allBoards) {
        delete board;
        board = nullptr;
    }
    allBoards.clear();

    return EXIT_SUCCESS;
}