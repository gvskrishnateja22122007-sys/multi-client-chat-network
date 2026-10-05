import socket
import threading

HOST = "0.0.0.0"
PORT = 5000

clients = []
clients_lock = threading.Lock()


def handle_client(client_socket, client_address):
    print(f"Client connected: {client_address}")

    with clients_lock:
        clients.append(client_socket)

    try:
        client_socket.sendall(
            b"Connected to Multi-Client Chat Server\n"
        )

        while True:
            data = client_socket.recv(1024)

            if not data:
                break

            message = data.decode("utf-8").strip()

            if message:
                print(f"{client_address}: {message}")

                with clients_lock:
                    for client in clients:
                        if client != client_socket:
                            try:
                                client.sendall(
                                    f"{client_address[0]}: {message}\n".encode("utf-8")
                                )
                            except OSError:
                                pass

    except OSError as error:
        print(f"Connection error with {client_address}: {error}")

    finally:
        with clients_lock:
            if client_socket in clients:
                clients.remove(client_socket)

        client_socket.close()

        print(f"Client disconnected: {client_address}")


def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server.setsockopt(
        socket.SOL_SOCKET,
        socket.SO_REUSEADDR,
        1
    )

    server.bind((HOST, PORT))
    server.listen()

    print(f"Server listening on {HOST}:{PORT}")

    while True:
        client_socket, client_address = server.accept()

        thread = threading.Thread(
            target=handle_client,
            args=(client_socket, client_address)
        )

        thread.start()


if __name__ == "__main__":
    start_server()
