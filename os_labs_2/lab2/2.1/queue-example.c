#include "queue.h"

int main() {
	queue_t *q;

	printf("main: [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(1000);

	for (int i = 0; i < 10; i++) {
		int ok = queue_add(q, i);
		if (ok == ERROR) {
			return ERROR;
		}
		if (ok != QUEUE_OP_SUCCESS)
			continue;

		printf("ok %d: add value %d\n", ok, i);

		queue_print_stats(q);
	}

	for (int i = 0; i < 12; i++) {
		int val = -1;
		int ok = queue_get(q, &val);
		if (ok == ERROR) {
			return ERROR;
		}
		if (ok != QUEUE_OP_SUCCESS){
			continue;
		}
		printf("ok: %d: get value %d\n", ok, val);

		queue_print_stats(q);
	}

	queue_destroy(q);

	return EXIT_SUCCESS;
}

