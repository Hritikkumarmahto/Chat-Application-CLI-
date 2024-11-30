#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>    // For std::remove
#include <winsock2.h>   // For socket functions (Windows)
#include <ws2tcpip.h>   // For sockaddr_in (Windows)
#pragma comment(lib, "ws2_32.lib") // Link with Ws2_32.lib
#include <cstring>      // For memset

#define PORT 8080

std::vector<int> clients;
std::mutex clients_mutex;

// Function to broadcast messages to all connected clients
void broadcast(const std::string &message, int sender_fd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int client_fd : clients) {
        if (client_fd != sender_fd) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }
}

// Function to handle a client connection
void handle_client(int client_fd) {
    char buffer[1024];
    std::string welcome_msg = "Welcome to the chat!\n";
    send(client_fd, welcome_msg.c_str(), welcome_msg.size(), 0);

    while (true) {
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Client disconnected.\n";
            closesocket(client_fd);
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());
            break;
        }
        buffer[bytes_received] = '\0';
        std::string message = "Client " + std::to_string(client_fd) + ": " + buffer;
        std::cout << message;
        broadcast(message, client_fd);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server started on port " << PORT << ". Waiting for connections...\n";

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(new_socket);
        }
        std::cout << "New client connected: " << new_socket << "\n";
        std::thread(handle_client, new_socket).detach();
    }

    return 0;
}
