#ifndef HEADER
#define HEADER

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 1
#define ERROR -1

void show_variable_addresses();
int* return_local_address();
int heap_memory_test();
int environment_test();

#endif
