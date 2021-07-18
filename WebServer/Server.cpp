#include "Server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>


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
