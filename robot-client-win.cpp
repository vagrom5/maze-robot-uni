#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcp_ip.h>
#include <SDL.h>

#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library
#pragma comment(lib, "SDL2.lib")   // Link with the SDL2 library

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

    char robot_ip[16];

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

    // Initialize SDL
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "SDL could_not_initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Open the joystick
    if (SDL_NumJoysticks() < 1) {
        std::cerr << "No joysticks detected" << std::endl;
        return -1;
    }

    SDL_Joystick *joystick = SDL_JoystickOpen(0);
    if (!joystick) {
        std::cerr << "Failed to open joystick: " << SDL_GetError() << std::endl;
        return -1;
    }

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                goto cleanup;
            }
        }

        // Get the current state of the joystick axes
        int axis0 = SDL_JoystickGetAxis(joystick, 0);
        int axis1 = SDL_JoystickGetAxis(joystick, 1);

        // Send the current state of the axes to the server
        char message[16];
        snprintf(message, sizeof(message), "%d,%d", axis0, axis1);
        send(sock, message, strlen(message), 0);

        SDL_Delay(100); // Small delay to avoid spamming the server
    }

cleanup:
    SDL_JoystickClose(joystick);
    SDL_Quit();
    closesocket(sock);
    WSACleanup();

    return 0;
}