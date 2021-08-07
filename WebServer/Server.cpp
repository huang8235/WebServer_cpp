#include "Server.h"
#include "Util.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include <memory.h>


Server::Server(EventLoop* loop, int threadNum, int port)
	: loop_(loop),
	  threadNum_(threadNum),
	  eventLoopThreadPool_(new EventLoopThreadPool(loop_,threadNum_)),
	  started_(false),
	  acceptChannel_(new Channel(loop_)),
	  port_(port),
	  listenFd_(socket_bind_listen(port_)) {
	acceptChannel_ -> setFd(listenFd_);
	handle_for_sigpipe();
	if(setSocketNonBlocking(listenFd_) < 0) {
		perror("set socket non block failed");
		abort();
	}
}

void Server::start() {
	eventLoopThreadPool_ -> start();
	acceptChannel_ -> setEvents(EPOLLIN | EPOLLET);
	acceptChannel_ -> setReadHandler(std::bind(&Server::handNewConn, this)); //非静态成员函数需要一个隐式的this参数，bind用于绑定二者
	acceptChannel_ -> setConnHandler(std::bind(&Server::handThisConn,this));
	loop_ -> addToPoller(acceptChannel_, 0);
	started_ =true;
}

void Server::handNewConn() {
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len = sizeof(client_addr);
	int accept_fd = 0;
	while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0) {
		EventLoop *loop = eventLoopThreadPool_ -> getNextLoop();

		if(accept_fd >= MAXFDS) {
			close(accept_fd);
			continue;
		}

		if(setSocketNonBlocking(accept_fd) < 0) {
			return;
		}

		setSocketNodelay(accept_fd);

		std::shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
		req_info -> getChannel() -> setHolder(req_info);
		loop -> queueInLoop(std::bind(&HttpData::newEvent, req_info));
	}
	acceptChannel_ -> setEvents(EPOLLIN | EPOLLET);
}