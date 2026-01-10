#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main() {
    const char* socket_path = "/tmp/my_unix_socket";
    const char* msg = "Hello via UNIX Socket!";

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Client: Connection failed. Is server running?" << std::endl;
        return 1;
    }

    write(sock_fd, msg, strlen(msg));
    std::cout << "Client: Message sent." << std::endl;

    close(sock_fd);
    return 0;
}