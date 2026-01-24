#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT "8888"
#define BUFFER_SIZE 512

int main() 
{
    WSADATA wsa;
    SOCKET listen_sock = INVALID_SOCKET;
    SOCKET clients[2] = { INVALID_SOCKET, INVALID_SOCKET };
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE];
    int recv_size;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        printf("getaddrinfo failed\n");
        WSACleanup();
        return 1;
    }

    listen_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listen_sock == INVALID_SOCKET) {
        printf("socket failed\n");
        freeaddrinfo(res);
        WSACleanup();
        return 1;
    }

    if (bind(listen_sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR)
    {
        printf("bind failed\n");
        closesocket(listen_sock);
        freeaddrinfo(res);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(res);

    if (listen(listen_sock, 2) == SOCKET_ERROR) 
    {
        printf("listen failed\n");
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %s\n", PORT);

    for (int i = 0; i < 2; i++) {
        clients[i] = accept(listen_sock, NULL, NULL);
        if (clients[i] == INVALID_SOCKET) {
            printf("accept failed\n");
            closesocket(listen_sock);
            WSACleanup();
            return 1;
        }
        printf("Client %d connected\n", i+1);
    }

    printf("Both clients connected! Relaying messages...\n");

    // Relay loop
    while (1) {
        for (int i = 0; i < 2; i++) 
        {
            recv_size = recv(clients[i], buffer, BUFFER_SIZE-1, 0);
            if (recv_size <= 0) {
                printf("Client %d disconnected\n", i+1);
                goto cleanup;
            }
            buffer[recv_size] = '\0';
            send(clients[1-i], buffer, recv_size, 0);
        }
    }

cleanup:
    closesocket(clients[0]);
    closesocket(clients[1]);
    closesocket(listen_sock);
    WSACleanup();
    return 0;
}
