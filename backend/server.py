import socket
import threading

from protocol import encode_message, decode_message

HOST = "0.0.0.0"
PORT = 5000

clients = {}
clients_lock = threading.Lock()


def send_message(client_socket, message):
    client_socket.sendall(encode_message(message))


def broadcast(message, exclude=None):
    dead_clients = []

    with clients_lock:
        for client_socket in clients:
            if client_socket == exclude:
                continue

            try:
                send_message(client_socket, message)
            except OSError:
                dead_clients.append(client_socket)

        for client_socket in dead_clients:
            clients.pop(client_socket, None)


def handle_client(client_socket, client_address):
    username = None

    print(f"Client connected: {client_address}")

    try:
        # First message must contain the username
        data = client_socket.recv(4096)

        if not data:
            return

        message = decode_message(data.decode("utf-8").strip())

        if message.get("type") != "join":
            send_message(
                client_socket,
                {
                    "type": "error",
                    "message": "First message must be a join request."
                }
            )
            return

        username = message.get("username", "").strip()

        if not username:
            send_message(
                client_socket,
                {
                    "type": "error",
                    "message": "Username cannot be empty."
                }
            )
            return

        with clients_lock:
            if username in clients.values():
                send_message(
                    client_socket,
                    {
                        "type": "error",
                        "message": "Username already in use."
                    }
                )
                return

            clients[client_socket] = username

        print(f"{username} joined from {client_address}")

        send_message(
            client_socket,
            {
                "type": "system",
                "message": f"Welcome, {username}!"
            }
        )

        broadcast(
            {
                "type": "system",
                "message": f"{username} joined the chat."
            },
            exclude=client_socket
        )

        while True:
            data = client_socket.recv(4096)

            if not data:
                break

            message = decode_message(data.decode("utf-8").strip())

            if message.get("type") == "chat":
                text = message.get("text", "").strip()

                if not text:
                    continue

                chat_message = {
                    "type": "chat",
                    "username": username,
                    "text": text
                }

                print(f"{username}: {text}")

                broadcast(chat_message)

            elif message.get("type") == "quit":
                break

            else:
                send_message(
                    client_socket,
                    {
                        "type": "error",
                        "message": "Unknown message type."
                    }
                )

    except (OSError, ValueError) as error:
        print(f"Connection error with {client_address}: {error}")

    finally:
        with clients_lock:
            clients.pop(client_socket, None)

        client_socket.close()

        if username:
            print(f"{username} disconnected")

            broadcast(
                {
                    "type": "system",
                    "message": f"{username} left the chat."
                }
            )


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
            args=(client_socket, client_address),
            daemon=True
        )

        thread.start()


if __name__ == "__main__":
    start_server()
