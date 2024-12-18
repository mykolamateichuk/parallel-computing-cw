#ifndef PARALLEL_COMPUTING_CW_SERVER_STATUS_HPP
#define PARALLEL_COMPUTING_CW_SERVER_STATUS_HPP

#include <iostream>

enum ServerStatus {
    ClientConnected,
    RequestReceived,
    RequestProcessing,
    RequestProcessed,
    ResponseSent,
    ClientDisconnected
};

std::string server_status_msg(ServerStatus server_status) {
    switch (server_status) {
        case 0:
            return "ClientConnected";
        case 1:
            return "RequestReceived";
        case 2:
            return "RequestProcessing";
        case 3:
            return "RequestProcessed";
        case 4:
            return "ResponseSent";
        case 5:
            return "ClientDisconnected";
    }
}

#endif //PARALLEL_COMPUTING_CW_SERVER_STATUS_HPP
