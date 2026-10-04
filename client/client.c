#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main(void)
{
    int client_fd;
    struct sockaddr_in server_addr;

    /*
     * 1. Create client socket
     */
    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    printf("Client socket created successfully.\n");

    /*
     * 2. Configure server address
     */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(client_fd);
        return EXIT_FAILURE;
    }

    /*
     * 3. Connect to server
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

    printf("Connected to server successfully!\n");

    /*
     * 4. Close connection
     */
    close(client_fd);

    printf("Client disconnected.\n");

    return EXIT_SUCCESS;
}
