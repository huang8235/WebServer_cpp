#include <getopt.h>
#include <string>
#include <iostream>
#include "EventLoop.h"
#include "Server.h"

constexpr int DEFAULT_THREAD_NUM = 4;
constexpr int DEFAULT_PORT_NUM = 80;
constexpr auto LOG_PATH = "./log/WebServer.log";

int main(int argc, char* argv[]) {

	// 默认参数
	int threadNum = DEFAULT_THREAD_NUM;
	int port = DEFAULT_PORT_NUM;
	std::string logPath = LOG_PATH;

	int opt;
	const char *str = "t:l:p:";
	while ((opt = getopt(argc, argv, str)) != -1) {
		switch (opt) {
			case 't': {
				threadNum = atoi(optarg);
				break;
			}
			case 'l': {
				logPath = optarg;
				if(logPath.size() < 2 || optarg[0] != '/') {
					std::cout << "logPath should start with \"/\"\n" << std::endl;
					abort();
				}
				break;
			}
			case 'p': {
				port = atoi(optarg);
				break;
			}
			default:
				break;
		}
	}

#ifndef _PTHREADS
	std::cout << "_PTHREADS is not defined!" << std::endl;
#endif

	EventLoop mainLoop;
	// 准备工作:建立socket监听符，线程池，工作线程等等
	Server myHttpServer(&mainLoop, threadNum, port);
	myHttpServer.start();
	std::cout << "Server start" << std::endl;
	// 主线程loop循环开始，监听新连接事件，分发事件符
	mainLoop.loop();
	return 0;
}
