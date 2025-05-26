#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{	
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
  	/* TO COMPLETE */
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
   	/* You should start programming from here... */
	   int i;
    
	   if (n < 3 || start < 1 || start > n) {
		   fprintf(stderr, "Error: n debe ser >= 3 y el proceso de inicio debe estar entre 1 y n.\n");
		   exit(EXIT_FAILURE);
	   }
	   
	   // Crear un array de pipes (uno por cada proceso)
	   int **pipes = (int **)malloc(n * sizeof(int *));
	   for (i = 0; i < n; i++) {
		   pipes[i] = (int *)malloc(2 * sizeof(int));
		   if (pipe(pipes[i]) < 0) {
			   perror("Error creando pipe");
			   exit(EXIT_FAILURE);
		   }
	   }
	   
	   // Crear un pipe adicional para que el último proceso envíe el resultado al padre
	   int final_pipe[2];
	   if (pipe(final_pipe) < 0) {
		   perror("Error creando pipe final");
		   exit(EXIT_FAILURE);
	   }
	   
	   // Crear los procesos hijos
	   for (i = 0; i < n; i++) {
		   pid = fork();
		   if (pid < 0) {
			   perror("Error en fork");
			   exit(EXIT_FAILURE);
		   }
		   
		   if (pid == 0) { // Proceso hijo
			   int my_id = i + 1;  // ID del proceso actual (1-based)
			   int prev_to_start = (start == 1) ? n : start - 1;
			   int value;
			   
			   // Cerrar todos los pipes excepto los que necesita este proceso
			   for (int j = 0; j < n; j++) {
				   if (j == i) {
					   // Este proceso lee de su propio pipe
					   close(pipes[j][1]); // Cierra el extremo de escritura
				   } else if (j == (i + 1) % n) {
					   // Este proceso escribe al pipe del siguiente
					   close(pipes[j][0]); // Cierra el extremo de lectura
				   } else {
					   // Cierra ambos extremos de los otros pipes
					   close(pipes[j][0]);
					   close(pipes[j][1]);
				   }
			   }
			   
			   // Solo el proceso anterior al inicial usa el pipe final
			   if (my_id != prev_to_start) {
				   close(final_pipe[0]);
				   close(final_pipe[1]);
			   } else {
				   close(final_pipe[0]); // Solo cierra la lectura, mantiene la escritura
			   }
			   
			   // Leer el valor
			   if (read(pipes[i][0], &value, sizeof(int)) < 0) {
				   perror("Error leyendo valor");
				   exit(EXIT_FAILURE);
			   }
			   
			   // Incrementar el valor
			   value++;
			   
			   // Si este es el proceso anterior al inicial, enviar el resultado al padre
			   if (my_id == prev_to_start) {
				   if (write(final_pipe[1], &value, sizeof(int)) < 0) {
					   perror("Error enviando resultado al padre");
					   exit(EXIT_FAILURE);
				   }
				   close(final_pipe[1]);
			   } else {
				   // Enviar al siguiente proceso en el anillo
				   int next = (i + 1) % n;
				   if (write(pipes[next][1], &value, sizeof(int)) < 0) {
					   perror("Error enviando valor al siguiente");
					   exit(EXIT_FAILURE);
				   }
				   close(pipes[next][1]);
			   }
			   
			   // Cerrar los pipes restantes
			   close(pipes[i][0]);
			   
			   exit(0);
		   }
	   }
	   
	   // Proceso padre
	   
	   // Cerrar los extremos de los pipes que no se utilizan
	   for (i = 0; i < n; i++) {
		   if (i == start - 1) {
			   // El pipe del proceso inicial recibe del padre
			   close(pipes[i][0]); // El padre no lee de este pipe
		   } else {
			   // Cerrar ambos extremos de los otros pipes
			   close(pipes[i][0]);
			   close(pipes[i][1]);
		   }
	   }
	   
	   // Cerrar el extremo de escritura del pipe final
	   close(final_pipe[1]);
	   
	   // Enviar el valor inicial al proceso de inicio
	   if (write(pipes[start - 1][1], &buffer[0], sizeof(int)) < 0) {
		   perror("Error enviando valor inicial");
		   exit(EXIT_FAILURE);
	   }
	   
	   // Cerrar el pipe de escritura después de usarlo
	   close(pipes[start - 1][1]);
	   
	   // Leer el resultado final
	   int result;
	   if (read(final_pipe[0], &result, sizeof(int)) < 0) {
		   perror("Error leyendo resultado final");
		   exit(EXIT_FAILURE);
	   }
	   
	   // Cerrar el pipe final
	   close(final_pipe[0]);
	   
	   // Mostrar el resultado
	   printf("El resultado final es: %d\n", result);
	   
	   // Esperar a que todos los hijos terminen
	   for (i = 0; i < n; i++) {
		   wait(&status);
	   }
	   
	   // Liberar memoria
	   for (i = 0; i < n; i++) {
		   free(pipes[i]);
	   }
	   free(pipes);
	   
	   return 0;
}
