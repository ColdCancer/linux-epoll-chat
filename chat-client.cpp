# include "utility.h"

int main(int argc, char *argv[]) {
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("sockfd");
		exit(-1);
	}

	int ret_conn = connect(sockfd,
			(sockaddr*) &server_addr,
			sizeof(server_addr));
	if (ret_conn < 0) {
		perror("connect");
		exit(-1);
	}

	int pipe_fd[2];
	if (pipe(pipe_fd) < 0) {
		perror("pipe");
		exit(-1);
	}

	int epfd = epoll_create(EPOLL_SIZE);
	if (epfd < 0) {
		perror("epfd");
		exit(-1);
	}

	epoll_event events[2];
	addfd(epfd, sockfd, true);
	addfd(epfd, pipe_fd[0], true);

	bool is_client_work = true;
	char message[BUF_SIZE];

	int pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(-1);
	}

	if (pid == 0) {
		close(pipe_fd[0]);
		printf("Please input 'exit' to exit the chat room\n");

		while (is_client_work) {
			memset(message, 0, BUF_SIZE);
			fgets(message, BUF_SIZE, stdin);

			if (strncasecmp(message, "exit", strlen("exit") == 0)) {
				is_client_work = false;
			} else {
				if (write(pipe_fd[1], message, strlen(message) - 1) < 0) {
					perror("fork");
					exit(-1);
				}
			}
		}
	} else {
		close(pipe_fd[1]);

		while (is_client_work) {
			int epoll_events_count = epoll_wait(epfd, events, 2, -1);

			for (int i = 0; i < epoll_events_count; i++) {
				memset(message, 0, BUF_SIZE);
				
				if (events[i].data.fd == sockfd) {
					int ret_recv = recv(sockfd, message, BUF_SIZE, 0);
					if(ret_recv == 0) {
						printf("Server closed connection : %d\n", sockfd);
						close(sockfd);
						is_client_work = false;
					} else {
						printf("%s\n", message);
					}
				} else {
					int ret_read = read(events[i].data.fd, message, BUF_SIZE);
					if (ret_read == 0) {
						is_client_work = false;
					} else {
						send(sockfd, message, BUF_SIZE, 0);
					}
				}
			}
		}
	}
	
	if (pid) {
		close(pipe_fd[0]);
		close(sockfd);
	} else {
		close(pipe_fd[1]);
	}

	return 0;
}






























