#include "Util.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const int MAX_BUFF = 4096;

void handle_for_sigpipe() {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE, &sa, NULL)) return;
}

int setSocketNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);	//获取fd的状态标志
	if (flag == -1) return -1;

	flag |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flag) == -1)	return -1;
	return 0;
}

void setSocketNodelay(int fd) {
	int enable = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable)); //禁止Nagle算法
}

void setSocketNoLinger(int fd) {
	struct linger linger_;
	linger_.l_onoff = 1;
	linger_.l_linger = 30;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&linger_, sizeof(linger_));
}

void shutDownWR(int fd) {
	shutdown(fd, SHUT_WR);
}

int socket_bind_listen(int port) {
	//检查port值，取正确区范围
	if(port < 0 || port > 65535) return -1;

	int listen_fd = 0;
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;

	//重用本地地址
	int optval = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		close(listen_fd);
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		close(listen_fd);
		return -1;
	}

	//开始监听
	if(listen(listen_fd, 2048) == -1) {
		close(listen_fd);
		return -1;
	}

	//无效监听描述符
	if(listen_fd == -1) {
		close(listen_fd);
		return -1;
	}

	return listen_fd;
}
