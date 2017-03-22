/*
 * Authors: Jeremy Olsen; Chad Lewis
 * Course: CS 450
 * Assignment: Piping and redirection homework
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "parsetools.h"
#include "exec.h"
#include <sys/wait.h>


int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    printf("Pipe_shell$ ");
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        //Parse shell command arguments and get the number of words
        int num_words = split_cmd_line(line, line_words);
        //Execute the commands
        exec(line_words, num_words);
        //Ask for next command at shell
        printf("Pipe_shell$ ");
    }
    
    printf("\n");

    return 0;
}