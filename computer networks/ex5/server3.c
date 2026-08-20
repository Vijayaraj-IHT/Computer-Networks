#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001

int sockfd, clientfd;
struct sockaddr_in server, client;

char codeword[105];

void createServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return;
    }
    printf("Server socket created\n");
}

void bindServer()
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        return;
    }
    printf("Server bind successful\n");
}

void waitClient()
{
    listen(sockfd, 5);
    printf("Waiting for client...\n");

    socklen_t len = sizeof(client);
    clientfd = accept(sockfd, (struct sockaddr *)&client, &len);

    if (clientfd < 0)
    {
        printf("Client connection failed\n");
        return;
    }
    printf("Client connected\n");
}

void checkVRC()
{
    memset(codeword, 0, sizeof(codeword));
    recv(clientfd, codeword, sizeof(codeword), 0);

    printf("\nReceived Codeword : %s\n", codeword);

    int count = 0;
    int len = strlen(codeword);

    for (int i = 0; i < len; i++)
    {
        if (codeword[i] == '1')
        {
            count++;
        }
    }

    printf("Total 1s Count    : %d\n", count);

    if (count % 2 == 0)
    {
        printf("Result            : No Error Detected (Even Parity Satisfied)\n");
    }
    else
    {
        printf("Result            : Error Detected (Parity Mismatch)\n");
    }
}

void closeServer()
{
    close(clientfd);
    close(sockfd);
    printf("Connection closed\n");
}

int main()
{
    createServer();
    bindServer();
    waitClient();
    checkVRC();
    closeServer();

    return 0;
}