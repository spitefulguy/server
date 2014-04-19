#include <stdlib.h>

#include <pthread.h>

#include <stdio.h>
#include <string.h>

#include "hnds.h"
#include "http.h"
#include "settings.h"
#include "config.h"

struct item {
	char *request;
	ssize_t length;
	int fd;
	struct item *next;
};

struct item **head;
static pthread_mutex_t *queue_mutex;
static pthread_cond_t *queue_cond;


void queue_init() {
	const int threads = get_number_of_threads();

	head = calloc(threads, sizeof(struct item*));
	queue_mutex = malloc(sizeof(pthread_mutex_t) * threads);
	queue_cond = malloc(sizeof(pthread_cond_t) * threads);
	int i;
	for (i = 0; i < threads; i++) {
		queue_mutex[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		queue_cond[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
	}
}

static void push(char *s_request, ssize_t length, int fd, int thread){

	struct item *item;

	pthread_mutex_lock(&queue_mutex[thread]);

	if (head[thread]) {
		item = head[thread];
		while( item->next ) item = item->next;
		item->next = malloc(sizeof(struct item));
		item = item->next;
	} else {
		head[thread] = malloc(sizeof(struct item));
		item = head[thread];
	}

	item->next = NULL;
	item->fd = fd;
	item->length = length;
	item->request = malloc(sizeof(char) * (length + 1));
	memcpy(item->request, s_request, length + 1);

	pthread_cond_signal(&queue_cond[thread]);
	pthread_mutex_unlock(&queue_mutex[thread]);
}

void queue_push(char *s_request, ssize_t length, int fd) {
	static int thread;

	push(s_request, length, fd, thread);

	if (++thread == get_number_of_threads()) thread = 0;
}

size_t queue_pop(char *s_request_buf, int *fd_buf, int thread) {

	pthread_mutex_lock(&queue_mutex[thread]);
	if (!head[thread]) {
		pthread_cond_wait(&queue_cond[thread], &queue_mutex[thread]);
	}

	struct item *item = head[thread];

	size_t length = item->length;
	*fd_buf = item->fd;
	memcpy(s_request_buf, item->request, length + 1);
	free(item->request);

	free(head[thread]);
	head[thread] = item->next;

	pthread_mutex_unlock(&queue_mutex[thread]);

	return length;
}


void *wait_for_request(void *this_thread) {
	int *this = (int *)this_thread;

	char s_request[MAX_REQUEST_LENGTH] = { 0 };
	ssize_t length;

	int fd;
	while (1) {
		if ( (length = queue_pop(s_request, &fd, *this)) )
			common_handler(s_request, fd, *this);
	}
	return 0;
}



