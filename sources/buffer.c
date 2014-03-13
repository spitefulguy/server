/*
 * worker.c
 *
 *  Created on: Feb 28, 2014
 *      Author: alex
 */

#include <stdlib.h>

#include <pthread.h>

#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "hnds.h"
#include "http.h"
#include "../settings.h"


#define CONS_PER_WORKER 256 //TODO: Make linked list

static int fds[WORKERS][CONS_PER_WORKER];
static char requests[MAX_CONNECTIONS][MAX_REQUEST_LENGTH];
//static int conditions[MAX_CONNECTIONS];

static void add_connection(int fd) {
	static int last;

#ifdef DEBUG
	int k, j;
	for (j = 0; j < WORKERS; j++ ) {
		for (k = 0; k < CONS_PER_WORKER; k++) {
			if (fds[j][k] == fd) {
				printf("WARNING: Connection (%d) already exists in list\n", fd);
				return;
			}
		}
	}
#endif

	int i = 0;
	while(fds[last][i++])
		;
	fds[last][i - 1] = fd;
	//printf("Worker %d handles %d\n", last, fd);

	last++;
	if (last == WORKERS) last = 0;
}

static void remove_connection(int fd, int thread) {
	int j = 0;
	do {
		if (fds[thread][j] == fd) {
			do {
				fds[thread][j] = fds[thread][j + 1];
			} while (fds[thread][++j]);
			return;
		}
	} while(fds[thread][++j]);
#ifdef DEBUG
	printf("WARNING: There is no connection %d in list\n", fd);
#endif
}

void access_connections_list(int fd, int thread, int action) {
	static pthread_mutex_t mutex_con = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex_con);
	if (action == ACT_STORE) add_connection(fd);
	if (action == ACT_DELETE) remove_connection(fd, thread);
	pthread_mutex_unlock(&mutex_con);
}

static void store_request(const char *s_request, ssize_t count, int fd) {
//TODO queue for multiple requests
	if (!requests[fd]) {
		printf("WARNING: request (%d) lost\n", fd);
		return;
	}
	strcpy(requests[fd], s_request);
}

static int obtain_request(char *s_request_buf,int fd){
	if (!requests[fd][0]) return 0;

	int len = strlen(requests[fd]);
	//s_request_buf = (char *)malloc(sizeof(char)*(len + 1));
	strcpy(s_request_buf, requests[fd]);
	requests[fd][0] = '\0';
	//printf("OBTAIN\n%p\n", s_request_buf);
	return len;
}

int access_buffer(char *s_obtain_request_buf, const char *s_store_request,
		ssize_t store_str_length, int fd, int action){

	static pthread_mutex_t mutex_buf = PTHREAD_MUTEX_INITIALIZER;
	int status;

	status = pthread_mutex_lock(&mutex_buf);
	if (status != 0) {
		printf("Mutex lock failed (%d)\n", status);
	}
	if (action == ACT_OBTAIN){
		int length = obtain_request(s_obtain_request_buf, fd);
		status = pthread_mutex_unlock(&mutex_buf);
		if (status != 0) {
			printf("Mutex unlock failed obtain (%d)\n", status);
		}

		return length;
	} else if (action == ACT_STORE){

		store_request(s_store_request, store_str_length, fd);
		status = pthread_mutex_unlock(&mutex_buf);
		if (status != 0) printf("Mutex unlock failed store (%d)\n", status);
	}
	return 0;
}

void *wait_for_request(void *this_thread) {
	int this = (int)this_thread;

	char s_request[MAX_REQUEST_LENGTH] = { 0 };
	int length;

	int i;
	while (1) {
		i = -1;
		while(fds[this][++i]) {
			length = access_buffer(s_request, NULL, 0, fds[this][i], ACT_OBTAIN);
			if ( length ) {
				common_handler(s_request, fds[this][i], this);
			}
		}
	}
	return 0;
}



