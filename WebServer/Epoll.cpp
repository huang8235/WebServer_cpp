#include "Epoll.h"
#include "Util.h"
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <deque>
#include <queue>
#include <arpa/inet.h>
#include <iostream>

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef std::shared_ptr<Channel> SP_Channel;

// 创建一个epoll实例
Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
	assert(epollFd_ > 0);
}

Epoll::~Epoll() {}

// 注册新描述符
void Epoll::epoll_add(SP_Channel request, int timeout) {
	int fd = request->getFd();
	if (timeout > 0) {
		add_timer(request, timeout);
		// 添加到fd和httpdata对应表
		fd2http_[fd] = request->getHolder();
	}
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request->getEvents();

	request->EqualAndUpdateLastEvents();

	// 添加到fd和channel对应表
	fd2chan_[fd] = request;

	// 往事件表中注册fd上的事件
	if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
		perror("epoll_add error");
		fd2chan_[fd].reset();
	}
}

// 修改描述符状态
void Epoll::epoll_mod(SP_Channel request, int timeout) {
	if (timeout > 0)
	{
		add_timer(request, timeout);
	}

	int fd = request->getFd();
	if (!request->EqualAndUpdateLastEvents()) {
		struct epoll_event event;
		event.data.fd = fd;
		event.events = request->getEvents();
		if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
			perror("epoll_mod error");
			fd2chan_[fd].reset();
		}
	}
}

// 从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request) {
	int fd = request->getFd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request->getlastEvents();
	if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
		perror("epoll_del error");
	}
	fd2chan_[fd].reset();
    fd2http_[fd].reset();
}

// epoll_wait封装，等待有events就绪
// 返回活跃事件数
std::vector<SP_Channel> Epoll::poll() {
	// 不断循环，直到取到事件就退出
	while (true) {
		int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
		if (event_count < 0)
		{
			perror("epoll wait error");
		}

		std::vector<SP_Channel> req_data = getEventsRequest(event_count);
		if (req_data.size() > 0)
			return req_data;
	}
}

void Epoll::handleExpired()
{
	timerManager_.handleExpiredEvent();
}

// 取出就绪的events
// 返回有事件请求的channel容器
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num) {
	std::vector<SP_Channel> req_data;
	for (int i = 0; i < events_num; ++i) {
		// 获取有事件产生的描述符,封装到cur_req中，然后返回vector(req_data)
		int fd = events_[i].data.fd;
		
		SP_Channel cur_req = fd2chan_[fd];

		if (cur_req) {
			cur_req->setRevents(events_[i].events);
			cur_req->setEvents(0);
			req_data.push_back(cur_req);
		}
		else {
			std::cout << "SP cur_req is invalid"<<std::endl;
		}
	}
	return req_data;
}

void Epoll::add_timer(SP_Channel request_data, int timeout) {
	std::shared_ptr<HttpData> t = request_data->getHolder();
	if (t)
		timerManager_.addTimer(t, timeout);
	else
		std::cout << "timer add fail" << std::endl;
}

