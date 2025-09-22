#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define THREAD_COUNT 5
#define ERROR 1
#define SUCCESS 0

int global_var = 5;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void *mythread(void *arg) {
	(void)arg;
	int local_var = 37;
	static int static_var = 10;
	const int const_var = 7;

	pthread_mutex_lock(&print_lock);
	printf("\nmythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
	printf("descriptor: %lu\n", (unsigned long)pthread_self());
	printf("global var: %p\n", (void*)&global_var);
	printf("local var: %p\n", (void*)&local_var);
	printf("static var: %p\n", (void*)&static_var);
	printf("const var: %p\n", (void*)&const_var);
	pthread_mutex_unlock(&print_lock);
	return (void *)pthread_self();
}

int main() {
	pthread_t tids[THREAD_COUNT];
	void *thread_res[5];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i < THREAD_COUNT; i++){
		err = pthread_create(&tids[i], NULL, mythread, NULL);
		if (err != SUCCESS) {
	    	printf("main: pthread_create() failed: %s\n", strerror(err));
			return EXIT_FAILURE;
		}
	}

	getchar();
	
	for (int i = 0; i < THREAD_COUNT; i++){
		err = pthread_join(tids[i], &thread_res[i]);
    	if (err != SUCCESS) {
        	printf("main: pthread_join() failed: %s\n", strerror(err));
        	return EXIT_FAILURE;
    	}
		pthread_t returned_id = (pthread_t)thread_res[i];

		if (pthread_equal(tids[i], returned_id)){
			printf("the same of equal\n");
		} else {
			printf("not the same of equal\n");
		}
		
	}
	
	return EXIT_SUCCESS;
}