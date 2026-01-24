#include <windows.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char* argv[]) 
{
    if (argc < 2)
        return 1;   

    DWORD pid = (DWORD)strtoul(argv[1], NULL, 10);
    uint64_t value;
    uintptr_t addr;

    HANDLE proc = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    
    while (1)
    {
        printf("Enter address: ");
        scanf("%Ix", &addr);

        printf("Enter 8-byte value: 0x");
        scanf("%llx", (unsigned long long*)&value);

        if (!proc) 
        {
            printf("OpenProcess failed (%lu)\n", GetLastError());
            return 1;
        }

        SIZE_T written;
        if (!WriteProcessMemory(proc, (LPVOID)addr, &value, sizeof(value), &written)) 
        {
            printf("WriteProcessMemory failed (%lu)\n", GetLastError());
            CloseHandle(proc);
            return 1;
        }
        printf("Wrote %zu bytes\n", written);    
    }
    CloseHandle(proc);
    return 0;
}
