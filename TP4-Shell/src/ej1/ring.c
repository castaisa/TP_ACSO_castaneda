#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char **argv)
{	
    int start, status, pid, n;
    int buffer[1];
    
    if (argc != 4){ 
        fprintf(stderr, "Uso: anillo <n> <c> <s> \n"); 
        exit(1);
    }
    
    char *endptr;
    
    // Convertir n
    errno = 0;
    long temp_n = strtol(argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || temp_n < 3 || temp_n > INT_MAX) {
        fprintf(stderr, "Error: el valor ingresado no es valido para n\n");
        exit(1);
    }
    n = (int)temp_n;
    
    //Convertir c
    errno = 0;
    long temp_buffer = strtol(argv[2], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || temp_buffer < INT_MIN || temp_buffer > INT_MAX) {
        fprintf(stderr, "Error: c debe ser un número entero válido\n");
        exit(1);
    }
    buffer[0] = (int)temp_buffer;
    
    // Convertir s
    errno = 0;
    long temp_start = strtol(argv[3], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || temp_start < 1 || temp_start > n) {
        fprintf(stderr, "Error: s debe ser un número entero en el rango [1, %d]\n", n);
        exit(1);
    }
    start = (int)temp_start;
    
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);

    
    //Crear n pipes para el ring + 1 pipe para devolver resultado al padre
    int ring_pipes[n][2];
    int result_pipe[2]; 
    
    //Crear pipes del ring
    for (int i = 0; i < n; i++) {
        if (pipe(ring_pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    
    //Crear pipe para resultado final
    if (pipe(result_pipe) == -1) {
        perror("pipe");
        exit(1);
    }
    
    //Crear n procesos hijos
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) {
            // Proceso hijo i
            
            //Cerrar pipes que no necesita
            for (int j = 0; j < n; j++) {
                if (j == i) {
                    close(ring_pipes[j][1]);
                } else if (j == (i + 1) % n) {
                    close(ring_pipes[j][0]);
                } else {
                    close(ring_pipes[j][0]);
                    close(ring_pipes[j][1]);
                }
            }
            
            // Cerrar pipe de resultado (solo lo usa el último proceso que completa el ring)
            close(result_pipe[0]);
            
            //Leer mensaje del pipe de entrada
            int mensaje;
            ssize_t bytes_read = read(ring_pipes[i][0], &mensaje, sizeof(int));
            if (bytes_read != sizeof(int)) {
                perror("read");
                exit(1);
            }
            close(ring_pipes[i][0]);
            
        
            mensaje++;
            
            //Determinar si este proceso completa el ring
            int next_process = (i + 1) % n;
            if (next_process == start - 1) {
                //El mensaje volvió al proceso inicial: enviar resultado final al padre
                ssize_t bytes_written = write(result_pipe[1], &mensaje, sizeof(int));
                if (bytes_written != sizeof(int)) {
                    perror("write");
                    exit(1);
                }
                close(result_pipe[1]);
            } else {
                //Enviar al siguiente proceso en el ring
                ssize_t bytes_written = write(ring_pipes[next_process][1], &mensaje, sizeof(int));
                if (bytes_written != sizeof(int)) {
                    perror("write");
                    exit(1);
                }
                close(ring_pipes[next_process][1]);
                close(result_pipe[1]);
            }
            
            exit(0);
        } else if (pid == -1) {
            perror("fork");
            exit(1);
        }
    }
    
    // Proceso padre
    
    //Cerrar todos los pipes del ring excepto el de escritura al proceso inicial
    for (int i = 0; i < n; i++) {
        if (i == start - 1) {
            //Mantener escritura al proceso inicial
            close(ring_pipes[i][0]);
        } else {
            close(ring_pipes[i][0]);
            close(ring_pipes[i][1]);
        }
    }
    
    //Cerrar escritura del pipe de resultado
    close(result_pipe[1]);
    
    ssize_t bytes_written = write(ring_pipes[start - 1][1], &buffer[0], sizeof(int));
    if (bytes_written != sizeof(int)) {
        perror("write");
        exit(1);
    }
    close(ring_pipes[start - 1][1]);
    
    //Recibir resultado final
    int resultado_final;
    ssize_t bytes_read = read(result_pipe[0], &resultado_final, sizeof(int));
    if (bytes_read != sizeof(int)) {
        perror("read");
        exit(1);
    }
    close(result_pipe[0]);
    
    printf("Resultado final: %d\n", resultado_final);
    
    //Esperar a todos los procesos hijos
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    
    return 0;
}