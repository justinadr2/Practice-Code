extern  printf              : proc
extern  scanf               : proc
extern  strtoul             : proc
extern  OpenProcess         : proc
extern  WriteProcessMemory  : proc
extern  GetLastError        : proc
extern  CloseHandle         : proc
extern  ExitProcess         : proc

.data
fmt_addr    db "Enter address: ",0
fmt_scan1   db "%Ix",0
fmt_val     db "Enter 8-byte value: 0x",0
fmt_scan2   db "%llx",0
fmt_write   db "Wrote %zu bytes",10,0
fmt_openerr db "OpenProcess failed (%lu)",10,0
fmt_wpmerr  db "WriteProcessMemory failed (%lu)",10,0

.code
main PROC
    sub rsp, 80h                ; shadow + locals + alignment

    ; if (argc < 2) return 1;
    cmp rcx, 2
    jl  exit_fail

    ; pid = strtoul(argv[1], NULL, 10)
    mov rcx, [rdx+8]            ; argv[1]
    xor rdx, rdx                ; NULL
    mov r8d, 10
    call strtoul
    mov dword ptr [rsp+20h], eax    ; pid (DWORD)

    ; OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid)
    mov ecx, 0020h | 0008h      ; desired access
    xor edx, edx                ; FALSE
    mov r8d, [rsp+20h]          ; pid
    call OpenProcess
    mov [rsp+28h], rax          ; HANDLE proc

loop_start:

    ; printf("Enter address: ");
    lea rcx, fmt_addr
    call printf

    ; scanf("%Ix", &addr)
    lea rcx, fmt_scan1
    lea rdx, [rsp+30h]          ; uintptr_t addr
    call scanf

    ; printf("Enter 8-byte value: 0x");
    lea rcx, fmt_val
    call printf

    ; scanf("%llx", &value)
    lea rcx, fmt_scan2
    lea rdx, [rsp+38h]          ; uint64_t value
    call scanf

    ; if (!proc)
    mov rax, [rsp+28h]
    test rax, rax
    jnz proc_ok

    call GetLastError
    mov rdx, rax
    lea rcx, fmt_openerr
    call printf
    jmp exit_fail

proc_ok:

    ; WriteProcessMemory(proc, addr, &value, 8, &written)
    mov rcx, [rsp+28h]          ; proc
    mov rdx, [rsp+30h]          ; addr
    lea r8,  [rsp+38h]          ; &value
    mov r9d, 8                  ; sizeof(uint64_t)
    lea rax, [rsp+40h]          ; written
    mov [rsp+20h], rax          ; 5th arg (stack)
    call WriteProcessMemory

    test eax, eax
    jnz write_ok

    call GetLastError
    mov rdx, rax
    lea rcx, fmt_wpmerr
    call printf
    mov rcx, [rsp+28h]
    call CloseHandle
    jmp exit_fail

write_ok:

    lea rcx, fmt_write
    mov rdx, [rsp+40h]
    call printf

    jmp loop_start

exit_fail:
    mov ecx, 1
    call ExitProcess

main ENDP
END
