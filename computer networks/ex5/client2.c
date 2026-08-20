#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int sockfd;
struct sockaddr_in server;

char str1[100];
char str2[100];
char result[256];

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
    printf("Enter 1st string: ");
    fgets(str1, sizeof(str1), stdin);
    str1[strcspn(str1, "\n")] = '\0';

    printf("Enter 2nd string: ");
    fgets(str2, sizeof(str2), stdin);
    str2[strcspn(str2, "\n")] = '\0';
}

void sendData()
{
    send(sockfd, str1, sizeof(str1), 0);
    send(sockfd, str2, sizeof(str2), 0);
    printf("Strings sent to server for comparison\n");
}

void receiveResult()
{
    memset(result, 0, sizeof(result));
    recv(sockfd, result, sizeof(result), 0);
    printf("\nServer Response:\n%s\n", result);
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
    sendData();
    receiveResult();
    closeClient();

    return 0;
}