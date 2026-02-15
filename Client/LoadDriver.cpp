#include "LoadDriver.h"

SC_HANDLE hManager = NULL;
SC_HANDLE hService = NULL;

// 安装驱动程序
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

// 启动驱动程序
BOOL StartDriver() {
    return StartServiceA(hService, NULL, NULL);
}

// 停止驱动程序
BOOL StopDriver() {
    SERVICE_STATUS status = { 0 };
    return  ControlService(hService, SERVICE_CONTROL_STOP, &status);
}

// 卸载驱动程序
BOOL UninstallDriver() {
    return DeleteService(hService);
}

// 加载驱动完整流程
int load() {
    InstallDriver("Study", "Study", "C:\\Users\\wm\\Desktop\\Driver.sys");
    BOOL success = StartDriver();
    if (success) {
        printf("启动成功!\n");
    } else {
        printf("启动失败!\n");
    }
    return 0;
}