#ifndef PARALLEL_COMPUTING_CW_CLIENT_REQUEST_HPP
#define PARALLEL_COMPUTING_CW_CLIENT_REQUEST_HPP

#include <iostream>

enum ClientRequest {
    SearchTerm,
    StartReindex,
    GetStatus,
    Disconnect
};

std::string client_request_msg(ClientRequest client_request) {
    switch (client_request) {
        case 0:
            return "SearchTerm";
        case 1:
            return "StartReindex";
        case 2:
            return "GetStatus";
        case 3:
            return "Disconnect";
    }
}

#endif //PARALLEL_COMPUTING_CW_CLIENT_REQUEST_HPP
