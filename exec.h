/*
 * Authors: Jeremy Olsen; Chad Lewis
 * Course: CS 450
 * Assignment: Piping and redirection homework
 */

#ifndef EXEC_H
#define EXEC_H

struct command {
    char **args;
    int *lessThan;
    int *greaterThan;
};


void redirectPos(struct command *cmd, int index);
char* stripQuotes(char* input);
int isMeta(char* c);
void exec(char** line, int num_words);
void freeMemory(struct command *cmd, int cmdNumber);
void exec_launch(struct command *cmd);
void error(char* s);



#endif 