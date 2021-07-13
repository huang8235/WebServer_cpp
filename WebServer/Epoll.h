#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Channel.h"

class Epoll {
public:
	Epoll();
	~Epoll();
	void epoll_add(Channel* request, int timeout);
	void epoll_mod(SP_Channel request, int timeout);
	void epoll_del(SP_Channel request);
	std::vector<Channel*> poll();
	std::vector<Channel*> getEventsRequest(int events_num);
	void add_timer(std::shared_ptr<Channel> request_data, int timeout);
	int getEpollFd() {return epollFd_;}
	//void handleExpired();

private:
	static const int MAXFDS = 100000;
	int epollFd_;
	std::vector<epoll_event> events_;
	//std::shared_ptr<Channel> fd2chan_[MAXFDS];
	Channel* fd2chan_[MAXFDS];
};

#endif
