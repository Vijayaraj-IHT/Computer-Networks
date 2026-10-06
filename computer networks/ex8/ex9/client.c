#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main()
{
    int client_socket;

    struct sockaddr_in server_address;

    char buffer[BUFFER_SIZE];

    /* Create TCP socket */

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    /* Server address */

    server_address.sin_family = AF_INET;

    server_address.sin_port = htons(PORT);

    server_address.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    /* Connect to server */

    if (connect(client_socket,
                (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        exit(1);
    }

    printf("Connected to RARP Server.\n");

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        printf("\nEnter MAC Address (or 'exit'): ");

        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\r\n")] = 0;

        /* Send MAC address to server */

        send(client_socket,
             buffer,
             strlen(buffer),
             0);

        /* Exit condition */

        if (strcmp(buffer, "exit") == 0 ||
            strcmp(buffer, "bye") == 0)
        {
            break;
        }

        /* Receive IP address or error */

        memset(buffer, 0, BUFFER_SIZE);

        int bytes_read =
            recv(client_socket,
                 buffer,
                 BUFFER_SIZE - 1,
                 0);

        if (bytes_read <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        buffer[bytes_read] = '\0';

        printf("RARP Result: %s\n", buffer);
    }

    close(client_socket);

    return 0;
}