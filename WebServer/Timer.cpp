#include "Timer.h"


TimerNode::TimerNode(int rot, int ts)
		: next(NULL), prev(NULL), rotation(rot), time_slot(ts) {}


TimeWheel::TimeWheel() : cur_slot(0){
	for(int i = 0; i < N; ++i) {
		slots[i] = NULL;			//初始化每个槽的头结点
	}
}

TimeWheel::~TimeWheel() {
	for(int i = 0; i < N; ++i){
		TimerNode* tmp = slots[i];
		while(tmp){
			slots[i] = tmp->next;
			delete tmp;
			tmp = slots[i];
		}
	}
}

TimerNode* TimeWheel::add_timernode(int timeout) {
	if(timeout < 0){
		return NULL;	
	}
	int ticks = 0;		//ticks记录滴答数，即定时器经过ticks个SI时间后触发
	if(timeout < SI) {
		ticks = 1;
	}
	else {
		ticks = timeout / SI;
	}
	int rotation = ticks / N;
	int ts = (cur_slot + (ticks % N)) % N;
	TimerNode* timernode = new TimerNode(rotation, ts);
	/* 如果第ts个槽尚无任何定时器，则将该定时器设为该槽的头结点 */
	if(!slots[ts]){
		slots[ts] = timernode;
	}
	/* 否则，将该槽原来的头结点后移，新定时器同样成为该槽头结点 */
	else{
		timernode->next = slots[ts];
		slots[ts]->prev = timernode;
		slots[ts] = timernode;
	}
	return timernode;
}

void TimeWheel::del_timernode(TimerNode* node){
	if(!node){
		return;
	}
	int ts = node->time_slot;
	/* slots[ts]是目标定时器所在槽的头结点。如果目标定时器就是该头结点，则重置该槽头结点 */
	if(node == slots[ts]) {
		slots[ts] = slots[ts]->next;
		if(slots[ts]) {
			slots[ts]->prev = NULL;
		}
		delete node;
	}
	else {
		node->prev->next = node->next;
		if(node->next) {
			node->next->prev = node->prev;
		}
		delete node;
	}
}

void TimeWheel::tick() {
	TimerNode* tmp = slots[cur_slot];	//tmp为时间轮上当前槽的头结点
	/* 遍历该槽上的所有定时器 */
	while(tmp) {
		/* rotation>0 说明这个这个定时器定时时间还没到 */
		if(tmp->rotation > 0) {
			tmp->rotation--;
			tmp = tmp->next;
		}
		/* 否则，说明定时器到期，则执行定时任务，然后删除该定时器 */
		else{
			//tmp->cb_func(tmp->user_data);	//执行回调函数
			if(tmp == slots[cur_slot]) {
				slots[cur_slot] = tmp->next;
				delete tmp;
				if(slots[cur_slot]){
					slots[cur_slot]->prev = NULL;
				}
				tmp = slots[cur_slot];
			}
			else {
				tmp->prev->next = tmp->next;
				if(tmp->next) {
					tmp->next->prev = tmp->prev;
				}
				TimerNode* tmp2 = tmp->next;
				delete tmp;
				tmp = tmp2;
			}
		}
	}
	cur_slot = ++cur_slot % N;
}
