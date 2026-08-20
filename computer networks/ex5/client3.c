#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001

int sockfd;
struct sockaddr_in server;

char data[100];
char codeword[105];
char parity_bit;

void createClient()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return;
    }
    printf("Client socket created\n");
}

void connectServer()
{
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    server.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        return;
    }
    printf("Connected to server\n");
}

void getData()
{
    printf("Enter binary data (e.g. 1011001): ");
    scanf("%99s", data);
}

void makeVRC()
{
    int count = 0;
    int len = strlen(data);

    for (int i = 0; i < len; i++)
    {
        if (data[i] == '1')
        {
            count++;
        }
    }

    if (count % 2 != 0)
    {
        parity_bit = '1';
    }
    else
    {
        parity_bit = '0';
    }

    strcpy(codeword, data);
    codeword[len] = parity_bit;
    codeword[len + 1] = '\0';

    printf("\nOriginal Data       : %s\n", data);
    printf("Number of 1s in Data: %d\n", count);
    printf("VRC Parity Bit      : %c (Even Parity)\n", parity_bit);
    printf("Codeword Generated  : %s\n", codeword);
}

void sendData()
{
    send(sockfd, codeword, strlen(codeword) + 1, 0);
    printf("Codeword sent to server\n");
}

void closeClient()
{
    close(sockfd);
    printf("Connection closed\n");
}

int main()
{
    createClient();
    connectServer();
    getData();
    makeVRC();
    sendData();
    closeClient();

    return 0;
}