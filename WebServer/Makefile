test1: test1.o EventLoop.o Channel.o Epoll.o
	g++ -o $@ $^
test1.o: test1.cpp EventLoop.h Channel.h Epoll.h
	g++ -c $< -o $@
EventLoop.o: EventLoop.cpp Channel.h Epoll.h CurrentThread.h
	g++ -c $< -o $@
Channel.o: Channel.cpp EventLoop.h Epoll.h
	g++ -c $< -o $@
Epoll.o: Epoll.cpp EventLoop.h Channel.h
	g++ -c $< -o $@
