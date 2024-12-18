#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "client_server/server_status.hpp"
#include "client_server/client_request.hpp"

const int PORT = 8000;
const char* SERVER_IP = "127.0.0.1";

void end_connection(int socket_fd) {
    ClientRequest request_type = ClientRequest::Disconnect;
    write(socket_fd, &request_type, sizeof(int));
}

std::vector<std::string> search_handler(int socket_fd, const std::string& argument) {
    ClientRequest request_type = ClientRequest::SearchTerm;
    if (send(socket_fd, &request_type, sizeof(int), 0) != sizeof(int)) {
        throw std::runtime_error("Failed to send request type");
    }

    std::size_t arg_len = argument.size();
    if (send(socket_fd, &arg_len, sizeof(std::size_t), 0) != sizeof(std::size_t)) {
        throw std::runtime_error("Failed to send argument length");
    }

    if (send(socket_fd, argument.c_str(), arg_len, 0) != static_cast<ssize_t>(arg_len)) {
        throw std::runtime_error("Failed to send argument data");
    }

    std::size_t results_len;
    if (recv(socket_fd, &results_len, sizeof(std::size_t), MSG_WAITALL) != sizeof(std::size_t)) {
        throw std::runtime_error("Failed to receive results length");
    }

    std::vector<std::string> results;
    for (std::size_t i = 0; i < results_len; ++i) {
        std::size_t file_len;
        if (recv(socket_fd, &file_len, sizeof(std::size_t), MSG_WAITALL) != sizeof(std::size_t)) {
            throw std::runtime_error("Failed to receive file name length");
        }

        std::vector<char> buffer(file_len + 1, 0); // +1 for null-termination
        if (recv(socket_fd, buffer.data(), file_len, MSG_WAITALL) != static_cast<ssize_t>(file_len)) {
            throw std::runtime_error("Failed to receive file name");
        }

        results.emplace_back(buffer.data(), file_len);
    }

    return results;
}

void reindex_handler(int socket_fd) {
    ClientRequest request_type = ClientRequest::StartReindex;
    if (send(socket_fd, &request_type, sizeof(int), 0) != sizeof(int)) {
        throw std::runtime_error("Failed to send request type");
    }
}

void status_handler(int socket_fd) {
    ClientRequest request_type = ClientRequest::GetStatus;
    if (send(socket_fd, &request_type, sizeof(int), 0) != sizeof(int)) {
        throw std::runtime_error("Failed to send request type");
    }

    ServerStatus server_status;
    if (recv(socket_fd, &server_status, sizeof(int), MSG_WAITALL) != sizeof(int)) {
        throw std::runtime_error("Failed to receive server status");
    }

    if (server_status == ServerStatus::ClientConnected) {
        std::printf("You don't have any tasks running on the server!\n");
    }
    else if (server_status == ServerStatus::RequestProcessing) {
        std::printf("Your request is still being processed!\n");
    }
    else if (server_status == ServerStatus::RequestProcessed) {
        std::printf("Reindex finished successfully!\n");
    }
}

void handle_input(int socket_fd) {
    std::string input;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "DISCONNECT") {
            end_connection(socket_fd);
            return;
        }

        std::istringstream stream(input);

        std::string command;
        stream >> command;

        std::string argument;
        stream >> argument;

        if (command == "SEARCH") {
            std::vector<std::string> results = search_handler(socket_fd, argument);
            for (const auto& file : results) std::printf("'%s'\n", file.c_str());
        }
        else if (command == "REINDEX") {
            reindex_handler(socket_fd);
        }
        else if (command == "STATUS") {
            status_handler(socket_fd);
        }
        else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }
}

int main() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "[ERROR]\t\tFailed to create socket\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[ERROR]\t\tFailed to connect to server\n";
        close(socket_fd);
        return 1;
    }

    std::printf("[INFO]\t\tConnected to the server!\n");

    handle_input(socket_fd);

    close(socket_fd);
    return 0;
}
