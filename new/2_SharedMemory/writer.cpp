#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    const char* shm_name = "/my_shm_example";
    const int SIZE = 4096;
    const char* msg = "Hello from Shared Memory!";

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);

    void* ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    std::memcpy(ptr, msg, std::strlen(msg) + 1);

    std::cout << "Writer: Data written to shared memory." << std::endl;
    std::cout << "Writer: Press Enter to exit and clean up..." << std::endl;
    std::cin.get();

    munmap(ptr, SIZE);
    shm_unlink(shm_name);
    return 0;
}