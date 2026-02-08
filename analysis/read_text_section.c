#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdint.h>

BOOL GetExecutableBaseAddress(DWORD pid, LPVOID* base)
{
    HANDLE snap = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        pid
    );

    if (snap == INVALID_HANDLE_VALUE)
        return FALSE;

    MODULEENTRY32W me;
    me.dwSize = sizeof(me);

    if (Module32FirstW(snap, &me))
    {
        *base = me.modBaseAddr; // main executable
        CloseHandle(snap);
        return TRUE;
    }

    CloseHandle(snap);
    return FALSE;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("usage: %s <pid>\n", argv[0]);
        return 1;
    }

    DWORD pid = strtoul(argv[1], NULL, 10);

    HANDLE hProcess = OpenProcess(
        PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        FALSE,
        pid
    );

    if (!hProcess)
    {
        printf("OpenProcess failed (%lu)\n", GetLastError());
        return 1;
    }

    LPVOID imageBase = NULL;
    if (!GetExecutableBaseAddress(pid, &imageBase))
    {
        printf("Failed to get executable base\n");
        CloseHandle(hProcess);
        return 1;
    }

    printf("[+] ImageBase: %p\n", imageBase);

    IMAGE_DOS_HEADER dos;
    if (!ReadProcessMemory(hProcess, imageBase, &dos, sizeof(dos), NULL) ||
        dos.e_magic != IMAGE_DOS_SIGNATURE)
    {
        printf("Invalid DOS header\n");
        CloseHandle(hProcess);
        return 1;
    }

#ifdef _WIN64
    IMAGE_NT_HEADERS64 nt;
#else
    IMAGE_NT_HEADERS32 nt;
#endif

    if (!ReadProcessMemory(
            hProcess,
            (BYTE*)imageBase + dos.e_lfanew,
            &nt,
            sizeof(nt),
            NULL) ||
        nt.Signature != IMAGE_NT_SIGNATURE)
    {
        printf("Invalid NT headers\n");
        CloseHandle(hProcess);
        return 1;
    }

    WORD sectionCount = nt.FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* sections =
        malloc(sizeof(IMAGE_SECTION_HEADER) * sectionCount);

    ReadProcessMemory(
        hProcess,
        (BYTE*)imageBase +
            dos.e_lfanew +
            sizeof(DWORD) +
            sizeof(IMAGE_FILE_HEADER) +
            nt.FileHeader.SizeOfOptionalHeader,
        sections,
        sizeof(IMAGE_SECTION_HEADER) * sectionCount,
        NULL
    );

    for (WORD i = 0; i < sectionCount; i++)
    {
        if (memcmp(sections[i].Name, ".text", 5) == 0)
        {
            DWORD size = sections[i].SizeOfRawData;
            DWORD rva  = sections[i].VirtualAddress;
            BYTE* textAddr = (BYTE*)imageBase + rva;

            printf("[+] .text RVA : 0x%08X\n", rva);
            printf("[+] .text Size: 0x%08X bytes\n\n", size);

            BYTE* buffer = malloc(size);
            if (!buffer)
                break;

            if (!ReadProcessMemory(hProcess, textAddr, buffer, size, NULL))
            {
                printf("ReadProcessMemory failed\n");
                free(buffer);
                break;
            }

            uintptr_t base = (uintptr_t)textAddr;

            for (DWORD j = 0; j < size; j++)
            {
                if (j % 16 == 0)
                    printf("%p: ", (void*)(base + j));

                printf("%02X ", buffer[j]);

                if ((j + 1) % 16 == 0)
                    printf("\n");
            }

            if (size % 16 != 0)
                printf("\n");

            free(buffer);
            break;
        }
    }

    free(sections);
    CloseHandle(hProcess);
    return 0;
}
