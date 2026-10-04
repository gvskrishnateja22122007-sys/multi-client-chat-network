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

static void broadcast_message(struct pollfd clients[],
                              int client_count,
                              int sender_fd,
                              const char *message,
                              ssize_t message_length)
{
    for (int i = 1; i < client_count; i++)
    {
        int fd = clients[i].fd;

        if (fd == -1 || fd == sender_fd)
        {
            continue;
        }

        ssize_t sent = send(fd, message, message_length, 0);

        if (sent == -1)
        {
            perror("send");

            /*
             * The client may have disconnected.
             * Remove it safely.
             */
            close(fd);
            clients[i].fd = -1;
            clients[i].events = POLLIN;

            printf("Removed disconnected client.\n");
        }
    }
}

int main(void)
{
    int server_fd;
    struct sockaddr_in server_addr;

    struct pollfd clients[MAX_CLIENTS + 1];

    /*
     * Prevent a broken client connection from
     * terminating the entire server.
     */
    signal(SIGPIPE, SIG_IGN);

    /*
     * Initialize all client slots.
     */
    for (int i = 0; i <= MAX_CLIENTS; i++)
    {
        clients[i].fd = -1;
        clients[i].events = POLLIN;
        clients[i].revents = 0;
    }

    /*
     * 1. Create server socket
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    /*
     * Allow immediate reuse of port 8080.
     */
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
     * 2. Configure server address
     */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    /*
     * 3. Bind
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
     * 4. Listen
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

    /*
     * Slot 0 is the listening socket.
     */
    clients[0].fd = server_fd;

    /*
     * 5. Main server loop
     */
    while (1)
    {
        int ready = poll(clients, MAX_CLIENTS + 1, -1);

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
         * Check for a new connection.
         */
        if (clients[0].revents & POLLIN)
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client_fd = accept(server_fd,
                                   (struct sockaddr *)&client_addr,
                                   &client_len);

            if (client_fd == -1)
            {
                perror("accept");
            }
            else
            {
                int added = 0;

                for (int i = 1; i <= MAX_CLIENTS; i++)
                {
                    if (clients[i].fd == -1)
                    {
                        clients[i].fd = client_fd;
                        clients[i].events = POLLIN;

                        printf("Client connected: %s:%d\n",
                               inet_ntoa(client_addr.sin_addr),
                               ntohs(client_addr.sin_port));

                        const char *welcome =
                            "Welcome to the Multi-Client Chat Server!\n";

                        if (send(client_fd,
                                 welcome,
                                 strlen(welcome),
                                 0) == -1)
                        {
                            perror("send welcome");
                            close(client_fd);
                            clients[i].fd = -1;
                        }

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

                    printf("Rejected client: server full.\n");
                }
            }
        }

        /*
         * Check connected clients.
         */
        for (int i = 1; i <= MAX_CLIENTS; i++)
        {
            int client_fd = clients[i].fd;

            if (client_fd == -1)
            {
                continue;
            }

            if (clients[i].revents &
                (POLLIN | POLLHUP | POLLERR))
            {
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
                    printf("Client disconnected.\n");

                    close(client_fd);
                    clients[i].fd = -1;
                    clients[i].events = POLLIN;

                    continue;
                }

                buffer[bytes_received] = '\0';

                printf("Message from client %d: %s",
                       client_fd,
                       buffer);

                /*
                 * Prepare broadcast message.
                 */
                char message[BUFFER_SIZE + 64];

                int length = snprintf(message,
                                      sizeof(message),
                                      "Client %d: %s",
                                      client_fd,
                                      buffer);

                if (length > 0)
                {
                    broadcast_message(clients,
                                       MAX_CLIENTS + 1,
                                       client_fd,
                                       message,
                                       length);
                }
            }
        }
    }

    /*
     * Cleanup.
     */
    for (int i = 0; i <= MAX_CLIENTS; i++)
    {
        if (clients[i].fd != -1)
        {
            close(clients[i].fd);
        }
    }

    return EXIT_SUCCESS;
}
