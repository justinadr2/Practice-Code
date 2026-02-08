#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CALLS 8000

struct Call 
{
    unsigned int offset;
    unsigned long long bytes;
};

int main() 
{
    const char* filename = "D:\\learn\\c\\main.exe"; 
    HANDLE hFile = CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) 
    {
        printf("Failed to open file. Error: %lu\n", GetLastError());
        return 1;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) 
    {
        printf("Failed to get file size. Error: %lu\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    if (fileSize.QuadPart <= 0x400) 
    {
        printf("File too small.\n");
        CloseHandle(hFile);
        return 1;
    }

    // Seek to offset 0x400
    LARGE_INTEGER liOffset;
    liOffset.QuadPart = 0x400;
    if (!SetFilePointerEx(hFile, liOffset, NULL, FILE_BEGIN)) 
    {
        printf("Failed to set file pointer. Error: %lu\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    // Read until EOF
    DWORD bytesToRead = (DWORD)(fileSize.QuadPart - 0x400);
    BYTE* buffer = (BYTE*)malloc(bytesToRead);
    if (!buffer) 
    {
        printf("Failed to allocate memory.\n");
        CloseHandle(hFile);
        return 1;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, bytesToRead, &bytesRead, NULL)) 
    {
        printf("Failed to read file. Error: %lu\n", GetLastError());
        free(buffer);
        CloseHandle(hFile);
        return 1;
    }

    struct Call calls[MAX_CALLS];
    size_t callCount = 0;

    // Scan for CALL rel32 (E8)
    for (DWORD i = 0; i + 4 < bytesRead; i++) 
    {
        if (buffer[i] == 0xE8) 
        {
            if (callCount >= MAX_CALLS)
                break;

            unsigned long long val = 0;

            // Big-endian: E8 xx xx xx xx → 0xE8XXXXXX
            for (int j = 0; j < 5; j++) 
            {
                val <<= 8;
                val |= buffer[i + j];
            }

            calls[callCount].offset = 0x400 + i;
            calls[callCount].bytes  = val;
            callCount++;
        }
    }

    printf("Found %zu CALL instructions.\n", callCount);
    printf("Printing first 100 calls:\n");

    for (size_t k = 0; k < callCount && k < 100; k++) 
    {
        printf("Offset: 0x%X, Bytes: 0x%010llX\n",
               calls[k].offset,
               calls[k].bytes);
    }

    free(buffer);
    CloseHandle(hFile);
    return 0;
}
