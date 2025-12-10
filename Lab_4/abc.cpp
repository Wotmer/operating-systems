#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
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
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, (DWORD)pid);
    DWORD ret = WaitForSingleObject(process, 0);
    CloseHandle(process);
    return ret == WAIT_TIMEOUT;
#else
    return kill((pid_t)pid, 0) == 0;
#endif
}

long launchDummyProcess() {
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(TEXT("C:\\Windows\\System32\\notepad.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to launch notepad." << std::endl;
        return -1;
    }
    return (long)pi.dwProcessId;
#else
    pid_t pid = fork();
    if (pid == 0) {
        execlp("sleep", "sleep", "1000", NULL);
        exit(1);
    }
    return (long)pid;
#endif
}

void runKiller(const std::string& args) {
    std::string cmd;
#ifdef _WIN32
    cmd = "killer.exe " + args;
    
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    char* cmdCStr = _strdup(cmd.c_str()); 
    
    if (CreateProcess(NULL, cmdCStr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Failed to start Killer. Error: " << GetLastError() << std::endl;
    }
    free(cmdCStr);
#else
    cmd = "./killer " + args;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (isProcessRunning(pid1)) std::cout << "Dummy started with PID: " << pid1 << std::endl;
    else std::cerr << "Dummy failed to start!" << std::endl;

    runKiller("--id " + std::to_string(pid1));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (!isProcessRunning(pid1)) std::cout << "SUCCESS: Process " << pid1 << " is dead." << std::endl;
    else std::cout << "FAIL: Process " << pid1 << " is still alive." << std::endl;

    std::cout << "\n[TEST 2] Kill by Name (--name)" << std::endl;
    long pid2 = launchDummyProcess();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    runKiller("--name " + dummyName);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (!isProcessRunning(pid2)) std::cout << "SUCCESS: Process by name is dead." << std::endl;
    else std::cout << "FAIL: Process is still alive." << std::endl;


    std::cout << "\n[TEST 3] Kill by Environment Variable (PROC_TO_KILL)" << std::endl;
    
    long pid3 = launchDummyProcess();
    long pid4 = launchDummyProcess();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string envVal = dummyName + ",some_other_process";
    setEnvVar("PROC_TO_KILL", envVal);
    std::cout << "Environment variable set: " << envVal << std::endl;

    runKiller("");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (!isProcessRunning(pid3) && !isProcessRunning(pid4)) {
        std::cout << "SUCCESS: Both processes killed via Env Var." << std::endl;
    } else {
        std::cout << "FAIL: Processes are still alive." << std::endl;
    }

    unsetEnvVar("PROC_TO_KILL");
    std::cout << "\nVariable removed. End of tests." << std::endl;

    return 0;
}