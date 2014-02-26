#ifndef HNDS_H_
#define HNDS_H_

#define MAX_CONNECTIONS 256
#define IN_BUF_SIZE 512

#define REQ_STORE 		1
#define REQ_OBTAIN		2

#define REQ_COND_EMPTY	0
#define REQ_COND_RAW	1
#define REQ_COND_INCOMP 2
#define REQ_COND_OK 	3

//void common_handler(char *request, int fd);
void *wait_request(void *fd);
struct Request *buffer_store(char *buf, int cfd, int isWrite);



#endif /* HNDS_H_ */
