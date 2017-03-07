#include "exec.h"
#include "constants.h"
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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


void redirectPos(struct command *cmd, int index) {
	// printf("in redirectPos\n");
	int i = 0; 
	while ( cmd[index].args[i] != '\0' ) {
		if ( strcmp(cmd[index].args[i], "<") == 0 ) {
			// printf("< is at index %d\n", i);
			*cmd[index].lessThan = i;
		}
		if ( strcmp(cmd[index].args[i], ">") == 0 ) {
			// printf("> is at index %d\n", i);
			*cmd[index].greaterThan = i;
		}
		i++;
	}
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


int isMeta(char* c) {
    // printf("isMeta: %s\n", c);
    if ( strcmp(c, "|") == 0 ) {
        return 1;
    } 
    return 0;
}

void exec(char** line_words, int num_words) {
	struct command *cmd = malloc(num_words * sizeof(*cmd));
    cmd[0].args = malloc(MAX_LINE_WORDS * sizeof(char*));
    cmd[0].lessThan = malloc(sizeof(int*));
    cmd[0].greaterThan = malloc(sizeof(int*));
    *cmd[0].lessThan = -1;
    *cmd[0].greaterThan = -1;
    int cmdNumber = 0;
    int argNumber = 0;
    for ( int i = 0; i < num_words; i++ ) {
        // printf("%lu\n", sizeof(cmd[cmdNumber].args));
        if ( isMeta(line_words[i]) ) {
            cmd[cmdNumber].args[argNumber] = NULL;
            argNumber = 0;
            redirectPos(cmd, cmdNumber);
            cmdNumber++;
            cmd[cmdNumber].args = malloc(MAX_LINE_WORDS * sizeof(char*));
            cmd[cmdNumber].lessThan = malloc(sizeof(int*));
    		cmd[cmdNumber].greaterThan = malloc(sizeof(int*));
            *cmd[cmdNumber].lessThan = -1;
            *cmd[cmdNumber].greaterThan = -1;
        } else {
        	cmd[cmdNumber].args[argNumber] = malloc(strlen(line_words[i]) * sizeof(char) + 1);
        	strcpy(cmd[cmdNumber].args[argNumber], line_words[i]);
            // cmd[cmdNumber].args[argNumber] = line_words[i];
            argNumber++;
            if ( (i + 1) == num_words ) {
                cmd[cmdNumber].args[argNumber] = NULL;
                redirectPos(cmd, cmdNumber);
            }
        }
    }

    if ( num_words > 0 ) {
        cmd[++cmdNumber].args = NULL;
        // printf("cmdNumber: %d\n", cmdNumber);
    } else {
        cmd[0].args = NULL;
    }

    // for ( int i = 0; i < cmdNumber; i++ ) {
    //     printf("cmd[%d]: ", i);
    //     int j = 0;
    //     // while ( cmd[i].args[j] != NULL ) {
    //     //     printf("%s ", cmd[i].args[j]);
    //     //     j++;
    //     // }
    //     printf("\n");
    //     printf("cmd lessThan: %d; cmd greaterThan: %d\n", *cmd[i].lessThan, *cmd[i].greaterThan);
    // }






    /*
    UNCOMMENT THE LINE BELOW (AND COMMENT OUT exec_test(cmd);) TO RUN WITH REDIRECTION ON 1 COMMAND ONLY
    */
    // single_exec(cmd);
    exec_test(cmd);

    freeMemory(cmd, cmdNumber);
}

void freeMemory(struct command *cmd, int cmdNumber){
    for ( int i = 0; i < cmdNumber; i++ ) {
    	for ( int j = 0; cmd[i].args[j] != NULL; j++ ) {
    		free(cmd[i].args[j]); cmd[i].args[j] = NULL;
    	}
		free(cmd[i].args); cmd[i].args = NULL;
    	free(cmd[i].lessThan); cmd[i].lessThan = NULL;
    	free(cmd[i].greaterThan); cmd[i].greaterThan = NULL;     
    }

    if ( cmd ) {
    	free(cmd); cmd = NULL;
    }
}

void single_exec(struct command *cmd) {
	pid_t pid;
	int fd_in = 0;
	int fd_out = 1;
	int num_cmd_words = 0;
	// printf("before dereference\n");
	int input_redirect = *cmd[0].lessThan;
	int output_redirect = *cmd[0].greaterThan;
	char** command;
	char* infile;
	char* outfile;

	// printf("before first loop\n");
	for ( int i = 0; cmd[0].args[i] != NULL && strcmp(cmd[0].args[i], "<") != 0 && strcmp(cmd[0].args[i], ">") != 0; i++ ) {
		// printf("first loop %d\n", i);
		num_cmd_words++;
	}
	// printf("after first loop\n");
	command = malloc( (num_cmd_words + 1) * sizeof(char*));
	for ( int i = 0; i < num_cmd_words; i++ ) {
		command[i] = malloc( strlen(cmd[0].args[i]) + 1);
		strcpy( command[i], cmd[0].args[i] );
	}
	command[num_cmd_words] = NULL;
	if ( input_redirect > -1 ) {
		infile = malloc(strlen(cmd[0].args[input_redirect + 1]) + 1);
		strcpy(infile, cmd[0].args[input_redirect + 1]);
		fd_in = open(infile, O_RDONLY);
		// printf("new fd_in: %d\n", fd_in);
	}
	if ( output_redirect > -1 ) {
		outfile = malloc(strlen(cmd[0].args[output_redirect + 1]) + 1);
		strcpy(outfile, cmd[0].args[output_redirect + 1]);
		fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
		// printf("new fd_out: %d\n", fd_out);
		// error("reporting file error");
	}
	if ( (pid = fork()) == 0 ) {
		dup2(fd_in, 0);
		dup2(fd_out, 1);
		execvp(command[0], command);
		error("Single Exec Failure");
	} else if ( pid == -1 ) {
		error("Fork error in single_exec");
	} else {
		while ( wait(NULL) != -1 );
		if ( fd_in != 0 ) {
			if ( close(fd_in) == -1 )
				error("Close fd_in failed");
		}
		if ( fd_out != 1 ) {
			if ( close(fd_out) == -1 ) 
				error("Close fd_out failed");
		}
	}

	for ( int i = 0; i < num_cmd_words; i++ ) {
		free(command[i]); command[i] = NULL;
	}
	if ( command ) {
		free(command); command = NULL;
	}
}

void exec_test(struct command *cmd) {
	int pfd[2];
	pid_t pid;
	int fd_in = 0;
	int count = 0;


	
	while ( cmd[count].args != NULL ) {
		if ( pipe(pfd) == -1 )
			error("Broken pipe");
		if ( (pid = fork()) == -1 ) {
			error("Fork failed");
		} else if ( pid == 0 ) {
			dup2(fd_in, 0);
			if ( cmd[count + 1].args != NULL ) {
				dup2( pfd[1], 1 );
			} 
			close(pfd[0]);
			execvp( cmd[count].args[0], cmd[count].args );
			error("Exec failure");
		} else {
			while (wait(NULL) != -1);
			close(pfd[1]);
			fd_in = pfd[0];
			count++;
		}
		// printf("progs address: %p\n", progs);
	}
}

void error(char* s) {
	extern int errno;

    fprintf( stderr, "%s\n", s );
    fprintf( stderr, " (%s)\n", strerror(errno) );
    exit( 1 );
}