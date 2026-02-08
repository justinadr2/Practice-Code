#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdint.h>

void EnumerateModuleSections(
    HANDLE hProcess,
    LPCVOID moduleBase,
    const wchar_t* modulePath
)
{
    IMAGE_DOS_HEADER dos = { 0 };
    IMAGE_NT_HEADERS64 nt = { 0 };

    if (!ReadProcessMemory(hProcess, moduleBase, &dos, sizeof(dos), NULL))
        return;

    if (dos.e_magic != IMAGE_DOS_SIGNATURE)
        return;

    if (!ReadProcessMemory(
        hProcess,
        (LPCBYTE)moduleBase + dos.e_lfanew,
        &nt,
        sizeof(nt),
        NULL))
        return;

    if (nt.Signature != IMAGE_NT_SIGNATURE)
        return;

    wprintf(L"\nModule: %s\n", modulePath);
    wprintf(L"  Base: %016llX\n", (unsigned long long)moduleBase);
    wprintf(L"  Sections:\n");

    WORD sectionCount = nt.FileHeader.NumberOfSections;

    IMAGE_SECTION_HEADER section = { 0 };

    LPCBYTE sectionBase =
        (LPCBYTE)moduleBase +
        dos.e_lfanew +
        sizeof(DWORD) +
        sizeof(IMAGE_FILE_HEADER) +
        nt.FileHeader.SizeOfOptionalHeader;

    for (WORD i = 0; i < sectionCount; i++)
    {
        if (!ReadProcessMemory(
            hProcess,
            sectionBase + i * sizeof(IMAGE_SECTION_HEADER),
            &section,
            sizeof(section),
            NULL))
            continue;

        printf(
            "    [%-8.8s] Base: %016llX Size: %08X\n",
            section.Name,
            (unsigned long long)((uintptr_t)moduleBase + section.VirtualAddress),
            section.Misc.VirtualSize
        );
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <pid>\n", argv[0]);
        return 0;
    }

    DWORD pid = strtoul(argv[1], NULL, 10);

    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        pid
    );

    if (!hProcess)
    {
        printf("OpenProcess failed (%lu)\n", GetLastError());
        return 1;
    }

    HANDLE snap = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        pid
    );

    if (snap == INVALID_HANDLE_VALUE)
    {
        printf("CreateToolhelp32Snapshot failed (%lu)\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    MODULEENTRY32W me = { 0 };
    me.dwSize = sizeof(me);

    if (Module32FirstW(snap, &me))
    {
        do
        {
            EnumerateModuleSections(
                hProcess,
                me.modBaseAddr,
                me.szExePath   // FULL MODULE PATH
            );
        } while (Module32NextW(snap, &me));
    }
    else
    {
        printf("Module32FirstW failed (%lu)\n", GetLastError());
    }

    CloseHandle(snap);
    CloseHandle(hProcess);
    return 0;
}
