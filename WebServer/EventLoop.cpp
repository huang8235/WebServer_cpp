#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>

__thread EventLoop* t_loopInThisThread = 0;

int createEventfd() {
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(evtfd < 0) {
		std::cout << "Failed in eventfd" << std::endl;
		abort();
	}
	return evtfd;
}

EventLoop::EventLoop()
	  : looping_(false), 
		threadId_(CurrentThread::tid()),
		poller_(new Epoll()),
		wakeupFd_(createEventfd()),	//新建事件唤醒的文件描述符
		quit_(false),
		eventHandling_(false),
		callingPendingFunctors_(false),
		pwakeupChannel_(new Channel(this, wakeupFd_)) //新建channel
{
	//LOG << "EventLoop created" << this << "in thread" <<threadId_;
	if(t_loopInThisThread) {
		//LOG_TRACE << "Another EventLoop" << t_loopInThisThread 
			// 	  << "exists in this thread" << threadId_;
	}
	else {
		t_loopInThisThread = this;
	}
	pwakeupChannel_ -> setEvents(EPOLLIN | EPOLLET);
	pwakeupChannel_ -> setReadHandler(std::bind(&EventLoop::handleRead, this));
	pwakeupChannel_ -> setConnHandler(std::bind(&EventLoop::handleConn, this));
	poller_ -> epoll_add(pwakeupChannel_, 0);
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

void EventLoop::handleConn() {
	updatePoller(pwakeupChannel_, 0);
}

void EventLoop::handleRead() {
	//读到wake()函数写的8个字节
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
		pendingFunctors_.emplace_back(std::move(cb)); //把实参传递给构造函数,move是数据转移
	}

	if(!isInLoopThread() || callingPendingFunctors_) wakeup();//唤醒fd写入数据
}

void EventLoop::loop() {
	assert(!looping_);			//事件循环前要进行检查，确保调用该事件循环的线程
	assert(isInLoopThread());	//就是该EventLoop所属的线程。
	looping_ = true;
	quit_ = false;
	
	//事件循环要做的事写在这里
	std::vector<SP_Channel> ret;
	//不断循环取出eventloop中epoll获得的事件
	while(!quit_) {
		ret.clear();
		//调用epoll_wait()，取到事件就退出返回到这里
		//一般wakeupChannel会阻塞在这里
		//当主线程有新的连接到来，分发给IO线程时，该IO线程因阻塞无法更新epoll注册表
		//因此需要有一个唤醒操作，主动往wakeupChannel写字节，epoll_wait()会取到事件，程序往下走好进行epoll事件表注册
		ret = poller_ -> poll();
		eventHandling_ = true;
		//处理epoll_wait拿到的事件
		for(auto& it : ret) it -> handleEvents();
		eventHandling_ = false;
		//处理未决事件
		//未决事件就是唤醒操作时需要进行的epoll事件表注册
		//至此，该IO线程开始监听主线程分配的新连接socketfd
		doPendingFunctors();
		//主线程处理过期连接
		poller_ -> handleExpired();
	}

	looping_ = false;
}

void EventLoop::quit() {
	quit_ = true;
	if(!isInLoopThread()) {
		wakeup();
	}
}

void EventLoop::doPendingFunctors() {
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for(size_t i = 0; i < functors.size(); ++i)	functors[i]();
	callingPendingFunctors_ = false;
}
