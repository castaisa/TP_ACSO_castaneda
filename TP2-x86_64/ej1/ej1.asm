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
extern memcpy
extern strlen
extern strcpy ; importo strcpy

mi_strdup:
    push rbp
    mov rbp, rsp
    push rbx
    
    ;Si me pasan null tiro error
    test rdi, rdi
    jz .error
    
    ;Guardo el puntero original
    mov rbx, rdi
    
    call strlen
    inc rax
    
    ;reservo memoria para copiar la nueva cadena
    mov rdi, rax
    call malloc
    
    ;retornar null si falla malloc
    test rax, rax
    jz .error
    
    ;copia la cadena original a la nueva memoria
    mov rdi, rax 
    mov rsi, rbx
    push rax
    call strcpy
    pop rax
    
    ;si funciona va a devolver el puntero a la nueva cadena
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

.error:
    ;Retorna NULL en caso de error
    xor rax, rax
    pop rbx
    mov rsp, rbp
    pop rbp
    ret


string_proc_list_create_asm:
    push rbp
    mov rbp, rsp
    
    ;reservar 16 bytes
    mov rdi, 16
    call malloc
    
    ;si malloc falla retornar null
    test rax, rax
    jz .error
    
    mov qword [rax], 0
    mov qword [rax + 8], 0
    
    ;rax ya tiene el puntero a retornar
    pop rbp
    ret

.error:
    ;retorna null
    pop rbp
    ret

string_proc_node_create_asm:
    ;guarda los registros que va a usar
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    
    ;guarda los parametros de la funcion
    mov bl, dil
    mov r12, rsi
    
    ;si el puntero al string es null retorno null
    test r12, r12
    jz .error
    
    ;reserva memoria para el nodo (32 bytes)
    mov rdi, 32
    call malloc
    mov r13, rax
    
    ;si malloc falla retorna null
    test r13, r13
    jz .error
    
    ;crea una copia del string usando mi strdup
    mov rdi, r12
    call mi_strdup
    
    ;se fija si mi strdup fue exitosa y si no, libera el nodo
    test rax, rax
    jz .cleanup_node
    
    ;inicializa los punteros al siguiete y al anterior del nodo, le asigna su proc_type y le pone su respectivo string
    mov qword [r13], 0
    mov qword [r13 + 8], 0
    mov byte [r13 + 16], bl
    mov qword [r13 + 24], rax
    
    ;si fue exitoso, retorna el puntero al nodo
    mov rax, r13
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.cleanup_node:    ;cleanup libera la memoria del nodo si strdup falla
    mov rdi, r13
    call free
    
.error:  ;error es una funcion que se llama si algo falla y retorna null
    xor rax, rax
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret


string_proc_list_add_node_asm:
push    rbp
        mov     rbp, rsp
        sub     rsp, 48
        mov     QWORD "" [rbp-24], rdi
        mov     eax, esi
        mov     QWORD "" [rbp-40], rdx
        mov     BYTE "" [rbp-28], al
        cmp     QWORD "" [rbp-24], 0
        je      .L19
        cmp     QWORD "" [rbp-40], 0
        je      .L19
        movzx   eax, BYTE "" [rbp-28]
        mov     rdx, QWORD "" [rbp-40]
        mov     rsi, rdx
        mov     edi, eax
        call    string_proc_node_create_asm
        mov     QWORD "" [rbp-8], rax
        cmp     QWORD "" [rbp-8], 0
        je      .L20
        mov     rax, QWORD "" [rbp-24]
        mov     rax, QWORD "" [rax]
        test    rax, rax
        jne     .L18
        mov     rax, QWORD "" [rbp-24]
        mov     rdx, QWORD "" [rbp-8]
        mov     QWORD "" [rax], rdx
        mov     rax, QWORD "" [rbp-24]
        mov     rdx, QWORD "" [rbp-8]
        mov     QWORD "" [rax+8], rdx
        jmp     .L13
.L18:
        mov     rax, QWORD "" [rbp-24]
        mov     rdx, QWORD "" [rax+8]
        mov     rax, QWORD "" [rbp-8]
        mov     QWORD "" [rax+8], rdx
        mov     rax, QWORD "" [rbp-24]
        mov     rax, QWORD "" [rax+8]
        mov     rdx, QWORD "" [rbp-8]
        mov     QWORD "" [rax], rdx
        mov     rax, QWORD "" [rbp-24]
        mov     rdx, QWORD "" [rbp-8]
        mov     QWORD "" [rax+8], rdx
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
        mov     QWORD "" [rbp-40], rdi
        mov     eax, esi
        mov     QWORD "" [rbp-56], rdx
        mov     BYTE "" [rbp-44], al
        cmp     QWORD "" [rbp-40], 0
        je      .L22
        cmp     QWORD "" [rbp-56], 0
        jne     .L23
.L22:
        mov     eax, 0
        jmp     .L24
.L23:
        mov     rax, QWORD "" [rbp-56]
        mov     rdi, rax
        call    mi_strdup
        mov     QWORD "" [rbp-8], rax
        cmp     QWORD "" [rbp-8], 0
        jne     .L25
        mov     eax, 0
        jmp     .L24
.L25:
        mov     rax, QWORD "" [rbp-40]
        mov     rax, QWORD "" [rax]
        mov     QWORD "" [rbp-16], rax
        jmp     .L26
.L29:
        mov     rax, QWORD "" [rbp-16]
        movzx   eax, BYTE "" [rax+16]
        cmp     BYTE "" [rbp-44], al
        jne     .L27
        mov     rax, QWORD "" [rbp-16]
        mov     rax, QWORD "" [rax+24]
        test    rax, rax
        je      .L27
        mov     rax, QWORD "" [rbp-16]
        mov     rdx, QWORD "" [rax+24]
        mov     rax, QWORD "" [rbp-8]
        mov     rsi, rdx
        mov     rdi, rax
        call    str_concat
        mov     QWORD "" [rbp-24], rax
        cmp     QWORD "" [rbp-24], 0
        jne     .L28
        mov     rax, QWORD "" [rbp-8]
        mov     rdi, rax
        call    free
        mov     eax, 0
        jmp     .L24
.L28:
        mov     rax, QWORD "" [rbp-8]
        mov     rdi, rax
        call    free
        mov     rax, QWORD "" [rbp-24]
        mov     QWORD "" [rbp-8], rax
.L27:
        mov     rax, QWORD "" [rbp-16]
        mov     rax, QWORD "" [rax]
        mov     QWORD "" [rbp-16], rax
.L26:
        cmp     QWORD "" [rbp-16], 0
        jne     .L29
        mov     rax, QWORD "" [rbp-8]
.L24:
        leave
        ret

