#ifndef HNDS_H_
#define HNDS_H_

#include <unistd.h>



#define ACT_STORE 	1
#define ACT_OBTAIN	2
#define ACT_DELETE 	3

#define REQ_COND_EMPTY	1
#define REQ_COND_RAW	2
#define REQ_COND_INCOMP 3
#define REQ_COND_OK 	4

//void common_handler(char *request, int fd);
//void *wait_request(void *fd);
//void close_connection(int cfd, char *status, int thread);
int common_handler(const char *req_c, int fd, int thread);
void close_connection(int fd, char *status, int thread);



#endif /* HNDS_H_ */
