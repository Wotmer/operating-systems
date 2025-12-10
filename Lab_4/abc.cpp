#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#endif

void setEnvVar(const std::string& name, const std::string& value) {
#ifdef _WIN32
    SetEnvironmentVariable(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

void unsetEnvVar(const std::string& name) {
#ifdef _WIN32
    SetEnvironmentVariable(name.c_str(), NULL);
#else
    unsetenv(name.c_str());
#endif
}

bool isProcessRunning(long pid) {
#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)pid);
    if (process == NULL) return false;

    DWORD exitCode;
    BOOL result = GetExitCodeProcess(process, &exitCode);
    CloseHandle(process);

    return result && exitCode == STILL_ACTIVE;
#else
    if (kill((pid_t)pid, 0) != 0) return false;

    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream statFile(path);
    if (!statFile.is_open()) return true;

    std::string line;
    std::getline(statFile, line);
    statFile.close();

    size_t pos = line.find_last_of(')');
    if (pos == std::string::npos || pos + 2 >= line.length()) return true;

    return line[pos + 2] != 'Z';
#endif
}

#ifdef _WIN32
bool isProcessRunningByName(const std::string& name) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    bool found = false;
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (std::string(pe.szExeFile) == name) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    DWORD exitCode;
                    if (GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE) {
                        found = true;
                    }
                    CloseHandle(hProcess);
                }
                if (found) break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return found;
}
#else
bool isProcessRunningByName(const std::string& name) {
    DIR* dir = opendir("/proc");
    if (!dir) return false;

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!isdigit(*ent->d_name)) continue;

        std::string cmdPath = "/proc/" + std::string(ent->d_name) + "/comm";
        std::ifstream cmdFile(cmdPath);
        if (!cmdFile.is_open()) continue;

        std::string cmdName;
        std::getline(cmdFile, cmdName);
        cmdFile.close();

        if (cmdName.empty() || cmdName != name) continue;

        std::string statPath = "/proc/" + std::string(ent->d_name) + "/stat";
        std::ifstream statFile(statPath);
        if (!statFile.is_open()) continue;

        std::string line;
        std::getline(statFile, line);
        statFile.close();

        size_t pos = line.find_last_of(')');
        if (pos != std::string::npos && pos + 2 < line.length() && line[pos + 2] != 'Z') {
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}
#endif

long launchDummyProcess() {
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(TEXT("C:\\Windows\\System32\\notepad.exe"), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        return -1;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (long)pi.dwProcessId;
#else
    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        execlp("sleep", "sleep", "1000", NULL);
        _exit(1);
    }
    return pid > 0 ? (long)pid : -1;
#endif
}

void runKiller(const std::string& args) {
#ifdef _WIN32
    std::string cmd = "killer.exe " + args;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    char* cmdStr = new char[cmd.length() + 1];
    strcpy(cmdStr, cmd.c_str());

    if (CreateProcess(NULL, cmdStr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    delete[] cmdStr;
#else
    std::string cmd = "./killer " + args;
    system(cmd.c_str());
#endif
}

int main() {
    std::cout << "--- User Application Started ---" << std::endl;

    std::string dummyName;
#ifdef _WIN32
    dummyName = "notepad.exe";
#else
    dummyName = "sleep";
#endif

    std::cout << "\n[TEST 1] Kill by ID (--id)" << std::endl;
    long pid1 = launchDummyProcess();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    if (isProcessRunning(pid1)) {
        std::cout << "Dummy started with PID: " << pid1 << std::endl;
    }

    runKiller("--id " + std::to_string(pid1));
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    if (!isProcessRunning(pid1)) {
        std::cout << "SUCCESS: Process " << pid1 << " is dead." << std::endl;
    } else {
        std::cout << "FAIL: Process " << pid1 << " is still alive." << std::endl;
    }

    std::cout << "\n[TEST 2] Kill by Name (--name)" << std::endl;
    long pid2 = launchDummyProcess();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    if (isProcessRunningByName(dummyName)) {
        std::cout << "Process " << dummyName << " is running." << std::endl;
    }

    runKiller("--name " + dummyName);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    if (!isProcessRunningByName(dummyName)) {
        std::cout << "SUCCESS: Process by name is dead." << std::endl;
    } else {
        std::cout << "FAIL: Process is still alive." << std::endl;
    }

    std::cout << "\n[TEST 3] Kill by Environment Variable (PROC_TO_KILL)" << std::endl;

    long pid3 = launchDummyProcess();
    long pid4 = launchDummyProcess();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    std::string envVal = dummyName + ",some_other_process";
    setEnvVar("PROC_TO_KILL", envVal);
    std::cout << "Environment variable set: " << envVal << std::endl;

    runKiller("");
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    if (!isProcessRunningByName(dummyName)) {
        std::cout << "SUCCESS: Processes killed via Env Var." << std::endl;
    } else {
        std::cout << "FAIL: Processes are still alive." << std::endl;
    }

    unsetEnvVar("PROC_TO_KILL");
    std::cout << "\nVariable removed. End of tests." << std::endl;

    return 0;
}