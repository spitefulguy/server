#define _GNU_SOURCE

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/http.h"
#include "headers/hnds.h"

#include <errno.h>
#include <pthread.h>

#include "settings.h"
#include "queue.h"


#define PARAM_UNUSED 1

int init_socket() {
	int sfd ;
	struct sockaddr_in server_addr;

	sfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sfd == -1)
		return -1;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(LOCAL_PORT);

	if (bind(sfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		return -1;

	return sfd;
}

int accept_connection(int sfd, int epfd){
	unsigned int addr_len;
	struct sockaddr client_addr;
	addr_len = sizeof(client_addr);
	int cfd;

	while (1) {
		cfd = accept4(sfd, (struct sockaddr *)&client_addr, &addr_len, SOCK_CLOEXEC | SOCK_NONBLOCK);
		if (cfd == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			  return 0;
			}
			perror("Connection socket accept");
			return -1;
		}

		struct epoll_event event;
		event.data.fd = cfd;
		event.events = EPOLLIN | EPOLLET;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &event) == -1) {
			perror("epoll_ctl(connection socket)");
			return -1;
		}

#ifdef DEBUG
		char s_host[56];
		char s_serv[56];
		int status = getnameinfo(&client_addr, addr_len,
				s_host, sizeof(s_host),
				s_serv, sizeof(s_serv),
				NI_NUMERICHOST | NI_NUMERICSERV);
		if (0 == status) {
			printf("Accepted connection on descriptor %d "
			"(host=%s, port=%s)\n", cfd, s_host, s_serv);
		} else {
			printf("Connection %d accepted\n", cfd);
		}
#endif
	}

	return 0;
}

int main(int argc, char **argv) {
	struct epoll_event events[MAX_EPOLL_EVENTS];
	int epfd;
	int sfd;

	char recv_buf[MAX_REQUEST_LENGTH];
	pthread_t threads[THREADS];

	sfd = init_socket();
	if (sfd < 0) {
		perror("Initializing socket");
		return -1;
	}

	int set = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&set, sizeof(int)) == -1) {
		perror("Set REUSEADDR");
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
	for (i = 0; i < THREADS; i++) {
		pthread_create(&threads[i], NULL, wait_for_request, (void *)i);
	}
	queue_init();

	printf("Server has started successfully\n");

	for(;;) {
		int nfds;
		nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			continue;
		}

		int j;
		for (j = 0; j < nfds; j++) {
			if ((events[j].events & EPOLLERR) ||
				  (events[j].events & EPOLLHUP) ||
				  (!(events[j].events & EPOLLIN))) {
#ifdef DEBUG
				perror("epoll");
#endif
				continue;
			}

			if (events[j].data.fd == sfd) {
				accept_connection(sfd, epfd);
				continue;
			} else {
				 if (events[j].events & EPOLLIN) {
					 while (1) {
						ssize_t bytes_read;
						bytes_read = recv(events[j].data.fd, recv_buf, sizeof(recv_buf), 0);

						if (bytes_read == -1) {
							if (errno != EAGAIN && errno != EWOULDBLOCK){
								perror ("recv");
							}

							break;

						} else if (bytes_read == 0){
#ifdef DEBUG
							printf("Client closed connection (%d) [main]\n", events[j].data.fd);
#endif
							close(events[j].data.fd);
							break;
						}

						recv_buf[bytes_read] = '\0';
						//printf("%s\n", recv_buf);
#ifdef DEBUG
						printf("Request received (%d) [main]\n", events[j].data.fd);
#endif
						queue_push(recv_buf, bytes_read, events[j].data.fd);
					 }
				 }
			}
		}
	}
	return 0;
}

