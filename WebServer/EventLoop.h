#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include <memory>
#include <vector>
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
#include "assert.h"
#include "base/CurrentThread.h"
#include "base/Thread.h"

class EventLoop {
public:
	typedef std::function<void()> Functor;
	friend class Channel;
	EventLoop();
	~EventLoop();

	void loop();	//事件循环
	void runInLoop(Functor&& cb);
	void queueInLoop(Functor&& cb);

	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

	/*停止不属于当前线程的循环*/
	void assertInLoopThread() {
		assert(isInLoopThread());
	}
	//EventLoop* getEventLoopOfCurrentThread();
	
	void quit();

	void updateChannel(Channel* channel);

	void removeFromPoller(std::shared_ptr<Channel> channel) {
		poller_ -> epoll_del(channel);
	}
	void updatePoller(std::shared_ptr<Channel> channel, int timeout = 0) {
		poller_ -> epoll_mod(channel, timeout);
	}
	void addToPoller(std::shared_ptr<Channel> channel, int timeout = 0) {
		poller_ -> epoll_add(channel, timeout);
	}
private:
	bool looping_;
	const pid_t threadId_;	//所属线程ID
	std::shared_ptr<Epoll> poller_;
	int wakeupFd_;
	bool quit_;
	bool eventHandling_;
	mutable MutexLock mutex_;
	std::vector<Functor> pendingFunctors_;
	bool callingPendingFunctors_;
	std::shared_ptr<Channel> pwakeupChannel_;

	void wakeup();
	void handleRead();
	void doPendingFunctors();
	void handleConn();
};

#endif
