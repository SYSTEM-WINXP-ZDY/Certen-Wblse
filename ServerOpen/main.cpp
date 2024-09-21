#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include <fstream>
#include <cstdio>
std::atomic<bool> serverRunning(true);

void printBanner() {
    std::cout << R"(
  ____            __            __          ___     _         
 / ___| ___ _ __| |_ ___ _ __  \ \        / / |__ | | ___ ___ 
| |    / _ \ '__| __/ _ \ '_ \  \ \  /\  / /| '_ \| |/ _ / __|
| |___|  __/ |  | ||  __/ | | |  \ \/  \/ / | |_) | |  __\__ \
 \____|\___|_|   \__\___|_| |_|   \_/\_/_/  |_.__/|_|\___|___/
)" << std::endl;
}

void monitorServerStatus(HANDLE serverProcess, const std::string& mappingPath, const std::string& mappingArgs) {
    WaitForSingleObject(serverProcess, INFINITE);
    serverRunning = false;
    std::cout << "服务器和映射程序已关闭。" << std::endl;
}

void monitorServerOutput(HANDLE process) {
    DWORD exitCode;
    while (GetExitCodeProcess(process, &exitCode) && exitCode == STILL_ACTIVE) {
        Sleep(1000);
    }
    std::cout << "服务器已停止运行。" << std::endl;
}

void startMappingProgram(const std::string& mappingPath, const std::string& mappingArgs) {
    std::string command = mappingPath + " " + mappingArgs;
    std::wstring wCommand(command.begin(), command.end());

    STARTUPINFO si = { sizeof(STARTUPINFO) }; // 初始化STARTUPINFO结构体
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(NULL, const_cast<LPWSTR>(wCommand.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "启动映射程序失败，请检查输入的参数。" << std::endl;
    }
}

void saveConfig(const std::string& javaPath, const std::string& maxMemory, const std::string& minMemory,
    const std::string& coreFilePath, const std::string& mappingPath, const std::string& mappingArgs,
    const std::string& SF) { // SF 应该是 const 引用
    std::ofstream configFile("config.txt");
    if (configFile) { // 简化检查文件是否打开
        configFile << javaPath << "\n"
            << maxMemory << "\n"
            << minMemory << "\n"
            << coreFilePath << "\n"
            << mappingPath << "\n"
            << mappingArgs << "\n"
            << SF << "\n"; // 去掉多余的分号
    }
    else {
        std::cerr << "无法打开配置文件进行写入。" << std::endl;
    }
}

void loadConfig(std::string& javaPath, std::string& maxMemory, std::string& minMemory,
    std::string& coreFilePath, std::string& mappingPath, std::string& mappingArgs,
    std::string& SF) {
    std::ifstream configFile("config.txt");
    if (configFile) { // 简化检查文件是否打开
        std::getline(configFile, javaPath);
        std::getline(configFile, maxMemory);
        std::getline(configFile, minMemory);
        std::getline(configFile, coreFilePath);
        std::getline(configFile, mappingPath);
        std::getline(configFile, mappingArgs);
        std::getline(configFile, SF);
    }
    else {
        std::cerr << "无法打开配置文件。" << std::endl;
    }
}


bool configFileExists(const std::string& filePath) {
    FILE* file;
    if (fopen_s(&file, filePath.c_str(), "r") == 0) {
        fclose(file);
        return true; // 文件存在
    }
    return false; // 文件不存在
}

void displayNewbieGuide() {
    std::cout << "如果你是第一次开这个服务器，如果服务器刚开启没多久就出现关闭现象，"
        "请检查MoJang的eula协议是否已经同意（<服务器目录>\\eula.txt，把里面的eula=false改成eula=true）。"
        "同意之后再开看看是否还出现异常的关闭现象，如果还出现，就询问作者（BiliBili-SYSTEM-WINXP-ZDY QQ:3688442118）。\n\n"
        "如果你要装插件，可以把插件放在<服务器目录>\\plugins里，再重启服务器。"
        << std::endl;
}

void disableNewbieGuide() {
    std::ofstream configFile("config.txt", std::ios::app); // 以追加模式打开文件
    if (configFile.is_open()) {
        configFile.seekp(0, std::ios::end); // 移动到文件末尾
        configFile << "\nTrue"; // 添加新行并写入True
        configFile.close();
        std::cout << "已永久关闭新手阅读。" << std::endl;
    }
    else {
        std::cerr << "无法打开配置文件。" << std::endl;
    }
}

bool askForNewbieGuide() {
    std::cout << "你是否阅读新手教程？(1.阅读 2.不阅读 3.永久关闭新手阅读): ";
    int choice;
    std::cin >> choice;
    std::cin.ignore();  // 清理输入缓冲区

    if (choice == 1) {
        displayNewbieGuide();
        return true;
    }
    else if (choice == 2) {
        return false;
    }
    else if (choice == 3) {
        disableNewbieGuide();
        return false;
    }
    return false;
}

int main() {
    std::string javaPath, maxMemory, minMemory, coreFilePath, mappingPath, mappingArgs, SF;
    loadConfig(javaPath, maxMemory, minMemory, coreFilePath, mappingPath, mappingArgs, SF);
    printBanner();
    if (SF != "True") {
        askForNewbieGuide();
    }
    
    const std::string configFilePath = "config.txt";
    bool configExists = configFileExists(configFilePath);
    if (configExists) {
        std::cout << "配置文件存在，是否读取之前的配置? (y/n): ";
        char loadConfigChoice;
        std::cin >> loadConfigChoice;
        std::cin.ignore();

        if (loadConfigChoice == 'y' || loadConfigChoice == 'Y') {
            loadConfig(javaPath, maxMemory, minMemory, coreFilePath, mappingPath, mappingArgs, SF);
        }
    }
    else {
        std::cout << "请输入 Java 路径: ";
        std::getline(std::cin, javaPath);
        std::cout << "请输入最大内存 (如 1024M): ";
        std::getline(std::cin, maxMemory);
        std::cout << "请输入最小内存 (如 512M): ";
        std::getline(std::cin, minMemory);
        std::cout << "请输入核心文件路径 (如 minecraft_server.jar): ";
        std::getline(std::cin, coreFilePath);
        std::cout << "请输入映射程序路径: ";
        std::getline(std::cin, mappingPath);
        std::cout << "请输入映射程序启动参数: ";
        std::getline(std::cin, mappingArgs);

        saveConfig(javaPath, maxMemory, minMemory, coreFilePath, mappingPath, mappingArgs,SF);
    }

    std::string serverCommand = javaPath + " -Xms" + minMemory + " -Xmx" + maxMemory + " -jar " + coreFilePath + " nogui";
    std::wstring wServerCommand(serverCommand.begin(), serverCommand.end());

    STARTUPINFO si = { sizeof(STARTUPINFO) }; // 使用初始化
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(NULL, const_cast<LPWSTR>(wServerCommand.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::thread monitorThread(monitorServerOutput, pi.hProcess);
        monitorThread.detach();

        // 添加监测服务器状态的线程
        std::thread statusThread(monitorServerStatus, pi.hProcess, mappingPath, mappingArgs);
        statusThread.detach();

        // 启动映射程序
        std::thread mappingThread(startMappingProgram, mappingPath, mappingArgs);
        mappingThread.detach();

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "启动服务器失败，请检查输入的参数。" << std::endl;
    }

    return 0;
}

