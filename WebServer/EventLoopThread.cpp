#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread()
  : loop_(NULL),
	exiting_(false),
	// 创建Thread类对象，并给thread绑定函数threadFunc
	thread_(std::bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
	mutex_(),
	cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
	exiting_ = true;
	if(loop_ != NULL) {
		loop_ -> quit();
		thread_.join();
	}
}

EventLoop* EventLoopThread::startLoop() {
	assert(!thread_.started());
	thread_.start();
	{
		MutexLockGuard lock(mutex_);
		//一直等到threadFunc在Thread里真正跑起来, loop()运行
		while(loop_ == NULL)
			cond_.wait();
	}
	return loop_;
}

void EventLoopThread::threadFunc() {
	// 创建一个新的loop
	EventLoop loop;
	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		cond_.notify();
	}

	// loop()开始运行
	loop.loop();
	loop_ = NULL;
}
