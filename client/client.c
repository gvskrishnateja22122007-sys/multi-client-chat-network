#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    int client_fd;

    struct sockaddr_in server_addr;

    struct pollfd fds[2];

    char buffer[BUFFER_SIZE];

    /*
     * Create socket.
     */
    client_fd = socket(AF_INET,
                       SOCK_STREAM,
                       0);

    if (client_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    printf("Client socket created successfully.\n");

    /*
     * Configure server address.
     */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET,
                  SERVER_IP,
                  &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(client_fd);
        return EXIT_FAILURE;
    }

    /*
     * Connect.
     */
    printf("Connecting to server...\n");

    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(client_fd);
        return EXIT_FAILURE;
    }

    printf("Connected to server!\n");

    /*
     * Receive username prompt.
     */
    ssize_t received =
        recv(client_fd,
             buffer,
             sizeof(buffer) - 1,
             0);

    if (received <= 0)
    {
        printf("Server disconnected.\n");
        close(client_fd);
        return EXIT_FAILURE;
    }

    buffer[received] = '\0';

    printf("%s", buffer);

    /*
     * Enter username.
     */
    if (fgets(buffer,
              sizeof(buffer),
              stdin) == NULL)
    {
        close(client_fd);
        return EXIT_FAILURE;
    }

    if (send(client_fd,
             buffer,
             strlen(buffer),
             0) == -1)
    {
        perror("send");
        close(client_fd);
        return EXIT_FAILURE;
    }

    /*
     * Set up poll().
     *
     * fds[0] = keyboard
     * fds[1] = server
     */
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = client_fd;
    fds[1].events = POLLIN;

    printf("\nYou are connected to the chat.\n");
    printf("Type messages and press Enter.\n");
    printf("Type /quit to leave.\n\n");

    while (1)
    {
        int ready = poll(fds,
                         2,
                         -1);

        if (ready == -1)
        {
            perror("poll");
            break;
        }

        /*
         * Keyboard input.
         */
        if (fds[0].revents & POLLIN)
        {
            if (fgets(buffer,
                      sizeof(buffer),
                      stdin) == NULL)
            {
                break;
            }

            if (strncmp(buffer,
                        "/quit",
                        5) == 0)
            {
                break;
            }

            if (send(client_fd,
                     buffer,
                     strlen(buffer),
                     0) == -1)
            {
                perror("send");
                break;
            }
        }

        /*
         * Server message.
         */
        if (fds[1].revents &
            (POLLIN | POLLHUP | POLLERR))
        {
            received =
                recv(client_fd,
                     buffer,
                     sizeof(buffer) - 1,
                     0);

            if (received <= 0)
            {
                printf("\nServer disconnected.\n");
                break;
            }

            buffer[received] = '\0';

            printf("\n%s", buffer);
            printf("> ");
            fflush(stdout);
        }
    }

    close(client_fd);

    printf("\nDisconnected from server.\n");

    return EXIT_SUCCESS;
}
