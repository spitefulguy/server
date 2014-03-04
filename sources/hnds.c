#include <hnds.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/sendfile.h>


#include "../settings.h"
#include "http.h"
#include "utils.h"
#include "buffer.h"

#define HEADERS_SIZE 512

void close_connection(int fd, char *status, int thread) {
	if (status) {
		int result;
		char headers[HEADERS_SIZE];
		result = set_headers(headers, status, 0, NULL);
		write(fd, headers, result);
	}
	//printf("Closing connection %d\n", fd);
	close(fd);
	if (thread != -1) {
		access_connections_list(fd, thread, ACT_DELETE);
	}
}

static int get_headers(char *headers, char *status, char *ext, ssize_t size){
	return set_headers(headers, status, size, ext);
}

static ssize_t get_file(int *file, char* filename, char *ext) {

	char fullpath[strlen(filename) + strlen(ROOT) + 1];

	sprintf(fullpath, "%s%s", ROOT, filename);
	printf("File requested %s\n",  fullpath );

	*file = open(fullpath, O_RDONLY);
	if (*file == -1) {
		return 0;
	}

	struct stat st;
	fstat(*file, &st);
	//file = mmap(file, st.st_size, PROT_READ, MAP_PRIVATE, source, 0);
	//close(source);
	return st.st_size;

}


int validate_method(char *method) {
	return strstr(METHODS_TO_FORBID, method);
}

int common_handler(const char *s_request, int fd, int thread) {
	struct request sr_request;
	//char *status;
	char ext[10];
	int file;
	char headers[HEADERS_SIZE];
	ssize_t result;
	ssize_t fsize;
	int hsize;
	//char *response;


	int encode_status;
	encode_status = encode_request(s_request, &sr_request);
	printf("Request Encoded\n");

	if (encode_status == -1) {
		close_connection(fd, HTTP_400, thread);
		return 0;
	}
	if (validate_method(sr_request.method)) {
		close_connection(fd, HTTP_405, thread);
		return 0;
	}

	int ext_length;
	ext_length = get_extension(ext, sr_request.uri);
	//printf("Got ext\n");

	if (ext_length){

		fsize = get_file(&file, sr_request.uri, ext);
		if (fsize) {
			hsize = get_headers(headers, HTTP_200, ext, fsize);
		} else {
			hsize = get_headers(headers, HTTP_404, NULL, 0);
		}

	} else {

		char uri[strlen(sr_request.uri) + strlen(DEFAULT_PAGE) + 1];
		sprintf(uri, "%s%s", sr_request.uri, DEFAULT_PAGE );
		get_extension(ext, DEFAULT_PAGE);
		fsize = get_file(&file, uri, ext);
		if (fsize) {
			hsize = get_headers(headers, HTTP_200, ext, fsize);
		} else {
			hsize = get_headers(headers, HTTP_404, NULL, 0);
		}
	}
	//free(sr_request.uri);

	//printf("Got file\n");

	result = write(fd, headers, hsize);
	if (strcmp(sr_request.method, HEAD))
		result += sendfile(fd, file, NULL, fsize);

	printf("Bytes sent (%d) %u\n", fd,  (unsigned int)result );

	close_connection(fd, NULL, thread);

	close(file);
	//
	return (int)result;
}





