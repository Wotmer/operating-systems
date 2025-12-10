#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

void killProcessByID(int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (hProcess == NULL) {
        std::cerr << "[Killer] Failed to open process " << pid << ". Error: " << GetLastError() << std::endl;
        return;
    }
    if (TerminateProcess(hProcess, 0)) {
        std::cout << "[Killer] Process " << pid << " terminated." << std::endl;
    } else {
        std::cerr << "[Killer] Failed to terminate " << pid << ". Error: " << GetLastError() << std::endl;
    }
    CloseHandle(hProcess);
}

void killProcessByName(const std::string& name) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            std::string currentName = pe.szExeFile;
            if (currentName == name) {
                killProcessByID(pe.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
}

#elif __linux__
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>

void killProcessByID(int pid) {
    if (kill((pid_t)pid, SIGKILL) == 0) {
        std::cout << "[Killer] Process " << pid << " terminated." << std::endl;
    } else {
        perror("[Killer] Failed to kill process");
    }
}

void killProcessByName(const std::string& name) {
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir("/proc")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (!isdigit(*ent->d_name)) continue;

            int pid = atoi(ent->d_name);
            
            std::string path = "/proc/" + std::string(ent->d_name) + "/comm";
            std::ifstream cmdFile(path.c_str());
            std::string cmdName;
            if (cmdFile.is_open()) {
                std::getline(cmdFile, cmdName);
                if (!cmdName.empty() && cmdName.back() == '\n') cmdName.pop_back();
                
                if (cmdName == name) {
                    killProcessByID(pid);
                }
                cmdFile.close();
            }
        }
        closedir(dir);
    }
}
#endif

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    std::cout << "[Killer] Started." << std::endl;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--id" && i + 1 < argc) {
            int pid = atoi(argv[++i]);
            killProcessByID(pid);
        } else if (arg == "--name" && i + 1 < argc) {
            std::string name = argv[++i];
            killProcessByName(name);
        }
    }

    const char* env_p = getenv("PROC_TO_KILL");
    if (env_p) {
        std::string env_val = env_p;
        std::cout << "[Killer] Found PROC_TO_KILL: " << env_val << std::endl;
        std::vector<std::string> names = split(env_val, ',');
        for (const auto& name : names) {
            if (!name.empty()) {
                killProcessByName(name);
            }
        }
    }

    return 0;
}