#ifndef HTTP_H_
#define HTTP_H_
#include "unistd.h"

#define POST 	"POST"
#define GET 	"GET"
#define HEAD 	"HEAD"
#define PUT 	"PUT"

#define HTTP_09 "HTTP/0.9\r\n"
#define HTTP_10 "HTTP/1.0\r\n"
#define HTTP_11 "HTTP/1.1\r\n"

#define HTTP_REQ_END "\r\n\r\n"

struct Request{
	char *request;
	char *method;
	char *uri;
	char *version;
	char *host;
};

int encode_request(char *req_c, struct Request *req_r);
int set_headers(char *headers, const char *filename, ssize_t filesize, char* ext);


#endif /* HTTP_H_ */
