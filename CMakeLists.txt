cmake_minimum_required (VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

file(GLOB server_file "server.cpp")
file(GLOB client_file "client.cpp")

add_executable(server
        ${server_file}
        "client_server/server_status.hpp"
        "client_server/client_request.hpp"
        "inverted_index/inverted_index.hpp"
        "inverted_index/index_text_files.hpp"
        "thread_pool/thread_pool.hpp"
        "inverted_index/inverted_index.cpp"
        "inverted_index/index_text_files.cpp"
        "thread_pool/thread_pool.cpp"
)

add_executable(client1
        ${client_file}
        "client_server/server_status.hpp"
        "client_server/client_request.hpp"
)

add_executable(client2
        ${client_file}
        "client_server/server_status.hpp"
        "client_server/client_request.hpp"
)
