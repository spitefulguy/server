#include "hnds.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../settings.h"
#include "../http/http.h"
#include "../utils/utils.h"

#define HEADERS_SIZE 512

static ssize_t send_file(int fd, char* filename, char *ext) {
	char fullpath[strlen(filename) + strlen(ROOT) + 1];

	sprintf(fullpath, "%s%s", ROOT, filename);

	int file = open(fullpath, O_RDONLY);
	if (file == -1) {
		fprintf(stderr, "File %s not found\n", fullpath);
		write(fd, "HTTP/1.1 404 Not Found\n\n", 100);
		return -1;
	}

	char *content;
	char *response;
	char headers[HEADERS_SIZE];
	ssize_t response_size;
	int headers_size;
	struct stat st;


	fstat(file, &st);

	headers_size = set_headers(headers, fullpath, st.st_size, ext);
	response_size = st.st_size + headers_size + 20;


	content = (char *)malloc( sizeof(char) * (st.st_size + 1) );
	response = (char *)malloc( sizeof(char)* response_size);

	printf("Memmory allocated for response %d\n", response_size);
	printf("Memory allocated for content %d\n", (int)(st.st_size + 1));

	if (!content || !response) {
		fprintf(stderr, "malloc failed\n");
	}

	printf("Bytes read %d\n", read(file, content, st.st_size - 2)) ;
	//content[st.st_size] = '\0';

	close(file);

	printf("Bytes printed into response %d\n", sprintf(response, "%s%s", headers, content));

	write(fd, response, response_size);
	close(fd);
	//fprintf(stderr, "Bytes sent (send_file)%d\n", response_size);

	free(content);
	free(response);

	return response_size;
}

void common_handler(char *request, int fd) {
	struct request *req = encode_request(request);
	if (!req) {
		fprintf(stderr, "Not a valid HTTP requset\n");
		return;

	}
	printf("URL requested %s\n", req->url);

	char *ext;

	ext = get_extension(req->url);
	//printf("For file %s extension defined %s\n", req->url, ext);
	if (ext)
		printf("Bytes sent %d\n", send_file(fd, req->url, ext));
}


