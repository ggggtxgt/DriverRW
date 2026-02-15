#pragma once

#include <iostream>
#include <Windows.h>

// 安装驱动程序
BOOL InstallDriver(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpBinaryPathName);
// 启动驱动程序
BOOL StartDriver();
// 停止驱动程序
BOOL StopDriver();
// 卸载驱动程序
BOOL UninstallDriver();
// 加载驱动完整流程
int load();