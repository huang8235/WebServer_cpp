// @Author Huang Xiaohua
// 每个Channel对象自始至终只负责一个文件描述符fd的IO事件分发
// Channel是一个通道 连接loop和对应的fd的通道

#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"

class EventLoop;
class HttpData;


class Channel {
private:
	typedef std::function<void()> CallBack;
	EventLoop *loop_;
	int fd_;
	__uint32_t events_;		// 这个channel关心的IO事件
	__uint32_t revents_;	// 目前活动的事件
	__uint32_t lastEvents_;

	// 方便找到上层持有该Channel的对象
	std::weak_ptr<HttpData> holder_;
	
public:
	Channel(EventLoop* loop, int fd);
	Channel(EventLoop* loop);
	~Channel();
	int getFd();
	void setFd(int fd);

	void setHolder(std::shared_ptr<HttpData> holder) {holder_ = holder;}
	std::shared_ptr<HttpData> getHolder() {
		std::shared_ptr<HttpData> ret(holder_.lock());
		return ret;
	}

	void setReadHandler(CallBack &&readHandler) {
		readHandler_ = readHandler;
	}
	void setWriteHandler(CallBack &&writeHandler) {
		writeHandler_ = writeHandler;
	}
	void setErrorHandler(CallBack &&errorHandler) {
		errorHandler_ = errorHandler;
	}
	void setConnHandler(CallBack &&connHandler) {
		connHandler_ = connHandler;
	}


	void handleEvents();	// Channel的核心，由EventLoop::loop()调用，它的功能是根据revents_的值分别调用不同的用户回调。
	void handleRead();
	void handleWrite();
	void handleError(int fd, int err_num, std::string short_msg);
	void handleConn();

	void setRevents(__uint32_t ev) {revents_ = ev;}
	void setEvents(__uint32_t ev) {events_ = ev;}
	__uint32_t &getEvents() {return events_;}
	__uint32_t getlastEvents() {return lastEvents_;}

	bool EqualAndUpdateLastEvents() {
		bool ret = (lastEvents_ == events_);
		lastEvents_ = events_;
		return ret;
	}


private:
	int parse_URI();
	int parsr_Headers();
	int analysisRequest();

	/* 读事件、写事件、错误事件、连接事件回调函数 */
	CallBack readHandler_;
	CallBack writeHandler_;
	CallBack errorHandler_;
	CallBack connHandler_;

};

typedef std::shared_ptr<Channel> SP_Channel;

#endif
