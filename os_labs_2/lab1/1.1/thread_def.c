#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define THREAD_COUNT 5
#define ERROR 1
#define SUCCESS 0

int global_var = 5;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void *mythread(void *arg) {
	int local_var = 37;
	static int static_var = 10;
	const int const_var = 7;
	int thread_num = (int)(long)arg;

	pthread_mutex_lock(&print_lock);

	printf("\nmythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
	printf("descriptor: %lu\n", (unsigned long)pthread_self());
	printf("global var: %p\n", (void*)&global_var);
	printf("local var: %p\n", (void*)&local_var);
	printf("static var: %p\n", (void*)&static_var);
	printf("const var: %p\n", (void*)&const_var);

	printf("Thread %d: global_var = %d, local_var = %d, static_var = %d\n", 
           thread_num, global_var, local_var, static_var);
    
    global_var += 10;
    local_var += 10;
    static_var += 10;
    
    printf("Thread %d after change: global_var = %d, local_var = %d, static_var = %d\n", 
           thread_num, global_var, local_var, static_var);

	pthread_mutex_unlock(&print_lock);
	return NULL;
}

int main() {
	pthread_t tid[THREAD_COUNT];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i < THREAD_COUNT; i++){
		err = pthread_create(&tid[i], NULL, mythread, (void*)(long)i);
		if (err != SUCCESS) {
	    	printf("main: pthread_create() failed: %s\n", strerror(err));
			return EXIT_FAILURE;
		}
	}

	getchar();
	
	for (int i = 0; i < THREAD_COUNT; i++){
		err = pthread_join(tid[i], NULL);
    	if (err != SUCCESS) {
        	printf("main: pthread_join() failed: %s\n", strerror(err));
        	return EXIT_FAILURE;
    	}
	}
	return EXIT_SUCCESS;
}