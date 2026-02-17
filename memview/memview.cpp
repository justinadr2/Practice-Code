#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <array>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

constexpr int ID_EDIT   = 1001;
constexpr int ID_BUTTON = 1002;

HANDLE gProcess = nullptr;
HWND   gEdit    = nullptr;
HWND   gOutput  = nullptr;

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
            hwnd, nullptr, nullptr, nullptr
        );

        gEdit = CreateWindowA(
            "EDIT", "0x0",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            160, 10, 300, 22,
            hwnd, reinterpret_cast<HMENU>(ID_EDIT), nullptr, nullptr
        );

        CreateWindowA(
            "BUTTON", "Read 64 Bytes",
            WS_VISIBLE | WS_CHILD,
            480, 10, 120, 22,
            hwnd, reinterpret_cast<HMENU>(ID_BUTTON), nullptr, nullptr
        );

        gOutput = CreateWindowA(
            "STATIC", "",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            10, 50, 760, 260,
            hwnd, nullptr, nullptr, nullptr
        );
        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON)
        {
            char text[64]{};
            GetWindowTextA(gEdit, text, sizeof(text));

            uintptr_t base =
                static_cast<uintptr_t>(std::strtoull(text, nullptr, 0));

            std::array<std::uint8_t, 64> buffer{};
            SIZE_T bytesRead = 0;

            std::ostringstream out;

            if (ReadProcessMemory(gProcess,reinterpret_cast<LPCVOID>(base),
                buffer.data(), buffer.size(), &bytesRead))
            {
                for (int line = 0; line < 4; ++line)
                {
                    uintptr_t lineAddr = base + line * 16;
                    out << reinterpret_cast<void*>(lineAddr) << ": ";

                    for (int i = 0; i < 16; ++i)
                    {
                        out << std::hex << std::uppercase
                            << std::setw(2) << std::setfill('0')
                            << static_cast<int>(buffer[line * 16 + i])
                            << ' ';
                    }
                    out << "\r\n";
                }
            }
            else
            {
                out << "ReadProcessMemory failed. Error: " << GetLastError();
            }

            std::string result = out.str();
            SetWindowTextA(gOutput, result.c_str());
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

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR cmdLine,
    int)
{
    if (!cmdLine || !*cmdLine)
    {
        MessageBoxA(nullptr,
            "Usage: memview.exe <PID>",
            "Error",
            MB_ICONERROR);
        return 1;
    }

    DWORD pid = std::strtoul(cmdLine, nullptr, 10);

    gProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (!gProcess)
    {
        MessageBoxA(nullptr,
            "Failed to open process",
            "Error",
            MB_ICONERROR);
        return 1;
    }

    WNDCLASSA wc{};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "MemViewerCPP";

    RegisterClassA(&wc);

    CreateWindowA(
        wc.lpszClassName,
        "Memory Viewer – 64 Bytes (C++)",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 800, 360,
        nullptr, nullptr, hInstance, nullptr
    );

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(gProcess);
    return 0;
}
