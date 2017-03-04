#include "exec.h"
#include "constants.h"
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

int pipePos(char **line, int startPos) {
	int i = startPos;
	while (line[i] != '\0') {
		if (strcmp(line[i], "|") == 0) {
			return i;
		}
		i++;
	}
	return -1;
}


int redirectPos(char **line, int startPos) {
	int i = startPos;
	while (line[i] != '\0') {
		if (strcmp(line[i], "<") == 0 || strcmp(line[i], ">") == 0) {
			return i;
		}
		i++;
	}
	return -1;
}

int numCommands(char **line, int *pipes, int *redirects) {
	if (line == NULL) {
		return 0;
	}
	int i = 0;
	int commands = 1;

	while (line[i] != '\0') {
		if ( strcmp(line[i], "|") == 0 ) {
			commands++;
			(*pipes)++;
		}
		if ( strcmp(line[i], "<") == 0 || strcmp(line[i], ">") == 0 ) {
			commands++;
			(*redirects)++;
		}
		i++;
	}
	return commands;
}


// if (piped > 0) {
//         prog1 = malloc(piped * sizeof(char));
//         for (int i = 0; i < piped; i++) {
//             prog1[i] = line[i];
//         }
//         prog1[piped] = 0;
//     } else {
//         prog1 = line;
//     }

char** getCommand(char** line, int startPos, int endPos) {
	char** cmd = malloc( (endPos - startPos) * sizeof(char*));
	int j = 0;
	for ( int i = startPos; i < endPos; i++ ) {
		// printf("line[%d]: %s\n", i, line[i]);
		cmd[j] = line[i];
		j++;
	}
	cmd[j] = 0;
	// printf("cmd: %s\n", *cmd);
	return cmd;
}



void exec(char **line) {
	// printf("Printing line in exec: %s\n", line[0]);
	// printf("printing line in exec: %s\n", line[1]);
	// printf("printing line in exec: %s\n", line[2]);
	// printf("Printing line in exec: %s\n", line[3]);
	// printf("Printing line in exec: %s\n", line[4]);
	pid_t pid;

	int numPipes = 0;
	int *p = &numPipes;
	int numRedirects = 0;
	int *r = &numRedirects;
	int commands = numCommands(line, p, r);
	char*** progs = malloc(commands * sizeof(char**));
	int _numProgs = 0;

	
	if ( commands > 1 ) {
		int nextPipePosition = 0;
		int lastPipePosition = -1;
		for ( int i = 0; i < commands; i++ ) {
			if ( i == 0 ) {
				nextPipePosition = pipePos(line, 0);
			} else {
				lastPipePosition = nextPipePosition;
				nextPipePosition = pipePos(line, lastPipePosition + 1);
			}
			if ( nextPipePosition == -1 ) {
				nextPipePosition = strlen(*line);
			}
			progs[i] = getCommand(line, lastPipePosition + 1, nextPipePosition);
			_numProgs++;
			// printf("progs[%d]: %s\n", i, *progs[i]);
		}
		progs[_numProgs + 1] = NULL;
		loop_exec(progs, _numProgs);
	} else {
		progs[0] = line;
		switch( pid = fork() ) {
			case -1:
				error("Fork failed.");
			case 0:
				if ( execvp(progs[0][0], progs[0]) == -1) {
					error("Exec failed");
				}
		}
	}

	// execvp(*progs[0], progs[0]);

	// Debug printing:
	// for (int i = 0; i < commands; i++) {
	// 	printf("in debug printing for loop\n");
	// 	printf("prooggggggs: %s\n", *progs[i]);
	// }

	// printf("Num pipes: %d\n", numPipes);
	// printf("Num redirects: %d\n", numRedirects);
	// printf("Num commands: %d\n", commands);

	// if (pipes > 0) {
		
	// } else {
	// 	prog1 = line;
	// }

	// switch( pid = fork() ) {
	// 	case -1:
	// 		error("Fork failed.");
	// 	case 0:
	// 		if ( execvp(line[0], prog1) == -1) {
	// 			char *err = "Exec failed on %s", prog1;
	// 			error(err);
	// 		}
	// }
}

void loop_exec(char ***progs, int n) {
	// int pfd[2];
	// pid_t pid;
	// int fd_in = 0;
	
}

void error(char* s) {
	extern int errno;

    fprintf( stderr, "%s\n", s );
    fprintf( stderr, " (%s)\n", strerror(errno) );
    exit( 1 );
}