#include "Timer.h"

int main() {
	TimeWheel wheel;
	wheel.add_timernode(5);
	while(1) {
		wheel.tick();
	}
	return 0;
}


