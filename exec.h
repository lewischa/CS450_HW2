#ifndef EXEC_H
#define EXEC_H

int isPiped(char **line, int startPos);
int isRedirect(char **line, int startPos);
void exec(char **line);

#endif 