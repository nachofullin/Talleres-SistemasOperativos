#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "constants.h"
#include <signal.h>

int generate_random_number(){
	return (rand() % 50);
}

int main(int argc, char **argv) {	
	//Funcion para cargar nueva semilla para el numero aleatorio
	srand(time(NULL));

	int status, pid, n, start, buffer;
	n = atoi(argv[1]);
	buffer = atoi(argv[2]);
	start = atoi(argv[3]);

	if (argc != 4) { 
		printf("Uso: anillo <n> <c> <s> \n"); 
		exit(0);
	}
    
  	/* COMPLETAR */
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer, start);
	// Crear pipe resultado para comunicación hijo -> padre
 	int pipe_resultado[2];
	pipe(pipe_resultado);
    // Creo pipes para anillo
	int pipes[n][2];
	for (int i = 0; i < n; i++) {
		if (pipe(pipes[i]) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}
	// Creo los hijos
	pid_t* children = malloc(n * sizeof(pid_t));
	
	for	(int i = 0; i < n; i++) {
		children[i] = fork();
		int numero_secreto, mensaje;
		if (children[i] == 0) {
			// // Cerrar pipes no utilizados
            /*for (int j = 0; j < n; j++) {
                if (j != i) close(pipes[j][PIPE_WRITE]);
                if (j != (i + n - 1) % n) close(pipes[j][PIPE_READ]);
            }*/
			// Si estoy en el proceso inicial creo el num secreto
			if (i == start) {
				numero_secreto = generate_random_number();
				printf("Proceso %d creo el numero secreto: %d\n", i, numero_secreto);
				write(pipes[(i + n - 1) % n][PIPE_WRITE], &buffer, sizeof(buffer));
			}

			mensaje = buffer;
			while (1) {
				read(pipes[(i + n - 1) % n][PIPE_READ], &mensaje, sizeof(mensaje));
				printf("Proceso %d leyo el mensaje: %d\n", i, mensaje);

				if (mensaje >= numero_secreto && i == start) {
					printf("Proceso %d: el mensaje (%d) es mayor o igual a numero secreto %d\n", i, mensaje, numero_secreto);
					write(pipe_resultado[PIPE_WRITE], &mensaje, sizeof(mensaje));
					break;
				}
				
				mensaje++;
				
				write(pipes[i][PIPE_WRITE], &mensaje, sizeof(mensaje));
				printf("Proceso %d envio mensaje %d\n", i, mensaje);
			}
			/*close(pipes[(i + n - 1) % n][PIPE_READ]);
			close(pipes[i][PIPE_WRITE]);*/

			exit(EXIT_SUCCESS);
		} else if (pid == -1) {
			exit(EXIT_FAILURE);
		}
	}
	int resultado;
	//close(pipe_resultado[PIPE_WRITE]);
	read(pipe_resultado[PIPE_READ], &resultado, sizeof(resultado));

	printf("Resultado final: %d\n", resultado);

	for (int i = 0; i < n; i++)
	{
		if (i != start)
		{
			kill(children[i], SIGKILL);
		}
	}
	
	free(children);

	// Esperar a que los procesos hijos terminen
    /*for (int i = 0; i < n; i++) {
        wait(NULL);
    }*/

    return 0;
}
