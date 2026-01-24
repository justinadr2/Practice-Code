#include <stdio.h>
#include <winsock2.h>

int main() 
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    char ip[64];
    int port;

    printf("Enter IP address (e.g., 127.0.0.1): ");
    scanf("%63s", ip);

    printf("Enter port: ");
    scanf("%d", &port);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) 
    {
        printf("Failed to create socket.\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    printf("Checking %s:%d ...\n", ip, port);

    int result = connect(s, (struct sockaddr*)&addr, sizeof(addr));

    if (result == 0)
        printf("Port %d is OPEN.\n", port);
    else
        printf("Port %d is CLOSED. Error: %d\n", port, WSAGetLastError());

    closesocket(s);
    WSACleanup();
    return 0;
}
