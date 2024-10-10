#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <fstream>
#include <cerrno>

#define PORT 33033
#define BUFFER_SIZE 1024

// GPIO pins for controlling the robot
#define GPIO_FORWARD 17
#define GPIO_BACKWARD 18
#define GPIO_LEFT 22
#define GPIO_RIGHT 23

// UART settings
#define UART_DEVICE "/dev/ttyUSB0"
#define UART_BAUD_RATE B9600

void setup_gpio(int gpio) {
    std::ostringstream ss;
    ss << "/sys/class/gpio/export";
    std::ofstream export_gpio(ss.str());
    export_gpio << gpio;

    ss.str("");
    ss << "/sys/class/gpio/gpio" << gpio << "/direction";
    std::ofstream direction_gpio(ss.str());
    direction_gpio << "out";
}

void set_gpio_value(int gpio, int value) {
    std::ostringstream ss;
    ss << "/sys/class/gpio/gpio" << gpio << "/value";
    std::ofstream value_gpio(ss.str());
    value_gpio << value;
}

void setup_uart(int &uart_fd) {
    uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY | O_SYNC);
    if (uart_fd < 0) {
        std::cerr << "Failed to open UART device: " << strerror(errno) << std::endl;
        return;
    }

    struct termios tty;
    if (tcgetattr(uart_fd, &tty) != 0) {
        std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
        return;
    }

    cfsetospeed(&tty, UART_BAUD_RATE);
    cfsetispeed(&tty, UART_BAUD_RATE);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit_chars
    tty.c_iflag &= ~IGNBRK;                         // disable break processing
    tty.c_lflag = 0;                                // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;                            // read doesn't block
    tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);              // shut off parity
    tty.c_cflag &= ~CSTOPB;                         // only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                        // no hardware flowcontrol

    if (tcsetattr(uart_fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error_from_tcsetattr: " << strerror(errno) << std::endl;
        return;
    }
}

void send_uart(int uart_fd, const std::string &message) {
    if (write(uart_fd, message.c_str(), message.size()) < 0) {
        std::cerr << "Failed to send message via UART: " << strerror(errno) << std::endl;
    }
}

void handle_command(const std::string &command, int uart_fd) {
    int axis0, axis1;
    if (sscanf(command.c_str(), "%d,%d", &axis0, &axis1) != 2) {
        std::cerr << "Invalid_command_format: " << command << std::endl;
        return;
    }

    // Map joystick values to GPIO commands
    if (axis0 > 0) {
        // Right
        set_gpio_value(GPIO_RIGHT, 1);
        set_gpio_value(GPIO_LEFT, 0);
    } else if (axis0 < 0) {
        // Left
        set_gpio_value(GPIO_RIGHT, 0);
        set_gpio_value(GPIO_LEFT, 1);
    } else {
        set_gpio_value(GPIO_RIGHT, 0);
        set_gpio_value(GPIO_LEFT, 0);
    }

    if (axis1 > 0) {
        // Backward
        set_gpio_value(GPIO_FORWARD, 0);
        set_gpio_value(GPIO_BACKWARD, 1);
    } else if (axis1 < 0) {
        // Forward
        set_gpio_value(GPIO_FORWARD, 1);
        set_gpio_value(GPIO_BACKWARD, 0);
    } else {
        set_gpio_value(GPIO_FORWARD, 0);
        set_gpio_value(GPIO_BACKWARD, 0);
    }

    // Send the current state of the joystick axes to the Arduino
    char message[16];
    snprintf(message, sizeof(message), "%d,%d\n", axis0, axis1);
    send_uart(uart_fd, message);
}

void log_message(const std::string &message) {
    std::ofstream log_file("log.txt", std::ios_base::app);
    if (log_file.is_open()) {
        log_file << message << std::endl;
    }
}

int main() {
    int server_fd, new_socket, uart_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Set up GPIO pins
    setup_gpio(GPIO_FORWARD);
    setup_gpio(GPIO_BACKWARD);
    setup_gpio(GPIO_LEFT);
    setup_gpio(GPIO_RIGHT);

    // Set up UART
    setup_uart(uart_fd);

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        log_message("socket failed: " + std::string(strerror(errno)));
        return -1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        log_message("setsockopt failed: " + std::string(strerror(errno)));
        return -1;
    }

    // Bind the socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        log_message("bind failed: " + std::string(strerror(errno)));
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        log_message("listen failed: " + std::string(strerror(errno)));
        return -1;
    }

    log_message("Server is listening on port " << PORT);

    // Accept a connection
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        log_message("accept.failed: " + std::string(strerror(errno)));
        return -1;
    }

    log_message("Connection accepted");

    while (true) {
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            break;
        }

        std::string command(buffer);
        handle_command(command, uart_fd);

        // Clear the buffer
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(new_socket);
    close(server_fd);
    close(uart_fd);

    return 0;
}