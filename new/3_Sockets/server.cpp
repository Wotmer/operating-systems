#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main() {
    const char* socket_path = "/tmp/my_unix_socket";
    
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    unlink(socket_path);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    std::cout << "Server: Listening..." << std::endl;

    int client_fd = accept(server_fd, NULL, NULL);
    
    char buffer[100];
    int bytes = read(client_fd, buffer, sizeof(buffer)-1);
    buffer[bytes] = '\0';
    
    std::cout << "Server: Received -> " << buffer << std::endl;

    close(client_fd);
    close(server_fd);
    unlink(socket_path);
    return 0;
}