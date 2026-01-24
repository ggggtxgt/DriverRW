#include <iostream>
#include <Windows.h>
#include "resource.h"

#include "comm.cpp"

SC_HANDLE hManager = NULL;
SC_HANDLE hService = NULL;

BOOL InstallDriver(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpBinaryPathName) {
    // 打开任务管理器
    hManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    // 创建服务
    hService = CreateServiceA(hManager, lpServiceName, lpDisplayName, SC_MANAGER_ALL_ACCESS, SERVICE_KERNEL_DRIVER, 
                              SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
    if (!hService) {
        printf("CreateServiceA Error!");
        return FALSE;
    }
    return TRUE;
}

BOOL StartDriver() {
    return StartServiceA(hService, NULL, NULL);
}

BOOL StopDriver() {
    SERVICE_STATUS status = { 0 };
   return  ControlService(hService, SERVICE_CONTROL_STOP, &status);
}

BOOL UninstallDriver() {
    return DeleteService(hService);
}

void RandomDriverName(char *driverName, DWORD len) {
    char directory[20] = { 'A', 'B', 'C', 'D', 'E', 'F', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'X'};
    for (size_t i = 0; i < len; i++) {
        srand(time(NULL));
        int num = rand() % 20;
        driverName[i] = directory[i];
    }
}

void LoadRes() {
    HRSRC hRsc = FindResourceA(NULL, MAKEINTRESOURCEA(IDR_MYSYS1), "MySys");
    HGLOBAL hg = LoadResource(NULL, hRsc);
    LPVOID fileBuff = LockResource(hg);
    HANDLE hFile = CreateFileA("D:\\text.sys", FILE_ALL_ACCESS, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD fileSize = SizeofResource(NULL, hRsc);
    DWORD realWriteLen = 0;
    WriteFile(hFile, fileBuff, fileSize, &realWriteLen, NULL);
    CloseHandle(hRsc);
}

int load() {
    LoadRes();
    char strName[20] = { 0 };
    RandomDriverName(strName, 19);
    InstallDriver(strName, strName, "C:\\Users\\win10\Desktop\\Driver.sys");
    BOOL success = StartDriver();
    if (success) {
        printf("启动成功!");
    }
    system("pause");
    return 0;
}