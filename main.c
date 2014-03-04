#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include "hnds.h"

#include <errno.h>
#include <pthread.h>

#include "settings.h"
#include "buffer.h"


#define PARAM_UNUSED 1

int init_socket() {
	int sfd ;
	struct sockaddr_in server_addr;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		return -1;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(LOCAL_PORT);

	if (bind(sfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		return -1;

	return sfd;
}

int set_nonblocking(int fd){
	int flags;

	flags = fcntl(fd, F_GETFD, 0);
	if (flags == -1)
		return -1;

	flags |= O_NONBLOCK;

	if (fcntl(fd, F_SETFD, flags) == -1)
		return -1;

	return 0;
}

int accept_connection(int sfd, int epfd){
	unsigned int addr_len;
	struct sockaddr_in client_addr;
	int cfd;

	addr_len = sizeof(client_addr);
	cfd = accept(sfd, (struct sockaddr *)&client_addr, &addr_len);
	if (cfd == -1) {
		return -1;
		perror("Connection socket accept");
	}

	//printf("Connection %d accepted\n", cfd);
	if (set_nonblocking(cfd) == -1){
		return -1;
		perror("Set non-blocking");
	}

	struct epoll_event event;
	event.data.fd = cfd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &event) == -1) {
		perror("epoll_ctl(connection socket)");
		return -1;
	}

	access_connections_list(cfd, 0, ACT_STORE);

	return cfd;
}

int main(int argc, char **argv) {
	struct epoll_event events[MAX_EPOLL_EVENTS];
	int epfd, nfds;
	int sfd;

	char recv_buf[IN_BUF_SIZE];
	pthread_t threads[WORKERS];

	sfd = init_socket();
	if (sfd < 0) {
		perror("Initializing socket");
		return -1;
	}

	if (set_nonblocking(sfd) == -1) {
		perror("Set listener non-blocking");
		return -1;
	}

	if (listen(sfd, MAX_LISTEN) == -1){
		perror("Socket won't listen");
		return -1;
	}

	epfd = epoll_create(PARAM_UNUSED);
	if (epfd == -1) {
		perror("Create epoll instance");
		return 0;
	}

	struct epoll_event event;
	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &event) < 0){
		fprintf(stderr, "epoll_ctl failed\n");
		return -1;
	}

	int i;
	for (i = 0; i < WORKERS; i++) {
		pthread_create(&threads[i], NULL, wait_for_request, (void *)i);
	}
	printf("Server has started successfully\n");

	for(;;) {
		nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1) {
			fprintf(stderr, "epoll_wait failed\n");
		}

		int j;
		for (j = 0; j < nfds; j++) {
			if (events[j].data.fd == sfd) {
				accept_connection(sfd, epfd);
			} else {
				 if (events[j].events & EPOLLIN) {
					ssize_t bytes_sent;
					bytes_sent = read(events[j].data.fd, recv_buf, sizeof(recv_buf));
					recv_buf[bytes_sent] = '\0';
					//printf("%s\n\n", recv_buf);
					if (bytes_sent > 0) {
					   access_buffer(NULL, recv_buf, bytes_sent, events[j].data.fd, ACT_STORE);
					}
				}
			}
		}
	}

	return 0;
}

