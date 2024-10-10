#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library

#define PORT 33033
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return -1;
    }

    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    char robot_ip;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    std::cout << "Enter IP: ";
    std::cin >> robot_ip;

    // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, robot_ip, &server_address.sin_addr);

    // Connect to the server
    result = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (result == SOCKET_ERROR) {
        std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    std::cout << "Connected to the server" << std::endl;

    char input;
    while (true) {
        std::cout << "Enter command (WASD): ";
        std::cin >> input;

        if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
            send(sock, &input, 1, 0);
        } else {
            std::cout << "Invalid command" << std::endl;
        }
    }

    // Clean up
    closesocket(sock);
    WSACleanup();

    return 0;
}
