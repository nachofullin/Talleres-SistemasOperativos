#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"
#include "mini-shell-parser.c"

typedef char*** matrix;

/*
	Explicacion del argumento progs:

	Corresponde con una matriz donde cada fila representa el iesimo programa pasado
	dividido por el pipe. En cada fila esta el nombre del programa 
	en su primera posicion y los parametros en las posiciones restantes.

	Ejemplo: 'du -h /home | head -n 10 | sort -rh'
	
	Representacion:
	[
		['du', '-h', '/home'],
		['head', '-n', '10'],
		['sort', '-rh']
	]

*/

void ejecutarPrograma(int i, size_t count, matrix progs, int pipes[][2]){
	if (i == 0) {
		// Cerrar todos los descriptores de pipes en el hijo
		for (size_t j = 0; j < count-1; j++) {
			if (j != i){
				close(pipes[j][PIPE_READ]);
				close(pipes[j][PIPE_WRITE]);
			}
		}
		close(pipes[i][PIPE_READ]);
  		// Conectar escritura a stdout
  		dup2(pipes[i][PIPE_WRITE], STD_OUTPUT);
	}
	else if (i == count-1) {
		for (size_t j = 0; j < count-1; j++) {
			if (j != i-1){
				close(pipes[j][PIPE_READ]);
				close(pipes[j][PIPE_WRITE]);
			}
		}
		close(pipes[i-1][PIPE_WRITE]);
  		// Conectar lectura a stdin
  		dup2(pipes[i-1][PIPE_READ], STD_INPUT);
	}
	else {
		for (size_t j = 0; j < count-1; j++) {
			if ((j != i-1) || (j != i)){
				close(pipes[j][PIPE_READ]);
				close(pipes[j][PIPE_WRITE]);
			}
		}
		close(pipes[i-1][PIPE_WRITE]);
        dup2(pipes[i-1][PIPE_READ], STD_INPUT);
        close(pipes[i][PIPE_READ]);
        dup2(pipes[i][PIPE_WRITE], STD_OUTPUT);
	}
	// Ejecutar programa
	execvp(progs[i][0], progs[i]);
}

static int run(matrix progs, size_t count)
{	
	int r, status;

	//Reservo memoria para el arreglo de pids
	//TODO: Guardar el PID de cada proceso hijo creado en children[i]
	pid_t *children = malloc(sizeof(*children) * count);

	//TODO: Pensar cuantos procesos necesito = cant de prog
	//TODO: Pensar cuantos pipes necesito. = cant de prog -1
	int pipes[count-1][2];
  	for (int i = 0; i < count-1; i++) {
    	pipe(pipes[i]);
  	}	

	for	(size_t i = 0; i < count; i++) {
		children[i] = fork();
		if (children[i] == 0) {
			ejecutarPrograma(i, count, progs, pipes);
		}
	}

	for (int i = 0; i < count-1; i++) {
		close(pipes[i][PIPE_WRITE]);
		close(pipes[i][PIPE_READ]);
	}
	//TODO: Para cada proceso hijo:
			//1. Redireccionar los file descriptors adecuados al proceso
			//2. Ejecutar el programa correspondiente

	//Espero a los hijos y verifico el estado que terminaron
	for (int i = 0; i < count; i++) {
		waitpid(children[i], &status, 0);

		if (!WIFEXITED(status)) {
			fprintf(stderr, "proceso %d no terminó correctamente [%d]: ",
			    (int)children[i], WIFSIGNALED(status));
			perror("");
			return -1;
		}
	}
	r = 0;
	free(children);

	return r;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("El programa recibe como parametro de entrada un string con la linea de comandos a ejecutar. \n"); 
		printf("Por ejemplo ./mini-shell 'ls -a | grep anillo'\n");
		return 0;
	}
	int programs_count;
	matrix programs_with_parameters = parse_input(argv, &programs_count);

	printf("status: %d\n", run(programs_with_parameters, programs_count));

	fflush(stdout);
	fflush(stderr);

	return 0;
}
