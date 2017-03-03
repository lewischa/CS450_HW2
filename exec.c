#include "exec.h"
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

int isPiped(char **line, int startPos) {
	int i = startPos;
	while (line[i] != '\0') {
		if (strcmp(line[i], "|") == 0) {
			return i;
		}
		i++;
	}
	return -1;
}


int isRedirect(char **line, int startPos) {
	int i = startPos;
	while (line[i] != '\0') {
		if (strcmp(line[i], "<") == 0 || strcmp(line[i], ">") == 0) {
			return i;
		}
		i++;
	}
	return -1;
}

void exec(char **line) {
	pid_t pid;

	int piped = isPiped(line, 0);
	int redirected = isRedirect(line, 0);
	char** args;

	if (piped > 0) {
		args = malloc(piped * sizeof(char));
		for (int i = 0; i < piped; i++) {
			args[i] = line[i];
		}
		args[piped] = 0;
	} else {
		args = line;
	}

	switch( pid = fork() ) {
		case -1:
			printf("Fork failed.");
			exit(1);
		case 0:
			execvp(line[0], args);
	}
}