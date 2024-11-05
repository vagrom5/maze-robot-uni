#include <iostream>
#include <pigpio.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 33033  // Порт для получения данных
const char* UART_DEVICE = "/dev/ttyS0";  // UART устройство

int main() {
    // Инициализация pigpio
    if (gpioInitialise() < 0) {
        std::cerr << "pigpio error" << std::endl;
        return -1;
    }

    // Открытие UART
    int uart = serOpen(UART_DEVICE, 4800, 0);
    if (uart < 0) {
        std::cerr << "UART error" << std::endl;
        gpioTerminate();
        return -1;
    }
    
    // Настройка сокета для приема данных по UDP
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        std::cerr << "socket error" << std::endl;
        serClose(uart);
        gpioTerminate();
        return -1;
    }

    // Настройка локального адреса для приема данных
    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(SERVER_PORT);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        std::cerr << "socket bind error" << std::endl;
        close(sock);
        serClose(uart);
        gpioTerminate();
        return -1;
    }

    // Основной цикл: прием данных и их передача в UART
    while (true) {
        char buffer[1];
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        // Получаем данные по UDP
        int received = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &addrLen);
        if (received > 0) {
            serWriteByte(uart, buffer[0]);  // Отправляем команду в UART
            std::cout << "sent and received: " << buffer[0] << std::endl;
        }

        // Чтение данных с Arduino через UART
        if (serDataAvailable(uart)) {
            char arduino_response[100];
            int len = serRead(uart, arduino_response, sizeof(arduino_response) - 1);
            
            if (len > 0) {
                arduino_response[len] = '\0';
                std::cout << "arduino response: " << arduino_response << std::endl;
            }
        }
    }

    // Закрываем сокет и UART, завершаем работу pigpio
    close(sock);
    serClose(uart);
    gpioTerminate();
    return 0;
}