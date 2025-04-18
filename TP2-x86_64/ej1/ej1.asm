; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     edi, 16
        call    malloc
        mov     QWORD PTR [rbp-8], rax
        cmp     QWORD PTR [rbp-8], 0
        jne     .L6
        mov     eax, 0
        jmp     .L7
.L6:
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], 0
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax+8], 0
        mov     rax, QWORD PTR [rbp-8]
.L7:
        leave
        ret

string_proc_node_create_asm:
push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        mov     eax, edi
        mov     QWORD PTR [rbp-32], rsi
        mov     BYTE PTR [rbp-20], al
        cmp     QWORD PTR [rbp-32], 0
        jne     .L9
        mov     eax, 0
        jmp     .L10
.L9:
        mov     edi, 32
        call    malloc
        mov     QWORD PTR [rbp-8], rax
        cmp     QWORD PTR [rbp-8], 0
        jne     .L11
        mov     eax, 0
        jmp     .L10
.L11:
        mov     rax, QWORD PTR [rbp-32]
        mov     rdi, rax
        call    mi_strdup
        mov     QWORD PTR [rbp-16], rax
        cmp     QWORD PTR [rbp-16], 0
        jne     .L12
        mov     rax, QWORD PTR [rbp-8]
        mov     rdi, rax
        call    free
        mov     eax, 0
        jmp     .L10
.L12:
        mov     rax, QWORD PTR [rbp-8]
        movzx   edx, BYTE PTR [rbp-20]
        mov     BYTE PTR [rax+16], dl
        mov     rax, QWORD PTR [rbp-8]
        mov     rdx, QWORD PTR [rbp-16]
        mov     QWORD PTR [rax+24], rdx
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], 0
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax+8], 0
        mov     rax, QWORD PTR [rbp-8]
.L10:
        leave
        ret

string_proc_list_add_node_asm:
push    rbp
        mov     rbp, rsp
        sub     rsp, 48
        mov     QWORD PTR [rbp-24], rdi
        mov     eax, esi
        mov     QWORD PTR [rbp-40], rdx
        mov     BYTE PTR [rbp-28], al
        cmp     QWORD PTR [rbp-24], 0
        je      .L19
        cmp     QWORD PTR [rbp-40], 0
        je      .L19
        movzx   eax, BYTE PTR [rbp-28]
        mov     rdx, QWORD PTR [rbp-40]
        mov     rsi, rdx
        mov     edi, eax
        call    string_proc_node_create
        mov     QWORD PTR [rbp-8], rax
        cmp     QWORD PTR [rbp-8], 0
        je      .L20
        mov     rax, QWORD PTR [rbp-24]
        mov     rax, QWORD PTR [rax]
        test    rax, rax
        jne     .L18
        mov     rax, QWORD PTR [rbp-24]
        mov     rdx, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], rdx
        mov     rax, QWORD PTR [rbp-24]
        mov     rdx, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax+8], rdx
        jmp     .L13
.L18:
        mov     rax, QWORD PTR [rbp-24]
        mov     rdx, QWORD PTR [rax+8]
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax+8], rdx
        mov     rax, QWORD PTR [rbp-24]
        mov     rax, QWORD PTR [rax+8]
        mov     rdx, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], rdx
        mov     rax, QWORD PTR [rbp-24]
        mov     rdx, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax+8], rdx
        jmp     .L13
.L19:
        nop
        jmp     .L13
.L20:
        nop
.L13:
        leave
        ret

string_proc_list_concat_asm:
push    rbp
        mov     rbp, rsp
        sub     rsp, 64
        mov     QWORD PTR [rbp-40], rdi
        mov     eax, esi
        mov     QWORD PTR [rbp-56], rdx
        mov     BYTE PTR [rbp-44], al
        cmp     QWORD PTR [rbp-40], 0
        je      .L22
        cmp     QWORD PTR [rbp-56], 0
        jne     .L23
.L22:
        mov     eax, 0
        jmp     .L24
.L23:
        mov     rax, QWORD PTR [rbp-56]
        mov     rdi, rax
        call    mi_strdup
        mov     QWORD PTR [rbp-8], rax
        cmp     QWORD PTR [rbp-8], 0
        jne     .L25
        mov     eax, 0
        jmp     .L24
.L25:
        mov     rax, QWORD PTR [rbp-40]
        mov     rax, QWORD PTR [rax]
        mov     QWORD PTR [rbp-16], rax
        jmp     .L26
.L29:
        mov     rax, QWORD PTR [rbp-16]
        movzx   eax, BYTE PTR [rax+16]
        cmp     BYTE PTR [rbp-44], al
        jne     .L27
        mov     rax, QWORD PTR [rbp-16]
        mov     rax, QWORD PTR [rax+24]
        test    rax, rax
        je      .L27
        mov     rax, QWORD PTR [rbp-16]
        mov     rdx, QWORD PTR [rax+24]
        mov     rax, QWORD PTR [rbp-8]
        mov     rsi, rdx
        mov     rdi, rax
        call    str_concat
        mov     QWORD PTR [rbp-24], rax
        cmp     QWORD PTR [rbp-24], 0
        jne     .L28
        mov     rax, QWORD PTR [rbp-8]
        mov     rdi, rax
        call    free
        mov     eax, 0
        jmp     .L24
.L28:
        mov     rax, QWORD PTR [rbp-8]
        mov     rdi, rax
        call    free
        mov     rax, QWORD PTR [rbp-24]
        mov     QWORD PTR [rbp-8], rax
.L27:
        mov     rax, QWORD PTR [rbp-16]
        mov     rax, QWORD PTR [rax]
        mov     QWORD PTR [rbp-16], rax
.L26:
        cmp     QWORD PTR [rbp-16], 0
        jne     .L29
        mov     rax, QWORD PTR [rbp-8]
.L24:
        leave
        ret

