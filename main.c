#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
//#include <sys/stat.h>

#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <signal.h>

//#include <string.h>
//#include <unistd.h>

#include "http/http.h"
//#include <arpa/inet.h>
//#include <fcntl.h>

#include "hnds/hnds.h"

#include <errno.h>
#include <pthread.h>

#define MAX_LISTEN 64
#define MAX_EPOLL_EVENTS 64
#define LOCAL_PORT 8000

int sfd;

int init_socket() {
	int sfd ;
	struct sockaddr_in server_addr;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		return -1;

	char optval = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(LOCAL_PORT);

	if (bind(sfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		return -1;

	return sfd;
}

int make_nonblocking(int fd){
	int flags;

	flags = fcntl(fd, F_GETFD, 0);
	if (flags == -1)
		return -1;

	flags |= O_NONBLOCK;

	if (fcntl(fd, F_SETFD, flags) == -1)
		return -1;

	return 0;
}


int main(int argc, char **argv) {
	struct epoll_event event;
	struct epoll_event events[MAX_EPOLL_EVENTS];
	struct sockaddr_in client_addr;
	int cfd, epfd, nfds;
	int i;
	unsigned int addr_len;

	char recv_buf[IN_BUF_SIZE];
	pthread_t threads[MAX_CONNECTIONS];

	sfd = init_socket();
	if (sfd < 0) {
		perror("Initializing socket");
		return -1;
	}

	if (make_nonblocking(sfd) == -1) {
		perror("Set listener non-blocking");
		return -1;
	}

	int optval = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
	{
		perror("Set socket option error");
		return -1;
	}

	if (listen(sfd, MAX_LISTEN) == -1){
		perror("Socket won't listen");
		return -1;
	}

	epfd = epoll_create(1);
	if (epfd == -1) {
		perror("Create epoll instance");
		return 0;
	}
	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &event) < 0){
		fprintf(stderr, "epoll_ctl failed\n");
		return -1;
	}

	printf("Server has started successfully\n");

	for(;;) {

		nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1) {
			fprintf(stderr, "epoll_wait failed\n");
		}

		for (i = 0; i < nfds; i++) {
			if (events[i].data.fd == sfd) {
				addr_len = sizeof(client_addr);
				cfd = accept(sfd, (struct sockaddr *)&client_addr, &addr_len);
				if (cfd == -1) {
					perror("Connection socket accept");
					continue;
				}

				printf("Connection %d accepted\n", cfd);
				if (make_nonblocking(cfd) == -1){
					perror("Set non-blocking");
					continue;
				}


				event.data.fd = cfd;
				event.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &event) == -1) {
					perror("epoll_ctl(connection socket)");
				}

				int *ptr_cfd = malloc(sizeof(int));
				*ptr_cfd = cfd;
				pthread_create(&threads[cfd], NULL, wait_request, (void *)ptr_cfd);

				continue;
			} else {
				 if (events[i].events & EPOLLIN) {
					   ssize_t count;
					   count = read(events[i].data.fd, recv_buf, sizeof(recv_buf));
					   recv_buf[count] = '\0';
					   printf("%d bytes received from %d\n", count, events[i].data.fd);
					   if (count > 0) {
						   buffer_store(recv_buf, events[i].data.fd, REQ_STORE);
						   //close (events[i].data.fd);
					   }
				 }
			}
		}
	}

	return 0;
}

