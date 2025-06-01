#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
// ver que corran bien los procesos en paralelo
// que se pase bien la informacion
int main(int argc, char **argv)
{	
    int start, status, pid, n;
    int buffer[1];
    
    if (argc != 4){ 
        printf("Uso: anillo <n> <c> <s> \n"); 
        exit(1);  // Cambiar exit(0) por exit(1) para indicar error
    }
    
    /* Parsing of arguments con mejor manejo de números negativos */
    char *endptr;
    
    // Convertir n con validación
    errno = 0;
    long temp_n = strtol(argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || temp_n <= 0 || temp_n > INT_MAX) {
        fprintf(stderr, "Error: n debe ser un número entero positivo\n");
        exit(1);
    }
    n = (int)temp_n;
    
    // Convertir valor inicial con mejor manejo de negativos
    errno = 0;
    long temp_buffer = strtol(argv[2], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || temp_buffer < INT_MIN || temp_buffer > INT_MAX) {
        fprintf(stderr, "Error: c debe ser un número entero válido\n");
        exit(1);
    }
    buffer[0] = (int)temp_buffer;  // Conversión explícita
    
    // Convertir start
    errno = 0;
    long temp_start = strtol(argv[3], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || temp_start < 0 || temp_start > INT_MAX) {
        fprintf(stderr, "Error: s debe ser un número entero no negativo\n");
        exit(1);
    }
    start = (int)temp_start;
    
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
    /* You should start programming from here... */
    
    // Crear n+1 pipes: n para el anillo + 1 para que el último hijo devuelva al padre
    int pipes[n+1][2];
    for (int i = 0; i <= n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    
    // Crear n procesos hijos
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) {
            // Proceso hijo i
            // Cerrar todos los descriptores de pipes excepto los que necesita
            for (int j = 0; j <= n; j++) {
                if (j == i) {
                    // Pipe de entrada: lee de pipes[i][0]
                    close(pipes[i][1]);
                } else if (j == (i + 1) % n) {
                    // Pipe de salida al siguiente hijo: escribe a pipes[(i+1)%n][1]
                    close(pipes[j][0]);
                } else if (i == n - 1 && j == n) {
                    // El último hijo (n-1) escribe al padre por pipes[n][1]
                    close(pipes[j][0]);
                } else {
                    // Cerrar pipes que no usa
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            
            // Leer mensaje del proceso anterior
            int mensaje;
            ssize_t bytes_read = read(pipes[i][0], &mensaje, sizeof(int));
            if (bytes_read != sizeof(int)) {
                perror("read");
                exit(1);
            }
            
            // Incrementar el mensaje
            mensaje++;
            
            // Si es el último proceso, enviar de vuelta al padre
            if (i == n - 1) {
                ssize_t bytes_written = write(pipes[n][1], &mensaje, sizeof(int));
                if (bytes_written != sizeof(int)) {
                    perror("write");
                    exit(1);
                }
                close(pipes[n][1]);
            } else {
                // Enviar mensaje al siguiente proceso
                ssize_t bytes_written = write(pipes[(i + 1) % n][1], &mensaje, sizeof(int));
                if (bytes_written != sizeof(int)) {
                    perror("write");
                    exit(1);
                }
                close(pipes[(i + 1) % n][1]);
            }
            
            // Cerrar el pipe de entrada
            close(pipes[i][0]);
            
            exit(0);
        } else if (pid == -1) {
            perror("fork");
            exit(1);
        }
    }
    
    // Proceso padre: cerrar descriptores innecesarios
    for (int i = 0; i <= n; i++) {
        if (i == 0) {
            // Mantener pipes[0][1] para enviar mensaje inicial
            close(pipes[i][0]);
        } else if (i == n) {
            // Mantener pipes[n][0] para recibir resultado final
            close(pipes[i][1]);
        } else {
            // Cerrar todos los otros pipes
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }
    
    // Enviar mensaje inicial al primer proceso
    ssize_t bytes_written = write(pipes[0][1], &buffer[0], sizeof(int));
    if (bytes_written != sizeof(int)) {
        perror("write");
        exit(1);
    }
    close(pipes[0][1]);
    
    // Recibir resultado final del último proceso
    int resultado_final;
    ssize_t bytes_read = read(pipes[n][0], &resultado_final, sizeof(int));
    if (bytes_read != sizeof(int)) {
        perror("read");
        exit(1);
    }
    close(pipes[n][0]);
    
    // Mostrar el resultado final
    printf("Resultado final: %d\n", resultado_final);
    
    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    
    return 0;
}