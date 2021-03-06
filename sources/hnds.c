

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
//#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>

#include "../headers/settings.h"
#include "../headers/hnds.h"
#include "../headers/http.h"
#include "../headers/utils.h"
#include "../headers/queue.h"
#include "../headers/config.h"

#define HEADERS_SIZE 512

static void *get_dynamic_handler(struct request *sr_request) {
	return NULL;
}

void close_connection(int fd, char *status, int thread) {
	if (status) {
		int result;
		char headers[HEADERS_SIZE];
		result = set_headers(headers, status, 0, NULL);
		write(fd, headers, result);
	}
#ifdef DEBUG
	printf("Closing connection (%d)\n", fd);
#endif

	close(fd);
}

static int get_headers(char *headers, char *status, char *ext, ssize_t size){
	return set_headers(headers, status, size, ext);
}

static ssize_t send_data(int fd, void *data, size_t length) {
	ssize_t bytes_sent = 0;
	ssize_t to_send;

	to_send = length;
	while (1){
		bytes_sent = send(fd, data, to_send, MSG_NOSIGNAL);
		if ( -1 == bytes_sent ){
			//perror("send");
			if (errno != EAGAIN) return -1;
			bytes_sent = 0;
		}
		if ( length == bytes_sent ) break;

		to_send -= bytes_sent;
		if ( 0 == to_send) break;
		data += bytes_sent;

		struct timeval interval;
		interval.tv_sec = 0;
		interval.tv_usec = get_send_retry_interval_usec();
		fd_set set;
		FD_SET(fd, &set);
		select(2, NULL, &set, NULL, &interval);

	}

	return bytes_sent;
}

static ssize_t open_file(struct stat *st_buf, int *source_d_buf, const char *path) {

	*source_d_buf = open(path, O_RDONLY);
	if ( -1 == *source_d_buf) {
		return -1;
	}

	if ( -1 == fstat(*source_d_buf, st_buf)) {
		perror("fstat");
		return -1;
	}
	return 0;

}

static void *map_file(int file_d, struct stat *st) {
	void *file;
	file = mmap(NULL, st->st_size, PROT_READ, MAP_PRIVATE, file_d, 0);
	if (file == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}
	close(file_d);
	return file;
}


static ssize_t static_handler(int fd, struct request *sr_request, int thread) {
	char headers[HEADERS_SIZE];
	int headers_size;
	int source_d ;
	void *file_map;
	struct stat st;
	ssize_t result;
	ssize_t result_total = 0;
	char ext[10];

	const char *static_dir = get_static_dir();
	const char *default_page = get_default_page();

	char *fullpath;

	fullpath = malloc((strlen(static_dir) + strlen(sr_request->uri) + strlen(default_page) + 1 )*sizeof(char));
	sprintf(fullpath, "%s%s", static_dir, sr_request->uri);

	if ( -1 == open_file(&st, &source_d, fullpath) ){
		close_connection(fd, HTTP_404, thread);
		return 0;
	}

	if (S_ISDIR(st.st_mode)) {
		close(source_d);
		strcat(fullpath, default_page);
		if ( -1 == open_file(&st, &source_d, fullpath) ){
			close_connection(fd, HTTP_403, thread);
			return 0;
		}
	}

	get_extension(ext, fullpath);

	headers_size = get_headers(headers, HTTP_200, ext, st.st_size);
	result = send_data(fd, headers, headers_size);
	if ( -1 == result) {
#ifdef DEBUG
		printf("Error while sending headers for %s (%d)\n", fullpath, fd);
#endif
		close_connection(fd, NULL, thread);
		return -1;
	}
	result_total += result;

	file_map = map_file(source_d, &st);
	if (!file_map) {
		close_connection(fd, HTTP_500, thread);
	}

	if (!strstr(sr_request->method, HEAD)) {
		result = send_data(fd, file_map, st.st_size);
		if ( -1 == result) {
#ifdef DEBUG
			printf("Error while sending file %s (%d)\n", fullpath, fd);
#endif
			close_connection(fd, NULL, thread);
			return -1;
		}
	}
	result_total += result;

	close_connection(fd, NULL, thread);
	return result_total;
}


int validate_method(char *method) {
	if (strstr(get_methods_to_forbid(), method))
		return 0;
	else
		return 1;
}


int common_handler(const char *s_request, int fd, int thread) {
	struct request sr_request;

	int encode_status;

	encode_status = encode_request(s_request, &sr_request);

	if (encode_status == -1) {
		close_connection(fd, HTTP_400, thread);
		return 0;
	}

	if (!validate_method(sr_request.method)) {
		close_connection(fd, HTTP_405, thread);
		return 0;
	}

	void *dynamic_handler;
	dynamic_handler = get_dynamic_handler(&sr_request);
	if (dynamic_handler) {
		//TODO call dynamic handler
		return 0;
	}


	static_handler(fd, &sr_request, thread);
	free(sr_request.uri);

return 0;


}





