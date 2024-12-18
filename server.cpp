#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <functional>
#include <filesystem>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "client_server/server_status.hpp"
#include "client_server/client_request.hpp"

#include "inverted_index/inverted_index.hpp"
#include "inverted_index/index_text_files.hpp"

#include "thread_pool/thread_pool.hpp"

constexpr int PORT = 8000;
constexpr std::size_t NUM_WORKERS = 4;

std::unordered_map<int*, ServerStatus> server_status_map;
std::unordered_map<int*, std::vector<std::future<void>>> client_reindex_tasks;

inverted_index_parallel inv_index;
thread_pool workers(inv_index, NUM_WORKERS);

bool search_request_handler(int client_socket) {
    server_status_map[&client_socket] = ServerStatus::RequestReceived;
    std::printf("[STATUS]\t%s\n", server_status_msg(server_status_map[&client_socket]).c_str());

    std::size_t term_len;
    if (recv(client_socket, &term_len, sizeof(std::size_t), MSG_WAITALL) != sizeof(std::size_t)) {
        std::cerr << "Failed to receive term length.\n";
        return false;
    }

    if (term_len == 0 || term_len > 1024) {
        std::cerr << "Invalid term length: " << term_len << "\n";
        return false;
    }

    std::vector<char> buffer(term_len + 1, 0);
    if (recv(client_socket, buffer.data(), term_len, MSG_WAITALL) != static_cast<ssize_t>(term_len)) {
        std::cerr << "Failed to receive search term.\n";
        return false;
    }

    std::string term(buffer.data(), term_len);
    std::printf("[INFO]\t\tSearching for '%s' ...\n", term.c_str());

    std::set<std::string> results = inv_index.find(term);

    std::size_t results_len = results.size();
    if (send(client_socket, &results_len, sizeof(std::size_t), 0) != sizeof(std::size_t)) {
        std::cerr << "Failed to send results length.\n";
        return false;
    }

    for (const auto& file : results) {
        std::size_t file_len = file.size();
        if (send(client_socket, &file_len, sizeof(std::size_t), 0) != sizeof(std::size_t)) {
            std::cerr << "Failed to send file name length.\n";
            return false;
        }
        if (send(client_socket, file.c_str(), file_len, 0) != static_cast<ssize_t>(file_len)) {
            std::cerr << "Failed to send file name.\n";
            return false;
        }
    }
    std::printf("[INFO]\t\tResults sent successfully!\n");

    return true;
}

bool reindex_request_handler(int client_socket) {
    server_status_map[&client_socket] = ServerStatus::RequestReceived;
    std::printf("[STATUS]\t%s\n", server_status_msg(server_status_map[&client_socket]).c_str());
    std::printf("[INFO]\t\tStarting reindex...\n");

    std::vector<std::string> file_names = get_file_names(FILES_DIR_PATH);
    std::vector<std::pair<std::size_t, std::size_t>> worker_bounds = get_bounds(file_names.size(), NUM_WORKERS);

    std::vector<std::future<void>> futs;
    for (int i = 0; i < NUM_WORKERS; i++) {
        std::vector<std::string> temp;
        for (std::size_t j = worker_bounds[i].first; j <= worker_bounds[i].second; j++) {
            temp.push_back(file_names[j]);
        }
        futs.push_back(workers.add_task(index_text_files, temp));
    }

    client_reindex_tasks[&client_socket] = std::move(futs);
    return true;
}

bool status_request_handler(int client_socket) {
    server_status_map[&client_socket] = ServerStatus::RequestReceived;
    std::printf("[STATUS]\t%s\n", server_status_msg(server_status_map[&client_socket]).c_str());

    std::size_t num_tasks = client_reindex_tasks[&client_socket].size();

    if (num_tasks == 0) {
        ServerStatus server_status = ServerStatus::ClientConnected;
        if (send(client_socket, &server_status, sizeof(int), 0) != sizeof(int)) {
            std::cerr << "Failed to send server status.\n";
            return false;
        }
        return true;
    }

    std::size_t num_ready_tasks = 0;
    for (const auto& task : client_reindex_tasks[&client_socket]) {
        if ((int) task.wait_for(std::chrono::milliseconds(0)) == 0) {
            num_ready_tasks += 1;
        }
    }

    ServerStatus server_status;
    if (num_ready_tasks == num_tasks) {
        server_status = ServerStatus::RequestProcessed;

    } else {
        server_status = ServerStatus::RequestProcessing;
    }

    if (send(client_socket, &server_status, sizeof(int), 0) != sizeof(int)) {
        std::cerr << "Failed to send server status.\n";
        return false;
    }

    return true;
}

bool disconnect_handler(int client_socket) {
    server_status_map[&client_socket] = ServerStatus::ClientDisconnected;
    std::printf("[STATUS]\t%s\n", server_status_msg(server_status_map[&client_socket]).c_str());

    return false;
}

bool handle_request(int client_socket) {
    ClientRequest client_req;
    recv(client_socket, &client_req, sizeof(int), 0);

    std::printf("[SUCCESS]\tRequest received: %s\n", client_request_msg(client_req).c_str());

    std::unordered_map<ClientRequest, std::function<bool(int)>> request_handlers =
    {
        { ClientRequest::SearchTerm,    search_request_handler  },
        { ClientRequest::StartReindex,  reindex_request_handler },
        { ClientRequest::GetStatus,     status_request_handler  },
        { ClientRequest::Disconnect,    disconnect_handler      }
    };

    return request_handlers[client_req](client_socket);
}

void handle_client(int client_socket) {
    server_status_map[&client_socket] = ServerStatus::ClientConnected;
    std::printf("[STATUS]\t%s\n", server_status_msg(server_status_map[&client_socket]).c_str());

    while (handle_request(client_socket)) {}

    close(client_socket);
}

int main() {
    std::vector<std::string> file_names = get_file_names(FILES_DIR_PATH);
    std::vector<std::pair<std::size_t, std::size_t>> worker_bounds = get_bounds(file_names.size(), NUM_WORKERS);

    std::vector<std::future<void>> futs;
    for (int i = 0; i < NUM_WORKERS; i++) {
        std::vector<std::string> temp;
        for (std::size_t j = worker_bounds[i].first; j <= worker_bounds[i].second; j++) {
            temp.push_back(file_names[j]);
        }
        workers.add_task(index_text_files, temp);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "[ERROR]\t\tFailed to create socket\n";
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[ERROR]\t\tFailed to bind socket\n";
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        std::cerr << "[ERROR]\t\tFailed to listen on socket\n";
        return 1;
    }

    try {
        std::printf("[INFO]\t\tListening for client to connect...\n");
        while (true) {
            client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
            if (client_socket < 0) {
                std::cerr << "[ERROR]\t\tFailed to accept client connection\n";
                continue;
            }

            std::thread(handle_client, client_socket).detach();
        }
    }
    catch (const char* exception) {
		std::cerr << "[ERROR]\t\tException caught: " << exception << "\n";
	}

    close(server_socket);
    return 0;
}
