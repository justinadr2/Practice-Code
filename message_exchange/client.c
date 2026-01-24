#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT "8888"
#define BUFFER_SIZE 512

HWND hwndEditLog, hwndEditInput, hwndButtonSend;
SOCKET sock;

void append_text(const char *text) 
{
    int len = GetWindowTextLength(hwndEditLog);
    SendMessage(hwndEditLog, EM_SETSEL, len, len);
    SendMessage(hwndEditLog, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(hwndEditLog, EM_SCROLLCARET, 0, 0);
}

DWORD WINAPI recv_thread(LPVOID param) 
{
    char buffer[BUFFER_SIZE];
    int recv_size;

    while ((recv_size = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) 
    {
        buffer[recv_size] = '\0';
        strcat(buffer, "\r\n");
        append_text(buffer);
    }

    append_text("[Disconnected from server]\r\n");
    return 0;
}

void send_message()
{
    char buffer[BUFFER_SIZE];
    GetWindowText(hwndEditInput, buffer, BUFFER_SIZE);
    if (strlen(buffer) == 0) return;

    send(sock, buffer, (int)strlen(buffer), 0);

    char logline[BUFFER_SIZE + 16];
    sprintf(logline, "Me: %s\r\n", buffer);
    append_text(logline);

    SetWindowText(hwndEditInput, ""); 
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_COMMAND:
            if ((HWND)lParam == hwndButtonSend) {
                send_message();
            }
            break;
        case WM_DESTROY:
            closesocket(sock);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) 
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) 
    {
        MessageBox(NULL, "WSAStartup failed", "Error", MB_OK);
        return 1;
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "ChatClient";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("ChatClient", "Chat Client",  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        400, 400, NULL, NULL, hInst, NULL);
    ShowWindow(hwnd, nCmdShow);

    hwndEditLog = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        10, 10, 360, 250, hwnd, NULL, hInst, NULL);

    hwndEditInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        10, 270, 260, 25, hwnd, NULL, hInst, NULL);

    hwndButtonSend = CreateWindow("BUTTON", "Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        280, 270, 90, 25, hwnd, NULL, hInst, NULL);

    struct addrinfo hints, *res;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(SERVER_IP, PORT, &hints, &res) != 0) {
        MessageBox(NULL, "getaddrinfo failed", "Error", MB_OK);
        return 1;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        MessageBox(NULL, "Socket creation failed", "Error", MB_OK);
        freeaddrinfo(res);
        return 1;
    }

    if (connect(sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) 
    {
        MessageBox(NULL, "Connect failed", "Error", MB_OK);
        closesocket(sock);
        freeaddrinfo(res);
        return 1;
    }

    freeaddrinfo(res);

    append_text("[Connected to server]\r\n");

    CreateThread(NULL, 0, recv_thread, NULL, 0, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WSACleanup();
    return 0;
}
