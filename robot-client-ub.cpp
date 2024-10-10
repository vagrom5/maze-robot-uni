#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 33033
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    char ip_rpi;
    std::cin >> ip_rpi; 

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_rpi, &server_address.sin_addr) <= 0) {
        perror("inet_pton failed");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect failed");
        return -1;
    }

    std::cout << "Connected to the server" << std::endl;
    std::cout << "Enter command (WASD): ";

    char input;
    while (true) {
        std::cin >> input;

        if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
            send(sock, &input, 1, 0);
        } else {
            std::cout << "Invalid command" << std::endl;
        }
    }

    close(sock);

    return 0;
}
