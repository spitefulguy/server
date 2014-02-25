#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
//#include <sys/stat.h>

#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

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
#define MAX_CONNECTIONS 256

#define IN_BUF_SIZE 512

int sfd;

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

void signal_callback_handler(int signum) {

	printf("Caught signal %d\n", signum);
	close(sfd);
	exit(signum);

}

int main(int argc, char **argv) {
	signal(SIGINT, signal_callback_handler);

	struct epoll_event event;
	struct epoll_event events[MAX_EPOLL_EVENTS];
	struct sockaddr_in client_addr;
	int cfd, epfd, nfds;
	int i, n;
	unsigned int addr_len;

	char recv_buf[IN_BUF_SIZE];

	sfd = init_socket();
	if (sfd < 0) {
		perror("Initializing socket");
		return -1;
	}

	if (make_nonblocking(sfd) == -1) {
		perror("Set listener non-blocking");
		return -1;
	}

	if (listen(sfd, MAX_LISTEN) == -1){
		perror("Socket won't listen");
		return -1;
	}

	epfd = epoll_create(1);
	if (epfd == -1) {
		perror("Craate epoll instance");
		return 0;
	}
	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &event) < 0){
		fprintf(stderr, "epoll_ctl failed\n");
		return -1;
	}

	printf("Server has started sucessfully\n");

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

				printf("Connection accepted (%d)\nClient's IP is %d\n",cfd, inet_ntoa(client_addr.sin_addr.s_addr));
				if (make_nonblocking(cfd) == -1){
					perror("Set non-blocking");
					continue;
				}

				event.data.fd = cfd;
				event.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &event) == -1) {
					perror("epoll_ctl(connection socket)");
				}

				continue;
			} else {
				 if (events[i].events & EPOLLIN) {
					   ssize_t count;
					   count = read(events[i].data.fd, recv_buf, sizeof(recv_buf));
					   printf("Count %d\n", count);
					   if (count > 0) {
						   printf("Data received (%d): \n%s\nEnd of request\n", events[i].data.fd, recv_buf);
						   //recv_buf[count - 1] = '\0';
						   common_handler(recv_buf, events[i].data.fd);
						   close (events[i].data.fd);
					   }
				 }
			}
		}
	}

	return 0;
}

