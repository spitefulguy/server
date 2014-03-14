#ifndef QUEUE_H_
#define QUEUE_H_

void *wait_for_request(void *);
size_t queue_pop(char *s_request_buf, int *fd_buf, int thread);
void queue_push(char *s_request, ssize_t length, int fd);
void queue_init();

#endif
