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
    cmp rdi, NULL
    je .error
    
    ;Guardo el puntero original
    mov rbx, rdi
    
    call strlen
    inc rax
    
    ;reservo memoria para copiar la nueva cadena
    mov rdi, rax
    call malloc
    
    ;retornar null si falla malloc
    cmp rax, NULL
    je .error
    
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
    mov rax, NULL
    pop rbx
    mov rsp, rbp
    pop rbp
    ret


string_proc_list_create_asm:
    push rbp
    mov rbp, rsp
    
    ;reservar 16 bytes para la estructura de la lista (puntero al primero y all ultimo)
    mov rdi, 16
    call malloc
    
    ;si malloc falla retornar null
    cmp rax, NULL
    je .error
    
    ;inicializa los punteros con null
    mov qword [rax], NULL
    mov qword [rax + 8], NULL
    
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
    cmp r12, NULL
    je .error
    
    ;reserva memoria para el nodo (32 bytes)
    mov rdi, 32
    call malloc
    mov r13, rax
    
    ;si malloc falla retorna null
    cmp r13, NULL
    je .error
    
    ;crea una copia del string usando mi strdup
    mov rdi, r12
    call mi_strdup
    
    ;se fija si mi strdup fue exitosa y si no, libera el nodo
    cmp rax, NULL
    je .cleanup_node
    
    ;inicializa los punteros al siguiete y al anterior del nodo, le asigna su proc_type y le pone su respectivo string
    mov qword [r13], NULL
    mov qword [r13 + 8], NULL
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
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret


string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    
    ;guarda los parametros
    mov rbx, rdi
    mov r12b, sil
    mov r13, rdx

    ;si la lista es null da error
    cmp rbx, NULL
    je .error
    
    ;si el string es null da error
    cmp r13, NULL  
    je .error
    
    ;crea un nuevo nodo con sus respectivos parametros
    movzx rdi, r12b
    mov rsi, r13
    call string_proc_node_create_asm
    mov r13, rax
    
    ;si falla la creacion da error
    cmp r13, NULL
    je .error
    
    ;se fija si la lista esta vacia
    mov rax, qword [rbx]
    cmp rax, NULL
    je .lista_vacia
    
    mov rax, qword [rbx + 8]
    mov qword [r13 + 8], rax
    mov qword [rax], r13
    mov qword [rbx + 8], r13
    jmp .salir
    
.lista_vacia:
    mov qword [rbx], r13
    mov qword [rbx + 8], r13
    jmp .salir
    
.error: ;como devuelve void la funcion no tengo que cambiar rax, no hago nada
    
    
.salir: ;libera los registros y retorna
    pop r13
    pop r12  
    pop rbx
    pop rbp
    ret

string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    
    ;guardo parametros
    mov rbx, rdi
    mov r12b, sil
    mov r13, rdx
    
    ;si la lista es null da error
    test rbx, rbx
    jz .error
    
    ;si string inicial es null da error
    test r13, r13
    jz .error
    
    ;duplica el string inicial
    mov rdi, r13
    call mi_strdup
    mov r14, rax
    
    ;si falla strdup da error
    test r14, r14
    jz .error
    
    ;inicializa la iteración por la lista
    mov r13, qword [rbx]    ;r13 va a ser el primer nodo (current)
    
.recorrer_lista:
    ;si current es null, termina
    test r13, r13
    jz .terminado
    
    ;verifica si el nodo tiene el type buscado
    movzx eax, byte [r13 + 16]  ;eax = current->proc_type
    cmp r12b, al
    jne .siguiente_nodo         ;si no coincide va al siguiente nodo
    
    ;verifica si el nodo tiene un string válido
    mov rax, qword [r13 + 24]
    test rax, rax
    jz .siguiente_nodo          ;si string es null va al siguiente nodo
    
    ;concatenar strings
    mov rdi, r14
    mov rsi, rax
    call str_concat
    mov rbx, rax        ;rbx = nuevo string concatenado
    
    ;verifica si la concatenación fue exitosa
    test rbx, rbx
    jz .error_con_cleanup       ;si falla, limpia y da error
    
    ;libera el string anterior y asigna el nuevo
    mov rdi, r14
    call free
    mov r14, rbx        ;r14 = nuevo string resultado
    
.siguiente_nodo:
    ;va al siguiente nodo
    mov r13, qword [r13]    ;r13 = current->next
    jmp .recorrer_lista
    
.terminado:
    ;devuelve el string concatenado
    mov rax, r14
    jmp .salir
    
.error_con_cleanup:
    ;libera la memoria en caso de error
    mov rdi, r14
    call free
    
.error:
    ;retirna null
    xor rax, rax
    
.salir:
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret