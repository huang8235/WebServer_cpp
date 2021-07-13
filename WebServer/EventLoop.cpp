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
	std::cout<<"loop run.."<<std::endl;
	assert(!looping_);			//事件循环前要进行检查，确保调用该事件循环的线程
	assert(isInLoopThread());	//就是该EventLoop所属的线程。
	looping_ = true;
	quit_ = false;
	
	//::poll(NULL, 0, 5 * 1000);
	//事件循环要做的事写在这里
	std::vector<Channel*> ret;
	while(!quit_) {
		std::cout<<"handling event.."<<std::endl;
		ret.clear();
		ret = poller_ -> poll();
		std::cout<<"poll done.."<<std::endl;
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

void EventLoop::updateChannel(Channel* channel) {
	poller_ -> epoll_add(channel, 0);	
}
