#ifndef EXEC_H
#define EXEC_H

int pipePos(char **line, int startPos);
int redirectPos(char **line, int startPos);
int numCommands(char **line, int *pipes, int *redirects);
char** getCommand(char** line, int startPos, int endPos);
void exec(char **line);
void loop_exec(char ***progs, int n);
void error(char* s);

#endif 