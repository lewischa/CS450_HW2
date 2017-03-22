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

char* stripQuotes(char* input){
    int sizeOfInput = 0;
    for(int i = 0; input[i] != NULL; i++){
        if(input[i] != '\"' && input[i] != '\''){
            sizeOfInput++;
        }
    }
    char* result = malloc(sizeOfInput * sizeof(char));
    int resultIterator = 0;
    for(int i = 0; input[i] != NULL; i++){
        if(input[i] != '\"' && input[i] != '\''){
            result[resultIterator++] = input[i];
        }
    }
    return result;
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

int isQuote(char* arg) {
	if ( arg[0] == '\0' ) return 0;
	if ( arg[0] == '"' ) return 1;
	return 0;
}

int isEndQuote(char* word) {
	if ( word[strlen(word) - 1] == '"' ) {
		return 1;
	}
	return 0;
}


// getQuotedArg will return the 'words' in char **line that are surrounded by quotes, 
// concatenated into one string
char* getQuotedArg(char **line, int pos) {
	char *arg = malloc(strlen(line[pos]) + 1);
	if ( isEndQuote(line[pos]) ) {
		strcpy(arg, line[pos] + 1);
		arg[strlen(arg) - 1] = '\0';
		return arg;
	}
	//////////////////////////////////////////////////////////// CONTINUE HERE
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
    char* quotedArg;

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
//        } else if ( isQuote(line_words[i]) ) {
//        	// This block is executed when line_words[i] begins with a " (quote)
//
//        	cmd[cmdNumber].args[argNumber] = malloc(strlen(getQuotedArg(line_words, i)) + 1);
//
//        	// Get all elements of line_words from starting quote to end quote, copy into current args[argNumber]
//        	// May need to modify free memory function, not sure though
//        	strcpy(cmd[cmdNumber].args[argNumber], getQuotedArg(line_words, i));
//        	//////////////////////////////////////////////////////////// CONTINUE HERE
//        	while ( !isEndQuote(line_words[i]) ) {
//        		// Need this loop to advance i to the correct place after getQuotedArg()
//        		i++;
//        	}

        	// will need to increment argNumber here and likely do the same check as the below else{} statement 
        	// (if the next i is num_words, aka we're done and set last args[argNumber] to NULL)

        } else {
        	cmd[cmdNumber].args[argNumber] = malloc(strlen(line_words[i]) * sizeof(char) + 1);
        	strcpy(cmd[cmdNumber].args[argNumber], stripQuotes(line_words[i]));
            argNumber++;
            if ( (i + 1) == num_words ) {
                cmd[cmdNumber].args[argNumber] = NULL;
                redirectPos(cmd, cmdNumber);
            }
        }
    }

    if ( num_words > 0 ) {
        cmd[++cmdNumber].args = NULL;
    	exec_launch(cmd);
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






/* 
		This function should no longer be necessary. Leaving commented out for now.
*/


// void single_exec(struct command *cmd, int pos, int pfd[], int inDescriptor) {
// 	pid_t pid;
// 	int fd_in = 0;
// 	int fd_out = 1;
// 	int num_cmd_words = 0;
// 	// printf("before dereference\n");
// 	int input_redirect = *cmd[0].lessThan;
// 	int output_redirect = *cmd[0].greaterThan;
// 	char** command;
// 	char* infile;
// 	char* outfile;

// 	// printf("before first loop\n");
// 	for ( int i = 0; cmd[pos].args[i] != NULL && strcmp(cmd[pos].args[i], "<") != 0 && strcmp(cmd[pos].args[i], ">") != 0; i++ ) {
// 		// printf("first loop %d\n", i);
// 		num_cmd_words++;
// 	}
// 	// printf("after first loop\n");
// 	command = malloc( (num_cmd_words + 1) * sizeof(char*));
// 	for ( int i = 0; i < num_cmd_words; i++ ) {
// 		command[i] = malloc( strlen(cmd[pos].args[i]) + 1);
// 		strcpy( command[i], cmd[pos].args[i] );
// 	}
// 	command[num_cmd_words] = NULL;
// 	if ( input_redirect > -1 ) {
// 		infile = malloc(strlen(cmd[pos].args[input_redirect + 1]) + 1);
// 		strcpy(infile, cmd[pos].args[input_redirect + 1]);
// 		fd_in = open(infile, O_RDONLY);
// 		// printf("Could not open input file: %s. Proceeding with stdin.\n", infile);
// 	}
// 	if ( output_redirect > -1 ) {
// 		outfile = malloc(strlen(cmd[pos].args[output_redirect + 1]) + 1);
// 		strcpy(outfile, cmd[pos].args[output_redirect + 1]);
// 		fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
// 		// printf("Could not open output file: %s. Proceeding with stdout.\n", outfile);
// 	}
// 	if ( (pid = fork()) == 0 ) {
// 		if ( fd_in != -1 ) {
// 			dup2(fd_in, 0);
// 		}
// 		if ( fd_out != -1 ) {
// 			dup2(fd_out, 1);
// 		}
// 		execvp(command[0], command);
// 		error("Single Exec Failure");
// 	} else if ( pid == -1 ) {
// 		error("Fork error in single_exec");
// 	} else {
// 		while ( wait(NULL) != -1 );
// 		if ( fd_in != 0 ) {
// 			if ( close(fd_in) == -1 )
// 				error("Close fd_in failed");
// 		}
// 		if ( fd_out != 1 ) {
// 			if ( close(fd_out) == -1 ) 
// 				error("Close fd_out failed");
// 		}
// 	}

// 	for ( int i = 0; i < num_cmd_words; i++ ) {
// 		free(command[i]); command[i] = NULL;
// 	}
// 	if ( command ) {
// 		free(command); command = NULL;
// 	}

// 	canContinue = 1;
// 	// if ( infile ) {
// 	// 	free(infile); infile = NULL;
// 	// }
// 	// if ( outfile ) {
// 	// 	free(outfile); outfile = NULL;
// 	// }

// }





char** getRedirectedCommand(struct command *cmd, int pos) {
	// Returns command portion of a redirected command

	/* Example:

		If the line is: head -5 < main.c > test.txt
		getRedirectedCommand() will return: head -5

	*/
	int num_cmd_words = 0;
	char **command = malloc(sizeof(cmd[pos].args));
	for ( int i = 0; cmd[pos].args[i] != NULL && strcmp(cmd[pos].args[i], "<") != 0 && strcmp(cmd[pos].args[i], ">") != 0; i++ ) {
		num_cmd_words++;
	}
	command = malloc( (num_cmd_words + 1) * sizeof(char*));
	for ( int i = 0; i < num_cmd_words; i++ ) {
		command[i] = malloc( strlen(cmd[pos].args[i]) + 1);
		strcpy( command[i], cmd[pos].args[i] );
	}
	command[num_cmd_words] = NULL;
	return command;
}

char* getOutfile(struct command *cmd, int pos) {
	// Returns output file name of a redirected command

	/* Example:

		If the line is: head -5 < main.c > test.txt
		getOutfile() will return: test.txt

	*/
	char *outfile = malloc(strlen(cmd[pos].args[*cmd[pos].greaterThan + 1]) + 1);
	strcpy(outfile, cmd[pos].args[*cmd[pos].greaterThan + 1]);
	return outfile;
}

char* getInfile(struct command *cmd, int pos) {
	// Returns input file name of a redirected command

	/* Example:

		If the line is: head -5 < main.c > test.txt
		getInfile() will return: main.c

	*/
	char *infile = malloc(strlen(cmd[pos].args[*cmd[pos].lessThan + 1]) + 1);
	strcpy(infile, cmd[pos].args[*cmd[pos].lessThan + 1]);
	return infile;
}



void exec_launch(struct command *cmd) {
	int pfd[2];
	pid_t pid;
	int fd_in = 0;
	int fd_out = 1;
	int count = 0;
	char **redirectedCommand;
	char *input_file;
	char *output_file;

	
	while ( cmd[count].args != NULL ) {
		if ( pipe(pfd) == -1 )
			error("Broken pipe");
		if ( (pid = fork()) == -1 ) {
			error("Fork failed");
		} else if ( pid == 0 ) {
			if ( *cmd[count].lessThan != -1 || *cmd[count].greaterThan != -1 ) {
				// This if statement is executed when input or output is redirected using '<' or '>', or both
				redirectedCommand = malloc(sizeof(cmd[count].args));
				redirectedCommand = getRedirectedCommand(cmd, count);
				if ( *cmd[count].lessThan != -1 ) {
					// Handle input redirection
						// Get filename, open the file, and set fd_in to the file descriptor
					input_file = malloc(strlen(cmd[count].args[*cmd[count].lessThan + 1]) + 1);
					input_file = getInfile(cmd, count);
					fd_in = open(input_file, O_RDONLY);
					// LIKELY NEEDS ERROR HANDLING
				}
				if ( *cmd[count].greaterThan != -1 ) {
					// Handle output redirection
						// Get filename, open the file, and set fd_out to the file descriptor
					output_file = malloc(strlen(cmd[count].args[*cmd[count].greaterThan + 1]) + 1);
					output_file = getOutfile(cmd, count);
					fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
					// Replace stdout with fd_out
						// NEEDS ERROR HANDLING
					dup2(fd_out, 1);
				} else {
					if ( cmd[count + 1].args != NULL ) {
						dup2( pfd[1], 1 );
					}
				}
				dup2(fd_in, 0);
				close(pfd[0]);
				execvp(redirectedCommand[0], redirectedCommand);
				error("Redirected Exec Failure");
			}
			else {
				dup2(fd_in, 0);
				if ( cmd[count + 1].args != NULL ) {
					dup2( pfd[1], 1 );
				} 
				close(pfd[0]);
				execvp( cmd[count].args[0], cmd[count].args );
				error("Exec failure");
			}
		} else {
			while (wait(NULL) != -1);
			close(pfd[1]);
			fd_in = pfd[0];
			count++;
		}
	}
}

void error(char* s) {
	extern int errno;

    fprintf( stderr, "%s\n", s );
    fprintf( stderr, " (%s)\n", strerror(errno) );
    exit( 1 );
}