#include "Epoll.h"
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

Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
	assert(epollFd_ > 0);
}
Epoll::~Epoll() {}

//注册新描述符
void Epoll::epoll_add(Channel* request, int timeout) {
	int fd = request -> getFd();
	if(timeout > 0) {
		//add_timer(request, timeout);
		//fd2http_[fd] = request -> getHolder();
	}
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request -> getEvents();

	fd2chan_[fd] = request;

	if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
		perror("epoll_add error");
		//fd2chan_[fd].reset();
	}
}

//修改描述符状态
void Epoll::epoll_mod(SP_Channel request, int timeout) {
	if(timeout > 0) //add_timer(request, timeout);
	int fd = request -> getFd();

}

//从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request) {
	int fd = request -> getFd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request -> getlastEvents();
	if(epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
		perror("epoll_del error");
	}
	//fd2chan_[fd].reset();
 //	fd2http_[fd].reset();
}

//返回活跃事件数
std::vector<Channel*> Epoll::poll() {
	while(true) {
		int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
		std::cout<<"event_count = "<<event_count<<std::endl;
		if(event_count < 0) perror("epoll wait error");
		std::vector<Channel*> req_data = getEventsRequest(event_count);
		if(req_data.size() > 0) return req_data;
	}
}

//void Epoll::handleExpired() {timerManager_.handleExpiredEvent();}

//分发处理函数

std::vector<Channel*> Epoll::getEventsRequest(int events_num) {
	std::vector<Channel*> req_data;
	//std::vector<SP_Channel> req_data;
	for(int i = 0; i < events_num; ++i) {
		int fd = events_[i].data.fd;
		
	std::cout<<"get events request running..fd="<<fd<<std::endl;
		Channel* cur_req = fd2chan_[fd];
		if(cur_req) {
			cur_req -> setRevents(events_[i].events);
			cur_req -> setEvents(0);
			req_data.push_back(cur_req);
		}
		else {std::cout << "SP cur_req is invalid"<<std::endl;}
	}
	return req_data;
}
/*
void Epoll::add_timer(SP_Channel request_data, int timeout) {
	
}
*/
