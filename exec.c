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
	if (i == startPos) {
		return -1;
	}
	return i;
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
		// printf("cmd[%d]: %s\n", j, cmd[j]);
		j++;
	}
	cmd[j] = NULL;
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
			// printf("progs[%d]: %s\n", i, *progs[i]);
		}
		progs[commands] = NULL;
		loop_exec(progs);
		while ( wait(NULL) != -1 );
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
}

void loop_exec(char ***progs) {
	int pfd[2];
	pid_t pid;
	int fd_in = 0;
	int count = 0;
	
	while ( progs[count] != NULL ) {
		if ( pipe(pfd) == -1 )
			error("Broken pipe");
		if ( (pid = fork()) == -1 ) {
			error("Fork failed");
		} else if ( pid == 0 ) {
			dup2(fd_in, 0);
			if ( progs[count + 1] != NULL ) {
				dup2( pfd[1], 1 );
			} 
			close(pfd[0]);
			execvp( progs[count][0], progs[count] );
			error("Exec failure");
		} else {
			while (wait(NULL) != -1);
			close(pfd[1]);
			fd_in = pfd[0];
			// close(pfd[0]);
			// progs++;
			count++;
		}
		// printf("progs address: %p\n", progs);
	}
	// progs = progs - count;
	free(progs);
}

void error(char* s) {
	extern int errno;

    fprintf( stderr, "%s\n", s );
    fprintf( stderr, " (%s)\n", strerror(errno) );
    exit( 1 );
}