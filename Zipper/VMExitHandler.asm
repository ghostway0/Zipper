SaveRegs PROC
    mov [rcx + 0x00], rax
    mov [rcx + 0x08], rbx
    mov [rcx + 0x10], rcx
    mov [rcx + 0x18], rdx
    mov [rcx + 0x20], rsi
    mov [rcx + 0x28], rdi
    mov [rcx + 0x30], rsp
    mov [rcx + 0x38], rbp
    mov [rcx + 0x40], r8
    mov [rcx + 0x48], r9
    mov [rcx + 0x50], r10
    mov [rcx + 0x58], r11
    mov [rcx + 0x60], r12
    mov [rcx + 0x68], r13
    mov [rcx + 0x70], r14
    mov [rcx + 0x78], r15

    pushfq
    pop QWORD PTR [rcx + 0x88]

    mov rax, cr0
    mov [rcx + 0x90], rax
    mov rax, cr2
    mov [rcx + 0x98], rax
    mov rax, cr3
    mov [rcx + 0xa0], rax
    mov rax, cr4
    mov [rcx + 0xa8], rax

    mov rax, dr0
    mov [rcx + 0xb0], rax
    mov rax, dr1
    mov [rcx + 0xb8], rax
    mov rax, dr2
    mov [rcx + 0xc0], rax
    mov rax, dr3
    mov [rcx + 0xc8], rax
    mov rax, dr6
    mov [rcx + 0xd0], rax
    mov rax, dr7
    mov [rcx + 0xd8], rax

    mov ax, cs
    mov [rcx + 0xe0], ax
    mov ax, ds
    mov [rcx + 0xe2], ax
    mov ax, es
    mov [rcx + 0xe4], ax
    mov ax, fs
    mov [rcx + 0xe6], ax
    mov ax, gs
    mov [rcx + 0xe8], ax
    mov ax, ss
    mov [rcx + 0xea], ax

    RDFSBASE rax
    mov [rcx + 0xec], rax

    RDFSBASE rax
    mov [rcx + 0xf4], rax

    mov ecx, 0xC0000080 ; EFER
    rdmsr
    mov [rcx + 0xfc], rax

    ret
SaveRegs ENDP
