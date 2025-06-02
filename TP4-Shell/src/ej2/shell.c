#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 65
#define MAX_COMMAND_LENGTH 1024

//Función para parsear argumentos respetando comillas simples y dobles
int parse_arguments(char *command_str, char *args[]) {
    int arg_count = 0;
    char *ptr = command_str;
    
    while (*ptr && arg_count < MAX_ARGS - 1) {
        //Saltar espacios en blanco
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (*ptr == '\0') break;
        
        char *arg_start = ptr;
        char quote_char = '\0';
        
        //Detectar si empieza con comilla (simple o doble)
        if (*ptr == '"' || *ptr == '\'') {
            quote_char = *ptr;
            ptr++;
            arg_start = ptr; 
            
            //Buscar la comilla de cierre del mismo tipo
            while (*ptr && *ptr != quote_char) {
                ptr++;
            }
            
            if (*ptr == quote_char) {
                *ptr = '\0';
                ptr++;
            } else {
                // Comilla no cerrada - tratarla como texto normal
                ptr = arg_start - 1;
                goto parse_normal;
            }
        } else {
            parse_normal:
            //Argumento normal, buscar el siguiente espacio
            while (*ptr && *ptr != ' ' && *ptr != '\t') {
                ptr++;
            }
            
            if (*ptr) {
                *ptr = '\0';
                ptr++;
            }
        }
        
        args[arg_count++] = arg_start;
    }
    
    //Verificar si hay más argumentos después del límite
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '\0') {
        return -1;
    }
    
    args[arg_count] = NULL;
    return arg_count;
}

// Función para separar comandos por pipes respetando comillas
int parse_pipeline(char *command, char *commands[], int max_commands) {
    int command_count = 0;
    char *start = command;
    char *ptr = command;
    
    while (*ptr && command_count < max_commands) {
        if (*ptr == '"' || *ptr == '\'') {
            char quote_char = *ptr;
            ptr++;
            
            while (*ptr && *ptr != quote_char) {
                ptr++;
            }
            
            if (*ptr == quote_char) {
                ptr++;
            }
            continue;
        }
        
        //Si encontramos un pipe fuera de comillas
        if (*ptr == '|') {
            //Terminar el comando actual
            *ptr = '\0';
            
            //Limpiar espacios del comando
            char *cmd_start = start;
            while (*cmd_start == ' ' || *cmd_start == '\t') cmd_start++;
            
            char *cmd_end = ptr - 1;
            while (cmd_end > cmd_start && (*cmd_end == ' ' || *cmd_end == '\t')) {
                *cmd_end = '\0';
                cmd_end--;
            }
            
            // Verificar que el comando no esté vacío
            if (strlen(cmd_start) == 0) {
                fprintf(stderr, "Error: Comando vacío en pipeline\n");
                return -1;
            }
            
            commands[command_count++] = cmd_start;
            
            //Avanzar al siguiente comando
            ptr++;
            start = ptr;
            continue;
        }
        
        ptr++;
    }
    
    // Procesar el último comando
    if (*start) {
        //Limpiar espacios del último comando
        char *cmd_start = start;
        while (*cmd_start == ' ' || *cmd_start == '\t') cmd_start++;
        
        char *cmd_end = start + strlen(start) - 1;
        while (cmd_end > cmd_start && (*cmd_end == ' ' || *cmd_end == '\t')) {
            *cmd_end = '\0';
            cmd_end--;
        }
        
        if (strlen(cmd_start) > 0) {
            commands[command_count++] = cmd_start;
        }
    }
    
    //Verificar si se excedió el límite de comandos
    if (command_count >= max_commands && *ptr) {
        fprintf(stderr, "Error: Pipeline excede el límite máximo de %d comandos\n", max_commands);
        return -1;
    }
    
    return command_count;
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    char *commands[MAX_COMMANDS];
    int command_count = 0;


    while (1) 
    {
        //resetea oara cada linea
        command_count = 0;
        
        //Lee la línea de comandos del usuario
        fflush(stdout);
        
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }
        
        //Verificar si el comando fue truncado (línea demasiado larga)
        int len = strlen(command);
        if (len == MAX_COMMAND_LENGTH - 1 && command[len-1] != '\n') {
            fprintf(stderr, "Error: Comando demasiado largo (máximo %d caracteres)\n", MAX_COMMAND_LENGTH - 1);
            // Limpiar el buffer de entrada
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        command[strcspn(command, "\n")] = '\0';
        if (strlen(command) == 0) {
            continue;
        }
        
        //Comando 'exit' para salir del shell
        if (strcmp(command, "exit") == 0) {
            break;
        }

        //Verificar pipe al inicio
        if (command[0] == '|') {
            fprintf(stderr, "zsh: parse error near `|'\n");
            continue;
        }
        
        //Verificar pipe al final
        len = strlen(command);
        if (len > 0 && command[len-1] == '|') {
            fprintf(stderr, "Error: Pipe al final del comando\n");
            continue;
        }
        
        //Verificar pipes dobles (fuera de comillas)
        char *check_ptr = command;
        int found_double_pipe = 0;
        while (*check_ptr) {
            if (*check_ptr == '"' || *check_ptr == '\'') {
                char quote_char = *check_ptr;
                check_ptr++;
                while (*check_ptr && *check_ptr != quote_char) check_ptr++;
                if (*check_ptr) check_ptr++;
            } else if (*check_ptr == '|' && *(check_ptr + 1) == '|') {
                found_double_pipe = 1;
                break;
            } else {
                check_ptr++;
            }
        }
        
        if (found_double_pipe) {
            fprintf(stderr, "Error: Pipes dobles no permitidos\n");
            continue;
        }

        command_count = parse_pipeline(command, commands, MAX_COMMANDS);
        
        if (command_count <= 0) {
            continue;  //Error en parsing o no hay comandos válidos
        }


        int num_pipes = command_count - 1;
        int pipes[MAX_COMMANDS-1][2];
        
        for (int i = 0; i < num_pipes; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("Error creando pipe");
                goto cleanup_and_continue;
            }
        }

        
        pid_t pids[MAX_COMMANDS];
        int pids_count = 0;
        int command_error = 0;
        
        for (int i = 0; i < command_count; i++) 
        {
            //PARSING DE ARGUMENTOS PARA CADA COMANDO
            char *args[MAX_ARGS];
            int arg_count = 0;
            
            //Crear copia del comando para no modificar el original
            char command_copy[1024];
            strncpy(command_copy, commands[i], sizeof(command_copy) - 1);
            command_copy[sizeof(command_copy) - 1] = '\0';
            
            //Parsear argumentos con manejo de comillas
            arg_count = parse_arguments(command_copy, args);
            
            if (arg_count == -1) {
                fprintf(stderr, "Error: El comando '%s' excede el límite máximo de %d argumentos\n", 
                        commands[i], MAX_ARGS - 1);
                command_error = 1;
                break;
            }
            
            if (args[0] == NULL) {
                fprintf(stderr, "ERROR - Comando %d vacío\n", i);
                command_error = 1;
                break;
            }
            
            //CREAR PROCESO HIJO
            pid_t pid = fork();
            
            if (pid == -1) {
                perror("Error en fork");
                command_error = 1;
                break;
            }
            
            if (pid == 0) {
                
                if (i > 0) {
                    if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                        perror("Error en dup2 (stdin)");
                        exit(1);
                    }
                }
                
                if (i < command_count - 1) {
                    if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                        perror("Error en dup2 (stdout)");
                        exit(1);
                    }
                }
                // Cerrar todos los pipes en el proceso hijo
                for (int j = 0; j < num_pipes; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                
                //Ejecutar el programa
                execvp(args[0], args);
                
                //Si llega aca execvp fallo
                perror("Error en execvp");
                exit(127);
                
            } else {
                pids[pids_count++] = pid;
            }
        }

        //Si hubo error en algún comando, limpiar y continuar
        if (command_error) {
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            for (int j = 0; j < pids_count; j++) {
                kill(pids[j], SIGTERM);
                waitpid(pids[j], NULL, 0);
            }
            continue;
        }

        //Cerrar los pipes en el proceso padre
        for (int i = 0; i < num_pipes; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < pids_count; i++) {
            int status;
            pid_t finished_pid = waitpid(pids[i], &status, 0);
            
            if (finished_pid == -1) {
                perror("Error en waitpid");
            }
        }
        
        continue;
        
        cleanup_and_continue:
        //Limpiar pipes en caso de error
        for (int i = 0; i < num_pipes; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        continue;
    }
    
    return 0;
}