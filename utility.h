#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2048
#define EPOLL_SIZE 100
#define BUF_SIZE 512

#define SERVER_MESSAGE "ClientID %d say >> %s"
#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is: Client #%d"
#define CAUTION "Threr is only one int the char room!"

using namespace std;

list<int> client_list;

int setnonblocking(int sockfd) {
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
	return 0;
}

void addfd(int epollfd, int fd, bool enable_et) {
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	if (enable_et) ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	setnonblocking(fd);
	printf("fd added to epoll!\n\n");
}

int sendBroadcastMessage(int clientfd) {
	char buf[BUF_SIZE], message[BUF_SIZE];
	memset(buf, 0, sizeof(buf));
	memset(message, 0, sizeof(message));
	
	printf("read from client(ID = %d)\n", clientfd);
	int len = recv(clientfd, buf, BUF_SIZE, 0);

	if (len == 0) {
		close(clientfd);
		client_list.remove(clientfd);
		printf("clientID = %d closed.\n now there are %d client in the char room\n", clientfd, (int)client_list.size());
	} else {
		if (client_list.size() == 1) {
			send(clientfd, CAUTION, strlen(CAUTION), 0);
			return len;
		}

		sprintf(message, SERVER_MESSAGE, clientfd, buf);

		list<int>::iterator it;
		for (it = client_list.begin(); it != client_list.end(); it++) {
			if (*it != clientfd) {
				if( send(*it, message, BUF_SIZE, 0) < 0) {
					perror("error");
					exit(-1);
				}
			}
		}
	}
	return len;
}

#endif
