#include <strings.h>
#include <sys/timerfd.h>
#include <iostream>
#include "EventLoop.h"

EventLoop* g_loop;

void timeout() {
	std::cout << "Timeout!\n";
	g_loop -> quit();
}

int main() {
	EventLoop loop;
	g_loop = &loop;

	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	
	Channel channel(&loop, timerfd);
	channel.setReadHandler(timeout);

	struct itimerspec howlong;
	bzero(&howlong, sizeof howlong);
	howlong.it_value.tv_sec = 5;
	::timerfd_settime(timerfd, 0, &howlong, NULL);

	loop.loop();
	std::cout<<"loop run..."<<std::endl;

	::close(timerfd);
}
