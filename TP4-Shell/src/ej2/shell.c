#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 65
#define MAX_COMMAND_LENGTH 1024  // Aumentado de 1024 a 8192 (8KB)

// Función para parsear argumentos respetando comillas simples y dobles
int parse_arguments(char *command_str, char *args[]) {
    int arg_count = 0;
    char *ptr = command_str;
    
    while (*ptr && arg_count < MAX_ARGS - 1) {
        // Saltar espacios en blanco
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (*ptr == '\0') break;
        
        char *arg_start = ptr;
        char quote_char = '\0';
        
        // Detectar si empieza con comilla (simple o doble)
        if (*ptr == '"' || *ptr == '\'') {
            quote_char = *ptr;
            ptr++; // Saltar la comilla de apertura
            arg_start = ptr; // El argumento empieza después de la comilla
            
            // Buscar la comilla de cierre del mismo tipo
            while (*ptr && *ptr != quote_char) {
                ptr++;
            }
            
            if (*ptr == quote_char) {
                *ptr = '\0'; // Terminar la cadena en la comilla de cierre
                ptr++; // Avanzar después de la comilla
            } else {
                // Comilla no cerrada - tratarla como texto normal
                ptr = arg_start - 1; // Volver al inicio incluyendo la comilla
                goto parse_normal;
            }
        } else {
            parse_normal:
            // Argumento normal, buscar el siguiente espacio
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
    
    // Verificar si hay más argumentos después del límite
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '\0') {
        return -1;
    }
    
    args[arg_count] = NULL;
    return arg_count;
}

// Función mejorada para separar comandos por pipes respetando comillas
int parse_pipeline(char *command, char *commands[], int max_commands) {
    int command_count = 0;
    char *start = command;
    char *ptr = command;
    
    while (*ptr && command_count < max_commands) {
        // Si encontramos una comilla, saltarla completamente
        if (*ptr == '"' || *ptr == '\'') {
            char quote_char = *ptr;
            ptr++; // Saltar comilla de apertura
            
            // Buscar comilla de cierre
            while (*ptr && *ptr != quote_char) {
                ptr++;
            }
            
            if (*ptr == quote_char) {
                ptr++; // Saltar comilla de cierre
            }
            continue;
        }
        
        // Si encontramos un pipe fuera de comillas
        if (*ptr == '|') {
            // Terminar el comando actual
            *ptr = '\0';
            
            // Limpiar espacios del comando
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
            
            // Avanzar al siguiente comando
            ptr++;
            start = ptr;
            continue;
        }
        
        ptr++;
    }
    
    // Procesar el último comando
    if (*start) {
        // Limpiar espacios del último comando
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
    
    // Verificar si se excedió el límite de comandos
    if (command_count >= max_commands && *ptr) {
        fprintf(stderr, "Error: Pipeline excede el límite máximo de %d comandos\n", max_commands);
        return -1;
    }
    
    return command_count;
}

int main() {
    char command[MAX_COMMAND_LENGTH];  // Ahora puede manejar hasta 8KB
    char *commands[MAX_COMMANDS];
    int command_count = 0;


    while (1) 
    {
        // RESETEAR PARA CADA LÍNEA
        command_count = 0;
        
        // Leer línea de comandos del usuario
        fflush(stdout);
        
        if (!fgets(command, sizeof(command), stdin)) {
            break; // EOF
        }
        
        // Verificar si el comando fue truncado (línea demasiado larga)
        int len = strlen(command);
        if (len == MAX_COMMAND_LENGTH - 1 && command[len-1] != '\n') {
            fprintf(stderr, "Error: Comando demasiado largo (máximo %d caracteres)\n", MAX_COMMAND_LENGTH - 1);
            // Limpiar el buffer de entrada
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        // Remover el salto de línea
        command[strcspn(command, "\n")] = '\0';
        
        // Si el usuario presiona solo Enter, continuar
        if (strlen(command) == 0) {
            continue;
        }
        
        // Comando 'exit' para salir del shell
        if (strcmp(command, "exit") == 0) {
            break;
        }

        // VALIDACIÓN: Verificar pipes mal formados
        // Verificar pipe al inicio
        if (command[0] == '|') {
            fprintf(stderr, "zsh: parse error near `|'\n");
            continue;
        }
        
        // Verificar pipe al final
        len = strlen(command);
        if (len > 0 && command[len-1] == '|') {
            fprintf(stderr, "Error: Pipe al final del comando\n");
            continue;
        }
        
        // Verificar pipes dobles (fuera de comillas)
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

        // PASO 1: PARSING MEJORADO DE PIPELINE
        command_count = parse_pipeline(command, commands, MAX_COMMANDS);
        
        if (command_count <= 0) {
            continue;  // Error en parsing o no hay comandos válidos
        }

        // PASO 2: CREAR LOS PIPES NECESARIOS
        int num_pipes = command_count - 1;
        int pipes[MAX_COMMANDS-1][2];
        
        for (int i = 0; i < num_pipes; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("Error creando pipe");
                goto cleanup_and_continue;
            }
        }

        // PASO 3: CREAR PROCESOS Y CONFIGURAR REDIRECCIONES
        pid_t pids[MAX_COMMANDS];
        int pids_count = 0;
        int command_error = 0;
        
        for (int i = 0; i < command_count; i++) 
        {
            // PARSING DE ARGUMENTOS PARA CADA COMANDO
            char *args[MAX_ARGS];
            int arg_count = 0;
            
            // Crear copia del comando para no modificar el original
            char command_copy[1024];  // Buffer más pequeño para cada comando individual
            strncpy(command_copy, commands[i], sizeof(command_copy) - 1);
            command_copy[sizeof(command_copy) - 1] = '\0';
            
            // Parsear argumentos con manejo de comillas
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
            
            // CREAR PROCESO HIJO
            pid_t pid = fork();
            
            if (pid == -1) {
                perror("Error en fork");
                command_error = 1;
                break;
            }
            
            if (pid == 0) {
                // ========== CÓDIGO DEL PROCESO HIJO ==========
                
                // REDIRECCIÓN DE ENTRADA (stdin)
                if (i > 0) {
                    if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                        perror("Error en dup2 (stdin)");
                        exit(1);
                    }
                }
                
                // REDIRECCIÓN DE SALIDA (stdout)
                if (i < command_count - 1) {
                    if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                        perror("Error en dup2 (stdout)");
                        exit(1);
                    }
                }
                
                // CERRAR TODOS LOS DESCRIPTORES DE PIPES EN EL HIJO
                for (int j = 0; j < num_pipes; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                
                // EJECUTAR EL PROGRAMA
                execvp(args[0], args);
                
                // Si llegamos aquí, execvp() falló
                perror("Error en execvp");
                exit(127);
                
            } else {
                // ========== CÓDIGO DEL PROCESO PADRE ==========
                pids[pids_count++] = pid;
            }
        }

        // Si hubo error en algún comando, limpiar y continuar
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

        // CERRAR PIPES EN EL PROCESO PADRE
        for (int i = 0; i < num_pipes; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // PASO 4: ESPERAR A QUE TERMINEN TODOS LOS PROCESOS HIJOS
        for (int i = 0; i < pids_count; i++) {
            int status;
            pid_t finished_pid = waitpid(pids[i], &status, 0);
            
            if (finished_pid == -1) {
                perror("Error en waitpid");
            }
        }
        
        continue;
        
        cleanup_and_continue:
        // Limpiar pipes en caso de error
        for (int i = 0; i < num_pipes; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        continue;
    }
    
    return 0;
}


// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <string.h>
// #include <signal.h>

// #define MAX_COMMANDS 200
// #define MAX_ARGS 65

// // Función para parsear argumentos respetando comillas simples y dobles
// // Retorna -1 si excede MAX_ARGS, o el número de argumentos parseados
// int parse_arguments(char *command_str, char *args[]) {
//     int arg_count = 0;
//     char *ptr = command_str;
    
//     while (*ptr && arg_count < MAX_ARGS - 1) {
//         // Saltar espacios en blanco
//         while (*ptr == ' ' || *ptr == '\t') ptr++;
        
//         if (*ptr == '\0') break;
        
//         char *arg_start = ptr;
//         char quote_char = '\0';
        
//         // Detectar si empieza con comilla (simple o doble)
//         if (*ptr == '"' || *ptr == '\'') {
//             quote_char = *ptr;
//             ptr++; // Saltar la comilla de apertura
//             arg_start = ptr; // El argumento empieza después de la comilla
            
//             // Buscar la comilla de cierre del mismo tipo
//             while (*ptr && *ptr != quote_char) {
//                 ptr++;
//             }
            
//             if (*ptr == quote_char) {
//                 *ptr = '\0'; // Terminar la cadena en la comilla de cierre
//                 ptr++; // Avanzar después de la comilla
//             } else {
//                 // Comilla no cerrada - tratarla como texto normal
//                 ptr = arg_start - 1; // Volver al inicio incluyendo la comilla
//                 goto parse_normal;
//             }
//         } else {
//             parse_normal:
//             // Argumento normal, buscar el siguiente espacio
//             while (*ptr && *ptr != ' ' && *ptr != '\t') {
//                 ptr++;
//             }
            
//             if (*ptr) {
//                 *ptr = '\0';
//                 ptr++;
//             }
//         }
        
//         args[arg_count++] = arg_start;
//     }
    
//     // Verificar si hay más argumentos después del límite
//     while (*ptr == ' ' || *ptr == '\t') ptr++;
//     if (*ptr != '\0') {
//         // Hay más argumentos, se excedió el límite
//         return -1;
//     }
    
//     args[arg_count] = NULL;
//     return arg_count;
// }

// // Función mejorada para separar comandos por pipes respetando comillas
// int parse_pipeline(char *command, char *commands[], int max_commands) {
//     int command_count = 0;
//     char *start = command;
//     char *ptr = command;
    
//     while (*ptr && command_count < max_commands) {
//         // Si encontramos una comilla, saltarla completamente
//         if (*ptr == '"' || *ptr == '\'') {
//             char quote_char = *ptr;
//             ptr++; // Saltar comilla de apertura
            
//             // Buscar comilla de cierre
//             while (*ptr && *ptr != quote_char) {
//                 ptr++;
//             }
            
//             if (*ptr == quote_char) {
//                 ptr++; // Saltar comilla de cierre
//             }
//             continue;
//         }
        
//         // Si encontramos un pipe fuera de comillas
//         if (*ptr == '|') {
//             // Terminar el comando actual
//             *ptr = '\0';
            
//             // Limpiar espacios del comando
//             char *cmd_start = start;
//             while (*cmd_start == ' ' || *cmd_start == '\t') cmd_start++;
            
//             char *cmd_end = ptr - 1;
//             while (cmd_end > cmd_start && (*cmd_end == ' ' || *cmd_end == '\t')) {
//                 *cmd_end = '\0';
//                 cmd_end--;
//             }
            
//             // Verificar que el comando no esté vacío
//             if (strlen(cmd_start) == 0) {
//                 fprintf(stderr, "Error: Comando vacío en pipeline\n");
//                 return -1;
//             }
            
//             commands[command_count++] = cmd_start;
            
//             // Avanzar al siguiente comando
//             ptr++;
//             start = ptr;
//             continue;
//         }
        
//         ptr++;
//     }
    
//     // Procesar el último comando
//     if (*start) {
//         // Limpiar espacios del último comando
//         char *cmd_start = start;
//         while (*cmd_start == ' ' || *cmd_start == '\t') cmd_start++;
        
//         char *cmd_end = start + strlen(start) - 1;
//         while (cmd_end > cmd_start && (*cmd_end == ' ' || *cmd_end == '\t')) {
//             *cmd_end = '\0';
//             cmd_end--;
//         }
        
//         if (strlen(cmd_start) > 0) {
//             commands[command_count++] = cmd_start;
//         }
//     }
    
//     // Verificar si se excedió el límite de comandos
//     if (command_count >= max_commands && *ptr) {
//         fprintf(stderr, "Error: Pipeline excede el límite máximo de %d comandos\n", max_commands);
//         return -1;
//     }
    
//     return command_count;
// }

// int main() {
//     char command[1024];  // Aumentado para comandos más largos
//     char *commands[MAX_COMMANDS];
//     int command_count = 0;

//     while (1) 
//     {
//         // RESETEAR PARA CADA LÍNEA
//         command_count = 0;
        
//         // Leer línea de comandos del usuario
//         if (!fgets(command, sizeof(command), stdin)) {
//             break; // EOF
//         }
        
//         // Remover el salto de línea
//         command[strcspn(command, "\n")] = '\0';
        
//         // Si el usuario presiona solo Enter, continuar
//         if (strlen(command) == 0) {
//             continue;
//         }
        
//         // Comando 'exit' para salir del shell
//         if (strcmp(command, "exit") == 0) {
//             break;
//         }

//         // VALIDACIÓN: Verificar pipes mal formados
//         // Verificar pipe al inicio
//         if (command[0] == '|') {
//             fprintf(stderr, "zsh: parse error near `|'\n");
//             continue;
//         }
        
//         // Verificar pipe al final
//         int len = strlen(command);
//         if (len > 0 && command[len-1] == '|') {
//             fprintf(stderr, "Error: Pipe al final del comando\n");
//             continue;
//         }
        
//         // Verificar pipes dobles (fuera de comillas)
//         char *check_ptr = command;
//         int found_double_pipe = 0;
//         while (*check_ptr) {
//             if (*check_ptr == '"' || *check_ptr == '\'') {
//                 char quote_char = *check_ptr;
//                 check_ptr++;
//                 while (*check_ptr && *check_ptr != quote_char) check_ptr++;
//                 if (*check_ptr) check_ptr++;
//             } else if (*check_ptr == '|' && *(check_ptr + 1) == '|') {
//                 found_double_pipe = 1;
//                 break;
//             } else {
//                 check_ptr++;
//             }
//         }
        
//         if (found_double_pipe) {
//             fprintf(stderr, "Error: Pipes dobles no permitidos\n");
//             continue;
//         }

//         // PASO 1: PARSING MEJORADO DE PIPELINE
//         command_count = parse_pipeline(command, commands, MAX_COMMANDS);
        
//         if (command_count <= 0) {
//             continue;  // Error en parsing o no hay comandos válidos
//         }
        
//         // Debug: mostrar comandos parseados
//         /*
//         printf("DEBUG - Total comandos encontrados: %d\n", command_count);
//         for (int i = 0; i < command_count; i++) {
//             printf("DEBUG - Comando %d: '%s'\n", i, commands[i]);
//         }
//         */

//         // PASO 2: CREAR LOS PIPES NECESARIOS
//         int num_pipes = command_count - 1;
//         int pipes[MAX_COMMANDS-1][2];
        
//         for (int i = 0; i < num_pipes; i++) {
//             if (pipe(pipes[i]) == -1) {
//                 perror("Error creando pipe");
//                 goto cleanup_and_continue;
//             }
//         }

//         // PASO 3: CREAR PROCESOS Y CONFIGURAR REDIRECCIONES
//         pid_t pids[MAX_COMMANDS];
//         int pids_count = 0;
//         int command_error = 0;
        
//         for (int i = 0; i < command_count; i++) 
//         {
//             // PARSING DE ARGUMENTOS PARA CADA COMANDO
//             char *args[MAX_ARGS];
//             int arg_count = 0;
            
//             // Crear copia del comando para no modificar el original
//             char command_copy[512];
//             strncpy(command_copy, commands[i], sizeof(command_copy) - 1);
//             command_copy[sizeof(command_copy) - 1] = '\0';
            
//             // Parsear argumentos con manejo de comillas
//             arg_count = parse_arguments(command_copy, args);
            
//             if (arg_count == -1) {
//                 fprintf(stderr, "Error: El comando '%s' excede el límite máximo de %d argumentos\n", 
//                         commands[i], MAX_ARGS - 1);
//                 command_error = 1;
//                 break;
//             }
            
//             if (args[0] == NULL) {
//                 fprintf(stderr, "ERROR - Comando %d vacío\n", i);
//                 command_error = 1;
//                 break;
//             }
            
//             // Debug: mostrar argumentos parseados
//             /*
//             printf("DEBUG - Comando %d: '%s' con %d argumentos: ", i, args[0], arg_count);
//             for (int j = 0; j < arg_count; j++) {
//                 printf("'%s' ", args[j]);
//             }
//             printf("\n");
//             */
            
//             // CREAR PROCESO HIJO
//             pid_t pid = fork();
            
//             if (pid == -1) {
//                 perror("Error en fork");
//                 command_error = 1;
//                 break;
//             }
            
//             if (pid == 0) {
//                 // ========== CÓDIGO DEL PROCESO HIJO ==========
                
//                 // REDIRECCIÓN DE ENTRADA (stdin)
//                 if (i > 0) {
//                     if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
//                         perror("Error en dup2 (stdin)");
//                         exit(1);
//                     }
//                 }
                
//                 // REDIRECCIÓN DE SALIDA (stdout)
//                 if (i < command_count - 1) {
//                     if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
//                         perror("Error en dup2 (stdout)");
//                         exit(1);
//                     }
//                 }
                
//                 // CERRAR TODOS LOS DESCRIPTORES DE PIPES EN EL HIJO
//                 for (int j = 0; j < num_pipes; j++) {
//                     close(pipes[j][0]);
//                     close(pipes[j][1]);
//                 }
                
//                 // EJECUTAR EL PROGRAMA
//                 execvp(args[0], args);
                
//                 // Si llegamos aquí, execvp() falló
//                 perror("Error en execvp");
//                 exit(127);
                
//             } else {
//                 // ========== CÓDIGO DEL PROCESO PADRE ==========
//                 pids[pids_count++] = pid;
//             }
//         }

//         // Si hubo error en algún comando, limpiar y continuar
//         if (command_error) {
//             for (int j = 0; j < num_pipes; j++) {
//                 close(pipes[j][0]);
//                 close(pipes[j][1]);
//             }
//             for (int j = 0; j < pids_count; j++) {
//                 kill(pids[j], SIGTERM);
//                 waitpid(pids[j], NULL, 0);
//             }
//             continue;
//         }

//         // CERRAR PIPES EN EL PROCESO PADRE
//         for (int i = 0; i < num_pipes; i++) {
//             close(pipes[i][0]);
//             close(pipes[i][1]);
//         }

//         // PASO 4: ESPERAR A QUE TERMINEN TODOS LOS PROCESOS HIJOS
//         for (int i = 0; i < pids_count; i++) {
//             int status;
//             pid_t finished_pid = waitpid(pids[i], &status, 0);
            
//             if (finished_pid == -1) {
//                 perror("Error en waitpid");
//             }
//         }
        
//         continue;
        
//         cleanup_and_continue:
//         // Limpiar pipes en caso de error
//         for (int i = 0; i < num_pipes; i++) {
//             close(pipes[i][0]);
//             close(pipes[i][1]);
//         }
//         continue;
//     }
    
//     return 0;
// }












// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <string.h>

// #define MAX_COMMANDS 200
// #define MAX_ARGS 50  // Máximo número de argumentos por comando

// // Función para parsear argumentos respetando comillas
// int parse_arguments(char *command_str, char *args[]) {
//     int arg_count = 0;
//     char *ptr = command_str;
    
//     while (*ptr && arg_count < MAX_ARGS - 1) {
//         // Saltar espacios en blanco
//         while (*ptr == ' ' || *ptr == '\t') ptr++;
        
//         if (*ptr == '\0') break;
        
//         char *arg_start = ptr;
        
//         // Si encontramos una comilla, buscar la comilla de cierre
//         if (*ptr == '"') {
//             ptr++; // Saltar la comilla de apertura
//             arg_start = ptr; // El argumento empieza después de la comilla
            
//             // Buscar la comilla de cierre
//             while (*ptr && *ptr != '"') ptr++;
            
//             if (*ptr == '"') {
//                 *ptr = '\0'; // Terminar la cadena en la comilla de cierre
//                 ptr++; // Avanzar después de la comilla
//             }
//         } else {
//             // Argumento normal, buscar el siguiente espacio
//             while (*ptr && *ptr != ' ' && *ptr != '\t') ptr++;
            
//             if (*ptr) {
//                 *ptr = '\0';
//                 ptr++;
//             }
//         }
        
//         args[arg_count++] = arg_start;
//     }
    
//     args[arg_count] = NULL;
//     return arg_count;
// }

// int main() {
//     char command[256];
//     char *commands[MAX_COMMANDS];
//     int command_count = 0;

//     while (1) 
//     {
//         // RESETEAR PARA CADA LÍNEA
//         command_count = 0;
        
//         //printf("Shell> ");
        
//         // Leer línea de comandos del usuario
//         fgets(command, sizeof(command), stdin);
        
//         // Remover el salto de línea
//         command[strcspn(command, "\n")] = '\0';
        
//         // Si el usuario presiona solo Enter, continuar
//         if (strlen(command) == 0) {
//             continue;
//         }
        
//         // Comando 'exit' para salir del shell
//         if (strcmp(command, "exit") == 0) {
//             //printf("Saliendo del shell...\n");
//             break;
//         }

//         // PASO 1: TOKENIZAR POR PIPES
//         // Separar comandos por el carácter '|'
//         char *token = strtok(command, "|");
//         while (token != NULL && command_count < MAX_COMMANDS) 
//         {
//             // Eliminar espacios al inicio y final de cada comando
//             while (*token == ' ' || *token == '\t') token++;  // Eliminar espacios al inicio
            
//             char *end = token + strlen(token) - 1;
//             while (end > token && (*end == ' ' || *end == '\t')) *end-- = '\0';  // Eliminar espacios al final
            
//             commands[command_count++] = token;
//             token = strtok(NULL, "|");
//         }
        
//         if (command_count == 0) {
//             continue;  // No hay comandos válidos
//         }
        
//         //printf("DEBUG - Total comandos encontrados: %d\n", command_count);
//         for (int i = 0; i < command_count; i++) {
//             //printf("DEBUG - Comando %d: '%s'\n", i, commands[i]);
//         }

//         // PASO 3: CREAR LOS PIPES NECESARIOS
//         int num_pipes = command_count - 1;  // N comandos = N-1 pipes
//         int pipes[MAX_COMMANDS-1][2];
        
//         // Crear todos los pipes necesarios
//         for (int i = 0; i < num_pipes; i++) {
//             if (pipe(pipes[i]) == -1) {
//                 perror("Error creando pipe");
//                 continue;  // Continuar con el siguiente comando
//             }
//             //printf("DEBUG - Pipe %d creado: lectura=%d, escritura=%d\n", 
//                    //i, pipes[i][0], pipes[i][1]);
//         }
        
//         //printf("DEBUG - Total pipes creados: %d\n", num_pipes);

//         // PASO 4: CREAR PROCESOS Y CONFIGURAR REDIRECCIONES
//         pid_t pids[MAX_COMMANDS];
        
//         for (int i = 0; i < command_count; i++) 
//         {
//             // PASO 2: PARSING DE ARGUMENTOS PARA CADA COMANDO
//             char *args[MAX_ARGS];
//             int arg_count = 0;
            
//             // Crear copia del comando para no modificar el original
//             char command_copy[256];
//             strcpy(command_copy, commands[i]);
            
//             // Usar la nueva función de parsing que maneja comillas
//             arg_count = parse_arguments(command_copy, args);
            
//             if (args[0] == NULL) {
//                 printf("ERROR - Comando %d vacío\n", i);
//                 continue;
//             }
            
//             //printf("DEBUG - Procesando comando %d: '%s' con %d argumentos\n", 
//                    //i, args[0], arg_count);
            
//             // CREAR PROCESO HIJO
//             pid_t pid = fork();
            
//             if (pid == -1) {
//                 perror("Error en fork");
//                 continue;
//             }
            
//             if (pid == 0) {
//                 // ========== CÓDIGO DEL PROCESO HIJO ==========
//                 //printf("DEBUG - Hijo %d: configurando redirecciones\n", i);
                
//                 // REDIRECCIÓN DE ENTRADA (stdin)
//                 // Si NO es el primer comando, leer del pipe anterior
//                 if (i > 0) {
//                     //printf("DEBUG - Hijo %d: redirigiendo stdin desde pipe[%d][0]=%d\n", 
//                            //i, i-1, pipes[i-1][0]);
//                     if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
//                         perror("Error en dup2 (stdin)");
//                         exit(1);
//                     }
//                 }
                
//                 // REDIRECCIÓN DE SALIDA (stdout)  
//                 // Si NO es el último comando, escribir al pipe siguiente
//                 if (i < command_count - 1) {
//                     //printf("DEBUG - Hijo %d: redirigiendo stdout hacia pipe[%d][1]=%d\n", 
//                            //i, i, pipes[i][1]);
//                     if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
//                         perror("Error en dup2 (stdout)");
//                         exit(1);
//                     }
//                 }
                
//                 // CERRAR TODOS LOS DESCRIPTORES DE PIPES EN EL HIJO
//                 for (int j = 0; j < num_pipes; j++) {
//                     close(pipes[j][0]);
//                     close(pipes[j][1]);
//                 }
                
//                 //printf("DEBUG - Hijo %d: ejecutando '%s'\n", i, args[0]);
                
//                 // EJECUTAR EL PROGRAMA
//                 execvp(args[0], args);
                
//                 // Si llegamos aquí, execvp() falló
//                 perror("Error en execvp");
//                 exit(127);  // Código estándar para "comando no encontrado"
                
//             } else {
//                 // ========== CÓDIGO DEL PROCESO PADRE ==========
//                 pids[i] = pid;
//                 //printf("DEBUG - Padre: creado hijo %d con PID %d para comando '%s'\n", 
//                        //i, pid, args[0]);
//             }
//         }

//         // CERRAR PIPES EN EL PROCESO PADRE
//         // ¡IMPORTANTE! Esto debe hacerse DESPUÉS de crear todos los hijos
//         // pero ANTES de esperar por ellos
//         for (int i = 0; i < num_pipes; i++) {
//             close(pipes[i][0]);
//             close(pipes[i][1]);
//             //printf("DEBUG - Padre: pipe %d cerrado\n", i);
//         }

//         // PASO 5: ESPERAR A QUE TERMINEN TODOS LOS PROCESOS HIJOS
//         //printf("DEBUG - Padre: esperando a que terminen todos los procesos hijos...\n");
        
//         for (int i = 0; i < command_count; i++) {
//             int status;
//             pid_t finished_pid = waitpid(pids[i], &status, 0);
            
//             if (finished_pid == -1) {
//                 perror("Error en waitpid");
//             } else {
//                 //printf("DEBUG - Proceso hijo PID %d (comando %d) terminó", finished_pid, i);
                
//                 if (WIFEXITED(status)) {
//                     int exit_code = WEXITSTATUS(status);
//                     //printf(" con código de salida: %d\n", exit_code);
                    
                    
//                     if (exit_code != 0) {
//                         //printf("ADVERTENCIA - El comando terminó con error (código %d)\n", exit_code);
//                     }
//                 } else if (WIFSIGNALED(status)) {
//                     int signal_num = WTERMSIG(status);
//                     //printf(" por señal: %d\n", signal_num);
//                 } else {
//                     //printf(" de forma anormal\n");
//                 }
//             }
//         }
        
//         //printf("DEBUG - Todos los procesos hijos han terminado\n");
//         //printf("========================================\n");
//     }
    
//     //printf("Shell terminado.\n");
//     return 0;
// }












// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <string.h>

// #define MAX_COMMANDS 200
// #define MAX_ARGS 50  // Máximo número de argumentos por comando

// int main() {
//     char command[256];
//     char *commands[MAX_COMMANDS];
//     int command_count = 0;

//     while (1) 
//     {
//         // RESETEAR PARA CADA LÍNEA
//         command_count = 0;
        
//         //printf("Shell> ");
        
//         // Leer línea de comandos del usuario
//         fgets(command, sizeof(command), stdin);
        
//         // Remover el salto de línea
//         command[strcspn(command, "\n")] = '\0';
        
//         // Si el usuario presiona solo Enter, continuar
//         if (strlen(command) == 0) {
//             continue;
//         }
        
//         // Comando 'exit' para salir del shell
//         if (strcmp(command, "exit") == 0) {
//             //printf("Saliendo del shell...\n");
//             break;
//         }

//         // PASO 1: TOKENIZAR POR PIPES
//         // Separar comandos por el carácter '|'
//         char *token = strtok(command, "|");
//         while (token != NULL && command_count < MAX_COMMANDS) 
//         {
//             // Eliminar espacios al inicio y final de cada comando
//             while (*token == ' ' || *token == '\t') token++;  // Eliminar espacios al inicio
            
//             char *end = token + strlen(token) - 1;
//             while (end > token && (*end == ' ' || *end == '\t')) *end-- = '\0';  // Eliminar espacios al final
            
//             commands[command_count++] = token;
//             token = strtok(NULL, "|");
//         }
        
//         if (command_count == 0) {
//             continue;  // No hay comandos válidos
//         }
        
//         //printf("DEBUG - Total comandos encontrados: %d\n", command_count);
//         for (int i = 0; i < command_count; i++) {
//             //printf("DEBUG - Comando %d: '%s'\n", i, commands[i]);
//         }

//         // PASO 3: CREAR LOS PIPES NECESARIOS
//         int num_pipes = command_count - 1;  // N comandos = N-1 pipes
//         int pipes[MAX_COMMANDS-1][2];
        
//         // Crear todos los pipes necesarios
//         for (int i = 0; i < num_pipes; i++) {
//             if (pipe(pipes[i]) == -1) {
//                 perror("Error creando pipe");
//                 continue;  // Continuar con el siguiente comando
//             }
//             //printf("DEBUG - Pipe %d creado: lectura=%d, escritura=%d\n", 
//                    //i, pipes[i][0], pipes[i][1]);
//         }
        
//         //printf("DEBUG - Total pipes creados: %d\n", num_pipes);

//         // PASO 4: CREAR PROCESOS Y CONFIGURAR REDIRECCIONES
//         pid_t pids[MAX_COMMANDS];
        
//         for (int i = 0; i < command_count; i++) 
//         {
//             // PASO 2: PARSING DE ARGUMENTOS PARA CADA COMANDO
//             char *args[MAX_ARGS];
//             int arg_count = 0;
            
//             // Crear copia del comando para no modificar el original
//             char command_copy[256];
//             strcpy(command_copy, commands[i]);
            
//             // Parsear argumentos separando por espacios
//             char *arg = strtok(command_copy, " \t");
//             while (arg != NULL && arg_count < MAX_ARGS - 1) 
//             {
//                 args[arg_count++] = arg;
//                 arg = strtok(NULL, " \t");
//             }
//             args[arg_count] = NULL;  // execvp requiere terminación NULL
            
//             if (args[0] == NULL) {
//                 printf("ERROR - Comando %d vacío\n", i);
//                 continue;
//             }
            
//             //printf("DEBUG - Procesando comando %d: '%s' con %d argumentos\n", 
//                    //i, args[0], arg_count);
            
//             // CREAR PROCESO HIJO
//             pid_t pid = fork();
            
//             if (pid == -1) {
//                 perror("Error en fork");
//                 continue;
//             }
            
//             if (pid == 0) {
//                 // ========== CÓDIGO DEL PROCESO HIJO ==========
//                 //printf("DEBUG - Hijo %d: configurando redirecciones\n", i);
                
//                 // REDIRECCIÓN DE ENTRADA (stdin)
//                 // Si NO es el primer comando, leer del pipe anterior
//                 if (i > 0) {
//                     //printf("DEBUG - Hijo %d: redirigiendo stdin desde pipe[%d][0]=%d\n", 
//                            //i, i-1, pipes[i-1][0]);
//                     if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
//                         perror("Error en dup2 (stdin)");
//                         exit(1);
//                     }
//                 }
                
//                 // REDIRECCIÓN DE SALIDA (stdout)  
//                 // Si NO es el último comando, escribir al pipe siguiente
//                 if (i < command_count - 1) {
//                     //printf("DEBUG - Hijo %d: redirigiendo stdout hacia pipe[%d][1]=%d\n", 
//                            //i, i, pipes[i][1]);
//                     if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
//                         perror("Error en dup2 (stdout)");
//                         exit(1);
//                     }
//                 }
                
//                 // CERRAR TODOS LOS DESCRIPTORES DE PIPES EN EL HIJO
//                 for (int j = 0; j < num_pipes; j++) {
//                     close(pipes[j][0]);
//                     close(pipes[j][1]);
//                 }
                
//                 //printf("DEBUG - Hijo %d: ejecutando '%s'\n", i, args[0]);
                
//                 // EJECUTAR EL PROGRAMA
//                 execvp(args[0], args);
                
//                 // Si llegamos aquí, execvp() falló
//                 perror("Error en execvp");
//                 exit(127);  // Código estándar para "comando no encontrado"
                
//             } else {
//                 // ========== CÓDIGO DEL PROCESO PADRE ==========
//                 pids[i] = pid;
//                 //printf("DEBUG - Padre: creado hijo %d con PID %d para comando '%s'\n", 
//                        //i, pid, args[0]);
//             }
//         }

//         // CERRAR PIPES EN EL PROCESO PADRE
//         // ¡IMPORTANTE! Esto debe hacerse DESPUÉS de crear todos los hijos
//         // pero ANTES de esperar por ellos
//         for (int i = 0; i < num_pipes; i++) {
//             close(pipes[i][0]);
//             close(pipes[i][1]);
//             //printf("DEBUG - Padre: pipe %d cerrado\n", i);
//         }

//         // PASO 5: ESPERAR A QUE TERMINEN TODOS LOS PROCESOS HIJOS
//         //printf("DEBUG - Padre: esperando a que terminen todos los procesos hijos...\n");
        
//         for (int i = 0; i < command_count; i++) {
//             int status;
//             pid_t finished_pid = waitpid(pids[i], &status, 0);
            
//             if (finished_pid == -1) {
//                 perror("Error en waitpid");
//             } else {
//                 //printf("DEBUG - Proceso hijo PID %d (comando %d) terminó", finished_pid, i);
                
//                 if (WIFEXITED(status)) {
//                     int exit_code = WEXITSTATUS(status);
//                     //printf(" con código de salida: %d\n", exit_code);
                    
                    
//                     if (exit_code != 0) {
//                         //printf("ADVERTENCIA - El comando terminó con error (código %d)\n", exit_code);
//                     }
//                 } else if (WIFSIGNALED(status)) {
//                     int signal_num = WTERMSIG(status);
//                     //printf(" por señal: %d\n", signal_num);
//                 } else {
//                     //printf(" de forma anormal\n");
//                 }
//             }
//         }
        
//         //printf("DEBUG - Todos los procesos hijos han terminado\n");
//         //printf("========================================\n");
//     }
    
//     //printf("Shell terminado.\n");
//     return 0;
// }




























































































// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <string.h>

// #define MAX_COMMANDS 200

// // ========== CÓDIGO NUEVO ==========
// #define MAX_ARGS 50  // Máximo número de argumentos por comando

// int main() {

//     char command[256];
//     char *commands[MAX_COMMANDS];
//     int command_count = 0;

//     while (1) 
//     {
//         // ========== CÓDIGO NUEVO - RESETEAR PARA CADA LÍNEA ==========
//         command_count = 0;  // ¡IMPORTANTE! Resetear contador para cada nueva línea
//         // ========== FIN CÓDIGO NUEVO ==========
        
//         printf("Shell> ");
        
//         /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
//         fgets(command, sizeof(command), stdin);
        
//         /* Removes the newline character (\n) from the end of the string stored in command, if present. 
//            This is done by replacing the newline character with the null character ('\0').
//            The strcspn() function returns the length of the initial segment of command that consists of 
//            characters not in the string specified in the second argument ("\n" in this case). */
//         command[strcspn(command, "\n")] = '\0';

//         /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
//            Each resulting token is stored in the commands[] array. 
//            The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
//            In each iteration of the while loop, strtok() returns the next token found in command. 
//            The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
//         char *token = strtok(command, "|");
//         while (token != NULL) 
//         {
//             commands[command_count++] = token;
//             token = strtok(NULL, "|");
            
//         }

//         /* You should start programming from here... */

//         // ========== PASO 3: CREAR LOS PIPES NECESARIOS ==========
        
//         // Calcular cuántos pipes necesitamos: N comandos = N-1 pipes
//         int num_pipes = command_count - 1;

//         // Array bidimensional para almacenar todos los pipes
//         // pipes[i][0] = descriptor de lectura del pipe i
//         // pipes[i][1] = descriptor de escritura del pipe i
//         int pipes[MAX_COMMANDS-1][2]; // ver si es mejor hacerlo dinámico
        
//         // Crear todos los pipes necesarios
//         for (int i = 0; i < num_pipes; i++) {
//             if (pipe(pipes[i]) == -1) {
//                 perror("Error creando pipe");
//                 exit(1);
//             }
//             printf("DEBUG - Pipe %d creado: lectura=%d, escritura=%d\n", 
//                    i, pipes[i][0], pipes[i][1]);
//         }
        
//         printf("DEBUG - Total pipes creados: %d para %d comandos\n", num_pipes, command_count);

        
//         // ========== FIN PASO 3 ==========

//         // ========== PASO 4: CREAR PROCESOS Y REDIRECCIONES ==========
        
//         // Array para almacenar los PIDs de los procesos hijos
//         pid_t pids[MAX_COMMANDS];
        
//         // Procesar cada comando: crear proceso hijo y configurar redirecciones
//         for (int i = 0; i < command_count; i++) 
//         {
//             // ========== PARSING DE ARGUMENTOS (del Paso 2) ==========
//             char *args[MAX_ARGS];
//             int arg_count = 0;
            
//             // Crear una copia del comando para no modificar el original
//             char command_copy[256];
//             strcpy(command_copy, commands[i]);
            
//             // Parsear argumentos
//             char *arg = strtok(command_copy, " \t");
//             while (arg != NULL && arg_count < MAX_ARGS - 1) 
//             {
//                 args[arg_count++] = arg;
//                 arg = strtok(NULL, " \t");
//             }
//             args[arg_count] = NULL;
            
//             printf("DEBUG - Procesando comando %d: %s\n", i, args[0]);
            
//             // ========== CREAR PROCESO HIJO ==========
//             pid_t pid = fork();
            
//             if (pid == -1) {
//                 perror("Error en fork");
//                 exit(1);
//             }
            
//             if (pid == 0) {
//                 // ========== CÓDIGO DEL PROCESO HIJO ==========
//                 printf("DEBUG - Hijo %d: configurando redirecciones\n", i);
                
//                 // REDIRECCIÓN DE ENTRADA (stdin)
//                 // Si NO es el primer comando, leer del pipe anterior
//                 if (i > 0) {
//                     printf("DEBUG - Hijo %d: redirigiendo stdin desde pipe[%d][0]=%d\n", 
//                            i, i-1, pipes[i-1][0]);
//                     if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
//                         perror("Error en dup2 (stdin)");
//                         exit(1);
//                     }
//                 }
                
//                 // REDIRECCIÓN DE SALIDA (stdout)  
//                 // Si NO es el último comando, escribir al pipe siguiente
//                 if (i < command_count - 1) {
//                     printf("DEBUG - Hijo %d: redirigiendo stdout hacia pipe[%d][1]=%d\n", 
//                            i, i, pipes[i][1]);
//                     if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
//                         perror("Error en dup2 (stdout)");
//                         exit(1);
//                     }
//                 }
                
//                 // CERRAR TODOS LOS DESCRIPTORES DE PIPES
//                 // ¡MUY IMPORTANTE! Cada hijo debe cerrar todos los pipes
//                 for (int j = 0; j < num_pipes; j++) {
//                     close(pipes[j][0]);
//                     close(pipes[j][1]);
//                 }
//                 printf("DEBUG - Hijo %d: pipes cerrados, ejecutando %s\n", i, args[0]);
                
//                 // EJECUTAR EL PROGRAMA
//                 execvp(args[0], args);
                
//                 // Si llegamos aquí, execvp() falló
//                 perror("Error en execvp");
//                 exit(1);
                
//             } else {
//                 // ========== CÓDIGO DEL PROCESO PADRE ==========
//                 pids[i] = pid;  // Guardar PID del hijo
//                 printf("DEBUG - Padre: creado hijo %d con PID %d para comando '%s'\n", 
//                        i, pid, args[0]);
//             }
//         }
        
//         // ========== FIN PASO 4 ==========
        
//         // ========== CÓDIGO NUEVO - PASO 2: PARSING DE ARGUMENTOS ==========
//         // Aquí procesaremos cada comando para separarlo en programa + argumentos
//         for (int i = 0; i < command_count; i++) 
//         {
//             // Array para almacenar los argumentos de cada comando individual
//             char *args[MAX_ARGS];
//             int arg_count = 0;
            
//             // Crear una copia del comando para no modificar el original con strtok
//             char command_copy[256];
//             strcpy(command_copy, commands[i]);
            
//             // Parsear el comando separando por espacios para obtener programa + argumentos
//             char *arg = strtok(command_copy, " \t"); // Separar por espacios y tabs
//             while (arg != NULL && arg_count < MAX_ARGS - 1) 
//             {
//                 args[arg_count++] = arg;
//                 arg = strtok(NULL, " \t");
//             }
//             args[arg_count] = NULL; // execvp() requiere que el array termine en NULL
            
//             // TEMPORALMENTE: Para verificar que el parsing funciona (QUITAR DESPUÉS)
//             printf("DEBUG - Command %d: %s -> Programa: %s\n", i, commands[i], args[0]);
            
//             // Aquí irá la lógica de pipes y ejecución (Pasos 3, 4, 5)
//             // TODO: Implementar creación de pipes, fork(), dup2(), execvp(), wait()
//         }

//         // ========== PASO 3: LIMPIAR PIPES (IMPORTANTE) ==========
//         // Cerrar todos los pipes en el proceso padre después del procesamiento
//         for (int i = 0; i < num_pipes; i++) {
//             close(pipes[i][0]);  // Cerrar descriptor de lectura
//             close(pipes[i][1]);  // Cerrar descriptor de escritura
//             printf("DEBUG - Pipe %d cerrado\n", i);
//         }
//         // ========== FIN PASO 3 ==========

//         // ========== PASO 5: ESPERAR PROCESOS HIJOS ==========
//         printf("DEBUG - Padre: esperando a que terminen todos los procesos hijos...\n");
        
//         // Esperar a que terminen todos los procesos hijos
//         for (int i = 0; i < command_count; i++) {
//             int status;
//             pid_t finished_pid = waitpid(pids[i], &status, 0);
            
//             if (finished_pid == -1) {
//                 perror("Error en waitpid");
//             } else {
//                 printf("DEBUG - Proceso hijo PID %d (comando %d) terminó", finished_pid, i);
                
//                 // Verificar cómo terminó el proceso
//                 if (WIFEXITED(status)) {
//                     int exit_code = WEXITSTATUS(status);
//                     printf(" con código de salida: %d\n", exit_code);
                    
//                     if (exit_code != 0) {
//                         printf("ADVERTENCIA - El comando terminó con error (código %d)\n", exit_code);
//                     }
//                 } else if (WIFSIGNALED(status)) {
//                     int signal_num = WTERMSIG(status);
//                     printf(" por señal: %d\n", signal_num);
//                 } else {
//                     printf(" de forma anormal\n");
//                 }
//             }
//         }
        
//         printf("DEBUG - Todos los procesos hijos han terminado\n");
//         printf("========================================\n");
//         // ========== FIN PASO 5 ==========

//         // ========== CÓDIGO NUEVO - RESET PARA SIGUIENTE ITERACIÓN ==========
//         command_count = 0; // Resetear contador para la siguiente línea de comandos
//         // ========== FIN CÓDIGO NUEVO ==========    
//     }
//     return 0;
// }