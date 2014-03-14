#ifndef HNDS_H_
#define HNDS_H_

#include <unistd.h>

//void common_handler(char *request, int fd);
//void *wait_request(void *fd);
//void close_connection(int cfd, char *status, int thread);
int common_handler(const char *req_c, int fd, int thread);
void close_connection(int fd, char *status, int thread);



#endif /* HNDS_H_ */
