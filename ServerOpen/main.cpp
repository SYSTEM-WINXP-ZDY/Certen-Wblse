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
    std::cout << "��������ӳ������ѹرա�" << std::endl;
}

void monitorServerOutput(HANDLE process) {
    DWORD exitCode;
    while (GetExitCodeProcess(process, &exitCode) && exitCode == STILL_ACTIVE) {
        Sleep(1000);
    }
    std::cout << "��������ֹͣ���С�" << std::endl;
}

void startMappingProgram(const std::string& mappingPath, const std::string& mappingArgs) {
    std::string command = mappingPath + " " + mappingArgs;
    std::wstring wCommand(command.begin(), command.end());

    STARTUPINFO si = { sizeof(STARTUPINFO) }; // ��ʼ��STARTUPINFO�ṹ��
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(NULL, const_cast<LPWSTR>(wCommand.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "����ӳ�����ʧ�ܣ���������Ĳ�����" << std::endl;
    }
}

void saveConfig(const std::string& javaPath, const std::string& maxMemory, const std::string& minMemory,
    const std::string& coreFilePath, const std::string& mappingPath, const std::string& mappingArgs,
    const std::string& SF) { // SF Ӧ���� const ����
    std::ofstream configFile("config.txt");
    if (configFile) { // �򻯼���ļ��Ƿ��
        configFile << javaPath << "\n"
            << maxMemory << "\n"
            << minMemory << "\n"
            << coreFilePath << "\n"
            << mappingPath << "\n"
            << mappingArgs << "\n"
            << SF << "\n"; // ȥ������ķֺ�
    }
    else {
        std::cerr << "�޷��������ļ�����д�롣" << std::endl;
    }
}

void loadConfig(std::string& javaPath, std::string& maxMemory, std::string& minMemory,
    std::string& coreFilePath, std::string& mappingPath, std::string& mappingArgs,
    std::string& SF) {
    std::ifstream configFile("config.txt");
    if (configFile) { // �򻯼���ļ��Ƿ��
        std::getline(configFile, javaPath);
        std::getline(configFile, maxMemory);
        std::getline(configFile, minMemory);
        std::getline(configFile, coreFilePath);
        std::getline(configFile, mappingPath);
        std::getline(configFile, mappingArgs);
        std::getline(configFile, SF);
    }
    else {
        std::cerr << "�޷��������ļ���" << std::endl;
    }
}


bool configFileExists(const std::string& filePath) {
    FILE* file;
    if (fopen_s(&file, filePath.c_str(), "r") == 0) {
        fclose(file);
        return true; // �ļ�����
    }
    return false; // �ļ�������
}

void displayNewbieGuide() {
    std::cout << "������ǵ�һ�ο����������������������տ���û��þͳ��ֹر�����"
        "����MoJang��eulaЭ���Ƿ��Ѿ�ͬ�⣨<������Ŀ¼>\\eula.txt���������eula=false�ĳ�eula=true����"
        "ͬ��֮���ٿ������Ƿ񻹳����쳣�Ĺر�������������֣���ѯ�����ߣ�BiliBili-SYSTEM-WINXP-ZDY QQ:3688442118����\n\n"
        "�����Ҫװ��������԰Ѳ������<������Ŀ¼>\\plugins���������������"
        << std::endl;
}

void disableNewbieGuide() {
    std::ofstream configFile("config.txt", std::ios::app); // ��׷��ģʽ���ļ�
    if (configFile.is_open()) {
        configFile.seekp(0, std::ios::end); // �ƶ����ļ�ĩβ
        configFile << "\nTrue"; // ������в�д��True
        configFile.close();
        std::cout << "�����ùر������Ķ���" << std::endl;
    }
    else {
        std::cerr << "�޷��������ļ���" << std::endl;
    }
}

bool askForNewbieGuide() {
    std::cout << "���Ƿ��Ķ����ֽ̳̣�(1.�Ķ� 2.���Ķ� 3.���ùر������Ķ�): ";
    int choice;
    std::cin >> choice;
    std::cin.ignore();  // �������뻺����

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
        std::cout << "�����ļ����ڣ��Ƿ��ȡ֮ǰ������? (y/n): ";
        char loadConfigChoice;
        std::cin >> loadConfigChoice;
        std::cin.ignore();

        if (loadConfigChoice == 'y' || loadConfigChoice == 'Y') {
            loadConfig(javaPath, maxMemory, minMemory, coreFilePath, mappingPath, mappingArgs, SF);
        }
    }
    else {
        std::cout << "������ Java ·��: ";
        std::getline(std::cin, javaPath);
        std::cout << "����������ڴ� (�� 1024M): ";
        std::getline(std::cin, maxMemory);
        std::cout << "��������С�ڴ� (�� 512M): ";
        std::getline(std::cin, minMemory);
        std::cout << "����������ļ�·�� (�� minecraft_server.jar): ";
        std::getline(std::cin, coreFilePath);
        std::cout << "������ӳ�����·��: ";
        std::getline(std::cin, mappingPath);
        std::cout << "������ӳ�������������: ";
        std::getline(std::cin, mappingArgs);

        saveConfig(javaPath, maxMemory, minMemory, coreFilePath, mappingPath, mappingArgs,SF);
    }

    std::string serverCommand = javaPath + " -Xms" + minMemory + " -Xmx" + maxMemory + " -jar " + coreFilePath + " nogui";
    std::wstring wServerCommand(serverCommand.begin(), serverCommand.end());

    STARTUPINFO si = { sizeof(STARTUPINFO) }; // ʹ�ó�ʼ��
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(NULL, const_cast<LPWSTR>(wServerCommand.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::thread monitorThread(monitorServerOutput, pi.hProcess);
        monitorThread.detach();

        // ��Ӽ�������״̬���߳�
        std::thread statusThread(monitorServerStatus, pi.hProcess, mappingPath, mappingArgs);
        statusThread.detach();

        // ����ӳ�����
        std::thread mappingThread(startMappingProgram, mappingPath, mappingArgs);
        mappingThread.detach();

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "����������ʧ�ܣ���������Ĳ�����" << std::endl;
    }

    return 0;
}

