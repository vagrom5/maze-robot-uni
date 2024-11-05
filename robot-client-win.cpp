#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <conio.h>  // Для работы с функцией _kbhit() и _getch()


#pragma comment(lib, "ws2_32.lib")  // Подключение библиотеки Winsock


// #define SERVER_IP "192.168.1.46"  // IP адрес Raspberry  указываем вручную в переменной server_ip
#define SERVER_PORT 33033  // Порт для передачи данных


int main() {
    char server_ip[16];
    std::cin>>server_ip;
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации Winsock" << std::endl;
        return -1;
    }

    // Создание сокета
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        WSACleanup();
        return -1;
    }

    // Настройка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(server_ip);

    // Основной цикл обработки клавиш
    while (true) {
        if (_kbhit()) {  // Проверяем, была ли нажата клавиша
            char key = _getch();  // Считываем клавишу

            char command = 0;  // Команда, которая будет отправлена
            switch (key) {
                case 'w': command = '1'; break;
                case 's': command = '2'; break;
                case 'd': command = '3'; break;
                case 'a': command = '4'; break;
                case 72: command = '5'; break;  // Вверх
                case 80: command = '6'; break;  // Вниз
                case 75: command = '7'; break;  // Влево
                case 77: command = '8'; break;  // Вправо
            }

            if (command != 0) {
                // Отправляем команду на сервер
                sendto(sock, &command, sizeof(command), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
            }
        }
    }

    // Закрываем сокет и освобождаем ресурсы
    closesocket(sock);
    WSACleanup();
    return 0;
}
