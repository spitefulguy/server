#ifndef HTTP_H_
#define HTTP_H_
#include "unistd.h"


enum method {
	GET, POST, HEAD, PUT
};

struct request {
	enum method method;
	char *url;
	char *query;
};

struct request* encode_request(char *request);
int set_headers(char *headers, const char *filename, ssize_t filesize, char* ext);


#endif /* HTTP_H_ */
