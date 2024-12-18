import socket
import struct

from utils import CLIENT_REQUEST, SERVER_STATUS

SERVER_IP = "127.0.0.1"
PORT = 8000


def handle_disconnect(client_socket: socket.socket) -> None:
    req = CLIENT_REQUEST["Disconnect"]
    client_socket.send(struct.pack("I", req))


def search_handler(client_socket: socket.socket, term: str) -> list[str]:
    req = CLIENT_REQUEST["SearchTerm"]
    client_socket.send(struct.pack("I", req))

    term_len = len(term)
    client_socket.send(struct.pack("N", term_len), 0)

    client_socket.send(bytes(term, encoding="utf-8"), 0)

    results_len = struct.unpack("N", client_socket.recv(8, socket.MSG_WAITALL))[0]

    results = [None for _ in range(results_len)]
    for i in range(results_len):
        file_len = struct.unpack(
            "N", client_socket.recv(8, socket.MSG_WAITALL)
        )[0]
        results[i] = struct.unpack(
            f"{file_len}s", client_socket.recv(file_len, socket.MSG_WAITALL)
        )[0].decode("utf-8")

    return results


def reindex_handler(client_socket: socket.socket) -> None:
    req = CLIENT_REQUEST["StartIndex"]
    client_socket.send(struct.pack("I", req))


def status_handler(client_socket: socket.socket) -> None:
    req = CLIENT_REQUEST["GetStatus"]
    client_socket.send(struct.pack("I", req))

    serv_status = struct.unpack(
        "I", client_socket.recv(4, socket.MSG_WAITALL)
    )[0]

    if serv_status == SERVER_STATUS["ClientConnected"]:
        print("You don't have any tasks running on the server!")
    elif serv_status == SERVER_STATUS["RequestProcessing"]:
        print("Your request is still being processed!")
    elif serv_status == SERVER_STATUS["RequestProcessed"]:
        print("Reindex finished successfully!")


def handle_input(client_socket: socket.socket) -> None:
    while True:
        inp = input("> ")
        literals = inp.split()

        if literals[0] == "SEARCH":
            results = search_handler(client_socket, literals[1])
            for file in results:
                print(f"'{file}'")
        elif literals[0] == "REINDEX":
            reindex_handler(client_socket)
        elif literals[0] == "STATUS":
            status_handler(client_socket)
        elif literals[0] == "DISCONNECT":
            handle_disconnect(client_socket)
            return
        else:
            print(f"Unknown command: {literals[0]}")


def main() -> None:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
    server_addr = (SERVER_IP, PORT)

    try:
        client_socket.connect(server_addr)
        handle_input(client_socket)

    except Exception as e:
        print(f"Exception caught: {e}")
    finally:
        client_socket.shutdown(socket.SHUT_RDWR)
        client_socket.close()


if __name__ == "__main__":
    main()
