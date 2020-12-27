#include "utility.h"

int main(int argc, char *argv[]) {
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0) {
		perror("server-socket");
		exit(-1);
	}
	printf("server socket created.\n");

	int ret_bind = bind(server_sockfd, 
			(sockaddr *) &server_addr, 
			sizeof(server_addr));
	if (ret_bind < 0) {
		perror("bind");
		exit(-1);
	}
	printf("server bind seccessed.\n");

	int ret_listen = listen(server_sockfd, 10);
	if (ret_listen < 0) {
		perror("listen");
		exit(-1);
	}
	printf("start to listen: %s\n", SERVER_IP);

	int epfd = epoll_create(EPOLL_SIZE);
	if (epfd < 0) {
		perror("epoll-create");
		exit(-1);
	}
	printf("epoll created, epollfd = %d\n", epfd);

	epoll_event events[EPOLL_SIZE];
	addfd(epfd, server_sockfd, true);

	while (true) {
		int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if (epoll_events_count < 0) {
			perror("epoll_event");
			exit(-1);
		}
		printf("epoll_events_cout = %d\n", epoll_events_count);

		for (int i = 0; i < epoll_events_count; i++) {
			int sockfd = events[i].data.fd;
			if (sockfd == server_sockfd) {
				sockaddr_in client_addr;
				socklen_t client_addr_length = sizeof(sockaddr_in);

				int client_sockfd = accept(server_sockfd, 
						(sockaddr*) &client_addr, 
						&client_addr_length);
				printf("client connection from: %s : %d, clientfd = %d\n",
						inet_ntoa(client_addr.sin_addr),
						ntohs(client_addr.sin_port),
						client_sockfd);

				addfd(epfd, client_sockfd, true);

				client_list.push_back(client_sockfd);
				printf("Add new clientfd = %d to epoll.\n", client_sockfd);
				printf("Now there are %d clients in the chat room\n");

				printf("welcome message");
				char message[BUF_SIZE];
				memset(message, 0, BUF_SIZE);
				sprintf(message, SERVER_WELCOME, client_sockfd);

				int ret_send = send(client_sockfd, message, BUF_SIZE, 0);
				if (ret_send < 0) {
					perror("send");
					exit(-1);
				}
			} else {
				int ret_tmp = sendBroadcastMessage(sockfd);
				if (ret_tmp < 0) {
					perror("other send");
					exit(-1);
				}
			}
		}
	}
	close(epfd);
	close(server_sockfd);
	return 0;
}

