#include "http.h"
#include "../utils/utils.h"
#include <stdlib.h>
#include "string.h"
#include "stdio.h"
#include <time.h>

struct request* encode_request(char *request){
	int i;
	char *ptr;
	struct request *req = malloc(sizeof(struct request));

	if (strstr(request,"GET")) req->method = GET;
	else if (strstr(request, "POST")) req->method = POST;
	else {
		free(req);
		return NULL;
	}

	ptr = strchr(request, '/');
	req->url = malloc(sizeof(char) * (strchr(ptr, ' ') - ptr) + 1);

	i = 0;
	do{
		req->url[i] = ptr[i];
	} while( ptr[++i] != ' ');
	req->url[i] = '\0';

	return req;
}


static char* get_content_type(char* ext) {
	//fprintf(stderr, "Format type to identify is %s\n", ext);
	char *content_type =  malloc(sizeof(char)*30);
	if (!strcmp(ext, "css")) content_type = "text/css";
	else if (!strcmp(ext, "html")) content_type = "text/html";
	else if (!strcmp(ext, "js")) content_type = "application/javascript";
	else if (!strcmp(ext, "jpg")) content_type = "image/jpeg";
	else if (!strcmp(ext, "jpeg")) content_type = "image/jpeg";
	else if (!strcmp(ext, "png")) content_type = "image/png";
	else if (!strcmp(ext, "gif")) content_type = "image/gif";
	else if (!strcmp(ext, "tiff")) content_type = "image/tiff";
	else content_type = "application/misc";
	//	text/csv: CSV (RFC 4180)
	//	text/plain: текстовые данные (RFC 2046 и RFC 3676)
	//	text/xml: Extensible Markup Lang
	//	image/pjpeg: JPEG[7]
	//	image/png: Portable Network Graphics[8](RFC 2083)
	//	image/svg+xml: SVG[9]
	//	image/vnd.microsoft.icon: ICO[10]
	//	image/vnd.wap.wbmp: WBMP
	//	text/cmd: команды
	//	text/csv: CSV (RFC 4180)
	//	text/plain: текстовые данные (RFC 2046 и RFC 3676)
	//	text/xml: Extensible Markup Lang
	return content_type;
}

int set_headers(char *headers, const char *filename, ssize_t filesize, char* ext){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	int offset;

	offset = sprintf(headers, "HTTP/1.1 200 Ok\nServer: MyServer\nDate: %s\n", weekday(tm.tm_year, tm.tm_mon, tm.tm_mday));
	offset += sprintf(headers + offset, "Content-type: %s; charset=UTF-8\n", get_content_type(ext));
	offset += sprintf(headers + offset, "Content-length: %d\n", filesize + 2);

	strcat(headers, "\n");

	return offset + 1;
}
