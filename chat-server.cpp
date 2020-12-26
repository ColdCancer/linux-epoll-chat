#include "utility.h"

void init_server_config(int &sockfd, int &ret, int &epfd) {
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(-1);
	}
	printf("socket created.\n");

	ret = listen(sockfd, 10);
	if (ret < 0) {
		perror("listen");
		exit(-1);
	}
	printf("Start to listen: %s\n", SERVER_IP);

	epfd = epoll_create(EPOLL_SIZE);
	if (epfd < 0) {
		perror("epoll");
		exit(-1);
	}
	printf("epoll created, epollfd = %d\n", epfd);

	//static struct epoll_event events[EPOLL_SIZE];

	addfd(epfd, sockfd, true);

	return;
}

int main(int argc, char *argv[]) {
	int listener, ret, epfd;
	init_server_config(listener, ret, epfd);
	
	static struct epoll_event events[EPOLL_SIZE];

	while (true) {
		int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if (epoll_events_count < 0) {
			perror("epoll failure");
			break;
		}
		printf("epoll_event_count = %d\n", epoll_events_count);

		for (int i = 0; i < epoll_events_count; i++) {
			int sockfd = events[i].data.fd;
			if(sockfd == listener) {
				struct sockaddr_in client_address;
				socklen_t client_addrLength = sizeof(struct sockaddr_in);
				int clientfd = accept(listener, (struct sockaddr*) &client_address, &client_addrLength);

				printf("client connection fron: %s : %d (IP : port), clientfd = %d\n",
						inet_ntoa(client_address.sin_addr),
						ntohs(client_address.sin_port),
						clientfd);

				addfd(epfd, clientfd, true);

				client_list.push_back(clientfd);
				printf("Add new clientfd = %d to epoll\n", clientfd);
				printf("Now there are %d clients int the char room\n", (int)client_list.size());
				printf("werlcome message\n");

				char message[BUF_SIZE];
				memset(message, 0, sizeof(message));
				sprintf(message, SERVER_WELCOME, clientfd);

				int ret = send(clientfd, message, BUF_SIZE, 0);
				if (ret < 0) {
					perror("send error");
					exit(-1);
				}
			} else {
				int ret = sendBroadcastMessage(sockfd);
				if (ret < 0) {
					perror("send.. error");
					exit(-1);
				}
			}
		}
	}
	close(epfd);
	close(listener);
	return 0;
}


