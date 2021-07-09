#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
	  : looping_(false), 
		threadId_(CurrentThread::tid()),
		poller_(new Epoll()),
		quit_(false)
{
	//LOG << "EventLoop created" << this << "in thread" <<threadId_;
	if(t_loopInThisThread) {
		//LOG_TRACE << "Another EventLoop" << t_loopInThisThread 
			// 	  << "exists in this thread" << threadId_;
	}
	else {
		t_loopInThisThread = this;
	}
}

EventLoop::~EventLoop() {
	assert(!looping_);
	t_loopInThisThread = NULL;
}

void EventLoop::loop() {
	assert(!looping_);			//事件循环前要进行检查，确保调用该事件循环的线程
	assert(isInLoopThread());	//就是该EventLoop所属的线程。
	looping_ = true;
	quit_ = false;
	
	//::poll(NULL, 0, 5 * 1000);
	//事件循环要做的事写在这里
	std::vector<SP_Channel> ret;
	while(!quit_) {
		ret.clear();
		ret = poller_ -> poll();
		for(auto& it : ret) it -> handleEvents();
		//poller_ -> handleExpired();
	}

	looping_ = false;
}

void EventLoop::quit() {
	quit_ = true;
	if(!isInLoopThread()) {
		//wakeup();
	}
}
