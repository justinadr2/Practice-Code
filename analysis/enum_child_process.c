#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <winternl.h>

typedef NTSTATUS (NTAPI *pNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
);

void PrintChildArguments(DWORD processID) 
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    pNtQueryInformationProcess NtQueryInfo = (pNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");

    PROCESS_BASIC_INFORMATION pbi;
    ULONG retLen;
    if (NtQueryInfo(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &retLen) == 0) 
    {
        PEB peb;
        RTL_USER_PROCESS_PARAMETERS params;

        // 1. Read the PEB from the child process
        if (ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), NULL)) 
        {
            // 2. Read the ProcessParameters from the PEB
            if (ReadProcessMemory(hProcess, peb.ProcessParameters, &params, sizeof(params), NULL)) 
            {
                // 3. Read the actual Command-Line Unicode string
                PWSTR cmdLineContents = (PWSTR)malloc(params.CommandLine.Length + sizeof(WCHAR));
                if (ReadProcessMemory(hProcess, params.CommandLine.Buffer, cmdLineContents, params.CommandLine.Length, NULL)) 
                {
                    cmdLineContents[params.CommandLine.Length / sizeof(WCHAR)] = L'\0';
                    wprintf(L"  [Args]: %s\n", cmdLineContents);
                }
                free(cmdLineContents);
            }
        }
    }
    CloseHandle(hProcess);
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) 
    { 
        printf("Usage: main <PID>\n"); 
        return 1; 
    }

    DWORD parentPID = (DWORD)atoi(argv[1]);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) 
    {
        do 
        {
            if (pe.th32ParentProcessID == parentPID) 
            {
                printf("\nFound Child: %s (PID: %lu)\n", pe.szExeFile, pe.th32ProcessID);
                PrintChildArguments(pe.th32ProcessID);
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return 0;
}