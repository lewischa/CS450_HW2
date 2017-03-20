#ifndef EXEC_H
#define EXEC_H

struct command {
    char **args;
    int *lessThan;
    int *greaterThan;
};

int pipePos(char **line, int startPos);
void redirectPos(struct command *cmd, int index);
int numCommands(char **line, int *pipes, int *redirects);
char** getCommand(char** line, int startPos, int endPos);
int isMeta(char* c);
void exec(char** line, int num_words);
void freeMemory(struct command *cmd, int cmdNumber);
void exec_launch(struct command *cmd);
void error(char* s);
// void single_exec(struct command *cmd, int pos, int pfd[], int inDescriptor);


#endif 