#include <stdio.h>
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
        int num_words = split_cmd_line(line, line_words);

        // Just for demonstration purposes
        // for (int i=0; i < num_words; i++)
        //     printf("%s ", line_words[i]);

        exec(line_words);
        while (wait(NULL) != -1);
        // printf("line first piped at position %d\n", piped);
        // printf("line first redirected at position %d\n", redirect);
        // printf("\n");
        printf("Pipe_shell$ ");
    }

    return 0;
}



