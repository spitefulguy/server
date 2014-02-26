#include "hnds.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#include "../settings.h"
#include "../http/http.h"
#include "../utils/utils.h"

#define HEADERS_SIZE 512

static ssize_t send_file(int fd, char* filename, char *ext) {
	int len = (int)(strchr(filename, ' ') - filename);

	printf("File requested %.*s\n", len, filename );

	char fullpath[len + strlen(ROOT) + 1];
	int result;

	strcpy(fullpath, ROOT);
	strncat(fullpath, filename, len);

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
	response_size = st.st_size + headers_size + 2;

	content = 	(char *)malloc( sizeof(char) *(st.st_size + 1) );
	response = 	(char *)malloc( sizeof(char)* response_size);

	if (!content || !response) {
		fprintf(stderr, "malloc failed\n");
		return -1;
	}

	result = read(file, content, st.st_size - 2);
	content[result] = '\0';

	close(file);
	result = sprintf(response, "%s%s", headers, content);

	write(fd, response, result);
	//close(fd);
	//fprintf(stderr, "Bytes sent (send_file)%d\n", response_size);

	free(content);
	free(response);

	return result;
}

int common_handler(struct Request *req_r, int fd) {
	char *ext;
	ext = get_extension(req_r->uri);
	if (ext)
		return send_file(fd, req_r->uri, ext);
	else {
		//printf("Default page loaded\n");
		return send_file(fd, "/index.html ", "html");
	}

}

void *wait_request(void *fd) {
	int *cfd = fd;
	printf("Thread created for %d\n", *cfd);
	struct Request *request;
	while (!(request = buffer_store(NULL, *cfd, REQ_OBTAIN)))
		;
	common_handler(request, *cfd);

	printf("Closing connection %d\n", *cfd);
	close(*cfd);
	free(fd);
	pthread_exit(NULL);
	return 0;
}

struct Request *buffer_store(char *buf, int cfd, int action) {
	static char bufs[MAX_CONNECTIONS][IN_BUF_SIZE];
	static struct Request reqs[MAX_CONNECTIONS];
	static int conditions[MAX_CONNECTIONS];
	int status;

	if (action == REQ_STORE && conditions[cfd] == REQ_COND_EMPTY) {
		strcpy(bufs[cfd], buf);
		conditions[cfd] = REQ_COND_RAW;
		return NULL;
	}
	if (action == REQ_STORE && conditions[cfd] == REQ_COND_INCOMP) {
			strcat(bufs[cfd], buf);
			conditions[cfd] = REQ_COND_RAW;
			return NULL;
		}

	if (action == REQ_OBTAIN && conditions[cfd] == REQ_COND_RAW) {
		status = encode_request(bufs[cfd], &reqs[cfd]);
		if (status == -1)
			{
				close(cfd);
				conditions[cfd] = REQ_COND_EMPTY;
				return NULL;
			}
		if (status == 1) {
			conditions[cfd] = REQ_COND_EMPTY;
			return &reqs[cfd];
		}
		if (status == 0) {
			conditions[cfd] = REQ_COND_INCOMP;
			return NULL;
		}
	} else {
		return NULL;
	}
	return NULL;
}

