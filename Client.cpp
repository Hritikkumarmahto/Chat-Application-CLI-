#include <iostream>
#include <thread>
#include <string>
#include <arpa/inet.h> // For sockets (Linux/Mac)
#include <unistd.h>    // For close()

#define PORT 8080

// Function to handle receiving messages
void receive_messages(int sock) {
    char buffer[1024];
    while (true) {
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server.\n";
            close(sock);
            exit(0);
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer << std::endl;
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error.\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address.\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        return -1;
    }

    std::cout << "Connected to server. Type your messages below:\n";

    // Start receiving messages in a separate thread
    std::thread receiver(receive_messages, sock);
    receiver.detach();

    // Send messages
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(sock, message.c_str(), message.size(), 0);
    }

    return 0;
}
