#include <iostream>
#include <fstream>
#include <string>

void checkWiFiAndControlPin(int pin) {
    // Проверка соединения Wi-Fi
    std::ifstream carrierFile("/sys/class/net/wlan0/carrier");
    int carrierStatus;

    if (!carrierFile.is_open()) {
        std::cerr << "Ошибка при чтении статуса сетевого интерфейса." << std::endl;
        return;
    }

    carrierFile >> carrierStatus;
    carrierFile.close();

    // Экспорт пина
    std::ofstream exportFile("/sys/class/gpio/export");
    if (exportFile.is_open()) {
        exportFile << pin;
        exportFile.close();
    }

    // Установка направления пина
    std::ofstream directionFile("/sys/class/gpio/gpio" + std::to_string(pin) + "/direction");
    if (directionFile.is_open()) {
        directionFile << "out";
        directionFile.close();
    }

    // Управление пином в зависимости от состояния Wi-Fi
    std::ofstream pinValueFile("/sys/class/gpio/gpio" + std::to_string(pin) + "/value");
    if (pinValueFile.is_open()) {
        pinValueFile << (carrierStatus == 1 ? 0 : 1); // 0 - Wi-Fi есть, 1 - нет
        pinValueFile.close();

        std::cout << (carrierStatus == 1 ? "Соединение с Wi-Fi установлено." : "Нет соединения с Wi-Fi. Пин 24 активирован.") << std::endl;
    } else {
        std::cerr << "Ошибка при установке значения пина " << pin << std::endl;
    }
}

int main() {
    checkWiFiAndControlPin(24); // Проверяем соединение и управляем пином 24
    return 0;
}
