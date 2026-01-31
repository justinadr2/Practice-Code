#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

void print_registers(CONTEXT* c)
{
    printf(
        "\nRIP=%016llx RSP=%016llx RBP=%016llx\n"
        "RAX=%016llx RBX=%016llx RCX=%016llx RDX=%016llx\n"
        "RSI=%016llx RDI=%016llx\n"
        "R8 =%016llx R9 =%016llx R10=%016llx R11=%016llx\n"
        "R12=%016llx R13=%016llx R14=%016llx R15=%016llx\n"
        "EFLAGS=%08llx\n",
        c->Rip, c->Rsp, c->Rbp,
        c->Rax, c->Rbx, c->Rcx, c->Rdx,
        c->Rsi, c->Rdi,
        c->R8, c->R9, c->R10, c->R11,
        c->R12, c->R13, c->R14, c->R15,
        c->EFlags
    );
}

void dump_stack(HANDLE hProcess, ULONG64 rsp)
{
    BYTE buf[32];
    SIZE_T read;

    if (ReadProcessMemory(hProcess, (LPCVOID)rsp, buf, sizeof(buf), &read)) 
    {
        printf("Stack [RSP .. RSP+32]:\n");
        for (int i = 0; i < 32; i += 8) {
            printf("  %016llx : %016llx\n",
                rsp + i,
                *(ULONG64*)(buf + i)
            );
        }
    }
}

BOOL is_call_instruction(HANDLE hProcess, ULONG64 rip)
{
    BYTE opcode;
    SIZE_T read;
    ReadProcessMemory(hProcess, (LPCVOID)rip, &opcode, 1, &read);

    // CALL rel32
    return opcode == 0xE8;
}

int main(int argc, char* argv[])
{
    if (argc < 2) 
    {
        printf("Usage: debugger.exe <program> [args]\n");
        return 0;
    }

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    char cmdline[1024] = { 0 };
    strcpy(cmdline, argv[1]);
    for (int i = 2; i < argc; i++) 
    {
        strcat(cmdline, " ");
        strcat(cmdline, argv[i]);
    }

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
        NULL, NULL, &si, &pi))
    {
        printf("CreateProcess failed (%lu)\n", GetLastError());
        return 1;
    }

    DEBUG_EVENT dbg;
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_FULL;

    BYTE saved_byte = 0;
    ULONG64 temp_bp = 0;

    while (1) 
    {
        WaitForDebugEvent(&dbg, INFINITE);

        DWORD cont = DBG_CONTINUE;

        if (dbg.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) 
        {
            DWORD code = dbg.u.Exception.ExceptionRecord.ExceptionCode;

            HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, dbg.dwThreadId);

            GetThreadContext(hThread, &ctx);

            if (code == EXCEPTION_BREAKPOINT) 
            {
                if (temp_bp) 
                {
                    WriteProcessMemory(pi.hProcess, (LPVOID)temp_bp, &saved_byte, 1, NULL);
                    temp_bp = 0;
                }

                print_registers(&ctx);
                dump_stack(pi.hProcess, ctx.Rsp);

                printf("\n[ENTER] Step over\n");
                getchar();

                if (is_call_instruction(pi.hProcess, ctx.Rip)) 
                {
                    // CALL rel32 assumed (5 bytes)
                    temp_bp = ctx.Rip + 5;

                    ReadProcessMemory(pi.hProcess, (LPCVOID)temp_bp, &saved_byte, 1, NULL);

                    BYTE int3 = 0xCC;
                    WriteProcessMemory(pi.hProcess, (LPVOID)temp_bp, &int3, 1, NULL);
                }
                else 
                {
                    ctx.EFlags |= 0x100;
                    SetThreadContext(hThread, &ctx);
                }
            }
            else if (code == EXCEPTION_SINGLE_STEP) 
            {
                print_registers(&ctx);
                dump_stack(pi.hProcess, ctx.Rsp);

                printf("\n[ENTER] Step over\n");
                getchar();

                ctx.EFlags |= 0x100;
                SetThreadContext(hThread, &ctx);
            }

            CloseHandle(hThread);
        }
        else if (dbg.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) 
        {
            printf("\nProcess exited.\n");
            break;
        }

        ContinueDebugEvent(dbg.dwProcessId, dbg.dwThreadId, cont);
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
