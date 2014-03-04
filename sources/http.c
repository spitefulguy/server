#include "http.h"
#include <stdlib.h>
#include "string.h"
#include "stdio.h"
#include <time.h>

#include "utils.h"
#include "../settings.h"



int encode_request(const char *req_c, struct request *req_r){
	char *ptr;
	char *result_c;
	size_t size;
	int result;

	if 		( !strncmp(req_c, GET, strlen(GET)) ) 			req_r->method = GET;
	else if ( !strncmp(req_c, POST, strlen(POST)) ) 		req_r->method = POST;
	else if ( !strncmp(req_c, HEAD, strlen(HEAD)) ) 		req_r->method = HEAD;
	else if ( !strncmp(req_c, PUT, strlen(PUT)) ) 			req_r->method = PUT;
	else if ( !strncmp(req_c, PATCH, strlen(PATCH)) ) 		req_r->method = PATCH;
	else if ( !strncmp(req_c, DELETE, strlen(DELETE)) ) 	req_r->method = DELETE;
	else if ( !strncmp(req_c, TRACE, strlen(TRACE)) ) 		req_r->method = TRACE;
	else if ( !strncmp(req_c, LINK, strlen(LINK)) ) 		req_r->method = LINK;
	else if ( !strncmp(req_c, UNLINK, strlen(UNLINK)) ) 	req_r->method = UNLINK;
	else if ( !strncmp(req_c, CONNECT, strlen(CONNECT)) )	req_r->method = CONNECT;
	else {
		printf("Wrong method\n");
		return -1;
	}

	ptr = req_c + strlen(req_r->method);

	if (*(++ptr) != '/') {
		printf("Wrong URI\n");
		return -1;
	}

	//TODO parse "Connection:"
	req_r->connection = HTTP_CLOSE;

	result_c = strchr(ptr, '?');
	if (result_c) {
		size = result_c - ptr;
	} else {
		if ((result_c = strstr(ptr, " HTTP/")))
			size = result_c - ptr;
		else
			return -1;
	}

	req_r->uri = (char *)malloc(sizeof(char) * (size + 1));

	//printf("URI to be decoded %.*s\n", (int)size, ptr);
	result = percent_decode(ptr, size, req_r->uri, size);
	if ( result == -1) {
		free(req_r->uri);
		printf("Error decoding URI\n");
		return -1;
	}
	req_r->uri[result] = '\0';

	//printf("Requested path %s\nsize %d\nresult %d", req_r->uri, size, result);

	if (strstr(req_r->uri, "../")) {
		free(req_r->uri);
		return -1;
	}
	//printf("URI after decoding %s\n", req_r->uri);

	ptr += size;
	if 		(memcmp( (++ptr), HTTP_09, strlen(HTTP_09))) req_r->version = HTTP_09;
	else if (memcmp( (++ptr), HTTP_10, strlen(HTTP_10))) req_r->version = HTTP_10;
	else if (memcmp( (++ptr), HTTP_11, strlen(HTTP_11))) req_r->version = HTTP_11;
	else {
		printf("Wrong version\n");
		free(req_r->uri);
		return -1;
	}

	if (!strstr(ptr, HTTP_REQ_END)) {
		printf("Request incomplete\n %s\n", req_c);
		free(req_r->uri);
		return -1;
	}

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
		return sprintf(headers, "%sServer: %sDate: %sContent-Type: %sContent-Length: %d\r\nConnection: close\r\n\r\n",
				HTTP_200,
				SERVER_NAME,
				date,
				get_content_type(ext),
				(int)filesize);
	} else {
		return sprintf(headers, "%sServer: %sDate: %sConnection: close\r\n\r\n",
				status,
				SERVER_NAME,
				date);
	}

	//return 0;
}
