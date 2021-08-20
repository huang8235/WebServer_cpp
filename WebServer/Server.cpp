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
	//将处理接受连接的acceptChannel挂到epoll红黑树上
	loop_ -> addToPoller(acceptChannel_, 0);
	started_ =true;
}

//socket建立新连接
void Server::handNewConn() {
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len = sizeof(client_addr);
	int accept_fd = 0;
	//accept接受新连接，主线程拿到新连接后，将连接socket分发给IO线程
	while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0) {
		EventLoop *loop = eventLoopThreadPool_ -> getNextLoop(); //分发新事件

		//限制服务器的最大并发连接数
		if(accept_fd >= MAXFDS) {
			close(accept_fd);
			continue;
		}

		//设为非阻塞模式
		if(setSocketNonBlocking(accept_fd) < 0) {
			return;
		}

		setSocketNodelay(accept_fd);

		//创建HttpData对象，然后HttpData对象建立并绑定的Channel
		//将新的连接socket分发给下一个loop,也即分发给IO线程
		//新Channel的事件响应函数在上层HttpData对象创建时设置，绑定的是HttpData类里定义的事件相应函数
		std::shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
		req_info -> getChannel() -> setHolder(req_info);
		//分发新事件给loop池中的loop, 向fd中写入数据
		//在这里唤醒IO线程去处理事件
		//通过newEvent函数向唤醒loop的epoll事件表上注册新accept_fd
		loop -> queueInLoop(std::bind(&HttpData::newEvent, req_info));
	}
	acceptChannel_ -> setEvents(EPOLLIN | EPOLLET);
}
