#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 33033
#define BUFFER_SIZE 1024

// GPIO pins for controlling the robot
#define GPIO_FORWARD 17
#define GPIO_BACKWARD 18
#define GPIO_LEFT 22
#define GPIO_RIGHT 23

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

void handle_command(const std::string& command) {
    if (command == "w") {
        //std::cout << "Moving Forward" << std::endl;
        set_gpio_value(GPIO_FORWARD, 1);
        set_gpio_value(GPIO_BACKWARD, 0);
        set_gpio_value(GPIO_LEFT, 0);
        set_gpio_value(GPIO_RIGHT, 0);
    } else if (command == "s") {
        //std::cout << "Moving Backward" << std::endl;
        set_gpio_value(GPIO_FORWARD, 0);
        set_gpio_value(GPIO_BACKWARD, 1);
        set_gpio_value(GPIO_LEFT, 0);
        set_gpio_value(GPIO_RIGHT, 0);
    } else if (command == "a") {
        //std::cout << "Turning Left" << std::endl;
        set_gpio_value(GPIO_FORWARD, 0);
        set_gpio_value(GPIO_BACKWARD, 0);
        set_gpio_value(GPIO_LEFT, 1);
        set_gpio_value(GPIO_RIGHT, 0);
    } else if (command == "d") {
        //std::cout << "Turning Right" << std::endl;
        set_gpio_value(GPIO_FORWARD, 0);
        set_gpio_value(GPIO_BACKWARD, 0);
        set_gpio_value(GPIO_LEFT, 0);
        set_gpio_value(GPIO_RIGHT, 1);
    } else {
        std::cout << "Unknown Command: " << command << std::endl;
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Set up GPIO pins
    setup_gpio(GPIO_FORWARD);
    setup_gpio(GPIO_BACKWARD);
    setup_gpio(GPIO_LEFT);
    setup_gpio(GPIO_RIGHT);

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        return -1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        return -1;
    }

    // Bind the socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return -1;
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    // Accept a connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        return -1;
    }

    std::cout << "Connection accepted" << std::endl;

    while (true) {
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            break;
        }

        std::string command(buffer);
        handle_command(command);

        // Clear the buffer
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(new_socket);
    close(server_fd);

    return 0;
}
