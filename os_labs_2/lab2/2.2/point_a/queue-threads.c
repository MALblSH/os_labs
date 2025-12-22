#include "queue.h"

int set_cpu(int n) {
	int err;
	cpu_set_t cpuset;
	pthread_t tid = pthread_self();

	CPU_ZERO(&cpuset);
	CPU_SET(n, &cpuset);

	err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	if (err != SUCCESS) {
		printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
		return ERROR;
	}

	printf("set_cpu: set cpu %d\n", n);
	return SUCCESS;
}

void *reader(void *arg) {
	int expected = 0;
	int err;
	queue_t *q = (queue_t *)arg;
	printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

	err = set_cpu(1);
	if (err != SUCCESS){
		return (void*)(long)ERROR;
	}

	while (true) {
		int val = -1;
		int ok = queue_get(q, &val);
		if (ok == ERROR) {
			return (void*)(long)ERROR;
		}
		if (ok != QUEUE_OP_SUCCESS)
			continue;

		if (expected != val)
			printf(RED"ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);

		expected = val + 1;
	}

	return (void*)(long)SUCCESS;
}

void *writer(void *arg) {
	int err;
	int i = 0;
	queue_t *q = (queue_t *)arg;
	printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

	err = set_cpu(1);
	if (err != SUCCESS){
		return (void*)(long)ERROR;
	}

	while (true) {
		int ok = queue_add(q, i);
		if (ok == ERROR) {
			return (void*)(long)ERROR;
		}
		if (ok != QUEUE_OP_SUCCESS)
			continue;
		i++;
	}

	return (void*)(long)SUCCESS;
}

int main() {
	pthread_t reader_tid, writer_tid;
	void *writer_err;
	void *reader_err;
	queue_t *q;
	int err;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(100000);
	if (q == NULL){
		return EXIT_FAILURE;
	}

	err = pthread_create(&reader_tid, NULL, reader, q);
	if (err != SUCCESS) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		queue_destroy(q);
		return EXIT_FAILURE;
	}

	sched_yield();

	err = pthread_create(&writer_tid, NULL, writer, q);
	if (err != SUCCESS) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		queue_destroy(q);
		return EXIT_FAILURE;
	}

	err = pthread_join(reader_tid, &reader_err);
    if (err != SUCCESS) {
        printf("main: pthread_join() failed: %s\n", strerror(err));
		queue_destroy(q);
		return EXIT_FAILURE;
    }
	if(reader_err != SUCCESS){
		queue_destroy(q);
		return EXIT_FAILURE;
	}
	
	err = pthread_join(writer_tid, &writer_err);
    if (err != SUCCESS) {
        printf("main: pthread_join() failed: %s\n", strerror(err));
		queue_destroy(q);
		return EXIT_FAILURE;
    }	
	if(writer_err != SUCCESS){
		queue_destroy(q);
		return EXIT_FAILURE;
	}

	err = queue_destroy(q);
	if (err != SUCCESS){
		return EXIT_FAILURE;
	}

	pthread_exit(NULL);
}
