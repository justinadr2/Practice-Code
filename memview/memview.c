#include <windows.h>
#include <stdio.h>

#define ID_EDIT   1001
#define ID_BUTTON 1002

HANDLE g_proc;
HWND hEdit, hOutput;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        CreateWindowA(
            "STATIC", "Base Address (hex):",
            WS_VISIBLE | WS_CHILD,
            10, 10, 140, 20,
            hwnd, NULL, NULL, NULL
        );

        hEdit = CreateWindowA(
            "EDIT", "0x0",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            160, 10, 300, 22,
            hwnd, (HMENU)ID_EDIT, NULL, NULL
        );

        CreateWindowA(
            "BUTTON", "Read 64 Bytes",
            WS_VISIBLE | WS_CHILD,
            480, 10, 120, 22,
            hwnd, (HMENU)ID_BUTTON, NULL, NULL
        );

        hOutput = CreateWindowA(
            "STATIC", "",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            10, 50, 760, 260,
            hwnd, NULL, NULL, NULL
        );
        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON)
        {
            char addrText[64];
            GetWindowTextA(hEdit, addrText, sizeof(addrText));

            DWORD_PTR base = strtoull(addrText, NULL, 0);

            BYTE buffer[64];
            SIZE_T bytesRead;
            char out[1024] = {0};
            char* p = out;

            if (ReadProcessMemory(g_proc, (LPCVOID)base, buffer, 64, &bytesRead))
            {
                for (int line = 0; line < 4; line++)
                {
                    DWORD_PTR lineAddr = base + (line * 16);
                    p += sprintf(p, "%p: ", (void*)lineAddr);

                    for (int i = 0; i < 16; i++)
                    {
                        p += sprintf(
                            p,
                            "%02X ",
                            buffer[line * 16 + i]
                        );
                    }
                    p += sprintf(p, "\r\n");
                }
            }
            else
            {
                sprintf(out, "ReadProcessMemory failed. Error: %u", GetLastError());
            }

            SetWindowTextA(hOutput, out);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        MessageBoxA(NULL, "Usage: memview.exe <PID>", "Error", MB_ICONERROR);
        return 1;
    }

    DWORD pid = strtoul(argv[1], NULL, 10);

    g_proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!g_proc)
    {
        MessageBoxA(NULL, "Failed to open process", "Error", MB_ICONERROR);
        return 1;
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MemViewer";

    RegisterClassA(&wc);

    CreateWindowA(
        "MemViewer",
        "Memory Viewer – 64 Bytes",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 800, 360,
        NULL, NULL, hInstance, NULL
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(g_proc);
    return 0;
}
