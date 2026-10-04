#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#define SERVER_PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 32

typedef struct
{
    int fd;
    char username[USERNAME_SIZE];
} Client;

static void broadcast_message(struct pollfd poll_clients[],
                              Client clients[],
                              int count,
                              int sender_fd,
                              const char *message)
{
    for (int i = 1; i < count; i++)
    {
        if (poll_clients[i].fd == -1)
        {
            continue;
        }

        if (poll_clients[i].fd == sender_fd)
        {
            continue;
        }

        if (send(poll_clients[i].fd,
                 message,
                 strlen(message),
                 0) == -1)
        {
            perror("send");
        }
    }
}

static int find_client(struct pollfd poll_clients[],
                       int count,
                       int fd)
{
    for (int i = 1; i < count; i++)
    {
        if (poll_clients[i].fd == fd)
        {
            return i;
        }
    }

    return -1;
}

int main(void)
{
    int server_fd;

    struct sockaddr_in server_addr;

    struct pollfd poll_clients[MAX_CLIENTS + 1];

    Client clients[MAX_CLIENTS + 1];

    signal(SIGPIPE, SIG_IGN);

    /*
     * Initialize client slots.
     */
    for (int i = 0; i <= MAX_CLIENTS; i++)
    {
        poll_clients[i].fd = -1;
        poll_clients[i].events = POLLIN;
        poll_clients[i].revents = 0;

        clients[i].fd = -1;
        clients[i].username[0] = '\0';
    }

    /*
     * Create server socket.
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    int reuse = 1;

    if (setsockopt(server_fd,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &reuse,
                   sizeof(reuse)) == -1)
    {
        perror("setsockopt");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server socket created successfully.\n");

    /*
     * Configure server address.
     */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    /*
     * Bind.
     */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server bound to port %d.\n", SERVER_PORT);

    /*
     * Listen.
     */
    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server is listening on port %d...\n", SERVER_PORT);
    printf("Maximum clients: %d\n", MAX_CLIENTS);
    printf("Waiting for connections...\n\n");

    poll_clients[0].fd = server_fd;
    clients[0].fd = server_fd;

    /*
     * Main server loop.
     */
    while (1)
    {
        int ready = poll(poll_clients,
                         MAX_CLIENTS + 1,
                         -1);

        if (ready == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("poll");
            break;
        }

        /*
         * New connection.
         */
        if (poll_clients[0].revents & POLLIN)
        {
            struct sockaddr_in client_addr;

            socklen_t client_len =
                sizeof(client_addr);

            int client_fd =
                accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);

            if (client_fd == -1)
            {
                perror("accept");
            }
            else
            {
                int added = 0;

                for (int i = 1;
                     i <= MAX_CLIENTS;
                     i++)
                {
                    if (poll_clients[i].fd == -1)
                    {
                        poll_clients[i].fd = client_fd;
                        poll_clients[i].events = POLLIN;

                        clients[i].fd = client_fd;
                        clients[i].username[0] = '\0';

                        printf(
                            "New connection from %s:%d\n",
                            inet_ntoa(client_addr.sin_addr),
                            ntohs(client_addr.sin_port));

                        const char *prompt =
                            "Enter your username: ";

                        send(client_fd,
                             prompt,
                             strlen(prompt),
                             0);

                        added = 1;
                        break;
                    }
                }

                if (!added)
                {
                    const char *full =
                        "Server is full. Try again later.\n";

                    send(client_fd,
                         full,
                         strlen(full),
                         0);

                    close(client_fd);

                    printf(
                        "Rejected client: server full.\n");
                }
            }
        }

        /*
         * Handle connected clients.
         */
        for (int i = 1;
             i <= MAX_CLIENTS;
             i++)
        {
            int client_fd =
                poll_clients[i].fd;

            if (client_fd == -1)
            {
                continue;
            }

            if (!(poll_clients[i].revents &
                  (POLLIN | POLLHUP | POLLERR)))
            {
                continue;
            }

            char buffer[BUFFER_SIZE];

            ssize_t bytes_received =
                recv(client_fd,
                     buffer,
                     sizeof(buffer) - 1,
                     0);

            /*
             * Client disconnected.
             */
            if (bytes_received <= 0)
            {
                if (clients[i].username[0] != '\0')
                {
                    char leave_message[BUFFER_SIZE];

                    snprintf(
                        leave_message,
                        sizeof(leave_message),
                        "[Server]: %s left the chat.\n",
                        clients[i].username);

                    printf("%s", leave_message);

                    broadcast_message(
                        poll_clients,
                        clients,
                        MAX_CLIENTS + 1,
                        client_fd,
                        leave_message);
                }

                printf("Client disconnected.\n");

                close(client_fd);

                poll_clients[i].fd = -1;
                clients[i].fd = -1;
                clients[i].username[0] = '\0';

                continue;
            }

            buffer[bytes_received] = '\0';

            /*
             * First message = username.
             */
            if (clients[i].username[0] == '\0')
            {
                buffer[strcspn(buffer, "\r\n")] = '\0';

                strncpy(clients[i].username,
                        buffer,
                        USERNAME_SIZE - 1);

                clients[i].username[
                    USERNAME_SIZE - 1] = '\0';

                printf("Username registered: %s\n",
                       clients[i].username);

                char welcome[BUFFER_SIZE];

                snprintf(
                    welcome,
                    sizeof(welcome),
                    "[Server]: Welcome, %s!\n",
                    clients[i].username);

                send(client_fd,
                     welcome,
                     strlen(welcome),
                     0);

                char join_message[BUFFER_SIZE];

                snprintf(
                    join_message,
                    sizeof(join_message),
                    "[Server]: %s joined the chat.\n",
                    clients[i].username);

                broadcast_message(
                    poll_clients,
                    clients,
                    MAX_CLIENTS + 1,
                    client_fd,
                    join_message);

                continue;
            }

            /*
             * Normal chat message.
             */
            char message[BUFFER_SIZE];

            snprintf(
                message,
                sizeof(message),
                "[%s]: %s",
                clients[i].username,
                buffer);

            printf("%s", message);

            broadcast_message(
                poll_clients,
                clients,
                MAX_CLIENTS + 1,
                client_fd,
                message);
        }
    }

    /*
     * Cleanup.
     */
    for (int i = 0;
         i <= MAX_CLIENTS;
         i++)
    {
        if (poll_clients[i].fd != -1)
        {
            close(poll_clients[i].fd);
        }
    }

    return EXIT_SUCCESS;
}
