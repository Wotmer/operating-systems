#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    const char* pipeName = "/tmp/my_fifo";
    
    mkfifo(pipeName, 0666);

    std::cout << "Receiver: Waiting for data..." << std::endl;
    
    std::ifstream fifo(pipeName);
    std::string message;
    
    std::getline(fifo, message);
    
    std::cout << "Receiver: Got message -> " << message << std::endl;
    
    fifo.close();
    unlink(pipeName);
    return 0;
}