from enum import Enum

CLIENT_REQUEST = {
    "SearchTerm": 0,
    "StartIndex": 1,
    "GetStatus": 2,
    "Disconnect": 3
}

SERVER_STATUS = {
    "ClientConnected": 0,
    "RequestReceived": 1,
    "RequestProcessing": 2,
    "RequestProcessed": 3,
    "ResponseSent": 4,
    "ClientDisconnected": 5
}
