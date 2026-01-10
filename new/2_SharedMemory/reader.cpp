#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    const char* shm_name = "/my_shm_example";
    const int SIZE = 4096;

    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        std::cerr << "Reader: Shared memory object not found. Run writer first!" << std::endl;
        return 1;
    }

    void* ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    std::cout << "Reader: Read from SHM -> " << (char*)ptr << std::endl;

    munmap(ptr, SIZE);
    return 0;
}