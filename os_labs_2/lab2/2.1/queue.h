#ifndef __FITOS_QUEUE_H__
#define __FITOS_QUEUE_H__

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>

#define QUEUE_OP_SUCCESS 1
#define QUEUE_OP_FAILURE 0
#define SUCCESS 0
#define ERROR -1
#define SLEEP_TIME 1
#define RED "\033[41m"
#define NOCOLOR "\033[0m"

typedef struct _QueueNode {
	int val;
	struct _QueueNode *next;
} qnode_t;

typedef struct _Queue {
	qnode_t *first;
	qnode_t *last;

	pthread_t qmonitor_tid;

	int count;
	int max_count;

	// queue statistics
	long add_attempts;
	long get_attempts;
	long add_count;
	long get_count;
} queue_t;

queue_t* queue_init(int max_count);
void queue_destroy(queue_t *q);
int queue_add(queue_t *q, int val);
int queue_get(queue_t *q, int *val);
void queue_print_stats(queue_t *q);

#endif		// __FITOS_QUEUE_H__
