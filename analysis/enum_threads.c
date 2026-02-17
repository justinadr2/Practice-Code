// enumerate current threads of a process

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <PID>\n", argv[0]);
        return 0;
    }

    DWORD pid = strtoul(argv[1], NULL, 10);

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE)
    {
        printf("Failed to create snapshot (%lu)\n", GetLastError());
        return 1;
    }

    THREADENTRY32 te = { 0 };
    te.dwSize = sizeof(te);

    if (!Thread32First(snap, &te))
    {
        printf("Thread32First failed (%lu)\n", GetLastError());
        CloseHandle(snap);
        return 1;
    }

    printf("Threads in process %lu:\n", pid);

    do
    {
        if (te.th32OwnerProcessID == pid)
        {
            printf("\nThread ID: %lu\n", te.th32ThreadID);

            HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            if (hThread)
            {
                SuspendThread(hThread);

#ifdef _WIN64
                CONTEXT ctx = { 0 };
                ctx.ContextFlags = CONTEXT_ALL;
#else
                CONTEXT ctx = { 0 };
                ctx.ContextFlags = CONTEXT_FULL;
#endif

                if (GetThreadContext(hThread, &ctx))
                {
#ifdef _WIN64
                    printf("RAX=%llX \nRBX=%llX \nRCX=%llX \nRDX=%llX\n", ctx.Rax, ctx.Rbx, ctx.Rcx, ctx.Rdx);
                    printf("RIP=%llX \nRSP=%llX \nRBP=%llX\n", ctx.Rip, ctx.Rsp, ctx.Rbp);
#else
                    printf("    EAX=%X EBX=%X ECX=%X EDX=%X\n", ctx.Eax, ctx.Ebx, ctx.Ecx, ctx.Edx);
                    printf("    EIP=%X ESP=%X EBP=%X\n", ctx.Eip, ctx.Esp, ctx.Ebp);
#endif
                }
                else
                {
                    printf("    Failed to get context (%lu)\n", GetLastError());
                }

                ResumeThread(hThread);
                CloseHandle(hThread);
            }
            else
            {
                printf("    Failed to open thread (%lu)\n", GetLastError());
            }
        }

    } while (Thread32Next(snap, &te));

    CloseHandle(snap);
    return 0;
}
