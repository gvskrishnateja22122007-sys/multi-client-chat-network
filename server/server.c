#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 8080
#define BACKLOG 10

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    /*
     * 1. Create the server socket
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    printf("Server socket created successfully.\n");

    /*
     * 2. Configure the server address
     */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    /*
     * 3. Bind the socket to the port
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
     * 4. Start listening for clients
     */
    if (listen(server_fd, BACKLOG) == -1)
    {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server is listening...\n");
    printf("Waiting for a client to connect...\n");

    /*
     * 5. Accept one client
     */
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);

    if (client_fd == -1)
    {
        perror("accept");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Client connected!\n");

    /*
     * 6. Display the client's IP address
     */
    printf("Client IP: %s\n", inet_ntoa(client_addr.sin_addr));

    /*
     * 7. Close connections
     */
    close(client_fd);
    close(server_fd);

    printf("Server shut down.\n");

    return EXIT_SUCCESS;
}
