#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <queue>

#include "EventLoop.h"
#include "Channel.h"

Channel::Channel(EventLoop* loop)
	: loop_(loop), fd_(0), events_(0), lastEvents_(0) {}

Channel::Channel(EventLoop* loop, int fd)
	: loop_(loop), fd_(fd), events_(0), lastEvents_(0) {}

Channel::~Channel() {

}

int Channel::getFd() {return fd_;}
void Channel::setFd(int fd) {fd_ = fd;}

void Channel::handleRead() {
	if(readHandler_) {
		readHandler_();
	}
}
void Channel::handleWrite() {
	if(writeHandler_) {
		writeHandler_();
	}
}
void Channel::handleConn() {
	if(connHandler_) {
		connHandler_();
	}
}

void Channel::handleEvents() {
	events_ = 0;
	if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
		events_ = 0;
		return;
	}
	if(revents_ & EPOLLERR) {
		if(errorHandler_) errorHandler_();
		events_ = 0;
		return;
	}
	if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
		handleRead();
	}
	if(revents_ & EPOLLOUT) {
		handleWrite();
	}
	handleConn();
}

void Channel::addEvents() {
	events_ = EPOLLIN;	
	loop_ -> poller_ -> epoll_add(this, 0);
}


void Channel::update() {
	events_ = EPOLLIN;	
	loop_ -> updateChannel(this);
}
