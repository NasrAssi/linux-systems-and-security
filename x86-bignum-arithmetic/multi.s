section .data
    fmt_argc: db "%d", 10, 0       ; "%d\n" for printing argc
    STATE dw 0xACE1        ; initial seed, must not be 0
    MASK  dw 0xB400        ; standard 16-bit LFSR tap mask
    fmt_invalid_arg db "Invalid argument", 10, 0
    fmt_len db "%d", 10, 0       ; Format for printing length + newline

   x_struct: db 5, 0xaa, 1, 2, 0x44, 0x4f
   y_struct: db 6, 0xaa, 1, 2, 3, 0x44, 0x4f




section .bss
    input_buf resb 600
    result_ptr resd 1         ; reserve 4 bytes to store the result pointer
    byte_count  resd 1
    SHEROT  resd 1
    result_ptr1 resd 1 
    result_ptr2 resd 1 
    remaining_bytes resd 1   ;  remaining_bytes



section .rodata
    fmt_hex: db "%02hhx", 0
    newline: db 10, 0
    prompt_str: db "Enter hex digits (no spaces):", 0
    fmt_oom: db "Out of memory", 10, 0


section .text
    global main
    extern printf, puts,malloc,fgets,stdin,exit




main:
    push ebp
    mov ebp, esp

    ; argc = [ebp + 8], argv = [ebp + 12]
    mov eax, [ebp + 8]      ; argc
    mov esi, [ebp + 12]     ; argv

    cmp eax, 1
    je .use_static          ; if no arguments → default

    ; argv[1] = [esi + 4]
    mov ebx, [esi + 4]
    cmp word [ebx], '-i'
    je .input_mode
    cmp word [ebx], '-r'
    je .random_mode
    jmp .invalid

.use_static:
    mov eax, x_struct       ; first number
    mov ebx, y_struct       ; second number
    jmp .compute

.input_mode:
    ; print prompt for first
    pushad
    push prompt_str
    call puts
    add esp, 4

    popad
    push input_buf
    call getmulti
    add esp, 4
    mov eax, eax            ; first number
    mov [result_ptr2], eax

    pushad
    ; prompt second
    push prompt_str
    call puts
    add esp, 4

    popad
    push input_buf
    call getmulti
    add esp, 4
    mov ebx, eax            ; second number
    mov eax, [result_ptr2]   ; restore first
    jmp .compute

.random_mode:
    call PRmulti
    mov [result_ptr1], eax
    call PRmulti
    mov ebx, eax
    mov eax, [result_ptr1]
    jmp .compute

.compute:
    ; Save inputs to stack
    push eax    ; first number
    push ebx    ; second number

    ; Print first number
    push eax
    call print_multi
    add esp, 4
    push newline
    call printf
    add esp, 4

    ; Print second number
    push ebx
    call print_multi
    add esp, 4
    push newline
    call printf
    add esp, 4

    ; Restore input pointers
    pop ebx     ; second number
    pop eax     ; first number

    ; Now compute and print sum
    call add_multi
    push eax
    call print_multi
    add esp, 4
    push newline
    call printf
    add esp, 4

    jmp .exit




.invalid:
    push fmt_invalid_arg
    call puts
    add esp, 4

.exit:
    mov eax, 0
    mov esp, ebp
    pop ebp
    ret





; -------- part 1.A [print_multi] ---------- checked and working good 

print_multi:

    push ebp 
    mov ebp, esp
    push ecx
    push esi
    push edi

    

    mov esi, [ebp+8]               ; ESI = pointer to struct multi
    movzx ecx, byte [esi]          ; ECX = size (zero-extended from byte)
    test ecx, ecx
    jz .done                       ; exit if size = 0 (safety)

    mov edi, esi                   ; EDI = start of struct
    add edi, 1                     ; EDI now points to num[0] (first byte)

.next_byte:
    dec ecx                        ; index = size - 1 ... 0
    js .done                  ; signed check: if ecx < 0, we’re done

    pushad                         ; save all regs before printf
    movzx eax, byte [edi + ecx]    ; load num[i] into eax, zero-extend
    push eax                       ; push value to be printed
    push fmt_hex                   ; push format string
    call printf
    add esp, 8                     ; clean up printf args
    popad                          ; restore registers
    jmp .next_byte


.done:

    pop edi
    pop esi
    pop ecx
    mov esp, ebp
    pop ebp
    ret




; -------- part 1.B [get_multi] ----------


getmulti:

    push ebp
    mov  ebp, esp
    pushad

    ; ---------------- Read input line ----------------
    push dword [stdin]              ; stdin
    push dword 600
    push dword input_buf
    call fgets
    add esp, 12

    ; ---------------- Count hex digits ----------------
    xor ecx, ecx              ; ECX = hex char count
.count_loop:
    mov al, [input_buf + ecx]
    cmp al, 0
    je .count_done
    cmp al, 10                ; newline?
    je .count_done
    inc ecx
    jmp .count_loop

.count_done:
    mov edx, ecx              ; EDX = hex char count

    ; if count is odd → make it even
    test dl, 1
    jz .even_count

    ; ecx = index of last character (edx = char count)
    mov ecx, edx
    dec ecx            ; last char index

.shift_right:
    mov al, [input_buf + ecx]
    mov [input_buf + ecx + 1], al
    dec ecx
    jns .shift_right   ; while ecx >= 0

mov byte [input_buf], '0'  ; insert '0' at the beginning
inc edx                    ; new even length


.even_count:
    shr edx, 1                        ; now edx = byte count
              
    ; ----------- Allocate memory: size + bytes -----------
    mov [byte_count], edx     ; save byte count to global
    mov eax, edx
    inc eax                   ; total size = 1 + byte count
    push eax
    call malloc
    add esp, 4
    test eax, eax
    jz malloc_fail
    mov esi, eax             ; ESI = pointer to allocated struct
    mov edx, [byte_count]     ; restore byte count for use in .convert_loop

    ; ----------- Fill size field -----------
    mov [esi], dl            ; first byte = size
    lea edi, [esi + 1]       ; EDI points to num[0]

    ; ----------- Convert hex to bytes -----------
    xor ecx, ecx             ; input index
    xor ebx, ebx             ; output index

.convert_loop:
    test edx, edx           ; stop when no bytes remain (handles empty input)
    jz .convert_done

    ; Load first hex char → AL
    mov al, [input_buf + ecx]
    call hexchar_to_nibble
    shl al, 4                ; move to high nibble
    mov bl, al              ; save high nibble in BL

    ; Load second hex char → AL
    mov al, [input_buf + ecx + 1]
    call hexchar_to_nibble  ; now AL = low nibble

    ; Combine nibbles: BL (high) | AL (low)
    or al, bl               ; AL = full byte (e.g., DE)

    ; Store result in reverse order
    mov [edi + edx - 1], al

    dec edx                 ; next byte
    add ecx, 2              ; next input pair
    jmp .convert_loop

.convert_done:
    ; ----------- Return result -----------
    mov [result_ptr], esi         ; save result to global
    popad
    mov esp, ebp
    pop ebp
    mov eax, [result_ptr]         ; restore result to return it
    ret

; -----------------------------------------------------
; hexchar_to_nibble: converts hex char in AL to value 0-15
; returns result in AL
; -----------------------------------------------------
hexchar_to_nibble:
    cmp al, '0'
    jl .bad
    cmp al, '9'
    jle .ok_num
    cmp al, 'A'
    jl .lower
    cmp al, 'F'
    jle .upper_ok
    cmp al, 'a'
    jl .bad
    cmp al, 'f'
    jg .bad
.lower:
    sub al, 'a'
    add al, 10
    ret
.upper_ok:
    sub al, 'A'
    add al, 10
    ret
.ok_num:
    sub al, '0'
    ret
.bad:
    push fmt_invalid_arg
    call puts
    add esp, 4
    push dword 1
    call exit



; -------- part 2.A [MaxMin — ensure larger struct in eax] ----------
; Input:
;   eax = pointer to struct A
;   ebx = pointer to struct B
; Output:
;   eax = pointer to longer struct
;   ebx = pointer to shorter struct

MaxMin:

    push ecx 
    push edx

    movzx ecx, byte [eax]      ; load size of A into ECX
    movzx edx, byte [ebx]      ; load size of B into EDX
    cmp ecx, edx
    jae .done                  ; if A.size >= B.size, keep eax/ebx as-is
    xchg eax, ebx              ; else: swap them
.done:

    pop edx 
    pop ecx
    ret



; -------- part 2.B [ADDition] ----------


add_multi:

    push ebp
    mov ebp, esp
    push esi 
    push edi
    push ecx
    push edx

    ; Ensure eax = longer, ebx = shorter
    call MaxMin

    ; Save input A (longer) in edi, keep B in ebx
    mov edi, eax              ; edi = A (longer)
    
    ; Load sizes
    movzx esi, byte [edi]     ; esi = max_len
    movzx ecx, byte [ebx]     ; ecx = min_len
    
    mov edx, esi
    mov  [remaining_bytes] , edx
    sub [remaining_bytes],ecx
    add edx, 2                ; size byte + max_len + 1 spare byte for a final carry

    ; Allocate memory
    pushad
    push edx
    call malloc
    add esp, 4
    mov [SHEROT], eax         ; Save result pointer
    popad         
    mov eax,[SHEROT] 
    test eax, eax
    jz malloc_fail
    mov edx, eax              ; edx = result base address

    ; Store size (single byte, not a dword)
    mov eax, esi
    mov [edx], al
    lea edx, [edx + 1]        ; edx = result.num[0]

    xor esi, esi              ; esi = index
    clc     ;  Clear Carry Flag

    test ecx, ecx             ; min_len==0? nothing to add, just copy longer
    jz .check_remaining
.add_loop:

    mov al, [edi + esi + 1]   ; A.num[i]
    adc al, [ebx + esi + 1]   ;  just A[i] + B[i]
                    
    mov [edx + esi ], al       ; store result
    inc esi
    dec ecx
    jnz .add_loop
    
.check_remaining:
    inc dword[remaining_bytes]
    dec dword[remaining_bytes]
    jz .final_carry_check
    

.copy_remaining:

    mov al, [edi + esi + 1]
    adc al, 0
    mov [edx + esi], al
    inc esi
    dec dword[remaining_bytes]
    jnz .copy_remaining

.final_carry_check:
    jnc .done
    cmp byte [edx - 1], 0xFF   ; size maxed (255)? 256 bytes is unrepresentable
    je .done                   ; drop top carry rather than wrap size to 0
    mov byte [edx + esi], 1   ; extra byte for carry
    inc byte [edx - 1]        ; update size

.done:
    lea eax, [edx - 1]        ; return pointer to start of struct

    pop edx
    pop ecx
    pop edi
    pop esi
    mov esp, ebp
    pop ebp
    ret




; -------- part 3.A [random 16-bits] ----------

rand_num:

   
   push ebx
   push ecx


    mov ax, [STATE]
    mov bx, [MASK]
    and bx, ax
    xor cx, cx

.check_parity:
    test bx, 1
    jz .no_bit
    inc cx
.no_bit:
    shr bx, 1
    jnz .check_parity

    shr ax, 1
    test cx, 1
    jz .no_feedback
    or ax, 0x8000

.no_feedback:

    pop ecx
    pop ebx
    mov [STATE], ax        
    movzx eax, ax
    ret




; -------- part 3.B [random number] ----------

PRmulti:

    push ebx
    push esi
    push edi
    push ecx

.try_size:

    call rand_num
    test al, al             ; if AL == 0, try again
    jz .try_size
    mov cl, al              ; CL = size (1–255)

    ; allocate (size + 1) bytes: 1 for size + size bytes of data
    movzx edx, cl
    inc edx
    push ecx
    push edx
    call malloc
    pop edx
    pop ecx
    test eax, eax
    jz malloc_fail
    mov esi, eax            ; ESI = pointer to struct

    ; store size in first byte
    mov [esi], cl

    lea edi, [esi + 1]      ; EDI = pointer to num[]
    xor ebx, ebx            ; EBX = index = 0

.fill_loop:
    cmp bl, cl
    jae .done

    call rand_num
    mov [edi + ebx], al     ; store random byte
    inc bl
    jmp .fill_loop

.done:
    mov eax, esi            ; return struct pointer in EAX
    pop ecx
    pop edi
    pop esi
    pop ebx
    ret

malloc_fail:
    push fmt_oom
    call puts
    add esp, 4
    push dword 1
    call exit
