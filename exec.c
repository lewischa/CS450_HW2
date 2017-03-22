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



char* stripQuotes(char* input){
    int sizeOfInput = 0;
    //get new size of argument without quotation marks
    for(int i = 0; input[i] != '\0'; i++){
        if(input[i] != '\"'){
            sizeOfInput++;
        }
    }
    //allocate memory for result of new string
    char* result = malloc((sizeOfInput+1) * sizeof(char));
    int resultIterator = 0;
    //create new string
    for(int i = 0; input[i] != '\0'; i++){
        if(input[i] != '\"'){
            result[resultIterator++] = input[i];
        }
    }
    //Add null terminating character to the end of the string
    result[sizeOfInput] = '\0';
    return result;
}


void redirectPos(struct command *cmd, int index) {
/*
	For each command in *cmd, sets the position (in args) of
	'<' and '>'. If not found, the value will be -1 as initialized.
*/

	int i = 0; 
	while ( cmd[index].args[i] != '\0' ) {
		if ( strcmp(cmd[index].args[i], "<") == 0 ) {
			*cmd[index].lessThan = i;
		}
		if ( strcmp(cmd[index].args[i], ">") == 0 ) {
			*cmd[index].greaterThan = i;
		}
		i++;
	}
}




int isMeta(char* c) {
/*
	Return 1 if c is '|', return 0 otherwise
*/

    if ( strcmp(c, "|") == 0 ) {
        return 1;
    } 
    return 0;
}

int isQuote(char* arg) {
/*
	Return 1 if c is '"', return 0 otherwise
*/

	if ( arg[0] == '\0' ) return 0;
	if ( arg[0] == '"' ) return 1;
	return 0;
}

int isEndQuote(char* word) {
/*
	Return 1 if last characer in word is '"', return 0 otherwise
*/

	if ( word[strlen(word) - 1] == '"' ) {
		return 1;
	}
	return 0;
}



void exec(char** line_words, int num_words) {
/*
	Exec allocates memory for struct command, and populates it.
		- Each element of *cmd is its own command (including any I/O redirection with '<' and '>')
			- Each element of *cmd.args is a 'line_word'
		- Commands are delimited by '|' characters (not included in *cmd)
		- Strip quotes from arguments, concatenate words between begin/end quotes into one char* 
			- Add to *cmd.args as ONE element
*/

	// Allocate memory, initialize lessThan/greaterThan
	struct command *cmd = malloc(num_words * sizeof(*cmd));
    cmd[0].args = malloc(MAX_LINE_WORDS * sizeof(char*));
    cmd[0].lessThan = malloc(sizeof(int*));
    cmd[0].greaterThan = malloc(sizeof(int*));
    *cmd[0].lessThan = -1;
    *cmd[0].greaterThan = -1;

    // Indices
    int cmdNumber = 0;
    int argNumber = 0;

    // String to contain potential quoted arguments
    char* quotedArg;

    for ( int i = 0; i < num_words; i++ ) {
        if ( isMeta(line_words[i]) ) {
        	/*
        		Current line_word is '|': we've reached the end of a command.
        		Reset argNumber index and increment cmdNumber index.
        		Allocate memory for next command and its args, initialize values.
        	*/
            cmd[cmdNumber].args[argNumber] = NULL;
            argNumber = 0;
            redirectPos(cmd, cmdNumber);
            cmdNumber++;
            cmd[cmdNumber].args = malloc(MAX_LINE_WORDS * sizeof(char*));
            cmd[cmdNumber].lessThan = malloc(sizeof(int*));
    		cmd[cmdNumber].greaterThan = malloc(sizeof(int*));
            *cmd[cmdNumber].lessThan = -1;
            *cmd[cmdNumber].greaterThan = -1;

        } else if ( isQuote(line_words[i]) ) {

        	// This block is executed when line_words[i] begins with a " (quote)
                //iterate through line_words to get the size of the quoted string
                int qStrLength = strlen(stripQuotes(line_words[i]));
                for(int q = i+1; !isEndQuote(line_words[q-1]); q++){
                    // Get necessary size of de-quoted string for memory allocation
                    qStrLength += (strlen(stripQuotes(line_words[q]))+1);
                }
                //allocate memory for the command argument
        	cmd[cmdNumber].args[argNumber] = malloc((qStrLength+1) * sizeof(char));

        	// Get all elements of line_words from starting quote to end quote, copy into current args[argNumber]
        	while ( !isEndQuote(line_words[i]) ) {
                    cmd[cmdNumber].args[argNumber] = strcat(cmd[cmdNumber].args[argNumber],strcat(stripQuotes(line_words[i++])," "));
        	}
                //Grab last quoted argument and add it to command argument
                char* temp = malloc(strlen(stripQuotes(line_words[i])+1)*sizeof(char));
                strcpy(temp, stripQuotes(line_words[i]));
                cmd[cmdNumber].args[argNumber] = strcat(cmd[cmdNumber].args[argNumber],temp);
                
                //increment argNumber
                argNumber++;
        	
                if ( (i + 1) == num_words ) {
                	// the next i is num_words, aka we're done and set last args[argNumber] to NULL
                	cmd[cmdNumber].args[argNumber] = NULL;
                	redirectPos(cmd, cmdNumber);
                }
                
        } else {
        	/*
        		Current line_word is not '|' and is not a quoted word. 
        		Allocate memory and populate cmd[].args[argNumber] with the line_word
        	*/
        	cmd[cmdNumber].args[argNumber] = malloc(strlen(line_words[i]) * sizeof(char) + 1);
        	strcpy(cmd[cmdNumber].args[argNumber], line_words[i]);
                argNumber++;
                if ( (i + 1) == num_words ) {
                	/*
                		Signifies last iteration before end of line_words.
                		Set last args element to NULL
                	*/
                    cmd[cmdNumber].args[argNumber] = NULL;
               		// Set position of '<' and '>' if necessary
                    redirectPos(cmd, cmdNumber);
                }
        }
    }

    if ( num_words > 0 ) {
    	/*
    		Set the element after last valid command to NULL 
    			- Used to check end of command line in exec_launch
    	*/
        cmd[++cmdNumber].args = NULL;

        // Launch necessary process(es) with piping and redirection.
    	exec_launch(cmd);
    } else {

    	// The command line was a null string, set first command to NULL so as not to execute anything.
        cmd[0].args = NULL;
    }

    // Free allocated memory
    freeMemory(cmd, cmdNumber);
}

void freeMemory(struct command *cmd, int cmdNumber){
/*
	"Deep" free memory
*/

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








char** getRedirectedCommand(struct command *cmd, int pos) {
	// Returns command portion of a redirected command (stripped of file names and redirection operators)

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

	// Initial file descriptors for stdin and stdout
	int fd_in = 0;
	int fd_out = 1;

	// cmd index
	int count = 0;

	// Necessary file names and string array used with I/O redirection
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
					/*
						Handle input redirection
							- Get filename, open the file, and set fd_in to the file descriptor
							- Error out if open() fails
					*/
					input_file = malloc(strlen(cmd[count].args[*cmd[count].lessThan + 1]) + 1);
					input_file = getInfile(cmd, count);
					int temp_fd_in = open(input_file, O_RDONLY);
					if ( temp_fd_in < 0 )
						error("Could not open input file.");
					fd_in = temp_fd_in;
				}
				if ( *cmd[count].greaterThan != -1 ) {
					/*
						Handle output redirection
							- Get filename, open the file, and set fd_out to the file descriptor
							- Error out if open() fails
					*/
					output_file = malloc(strlen(cmd[count].args[*cmd[count].greaterThan + 1]) + 1);
					output_file = getOutfile(cmd, count);
					int temp_fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
					if ( temp_fd_out < 0 )
						error("Could not open output file.");
					fd_out = temp_fd_out;

					// Replace stdout with fd_out
					dup2(fd_out, 1);
				} else {
					/*
						There is no output redirection
							- Check if there's another command
								- If so, set up for pipe
					*/
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
				/* 
					No output or input redirection using '<' or '>'
				*/
				dup2(fd_in, 0);
				if ( cmd[count + 1].args != NULL ) {
					dup2( pfd[1], 1 );
				} 
				close(pfd[0]);
				execvp( cmd[count].args[0], cmd[count].args );
				error("Exec failure");
			}
		} else {
			while (wait(NULL) != -1);	// Wait for child to finish execution
			close(pfd[1]);
			fd_in = pfd[0];		// Pick up the next input file descriptor (from last exec'd and piped command)
			count++;
		}
	}
}

void error(char* s) {
/*
	Error out printing s and the error type.
*/

	extern int errno;

    fprintf( stderr, "%s\n", s );
    fprintf( stderr, " (%s)\n", strerror(errno) );
    exit( 1 );
}