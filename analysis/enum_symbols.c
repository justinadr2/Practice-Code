// enumerate all import and exported symbols of a process

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_NAME 512

BOOL read_mem(HANDLE hProc, LPCVOID addr, LPVOID buf, SIZE_T size)
{
    SIZE_T br;
    return ReadProcessMemory(hProc, addr, buf, size, &br) && br == size;
}

void print_exports(HANDLE hProc, uintptr_t base)
{
    IMAGE_DOS_HEADER dos;
    IMAGE_NT_HEADERS64 nt;

    if (!read_mem(hProc, (void*)base, &dos, sizeof(dos)) ||
        dos.e_magic != IMAGE_DOS_SIGNATURE)
        return;

    if (!read_mem(hProc, (void*)(base + dos.e_lfanew), &nt, sizeof(nt)) ||
        nt.Signature != IMAGE_NT_SIGNATURE)
        return;

    IMAGE_DATA_DIRECTORY dir =
        nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    if (!dir.VirtualAddress)
        return;

    IMAGE_EXPORT_DIRECTORY exp;
    if (!read_mem(hProc, (void*)(base + dir.VirtualAddress),
        &exp, sizeof(exp)))
        return;

    printf("  Exports:\n");

    DWORD* names = malloc(exp.NumberOfNames * sizeof(DWORD));
    WORD* ords  = malloc(exp.NumberOfNames * sizeof(WORD));

    if (!names || !ords)
        goto cleanup;

    read_mem(hProc, (void*)(base + exp.AddressOfNames),
        names, exp.NumberOfNames * sizeof(DWORD));

    read_mem(hProc, (void*)(base + exp.AddressOfNameOrdinals),
        ords, exp.NumberOfNames * sizeof(WORD));

    for (DWORD i = 0; i < exp.NumberOfNames; i++)
    {
        char name[MAX_NAME] = {0};
        read_mem(hProc, (void*)(base + names[i]), name, sizeof(name)-1);
        printf("    %s (ord %u)\n", name, exp.Base + ords[i]);
    }

cleanup:
    free(names);
    free(ords);
}

void print_imports(HANDLE hProc, uintptr_t base)
{
    IMAGE_DOS_HEADER dos;
    IMAGE_NT_HEADERS64 nt;

    if (!read_mem(hProc, (void*)base, &dos, sizeof(dos)) ||
        dos.e_magic != IMAGE_DOS_SIGNATURE)
        return;

    if (!read_mem(hProc, (void*)(base + dos.e_lfanew), &nt, sizeof(nt)) ||
        nt.Signature != IMAGE_NT_SIGNATURE)
        return;

    IMAGE_DATA_DIRECTORY dir =
        nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (!dir.VirtualAddress)
        return;

    printf("  Imports:\n");

    IMAGE_IMPORT_DESCRIPTOR imp;
    uintptr_t imp_addr = base + dir.VirtualAddress;

    while (1)
    {
        if (!read_mem(hProc, (void*)imp_addr, &imp, sizeof(imp)))
            break;

        if (!imp.Name)
            break;

        char dll[MAX_NAME] = {0};
        read_mem(hProc, (void*)(base + imp.Name), dll, sizeof(dll)-1);
        printf("    [%s]\n", dll);

        uintptr_t thunk =
            imp.OriginalFirstThunk ? imp.OriginalFirstThunk : imp.FirstThunk;

        uintptr_t thunk_addr = base + thunk;

        IMAGE_THUNK_DATA64 t;
        while (1)
        {
            if (!read_mem(hProc, (void*)thunk_addr, &t, sizeof(t)))
                break;

            if (!t.u1.AddressOfData)
                break;

            if (t.u1.Ordinal & IMAGE_ORDINAL_FLAG64)
            {
                printf("      Ordinal: %llu\n",
                    IMAGE_ORDINAL64(t.u1.Ordinal));
            }
            else
            {
                IMAGE_IMPORT_BY_NAME ibn;
                read_mem(hProc,
                    (void*)(base + t.u1.AddressOfData),
                    &ibn, sizeof(ibn));

                char fname[MAX_NAME] = {0};
                read_mem(hProc,
                    (void*)(base + t.u1.AddressOfData + 2),
                    fname, sizeof(fname)-1);

                printf("      %s\n", fname);
            }

            thunk_addr += sizeof(IMAGE_THUNK_DATA64);
        }

        imp_addr += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    }
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: %s <pid>\n", argv[0]);
        return 1;
    }

    DWORD pid = atoi(argv[1]);
    HANDLE hProc = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, pid);

    if (!hProc)
    {
        printf("[-] OpenProcess failed (%lu)\n", GetLastError());
        return 1;
    }

    HANDLE snap = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

    if (snap == INVALID_HANDLE_VALUE)
    {
        printf("[-] Snapshot failed\n");
        CloseHandle(hProc);
        return 1;
    }

    MODULEENTRY32 me;
    me.dwSize = sizeof(me);

    if (!Module32First(snap, &me))
    {
        printf("[-] Module32First failed\n");
        goto cleanup;
    }

    do
    {
        printf("\nModule: %ws\n", me.szModule);
        printf("Base:   %p\n", me.modBaseAddr);

        print_exports(hProc, (uintptr_t)me.modBaseAddr);
        print_imports(hProc, (uintptr_t)me.modBaseAddr);

    } while (Module32Next(snap, &me));

cleanup:
    CloseHandle(snap);
    CloseHandle(hProc);
    return 0;
}
