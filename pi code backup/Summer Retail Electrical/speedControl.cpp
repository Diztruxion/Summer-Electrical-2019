//SpeedControl.cpp
/*
Multithreaded support for motor speed control loops
Designed to be running in a continuous loop
Will look at set speed vs measured speed and adjust
Designed to handle starting/stopping

DOES NOT DEAL WITH CHANGING DIRECTION

To be started during the init function
*/

#include <stdio.h>
#include <pigpiod_if2.h>
#include <wiringPi.h>
#include "pwmControl.h"
#include "motorControl.h"
#include "speedControl.h"
#include <stdint.h>

//constructor controls the PI_VALUE, the pin for the encoder on the motor, and the motor itself.

SpeedControl::SpeedControl(int PI_VALUE, unsigned int encA, unsigned int encB, int motor, re_decoderCB_t callback){

	this->PI_VALUE = PI_VALUE;
	this->encA = encA;
	this->encB = encB;
	this->motor = motor;
	this->RPM = 0;
	
	mycallback = callback;
	
	//Initialize callback functions to monitor encoder pins
	callback_A = callback_ex(PI_VALUE, encA, EITHER_EDGE, _pulseEx, this);
	callback_B = callback_ex(PI_VALUE, encB, EITHER_EDGE, _pulseEx, this);
	
}
SpeedControl::~SpeedControl(){
	int temp = 0;
	
	//destroys callback events
	temp = callback_cancel(callback_A);
	temp = callback_cancel(callback_B);
		
}

//For now, the edge/Level isn't actually used... maybe use later for direction
//Also, the pin isn't used either... again may fix this later to do other things.
//A glitch offset will come with each tick... we might be able to ignore since each one has same offset
void SpeedControl::_pulse(int pin, int level, uint32_t tick){
	
	cur_tick = tick;
	
	//If this isn't the first tick, calculate the RPM based on the difference in time.
	//RPM is a float for now... maybe clean up later. PULSE_CONSTANT is defined in header
	if (prev_tick != 0) this->RPM = (cur_tick-prev_tick)*PULSE_CONSTANT;
	
	prev_tick = cur_tick; //set 
	
}

void SpeedControl::_pulseEx(int pin, int level, uint32_t tick, void *user){
	
	//Need a static callback to link with C.
	
	SpeedControl *mySelf = (SpeedControl *) user;

	mySelf_pulse(pin, level, tick); /* Call the instance callback. */
}
	
	

//encoder setup will setup the pins for output, and set debouce/glitch settings  

