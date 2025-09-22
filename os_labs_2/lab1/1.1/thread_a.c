#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define ERROR 1
#define SUCCESS 0

void *mythread(void *arg) {
	(void)arg;
	printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
	return NULL;
}

int main() {
	pthread_t tid;
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
	err = pthread_create(&tid, NULL, mythread, NULL);
	if (err != SUCCESS) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return EXIT_FAILURE;
	}
	
	err = pthread_join(tid, NULL);
    if (err != SUCCESS) {
    	printf("main: pthread_join() failed: %s\n", strerror(err));
    	return EXIT_FAILURE;
    }
	
	return EXIT_SUCCESS;
}