#include "../headers/http.h"
#include <stdlib.h>
#include "string.h"
#include "stdio.h"
#include <time.h>

#include "utils.h"
#include "settings.h"

int get_method(struct request *sr_req, const char *s_req) {
	if 	( !strncmp(s_req, GET, strlen(GET)) ) 					sr_req->method = GET;
		else if ( !strncmp(s_req, POST, strlen(POST)) ) 		sr_req->method = POST;
		else if ( !strncmp(s_req, HEAD, strlen(HEAD)) ) 		sr_req->method = HEAD;
		else if ( !strncmp(s_req, PUT, strlen(PUT)) ) 			sr_req->method = PUT;
		else if ( !strncmp(s_req, PATCH, strlen(PATCH)) ) 		sr_req->method = PATCH;
		else if ( !strncmp(s_req, DELETE, strlen(DELETE)) ) 	sr_req->method = DELETE;
		else if ( !strncmp(s_req, TRACE, strlen(TRACE)) ) 		sr_req->method = TRACE;
		else if ( !strncmp(s_req, LINK, strlen(LINK)) ) 		sr_req->method = LINK;
		else if ( !strncmp(s_req, UNLINK, strlen(UNLINK)) ) 	sr_req->method = UNLINK;
		else if ( !strncmp(s_req, CONNECT, strlen(CONNECT)) )	sr_req->method = CONNECT;
		else {
			return -1;
		}
	return strlen(sr_req->method);
}

int get_uri_and_query(struct request *sr_req, const char *s_req) {
	if (s_req[0] != '/') return -1;

	char *pc_result;
	int result;

	char *pc_line_end = strstr(s_req, HTTP_LINE_ENDING);
	if (!pc_line_end) return -1;

	size_t size;

	pc_result = strchr(s_req, '?');
	if (pc_result && pc_result < pc_line_end) {
		size = pc_result - s_req;
	} else {
		pc_result = strstr(s_req, " HTTP/");
		if (!pc_result) return -1;

		size = pc_result - s_req;
	}

	sr_req->uri = malloc(sizeof(char) * (size + 1));
	result = percent_decode(s_req, size, sr_req->uri, size);
	if ( result == -1) {
		printf("Error decoding URI\n");
		free(sr_req->uri);

		return -1;
	}
	sr_req->uri[result] = '\0';
//TODO query parsing
	return size;
}

int get_http_version(struct request *sr_req, const char *s_req) {
	if 		(memcmp( (s_req), HTTP_09, strlen(HTTP_09))) sr_req->version = HTTP_09;
	else if (memcmp( (s_req), HTTP_10, strlen(HTTP_10))) sr_req->version = HTTP_10;
	else if (memcmp( (s_req), HTTP_11, strlen(HTTP_11))) sr_req->version = HTTP_11;
	else {
		return -1;
	}

	return strlen(sr_req->version);
}


int encode_request(const char *s_req, struct request *sr_req){
	char *ptr = s_req;
	int result;

	result = get_method(sr_req, ptr);
	if (result == -1) {
		printf("Wrong method\n");

		return -1;
	}
	ptr += result;

	result = get_uri_and_query(sr_req, ++ptr);
	if (result == -1) {
		printf("Wrong URI\n");

		return -1;
	}
	ptr += result;

	if (strstr(sr_req->uri, "../")) {
		printf("Request contains ../\n");
		free(sr_req->uri);

		return -1;
	}

	result = get_http_version(sr_req, ++ptr);
	if (result == -1) {
		printf("Wrong version\n");
		free(sr_req->uri);

		return -1;
	}


	if (!strstr(ptr, HTTP_REQ_END)) {
		printf("Request incomplete\n %s\n", s_req);
		free(sr_req->uri);

		return -1;
	}

	//TODO parse "Connection:"
	sr_req->connection = HTTP_CLOSE;

	return 0;
}


static char* get_content_type(char* ext) {
	if 		(!strcmp(ext, "css")) 	return C_TYPE_CSS;
	else if (!strcmp(ext, "html")) 	return C_TYPE_HTML;
	else if (!strcmp(ext, "txt")) 	return C_TYPE_TEXT;
	else if (!strcmp(ext, "js")) 	return C_TYPE_JS;
	else if (!strcmp(ext, "jpg")) 	return C_TYPE_JPG;
	else if (!strcmp(ext, "jpeg")) 	return C_TYPE_JPG;
	else if (!strcmp(ext, "png")) 	return C_TYPE_PNG;
	else if (!strcmp(ext, "gif")) 	return C_TYPE_GIF;
	else if (!strcmp(ext, "tiff")) 	return C_TYPE_TIFF;
	else if (!strcmp(ext, "swf")) 	return C_TYPE_SWF;
	else 							return C_TYPE_MISC;
}

int set_headers(char *headers, char *status, ssize_t filesize, char* ext){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char date[64];

	strftime(date, 64, "%a, %e %b %Y %X %Z\r\n", &tm);

	if (!strcmp(status, HTTP_200)) {
		return sprintf(headers, "%sServer: %sDate:%s"
				"Content-Type: %s"
				"Content-Length: %d\r\n"
				"Connection: %s\r\n",
				HTTP_200,
				SERVER_NAME,
				date,
				get_content_type(ext),
				(int)filesize,
				HTTP_CLOSE);
	} else {
		return sprintf(headers, "%sServer: %sDate: %sConnection: close\r\n\r\n",
				status,
				SERVER_NAME,
				date);
	}

}
