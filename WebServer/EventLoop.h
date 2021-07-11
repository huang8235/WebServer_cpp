#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include "assert.h"
#include "CurrentThread.h"
#include "Epoll.h"
#include "Thread.h"

class EventLoop {
public:
	EventLoop();
	~EventLoop();

	void loop();	//事件循环

	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

	/*停止不属于当前线程的循环*/
	void assertInLoopThread() {
		assert(isInLoopThread());
	}
	//EventLoop* getEventLoopOfCurrentThread();
	
	void quit();
private:
	bool looping_;
	const pid_t threadId_;	//所属线程ID
	std::shared_ptr<Epoll> poller_;
	bool quit_;
};

#endif
