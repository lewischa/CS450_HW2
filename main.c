#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "parsetools.h"
#include "exec.h"
#include <sys/wait.h>

int isMeta(char* c) {
    // printf("isMeta: %s\n", c);
    if (strcmp(c, "|") == 0 || strcmp(c, "<") == 0 || strcmp(c, ">") == 0) {
        return 1;
    } 
    return 0;
}

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    printf("Pipe_shell$ ");
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);

        // Just for demonstration purposes
        // for (int i=0; i < num_words; i++)
        //     printf("%s\n", line_words[i]);

        struct command *cmd = malloc(num_words * sizeof(char**));
        cmd[0].args = malloc(5 * sizeof(char*));
        int cmdNumber = 0;
        int argNumber = 0;
        for ( int i = 0; i < num_words; i++ ) {
            // printf("%lu\n", sizeof(cmd[cmdNumber].args));
            if ( isMeta(line_words[i]) ) {
                cmd[cmdNumber].args[argNumber] = NULL;
                argNumber = 0;
                cmdNumber++;
                cmd[cmdNumber].args = malloc(5 * sizeof(char*));
            } else {
                cmd[cmdNumber].args[argNumber] = line_words[i];
                // printf("Inside else: %s\n", cmd[cmdNumber].args[argNumber]);
                argNumber++;
                if ( (i + 1) == num_words ) {
                    cmd[cmdNumber].args[argNumber] = NULL;
                }
            }
        }

        // for ( int i = 0; i < cmdNumber; i++ ) {
        //     printf("cmd[%d]: ", i);
        //     int j = 0;
        //     while ( cmd[i].args[j] != NULL ) {
        //         printf("%s ", cmd[i].args[j]);
        //         j++;
        //     }
        //     printf("\n");
        // }

        if ( num_words > 0 ) {
            cmd[++cmdNumber].args = NULL;
            // printf("cmdNumber: %d\n", cmdNumber);
        } else {
            cmd[0].args = NULL;
        }

        

        exec_test(cmd);

        
        // printf("\n");
        // exec(line_words);
        // while (wait(NULL) != -1);
        // printf("line first piped at position %d\n", piped);
        // printf("line first redirected at position %d\n", redirect);
        // printf("\n");
        // for ( int i = 0; i <= cmdNumber; i++ ) {
        //     free(cmd[i].args);
        // }
        free(cmd);
        printf("Pipe_shell$ ");
    }

    return 0;
}



