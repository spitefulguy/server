/*
 * worker.h
 *
 *  Created on: Feb 28, 2014
 *      Author: alex
 */

#ifndef WORKER_H_
#define WORKER_H_

void access_connections_list(int fd, int thread, int action);
void *wait_for_request(void *);
int access_buffer(char *s_obtain_request_buf, const char *s_store_request,
		ssize_t store_str_length, int fd, int action);

#endif /* WORKER_H_ */
