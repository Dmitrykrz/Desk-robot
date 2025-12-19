#include "Korzhenevskiy_tick_tack_2.h"


Korzhenevskiy_tick_tack_2* Korzhenevskiy_tick_tack_2::registry[Korzhenevskiy_tick_tack_2::len]{};

Korzhenevskiy_tick_tack_2::Korzhenevskiy_tick_tack_2(fptr callback, uint32_t timer) {
	
	for (int place_for_new_object = 0; place_for_new_object < len; place_for_new_object++)
	{
		
		if (registry[place_for_new_object] == 0)
		{
			registry[place_for_new_object] = (this);
			
			break;
		}
	}

	_timer = timer;
	this->callback = callback;
	_enabled = false;
	_lastTime = 0;
	counts = 0;

}

Korzhenevskiy_tick_tack_2::~Korzhenevskiy_tick_tack_2() {}

void Korzhenevskiy_tick_tack_2::start() {
	_lastTime = millis();
	_enabled = true;
	}
void Korzhenevskiy_tick_tack_2::start(uint32_t timer) {
	_lastTime = millis();
	_enabled = true;
	_timer = timer;
}


void Korzhenevskiy_tick_tack_2::stop() {
	_enabled = false;
	}


void Korzhenevskiy_tick_tack_2::tick() {
	if (_enabled) 
	{

		if ((millis() - _lastTime) >= _timer)
		{
			_lastTime = millis();
			counts++;
			callback();
		}
	}



}


void Korzhenevskiy_tick_tack_2::mass_start()
{
	for (int x = 0; x < len; x++)
	{
		if (registry[x] != nullptr) registry[x]->start();
		else break;
	}
	
}


void Korzhenevskiy_tick_tack_2::mass_tick()
{
	
	for (int x = 0; x < len; x++)
	{		
		if (registry[x] != nullptr) registry[x]->tick(); 
		else break;
	}


}


void Korzhenevskiy_tick_tack_2::mass_stop()
{
	for (int x = 0; x < len; x++)
	{
		if (registry[x] != nullptr) registry[x]->stop();
		else break;
	}

}


void Korzhenevskiy_tick_tack_2::setinterval(int timer) {
	_timer = timer;
}


void Korzhenevskiy_tick_tack_2::restart() {
	_lastTime = millis();

}


void Korzhenevskiy_tick_tack_2::zero_count() {
		counts = 0;
}

bool Korzhenevskiy_tick_tack_2::enabled() {
	return _enabled;
}
