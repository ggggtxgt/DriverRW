#include "resource.h"
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

// 获取随机驱动名称
void RandomDriverName(char* driverName, DWORD len) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    // len 应包含结尾 null 的空间，因此最多生成 len-1 个字符
    for (DWORD i = 0; i < len - 1; i++) {
        driverName[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    driverName[len - 1] = '\0';
}

// 从应用程序的资源中提取驱动程序文件并保存到指定路径
void LoadMyResource(LPCSTR szDestPath) {
    // 查找资源
    HRSRC hRsc = FindResourceA(NULL, MAKEINTRESOURCEA(IDR_MYSYS1), "MySys");
    if (!hRsc) {
        printf("找不到资源\n");
        return;
    }
    // 加载资源
    HGLOBAL hg = LoadResource(NULL, hRsc);
    LPVOID fileBuff = LockResource(hg);
    DWORD fileSize = SizeofResource(NULL, hRsc);

    // 创建目标文件
    HANDLE hFile = CreateFileA(szDestPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("创建文件失败，错误码：%d\n", GetLastError());
        return;
    }

    // 写入数据
    DWORD written = 0;
    WriteFile(hFile, fileBuff, fileSize, &written, NULL);
    CloseHandle(hFile);
}

// 加载驱动完整流程
int load() {
    // 初始化随机种子
    srand((unsigned)time(NULL));

    char szServiceName[64] = { 0 };
    char szDriverPath[MAX_PATH] = { 0 };
    char szTempPath[MAX_PATH] = { 0 };

    // 1. 生成随机服务名（长度不超过 63）
    RandomDriverName(szServiceName, sizeof(szServiceName));

    // 2. 获取临时目录，拼接驱动文件路径
    if (!GetTempPathA(MAX_PATH, szTempPath)) {
        printf("获取临时目录失败\n");
        return -1;
    }
    strcpy_s(szDriverPath, MAX_PATH, szTempPath);
    strcat_s(szDriverPath, MAX_PATH, szServiceName);
    strcat_s(szDriverPath, MAX_PATH, ".sys");

    // 3. 将资源中的驱动释放到目标路径
    LoadMyResource(szDriverPath);

    // 4. 安装服务
    if (!InstallDriver(szServiceName, szServiceName, szDriverPath)) {
        printf("安装服务失败\n");
        // 可删除已释放的文件，但这里简化处理
        return -1;
    }

    // 5. 启动服务
    if (!StartDriver()) {
        printf("启动服务失败，错误码：%d\n", GetLastError());
        // 清理已安装的服务
        UninstallDriver();
        return -1;
    }
    printf("驱动加载成功！\n");
    return 0;
}