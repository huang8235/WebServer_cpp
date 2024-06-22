#include "Util.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

const int MAX_BUFF = 4096;
ssize_t readn(int fd, void* buff, size_t n) {
	size_t nleft = n;
	ssize_t nread = 0;
	ssize_t readSum = 0;
	char* ptr = (char*)buff;
	while(nleft > 0) {
		if((nread = read(fd, ptr, nleft)) < 0) {
			if(errno == EINTR)
				nread = 0;
			else if(errno == EAGAIN) {
				return readSum;
			}
			else {
				return -1;
			}
		}
		else if(nread == 0)	break;
		readSum += nread;
		nleft -= nread;
		ptr += nread;
	}
	return readSum;
}

ssize_t readn(int fd, std::string &inBuffer, bool &zero) {
	ssize_t nread = 0;
	ssize_t readSum = 0;
	while(true) {
		char buff[MAX_BUFF];
		if((nread = read(fd, buff, MAX_BUFF)) < 0) {
			if(errno == EINTR)
				continue;
			else if(errno == EAGAIN) {
				return readSum;
			}
			else {
				perror("read error");
				return -1;
			}
		}
		else if(nread == 0) {
			zero = true;
			break;
		}
		readSum += nread;
		inBuffer += std::string(buff, buff + nread);
	}
	return readSum;
}

ssize_t readn(int fd, std::string &inBuffer) {
	ssize_t nread = 0;
	ssize_t readSum = 0;
	while(true) {
		char buff[MAX_BUFF];
		if((nread = read(fd, buff, MAX_BUFF)) < 0) {
			if(errno == EINTR)
				continue;
			else if(errno == EAGAIN) {
				return readSum;
			}
			else {
				perror("read error");
				return -1;
			}
		}
		else if(nread == 0) {
			break;
		}
		readSum += nread;
		inBuffer += std::string(buff, buff + nread);
	}
	return readSum;
}

ssize_t writen(int fd, void *buff, size_t n) {
	size_t nleft = n;
	ssize_t nwritten = 0;
	ssize_t writeSum = 0;
	char* ptr = (char*) buff;
	while(nleft > 0) {
		if((nwritten = write(fd, ptr, nleft)) <= 0) {
			if(nwritten < 0) {
				if(errno == EINTR) {
					nwritten = 0;
					continue;
				}
				else if(errno == EAGAIN) {
					return writeSum;
				}
				else {
					return -1;
				}
			}
		}
		writeSum += nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}	
		return writeSum;
}

ssize_t writen(int fd, std::string &sbuff) {
	size_t nleft = sbuff.size();
	ssize_t nwritten = 0;
	ssize_t writeSum = 0;
	const char* ptr = sbuff.c_str();
	while(nleft > 0) {
		if((nwritten = write(fd, ptr, nleft)) <= 0) {
			if(nwritten < 0) {
				if(errno == EINTR) {
					nwritten = 0;
					continue;
				}
				else if(errno == EAGAIN)	break;
				else						return -1;
			}
		}
		writeSum += nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}
	if(writeSum == static_cast<int>(sbuff.size()))
		sbuff.clear();
	else
		sbuff = sbuff.substr(writeSum);
	return writeSum;
}

void handle_for_sigpipe() {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE, &sa, NULL)) return;
}

int setSocketNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);	//获取fd的状态标志
	if (flag == -1)
	{
		std::cout << "get flag failed! fd = " << fd << std::endl;
		return -1;
	}

	flag |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flag) == -1)
	{
		std::cout << "set flag failed!" << std::endl;
		return -1;
	}
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
	std::cout << "port = " << port << std::endl;
	if(port < 0 || port > 65535) return -1;

	int listen_fd = 0;
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cout << "socket fail" << std::endl;
		return -1;
	}
	//重用本地地址
	int optval = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		close(listen_fd);
		std::cout << "setsockopt fail" << std::endl;
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((uint16_t)port);
	if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind fail");
		std::cout << "sin_family = " << server_addr.sin_family << std::endl;
		std::cout << "s_addr = " << server_addr.sin_addr.s_addr << std::endl;
		std::cout << "sin_port = " << server_addr.sin_port << std::endl;
		close(listen_fd);
		return -1;
	}

	//开始监听
	if(listen(listen_fd, 2048) == -1) {
		close(listen_fd);
		std::cout << "listen fail" << std::endl;
		return -1;
	}

	//无效监听描述符
	if(listen_fd == -1) {
		close(listen_fd);
		std::cout << "Invalid fd" << std::endl;
		return -1;
	}

	return listen_fd;
}
