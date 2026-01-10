#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    const char* pipeName = "/tmp/my_fifo";

    mkfifo(pipeName, 0666);

    std::cout << "Sender: Waiting for receiver..." << std::endl;
    
    std::ofstream fifo(pipeName);
    
    std::cout << "Sender: Connected! Sending message." << std::endl;
    fifo << "Hello from FIFO sender!" << std::endl;
    
    fifo.close();
    return 0;
}