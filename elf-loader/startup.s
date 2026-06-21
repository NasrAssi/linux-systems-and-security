section .text
global startup

startup:
    ; Get entry point function pointer from stack
    mov eax, [esp + 4]
    jmp eax
