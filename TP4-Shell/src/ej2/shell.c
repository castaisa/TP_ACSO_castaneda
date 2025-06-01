#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 50  // Máximo número de argumentos por comando

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) 
    {
        // RESETEAR PARA CADA LÍNEA
        command_count = 0;
        
        //printf("Shell> ");
        
        // Leer línea de comandos del usuario
        fgets(command, sizeof(command), stdin);
        
        // Remover el salto de línea
        command[strcspn(command, "\n")] = '\0';
        
        // Si el usuario presiona solo Enter, continuar
        if (strlen(command) == 0) {
            continue;
        }
        
        // Comando 'exit' para salir del shell
        if (strcmp(command, "exit") == 0) {
            //printf("Saliendo del shell...\n");
            break;
        }

        // PASO 1: TOKENIZAR POR PIPES
        // Separar comandos por el carácter '|'
        char *token = strtok(command, "|");
        while (token != NULL && command_count < MAX_COMMANDS) 
        {
            // Eliminar espacios al inicio y final de cada comando
            while (*token == ' ' || *token == '\t') token++;  // Eliminar espacios al inicio
            
            char *end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t')) *end-- = '\0';  // Eliminar espacios al final
            
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }
        
        if (command_count == 0) {
            continue;  // No hay comandos válidos
        }
        
        //printf("DEBUG - Total comandos encontrados: %d\n", command_count);
        for (int i = 0; i < command_count; i++) {
            //printf("DEBUG - Comando %d: '%s'\n", i, commands[i]);
        }

        // PASO 3: CREAR LOS PIPES NECESARIOS
        int num_pipes = command_count - 1;  // N comandos = N-1 pipes
        int pipes[MAX_COMMANDS-1][2];
        
        // Crear todos los pipes necesarios
        for (int i = 0; i < num_pipes; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("Error creando pipe");
                continue;  // Continuar con el siguiente comando
            }
            //printf("DEBUG - Pipe %d creado: lectura=%d, escritura=%d\n", 
                   //i, pipes[i][0], pipes[i][1]);
        }
        
        //printf("DEBUG - Total pipes creados: %d\n", num_pipes);

        // PASO 4: CREAR PROCESOS Y CONFIGURAR REDIRECCIONES
        pid_t pids[MAX_COMMANDS];
        
        for (int i = 0; i < command_count; i++) 
        {
            // PASO 2: PARSING DE ARGUMENTOS PARA CADA COMANDO
            char *args[MAX_ARGS];
            int arg_count = 0;
            
            // Crear copia del comando para no modificar el original
            char command_copy[256];
            strcpy(command_copy, commands[i]);
            
            // Parsear argumentos separando por espacios
            char *arg = strtok(command_copy, " \t");
            while (arg != NULL && arg_count < MAX_ARGS - 1) 
            {
                args[arg_count++] = arg;
                arg = strtok(NULL, " \t");
            }
            args[arg_count] = NULL;  // execvp requiere terminación NULL
            
            if (args[0] == NULL) {
                printf("ERROR - Comando %d vacío\n", i);
                continue;
            }
            
            //printf("DEBUG - Procesando comando %d: '%s' con %d argumentos\n", 
                   //i, args[0], arg_count);
            
            // CREAR PROCESO HIJO
            pid_t pid = fork();
            
            if (pid == -1) {
                perror("Error en fork");
                continue;
            }
            
            if (pid == 0) {
                // ========== CÓDIGO DEL PROCESO HIJO ==========
                //printf("DEBUG - Hijo %d: configurando redirecciones\n", i);
                
                // REDIRECCIÓN DE ENTRADA (stdin)
                // Si NO es el primer comando, leer del pipe anterior
                if (i > 0) {
                    //printf("DEBUG - Hijo %d: redirigiendo stdin desde pipe[%d][0]=%d\n", 
                           //i, i-1, pipes[i-1][0]);
                    if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                        perror("Error en dup2 (stdin)");
                        exit(1);
                    }
                }
                
                // REDIRECCIÓN DE SALIDA (stdout)  
                // Si NO es el último comando, escribir al pipe siguiente
                if (i < command_count - 1) {
                    printf("DEBUG - Hijo %d: redirigiendo stdout hacia pipe[%d][1]=%d\n", 
                           i, i, pipes[i][1]);
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
                
                //printf("DEBUG - Hijo %d: ejecutando '%s'\n", i, args[0]);
                
                // EJECUTAR EL PROGRAMA
                execvp(args[0], args);
                
                // Si llegamos aquí, execvp() falló
                perror("Error en execvp");
                exit(127);  // Código estándar para "comando no encontrado"
                
            } else {
                // ========== CÓDIGO DEL PROCESO PADRE ==========
                pids[i] = pid;
                //printf("DEBUG - Padre: creado hijo %d con PID %d para comando '%s'\n", 
                       //i, pid, args[0]);
            }
        }

        // CERRAR PIPES EN EL PROCESO PADRE
        // ¡IMPORTANTE! Esto debe hacerse DESPUÉS de crear todos los hijos
        // pero ANTES de esperar por ellos
        for (int i = 0; i < num_pipes; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
            //printf("DEBUG - Padre: pipe %d cerrado\n", i);
        }

        // PASO 5: ESPERAR A QUE TERMINEN TODOS LOS PROCESOS HIJOS
        //printf("DEBUG - Padre: esperando a que terminen todos los procesos hijos...\n");
        
        for (int i = 0; i < command_count; i++) {
            int status;
            pid_t finished_pid = waitpid(pids[i], &status, 0);
            
            if (finished_pid == -1) {
                perror("Error en waitpid");
            } else {
                //printf("DEBUG - Proceso hijo PID %d (comando %d) terminó", finished_pid, i);
                
                if (WIFEXITED(status)) {
                    int exit_code = WEXITSTATUS(status);
                    //printf(" con código de salida: %d\n", exit_code);
                    
                    
                    if (exit_code != 0) {
                        //printf("ADVERTENCIA - El comando terminó con error (código %d)\n", exit_code);
                    }
                } else if (WIFSIGNALED(status)) {
                    int signal_num = WTERMSIG(status);
                    //printf(" por señal: %d\n", signal_num);
                } else {
                    //printf(" de forma anormal\n");
                }
            }
        }
        
        //printf("DEBUG - Todos los procesos hijos han terminado\n");
        //printf("========================================\n");
    }
    
    //printf("Shell terminado.\n");
    return 0;
}




























































































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