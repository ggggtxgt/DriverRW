#include <iostream>
#include <Windows.h>

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

int main() {
    // @todo 使用随机名称
    InstallDriver("Driver", "Driver", "C:\\Users\\win10\Desktop\\Driver.sys");
    BOOL success = StartDriver();
    if (success) {
        printf("启动成功!");
    }
    system("pause");
    return 0;
}