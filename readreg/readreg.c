#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

void PrintRegisters(CONTEXT* ctx) 
{
    printf("RAX = 0x%llX\n", ctx->Rax);
    printf("RBX = 0x%llX\n", ctx->Rbx);
    printf("RCX = 0x%llX\n", ctx->Rcx);
    printf("RDX = 0x%llX\n", ctx->Rdx);
    printf("RSI = 0x%llX\n", ctx->Rsi);
    printf("RDI = 0x%llX\n", ctx->Rdi);
    printf("RSP = 0x%llX\n", ctx->Rsp);
    printf("RBP = 0x%llX\n", ctx->Rbp);
    printf("RIP = 0x%llX\n", ctx->Rip);
    printf("R8  = 0x%llX\n", ctx->R8);
    printf("R9  = 0x%llX\n", ctx->R9);
    printf("R10 = 0x%llX\n", ctx->R10);
    printf("R11 = 0x%llX\n", ctx->R11);
    printf("R12 = 0x%llX\n", ctx->R12);
    printf("R13 = 0x%llX\n", ctx->R13);
    printf("R14 = 0x%llX\n", ctx->R14);
    printf("R15 = 0x%llX\n", ctx->R15);
    printf("EFlags = 0x%llX\n", ctx->EFlags);
}


int main(int argc, char* argv[]) 
{
    if (argc != 2) {
        printf("Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    DWORD pid = atoi(argv[1]);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) 
    {
        printf("Failed to open process. Error: %lu\n", GetLastError());
        return 1;
    }

    // enumerate threads in the process
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) 
    {
        printf("Failed to create thread snapshot.\n");
        CloseHandle(hProcess);
        return 1;
    }

    THREADENTRY32 te;
    te.dwSize = sizeof(te);

    if (Thread32First(hSnapshot, &te)) 
    {
        do 
        {
            if (te.th32OwnerProcessID == pid) 
            {
                HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                if (hThread) {
                    CONTEXT ctx;
                    ctx.ContextFlags = CONTEXT_ALL;

                    SuspendThread(hThread); // pause the thread

                    if (GetThreadContext(hThread, &ctx)) 
                    {
                        printf("Thread ID: %lu\n", te.th32ThreadID);
                        PrintRegisters(&ctx);
                        printf("-------------------------\n");
                    }

                    ResumeThread(hThread);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
    CloseHandle(hProcess);
    return 0;
}
