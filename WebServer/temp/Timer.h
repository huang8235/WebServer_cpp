/*任务进度：
 *	实现时间轮
 *	未添加回调函数
 */
// @Author Huang Xiaohua

#ifndef TIME_H
#define TIME_H

#include <time.h>
#include <stdio.h>
#include "HttpData.h"

class HttpData;

/* 定义定时器类 */
class TimerNode {
public:
	TimerNode(int rot, int ts);
public:
	TimerNode* next;	//指向下一个定时器
	TimerNode* prev;	//指向下一个定时器
	int rotation; 		//记录定时器在时间轮转多少圈后生效
	int time_slot;		//记录定时器属于时间轮上的哪个槽
	//void (*cb_func) (HttpData*);	//定时器回调函数
};

/* 定义时间轮类 */
class TimeWheel {
public:
	TimeWheel();
	~TimeWheel();

	/* 根据timeout创建定时器，并插入合适的槽中。*/
	/*该函数返回一个TimerNode对象 */
	TimerNode* add_timernode(int timeout);
	/* 删除目标定时器time node */
	void del_timernode(TimerNode* node);
	/* SI时间到，调用该函数，时间轮向前滚动一个槽的间隔 */
	void tick();
	
private:
	static const int N = 60;	//时间轮上槽的数目
	static const int SI = 1;	//每1s时间轮转动一次，即槽间隔为1s
	TimerNode* slots[N];		//各个时间轮的槽头结点，代指各个槽
	int cur_slot;				//时间轮的当前槽
};

#endif
