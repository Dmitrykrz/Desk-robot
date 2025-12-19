 
#include <stdint.h>
#include "Arduino.h"
#define max_number_of_timers 10  
//memory usage is max_number_of_timers*2 bytes

typedef void(*fptr)();

class Korzhenevskiy_tick_tack_2
{

	static constexpr int len = max_number_of_timers;
	static Korzhenevskiy_tick_tack_2* registry[max_number_of_timers];

public:

	Korzhenevskiy_tick_tack_2(fptr callback, uint32_t timer);
	~Korzhenevskiy_tick_tack_2();
	void start();
	void start(uint32_t timer);
	void stop();
	void tick();
	void setinterval(int timer);
	void restart();
	void zero_count();
	bool enabled();
	static void mass_start();
	static void mass_tick();
	static void mass_stop();
	int counts;
	

private:

	bool _enabled;
	uint32_t _timer;
	fptr callback;
	uint32_t _lastTime;

};





