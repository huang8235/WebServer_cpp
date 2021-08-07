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

void EventLoop::wakeup() {
	uint64_t one = 1;
	ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
	if(n != sizeof one) {
		std::cout << "EventLoop::wakeup() writes" << n << "bytes instead of 8" << std::endl;
	}
}

void EventLoop::handleRead() {
	uint64_t one = 1;
	ssize_t n = readn(wakeupFd_, &one, sizeof one);
	if(n != sizeof one) {
		std::cout << "EventLoop::handleRead() reads" << n << "bytes instead of 8" << std::endl;
	}
	pwakeupChannel_ -> setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb) {
	if(isInLoopThread())	cb();
	else 					queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb) {
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.emplace_back(std::move(cb));
	}

	if(!isInLoopThread() || callingPendingFunctors_) wakeup();
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
		//std::cout<<"handling event.."<<std::endl;
		ret.clear();
		ret = poller_ -> poll();
		eventHandling_ = true;
		for(auto& it : ret) it -> handleEvents();
		eventHandling_ = false;
		doPendingFunctors();
		poller_ -> handleExpired();
	}

	looping_ = false;
}

void EventLoop::quit() {
	quit_ = true;
	if(!isInLoopThread()) {
		//wakeup();
	}
}

/*
void EventLoop::updateChannel(Channel* channel) {
	poller_ -> epoll_add(channel, 0);	
}
*/
