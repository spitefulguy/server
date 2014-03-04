#ifndef HTTP_H_
#define HTTP_H_
#include "unistd.h"

#define POST 	"POST"
#define GET 	"GET"
#define HEAD 	"HEAD"
#define PUT		"PUT"
#define PATCH	"PATCH"
#define	DELETE	"DELETE"
#define TRACE	"TRACE"
#define LINK	"LINK"
#define UNLINK	"UNLINK"
#define CONNECT "CONNECT"

#define HTTP_09 "HTTP/0.9\r\n"
#define HTTP_10 "HTTP/1.0\r\n"
#define HTTP_11 "HTTP/1.1\r\n"

#define HTTP_200 "HTTP/1.1 200 OK\r\n"
#define HTTP_400 "HTTP/1.1 400 Bad Request\r\n"
#define HTTP_403 "HTTP/1.1 403 Forbidden\r\n"
#define HTTP_404 "HTTP/1.1 404 Not Found\r\n"
#define HTTP_405 "HTTP/1.1 405 Method Not Allowed\r\n"
#define HTTP_413 "HTTP/1.1 413 Request Entity Too Large\r\n"
#define HTTP_500 "HTTP/1.1 500 Bad Gateway\r\n"

#define C_TYPE_CSS 	"text/css\r\n"
#define C_TYPE_HTML "text/html\r\n"
#define C_TYPE_TEXT "text/plain\r\n"
#define C_TYPE_JS 	"application/javascript\r\n"
#define C_TYPE_JPG 	"image/jpeg\r\n"
#define C_TYPE_PNG 	"image/png\r\n"
#define C_TYPE_GIF 	"image/gif\r\n"
#define C_TYPE_TIFF	"image/tiff\r\n"
#define C_TYPE_SWF 	"application/x-shockwave-flash\r\n"
#define C_TYPE_MISC "application/misc\r\n"

#define HTTP_CLOSE 		"Close\r\n"
#define HTTP_KEEPALIVE	"Keep-alive\r\n"

#define HTTP_REQ_END "\r\n\r\n"

struct request{
	//char *request;
	char *method;
	char *uri;
	char *query;
	char *version;
	char *host;
	char *connection;
};

int encode_request(const char *req_c, struct request *req_r);
int set_headers(char *headers, char *status, ssize_t filesize, char* ext);


#endif /* HTTP_H_ */
