#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define ERROR -1
#define SUCCESS 0
#define FORK_VAL_FOR_CHILD 0

int parent_print(int* local_var);
void child_print(int* local_var);
int proc_monitoring();

#endif