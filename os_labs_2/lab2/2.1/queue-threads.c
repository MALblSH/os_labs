#include "queue.h"

void set_cpu(int n) {
	int err;
	cpu_set_t cpuset;
	pthread_t tid = pthread_self();

	CPU_ZERO(&cpuset);
	CPU_SET(n, &cpuset);

	err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	if (err != SUCCESS) {
		printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
		return;
	}

	printf("set_cpu: set cpu %d\n", n);
}

void *reader(void *arg) {
	int expected = 0;
	queue_t *q = (queue_t *)arg;
	printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(1);

	while (true) {
		int val = -1;
		int ok = queue_get(q, &val);
		if (ok != QUEUE_OP_SUCCESS)
			continue;

		if (expected != val)
			printf(RED"ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);

		expected = val + 1;
	}

	return NULL;
}

void *writer(void *arg) {
	int i = 0;
	queue_t *q = (queue_t *)arg;
	printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(1);

	while (true) {
		int ok = queue_add(q, i);
		if (ok != QUEUE_OP_SUCCESS)
			continue;
		i++;
	}

	return NULL;
}

int main() {
	pthread_t reader_tid, writer_tid;
	queue_t *q;
	int err;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(100000);

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

	err = pthread_join(reader_tid, NULL);
    if (err != SUCCESS) {
        printf("main: pthread_join() failed: %s\n", strerror(err));
		queue_destroy(q);
		return ERROR;
    }
	
	err = pthread_join(writer_tid, NULL);
    if (err != SUCCESS) {
        printf("main: pthread_join() failed: %s\n", strerror(err));
		queue_destroy(q);
		return ERROR;
    }	
	queue_destroy(q);

	pthread_exit(NULL);
}
